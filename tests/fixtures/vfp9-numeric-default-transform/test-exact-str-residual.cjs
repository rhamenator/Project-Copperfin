// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{residual}=require('./exact-str-residual.cjs');
let checks=0;
for(const [o,n,negative,signed,magnitude,direction]of [
 ['12','10',false,'2','2','away'],['8','10',false,'-2','-2','toward'],
 ['-12','-10',true,'-2','2','away'],['-8','-10',true,'2','-2','toward'],
 ['0','0',true,'0','0','equal'],['0','-1',true,'1','-1','toward'],
 ['0','1',false,'-1','-1','toward'],
 ['100000000000000000001','100000000000000000000',false,'1','1','away']]){
 a.deepEqual(residual(o,n,negative),{signedUnits:signed,magnitudeUnits:magnitude,direction});++checks;
}
for(const value of ['-0','01','1.0','1e2','',null]){a.throws(()=>residual(value,'0',false));++checks;}
a.throws(()=>residual('0','0',0));++checks;
console.log(JSON.stringify({checks,passed:true,nativeInvoked:false}));
