// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Compare actual independently audited records to PRE-CAPTURE hypotheses only.
// Never used to emit native expected test data; imports no product code.
const fs=require('node:fs'),a=require('node:assert/strict');
const native=JSON.parse(fs.readFileSync(process.argv[2]));
const inventory=JSON.parse(fs.readFileSync(__dirname+'/holdout-cases.json'));
const prediction=JSON.parse(fs.readFileSync(__dirname+'/holdout-predictions.json'));
a.equal(prediction.kind,'PRE-CAPTURE HYPOTHESIS ONLY, NOT ACTUAL NATIVE EXPECTATIONS');
a.equal(prediction.rows.length,213);a.equal(native.observations.length,1704);
const predicted=new Map(prediction.rows.map(row=>[row.index,row]));
const differences=[];let compared=0,newCompared=0,calibrationCompared=0;
for(const row of inventory.rows) {
    const expected=predicted.get(row.index);a(expected);a.equal(expected.literal,row.literal);
    a.match(expected.bits,/^[0-9A-F]{16}$/);
    const records=native.observations.filter(o=>o.index===row.index);a.equal(records.length,8);
    for(const actual of records) {
        ++compared;
        if(row.kind==='heldout')++newCompared;else ++calibrationCompared;
        if(actual.result.bits!==expected.bits)differences.push({literal:row.literal,
            kind:row.kind,setting:actual.setting,fixed:actual.fixed,mode:actual.mode,
            predicted:expected.bits,actual:actual.result});
    }
}
a.equal(compared,1704);a.equal(newCompared,198*8);a.equal(calibrationCompared,15*8);
console.log(JSON.stringify({preCaptureCheckpoint:'4ff7d0d8b2c44027914370075e3adbd67c40f5c8',
    compared,newCompared,calibrationCompared,mismatches:differences.length,
    finiteProfileHypothesisPassed:differences.length===0,differences,
    limitation:'Held-out finite-profile prediction comparison, not a native expected emitter, universal conversion rule, product/SAME pass or Numeric/division/ListItem completion.'},null,2));
