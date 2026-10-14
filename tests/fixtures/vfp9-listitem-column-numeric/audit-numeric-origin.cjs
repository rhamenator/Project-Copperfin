// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native observation counterexample audit, NOT a chosen conversion requirement.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const root = process.argv[2];
function rows(folder) {
  const found = new Map();
  for (const line of fs.readFileSync(path.join(root, folder, 'first-results.txt'), 'utf8').split(/\r?\n/)) {
    const f = line.split('|');
    if (f[0] !== 'READ') continue;
    assert.equal(f.length, 17);
    const key = JSON.stringify([f[1], f[2], f[4]]);
    assert(!found.has(key));
    found.set(key, f);
  }
  return found;
}
const raw = rows('expression-shape');
const roundtrip = rows('binary-roundtrip-v2');
assert.equal(raw.size, 120);
assert.deepEqual([...raw.keys()], [...roundtrip.keys()]);
const changes = [];
for (const [key, before] of raw) {
  const after = roundtrip.get(key);
  assert.deepEqual(before.slice(5, 9), after.slice(5, 9), 'actual Numeric types/eight-byte values equal');
  assert.deepEqual(before.slice(13), after.slice(13), 'contexts and full sampled control states equal');
  if (JSON.stringify(before.slice(10, 13)) !== JSON.stringify(after.slice(10, 13)))
    changes.push({key: JSON.parse(key), bits: before[6],
      original: before.slice(10, 13), documentedDoubleRoundtrip: after.slice(10, 13)});
}
assert.equal(changes.length, 32);
const counterexamples = [];
for (const kind of ['ListBox', 'ComboBox']) for (const id of ['11', '22']) {
  const decimal = raw.get(JSON.stringify([kind, id, '1-1E-14']));
  const power = raw.get(JSON.stringify([kind, id, '1-90*2^(-53)']));
  assert.deepEqual(decimal.slice(5, 9), power.slice(5, 9));
  assert.deepEqual(decimal.slice(13), power.slice(13));
  assert.equal(decimal[6], '3FEFFFFFFFFFFFA6');
  assert.deepEqual(decimal.slice(10, 13), ['31', 'Invalid subscript reference.', '<not-called>']);
  assert.deepEqual(power.slice(10, 13), ['0', '', `C:id${id}c1`]);
  counterexamples.push({kind, id, bits: decimal[6], decimal: decimal.slice(10, 13), power: power.slice(10, 13)});
}
console.log(JSON.stringify({audit: 'observation-counterexamples-only', compared: raw.size,
  changedAfterBitPreservingRoundtrip: changes.length, counterexamples, changes,
  conclusion: 'A stateless helper keyed only by Numeric kind and these eight bytes cannot reproduce both observed outcomes; native internal cause and product policy not inferred.'}));
