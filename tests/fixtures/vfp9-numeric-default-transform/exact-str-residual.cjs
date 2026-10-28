// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: signed diagnostic residual, no tolerance.
'use strict';
const a=require('node:assert/strict');
function residual(observed,nearest,negative) {
  for(const value of [observed,nearest])a.match(value,/^(?:0|-?[1-9][0-9]*)$/);
  a.equal(typeof negative,'boolean');
  const delta=BigInt(observed)-BigInt(nearest);
  // Units are one decimal place at the separately retained requested count.
  // The raw payload sign orients the residual away/toward zero, even when
  // a rounded integer has lost the sign of zero. Never infer sign from delta.
  const magnitudeDelta=negative?-delta:delta;
  return {signedUnits:String(delta),magnitudeUnits:String(magnitudeDelta),
    direction:magnitudeDelta===0n?'equal':magnitudeDelta>0n?'away':'toward'};
}
module.exports={residual};
