// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Read native observed arithmetic bits, never compute or approve their outputs.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib'),crypto=require('node:crypto'),a=require('node:assert/strict');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-arithmetic-cases.json'))),source=fs.readFileSync(path.join(__dirname,'rounding-arithmetic.prg'),'utf8');
a.equal(inv.inputCount,264);a.equal(inv.recordsPerRun,528);a.equal(inv.inputs.length,264);a.equal(inv.arithmeticFields.length,8);
for(const r of inv.inputs){a(source.includes(`inputHex[${r.index}]='${r.bits}'`));a(source.includes(`targetDecimals[${r.index}]=${r.setting}`));}
const bytes=name=>{const p=path.join(process.argv[2],name);return fs.existsSync(p)?fs.readFileSync(p):zlib.gunzipSync(fs.readFileSync(p+'.gz'));};
const sha=x=>crypto.createHash('sha256').update(x).digest('hex');
function read(name){
 a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);
 const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|528');a.equal(lines.length,528);
 const rows=new Map(),fields=new Map();
 for(const line of lines){const f=line.split('|');a.equal(f.length,22);a(['OFF','ON'].includes(f[0]));
  const r=inv.inputs[Number(f[1])-1];a(r);a.equal(f[2],r.bits);a.equal(f[3],String(r.setting));
  a.match(f[4],/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
  const meta=Object.fromEntries(f[4].split(';').map(s=>s.split('=')));a.equal(meta.real,r.bits);a.equal(f[5],r.bits);
  for(const index of [6,7,10,12])a.match(f[index],/^(?:[0-9A-F]{2})*$/);a.equal(f[8],'C');a.equal(f[9],'N');const actualCount=Buffer.from(f[10],'hex').toString('ascii');a.match(actualCount,/^\s*[0-9]+(?:\.0+)?\s*$/);a.equal(Number(actualCount),r.setting);a.equal(f[11],'C');a.equal(Buffer.from(f[12],'hex').toString('ascii'),f[0]);
  const arithmetic={};for(let i=0;i<8;i++){a.match(f[13+i],/^[0-9A-F]{16}$/);a(Number.isFinite(Buffer.from(f[13+i],'hex').readDoubleBE()));arithmetic[inv.arithmeticFields[i]]=f[13+i];}a.equal(f[21],'NNNNNNNN');
  const key=JSON.stringify([r.index,f[0]]);a(!rows.has(key));rows.set(key,{...r,fixed:f[0],displayHex:f[6],strHex:f[7],appliedDecimalHex:f[10],appliedFixedHex:f[12],arithmetic});fields.set(key,f);
 }
 for(const r of inv.inputs)for(const fixed of ['OFF','ON'])a(rows.has(JSON.stringify([r.index,fixed])));
 return {raw,rows,fields};
}
const first=read('first');
if(process.argv[3]==='--single'){console.log(JSON.stringify({firstComplete:true,records:528,metadataAndBRSIdentityExact:true,appliedSettingsExact:true,firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);}
const repeat=read('repeat');let unusedIntegerDifferences=0;
for(const [key,row]of first.rows){a.deepEqual(row,repeat.rows.get(key));const x=first.fields.get(key),y=repeat.fields.get(key);if(x.join('|')===y.join('|'))continue;for(let i=0;i<x.length;i++)if(i!==4)a.equal(x[i],y[i]);const m=Object.fromEntries(x[4].split(';').map(s=>s.split('='))),n=Object.fromEntries(y[4].split(';').map(s=>s.split('=')));for(const slot of Object.keys(m))if(m[slot]!==n[slot]){a.equal(slot,'integer');++unusedIntegerDifferences;}}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:528,documentedFieldsRepeatEqual:true,metadataAndBRSIdentityExact:true,appliedSettingsExact:true,rawByteEqual:first.raw.equals(repeat.raw),unusedIntegerDifferences,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),limitation:inv.limitation,observations:[...first.rows.values()]},null,2));
