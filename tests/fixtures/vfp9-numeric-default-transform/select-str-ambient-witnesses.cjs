// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: actual native stimulus selection only.
'use strict';
const a=require('node:assert/strict'),path=require('node:path');
const {load}=require('./load-str-crt-coverage.cjs');
function select(report){
 const result=[];
 for(const index of [184,185,186,190,191,192]){
  const rows=report.observations.filter(r=>r.index===index);a.equal(rows.length,2);
  const off=rows.find(r=>r.fixed==='OFF'),on=rows.find(r=>r.fixed==='ON');a(off&&on);
  a.equal(off.bits,on.bits);a.equal(off.setting,15);a.equal(on.setting,15);
  const suffix=String(index<190?index:index-6);const bits=(index<190?'3':'B')+'CC1FF130772E'+suffix;
  a.equal(off.bits,bits);
  for(const row of rows){a.equal(row.originalMeta.real,bits);a.equal(row.originalMeta.type,'4E');
   a.equal(row.originalMeta.width,'0028');a.equal(row.originalMeta.decimals,'00000012');}
  result.push({index,bits,originalSetting:15,controls:{OFF:off,ON:on}});
 }
 for(let i=0;i<3;++i)a.equal(BigInt('0x'+result[i].bits)^(1n<<63n),BigInt('0x'+result[i+3].bits));
 return {sourceOnly:true,nativeInvoked:false,inputs:result,ambientSettings:[0,13,18],counts:[15,16,17,18],fixedModes:['OFF','ON'],observations:144,
  limitation:'Native payload/control selection, not generated output or rounding rule. Parent reader replay remains required.'};
}
module.exports={select};
if(require.main===module){
 const hash='13fba5c3663d1d44f0cf514d481abfc66e16063e3e300d1c8e33502b10282c6c';
 const report=load(path.join(__dirname,'str-middle-captures1'),hash,528);
 for(const flag of ['originalAndNativeParentControls','parentStrBeforeAfterExact','inlineMetadataAndBRSExact','publicStrTypesAndWidths'])a.equal(report[flag],true);
 console.log(JSON.stringify({...select(report),parentRawSha256:hash},null,2));
}
