// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: synthetic cross-source rejection tests.
'use strict';
const a=require('node:assert/strict'),{qualifyPrecision}=require('./load-str-crt-coverage.cjs');
const runtime=Object.fromEntries(['module','fileSample','fileSize','fileSha256','machine','magic',
  'stamp','imageSize','checksum','cw0','cw1'].map(k=>[k,'synthetic-'+k]));
const s={index:1,fixed:'OFF',bits:'3FF0000000000000',setting:15,
  originalDisplayHex:'31',originalStrHex:'2031',crt:runtime};
const p={index:1,fixed:'OFF',bits:s.bits,argumentBits:s.bits,setting:15,
  route:'ORIGINAL',kind:'F',count:16,displayHex:'31',strHex:'2031',crt:{...runtime}};
let tests=0;
qualifyPrecision([s],[p]);++tests;
for(const delta of [{index:2},{fixed:'ON'},{bits:'4000000000000000'},
  {argumentBits:'4000000000000000'},{setting:14},{route:'SHIFTED'},{kind:'E'},
  {count:15},{count:19},{displayHex:'32'},{strHex:'2032'}]) {
  a.throws(()=>qualifyPrecision([s],[{...p,...delta}]));++tests;
}
for(const field of Object.keys(runtime)) {
  a.throws(()=>qualifyPrecision([s],[{...p,crt:{...runtime,[field]:'changed'}}]));++tests;
}
a.throws(()=>qualifyPrecision([s,s],[p]));++tests;
console.log(JSON.stringify({syntheticPrecisionJoinTests:tests,nativeInvoked:false}));
