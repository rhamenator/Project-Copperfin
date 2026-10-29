// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: synthetic loader rejection tests.
'use strict';
const a = require('node:assert/strict'), z = require('node:zlib'), c = require('node:crypto');
const {load} = require('./load-str-crt-coverage.cjs');
const sha = b => c.createHash('sha256').update(b).digest('hex');
const raw = Buffer.from('synthetic fixture only\n'), digest = sha(raw);
function fixture(delta = {}) {
  const report = {recordsPerRun: 1, observations: [{}], documentedFieldsRepeatEqual: true,
    rawByteEqual: true, firstSha256: digest, repeatSha256: digest, ...delta};
  const files = new Map([['audit.json', Buffer.from(JSON.stringify(report))]]);
  for (const run of ['first', 'repeat']) for (const [suffix, value] of
    [['-results.txt',raw], ['.out',raw], ['.status',Buffer.from('0\n')], ['.stderr',Buffer.alloc(0)]])
    files.set(run + suffix, value);
  const gz = new Map([...files].map(([n,b]) => [n + '.gz', z.gzipSync(b)]));
  let manifest = ['audit.json', 'first-results.txt', 'repeat-results.txt']
    .map(n => sha(gz.get(n + '.gz')) + '  ' + n + '.gz').join('\n') + '\n';
  return {gz, setManifest: s => {manifest = s;}, getManifest: () => manifest,
    io: {readFileSync: (p, encoding) => p.endsWith('SHA256SUMS') ? manifest : gz.get(p.slice(3))}};
}
let tests = 0;
function check(f) {f(); ++tests;}
check(() => a.equal(load('/x',digest,1,fixture().io).observations.length,1));
for (const delta of [{recordsPerRun:2}, {observations:[]}, {documentedFieldsRepeatEqual:false},
  {rawByteEqual:false}, {firstSha256:'0'.repeat(64)}, {repeatSha256:'0'.repeat(64)}])
  check(() => a.throws(() => load('/x',digest,1,fixture(delta).io)));
for (const field of ['first.status','repeat.stderr','first.out','audit.json']) check(() => {
  const f = fixture(); f.gz.set(field + '.gz', z.gzipSync(Buffer.from('bad')));
  a.throws(() => load('/x',digest,1,f.io));
});
check(() => {const f=fixture(); f.setManifest(f.getManifest()+f.getManifest());
  a.throws(() => load('/x',digest,1,f.io), /duplicate manifest/);});
console.log(JSON.stringify({syntheticLoaderTests:tests,nativeInvoked:false}));
