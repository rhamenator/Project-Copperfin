// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: observed changing-context selection only.
'use strict';
const a=require('node:assert/strict');
function classify(pairs){
 const contexts=new Map();
 for(const pair of pairs){
  a([15,16,17].includes(pair.lowCount));a.equal(pair.highCount,pair.lowCount+1);
  const key=JSON.stringify([pair.index,pair.fixed]);
  if(!contexts.has(key))contexts.set(key,new Map());
  const context=contexts.get(key);a(!context.has(pair.lowCount));context.set(pair.lowCount,pair);
 }
 const rows=[];
 for(const context of contexts.values()){
  a.equal(context.size,3);const ordered=[15,16,17].map(n=>context.get(n)),first=ordered[0];
  for(const pair of ordered){a.equal(pair.bits,first.bits);a.equal(pair.setting,first.setting);}
  for(let i=1;i<3;++i)a.deepEqual(ordered[i-1].high,ordered[i].low);
  const changed=ordered.filter(p=>BigInt(p.observedStep)!==0n);
  rows.push({index:first.index,fixed:first.fixed,bits:first.bits,setting:first.setting,
   changedSteps:changed.length,changing:changed.length!==0,pairs:ordered});
 }
 return {contexts:rows,changing:rows.filter(r=>r.changing),unchanged:rows.filter(r=>!r.changing)};
}
module.exports={classify};
