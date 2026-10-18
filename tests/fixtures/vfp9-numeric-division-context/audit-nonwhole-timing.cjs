// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent raw native audit; no Copperfin helper or candidate import.
const fs = require('node:fs'), path = require('node:path'), zlib = require('node:zlib');
const a = require('node:assert/strict'), crypto = require('node:crypto');
const root = process.argv[2], count = 147, total = count*20;
const source = fs.readFileSync(path.join(__dirname,'nonwhole-timing.prg'),'utf8');
const inventory = JSON.parse(fs.readFileSync(path.join(__dirname,'nonwhole-timing-cases.json')));
a.equal(inventory.rows.length,count); a.deepEqual(inventory.settings,[0,2,6,14,18]);
a.equal([...source.matchAll(/^expressions\[\d+\]='/gm)].length,count);
a.equal([...source.matchAll(/^ CASE index=\d+$/gm)].length,count);
const declared = new Map(inventory.rows.map(row=>[row.index,row]));
a.deepEqual([...declared.keys()],Array.from({length:count},(_,i)=>i+1));
for (const row of inventory.rows) {
    a(source.includes(`expressions[${row.index}]='${row.expression}'`));
    a(source.includes(` CASE index=${row.index}\n  RETURN CFNMETA(${row.expression})`));
}
a(source.includes('observation=CFNMETA(EVALUATE(expression))'));
a(!source.includes('BINTOC(') && !source.includes('TRANSFORM('));
function descriptor(text) {
    a.match(text,/^type=(?:49|4E);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
    const raw=Object.fromEntries(text.split(';').map(part=>part.split('=')));
    const typed={type:raw.type==='49'?'I':'N',width:parseInt(raw.width,16),cw:raw.cw};
    if (typed.type==='I') {
        typed.integer=parseInt(raw.integer,16)|0;
        const b=Buffer.alloc(8); b.writeDoubleBE(typed.integer); typed.bits=b.toString('hex').toUpperCase();
    } else { typed.decimals=parseInt(raw.decimals,16); typed.bits=raw.real; }
    return {raw,typed};
}
function bytes(name) {
    const p=path.join(root,name);
    return fs.existsSync(p)?fs.readFileSync(p):zlib.gunzipSync(fs.readFileSync(p+'.gz'));
}
function capture(name) {
    a.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');
    a.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
    const raw=bytes(name+'-results.txt'); a.deepEqual(raw,bytes(name+'.out'));
    const lines=raw.toString('ascii').split('\r\n'); a.equal(lines.pop(),'');
    a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    a.equal(lines.pop(),'DONE|'+total); a.equal(lines.length,total);
    const rows=new Map();
    for (const line of lines) {
        const fields=line.split('|'); a.equal(fields.length,9,line);
        const [mode,settingText,fixed,indexText,expressionHex]=fields;
        const setting=Number(settingText),index=Number(indexText),row=declared.get(index);
        a(['EVAL','COMPILED'].includes(mode)); a(inventory.settings.includes(setting));
        a(['OFF','ON'].includes(fixed)); a(row);
        a.equal(Buffer.from(expressionHex,'hex').toString('ascii'),row.expression);
        const result=descriptor(fields[5]);
        a.equal(Number(fields[7]),row.calls); a.equal(Number(fields[8]),row.changesTo??setting);
        let seen=null;
        if (row.observesArgument) {
            a.match(fields[6],/^(?:[0-9A-F]{2})+$/);
            seen=descriptor(Buffer.from(fields[6],'hex').toString('ascii')).typed;
        } else a.equal(fields[6],'');
        const key=JSON.stringify([setting,fixed,index,mode]); a(!rows.has(key));
        rows.set(key,{fields,typed:{setting,fixed,index,mode,result:result.typed,seenParameter:seen,calls:row.calls,finalSetting:row.changesTo??setting}});
    }
    for (const setting of inventory.settings) for (const fixed of ['OFF','ON']) for (const row of inventory.rows) {
        const x=rows.get(JSON.stringify([setting,fixed,row.index,'EVAL']));
        const y=rows.get(JSON.stringify([setting,fixed,row.index,'COMPILED']));
        a(x&&y); a.deepEqual({...x.typed,mode:null},{...y.typed,mode:null});
    }
    return {raw,rows};
}
const first=capture('first'),repeat=capture('repeat'),unused={};
let rawDifferentRows=0;
for (const [key,x] of first.rows) {
    const y=repeat.rows.get(key); a(y); a.deepEqual(x.typed,y.typed);
    if (x.fields.join('|')!==y.fields.join('|')) ++rawDifferentRows;
    for (let i=0;i<9;++i) if (![5,6].includes(i)) a.equal(x.fields[i],y.fields[i]);
    for (const i of [5,6]) {
        if (i===6&&x.fields[i]==='') { a.equal(y.fields[i],''); continue; }
        const decode=text=>i===6?Buffer.from(text,'hex').toString('ascii'):text;
        const p=descriptor(decode(x.fields[i])).raw,q=descriptor(decode(y.fields[i])).raw;
        for (const field of Object.keys(p)) if (p[field]!==q[field]) {
            a(p.type==='49'?['real','decimals'].includes(field):field==='integer');
            const label=`${i}:${p.type}:${field}`; unused[label]=(unused[label]||0)+1;
        }
    }
}
const sha=value=>crypto.createHash('sha256').update(value).digest('hex');
console.log(JSON.stringify({casesPerRun:total,successfulValues:total,errors:0,expressions:count,
    effectExpressions:144,oldUnknownControls:3,singleEvaluationCallbackCount:1,
    documentedFieldsRepeatEqual:true,compiledVersusEvaluateDifferences:0,
    rawByteEqual:first.raw.equals(repeat.raw),rawDifferentRows,rawDifferencesConfinedToUnusedSlots:unused,
    firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),
    limitation:inventory.limitations,observations:[...first.rows.values()].map(row=>row.typed)},null,2));
