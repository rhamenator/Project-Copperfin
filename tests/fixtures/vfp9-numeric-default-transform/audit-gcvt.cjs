// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: independent strict observation reader.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'gcvt-cases.json'))),source=fs.readFileSync(path.join(__dirname,'gcvt.prg'),'utf8'),root=process.argv[2];a(root);
a.equal(inv.inputCount,264);a.equal(inv.recordsPerRun,1584);a.equal(inv.fieldsPerRecord,20);a.deepEqual(inv.counts,[15,16,17]);a.equal(inv.inputs.length,264);
a.equal(inv.inputRoute,'Inline CFNDEC(CFNWIDTH(payload,40),18) at every numeric consumer; no stored numeric argument.');
a(!/\bargument\s*=/i.test(source));a(source.includes('crtResult=CFNGCVT(CFNDEC(CFNWIDTH(payload,40),18),conversionCount)'));a(source.includes('FOR conversionCount=15 TO 17'));
for(const [i,r]of inv.inputs.entries()){a.equal(r.index,i+1);a(source.includes(`inputHex[${r.index}]='${r.bits}'`));a(source.includes(`targetDecimals[${r.index}]=${r.setting}`));}
const bytes=(dir,n)=>{const p=path.join(dir,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
const sha=b=>c.createHash('sha256').update(b).digest('hex');
function ascii(h){a.match(h,/^(?:[0-9A-F]{2})*$/);const b=Buffer.from(h,'hex');a(b.every(x=>x>0&&x<128));return b.toString('ascii');}
a.equal(inv.runtimeEnvironment,'WINEDLLOVERRIDES=msvcr71=n');a.equal(bytes(root,'supplied-runtime-env.txt').toString(),inv.runtimeEnvironment+'\n');a(['UNSET\n','\n'].includes(bytes(root,'inherited-dll-overrides.txt').toString()));
const parentRoot=path.join(__dirname,'arithmetic-inline-captures1'),parent=JSON.parse(bytes(parentRoot,'audit.json'));
a.equal(parent.recordsPerRun,528);for(const k of ['documentedFieldsRepeatEqual','typedResultAndControls','inlineArithmeticAndStoredParentControls','exactRuntimeIdentity'])a.equal(parent[k],true);
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],inv.parentNativeSha256);a.equal(sha(bytes(parentRoot,n+'-results.txt')),inv.parentNativeSha256);}
const controls=new Map(parent.observations.map(r=>[JSON.stringify([r.index,r.fixed]),r]));a.equal(controls.size,528);
function meta(s,bits){
 a.match(s,/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
 const m=Object.fromEntries(s.split(';').map(p=>p.split('=')));a.equal(m.real,bits);a.notEqual((BigInt('0x'+bits)>>52n)&2047n,2047n);delete m.integer;return m;
}
function identity(b,bits){
 a.equal(b.real,bits);a.equal(b.cw0,'027F');a.equal(b.cw1,'027F');a.equal(b.fileSample,'ONCE');
 a.equal(ascii(b.module).toLowerCase(),inv.modulePath.toLowerCase());a.equal(b.fileSha256.toLowerCase(),inv.moduleSha256);a.equal(parseInt(b.fileSize,16),inv.moduleFileSize);
 for(const [k,v]of Object.entries(inv.modulePe))a.equal(parseInt(b[k],16),v);
}
function control(s,bits){
 a.match(s,/^kind=F;count=00000011;real=[0-9A-F]{16};decimal=[0-9A-F]{8};sign=[0-9A-F]{8};cw0=027F;cw1=027F;module=(?:[0-9A-F]{2})+;fileSample=ONCE;fileSize=[0-9A-F]{8};fileSha256=[0-9A-F]{64};machine=[0-9A-F]{4};magic=[0-9A-F]{4};stamp=[0-9A-F]{8};imageSize=[0-9A-F]{8};checksum=[0-9A-F]{8};digits=(?:3[0-9])*$/);
 const b=Object.fromEntries(s.split(';').map(p=>p.split('=')));identity(b,bits);a(b.digits.length<=254);return b;
}
function gcvt(s,bits,count){
 a.match(s,/^kind=G;count=[0-9A-F]{8};real=[0-9A-F]{16};cw0=027F;cw1=027F;module=(?:[0-9A-F]{2})+;fileSample=ONCE;fileSize=[0-9A-F]{8};fileSha256=[0-9A-F]{64};machine=[0-9A-F]{4};magic=[0-9A-F]{4};stamp=[0-9A-F]{8};imageSize=[0-9A-F]{8};checksum=[0-9A-F]{8};capacity=00000800;canary=PASS;pointer=OWN;text=(?:[0-9A-F]{2})+$/);
 const b=Object.fromEntries(s.split(';').map(p=>p.split('=')));identity(b,bits);a.equal(parseInt(b.count,16),count);a(b.text.length<=254);
 a.match(ascii(b.text),/^[+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?$/);return b;
}
function read(n){
 a.equal(bytes(root,n+'.status').toString(),'0\n');a.equal(bytes(root,n+'.stderr').length,0);
 const raw=bytes(root,n+'-results.txt');a.deepEqual(raw,bytes(root,n+'.out'));a(raw.every(x=>x<128));const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
 a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|1584');a.equal(lines.length,1584);
 const observations=[],fields=[];let ordinal=0;
 for(const r of inv.inputs)for(const fixed of ['OFF','ON'])for(const count of [15,16,17]){
  const f=lines[ordinal++].split('|');a.equal(f.length,20);a.equal(f[0],fixed);a.equal(f[1],String(r.index));a.equal(f[2],r.bits);a.equal(f[3],String(r.setting));
  const old=controls.get(JSON.stringify([r.index,fixed]));a(old);a.equal(old.bits,r.bits);a.equal(old.setting,r.setting);
  a.deepEqual(meta(f[4],r.bits),meta(f[15],r.bits));a.equal(f[5],r.bits);a.equal(f[16],r.bits);
  for(const k of [6,7,9,11,17,18])ascii(f[k]);a.equal(f[6],old.originalDisplayHex);a.equal(f[7],old.originalStrHex);a.equal(f[17],f[6]);a.equal(f[18],f[7]);
  a.equal(f[8],'N');a.match(ascii(f[9]),/^\s*[0-9]+(?:\.0+)?\s*$/);a.equal(Number(ascii(f[9])),r.setting);a.equal(f[10],'C');a.equal(ascii(f[11]),fixed);
  const crt=control(f[12],r.bits);a.deepEqual(crt,old.crt);a.deepEqual(control(f[19],r.bits),crt);a.equal(f[13],String(count));const g=gcvt(f[14],r.bits,count);
  observations.push({...r,fixed,count,originalMeta:meta(f[4],r.bits),originalDisplayHex:f[6],originalStrHex:f[7],appliedDecimalHex:f[9],appliedFixedHex:f[11],crt,gcvt:g});fields.push(f);
 }
 return {raw,observations,fields};
}
const first=read('first');
if(process.argv[3]==='--single'){console.log(JSON.stringify({firstComplete:true,records:1584,originalParentAndTypedControls:true,exactRuntimeIdentity:true,guardedBufferAndGrammar:true,firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);}
a.equal(bytes(root,'first-structure.status').toString(),'0\n');a.equal(bytes(root,'first-structure.stderr').length,0);
const single=JSON.parse(bytes(root,'first-structure.json'));a.equal(single.firstSha256,sha(first.raw));a.equal(single.firstComplete,true);a.equal(single.pairedQualified,false);
const repeat=read('repeat');a.deepEqual(first.observations,repeat.observations);let inactiveSlotDifferences=0;
for(let i=0;i<1584;i++)for(let k=0;k<20;k++){
 const x=first.fields[i][k],y=repeat.fields[i][k];if(x===y)continue;a([4,15].includes(k));a.deepEqual(meta(x,first.fields[i][2]),meta(y,repeat.fields[i][2]));++inactiveSlotDifferences;
}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:1584,documentedFieldsRepeatEqual:true,originalParentAndTypedControls:true,exactRuntimeIdentity:true,guardedBufferAndGrammar:true,rawByteEqual:first.raw.equals(repeat.raw),inactiveSlotDifferences,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),limitation:inv.limitation,observations:first.observations},null,2));
