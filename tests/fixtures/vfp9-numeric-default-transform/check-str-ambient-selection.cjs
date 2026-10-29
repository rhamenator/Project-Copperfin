// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict');
const result=JSON.parse(fs.readFileSync(process.argv[2]));
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,'str-middle-captures1/audit.json.gz'))));
a.deepEqual(result.inputs.map(r=>r.index),[184,185,186,190,191,192]);
a.deepEqual(result.ambientSettings,[0,13,18]);a.deepEqual(result.counts,[15,16,17,18]);a.deepEqual(result.fixedModes,['OFF','ON']);
a.equal(result.observations,6*3*4*2);a.equal(result.nativeInvoked,false);
for(const input of result.inputs){
 a.equal(input.originalSetting,15);
 for(const fixed of ['OFF','ON']){
  const rows=parent.observations.filter(r=>r.index===input.index&&r.fixed===fixed);a.equal(rows.length,1);
  a.deepEqual(input.controls[fixed],rows[0]);a.equal(input.bits,rows[0].bits);a.equal(rows[0].setting,15);
 }
}
for(let i=0;i<3;++i){const positive=BigInt('0x'+result.inputs[i].bits),negative=BigInt('0x'+result.inputs[i+3].bits);
 a.equal(positive>>63n,0n);a.equal(negative>>63n,1n);a.equal(negative-positive,2n**63n);}
console.log(JSON.stringify({independentReadback:true,inputs:6,originalControls:12,plannedObservations:144,nativeInvoked:false}));
