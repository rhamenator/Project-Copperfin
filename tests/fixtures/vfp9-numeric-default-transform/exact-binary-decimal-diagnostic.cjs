// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: diagnostic hypotheses, NOT expected text.
'use strict';
const a=require('node:assert/strict');

// Input is the retained BINTOC(...,'BRS') hexadecimal byte order. Decode it
// explicitly; never convert through a JavaScript floating-point Number.
function scaled(bits,count) {
  a.match(bits,/^[0-9A-F]{16}$/);
  a(Number.isInteger(count)&&count>=0&&count<=21);
  const u=BigInt('0x'+bits),negative=(u>>63n)!==0n;
  const exponent=Number((u>>52n)&2047n),fraction=u&((1n<<52n)-1n);
  a.notEqual(exponent,2047,'finite payload required');
  const mantissa=exponent===0?fraction:(1n<<52n)+fraction;
  const power=exponent===0?-1074:exponent-1023-52;
  let numerator=mantissa*10n**BigInt(count),denominator=1n;
  if(power>=0)numerator<<=BigInt(power);else denominator<<=BigInt(-power);
  const quotient=numerator/denominator,remainder=numerator%denominator;
  return {negative,numerator,denominator,quotient,remainder};
}

function candidates(bits,count) {
  const r=scaled(bits,count),twice=2n*r.remainder;
  const above=twice>r.denominator,tie=twice===r.denominator;
  const sign=r.negative?-1n:1n;
  return {negative:r.negative,exact:r.remainder===0n,tie,
    truncated:String(sign*r.quotient),
    nearestEven:String(sign*(r.quotient+BigInt(above||(tie&&r.quotient%2n!==0n)))),
    nearestAway:String(sign*(r.quotient+BigInt(above||tie))),
    remainder:String(r.remainder),denominator:String(r.denominator)};
}
module.exports={scaled,candidates};
