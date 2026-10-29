// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{select}=require('./select-str-ambient-witnesses.cjs');
function fixture(){return {observations:[184,185,186,190,191,192].flatMap(index=>['OFF','ON'].map(fixed=>{
 const bits=(index<190?'3':'B')+'CC1FF130772E'+String(index<190?index:index-6);
 return {index,fixed,bits,setting:15,originalMeta:{type:'4E',width:'0028',decimals:'00000012',real:bits}};
}))};}
let checks=0;
const original=fixture(),copy=structuredClone(original),result=select(original);
a.equal(result.inputs.length,6);a.equal(result.observations,144);a.deepEqual(original,copy);++checks;
for(const mutate of [r=>r.observations.pop(),r=>r.observations.push(structuredClone(r.observations[0])),
 r=>r.observations[0].fixed='ON',r=>r.observations[0].setting=13,
 r=>r.observations[0].bits='3CC1FF130772E183',r=>r.observations[0].originalMeta.real='0000000000000000',
 r=>r.observations[0].originalMeta.type='49',r=>r.observations[0].originalMeta.width='0020',
 r=>r.observations[0].originalMeta.decimals='0000000F']){
 const rows=fixture();mutate(rows);a.throws(()=>select(rows));++checks;
}
console.log(JSON.stringify({checks,passed:true,nativeInvoked:false}));
