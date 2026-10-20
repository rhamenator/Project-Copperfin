// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER-capture finite cross-capture join.
const fs=require('node:fs'),p=require('node:path'),z=require('node:zlib'),c=require('node:crypto'),a=require('node:assert/strict');
const sha=b=>c.createHash('sha256').update(b).digest('hex');
const parents={159:'3D2C1E8DBBA38061',160:'3D2C1E8DBBA38062',165:'BD2C1E8DBBA38061',166:'BD2C1E8DBBA38062'};
const specs=[['original-scale','26407441cae30774f2b75ee381edf60a3b4c6c0f','ec82060851762e0314f7b08434c170e89f2acbcbcf2b1fb9946d15bac812148d',[13,18]],['original-scale-middle','daf531078fccbed0208a563a2a5d869fc671649e','aa6b7f0675cd237c89c0719abce44ea693e90efdc3fa9a92c292a7472a693a43',[14,15,16,17]],['original-setting','886b8720d56fa770847c79a9dacba4dbdf1d0a5f','47232c78c28f7b7536b654b3c85fd94bd7a6eead587f719298f80762f02c2499',[13,14]]];
const meta=s=>{const m=Object.fromEntries(s.split(';').map(x=>x.split('=')));a.equal(m.type,'4E');delete m.integer;return m;};
function unique(rows,key){const m=new Map();for(const r of rows){const k=key(r);a(!m.has(k),'duplicate join key');m.set(k,r);}return m;}
const key=r=>[r.parentIndex,r.fixed,r.setting,r.scale].join(':');
const controls=f=>f.slice(8,21).map((s,i)=>[0,5].includes(i)?meta(s):s);
const sources=[],rows=[];
for(const [name,commit,digest,scales]of specs){
 const dir=p.join(__dirname,name+'-consumer-captures1'),manifest=JSON.parse(fs.readFileSync(p.join(dir,'manifest.json'))),read=n=>z.gunzipSync(fs.readFileSync(p.join(dir,n+'.gz')));
 a.equal(manifest.sourceCommit,commit);a.equal(manifest.sourceStatusClean,true);a.equal(manifest.pairedQualified,true);
 const analysisManifest=JSON.parse(fs.readFileSync(p.join(dir,'analysis1-manifest.json')));
 const bound=(n,m)=>{const b=read(n),r=m.files.find(r=>r.name===n||r.name===n+'.gz');a(r);a.equal(sha(b),r.sha256);return b;};
 const raw=bound('first-results.txt',manifest);a.equal(sha(raw),digest);a.deepEqual(raw,bound('repeat-results.txt',manifest));
 const audit=JSON.parse(bound('audit.json',manifest)),analysis=JSON.parse(bound('analysis1-analysis.json',analysisManifest));
 for(const k of ['documentedFieldsRepeatEqual','targetMetadataAndBRSExact','exactRuntimeIdentity','rawByteEqual'])a.equal(audit[k],true);
 if(name==='original-setting'){a.equal(audit.parentPayloadAndRuntimeExact,true);a.equal(audit.newSettingControlInvariance,true);}else a.equal(audit.parentControlsExact,true);
 a.equal(analysis.rawSha256,digest);a.equal(analysis.metricSelfChecks,8);a.equal(analysis.nativeInvoked,false);
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|'+audit.recordsPerRun);a.equal(lines.length,audit.observations.length);
 for(const [i,r]of audit.observations.entries()){
  const f=lines[i].split('|');a.equal(f.length,42);a(scales.includes(r.scale));a.equal(r.kind,'scale'+r.scale);a((name==='original-setting'?[12,14]:[13]).includes(r.setting));a.equal(r.width,40);a.equal(r.bits,parents[r.parentIndex]);a.equal(r.originalBits,r.bits);a(['OFF','ON'].includes(r.fixed));
  a.deepEqual(f.slice(0,8),[r.fixed,String(r.index),String(r.parentIndex),r.originalBits,r.bits,String(r.setting),String(r.scale),r.kind]);a.deepEqual(meta(f[22]),r.targetMeta);a.equal(f[23],r.bits);a.equal(f[24],r.targetDisplayHex);a.equal(f[25],r.targetStrHex);a.deepEqual(Object.fromEntries(f[26].split(';').map(x=>x.split('='))),r.targetCrt);
  const prior=analysis.targetTransformVsStr.find(s=>s.index===r.index&&s.parentIndex===r.parentIndex&&s.fixed===r.fixed&&s.kind===r.kind&&s.setting===r.setting);a(prior);a.equal(prior.comparison.leftHex,r.targetDisplayHex);a.equal(prior.comparison.rightHex,r.targetStrHex);
  if((name==='original-scale'&&r.scale===13)||(name==='original-scale-middle'&&r.scale===14)||name==='original-setting')rows.push({...r,capture:name,sourceCommit:commit,rawSha256:digest,controls:controls(f)});
 }
 a.equal(audit.observations.length,(name==='original-setting'?16:8)*scales.length);sources.push({capture:name,sourceCommit:commit,rawSha256:digest,scales});
}
const map=unique(rows,key);a.equal(map.size,48);
function sharedIdentity(l,r){a.equal(l.bits,r.bits);for(const k of ['type','width','real','cw'])a.equal(l.targetMeta[k],r.targetMeta[k]);a.deepEqual(l.targetCrt,r.targetCrt);a.deepEqual(l.controls[0],r.controls[0]);a.equal(l.controls[1],r.controls[1]);a.equal(l.controls[4],r.controls[4]);}
function sameControls(l,r){sharedIdentity(l,r);a.equal(l.setting,r.setting);a.deepEqual(l.controls,r.controls);}
const get=(parent,fixed,setting,scale)=>{const r=map.get([parent,fixed,setting,scale].join(':'));a(r,'missing join key');return r;};
let sharedControlComparisons=0;
for(const parent of Object.keys(parents))for(const fixed of ['OFF','ON'])for(const setting of [12,13,14])for(const scale of [13,14]){const r=get(parent,fixed,setting,scale);sameControls(get(parent,fixed,setting,13),r);sharedIdentity(get(parent,fixed,12,13),r);++sharedControlComparisons;}
function decimal(s){const m=s.trim().match(/^([+-]?)(\d+)(?:\.(\d*))?(?:[eE]([+-]?\d+))?$/);a(m);let n=BigInt(m[2]+(m[3]||'')),e=BigInt(m[4]||0)-BigInt((m[3]||'').length);const negative=m[1]==='-';if(negative)n=-n;const zero=n===0n;if(zero)e=0n;else while(n%10n===0n){n/=10n;++e;}return {coefficient:String(n),exponent:String(e),negative,zero};}
const equal=(l,r)=>l.coefficient===r.coefficient&&l.exponent===r.exponent;
let metricSelfChecks=0;
for(const [l,r,want]of [['0','-0.000',true],['1.20','1.2e0',true],['0.0001','1e-4',true],['-12.3400','-1234e-2',true],['0.000000001','0',false],['1','-1',false]]){a.equal(equal(decimal(l),decimal(r)),want);++metricSelfChecks;}
a.equal(decimal('-0.00').negative,true);a.equal(decimal(' 0.00 ').negative,false);metricSelfChecks+=2;
function compare(lh,rh){const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})+$/);const b=Buffer.from(h,'hex');a(b.every(n=>n>0&&n<128));return b.toString('ascii');};const l=ascii(lh),r=ascii(rh),x=decimal(l),y=decimal(r);return {leftHex:lh,rightHex:rh,literalEqual:lh===rh,trimmedLiteralEqual:l.trimStart()===r.trimStart(),valueEqual:equal(x,y),zeroSignDifferent:x.zero&&y.zero&&x.negative!==y.negative};}
let joinSelfChecks=0;
a.throws(()=>unique([{n:1},{n:1}],r=>r.n));++joinSelfChecks;
const sample=rows[0];for(const mutate of [r=>{r.controls[2]='00';},r=>{r.targetCrt.module='00';},r=>{r.targetMeta.width='0020';},r=>{r.bits='0000000000000000';}]){const changed=structuredClone(sample);mutate(changed);a.throws(()=>sameControls(sample,changed));++joinSelfChecks;}
const changedD=structuredClone(sample);changedD.setting=sample.setting===12?14:12;changedD.controls[2]='00';a.doesNotThrow(()=>sharedIdentity(sample,changedD));++joinSelfChecks;
const ref=r=>({capture:r.capture,index:r.index,parentIndex:r.parentIndex,fixed:r.fixed,setting:r.setting,scale:r.scale,bits:r.bits});

