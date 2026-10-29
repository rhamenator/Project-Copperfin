// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{analyze}=require('./analyze-str-ambient.cjs');
const rows=[];
for(let index=1;index<=6;++index)for(const fixed of ['OFF','ON'])for(const ambientD of [0,13,18])rows.push({index,bits:index.toString(16).padStart(16,'0').toUpperCase(),fixed,ambientD,str15Hex:'15',str16Hex:'16',str17Hex:'17',str18Hex:'18'});
const clean=analyze(rows);a.equal(clean.exactAmbientComparisons,96);a.equal(clean.ambientMismatchCount,0);a.equal(clean.ambientIndependentForObservedGrid,true);a.deepEqual(clean.ambientMismatches,[]);
const changed=structuredClone(rows),witness=changed.find(r=>r.index===4&&r.fixed==='ON'&&r.ambientD===13);witness.str17Hex='DIFFERENT';
const report=analyze(changed);a.equal(report.ambientMismatchCount,1);a.equal(report.ambientIndependentForObservedGrid,false);
a.deepEqual(report.ambientMismatches,[{index:4,bits:'0000000000000004',fixed:'ON',count:17,baselineAmbientD:0,actualAmbientD:13,baselineHex:'17',actualHex:'DIFFERENT'}]);
a.throws(()=>analyze(rows.slice(1)));console.log('str ambient analysis: 8 checks passed');
