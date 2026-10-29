// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native observation reader; input-bit checks are stimulus, not display models.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-ladder-cases.json')));
const source=fs.readFileSync(path.join(__dirname,'rounding-ladder.prg'),'utf8');
a.equal(inv.centers.length,8);a.equal(inv.groups.length,16);a.equal(inv.offsets.length,121);
a.equal(inv.inputCount,1936);a.equal(inv.recordsPerRun,3872);
const inputs=[];
for(const group of inv.groups){
    const center=inv.centers.find(c=>c.id===group.center);a(center);a.equal(group.first,inputs.length+1);
    for(const offset of inv.offsets){
        const magnitude=BigInt('0x'+center.bits)+BigInt(offset);
        const bits=(magnitude|(group.negative?0x8000000000000000n:0n)).toString(16).padStart(16,'0').toUpperCase();
        const input={index:inputs.length+1,center:center.id,negative:group.negative,offset,bits,setting:center.setting};
        a(source.includes(`inputHex[${input.index}]='${bits}'`));inputs.push(input);
    }
    a.equal(group.last,inputs.length);
}
a.equal(inputs.length,1936);
const bytes=name=>{const p=path.join(process.argv[2],name);return fs.existsSync(p)?fs.readFileSync(p):zlib.gunzipSync(fs.readFileSync(p+'.gz'));};
const sha=x=>crypto.createHash('sha256').update(x).digest('hex');
function read(name){
    a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);
    const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));
    const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
    a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    a.equal(lines.pop(),'DONE|3872');a.equal(lines.length,3872);
    const rows=new Map(),fields=new Map();
    for(const line of lines){
        const f=line.split('|');a.equal(f.length,9);a(['OFF','ON'].includes(f[0]));
        const input=inputs[Number(f[1])-1];a(input);a.equal(f[2],input.bits);a.equal(Number(f[3]),input.setting);
        a.match(f[4],/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
        const meta=Object.fromEntries(f[4].split(';').map(s=>s.split('=')));a.equal(meta.real,input.bits);a.equal(f[5],input.bits);
        a.match(f[6],/^(?:[0-9A-F]{2})*$/);a.match(f[7],/^(?:[0-9A-F]{2})*$/);a.equal(f[8],'C');
        const key=JSON.stringify([input.index,f[0]]);a(!rows.has(key));
        rows.set(key,{...input,fixed:f[0],width:40,scale:18,displayHex:f[6],strHex:f[7]});fields.set(key,f);
    }
    for(const input of inputs)for(const fixed of ['OFF','ON'])a(rows.has(JSON.stringify([input.index,fixed])));
    return {raw,rows,fields};
}
const first=read('first');
if(process.argv[3]==='--single'){
    console.log(JSON.stringify({firstComplete:true,records:3872,metadataAndBRSIdentityExact:true,
        firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);
}
const repeat=read('repeat');let rawDifferentRows=0,unusedIntegerDifferences=0;
for(const [key,p]of first.rows){
    a.deepEqual(p,repeat.rows.get(key));const x=first.fields.get(key),y=repeat.fields.get(key);
    if(x.join('|')===y.join('|'))continue;++rawDifferentRows;
    for(let i=0;i<x.length;++i)if(i!==4)a.equal(x[i],y[i]);
    const m=Object.fromEntries(x[4].split(';').map(s=>s.split('='))),n=Object.fromEntries(y[4].split(';').map(s=>s.split('=')));
    for(const slot of Object.keys(m))if(m[slot]!==n[slot]){a.equal(slot,'integer');++unusedIntegerDifferences;}
}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:3872,documentedFieldsRepeatEqual:true,
    rawByteEqual:first.raw.equals(repeat.raw),rawDifferentRows,unusedIntegerDifferences,
    metadataAndBRSIdentityExact:true,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),
    limitation:inv.limitation,observations:[...first.rows.values()]},null,2));
