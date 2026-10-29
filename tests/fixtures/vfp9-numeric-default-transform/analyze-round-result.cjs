// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Offline comparison AFTER qualified capture; no expected formatter or internal-call claim.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const root=process.argv[2];a(root);const bytes=(dir,n)=>{const p=path.join(dir,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
const current=JSON.parse(bytes(root,'audit.json')),parentRoot=path.join(__dirname,'crt-precision-captures1'),parent=JSON.parse(bytes(parentRoot,'audit.json'));
a.equal(current.recordsPerRun,528);a.equal(current.observations.length,528);
for(const k of ['documentedFieldsRepeatEqual','typedResultAndControls','exactRuntimeIdentity'])a.equal(current[k],true);
const hash=b=>c.createHash('sha256').update(b).digest('hex'),nativeHash='0be810176af4c28eca3864eae70dc8adbfe409986ca227cdde8ccaa7c8013607',parentHash='ba189979b2d7cd67ab15f1d719af586f815f28f98427d30344629760f1fbabd3';
for(const n of ['first','repeat']){a.equal(current[n+'Sha256'],nativeHash);a.equal(hash(bytes(root,n+'-results.txt')),nativeHash);a.equal(parent[n+'Sha256'],parentHash);a.equal(hash(bytes(parentRoot,n+'-results.txt')),parentHash);}
const old=new Map(parent.observations.filter(r=>r.count===r.setting+1).map(r=>[JSON.stringify([r.index,r.fixed]),r]));a.equal(old.size,528);
const seen=new Set();for(const r of current.observations){const key=JSON.stringify([r.index,r.fixed]);a(!seen.has(key));seen.add(key);const p=old.get(key);a(p);a.equal(r.bits,p.bits);a.equal(r.setting,p.setting);a.equal(r.originalDisplayHex,p.displayHex);a.equal(r.originalStrHex,p.strHex);}
const results=[];for(const fixed of ['OFF','ON']){
 const rows=current.observations.filter(r=>r.fixed===fixed);a.equal(rows.length,264);const groups=new Map();
 for(const r of rows){const m=r.resultMeta,key=JSON.stringify([r.fixed,r.setting,40,18,m.type,m.width,m.decimals,r.resultBits]),g=groups.get(key)||[];g.push(r);groups.set(key,g);}
 const conflicts=[...groups].filter(([,g])=>new Set(g.map(r=>r.originalDisplayHex)).size>1);
 const view=r=>({index:r.index,setting:r.setting,bits:r.bits,resultBits:r.resultBits,resultMeta:r.resultMeta,originalDisplayHex:r.originalDisplayHex,resultDisplayHex:r.resultDisplayHex,originalStrHex:r.originalStrHex,resultStrHex:r.resultStrHex});
 results.push({fixed,contexts:264,bitsChanged:rows.filter(r=>r.bits!==r.resultBits).length,displayMismatch:rows.filter(r=>r.originalDisplayHex!==r.resultDisplayHex).length,strMismatch:rows.filter(r=>r.originalStrHex!==r.resultStrHex).length,distinctResultTuples:groups.size,conflictingTuples:conflicts.length,conflictingContexts:conflicts.reduce((n,[,g])=>n+g.length,0),counterexamples:conflicts.map(([key,g])=>({key:JSON.parse(key),observations:g.map(view)})),allComparisons:rows.map(view)});
}
console.log(JSON.stringify({analysisOnly:true,sourceSha256:nativeHash,previousContextComparison:{contexts:528,changedOriginalDisplay:0,changedOriginalStr:0,scope:'ONLY this528-context overlap with the bound higher-count pair; not earlier-phase or Windows retroqualification'},tupleFields:['FIXED','D','originalWidth40','originalScale18','storedResultType','storedResultWidth','storedResultDecimals','storedResultBits'],results,interpretation:'Stored public ROUND(value,D) descriptors/payloads cannot determine original TRANSFORM display in these conflicting finite groups. The key intentionally omits original input bits. Assignment is part of this result route: no conclusion about inline ROUND metadata, TRANSFORM internal calls, other information, tolerance/cap/operation order or an accepted formatting rule.'},null,2));
