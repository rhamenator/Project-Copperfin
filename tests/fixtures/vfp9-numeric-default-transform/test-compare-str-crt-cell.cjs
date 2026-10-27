// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: synthetic metric checks, not native expected digits.
'use strict';
const a=require('node:assert/strict'),{decimal,compare}=require('./compare-str-crt-cell.cjs');
const crt=(digits,point,negative=false)=>({digits:Buffer.from(digits).toString('hex').toUpperCase(),
  decimal:(BigInt(point)&0xFFFFFFFFn).toString(16).toUpperCase().padStart(8,'0'),sign:negative?'00000001':'00000000'});
let tests=0;
for(const [text,digits,point,negative] of [[' 1.00','100',1,false],['-12.34','1234',2,true],
  ['5e-16','50',-15,false],['-0','000',1,true],['0.000','0',-18,false]]) {
  const r=compare(text,crt(digits,point,negative));a.equal(r.exactValueDifferent,false);
  a.equal(r.strText,text);a.equal(r.crtDigits,digits);++tests;
}
let r=compare('-0.00',crt('0',1));a.equal(r.exactValueDifferent,false);a.equal(r.zeroSignDifferent,true);++tests;
r=compare('0',crt('',-15));a.equal(r.crtValue,null);a.equal(r.exactValueDifferent,null);++tests;
a.equal(compare('1.0000000000000001',crt('10000000000000000',1)).exactValueDifferent,true);++tests;
a.equal(compare('1e-18',crt('1',-16)).exactValueDifferent,true);++tests;
for(const text of ['NaN','Infinity','','1,2','1x','.','-']) {a.throws(()=>decimal(text));++tests;}
for(const delta of [{digits:'GG'},{digits:'2D31'},{decimal:'FFFFFFFFF'},{sign:'x'}]) {
  a.throws(()=>compare('1',{...crt('1',1),...delta}));++tests;
}
a.deepEqual(decimal('-0e99'),{coefficient:'0',exponent:'0',negative:true,zero:true});++tests;
console.log(JSON.stringify({syntheticCellMetricTests:tests,nativeInvoked:false}));
