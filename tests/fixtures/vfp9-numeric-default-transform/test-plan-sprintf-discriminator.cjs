// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{plan}=require('./plan-sprintf-discriminator.cjs');
const rows=[];for(let index=1;index<=2;++index)for(const fixed of ['OFF','ON'])rows.push({index,fixed,bits:index.toString(16).padStart(16,'0').toUpperCase(),setting:13,str15Hex:'A'+index,str16Hex:'B'+index,str17Hex:'C'+index,str18Hex:'D'+index});
const report=plan(rows,2);a.equal(report.inputs,2);a.equal(report.vfpComparisons,16);a.equal(report.crtCalls,8);a.equal(report.bufferBytes,2048);a.deepEqual(report.exports,['sprintf','_snprintf']);a.equal(report.cases[1].expectedStrHex[17],'C2');
a.throws(()=>plan(rows.slice(1),2));const changed=structuredClone(rows);changed[3].str18Hex='DIFFERENT';a.throws(()=>plan(changed,2));
console.log('sprintf discriminator plan: 8 checks passed');
