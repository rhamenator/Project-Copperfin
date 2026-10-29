// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Strict actual native observation reader, not a rounding/formatting model.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-cases.json')));
const source=fs.readFileSync(path.join(__dirname,'rounding.prg'),'utf8');
a.equal(inv.centers.length,8);a.equal(inv.inputCount,1000);a.equal(inv.recordsPerRun,2000);
const inputs=[];
for(const center of inv.centers)for(let offset=center.low;offset<=center.high;++offset){
    const bits=(BigInt('0x'+center.bits)+BigInt(offset)).toString(16).padStart(16,'0').toUpperCase();
    const input={index:inputs.length+1,center:center.id,offset,bits,setting:center.setting};
    a(source.includes(`inputHex[${input.index}]='${bits}'`));inputs.push(input);
}
a.equal(inputs.length,1000);
const bytes=name=>{const p=path.join(process.argv[2],name);return fs.existsSync(p)?fs.readFileSync(p):zlib.gunzipSync(fs.readFileSync(p+'.gz'));};
const sha=x=>crypto.createHash('sha256').update(x).digest('hex');
function read(name){
    a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);
    const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));
    const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
    a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    a.equal(lines.pop(),'DONE|2000');a.equal(lines.length,2000);
    const rows=new Map(),fields=new Map();
    for(const line of lines){
        const f=line.split('|');a.equal(f.length,9);a(['OFF','ON'].includes(f[0]));
        const input=inputs[Number(f[1])-1];a(input);a.equal(f[2],input.bits);a.equal(Number(f[3]),input.setting);
        a.match(f[4],/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
        const meta=Object.fromEntries(f[4].split(';').map(s=>s.split('=')));
        a.equal(meta.real,input.bits);a.equal(f[5],input.bits);
        a.match(f[6],/^(?:[0-9A-F]{2})*$/);a.match(f[7],/^(?:[0-9A-F]{2})*$/);a.equal(f[8],'C');
        const key=JSON.stringify([input.index,f[0]]);a(!rows.has(key));
        rows.set(key,{...input,fixed:f[0],width:40,scale:18,displayHex:f[6],strHex:f[7]});fields.set(key,f);
    }
    for(const input of inputs)for(const fixed of ['OFF','ON'])a(rows.has(JSON.stringify([input.index,fixed])));
    return {raw,rows,fields};
}
const first=read('first');
if(process.argv[3]==='--single'){
    console.log(JSON.stringify({firstComplete:true,records:2000,metadataAndBRSIdentityExact:true,
        firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);
}
const repeat=read('repeat');let rawDifferentRows=0,unusedIntegerDifferences=0;
for(const [key,p]of first.rows){
    a.deepEqual(p,repeat.rows.get(key));const x=first.fields.get(key),y=repeat.fields.get(key);
    if(x.join('|')===y.join('|'))continue;++rawDifferentRows;
    for(let i=0;i<x.length;++i)if(i!==4)a.equal(x[i],y[i]);
    const m=Object.fromEntries(x[4].split(';').map(s=>s.split('=')));
    const n=Object.fromEntries(y[4].split(';').map(s=>s.split('=')));
    for(const slot of Object.keys(m))if(m[slot]!==n[slot]){a.equal(slot,'integer');++unusedIntegerDifferences;}
}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:2000,documentedFieldsRepeatEqual:true,
    rawByteEqual:first.raw.equals(repeat.raw),rawDifferentRows,unusedIntegerDifferences,
    metadataAndBRSIdentityExact:true,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),
    limitation:inv.limitation,observations:[...first.rows.values()]},null,2));
