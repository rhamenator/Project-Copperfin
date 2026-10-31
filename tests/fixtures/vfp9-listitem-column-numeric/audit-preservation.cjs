// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// NON-NATIVE complete framing only; current behavior is not a requirement oracle.
const fs = require('fs');
const assert = require('assert');
const crypto = require('crypto');
assert.equal(process.argv.length, 3, 'Expected one complete256 preservation capture');
const bytes = fs.readFileSync(process.argv[2]);
const lines = bytes.toString('utf8').trimEnd().split(/\r?\n/);
assert.equal(lines.pop(), 'DONE|cases=32|calls=256');
assert.equal(lines.length, 256);
const expected = {
  arity: 2,
  'second-type': 6,
  'first-selector': 8,
  binding: 3,
  'nonexact-rst': 6,
  'missing-column': 3,
  'ordinary-control': 4,
};
assert.equal(Object.values(expected).reduce((sum, count) => sum + count, 0), 32);
const identities = new Set();
const contexts = new Map();
for (const line of lines) {
  const match = line.match(/^(ListBox|ComboBox)\|(COPPERFIN|VFP9)\|([12])\|([^|]+)\|([^|]+)\|completed=1\|/);
  assert(match, 'complete framed invocation');
  assert(Object.hasOwn(expected, match[4]), 'known group');
  const identity = match.slice(1, 6).join('|');
  assert(!identities.has(identity), 'unique case identity');
  identities.add(identity);
  const context = match.slice(1, 4).join('|');
  if (!contexts.has(context)) contexts.set(context, {});
  const counts = contexts.get(context);
  counts[match[4]] = (counts[match[4]] ?? 0) + 1;
  for (const field of ['cphase', 'ncode', 'vresult', 'ncount', 'nfocus', 'nid',
    'cvalue', 'cdisplay', 'ntop', 'ntopid', 'ccontext', 'cother', 'ids',
    'primary', 'selected', 'rows', 'data']) {
    assert(line.includes(`|${field}=`), `retained ${field}`);
  }
}
assert.equal(contexts.size, 8);
for (const counts of contexts.values()) assert.deepEqual(counts, expected);
console.log(JSON.stringify({
  calls: 256,
  contexts: 8,
  uniqueCases: identities.size,
  groupsPerContext: expected,
  rawSHA256: crypto.createHash('sha256').update(bytes).digest('hex'),
  note: 'NON-NATIVE framing only; baseline/fixed SAME-compiler whole-byte equality is mandatory.',
}));
