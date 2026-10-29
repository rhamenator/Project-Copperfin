// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: exact diagnostic metrics, not a formatter.
'use strict';
const a=require('node:assert/strict');
function normalized(n,e,negative) {
  if(n===0n) return {coefficient:'0',exponent:'0',negative,zero:true};
  while(n%10n===0n){n/=10n;++e;}
  return {coefficient:String(negative?-n:n),exponent:String(e),negative,zero:false};
}
function decimal(text) {
  a.equal(typeof text,'string');a(text.length<=127);
  const m=text.trim().match(/^([+-]?)(\d+)(?:\.(\d*))?(?:[eE]([+-]?\d+))?$/);a(m);
  const fraction=m[3]||'';
  return normalized(BigInt(m[2]+fraction),BigInt(m[4]||'0')-BigInt(fraction.length),m[1]==='-');
}
function compare(text,crt) {
  a.match(crt.digits,/^(?:[0-9A-F]{2})*$/);a.match(crt.decimal,/^[0-9A-F]{8}$/);
  a.match(crt.sign,/^[0-9A-F]{8}$/);
  const digits=Buffer.from(crt.digits,'hex').toString('ascii');a.match(digits,/^[0-9]*$/);
  const observed=decimal(text),negative=BigInt('0x'+crt.sign)!==0n;
  let point=BigInt('0x'+crt.decimal);if(point>=0x80000000n)point-=0x100000000n;
  // Empty CRT returns remain distinct, not invented zeros or discarded rows.
  const native=digits===''?null:normalized(BigInt(digits),point-BigInt(digits.length),negative);
  return {strText:text,crtDigits:digits,crtPoint:String(point),crtNegative:negative,
    strValue:observed,crtValue:native,
    exactValueDifferent:native===null?null:native.coefficient!==observed.coefficient||native.exponent!==observed.exponent,
    zeroSignDifferent:native!==null&&native.zero&&observed.zero?native.negative!==observed.negative:null};
}
module.exports={decimal,compare};
