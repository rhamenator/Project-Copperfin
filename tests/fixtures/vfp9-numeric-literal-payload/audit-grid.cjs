// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Capture-integrity/type audit only. No conversion algorithm or product import.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2];
const source=fs.readFileSync(path.join(__dirname,'grid.prg'),'utf8');
const inventory=JSON.parse(fs.readFileSync(path.join(__dirname,'cases.json')));
a.equal(inventory.bases.length,245);a.equal(inventory.rows.length,980);
a.deepEqual(inventory.settings,[0,18]);
const declared=new Map(inventory.rows.map(row=>[row.index,row]));
a.deepEqual([...declared.keys()],Array.from({length:980},(_,i)=>i+1));
a.equal([...source.matchAll(/^expressions\[\d+\]='/gm)].length,980);
a.equal([...source.matchAll(/^ CASE index=\d+$/gm)].length,980);
for(const row of inventory.rows) {
    a(source.includes(`expressions[${row.index}]='${row.expression}'`));
    a(source.includes(` CASE index=${row.index}\n  RETURN CFNMETA(${row.expression})+'|'+HexBytes(BINTOC(${row.expression},'BRS'))+'|'+HexBytes(TRANSFORM(${row.expression}))`));
}
function descriptor(text) {
    a.match(text,/^type=(?:49|4E);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
    const raw=Object.fromEntries(text.split(';').map(p=>p.split('=')));
    const typed={type:raw.type==='49'?'I':'N',width:parseInt(raw.width,16),cw:raw.cw};
    if(typed.type==='I') {
        typed.integer=parseInt(raw.integer,16)|0;
        const b=Buffer.alloc(8);b.writeDoubleBE(typed.integer);typed.bits=b.toString('hex').toUpperCase();
    } else {typed.decimals=parseInt(raw.decimals,16);typed.bits=raw.real;}
    return {raw,typed};
}
function bytes(name) {
    const file=path.join(root,name);
    return fs.existsSync(file)?fs.readFileSync(file):zlib.gunzipSync(fs.readFileSync(file+'.gz'));
}
function capture(name) {
    a.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');
    a.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
    const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));
    const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
    a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    a.equal(lines.pop(),'DONE|7840');a.equal(lines.length,7840);
    const rows=new Map();let emptyDisplays=0;
    for(const line of lines) {
        const fields=line.split('|');a.equal(fields.length,8,line);
        const [mode,settingText,fixed,indexText,expressionHex]=fields;
        const setting=Number(settingText),index=Number(indexText),row=declared.get(index);
        a(['COMPILED','EVAL'].includes(mode));a(inventory.settings.includes(setting));
        a(['OFF','ON'].includes(fixed));a(row);
        a.equal(Buffer.from(expressionHex,'hex').toString('ascii'),row.expression);
        const result=descriptor(fields[5]);a.equal(fields[6],result.typed.bits);
        a.match(fields[7],/^(?:[0-9A-F]{2})*$/);if(fields[7]==='')++emptyDisplays;
        const key=JSON.stringify([setting,fixed,index,mode]);a(!rows.has(key));
        rows.set(key,{fields,typed:{setting,fixed,index,mode,result:result.typed,displayHex:fields[7]}});
    }
    const compilerEvaluateDifferences=[];
    for(const setting of inventory.settings)for(const fixed of ['OFF','ON'])for(const row of inventory.rows) {
        const x=rows.get(JSON.stringify([setting,fixed,row.index,'EVAL']));
        const y=rows.get(JSON.stringify([setting,fixed,row.index,'COMPILED']));a(x&&y);
        if(JSON.stringify({...x.typed,mode:null})!==JSON.stringify({...y.typed,mode:null}))
            compilerEvaluateDifferences.push({setting,fixed,index:row.index,expression:row.expression,eval:x.typed,compiled:y.typed});
    }
    return {raw,rows,emptyDisplays,compilerEvaluateDifferences};
}
const first=capture('first'),repeat=capture('repeat'),unused={};
let rawDifferentRows=0;
for(const [key,x] of first.rows) {
    const y=repeat.rows.get(key);a(y);a.deepEqual(x.typed,y.typed);
    if(x.fields.join('|')!==y.fields.join('|'))++rawDifferentRows;
    for(let i=0;i<8;++i)if(i!==5)a.equal(x.fields[i],y.fields[i]);
    const p=descriptor(x.fields[5]).raw,q=descriptor(y.fields[5]).raw;
    for(const field of Object.keys(p))if(p[field]!==q[field]) {
        a(p.type==='49'?['real','decimals'].includes(field):field==='integer');
        const label=`${p.type}:${field}`;unused[label]=(unused[label]||0)+1;
    }
}
a.deepEqual(first.compilerEvaluateDifferences,repeat.compilerEvaluateDifferences);
const sha=x=>crypto.createHash('sha256').update(x).digest('hex');
console.log(JSON.stringify({expressions:980,valuesPerRun:7840,errors:0,documentedFieldsRepeatEqual:true,
    compilerEvaluateDifferences:first.compilerEvaluateDifferences,emptyDisplaysPerRun:first.emptyDisplays,
    rawByteEqual:first.raw.equals(repeat.raw),rawDifferentRows,rawDifferencesConfinedToUnusedSlots:unused,
    firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),
    limitation:'Observed records, not an accepted conversion algorithm or product parity. Displays retained only; no display migration accepted.',
    observations:[...first.rows.values()].map(x=>x.typed)},null,2));
