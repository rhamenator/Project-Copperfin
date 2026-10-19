// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Require typed inline REJECTED-graph observations, never predict or accept a formatter.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'arithmetic-inline-cases.json'))),source=fs.readFileSync(path.join(__dirname,'arithmetic-inline.prg'),'utf8');
a.equal(inv.inputCount,264);a.equal(inv.recordsPerRun,528);a.equal(inv.inputs.length,264);a.equal(inv.fieldsPerRecord,66);a.equal(inv.stageNames.length,8);a.equal(inv.stageExpressions.length,8);
a.equal(inv.arithmeticRoute,'Each rejected-graph stage is independently evaluated inline for VARTYPE/CFNMETA/BINTOC before and CFNMETA/BINTOC after the CRT control; only character observations are assigned. No numeric intermediate assignment.');
for(let i=0;i<8;i++){const e=inv.stageExpressions[i];for(const [array,consumer]of [['stageType','VARTYPE'],['stageMeta','CFNMETA']])a(source.includes(`${array}[${i+1}]=${consumer}(${e})`));a(source.includes(`stageBits[${i+1}]=HexBytes(BINTOC(${e},'BRS'))`));}
for(const name of inv.stageNames)a(!new RegExp('\\b'+name+'\\s*=','i').test(source));
a.equal(inv.inputRoute,'Inline CFNDEC(CFNWIDTH(payload,40),18) expression at every consumer, never stored in an argument variable.');
a.equal(inv.resultRoute,'Public ROUND expression assigned to local rounded; metadata/payload/display/STR of that stored result are observed, not assumed equal to inline result metadata.');
a(source.includes('rounded=ROUND(CFNDEC(CFNWIDTH(payload,40),18),decimalCount)'));a(!/\bargument\s*=/i.test(source));
for(const r of inv.inputs){a(source.includes(`inputHex[${r.index}]='${r.bits}'`));a(source.includes(`targetDecimals[${r.index}]=${r.setting}`));}
const bytes=(root,n)=>{const p=path.join(root,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));},root=process.argv[2];a(root);
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),ascii=h=>Buffer.from(h,'hex').toString('ascii');
a.equal(inv.runtimeEnvironment,'WINEDLLOVERRIDES=msvcr71=n');a.equal(bytes(root,'supplied-runtime-env.txt').toString(),inv.runtimeEnvironment+'\n');a(['UNSET\n','\n'].includes(bytes(root,'inherited-dll-overrides.txt').toString()));
const parentRoot=path.join(__dirname,'round-inline-captures1'),parent=JSON.parse(bytes(parentRoot,'audit.json'));
a.equal(parent.recordsPerRun,528);a.equal(parent.typedResultAndControls,true);a.equal(parent.exactRuntimeIdentity,true);a.equal(parent.documentedFieldsRepeatEqual,true);
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],inv.storedParentSha256);a.equal(sha(bytes(parentRoot,n+'-results.txt')),inv.storedParentSha256);}
const controls=new Map(parent.observations.map(r=>[JSON.stringify([r.index,r.fixed]),r]));a.equal(controls.size,528);
const finite=bits=>{a.match(bits,/^[0-9A-F]{16}$/);a.notEqual((BigInt('0x'+bits)>>52n)&2047n,2047n);};
function meta(s,bits,type){
 a.match(s,/^type=(?:4E|49);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
 const m=Object.fromEntries(s.split(';').map(p=>p.split('=')));if(type)a.equal(m.type,type==='N'?'4E':'49');else type=m.type==='49'?'I':'N';finite(bits);
 if(type==='N'){a.equal(m.real,bits);delete m.integer;}else{const b=Buffer.alloc(8);b.writeDoubleBE(parseInt(m.integer,16)|0);a.equal(b.toString('hex').toUpperCase(),bits);delete m.real;delete m.decimals;}
 return m;
}
function binding(s,bits){
 a.match(s,/^kind=F;count=00000011;real=[0-9A-F]{16};decimal=[0-9A-F]{8};sign=[0-9A-F]{8};cw0=027F;cw1=027F;module=(?:[0-9A-F]{2})+;fileSample=ONCE;fileSize=[0-9A-F]{8};fileSha256=[0-9A-F]{64};machine=[0-9A-F]{4};magic=[0-9A-F]{4};stamp=[0-9A-F]{8};imageSize=[0-9A-F]{8};checksum=[0-9A-F]{8};digits=(?:3[0-9])*$/);
 const c=Object.fromEntries(s.split(';').map(p=>p.split('=')));a.equal(c.real,bits);a(c.digits.length<=254);
 a.equal(ascii(c.module).toLowerCase(),inv.modulePath.toLowerCase());a.equal(c.fileSha256.toLowerCase(),inv.moduleSha256);a.equal(parseInt(c.fileSize,16),inv.moduleFileSize);
 for(const [k,v]of Object.entries(inv.modulePe))a.equal(parseInt(c[k],16),v);return c;
}
function read(n){
 a.equal(bytes(root,n+'.status').toString(),'0\n');a.equal(bytes(root,n+'.stderr').length,0);
 const raw=bytes(root,n+'-results.txt');a.deepEqual(raw,bytes(root,n+'.out'));const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
 a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|528');a.equal(lines.length,528);
 const seen=new Set(),observations=[],fields=[];
 for(const line of lines){
  const f=line.split('|');a.equal(f.length,66);a(['OFF','ON'].includes(f[0]));const r=inv.inputs[Number(f[1])-1];a(r);a.equal(f[2],r.bits);a.equal(f[3],String(r.setting));
  const key=JSON.stringify([r.index,f[0]]);a(!seen.has(key));seen.add(key);const old=controls.get(key);a(old);a.equal(old.bits,r.bits);a.equal(old.setting,r.setting);
  for(const k of [5,19])a.equal(f[k],r.bits);
  for(const k of [4,18])a.match(f[k],/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
  a.deepEqual(meta(f[4],r.bits,'N'),meta(f[18],r.bits,'N'));
  for(const k of [6,7,9,11,16,17,20,24,25])a.match(f[k],/^(?:[0-9A-F]{2})*$/);
  a.equal(f[6],old.originalDisplayHex);a.equal(f[7],old.originalStrHex);a.equal(f[20],f[6]);
  a.equal(f[8],'N');a.match(ascii(f[9]),/^\s*[0-9]+(?:\.0+)?\s*$/);a.equal(Number(ascii(f[9])),r.setting);a.equal(f[10],'C');a.equal(ascii(f[11]),f[0]);
  const control=binding(f[12],r.bits);a.deepEqual(control,binding(f[21],r.bits));
  // Public VARTYPE returns N for both Numeric and Integer carriers; use the
  // published FLL Value tag, not VARTYPE, to select meaningful raw slots.
  a.equal(f[13],'N');const resultMeta=meta(f[14],f[15]);a.deepEqual(resultMeta,meta(f[22],f[23]));
  a.equal(f[15],f[23]);a.equal(f[16],f[24]);a.equal(f[17],f[25]);
  a.deepEqual(control,old.crt);a.deepEqual(resultMeta,old.resultMeta);a.equal(f[15],old.resultBits);a.equal(f[16],old.resultDisplayHex);a.equal(f[17],old.resultStrHex);
  const arithmetic={};for(let j=0;j<8;j++){const k=26+5*j;a.equal(f[k],'N');const m=meta(f[k+1],f[k+2]);a.deepEqual(m,meta(f[k+3],f[k+4]));a.equal(f[k+2],f[k+4]);arithmetic[inv.stageNames[j]]={varType:f[k],type:m.type==='49'?'I':'N',meta:m,bits:f[k+2]};}
  observations.push({...r,fixed:f[0],originalDisplayHex:f[6],originalStrHex:f[7],resultVarType:f[13],resultType:resultMeta.type==='49'?'I':'N',resultMeta,resultBits:f[15],resultDisplayHex:f[16],resultStrHex:f[17],arithmetic,crt:control});fields.push(f);
 }
 for(const r of inv.inputs)for(const fixed of ['OFF','ON'])a(seen.has(JSON.stringify([r.index,fixed])));
 return {raw,observations,fields};
}
const first=read('first');
if(process.argv[3]==='--single'){console.log(JSON.stringify({firstComplete:true,records:528,typedResultAndControls:true,inlineArithmeticAndStoredParentControls:true,exactRuntimeIdentity:true,firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);}
const repeat=read('repeat');a.deepEqual(first.observations,repeat.observations);let inactiveSlotDifferences=0;
for(let i=0;i<first.fields.length;i++)for(let k=0;k<66;k++){
 const x=first.fields[i][k],y=repeat.fields[i][k];if(x===y)continue;
 let type,bits;if(k<26){a([4,14,18,22].includes(k));const original=[4,18].includes(k);type=original?'N':first.observations[i].resultType;bits=original?first.fields[i][2]:first.fields[i][15];}
 else{const j=Math.floor((k-26)/5),offset=(k-26)%5;a([1,3].includes(offset));const stage=first.observations[i].arithmetic[inv.stageNames[j]];type=stage.type;bits=stage.bits;}
 a.deepEqual(meta(x,bits,type),meta(y,bits,type));++inactiveSlotDifferences;
}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:528,documentedFieldsRepeatEqual:true,typedResultAndControls:true,inlineArithmeticAndStoredParentControls:true,exactRuntimeIdentity:true,rawByteEqual:first.raw.equals(repeat.raw),inactiveSlotDifferences,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),limitation:inv.limitation,observations:first.observations},null,2));
