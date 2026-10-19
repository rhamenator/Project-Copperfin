// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Offline rejection/route comparison AFTER capture, never a formatter or expectation producer.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const root=process.argv[2];a(root);const bytes=(dir,n)=>{const p=path.join(dir,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));},read=dir=>JSON.parse(bytes(dir,'audit.json'));
const current=read(root),parentRoot=path.join(__dirname,'round-inline-captures1'),parent=read(parentRoot),legacyRoot=path.join(__dirname,'rounding-arithmetic-captures'),legacy=read(legacyRoot);
a.equal(current.recordsPerRun,528);a.equal(current.observations.length,528);for(const k of ['documentedFieldsRepeatEqual','typedResultAndControls','inlineArithmeticAndStoredParentControls','exactRuntimeIdentity'])a.equal(current[k],true);
const hash=b=>c.createHash('sha256').update(b).digest('hex'),nativeHash='1979626f4fec992924791da5cbc89ed3abef1583810690300fff1fd3cf95ea32',parentHash='254b46dadbb71fa51b10e8439d51785b2c2aa3eb871c8c948fd3b8bdd3da4873',legacyHash='2917ff9167e4a2752520614776580d08dda8fa4ba1520615154912bd937bd1c9';
for(const n of ['first','repeat'])for(const [report,dir,digest]of [[current,root,nativeHash],[parent,parentRoot,parentHash],[legacy,legacyRoot,legacyHash]]){a.equal(report[n+'Sha256'],digest);a.equal(hash(bytes(dir,n+'-results.txt')),digest);}
const key=r=>JSON.stringify([r.index,r.fixed]),parents=new Map(parent.observations.map(r=>[key(r),r])),old=new Map(legacy.observations.map(r=>[key(r),r]));a.equal(parents.size,528);a.equal(old.size,528);
const seen=new Set(),stageDifferences=[],legacyOriginalDifferences=[],mismatches=[],allMagnitudeComparisons=[];let compared=0;
const number=bits=>Buffer.from(bits,'hex').readDoubleBE();
for(const r of current.observations){const k=key(r);a(!seen.has(k));seen.add(k);const p=parents.get(k),o=old.get(k);a(p);a(o);
 for(const n of ['bits','setting','originalDisplayHex','originalStrHex','resultBits','resultDisplayHex','resultStrHex'])a.equal(r[n],p[n]);a.deepEqual(r.resultMeta,p.resultMeta);a.deepEqual(r.crt,p.crt);
 a.equal(r.bits,o.bits);a.equal(r.setting,o.setting);
 for(const [name,stage]of Object.entries(r.arithmetic))if(stage.bits!==o.arithmetic[name])stageDifferences.push({index:r.index,fixed:r.fixed,setting:r.setting,name,inlineBits:stage.bits,legacyScalarBits:o.arithmetic[name]});
 if(r.originalDisplayHex!==o.displayHex||r.originalStrHex!==o.strHex)legacyOriginalDifferences.push({index:r.index,fixed:r.fixed,originalDisplayHex:r.originalDisplayHex,legacyDisplayHex:o.displayHex,originalStrHex:r.originalStrHex,legacyStrHex:o.strHex});
 if(r.fixed!=='ON')continue;
 const q=number(r.arithmetic.effectiveCount.bits),quantized=number(r.arithmetic.quantized.bits);a(Number.isSafeInteger(q)&&q>=0&&q<=18);a(Number.isSafeInteger(quantized)&&quantized>=0);
 const text=Buffer.from(r.originalDisplayHex,'hex').toString('ascii');a.match(text,/^-?[0-9]+(?:\.[0-9]+)?$/);const [whole,fraction='']=text.replace(/^-/,'').split('.');
 // Cross-multiply exact decimal integer units; never Number(displayText).
 const actual=BigInt(whole+fraction)*10n**BigInt(q),candidate=BigInt(quantized)*10n**BigInt(fraction.length),equal=actual===candidate;
 const observation={index:r.index,setting:r.setting,bits:r.bits,observedHex:r.originalDisplayHex,observedStrHex:r.originalStrHex,observedCount:q,quantizedBits:r.arithmetic.quantized.bits,quantizedInteger:String(quantized),equal,arithmetic:r.arithmetic};
 ++compared;allMagnitudeComparisons.push(observation);if(!equal)mismatches.push(observation);
}
a.equal(seen.size,528);a.equal(compared,264);
console.log(JSON.stringify({analysisOnly:true,sourceSha256:nativeHash,previousBoundParentComparison:{contexts:528,originalAndStoredControlsUnchanged:true},legacyOverlap:{contexts:528,stageBitsDifferent:stageDifferences.length,stageDifferences,originalDisplayOrStrDifferent:legacyOriginalDifferences.length,legacyOriginalDifferences,limitation:'Only finite raw payload/text overlap with the earlier scalar capture. Its old runtime provenance is NOT retroqualified by this new native-bound capture.'},comparedOnMagnitudes:compared,publicInlineGraphRejected:mismatches.length>0,mismatches,allMagnitudeComparisons,interpretation:'This observed inline route is a discriminator for an already rejected graph, not an implementation. Exact decimal magnitude comparison does not establish sign/padding/negative-zero/text equivalence. No internal API/precision/cap/tolerance/order or accepted formatter follows.'},null,2));
