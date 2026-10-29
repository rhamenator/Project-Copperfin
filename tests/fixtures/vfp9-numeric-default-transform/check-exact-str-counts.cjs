// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict');
const report=JSON.parse(fs.readFileSync(process.argv[2]));
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,'exact-str-residual-captures1/analysis.out.gz'))));
a.equal(parent.rows.length,2112);a.equal(report.pairs.length,1584);
const seen=new Set();
for(const pair of report.pairs){
 const key=JSON.stringify([pair.index,pair.fixed,pair.lowCount]);a(!seen.has(key));seen.add(key);
 a([15,16,17].includes(pair.lowCount));a.equal(pair.highCount,pair.lowCount+1);
 const low=parent.rows.filter(r=>r.index===pair.index&&r.fixed===pair.fixed&&r.count===pair.lowCount);
 const high=parent.rows.filter(r=>r.index===pair.index&&r.fixed===pair.fixed&&r.count===pair.highCount);
 a.equal(low.length,1);a.equal(high.length,1);a.deepEqual(pair.low,low[0]);a.deepEqual(pair.high,high[0]);
 for(const endpoint of [pair.low,pair.high]){a.equal(endpoint.bits,pair.bits);a.equal(endpoint.setting,pair.setting);}
 const observed=BigInt(pair.high.observedScaled)-10n*BigInt(pair.low.observedScaled);a.equal(pair.observedStep,String(observed));
 for(const mode of ['nearestEven','nearestAway']){
  const nearest=BigInt(pair.high.candidate[mode])-10n*BigInt(pair.low.candidate[mode]);
  const residual=BigInt(pair.high.residuals[mode].signedUnits)-10n*BigInt(pair.low.residuals[mode].signedUnits);
  a.deepEqual(pair.transitions[mode],{nearestStep:String(nearest),residualStep:String(residual)});a.equal(observed,nearest+residual);
 }
}
const groups=[];
for(const fixed of ['OFF','ON'])for(const lowCount of [15,16,17]){
 const pairs=report.pairs.filter(r=>r.fixed===fixed&&r.lowCount===lowCount);a.equal(pairs.length,264);
 const values=pairs.map(r=>r.observedStep),unique=[...new Set(values)].sort((x,y)=>BigInt(x)<BigInt(y)?-1:BigInt(x)>BigInt(y)?1:0);
 groups.push({fixed,lowCount,highCount:lowCount+1,contexts:264,observedStepHistogram:unique.map(units=>({units,contexts:values.filter(x=>x===units).length}))});
}
a.deepEqual(report.groups,groups);a.deepEqual(report.nonzeroObservedSteps,report.pairs.filter(r=>r.observedStep!=='0'));
console.log(JSON.stringify({independentReadback:true,pairs:1584,groups,nonzeroPairs:report.nonzeroObservedSteps.length,nativeInvoked:false},null,2));
