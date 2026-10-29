// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: observed count transitions, not a cap hypothesis.
'use strict';
const a=require('node:assert/strict');
function join(rows) {
 const contexts=new Map();
 for(const row of rows){
  a(Number.isInteger(row.index)&&row.index>0);a(['OFF','ON'].includes(row.fixed));
  a([15,16,17,18].includes(row.count));a.match(row.bits,/^[0-9A-F]{16}$/);
  const key=JSON.stringify([row.index,row.fixed]);
  if(!contexts.has(key))contexts.set(key,new Map());
  const context=contexts.get(key);a(!context.has(row.count));context.set(row.count,row);
 }
 const pairs=[];
 for(const context of contexts.values()){
  a.equal(context.size,4);const first=context.get(15);
  for(const row of context.values()){a.equal(row.bits,first.bits);a.equal(row.setting,first.setting);}
  for(const count of [15,16,17]){
   const low=context.get(count),high=context.get(count+1);
   const observedStep=BigInt(high.observedScaled)-10n*BigInt(low.observedScaled);
   const transitions={};
   for(const mode of ['nearestEven','nearestAway']){
    const nearestStep=BigInt(high.candidate[mode])-10n*BigInt(low.candidate[mode]);
    const residualStep=BigInt(high.residuals[mode].signedUnits)-10n*BigInt(low.residuals[mode].signedUnits);
    a.equal(observedStep,nearestStep+residualStep);
    transitions[mode]={nearestStep:String(nearestStep),residualStep:String(residualStep)};
   }
   pairs.push({index:first.index,fixed:first.fixed,bits:first.bits,setting:first.setting,
    lowCount:count,highCount:count+1,observedStep:String(observedStep),transitions,
    low,high});
  }
 }
 return pairs;
}
module.exports={join};
