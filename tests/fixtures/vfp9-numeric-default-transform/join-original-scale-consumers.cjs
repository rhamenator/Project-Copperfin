// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER-capture finite cross-capture join.
const fs=require('node:fs'),p=require('node:path'),z=require('node:zlib'),c=require('node:crypto'),a=require('node:assert/strict');
const sha=b=>c.createHash('sha256').update(b).digest('hex');
const parents={159:'3D2C1E8DBBA38061',160:'3D2C1E8DBBA38062',165:'BD2C1E8DBBA38061',166:'BD2C1E8DBBA38062'};
const specs=[['original-scale','26407441cae30774f2b75ee381edf60a3b4c6c0f','ec82060851762e0314f7b08434c170e89f2acbcbcf2b1fb9946d15bac812148d',[13,18]],['original-scale-middle','daf531078fccbed0208a563a2a5d869fc671649e','aa6b7f0675cd237c89c0719abce44ea693e90efdc3fa9a92c292a7472a693a43',[14,15,16,17]]];
const meta=s=>{const m=Object.fromEntries(s.split(';').map(x=>x.split('=')));a.equal(m.type,'4E');delete m.integer;return m;};
function unique(rows,key){const m=new Map();for(const r of rows){const k=key(r);a(!m.has(k),'duplicate join key');m.set(k,r);}return m;}
const key=r=>[r.parentIndex,r.fixed,r.scale].join(':');
const controls=f=>f.slice(8,21).map((s,i)=>[0,5].includes(i)?meta(s):s);
const sources=[],rows=[];
for(const [name,commit,digest,scales]of specs){
 const dir=p.join(__dirname,name+'-consumer-captures1'),manifest=JSON.parse(fs.readFileSync(p.join(dir,'manifest.json'))),read=n=>z.gunzipSync(fs.readFileSync(p.join(dir,n+'.gz')));
 a.equal(manifest.sourceCommit,commit);a.equal(manifest.sourceStatusClean,true);a.equal(manifest.pairedQualified,true);
 const analysisManifest=JSON.parse(fs.readFileSync(p.join(dir,'analysis1-manifest.json')));
 const bound=(n,m)=>{const b=read(n),r=m.files.find(r=>r.name===n||r.name===n+'.gz');a(r);a.equal(sha(b),r.sha256);return b;};
 const raw=bound('first-results.txt',manifest);a.equal(sha(raw),digest);a.deepEqual(raw,bound('repeat-results.txt',manifest));
 const audit=JSON.parse(bound('audit.json',manifest)),analysis=JSON.parse(bound('analysis1-analysis.json',analysisManifest));
 for(const k of ['documentedFieldsRepeatEqual','parentControlsExact','targetMetadataAndBRSExact','exactRuntimeIdentity','rawByteEqual'])a.equal(audit[k],true);
 a.equal(analysis.rawSha256,digest);a.equal(analysis.metricSelfChecks,8);a.equal(analysis.nativeInvoked,false);
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|'+audit.recordsPerRun);a.equal(lines.length,audit.observations.length);
 for(const [i,r]of audit.observations.entries()){
  const f=lines[i].split('|');a.equal(f.length,42);a(scales.includes(r.scale));a.equal(r.kind,'scale'+r.scale);a.equal(r.setting,13);a.equal(r.width,40);a.equal(r.bits,parents[r.parentIndex]);a.equal(r.originalBits,r.bits);a(['OFF','ON'].includes(r.fixed));
  a.deepEqual(f.slice(0,8),[r.fixed,String(r.index),String(r.parentIndex),r.originalBits,r.bits,'13',String(r.scale),r.kind]);a.deepEqual(meta(f[22]),r.targetMeta);a.equal(f[23],r.bits);a.equal(f[24],r.targetDisplayHex);a.equal(f[25],r.targetStrHex);a.deepEqual(Object.fromEntries(f[26].split(';').map(x=>x.split('='))),r.targetCrt);
  const prior=analysis.targetTransformVsStr.find(s=>s.index===r.index&&s.parentIndex===r.parentIndex&&s.fixed===r.fixed&&s.kind===r.kind);a(prior);a.equal(prior.comparison.leftHex,r.targetDisplayHex);a.equal(prior.comparison.rightHex,r.targetStrHex);
  rows.push({...r,capture:name,sourceCommit:commit,rawSha256:digest,controls:controls(f)});
 }
 a.equal(audit.observations.length,8*scales.length);sources.push({capture:name,sourceCommit:commit,rawSha256:digest,scales});
}
const map=unique(rows,key);a.equal(map.size,48);
function sameControls(l,r){a.deepEqual(l.controls,r.controls);a.deepEqual(l.targetCrt,r.targetCrt);for(const k of ['type','width','real','cw'])a.equal(l.targetMeta[k],r.targetMeta[k]);a.equal(l.bits,r.bits);}
let sharedControlComparisons=0;
for(const parent of Object.keys(parents))for(const fixed of ['OFF','ON'])for(let scale=13;scale<=18;++scale){const r=map.get([parent,fixed,scale].join(':'));a(r,'missing join key');sameControls(map.get([parent,fixed,13].join(':')),r);++sharedControlComparisons;}
function decimal(s){const m=s.trim().match(/^([+-]?)(\d+)(?:\.(\d*))?(?:[eE]([+-]?\d+))?$/);a(m);let n=BigInt(m[2]+(m[3]||'')),e=BigInt(m[4]||0)-BigInt((m[3]||'').length);const negative=m[1]==='-';if(negative)n=-n;const zero=n===0n;if(zero)e=0n;else while(n%10n===0n){n/=10n;++e;}return {coefficient:String(n),exponent:String(e),negative,zero};}
const equal=(l,r)=>l.coefficient===r.coefficient&&l.exponent===r.exponent;
let metricSelfChecks=0;
for(const [l,r,want]of [['0','-0.000',true],['1.20','1.2e0',true],['0.0001','1e-4',true],['-12.3400','-1234e-2',true],['0.000000001','0',false],['1','-1',false]]){a.equal(equal(decimal(l),decimal(r)),want);++metricSelfChecks;}
a.equal(decimal('-0.00').negative,true);a.equal(decimal(' 0.00 ').negative,false);metricSelfChecks+=2;
function compare(lh,rh){const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})+$/);const b=Buffer.from(h,'hex');a(b.every(n=>n>0&&n<128));return b.toString('ascii');};const l=ascii(lh),r=ascii(rh),x=decimal(l),y=decimal(r);return {leftHex:lh,rightHex:rh,literalEqual:lh===rh,trimmedLiteralEqual:l.trimStart()===r.trimStart(),valueEqual:equal(x,y),zeroSignDifferent:x.zero&&y.zero&&x.negative!==y.negative};}
let joinSelfChecks=0;
a.throws(()=>unique([{n:1},{n:1}],r=>r.n));++joinSelfChecks;
const sample=rows[0];for(const mutate of [r=>{r.controls[2]='00';},r=>{r.targetCrt.module='00';},r=>{r.targetMeta.width='0020';},r=>{r.bits='0000000000000000';}]){const changed=structuredClone(sample);mutate(changed);a.throws(()=>sameControls(sample,changed));++joinSelfChecks;}
const ref=r=>({capture:r.capture,index:r.index,parentIndex:r.parentIndex,fixed:r.fixed,scale:r.scale,bits:r.bits});
const pair=(l,r)=>({left:ref(l),right:ref(r),transform:compare(l.targetDisplayHex,r.targetDisplayHex),str:compare(l.targetStrHex,r.targetStrHex)});
const interventions=[],adjacent=[];
for(const parent of Object.keys(parents))for(const fixed of ['OFF','ON'])for(let scale=13;scale<18;++scale){const l=map.get([parent,fixed,scale].join(':')),r=map.get([parent,fixed,scale+1].join(':'));sameControls(l,r);a.equal(parseInt(l.targetMeta.decimals,16),scale);a.equal(parseInt(r.targetMeta.decimals,16),scale+1);interventions.push(pair(l,r));}
a.equal(interventions.length,40);
for(const [lp,rp]of [[159,160],[165,166]])for(const fixed of ['OFF','ON'])for(let scale=13;scale<=18;++scale){const l=map.get([lp,fixed,scale].join(':')),r=map.get([rp,fixed,scale].join(':'));a.equal(BigInt('0x'+r.bits)-BigInt('0x'+l.bits),1n);adjacent.push(pair(l,r));}a.equal(adjacent.length,24);
const targetTransformVsStr=rows.map(r=>({source:ref(r),comparison:compare(r.targetDisplayHex,r.targetStrHex)}));
const summary=(rs,k)=>({comparisons:rs.length,literalDifferent:rs.filter(r=>!r[k].literalEqual).length,trimmedLiteralDifferent:rs.filter(r=>!r[k].trimmedLiteralEqual).length,valueDifferent:rs.filter(r=>!r[k].valueEqual).length,zeroSignDifferent:rs.filter(r=>r[k].zeroSignDifferent).length});
const paired=rs=>({transform:summary(rs,'transform'),str:summary(rs,'str')});
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,sources,contextsChecked:rows.length,sharedControlComparisons,metricSelfChecks,joinSelfChecks,summary:{metadataEdges:Object.fromEntries([13,14,15,16,17].map(s=>[s+'->'+(s+1),paired(interventions.filter(r=>r.left.scale===s))])),adjacent:paired(adjacent),targetTransformVsStr:summary(targetTransformVsStr,'comparison')},interventions,adjacent,targetTransformVsStr,limitation:'Finite four signed ORIGINAL D13 payloads, width40, metadata13..18 and both FIXED. Shared observed controls/runtime tuples checked, not same execution/session or internal-call proof. Distinct source/raw provenance retained. Exact decimal value, spelling/padding and zero sign separate; no fitted tolerance/cap, formatter rule, universal propagation/conversion or parity acceptance.'},null,2));
