// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent actual-observation reader; no product/model/expected imports.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'cases.json')));
const source=fs.readFileSync(path.join(__dirname,'probe.prg'),'utf8');
a.equal(inv.rows.length,82);a.equal(inv.literals.length,58);
a.equal(inv.recordsPerRun,3936);a.deepEqual(inv.settings,[0,1,2,6,14,18]);
a.deepEqual(inv.routes,['EVAL','COMPILED','STORE-EVAL','STORE-COMPILED']);
for(const r of inv.rows){
    a(source.includes(`expressions[${r.index}]='${r.expression}'`));
    a(source.includes(`   stored=${r.expression}\n`));
}
function bytes(name){const p=path.join(process.argv[2],name);return fs.existsSync(p)?fs.readFileSync(p):zlib.gunzipSync(fs.readFileSync(p+'.gz'));}
function typed(meta){
    a.match(meta,/^type=(?:49|4E);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
    const raw=Object.fromEntries(meta.split(';').map(p=>p.split('=')));
    const r={type:raw.type==='49'?'I':'N',width:parseInt(raw.width,16),cw:raw.cw};
    if(r.type==='I'){
        r.integer=parseInt(raw.integer,16)|0;const b=Buffer.alloc(8);b.writeDoubleBE(r.integer);
        r.bits=b.toString('hex').toUpperCase();
    }else{r.decimals=parseInt(raw.decimals,16);r.bits=raw.real;}
    a(Number.isFinite(Buffer.from(r.bits,'hex').readDoubleBE()));
    return r;
}
function capture(name){
    a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);
    const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));
    const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
    a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    a.equal(lines.pop(),`DONE|${inv.recordsPerRun}`);a.equal(lines.length,inv.recordsPerRun);
    const rows=new Map(),fields=new Map();
    for(const line of lines){
        const f=line.split('|');a.equal(f.length,9);a(inv.routes.includes(f[0]));
        const setting=Number(f[1]),fixed=f[2],index=Number(f[3]);
        a(inv.settings.includes(setting));a(['OFF','ON'].includes(fixed));
        a.equal(Buffer.from(f[4],'hex').toString('ascii'),inv.rows[index-1].expression);
        const value=typed(f[5]);a.equal(value.bits,f[6]);a.match(f[7],/^(?:[0-9A-F]{2})*$/);a.equal(f[8],'C');
        const key=JSON.stringify([setting,fixed,index,f[0]]);a(!rows.has(key));
        fields.set(key,f);
        rows.set(key,{setting,fixed,index,route:f[0],...value,displayHex:f[7]});
    }
    // Routes are separately retained: equality of direct and stored descriptors
    // is NOT an audit premise. Compare only the equivalent evaluation routes.
    for(const setting of inv.settings)for(const fixed of ['OFF','ON'])for(const r of inv.rows)
        for(const [x,y]of [['EVAL','COMPILED'],['STORE-EVAL','STORE-COMPILED']]){
            const p=rows.get(JSON.stringify([setting,fixed,r.index,x]));
            const q=rows.get(JSON.stringify([setting,fixed,r.index,y]));a(p&&q);
            a.deepEqual({...p,route:null},{...q,route:null});
        }
    return {raw,rows,fields};
}
const first=capture('first'),repeat=capture('repeat');
let rawDifferentRows=0;
const unusedSlotDifferences={};
for(const [key,row]of first.rows){
    a.deepEqual(row,repeat.rows.get(key));
    const p=first.fields.get(key),q=repeat.fields.get(key);
    if(p.join('|')===q.join('|'))continue;
    ++rawDifferentRows;
    for(let i=0;i<p.length;++i)if(i!==5)a.equal(p[i],q[i]);
    const x=Object.fromEntries(p[5].split(';').map(s=>s.split('=')));
    const y=Object.fromEntries(q[5].split(';').map(s=>s.split('=')));
    for(const slot of Object.keys(x))if(x[slot]!==y[slot]){
        a(x.type==='49'?['decimals','real'].includes(slot):slot==='integer');
        const label=x.type+':'+slot;unusedSlotDifferences[label]=(unusedSlotDifferences[label]||0)+1;
    }
}
const sha=x=>crypto.createHash('sha256').update(x).digest('hex');
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:inv.recordsPerRun,
    expressions:inv.rows.length,documentedFieldsRepeatEqual:true,rawByteEqual:first.raw.equals(repeat.raw),
    rawDifferentRows,unusedSlotDifferences,
    firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),
    limitation:inv.limitation,observations:[...first.rows.values()]},null,2));
