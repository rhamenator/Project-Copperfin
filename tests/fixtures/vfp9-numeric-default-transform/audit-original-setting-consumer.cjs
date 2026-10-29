// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: ORIGINAL payload decimal-metadata comparison.
// Independent parent/raw, metadata, runtime and repeat reader; no expected new display.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const root=process.argv[2];a(root);const sha=b=>c.createHash('sha256').update(b).digest('hex');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'original-setting-consumer-cases.json')));
const source=fs.readFileSync(path.join(__dirname,'original-setting-consumer.prg'),'utf8');
a.equal(inv.inputCount,16);a.equal(inv.recordsPerRun,32);a.equal(inv.fieldsPerRecord,42);
a.deepEqual(inv.scales,[13,14]);a.deepEqual(inv.settings,[12,14]);a.equal(inv.width,40);
// Independent literal witness table; do not import the emitter or predict target text.
const seeds=[[159,13,'3D2C1E8DBBA38061'],[160,13,'3D2C1E8DBBA38062'],[165,13,'BD2C1E8DBBA38061'],[166,13,'BD2C1E8DBBA38062']];
const expected=[];for(const[parentIndex,oldSetting,originalBits]of seeds)for(const setting of [12,14])for(const scale of [13,14])expected.push({index:expected.length+1,parentIndex,originalBits,bits:originalBits,setting,scale,width:40,kind:'scale'+scale});
a.deepEqual(inv.inputs,expected);
for(const r of expected)for(const[n,v]of [['inputOriginal',`'${r.originalBits}'`],['inputHex',`'${r.bits}'`],['inputParent',r.parentIndex],['inputD',r.setting],['inputScale',r.scale],['inputKind',`'${r.kind}'`],['inputWidth',r.width]])a(source.includes(`${n}[${r.index}]=${v}\n`));
const orig='CFNDEC(CFNWIDTH(originalPayload,40),18)',rounded=`ROUND(${orig},inputD[index])`,target='CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index])';
for(const[prefix,expr,typed,bound]of [['original',orig,false,true],['round',rounded,false,false],['target',target,true,true]])for(const suffix of ['','After']){
 const p=prefix+suffix;
 for(const[n,e]of [[p+'Meta',`CFNMETA(${expr})`],[p+'Bits',`HexBytes(BINTOC(${expr},'BRS'))`],[p+'Display',`HexBytes(TRANSFORM(${expr}))`],[p+'Str',`HexBytes(STR(${expr},40,inputD[index]))`],...(typed?[[p+'Type',`VARTYPE(${expr})`]]:[]),...(bound?[[p+'Binding',`CFNFCVT(${expr},17)`]]:[])])a(source.includes(`   ${n}=${e}\n`));
 if(bound)a(source.includes(`ERROR-${p}Binding|`));
}
a(source.includes('SET DECIMALS TO decimalCount'));a(source.includes('decimalCount=inputD[index]'));a(source.includes('ERROR-LOAD|'));
const outputLine=source.split('\n').filter(l=>l.startsWith('   line=fixedMode'));a.equal(outputLine.length,1);a.equal((outputLine[0].match(/\+'\|'/g)||[]).length,41);
const assignments=source.split('\n').filter(l=>/^\s+\w+=/.test(l));
for(const l of assignments)if(/=(?:CFNDEC|CFNWIDTH|ROUND)\(/i.test(l))a.fail('numeric argument/result assignment');
const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})*$/);const b=Buffer.from(h,'hex');a(b.every(n=>n>0&&n<128));return b.toString('ascii');};
function meta(s,bits,scale,width=40){
 a.match(bits,/^[0-9A-F]{16}$/);a.notEqual((BigInt('0x'+bits)>>52n)&2047n,2047n);
 a.match(s,/^type=4E;width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
 const m=Object.fromEntries(s.split(';').map(x=>x.split('=')));a.equal(m.real,bits);if(width!==null)a.equal(parseInt(m.width,16),width);if(scale!==null)a.equal(parseInt(m.decimals,16),scale);delete m.integer;return m;
}
function binding(s,bits){
 a.match(s,/^kind=F;count=00000011;real=[0-9A-F]{16};decimal=[0-9A-F]{8};sign=[0-9A-F]{8};cw0=027F;cw1=027F;module=(?:[0-9A-F]{2})+;fileSample=ONCE;fileSize=[0-9A-F]{8};fileSha256=[0-9A-F]{64};machine=[0-9A-F]{4};magic=[0-9A-F]{4};stamp=[0-9A-F]{8};imageSize=[0-9A-F]{8};checksum=[0-9A-F]{8};digits=(?:3[0-9])*$/);
 const r=Object.fromEntries(s.split(';').map(x=>x.split('=')));a.equal(r.real,bits);a(r.digits.length<=254);a.equal(ascii(r.module).toLowerCase(),inv.modulePath.toLowerCase());a.equal(r.fileSha256.toLowerCase(),inv.moduleSha256);a.equal(parseInt(r.fileSize,16),inv.moduleFileSize);for(const[k,v]of Object.entries(inv.modulePe))a.equal(parseInt(r[k],16),v);return r;
}
a.equal(inv.moduleSha256,'8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe');a.equal(inv.runtimeEnvironment,'WINEDLLOVERRIDES=msvcr71=n');
const parentRoot=path.join(__dirname,'round-inline-captures1'),parentBytes=n=>z.gunzipSync(fs.readFileSync(path.join(parentRoot,n+'.gz')));
const sums=new Map();for(const l of fs.readFileSync(path.join(parentRoot,'SHA256SUMS'),'utf8').trimEnd().split('\n')){const m=l.match(/^([0-9a-f]{64})  ([A-Za-z0-9_.-]+)$/);a(m);a(!sums.has(m[2]));sums.set(m[2],m[1]);}
for(const n of ['audit.json','first-results.txt','repeat-results.txt'])a.equal(sha(fs.readFileSync(path.join(parentRoot,n+'.gz'))),sums.get(n+'.gz'));
const parent=JSON.parse(parentBytes('audit.json')),parentHash='254b46dadbb71fa51b10e8439d51785b2c2aa3eb871c8c948fd3b8bdd3da4873';a.equal(inv.parentNativeSha256,parentHash);
for(const k of ['documentedFieldsRepeatEqual','inlineResultAndStoredParentControls','exactRuntimeIdentity','rawByteEqual'])a.equal(parent[k],true);
for(const run of ['first','repeat']){a.equal(parent[run+'Sha256'],parentHash);a.equal(sha(parentBytes(run+'-results.txt')),parentHash);a.deepEqual(parentBytes(run+'-results.txt'),parentBytes(run+'.out'));a.equal(parentBytes(run+'.status').toString(),'0\n');a.equal(parentBytes(run+'.stderr').length,0);}
const lines=parentBytes('first-results.txt').toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|528');a.equal(lines.length,528);
const parents=new Map();for(const line of lines){const f=line.split('|');a.equal(f.length,35);const index=Number(f[1]);if(!seeds.some(s=>s[0]===index))continue;
 const p=parent.observations.find(p=>p.index===index&&p.fixed===f[0]);a(p);a.equal(f[2],p.bits);a.equal(f[3],String(p.setting));a.equal(f[5],p.bits);a.deepEqual(meta(f[4],p.bits,18),meta(f[18],p.bits,18));a.equal(f[19],p.bits);a.equal(f[6],p.originalDisplayHex);a.equal(f[7],p.originalStrHex);a.equal(f[20],f[6]);
 a.deepEqual(binding(f[12],p.bits),p.crt);a.deepEqual(binding(f[21],p.bits),p.crt);a.equal(f[26],'N');a.equal(f[28],p.inlineBits);a.equal(f[32],p.inlineBits);a.deepEqual(meta(f[27],p.inlineBits,p.setting),p.inlineMeta);a.deepEqual(meta(f[31],p.inlineBits,p.setting),p.inlineMeta);a.equal(f[29],p.inlineDisplayHex);a.equal(f[33],f[29]);a.equal(f[30],p.inlineStrHex);a.equal(f[34],f[30]);
 const k=index+':'+f[0];a(!parents.has(k));parents.set(k,{p,f});
}a.equal(parents.size,8);
for(const r of expected)for(const fixed of ['OFF','ON']){const {p}=parents.get(r.parentIndex+':'+fixed);a.equal(p.bits,r.originalBits);a.equal(p.setting,13);a.equal(r.bits,seeds.find(s=>s[0]===r.parentIndex)[2]);}
if(root==='--source-only'){console.log(JSON.stringify({sourceOnly:true,inputs:16,records:32,fields:42,parentRawProvenanceChecked:8,originalPayloadStimuliOnly:true,nativeInvoked:false,newResultsRead:false}));process.exit(0);}
const bytes=n=>{const p=path.join(root,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
a.equal(bytes('supplied-runtime-env.txt').toString(),inv.runtimeEnvironment+'\n');a(['UNSET\n','\n'].includes(bytes('inherited-dll-overrides.txt').toString()));
function read(name){
 a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));a(raw.every(n=>n<128));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|32');a.equal(lines.length,32);
 const observations=[],fields=[],controlTuples=new Map();let ordinal=0;
 for(const r of expected)for(const fixed of ['OFF','ON']){
  const f=lines[ordinal++].split('|');a.equal(f.length,42);a.deepEqual(f.slice(0,8),[fixed,String(r.index),String(r.parentIndex),r.originalBits,r.bits,String(r.setting),String(r.scale),r.kind]);
  const {p,f:pf}=parents.get(r.parentIndex+':'+fixed);
  // The old D13 parent establishes original payload/runtime only, NOT new-setting text or ROUND payload.
  for(const start of [8,27]){a.deepEqual(meta(f[start],r.originalBits,18),meta(pf[4],r.originalBits,18));a.equal(f[start+1],r.originalBits);ascii(f[start+2]);a.equal(ascii(f[start+3]).length,40);a.deepEqual(binding(f[start+4],r.originalBits),p.crt);}
  for(const [left,right]of [[8,27],[13,32]]){a.deepEqual(meta(f[left],f[left+1],left===8?18:null,left===8?40:null),meta(f[right],f[right+1],right===27?18:null,right===27?40:null));for(let j=1;j<4;++j)a.equal(f[left+j],f[right+j]);ascii(f[left+2]);a.equal(ascii(f[left+3]).length,40);}
  a.equal(f[17],'N');a.match(ascii(f[18]).trim(),new RegExp('^'+r.setting+'(?:\\.0*)?$'));a.equal(f[19],'C');a.equal(ascii(f[20]),fixed);
  const controlKey=r.parentIndex+':'+r.setting+':'+fixed,control=f.slice(8,21).map((v,i)=>[0,5].includes(i)?meta(v,f[i===0?9:14],null,null):v);
  if(controlTuples.has(controlKey))a.deepEqual(controlTuples.get(controlKey),control);else controlTuples.set(controlKey,control);
  a.equal(f[21],'N');a.equal(f[36],'N');const targetMeta=meta(f[22],r.bits,r.scale,r.width);a.deepEqual(meta(f[37],r.bits,r.scale,r.width),targetMeta);a.equal(f[23],r.bits);a.equal(f[38],r.bits);a.equal(f[24],f[39]);a.equal(f[25],f[40]);ascii(f[24]);a.equal(ascii(f[25]).length,40);
  const crt=binding(f[26],r.bits);a.deepEqual(binding(f[41],r.bits),crt);
  observations.push({...r,fixed,targetMeta,targetDisplayHex:f[24],targetStrHex:f[25],targetCrt:crt,parentOriginalDisplayHex:f[10],parentOriginalStrHex:f[11],parentRoundDisplayHex:f[15],parentRoundStrHex:f[16],parentRoundBits:f[14],parentRoundMeta:meta(f[13],f[14],null,null),controlTuple:control});fields.push(f);
 }a.equal(controlTuples.size,16);return {raw,observations,fields};
}
const first=read('first');
if(process.argv[3]==='--single'){console.log(JSON.stringify({firstComplete:true,records:32,parentPayloadAndRuntimeExact:true,newSettingControlInvariance:true,targetMetadataAndBRSExact:true,exactRuntimeIdentity:true,firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);}
a.equal(bytes('first-structure.status').toString(),'0\n');a.equal(bytes('first-structure.stderr').length,0);const single=JSON.parse(bytes('first-structure.json'));a.equal(single.firstComplete,true);a.equal(single.pairedQualified,false);a.equal(single.firstSha256,sha(first.raw));
const repeat=read('repeat');a.deepEqual(first.observations,repeat.observations);let inactiveSlotDifferences=0;
for(let i=0;i<32;i++)for(let j=0;j<42;j++)if(first.fields[i][j]!==repeat.fields[i][j]){a([8,13,22,27,32,37].includes(j));const bits=first.fields[i][j+1],scale=[8,27].includes(j)?18:[13,32].includes(j)?null:expected[Math.floor(i/2)].scale,width=[13,32].includes(j)?null:40;a.deepEqual(meta(first.fields[i][j],bits,scale,width),meta(repeat.fields[i][j],bits,scale,width));++inactiveSlotDifferences;}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:32,documentedFieldsRepeatEqual:true,parentPayloadAndRuntimeExact:true,newSettingControlInvariance:true,targetMetadataAndBRSExact:true,exactRuntimeIdentity:true,rawByteEqual:first.raw.equals(repeat.raw),inactiveSlotDifferences,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),limitation:inv.limitation,observations:first.observations},null,2));
