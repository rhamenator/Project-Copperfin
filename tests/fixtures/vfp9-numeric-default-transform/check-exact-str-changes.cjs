// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict');
const report=JSON.parse(fs.readFileSync(process.argv[2]));
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,'exact-str-counts-captures1/analysis.out.gz'))));
a.equal(report.contexts.length,528);a.equal(parent.pairs.length,1584);const seen=new Set();
for(const context of report.contexts){
 const key=JSON.stringify([context.index,context.fixed]);a(!seen.has(key));seen.add(key);
 const pairs=parent.pairs.filter(p=>p.index===context.index&&p.fixed===context.fixed).sort((x,y)=>x.lowCount-y.lowCount);
 a.equal(pairs.length,3);a.deepEqual(pairs.map(p=>p.lowCount),[15,16,17]);a.deepEqual(context.pairs,pairs);
 for(const pair of pairs){a.equal(pair.bits,context.bits);a.equal(pair.setting,context.setting);}
 const changed=pairs.filter(p=>BigInt(p.high.observedScaled)!==10n*BigInt(p.low.observedScaled)).length;
 a.equal(context.changedSteps,changed);a.equal(context.changing,changed!==0);
}
a.deepEqual(report.changing,report.contexts.filter(r=>r.changedSteps>0));
a.deepEqual(report.unchanged,report.contexts.filter(r=>r.changedSteps===0));
const groups=[];for(const fixed of ['OFF','ON'])for(const changedSteps of [0,1,2,3]){
 const selected=report.contexts.filter(r=>r.fixed===fixed&&r.changedSteps===changedSteps);
 groups.push({fixed,changedSteps,contexts:selected.length,indices:selected.map(r=>r.index)});
}
a.deepEqual(report.groups,groups);
console.log(JSON.stringify({independentReadback:true,contexts:528,changing:report.changing.length,unchanged:report.unchanged.length,groups,nativeInvoked:false},null,2));
