// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Pre-observation holdout source and explicitly hypothetical predictions.
const fs=require('node:fs'),a=require('node:assert/strict');
const training=JSON.parse(fs.readFileSync(__dirname+'/cases.json'));
const previous=new Set(training.bases.map(x=>x.literal));
const rows=[],seen=new Set();
function add(literal,label) {
    a.match(literal,/^[0-9]+\.[0-9]+$/);a(literal.length<=256);
    if(seen.has(literal))return;
    seen.add(literal);rows.push({index:rows.length+1,literal,expression:literal,label,
        kind:previous.has(literal)?'calibration-overlap':'heldout'});
}
for(const length of [3,4,17,18,19,20,32,39,40,41,64,128,200,255,256]) {
    add('0'.repeat(length-3)+'1.0','leading-zero width boundary');
    add('1.0'+'0'.repeat(length-3),'trailing-zero scale boundary');
    add('0.'+'0'.repeat(length-3)+'1','small fraction/scale boundary');
    add('9'.repeat(length-2)+'.9','large whole/width boundary');
    add('0.'+'1234567890'.repeat(26).slice(0,length-2),'long fraction boundary');
}
let state=0x7206F19D;
const next=()=>state=(Math.imul(state,1664525)+1013904223)>>>0;
const lengths=[1,2,7,9,10,15,16,17,18,19,20,23,32,40,64,128,200,254];
for(let i=0;i<144;++i) {
    const length=lengths[i%lengths.length];let coefficient=String(1+next()%9);
    for(let j=1;j<length;++j)coefficient+=String(next()%10);
    const choices=[1,length,Math.max(1,Math.floor(length/2)),Math.max(1,length-1)];
    const scale=choices[next()%choices.length],digits=coefficient.padStart(scale+1,'0');
    add(digits.slice(0,-scale)+'.'+digits.slice(-scale),'seeded independent coefficient/scale');
}
const inventory={requirement:'RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001',
    baseline:training.baseline,settings:[0,18],seed:'7206F19D',rows,
    limitation:'Positive bare source literals through256 bytes; native full-expression sign/wrapper length limits are not inferred. Calibration overlap explicitly counted.'};
const bits=value=>{const b=Buffer.alloc(8);b.writeDoubleBE(value);return b.toString('hex').toUpperCase();};
function candidate(literal) {
    const [whole,fraction]=literal.split('.');let integer=0,numerator=0,divisor=1;
    for(const digit of whole)integer=integer*10+Number(digit);
    for(const digit of fraction){numerator=numerator*10+Number(digit);divisor*=10;}
    return bits(integer+numerator/divisor);
}
const predictions={kind:'PRE-CAPTURE HYPOTHESIS ONLY, NOT ACTUAL NATIVE EXPECTATIONS',
    algorithm:'Separate binary64 Horner whole/fraction accumulators; repeated-multiply divisor; whole plus fraction/divisor.',
    rows:rows.map(row=>({index:row.index,literal:row.literal,bits:candidate(row.literal)}))};
let source=fs.readFileSync(__dirname+'/grid.prg','utf8');
source=source.replace(/LOCAL ARRAY expressions\[980\],settings\[2\]\n(?:expressions\[\d+\]='[^\n]*'\n)+/,
    `LOCAL ARRAY expressions[${rows.length}],settings[2]\n${rows.map(r=>`expressions[${r.index}]=${r.expression.match(/.{1,120}/g).map(part=>"'"+part+"'").join('+')}`).join('\n')}\n`);
source=source.replace('grid-results.txt','holdout-results.txt');
source=source.replace(/FUNCTION CompiledObservation\n[\s\S]*?\nFUNCTION HexBytes/,
    `FUNCTION CompiledObservation\n LPARAMETERS index\n DO CASE\n${rows.map(r=>` CASE index=${r.index}\n  RETURN CFNMETA(${r.expression})+'|'+HexBytes(BINTOC(${r.expression},'BRS'))+'|'+HexBytes(TRANSFORM(${r.expression}))`).join('\n')}\n ENDCASE\n RETURN ''\n\nFUNCTION HexBytes`);
a.equal([...source.matchAll(/^expressions\[\d+\]='/gm)].length,rows.length);
a.equal([...source.matchAll(/^ CASE index=\d+$/gm)].length,rows.length);
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inventory,null,2)+'\n');
else if(process.argv[2]==='predictions')process.stdout.write(JSON.stringify(predictions,null,2)+'\n');
else console.log(JSON.stringify({expressions:rows.length,heldout:rows.filter(r=>r.kind==='heldout').length,
    calibration:rows.filter(r=>r.kind!=='heldout').length,observationsPerRun:rows.length*8,
    maximumLiteralBytes:Math.max(...rows.map(r=>r.literal.length))}));
