// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Actual-observation reader only; no product, candidate or expected imports.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'intervention-cases.json')));
const source=fs.readFileSync(path.join(__dirname,'intervention.prg'),'utf8');
a.equal(inv.rows.length,384);a.equal(inv.stored.length,12);a.equal(inv.recordsPerRun,2736);
a.deepEqual(inv.settings,[0,2,18]);a.deepEqual(inv.widths,[1,2,3,4,12,20,40]);
a.deepEqual(inv.scales,[0,1,2,14,15,18]);
for(const row of inv.rows)a(source.includes(`expressions[${row.index}]='${row.expression}'`));
for(const row of inv.stored){a(source.includes(`  ${row.name}=${row.expression}\n`));a(source.includes(`CFNMETA(${row.name})`));}
const bytes=name=>{const p=path.join(process.argv[2],name);return fs.existsSync(p)?fs.readFileSync(p):zlib.gunzipSync(fs.readFileSync(p+'.gz'));};
const hex=s=>a.match(s,/^(?:[0-9A-F]{2})*$/);
function typed(meta){
    a.match(meta,/^type=4E;width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
    const raw=Object.fromEntries(meta.split(';').map(p=>p.split('=')));
    const r={type:'N',width:parseInt(raw.width,16),scale:parseInt(raw.decimals,16),bits:raw.real,cw:raw.cw};
    a(r.width>=1&&r.width<=40);a(r.scale>=0&&r.scale<=18);
    a(Number.isFinite(Buffer.from(r.bits,'hex').readDoubleBE()));return r;
}
function read(name){
    a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);
    const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));
    const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
    a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    a.equal(lines.pop(),`DONE|${inv.recordsPerRun}`);a.equal(lines.length,inv.recordsPerRun);
    const rows=new Map(),fields=new Map();
    for(const line of lines){
        const f=line.split('|');a.equal(f.length,12);a(['GRID','STORED'].includes(f[0]));
        const family=f[0],birth=f[1]==='-'?null:Number(f[1]),birthFixed=f[2];
        const setting=Number(f[3]),fixed=f[4],index=Number(f[5]);
        a(inv.settings.includes(setting));a(['OFF','ON'].includes(fixed));
        if(family==='GRID'){a.equal(birth,null);a.equal(birthFixed,'-');}
        else {a(inv.settings.includes(birth));a(['OFF','ON'].includes(birthFixed));}
        const declared=(family==='GRID'?inv.rows:inv.stored)[index-1];a(declared);
        hex(f[6]);a.equal(Buffer.from(f[6],'hex').toString('ascii'),declared.expression);
        const value=typed(f[7]);a.equal(value.bits,f[8]);hex(f[9]);hex(f[10]);a.equal(f[11],'C');
        const key=JSON.stringify([family,birth,birthFixed,setting,fixed,index]);a(!rows.has(key));
        rows.set(key,{family,birth,birthFixed,setting,fixed,index,...value,displayHex:f[9],strHex:f[10]});fields.set(key,f);
    }
    for(const setting of inv.settings)for(const fixed of ['OFF','ON']){
        for(const row of inv.rows){
            const observed=rows.get(JSON.stringify(['GRID',null,'-',setting,fixed,row.index]));a(observed);
            if(row.kind==='rounding'||row.kind==='baseline')continue;
            const base=inv.rows.find(r=>r.kind==='baseline'&&r.seed===row.seed);
            const control=rows.get(JSON.stringify(['GRID',null,'-',setting,fixed,base.index]));a(control);
            a.equal(observed.bits,control.bits);
            if(row.kind==='intervention'){a.equal(observed.width,row.width);a.equal(observed.scale,row.scale);}
            else a.deepEqual({...observed,index:null},{...control,index:null});
        }
        for(const birth of inv.settings)for(const birthFixed of ['OFF','ON'])for(const row of inv.stored)
            a(rows.has(JSON.stringify(['STORED',birth,birthFixed,setting,fixed,row.index])));
    }
    return {raw,rows,fields};
}
const first=read('first'),repeat=read('repeat');let rawDifferentRows=0;
const unusedSlotDifferences={};
for(const [key,p]of first.rows){
    a.deepEqual(p,repeat.rows.get(key));const x=first.fields.get(key),y=repeat.fields.get(key);
    if(x.join('|')===y.join('|'))continue;++rawDifferentRows;
    for(let i=0;i<x.length;++i)if(i!==7)a.equal(x[i],y[i]);
    const m=Object.fromEntries(x[7].split(';').map(s=>s.split('=')));
    const n=Object.fromEntries(y[7].split(';').map(s=>s.split('=')));
    for(const slot of Object.keys(m))if(m[slot]!==n[slot]){
        a.equal(slot,'integer');unusedSlotDifferences[slot]=(unusedSlotDifferences[slot]||0)+1;
    }
}
let storedMetadataChanges=0,storedPayloadChanges=0;
for(const r of first.rows.values())if(r.family==='STORED'){
    const birth=first.rows.get(JSON.stringify(['STORED',r.birth,r.birthFixed,r.birth,r.birthFixed,r.index]));a(birth);
    if(r.width!==birth.width||r.scale!==birth.scale)++storedMetadataChanges;
    if(r.bits!==birth.bits)++storedPayloadChanges;
}
const sha=x=>crypto.createHash('sha256').update(x).digest('hex');
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:inv.recordsPerRun,
    documentedFieldsRepeatEqual:true,rawByteEqual:first.raw.equals(repeat.raw),rawDifferentRows,unusedSlotDifferences,
    cloneControlsEqual:true,interventionPayloadsUnchanged:true,requestedWidthScaleReadbackExact:true,
    storedMetadataChanges,storedPayloadChanges,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),
    limitation:inv.limitation,observations:[...first.rows.values()]},null,2));
