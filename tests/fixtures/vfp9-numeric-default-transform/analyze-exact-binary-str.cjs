// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: hypotheses only, not expected text.
'use strict';
const a=require('node:assert/strict'),path=require('node:path');
const {load}=require('./load-str-crt-coverage.cjs');
const {candidates}=require('./exact-binary-decimal-diagnostic.cjs');
const hash='13fba5c3663d1d44f0cf514d481abfc66e16063e3e300d1c8e33502b10282c6c';
const source=load(path.join(__dirname,'str-middle-captures1'),hash,528);
for(const flag of ['originalAndNativeParentControls','parentStrBeforeAfterExact','inlineMetadataAndBRSExact','publicStrTypesAndWidths'])a.equal(source[flag],true);
const rows=[],seen=new Set(),modes=['truncated','nearestEven','nearestAway'];
for(const r of source.observations)for(const count of [15,16,17,18]) {
 const key=JSON.stringify([r.index,r.fixed,count]);a(!seen.has(key));seen.add(key);
 const strHex=r['str'+count+'Hex'],text=Buffer.from(strHex,'hex').toString('ascii');
 a.equal(text.length,40);const m=text.trim().match(/^(-?)(\d+)\.(\d+)$/);a(m);a.equal(m[3].length,count);
 const observed=BigInt(m[2]+m[3])*(m[1]? -1n:1n),candidate=candidates(r.bits,count);
 const mismatches=Object.fromEntries(modes.map(mode=>[mode,candidate[mode]!==String(observed)]));
 rows.push({index:r.index,fixed:r.fixed,bits:r.bits,setting:r.setting,count,strHex,text,
  observedScaled:String(observed),observedNegative:m[1]==='-',candidate,mismatches,
  zeroSignDifferent:observed===0n?candidate.negative!==(m[1]==='-'):null});
}
a.equal(rows.length,2112);
const groups=[];for(const fixed of ['OFF','ON'])for(const count of [15,16,17,18]){
 const selected=rows.filter(r=>r.fixed===fixed&&r.count===count);
 groups.push({fixed,count,contexts:selected.length,...Object.fromEntries(modes.map(mode=>[mode,selected.filter(r=>r.mismatches[mode]).length])),zeroSignDifferent:selected.filter(r=>r.zeroSignDifferent===true).length});
}
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,sourceHash:hash,groups,rows,
 mismatches:rows.filter(r=>Object.values(r.mismatches).some(Boolean)||r.zeroSignDifferent===true),
 limitation:'Exact binary rounding hypotheses versus finite public STR values; no internal-call, general conversion or formatter acceptance. Raw sign and all mismatches retained.'},null,2));
