// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// AFTER capture, offline observations only; NOT formatter/expectation production.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const root=process.argv[2];a(root);const bytes=(d,n)=>{const p=path.join(d,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
const current=JSON.parse(bytes(root,'audit.json')),parentRoot=path.join(__dirname,'arithmetic-inline-captures1'),parent=JSON.parse(bytes(parentRoot,'audit.json'));
const sha=b=>c.createHash('sha256').update(b).digest('hex'),nativeHash='0b7950e0725e76d15742428b0b03614563ddcc15a1ac088a28f207b7788e7712',parentHash='1979626f4fec992924791da5cbc89ed3abef1583810690300fff1fd3cf95ea32';
a.equal(current.recordsPerRun,1584);a.equal(current.observations.length,1584);for(const k of ['documentedFieldsRepeatEqual','originalParentAndTypedControls','exactRuntimeIdentity','guardedBufferAndGrammar','rawByteEqual'])a.equal(current[k],true);
for(const n of ['first','repeat'])for(const [report,d,h]of [[current,root,nativeHash],[parent,parentRoot,parentHash]]){a.equal(report[n+'Sha256'],h);a.equal(sha(bytes(d,n+'-results.txt')),h);}
const key=r=>JSON.stringify([r.index,r.fixed]),previous=new Map(parent.observations.map(r=>[key(r),r]));a.equal(previous.size,528);
const contexts=new Map();for(const r of current.observations){const old=previous.get(key(r));a(old);for(const n of ['bits','setting','originalDisplayHex','originalStrHex'])a.equal(r[n],old[n]);a.deepEqual(r.crt,old.crt);a([15,16,17].includes(r.count));const rows=contexts.get(key(r))||[];a(!rows.some(x=>x.count===r.count));rows.push(r);contexts.set(key(r),rows);}
a.equal(contexts.size,528);for(const rows of contexts.values()){rows.sort((x,y)=>x.count-y.count);a.deepEqual(rows.map(r=>r.count),[15,16,17]);for(const r of rows)a.deepEqual(r.originalMeta,rows[0].originalMeta);}
const ascii=h=>Buffer.from(h,'hex').toString('ascii');
// Finite bounded observed strings only. Normalize exact decimal coefficients
// and exponents without binary Number(text), rounding, powers or tolerance.
// Equality here ignores whitespace/padding and signed-zero spelling ONLY as
// a separately labelled value metric; literal text equality is also reported.
function decimal(s){a(s.length<=127);const m=s.trim().match(/^([+-]?)(?:(\d+)(?:\.(\d*))?|\.(\d+))(?:[eE]([+-]?\d+))?$/);a(m);const fraction=m[3]===undefined?(m[4]||''):m[3];let n=BigInt((m[2]||'0')+fraction),e=BigInt(m[5]||'0')-BigInt(fraction.length);if(m[1]==='-')n=-n;if(n===0n)e=0n;else while(n%10n===0n){n/=10n;++e;}return [String(n),String(e)];}
const comparisons=[];for(const count of [15,16,17])for(const fixed of ['OFF','ON']){
 const rows=current.observations.filter(r=>r.count===count&&r.fixed===fixed),textMismatches=[],valueMismatches=[];a.equal(rows.length,264);
 for(const r of rows){const text=ascii(r.gcvt.text),display=ascii(r.originalDisplayHex),observation={index:r.index,setting:r.setting,bits:r.bits,gcvtText:text,originalDisplay:display};if(r.gcvt.text!==r.originalDisplayHex)textMismatches.push(observation);if(JSON.stringify(decimal(text))!==JSON.stringify(decimal(display)))valueMismatches.push(observation);}
 comparisons.push({count,fixed,contexts:264,literalTextDifferent:textMismatches.length,exactDecimalValueDifferent:valueMismatches.length,textMismatches,valueMismatches});
}
const tuples=[];for(const mode of [15,16,17,'ALL'])for(const fixed of ['OFF','ON']){
 const groups=new Map();for(const rows of contexts.values()){const r=rows[0];if(r.fixed!==fixed)continue;const selected=mode==='ALL'?rows:rows.filter(x=>x.count===mode);const tuple=[fixed,r.setting,r.originalMeta.type,r.originalMeta.width,r.originalMeta.decimals,...selected.map(x=>[x.count,x.gcvt.text])],k=JSON.stringify(tuple),g=groups.get(k)||[];g.push({index:r.index,bits:r.bits,displayHex:r.originalDisplayHex,strHex:r.originalStrHex});groups.set(k,g);}
 const conflicts=[...groups].filter(([,rows])=>new Set(rows.map(r=>r.displayHex)).size>1);tuples.push({counts:mode,fixed,contexts:264,distinctKeys:groups.size,conflictingKeys:conflicts.length,counterexamples:conflicts.map(([k,observations])=>({tuple:JSON.parse(k),observations}))});
}
console.log(JSON.stringify({analysisOnly:true,sourceSha256:nativeHash,previousBoundParentComparison:{contexts:528,originalDisplayStrAndCrtControlsUnchanged:true,scope:'ONLY this bound528-context overlap, not Windows or old-runtime retroqualification'},comparisons,tuples,interpretation:'Literal and exact-decimal-value mismatches measure public _gcvt output versus original TRANSFORM, not an accepted formatting route. Tuple conflicts reject sufficiency only where observed; a unique tuple including17-digit text is NOT a formatter/precision rule or VFP internal-call proof. No rounding, fitted tolerance, sign/padding equivalence or production implementation follows.'},null,2));
