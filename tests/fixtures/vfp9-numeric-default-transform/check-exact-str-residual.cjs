// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict');
const report=JSON.parse(fs.readFileSync(process.argv[2]));
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,'exact-binary-str-captures1/analysis.out.gz'))));
a.equal(report.rows.length,2112);a.equal(parent.rows.length,2112);
const modes=['nearestEven','nearestAway'];
for(const [i,row]of report.rows.entries()){
 const {residuals,...original}=row;a.deepEqual(original,parent.rows[i]);
 for(const mode of modes){
  const delta=BigInt(row.observedScaled)-BigInt(row.candidate[mode]);
  const oriented=row.bits[0]>='8'?-delta:delta;
  a.deepEqual(residuals[mode],{signedUnits:String(delta),magnitudeUnits:String(oriented),direction:oriented===0n?'equal':oriented<0n?'toward':'away'});
 }
}
const expected=[];
for(const fixed of ['OFF','ON'])for(const count of [15,16,17,18])for(const mode of modes){
 const selected=report.rows.filter(r=>r.fixed===fixed&&r.count===count);a.equal(selected.length,264);
 const units=selected.map(r=>r.residuals[mode].magnitudeUnits);
 const distinct=[...new Set(units)].sort((x,y)=>BigInt(x)<BigInt(y)?-1:BigInt(x)>BigInt(y)?1:0);
 expected.push({fixed,count,mode,contexts:264,histogram:distinct.map(magnitudeUnits=>({magnitudeUnits,contexts:units.filter(x=>x===magnitudeUnits).length}))});
}
a.deepEqual(report.groups,expected);
a.deepEqual(report.mismatches,report.rows.filter(r=>modes.some(m=>r.residuals[m].signedUnits!=='0')||r.zeroSignDifferent===true));
console.log(JSON.stringify({independentReadback:true,rows:2112,groups:expected,mismatchRows:report.mismatches.length,nativeInvoked:false},null,2));
