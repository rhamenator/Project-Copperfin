// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent parent/raw, metadata, runtime and repeat reader; no expected new display.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const root=process.argv[2];a(root);const sha=b=>c.createHash('sha256').update(b).digest('hex');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'scale-intervention-cases.json')));
const source=fs.readFileSync(path.join(__dirname,'scale-intervention.prg'),'utf8');
a.equal(inv.inputCount,20);a.equal(inv.recordsPerRun,40);a.equal(inv.fieldsPerRecord,42);
const seeds=[[193,16,'3CC203398FC21737'],[199,16,'BCC203398FC21737'],[205,17,'3CC203A3D096CFC9'],[211,17,'BCC203A3D096CFC9'],[217,18,'3CC203AE70AC1571'],[223,18,'BCC203AE70AC1571']];
const expected=[];for(const[parentIndex,setting,originalBits]of seeds)for(const kind of ['original','round'])for(const scale of setting===18?[18]:[18,setting])expected.push({index:expected.length+1,parentIndex,originalBits,bits:kind==='original'?originalBits:originalBits[0]==='B'?'BCC203AF9EE75616':'3CC203AF9EE75616',setting,scale,kind});
a.deepEqual(inv.inputs,expected);
for(const r of expected)for(const[n,v]of [['inputOriginal',`'${r.originalBits}'`],['inputHex',`'${r.bits}'`],['inputParent',r.parentIndex],['inputD',r.setting],['inputScale',r.scale],['inputKind',`'${r.kind}'`]])a(source.includes(`${n}[${r.index}]=${v}\n`));
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
function meta(s,bits,scale){
 a.match(bits,/^[0-9A-F]{16}$/);a.notEqual((BigInt('0x'+bits)>>52n)&2047n,2047n);
 a.match(s,/^type=4E;width=0028;decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
 const m=Object.fromEntries(s.split(';').map(x=>x.split('=')));a.equal(m.real,bits);a.equal(parseInt(m.decimals,16),scale);delete m.integer;return m;
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
}a.equal(parents.size,12);
for(const r of expected)for(const fixed of ['OFF','ON']){const {p}=parents.get(r.parentIndex+':'+fixed);a.equal(p.bits,r.originalBits);a.equal(p.setting,r.setting);a.equal(r.bits,r.kind==='original'?p.bits:p.inlineBits);}
if(root==='--source-only'){console.log(JSON.stringify({sourceOnly:true,inputs:20,records:40,fields:42,parentRawControlsChecked:12,nativeInvoked:false,newResultsRead:false}));process.exit(0);}
const bytes=n=>{const p=path.join(root,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
a.equal(bytes('supplied-runtime-env.txt').toString(),inv.runtimeEnvironment+'\n');a(['UNSET\n','\n'].includes(bytes('inherited-dll-overrides.txt').toString()));
function read(name){
 a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));a(raw.every(n=>n<128));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|40');a.equal(lines.length,40);
 const observations=[],fields=[];let ordinal=0;
 for(const r of expected)for(const fixed of ['OFF','ON']){
  const f=lines[ordinal++].split('|');a.equal(f.length,42);a.deepEqual(f.slice(0,8),[fixed,String(r.index),String(r.parentIndex),r.originalBits,r.bits,String(r.setting),String(r.scale),r.kind]);
  const {p,f:pf}=parents.get(r.parentIndex+':'+fixed);
  // Original and actual ROUND expression are unchanged parent controls, NOT target expectations.
  for(const start of [8,27]){a.deepEqual(meta(f[start],r.originalBits,18),meta(pf[4],r.originalBits,18));a.equal(f[start+1],r.originalBits);a.equal(f[start+2],p.originalDisplayHex);a.equal(f[start+3],p.originalStrHex);a.deepEqual(binding(f[start+4],r.originalBits),p.crt);}
  for(const start of [13,32]){a.deepEqual(meta(f[start],p.inlineBits,r.setting),p.inlineMeta);a.equal(f[start+1],p.inlineBits);a.equal(f[start+2],p.inlineDisplayHex);a.equal(f[start+3],p.inlineStrHex);}
  a.deepEqual(f.slice(17,21),pf.slice(8,12));a.equal(f[17],'N');a.equal(BigInt(ascii(f[18]).trim().split('.')[0]),BigInt(r.setting));a.equal(f[19],'C');a.equal(ascii(f[20]),fixed);
  a.equal(f[21],'N');a.equal(f[36],'N');const targetMeta=meta(f[22],r.bits,r.scale);a.deepEqual(meta(f[37],r.bits,r.scale),targetMeta);a.equal(f[23],r.bits);a.equal(f[38],r.bits);a.equal(f[24],f[39]);a.equal(f[25],f[40]);ascii(f[24]);a.equal(ascii(f[25]).length,40);
  const crt=binding(f[26],r.bits);a.deepEqual(binding(f[41],r.bits),crt);
  observations.push({...r,fixed,targetMeta,targetDisplayHex:f[24],targetStrHex:f[25],targetCrt:crt,parentOriginalDisplayHex:p.originalDisplayHex,parentOriginalStrHex:p.originalStrHex,parentRoundDisplayHex:p.inlineDisplayHex,parentRoundStrHex:p.inlineStrHex});fields.push(f);
 }return {raw,observations,fields};
}
const first=read('first');
if(process.argv[3]==='--single'){console.log(JSON.stringify({firstComplete:true,records:40,parentControlsExact:true,targetMetadataAndBRSExact:true,exactRuntimeIdentity:true,firstSha256:sha(first.raw),pairedQualified:false}));process.exit(0);}
a.equal(bytes('first-structure.status').toString(),'0\n');a.equal(bytes('first-structure.stderr').length,0);const single=JSON.parse(bytes('first-structure.json'));a.equal(single.firstComplete,true);a.equal(single.pairedQualified,false);a.equal(single.firstSha256,sha(first.raw));
const repeat=read('repeat');a.deepEqual(first.observations,repeat.observations);let inactiveSlotDifferences=0;
for(let i=0;i<40;i++)for(let j=0;j<42;j++)if(first.fields[i][j]!==repeat.fields[i][j]){a([8,13,22,27,32,37].includes(j));const r=expected[Math.floor(i/2)],p=parents.get(r.parentIndex+':'+(i%2?'ON':'OFF')).p,bits=[8,27].includes(j)?r.originalBits:[13,32].includes(j)?p.inlineBits:r.bits,scale=[8,27].includes(j)?18:[13,32].includes(j)?r.setting:r.scale;a.deepEqual(meta(first.fields[i][j],bits,scale),meta(repeat.fields[i][j],bits,scale));++inactiveSlotDifferences;}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:40,documentedFieldsRepeatEqual:true,parentControlsExact:true,targetMetadataAndBRSExact:true,exactRuntimeIdentity:true,rawByteEqual:first.raw.equals(repeat.raw),inactiveSlotDifferences,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),limitation:inv.limitation,observations:first.observations},null,2));