const joinDir=p.join(__dirname,'original-setting-join-captures1'),joinManifest=JSON.parse(fs.readFileSync(p.join(joinDir,'manifest.json'))),joinBytes=z.gunzipSync(fs.readFileSync(p.join(joinDir,'analysis.json.gz'))),joinEntry=joinManifest.files.find(r=>r.name==='analysis.json.gz');a(joinEntry);a.equal(sha(joinBytes),joinEntry.sha256);const joined=JSON.parse(joinBytes);a.deepEqual(joined.sources,sources);a.equal(joined.contextsChecked,48);a.equal(joined.metricSelfChecks,8);a.equal(joined.joinSelfChecks,6);
for(const r of rows){const prior=joined.targetTransformVsStr.find(s=>s.source.capture===r.capture&&s.source.index===r.index&&s.source.fixed===r.fixed);a(prior);a.deepEqual(prior.source,ref(r));a.deepEqual(prior.comparison,compare(r.targetDisplayHex,r.targetStrHex));}
const comparisons=[];
for(const tf of rows)for(const count of [12,13,14]){const str=get(tf.parentIndex,tf.fixed,count,tf.scale);sharedIdentity(tf,str);a.deepEqual(tf.targetMeta,str.targetMeta);comparisons.push({transform:ref(tf),str:{...ref(str),publicCount:count},sameSettingCount:tf.setting===count,metadataCount:tf.scale===count,comparison:compare(tf.targetDisplayHex,str.targetStrHex)});}a.equal(comparisons.length,144);
const summary=rs=>({comparisons:rs.length,literalDifferent:rs.filter(r=>!r.comparison.literalEqual).length,trimmedLiteralDifferent:rs.filter(r=>!r.comparison.trimmedLiteralEqual).length,valueDifferent:rs.filter(r=>!r.comparison.valueEqual).length,zeroSignDifferent:rs.filter(r=>r.comparison.zeroSignDifferent).length});
const sameSetting=comparisons.filter(r=>r.sameSettingCount),metadataCount=comparisons.filter(r=>r.metadataCount);a.equal(sameSetting.length,48);a.equal(metadataCount.length,48);
let selectorSelfChecks=0;const toy=[{setting:12,scale:13},{setting:13,scale:13},{setting:14,scale:13}];a.deepEqual(toy.map(r=>[12,13,14].filter(n=>n===r.setting)),[[12],[13],[14]]);++selectorSelfChecks;a.deepEqual(toy.map(r=>[12,13,14].filter(n=>n===r.scale)),[[13],[13],[13]]);++selectorSelfChecks;
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,sources,parentJoinSha256:sha(joinBytes),contextsChecked:rows.length,sharedControlComparisons,metricSelfChecks,joinSelfChecks,selectorSelfChecks,summary:{all:summary(comparisons),sameSettingCount:summary(sameSetting),metadataCount:summary(metadataCount),byFixed:Object.fromEntries(['OFF','ON'].map(fixed=>[fixed,{sameSettingCount:summary(sameSetting.filter(r=>r.transform.fixed===fixed)),metadataCount:summary(metadataCount.filter(r=>r.transform.fixed===fixed)),byStrCount:Object.fromEntries([12,13,14].map(n=>[n,summary(comparisons.filter(r=>r.transform.fixed===fixed&&r.str.publicCount===n))]))}]))},comparisons,limitation:'Finite four signed ORIGINAL D13 payloads, width40, metadata13/14, DECIMALS12/13/14 and both FIXED. Public STR counts are actual cross-setting observations with sourceD/count provenance, not newly computed target text. Shared payload/active metadata/runtime checked; controls need not equal across D. Same-setting and metadata-count output equality are finite consumer comparisons, not an internal call graph or universal formatter/rounding/conversion rule. Spelling/padding, exact decimal value and zero sign separate; no fitted tolerance/cap or parity acceptance.'},null,2));
