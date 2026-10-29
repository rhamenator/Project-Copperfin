// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: exact retained consumer comparisons only.
'use strict';
const path=require('node:path'),a=require('node:assert/strict');
const {load,combined}=require('./load-str-crt-coverage.cjs');
const {compare}=require('./compare-str-crt-cell.cjs');
const coverage=combined(__dirname);
const str=load(path.join(__dirname,'str-middle-captures1'),coverage.sourceHashes.str,528);
const old=load(path.join(__dirname,'crt-nativeonly-captures1'),coverage.sourceHashes.crt,4176);
const more=load(path.join(__dirname,'crt-precision-captures1'),coverage.sourceHashes.precision,1584);
const rows=[];
for(const cell of coverage.cells) {
  if(!cell.nativeCrtAvailable) continue;
  const s=str.observations.find(r=>r.index===cell.index&&r.fixed===cell.fixed);a(s);
  const candidates=[...old.observations,...more.observations].filter(r=>r.route==='ORIGINAL'&&
    r.kind==='F'&&r.index===cell.index&&r.fixed===cell.fixed&&r.bits===cell.bits&&
    r.argumentBits===cell.bits&&r.setting===cell.setting&&r.count===cell.count);
  a.equal(candidates.length,1);const native=candidates[0];
  const strHex=s['str'+cell.count+'Hex'];a.match(strHex,/^(?:[0-9A-F]{2})+$/);
  rows.push({...cell,offset:cell.count-cell.setting,
    source:cell.count===cell.setting?'crt-nativeonly-captures1':'crt-precision-captures1',
    strHex,nativeControl:native.crt,
    ...compare(Buffer.from(strHex,'hex').toString('ascii'),native.crt)});
}
a.equal(rows.length,coverage.availableCells);
const groups=[];
for(const offset of [0,1,2,3]) for(const fixed of ['OFF','ON']) {
  const selected=rows.filter(r=>r.offset===offset&&r.fixed===fixed);
  groups.push({offset,fixed,contexts:selected.length,
    emptyReturns:selected.filter(r=>r.crtValue===null).length,
    exactValueDifferent:selected.filter(r=>r.exactValueDifferent===true).length,
    zeroSignDifferent:selected.filter(r=>r.zeroSignDifferent===true).length});
}
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,sourceHashes:coverage.sourceHashes,
  available:rows.length,missingCells:coverage.missingCells,groups,rows,
  limitation:'Finite public STR versus CRT value comparison, not internal-call attribution, expected digits, conversion algorithm or formatter acceptance. Empty returns are separate; no tolerance.'},null,2));
