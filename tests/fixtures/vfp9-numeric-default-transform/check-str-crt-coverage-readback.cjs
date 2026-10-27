// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: independent retained coverage readback.
'use strict';
const fs = require('node:fs'), z = require('node:zlib'), path = require('node:path');
const a = require('node:assert/strict');
const output = JSON.parse(fs.readFileSync(process.argv[2]));
const read = name => JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,name,'audit.json.gz'))));
const str = read('str-middle-captures1').observations;
const crt = read('crt-nativeonly-captures1').observations;
a.equal(output.coverageOnly,true); a.equal(output.candidateOrEqualityEvidence,false);
a.equal(output.qualifiedJoinOrDigitEquality,false); a.equal(output.nativeInvoked,false);
let available=0; const expectedMissing=[]; let position=0;
for (const s of str) for (let count=15;count<=18;++count) {
  const matches=crt.filter(c => c.route==='ORIGINAL' && c.kind==='F' && c.index===s.index &&
    c.fixed===s.fixed && c.bits===s.bits && c.argumentBits===s.bits &&
    c.setting===s.setting && c.count===count);
  a(matches.length<=1);
  const cell={index:s.index,fixed:s.fixed,bits:s.bits,setting:s.setting,count,
    nativeCrtAvailable:matches.length===1};
  a.deepEqual(output.cells[position++],cell);
  if(matches.length) ++available; else expectedMissing.push(cell);
}
a.equal(position,output.cells.length); a.equal(position,output.requestedCells);
a.equal(available,output.availableCells); a.deepEqual(output.missingCells,expectedMissing);
console.log(JSON.stringify({independentCoverageReadback:true,requested:position,
  available,missing:expectedMissing.length,nativeInvoked:false}));
