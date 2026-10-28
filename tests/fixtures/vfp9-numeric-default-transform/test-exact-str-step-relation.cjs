// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{relation}=require('./exact-str-step-relation.cjs');
let checks=0;
for(const [o,n,r,category]of [['0','0','0','both-zero'],['0','2','-2','cancelled'],
 ['2','0','2','residual-only'],['-2','0','-2','residual-only'],['2','2','0','equal-step'],
 ['-2','-2','0','equal-step'],['2','1','1','mixed'],['-2','3','-5','mixed'],
 ['100000000000000000001','100000000000000000000','1','mixed']]){
 a.equal(relation(o,n,r).category,category);++checks;
}
for(const args of [['1','0','0'],['-0','0','0'],['1.0','1','0'],['1e2','100','0']]){a.throws(()=>relation(...args));++checks;}
console.log(JSON.stringify({checks,passed:true,nativeInvoked:false}));
