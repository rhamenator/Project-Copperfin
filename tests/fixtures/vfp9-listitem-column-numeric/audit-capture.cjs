// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Framing/precondition/immutability audit, NOT a numeric conversion expected table.
const fs = require('node:fs');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const path = require('node:path');
const root = process.argv[2];
const perContext = Number(process.argv[3] || 56);
const configuration = process.argv[4] === 'configuration';
assert(root && Number.isInteger(perContext) && perContext > 0,
  'usage: node audit-capture.cjs BUILD_DIRECTORY [OPERANDS_PER_CONTEXT]');
const first = fs.readFileSync(path.join(root, 'first-results.txt'));
const repeat = fs.readFileSync(path.join(root, 'repeat-results.txt'));
assert.deepEqual(first, repeat, 'full byte-identical repeat');
for (const phase of ['first', 'repeat']) {
  assert.equal(fs.readFileSync(path.join(root, `${phase}.status`), 'utf8'), '0\n');
  assert.equal(fs.statSync(path.join(root, `${phase}.stderr`)).size, 0);
  assert.deepEqual(fs.readFileSync(path.join(root, `${phase}.out`)), first);
}
const lines = first.toString('utf8').split('\r\n');
assert.equal(lines.pop(), '');
assert.equal(lines.shift(), 'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
assert.equal(lines.pop(), `DONE|${perContext * 4}`);
assert.equal(lines.length, perContext * 4);
const seen = new Set();
const contexts = new Map();
const observations = [];
const outcomes = {};
for (const line of lines) {
  const f = line.split('|');
  assert.equal(f.length, 17, 'complete capture fields');
  const [tag, kind, id, found, operand, type, bits, typeAfter, bitsAfter,
    stage, error, message, returned, context, contextAfter, before, after] = f;
  assert.equal(tag, 'READ');
  assert(['ListBox', 'ComboBox'].includes(kind));
  assert(['11', '22'].includes(id));
  assert.equal(found, id === '11' ? '1' : '2', 'existing stable-ID/display-row distinction');
  assert.equal(type, 'N');
  assert.equal(typeAfter, type);
  assert.match(bits, /^[0-9A-F]{16}$/);
  assert.equal(bitsAfter, bits, 'actual operand bits preserved');
  assert.equal(stage, 'invoke', 'every call reaches selected consumer');
  assert.match(error, /^\d+$/);
  if (error === '0') {
    assert.equal(message, '');
    assert.match(returned, /^C:/);
  } else {
    assert(message.length > 0);
    assert.equal(returned, '<not-called>');
  }
  assert.equal(context, '1;;1');
  assert.equal(contextAfter, context);
  assert.equal(after, before, 'complete sampled state unchanged');
  const configNo = configuration ? Number(operand.split(':')[0]) : 2;
  assert(Number.isInteger(configNo) && configNo >= 1 && configNo <= 6);
  const seedColumns = configNo === 4 || configNo === 5 ? 1 : configNo === 6 ? 256 : 9;
  const columns = configNo === 4 ? 0 : seedColumns;
  const binding = configNo === 3 ? '   ' : '';
  let expectedState = `count=3;columns=${columns};rst=N:0;binding=C:[${binding}];newid=33;newindex=3;index=2;itemid=22;display=id22c1;value=id22c1;top=`;
  expectedState += kind === 'ListBox' ? '2:22' : '<not-applicable>';
  for (let row = 1; row <= 3; ++row) {
    expectedState += `;row=${row},id=${row * 11},data=${row * 101},selected=${row === 2 ? '.T.' : '.F.'}`;
    for (let col = 1; col <= 9; ++col)
      expectedState += `,c${col}=[${col <= seedColumns ? `id${row * 11}c${col}` : ''}]`;
    if (configuration) for (const col of [255, 256])
      expectedState += `,c${col}=[${col <= seedColumns ? `id${row * 11}c${col}` : ''}]`;
  }
  assert.equal(before, expectedState, 'fresh fully populated supported setup before read');
  const key = `${kind}/${id}/${operand}`;
  assert(!seen.has(key));
  seen.add(key);
  const group = `${kind}/${id}`;
  contexts.set(group, (contexts.get(group) || 0) + 1);
  const outcome = `${error}:${message}:${returned.replaceAll(`id${id}c`, 'targetc')}`;
  outcomes[outcome] = (outcomes[outcome] || 0) + 1;
  observations.push({kind, id, operand, bits, error, message, returned});
}
assert.equal(contexts.size, 4);
for (const count of contexts.values()) assert.equal(count, perContext);
for (const row of observations.filter(r => r.kind === 'ListBox' && r.id === '11')) {
  const peers = observations.filter(r => r.operand === row.operand);
  assert.equal(peers.length, 4);
  for (const peer of peers) {
    assert.equal(peer.bits, row.bits);
    assert.equal(peer.error, row.error);
    assert.equal(peer.message, row.message);
    assert.equal(peer.returned.replaceAll(`id${peer.id}c`, 'targetc'),
      row.returned.replaceAll('id11c', 'targetc'));
  }
}
console.log(JSON.stringify({audit: 'capture-only', rows: perContext * 4, contexts: Object.fromEntries(contexts),
  outcomes, fullRawSha256: crypto.createHash('sha256').update(first).digest('hex'),
  conversionRequirement: 'not asserted; supplemental recovery required'}));
