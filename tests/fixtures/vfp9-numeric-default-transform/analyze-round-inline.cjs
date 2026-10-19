// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Offline route comparison AFTER capture; no formatter or internal-call attribution.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const root=process.argv[2];a(root);const bytes=(dir,n)=>{const p=path.join(dir,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
const current=JSON.parse(bytes(root,'audit.json')),parentRoot=path.join(__dirname,'round-result-captures1'),parent=JSON.parse(bytes(parentRoot,'audit.json'));
a.equal(current.recordsPerRun,528);a.equal(current.observations.length,528);
for(const k of ['documentedFieldsRepeatEqual','typedResultAndControls','inlineResultAndStoredParentControls','exactRuntimeIdentity'])a.equal(current[k],true);
const hash=b=>c.createHash('sha256').update(b).digest('hex'),nativeHash='254b46dadbb71fa51b10e8439d51785b2c2aa3eb871c8c948fd3b8bdd3da4873',parentHash='0be810176af4c28eca3864eae70dc8adbfe409986ca227cdde8ccaa7c8013607';
for(const n of ['first','repeat']){a.equal(current[n+'Sha256'],nativeHash);a.equal(hash(bytes(root,n+'-results.txt')),nativeHash);a.equal(parent[n+'Sha256'],parentHash);a.equal(hash(bytes(parentRoot,n+'-results.txt')),parentHash);}
const old=new Map(parent.observations.map(r=>[JSON.stringify([r.index,r.fixed]),r]));a.equal(old.size,528);
const seen=new Set();for(const r of current.observations){const key=JSON.stringify([r.index,r.fixed]);a(!seen.has(key));seen.add(key);const p=old.get(key);a(p);
 for(const k of ['bits','setting','originalDisplayHex','originalStrHex','resultBits','resultDisplayHex','resultStrHex'])a.equal(r[k],p[k]);a.deepEqual(r.resultMeta,p.resultMeta);a.deepEqual(r.crt,p.crt);}
const results=[];for(const fixed of ['OFF','ON']){
 const rows=current.observations.filter(r=>r.fixed===fixed);a.equal(rows.length,264);const fields={};
 for(const r of rows)for(const k of new Set([...Object.keys(r.inlineMeta),...Object.keys(r.resultMeta)]))if(r.inlineMeta[k]!==r.resultMeta[k])fields[k]=(fields[k]||0)+1;
 const routeGroups=[];for(const route of ['inline','stored']){const groups=new Map();
  for(const r of rows){const m=route==='inline'?r.inlineMeta:r.resultMeta,bits=route==='inline'?r.inlineBits:r.resultBits,key=JSON.stringify([r.fixed,r.setting,40,18,m.type,m.width,m.decimals,bits]),g=groups.get(key)||[];g.push(r);groups.set(key,g);}
  const conflicts=[...groups].filter(([,g])=>new Set(g.map(r=>r.originalDisplayHex)).size>1);
  routeGroups.push({route,distinctTuples:groups.size,conflictingTuples:conflicts.length,conflictingContexts:conflicts.reduce((n,[,g])=>n+g.length,0),counterexamples:conflicts.map(([key,observations])=>({key:JSON.parse(key),observations}))});
 }
 results.push({fixed,contexts:264,inlineVsStored:{bitsDifferent:rows.filter(r=>r.inlineBits!==r.resultBits).length,metaFieldsDifferent:fields,displayDifferent:rows.filter(r=>r.inlineDisplayHex!==r.resultDisplayHex).length,strDifferent:rows.filter(r=>r.inlineStrHex!==r.resultStrHex).length},inlineVsOriginal:{bitsDifferent:rows.filter(r=>r.inlineBits!==r.bits).length,displayDifferent:rows.filter(r=>r.inlineDisplayHex!==r.originalDisplayHex).length,strDifferent:rows.filter(r=>r.inlineStrHex!==r.originalStrHex).length},routeGroups,allComparisons:rows});
}
console.log(JSON.stringify({analysisOnly:true,sourceSha256:nativeHash,previousContextComparison:{contexts:528,originalAndStoredControlsUnchanged:true,scope:'ONLY this528-context overlap with the bound stored-result pair; not earlier-phase or Windows retroqualification'},tupleFields:['FIXED','D','originalWidth40','originalScale18','resultType','resultWidth','resultDecimals','resultBits'],results,interpretation:'Route differences and tuple collisions are finite observations only. Equal inline/stored payloads/text do not imply metadata equality or universal assignment behavior; conflicting result tuples cannot alone determine original display. No accepted formatter, internal-call attribution, tolerance/cap/order follows.'},null,2));
