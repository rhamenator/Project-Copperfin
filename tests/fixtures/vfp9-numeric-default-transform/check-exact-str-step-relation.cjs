// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict');
const report=JSON.parse(fs.readFileSync(process.argv[2]));
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,'exact-str-counts-captures1/analysis.out.gz'))));
a.equal(report.pairs.length,1584);a.equal(parent.pairs.length,1584);
const modes=['nearestEven','nearestAway'];
for(const [i,pair]of report.pairs.entries()){
 const {relations,...original}=pair;a.deepEqual(original,parent.pairs[i]);
 for(const mode of modes){
  const o=BigInt(pair.high.observedScaled)-10n*BigInt(pair.low.observedScaled);
  const n=BigInt(pair.high.candidate[mode])-10n*BigInt(pair.low.candidate[mode]);
  const r=BigInt(pair.high.residuals[mode].signedUnits)-10n*BigInt(pair.low.residuals[mode].signedUnits);a.equal(o,n+r);
  let category;if(o===n)category=o===0n?'both-zero':'equal-step';else if(o===0n)category='cancelled';else if(n===0n)category='residual-only';else category='mixed';
  a.deepEqual(relations[mode],{observedNonzero:o!==0n,nearestNonzero:n!==0n,residualNonzero:r!==0n,equalSteps:o===n,category});
 }
}
const groups=[];for(const fixed of ['OFF','ON'])for(const lowCount of [15,16,17])for(const mode of modes){
 const selected=report.pairs.filter(r=>r.fixed===fixed&&r.lowCount===lowCount);a.equal(selected.length,264);
 groups.push({fixed,lowCount,mode,contexts:264,categories:Object.fromEntries(['both-zero','cancelled','residual-only','equal-step','mixed'].map(category=>[category,selected.filter(r=>r.relations[mode].category===category).length]))});
}
a.deepEqual(report.groups,groups);a.deepEqual(report.changing,report.pairs.filter(r=>r.observedStep!=='0'));
console.log(JSON.stringify({independentReadback:true,pairs:1584,changing:report.changing.length,groups,nativeInvoked:false},null,2));
