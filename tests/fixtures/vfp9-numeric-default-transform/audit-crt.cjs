// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Read public native CRT observations; no candidate/expected digits are computed.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'crt-cases.json'))),
    source=fs.readFileSync(path.join(__dirname,'crt.prg'),'utf8');
a.equal(inv.inputCount,264);a.equal(inv.recordsPerRun,4176);a.equal(inv.inputs.length,264);
for(const r of inv.inputs){a(source.includes(`inputHex[${r.index}]='${r.bits}'`));a(source.includes(`shiftedHex[${r.index}]='${r.shiftedBits}'`));a(source.includes(`targetDecimals[${r.index}]=${r.setting}`));}
const bytes=name=>{const p=path.join(process.argv[2],name);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),ascii=h=>Buffer.from(h,'hex').toString('ascii');
a.equal(inv.runtimeEnvironment,'WINEDLLOVERRIDES=msvcr71=n');a.equal(inv.inputsAreExplicitPayloadStimuli,true);
a.equal(bytes('supplied-runtime-env.txt').toString(),inv.runtimeEnvironment+'\n');
a(['UNSET\n','\n'].includes(bytes('inherited-dll-overrides.txt').toString()));
function read(name){
 a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);
 const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
 a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|4176');a.equal(lines.length,4176);
 const observations=[],fields=[],seen=new Set();
 for(const line of lines){
  const f=line.split('|');a.equal(f.length,20);
  const r=inv.inputs[Number(f[1])-1];a(r);a(['OFF','ON'].includes(f[0]));a.equal(f[2],r.bits);a.equal(f[3],String(r.setting));
  a(['ORIGINAL','SHIFTED'].includes(f[4]));a.equal(f[5],f[4]==='ORIGINAL'?r.bits:r.shiftedBits);
  a(['F','E'].includes(f[6]));const count=Number(f[7]);
  if(f[6]==='F'){a(r.setting>0);a.equal(count,r.setting);}else a([15,16,17].includes(count));
  for(const m of [8,17]){
   a.match(f[m],/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
   a.equal(Object.fromEntries(f[m].split(';').map(s=>s.split('='))).real,f[5]);
  }
  a.equal(f[9],f[5]);a.equal(f[18],f[5]);a.equal(f[19],f[10]);
  for(const k of [10,11,13,15,19])a.match(f[k],/^(?:[0-9A-F]{2})*$/);
  a.equal(f[12],'N');const applied=ascii(f[13]);a.match(applied,/^\s*[0-9]+(?:\.0+)?\s*$/);a.equal(Number(applied),r.setting);
  a.equal(f[14],'C');a.equal(ascii(f[15]),f[0]);
  a.match(f[16],/^kind=[FE];count=[0-9A-F]{8};real=[0-9A-F]{16};decimal=[0-9A-F]{8};sign=[0-9A-F]{8};cw0=027F;cw1=027F;module=(?:[0-9A-F]{2})+;fileSample=ONCE;fileSize=[0-9A-F]{8};fileSha256=[0-9A-F]{64};machine=[0-9A-F]{4};magic=[0-9A-F]{4};stamp=[0-9A-F]{8};imageSize=[0-9A-F]{8};checksum=[0-9A-F]{8};digits=(?:3[0-9])*$/);
  const c=Object.fromEntries(f[16].split(';').map(s=>s.split('=')));
  a.equal(c.kind,f[6]);a.equal(parseInt(c.count,16),count);a.equal(c.real,f[5]);
  a.equal(ascii(c.module).toLowerCase(),inv.modulePath.toLowerCase());a(c.digits.length<=254);
  a.equal(c.fileSha256.toLowerCase(),inv.moduleSha256);a.equal(parseInt(c.fileSize,16),inv.moduleFileSize);
  for(const [key,value] of Object.entries(inv.modulePe))a.equal(parseInt(c[key],16),value);
  const key=JSON.stringify([r.index,f[0],f[4],f[6],count]);a(!seen.has(key));seen.add(key);
  observations.push({...r,fixed:f[0],route:f[4],argumentBits:f[5],kind:f[6],count,displayHex:f[10],strHex:f[11],appliedDecimalHex:f[13],appliedFixedHex:f[15],crt:c});
  fields.push(f);
 }
 for(const r of inv.inputs)for(const fixed of ['OFF','ON'])for(const route of ['ORIGINAL','SHIFTED']){
  if(r.setting>0)a(seen.has(JSON.stringify([r.index,fixed,route,'F',r.setting])));
  for(const count of [15,16,17])a(seen.has(JSON.stringify([r.index,fixed,route,'E',count])));
 }
 return {raw,observations,fields};
}
const first=read('first');
if(process.argv[3]==='--single'){console.log(JSON.stringify({firstComplete:true,records:4176,metadataAndBRSExact:true,settingsAndRuntimeModuleExact:true,firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);}
const repeat=read('repeat');a.deepEqual(first.observations,repeat.observations);let inactiveIntegerDifferences=0;
for(let i=0;i<first.fields.length;i++)for(let k=0;k<20;k++){
 const x=first.fields[i][k],y=repeat.fields[i][k];if(x===y)continue;
 a([8,17].includes(k));a.equal(x.replace(/integer=[0-9A-F]{8}/,'integer=UNUSED'),y.replace(/integer=[0-9A-F]{8}/,'integer=UNUSED'));++inactiveIntegerDifferences;
}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:4176,documentedFieldsRepeatEqual:true,metadataAndBRSExact:true,settingsAndRuntimeModuleExact:true,
 rawByteEqual:first.raw.equals(repeat.raw),inactiveIntegerDifferences,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),limitation:inv.limitation,observations:first.observations},null,2));
