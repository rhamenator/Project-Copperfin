// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: arithmetic relation, not causal attribution.
'use strict';
const a=require('node:assert/strict');
function relation(observed,nearest,residual){
 for(const value of [observed,nearest,residual])a.match(value,/^(?:0|-?[1-9][0-9]*)$/);
 const o=BigInt(observed),n=BigInt(nearest),r=BigInt(residual);a.equal(o,n+r);
 return {observedNonzero:o!==0n,nearestNonzero:n!==0n,residualNonzero:r!==0n,
  equalSteps:o===n,
  category:o===0n?(n===0n?'both-zero':'cancelled'):
   n===0n?'residual-only':o===n?'equal-step':'mixed'};
}
module.exports={relation};
