// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Diagnose actual ORIGINAL failures. Imports no product/candidate implementation.
const fs = require('node:fs'), path = require('node:path'), a = require('node:assert/strict');
const {execFileSync} = require('node:child_process');
const root = process.argv[2];
// CTest --verbose prefixes test output with its numeric inventory index.
const log = fs.readFileSync(path.join(root,'original-ctest.log'),'utf8').replace(/^\d+: /gm,'');
a.equal(fs.readFileSync(path.join(root,'original-build.status'),'utf8'),'0\n');
a.equal(fs.readFileSync(path.join(root,'original-ctest.status'),'utf8'),'8\n');
const matches = [...log.matchAll(/^non-whole division: descriptor=(\d+) bits=(\d+) literalBits=(\d+) evaluation=(\d+) controls=(\d+) total=(\d+)$/gm)];
a.equal(matches.length,1);
const [descriptor,bits,literalBits,evaluation,controls,total] = matches[0].slice(1).map(Number);
a(descriptor>0); a(bits>0); a.equal(evaluation,0); a.equal(controls,0);
a.equal(total,descriptor+bits+evaluation+controls);
const failures = log.split(/\r?\n/).filter(line=>line.startsWith('FAIL: '));
a.equal(failures.length,total);
a.equal(failures.filter(line=>line.endsWith(' native descriptor')).length,descriptor);
const bitLines = failures.filter(line=>/ bits actual=[0-9A-F]{16} native=[0-9A-F]{16}$/.test(line));
a.equal(bitLines.length,bits);
const literalLines = bitLines.filter(line=>line.includes(' literal-control:'));
a.equal(literalLines.length,literalBits);
const inventory = JSON.parse(fs.readFileSync(path.join(__dirname,'nonwhole-grid-cases.json')));
const native = JSON.parse(execFileSync(process.execPath,[path.join(__dirname,'audit-nonwhole-grid.cjs'),path.join(__dirname,'nonwhole-captures')],{maxBuffer:32*1024*1024}));
const literalFailures = [];
for (const row of inventory.rows.filter(r=>r.kind==='literal-control')) {
    const actual = literalLines.filter(line=>line.includes(` literal-control:${row.index} ${row.expression} bits `));
    if (!actual.length) continue;
    a.equal(actual.length,20);
    const comparisons = actual.map(line=>/actual=([0-9A-F]{16}) native=([0-9A-F]{16})$/.exec(line).slice(1));
    for (const comparison of comparisons) a.deepEqual(comparison,comparisons[0]);
    const observations = native.observations.filter(o=>o.index===row.index);
    a.equal(observations.length,20);
    for (const observation of observations) a.equal(observation.result.bits,comparisons[0][1]);
    literalFailures.push({expression:row.expression,actual:comparisons[0][0],native:comparisons[0][1],contexts:actual.length});
}
a.equal(literalFailures.length,2);
a.equal(literalBits,40);
// Independent counterfactual: replace only the captured native literal inputs
// by standard decimal-to-binary64 inputs, retaining the manifest's exact trees.
// This diagnoses recorded payload failures; it does not emit expectations or
// recover VFP's decimal conversion algorithm, and says nothing about metadata.
const bitsOf = value => { const b=Buffer.alloc(8); b.writeDoubleBE(value); return b.toString('hex').toUpperCase(); };
const cores = new Map(inventory.cores.map(core=>[core.id,core]));
function standardValue(node) {
    if (node.kind==='literal') return Number(node.text);
    if (node.kind==='negate') return -standardValue(node.operand);
    a.equal(node.kind,'divide');
    return standardValue(node.left)/standardValue(node.right);
}
let classifiedBitFailures=0, corePayloadRows=0;
for (const row of inventory.rows) {
    const actual=bitLines.filter(line=>line.includes(` ${row.kind}:${row.index} ${row.expression} bits `));
    if (!actual.length) continue;
    a(['core','literal-control'].includes(row.kind)); a.equal(actual.length,20);
    let value;
    if (row.kind==='literal-control') value=Number(row.literal);
    else {
        value=standardValue(cores.get(row.core).tree);
        if (['minus','minus-iif','minus-evaluate'].includes(row.wrapper)) value=-value;
        // Double negation preserves nonzero payloads in this manifest.
        if (row.wrapper==='double-minus') value=-(-value);
        ++corePayloadRows;
    }
    for (const line of actual) {
        const comparison=/actual=([0-9A-F]{16}) native=([0-9A-F]{16})$/.exec(line);
        a.equal(comparison[1],bitsOf(value));
        const observations=native.observations.filter(o=>o.index===row.index);
        a.equal(observations.length,20);
        for (const observation of observations) a.equal(comparison[2],observation.result.bits);
    }
    classifiedBitFailures+=actual.length;
}
a.equal(classifiedBitFailures,bits);
console.log(JSON.stringify({baseline:'ef3ea326f78d640546363abe97bbe27b6208bf45',
    checkpoint:'274a38635a463db154b4b9e0c78c502d6722c53c',
    descriptor,bits,literalBits,evaluation,controls,total,literalFailures,
    classifiedBitFailures,corePayloadRows,
    conclusion:'Standalone literal payload failures occur without division in both modes/all settings. A literal-producer prerequisite is required; no quotient correction or universal conversion algorithm is established.'},null,2));
