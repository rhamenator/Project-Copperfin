// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: diagnostic residuals, no tolerance.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const {residual}=require('./exact-str-residual.cjs');
const root=path.join(__dirname,'exact-binary-str-captures1');
const bytes=fs.readFileSync(path.join(root,'analysis.out.gz'));
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const digest='f95a4192f2a4889b50f6bb9e9b3d238124583eb89e07516b2df2ed1ac3df106f';
a.equal(sha(bytes),digest);
const manifest=JSON.parse(fs.readFileSync(path.join(root,'manifest.json')));
const bound=manifest.filter(r=>r.name==='analysis.out.gz');a.equal(bound.length,1);
a.equal(bound[0].sha256,digest);a.equal(bound[0].bytes,bytes.length);
const raw=z.gunzipSync(bytes);a.equal(sha(raw),bound[0].decodedSha256);a.equal(raw.length,bound[0].decodedBytes);
const input=JSON.parse(raw);a.equal(input.rows.length,2112);
a.equal(input.sourceHash,'13fba5c3663d1d44f0cf514d481abfc66e16063e3e300d1c8e33502b10282c6c');
const modes=['nearestEven','nearestAway'],seen=new Set();
const rows=input.rows.map(r=>{
 const key=JSON.stringify([r.index,r.fixed,r.count]);a(!seen.has(key));seen.add(key);
 a.match(r.bits,/^[0-9A-F]{16}$/);a.equal(r.candidate.negative,(BigInt('0x'+r.bits)>>63n)!==0n);
 const deltas=Object.fromEntries(modes.map(mode=>{
  const delta=residual(r.observedScaled,r.candidate[mode],r.candidate.negative);
  a.equal(delta.direction!=='equal',r.mismatches[mode]);return [mode,delta];
 }));
 return {...r,residuals:deltas};
});
const groups=[];
for(const fixed of ['OFF','ON'])for(const count of [15,16,17,18])for(const mode of modes){
 const selected=rows.filter(r=>r.fixed===fixed&&r.count===count);a.equal(selected.length,264);
 const histogram=new Map();for(const r of selected){const units=r.residuals[mode].magnitudeUnits;histogram.set(units,(histogram.get(units)||0)+1);}
 groups.push({fixed,count,mode,contexts:selected.length,
  histogram:[...histogram].sort(([x],[y])=>BigInt(x)<BigInt(y)?-1:BigInt(x)>BigInt(y)?1:0).map(([magnitudeUnits,contexts])=>({magnitudeUnits,contexts}))});
}
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,inputCompressedSha256:digest,
 inputDecodedSha256:sha(raw),sourceHash:input.sourceHash,groups,rows,
 mismatches:rows.filter(r=>modes.some(m=>r.residuals[m].direction!=='equal')||r.zeroSignDifferent===true),
 limitation:'Exact observed-minus-nearest residuals in units 10^-count, oriented by raw payload sign. All rows retained; no tolerance, fitted repair or formatter acceptance.'},null,2));
