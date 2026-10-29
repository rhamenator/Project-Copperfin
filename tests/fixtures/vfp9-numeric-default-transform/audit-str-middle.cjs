// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: strict independent STR16/17 reader with accepted15/18 controls.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const root=process.argv[2];a(root);const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'str-middle-cases.json'))),source=fs.readFileSync(path.join(__dirname,'str-middle.prg'),'utf8');
a.equal(inv.inputCount,264);a.equal(inv.recordsPerRun,528);a.equal(inv.fieldsPerRecord,32);a.deepEqual(inv.counts,[16,17]);a.deepEqual(inv.parentStrCounts,[15,18]);a.equal(inv.controlCount,17);a.equal(inv.inputs.length,264);
a.equal(inv.inputRoute,'Inline CFNDEC(CFNWIDTH(payload,40),18) at every numeric consumer; no stored numeric argument.');a(!/\bargument\s*=/i.test(source));
for(const count of [15,16,17,18])for(const [name,consumer]of [['Type','VARTYPE'],['Hex','HexBytes']])a(source.includes(`str${count}${name}=${consumer}(STR(CFNDEC(CFNWIDTH(payload,40),18),40,${count}))`));
for(const count of [15,18])for(const [name,consumer]of [['Type','VARTYPE'],['Hex','HexBytes']])a(source.includes(`str${count}After${name}=${consumer}(STR(CFNDEC(CFNWIDTH(payload,40),18),40,${count}))`));
for(const [i,r]of inv.inputs.entries()){a.equal(r.index,i+1);a(source.includes(`inputHex[${r.index}]='${r.bits}'`));a(source.includes(`targetDecimals[${r.index}]=${r.setting}`));}
const bytes=(d,n)=>{const p=path.join(d,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));},sha=b=>c.createHash('sha256').update(b).digest('hex');
const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})*$/);const b=Buffer.from(h,'hex');a(b.every(x=>x>0&&x<128));return b.toString('ascii');};
a.equal(inv.runtimeEnvironment,'WINEDLLOVERRIDES=msvcr71=n');if(root!=='--source-only'){a.equal(bytes(root,'supplied-runtime-env.txt').toString(),inv.runtimeEnvironment+'\n');a(['UNSET\n','\n'].includes(bytes(root,'inherited-dll-overrides.txt').toString()));}
const parentRoot=path.join(__dirname,'str-fixed-captures1'),parent=JSON.parse(bytes(parentRoot,'audit.json'));a.equal(parent.recordsPerRun,528);for(const k of ['documentedFieldsRepeatEqual','originalAndNativeParentControls','inlineMetadataAndBRSExact','publicStrTypesAndWidths'])a.equal(parent[k],true);
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],inv.parentNativeSha256);a.equal(sha(bytes(parentRoot,n+'-results.txt')),inv.parentNativeSha256);}
const parents=new Map(parent.observations.map(r=>[JSON.stringify([r.index,r.fixed]),r]));a.equal(parents.size,528);
// The qualified parent's JSON projection omitted applied-setting hex fields.
// Recover these ACTUAL bytes from its hash-bound raw24-field records, never
// manufacture formatting expectations or silently compare against undefined.
const parentLines=bytes(parentRoot,'first-results.txt').toString('ascii').split('\r\n');a.equal(parentLines.pop(),'');a.equal(parentLines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(parentLines.pop(),'DONE|528');a.equal(parentLines.length,528);
const parentSettings=new Map();for(const line of parentLines){const f=line.split('|');a.equal(f.length,24);const k=JSON.stringify([Number(f[1]),f[0]]),r=parents.get(k);a(r);a(!parentSettings.has(k));a.equal(f[2],r.bits);a.equal(f[3],String(r.setting));a.equal(f[8],'N');a.equal(f[10],'C');a.equal(Number(ascii(f[9])),r.setting);a.equal(ascii(f[11]),r.fixed);parentSettings.set(k,{decimalHex:f[9],fixedHex:f[11]});}a.equal(parentSettings.size,528);
if(root==='--source-only'){for(const r of inv.inputs)for(const fixed of ['OFF','ON']){const k=JSON.stringify([r.index,fixed]);a(parents.has(k)&&parentSettings.has(k));const p=parents.get(k);for(const n of ['str15Type','str15Hex','str18Type','str18Hex'])a.equal(typeof p[n],'string');a(p.crt&&p.gcvt&&p.originalMeta);}console.log(JSON.stringify({sourceOnly:true,fields:32,verifiedParentRecords:528,parentRawSettingsRecovered:true,nativeInvoked:false,newResultsRead:false}));process.exit(0);}
function meta(s,bits){a.match(s,/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);const m=Object.fromEntries(s.split(';').map(p=>p.split('=')));a.equal(m.real,bits);a.notEqual((BigInt('0x'+bits)>>52n)&2047n,2047n);delete m.integer;return m;}
function parentControl(s,old){a.equal(s,Object.entries(old).map(([k,v])=>k+'='+v).join(';'));return Object.fromEntries(s.split(';').map(p=>p.split('=')));}
function read(n){
 a.equal(bytes(root,n+'.status').toString(),'0\n');a.equal(bytes(root,n+'.stderr').length,0);const raw=bytes(root,n+'-results.txt');a.deepEqual(raw,bytes(root,n+'.out'));a(raw.every(x=>x<128));const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|528');a.equal(lines.length,528);
 let ordinal=0;const observations=[],fields=[];
 for(const r of inv.inputs)for(const fixed of ['OFF','ON']){const f=lines[ordinal++].split('|');a.equal(f.length,32);a.equal(f[0],fixed);a.equal(f[1],String(r.index));a.equal(f[2],r.bits);a.equal(f[3],String(r.setting));const old=parents.get(JSON.stringify([r.index,fixed]));a(old);a.equal(old.bits,r.bits);a.equal(old.setting,r.setting);
  const m=meta(f[4],r.bits);a.deepEqual(m,old.originalMeta);a.deepEqual(meta(f[18],r.bits),m);a.equal(f[5],r.bits);a.equal(f[19],r.bits);a.equal(f[6],old.originalDisplayHex);a.equal(f[7],old.originalStrHex);a.equal(f[20],f[6]);a.equal(f[21],f[7]);
  const settings=parentSettings.get(JSON.stringify([r.index,fixed]));a(settings);a.equal(f[8],'N');a.equal(f[9],settings.decimalHex);a.match(ascii(f[9]),/^\s*[0-9]+(?:\.0+)?\s*$/);a.equal(Number(ascii(f[9])),r.setting);a.equal(f[10],'C');a.equal(f[11],settings.fixedHex);a.equal(ascii(f[11]),fixed);
  const crt=parentControl(f[12],old.crt),gcvt=parentControl(f[13],old.gcvt);parentControl(f[22],old.crt);parentControl(f[23],old.gcvt);
  for(const [i,k]of [[14,'str15Type'],[15,'str15Hex'],[16,'str18Type'],[17,'str18Hex'],[28,'str15Type'],[29,'str15Hex'],[30,'str18Type'],[31,'str18Hex']])a.equal(f[i],old[k]);
  for(const [type,text]of [[14,15],[16,17],[24,25],[26,27],[28,29],[30,31]]){a.equal(f[type],'C');a.equal(ascii(f[text]).length,40);}
  observations.push({...r,fixed,originalMeta:m,originalDisplayHex:f[6],originalStrHex:f[7],str15Type:f[14],str15Hex:f[15],str18Type:f[16],str18Hex:f[17],str16Type:f[24],str16Hex:f[25],str17Type:f[26],str17Hex:f[27],crt,gcvt});fields.push(f);
 }
 return {raw,observations,fields};
}
const first=read('first');if(process.argv[3]==='--single'){console.log(JSON.stringify({firstComplete:true,records:528,originalAndNativeParentControls:true,parentStrBeforeAfterExact:true,inlineMetadataAndBRSExact:true,publicStrTypesAndWidths:true,firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);}
a.equal(bytes(root,'first-structure.status').toString(),'0\n');a.equal(bytes(root,'first-structure.stderr').length,0);const single=JSON.parse(bytes(root,'first-structure.json'));a.equal(single.firstSha256,sha(first.raw));a.equal(single.firstComplete,true);a.equal(single.pairedQualified,false);
const repeat=read('repeat');a.deepEqual(first.observations,repeat.observations);let inactiveSlotDifferences=0;for(let i=0;i<528;i++)for(let k=0;k<32;k++){const x=first.fields[i][k],y=repeat.fields[i][k];if(x===y)continue;a([4,18].includes(k));a.deepEqual(meta(x,first.fields[i][2]),meta(y,repeat.fields[i][2]));++inactiveSlotDifferences;}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:528,documentedFieldsRepeatEqual:true,originalAndNativeParentControls:true,parentStrBeforeAfterExact:true,inlineMetadataAndBRSExact:true,publicStrTypesAndWidths:true,rawByteEqual:first.raw.equals(repeat.raw),inactiveSlotDifferences,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),limitation:inv.limitation,observations:first.observations},null,2));
