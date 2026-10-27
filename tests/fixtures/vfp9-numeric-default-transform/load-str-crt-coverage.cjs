// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: retained-source coverage, not conversion.
'use strict';
const fs = require('node:fs'), path = require('node:path'), z = require('node:zlib');
const a = require('node:assert/strict'), crypto = require('node:crypto');
const {inventory} = require('./inventory-str-crt-coverage.cjs');
const sha = b => crypto.createHash('sha256').update(b).digest('hex');
function load(root, expected, rows, io = fs) {
  const read = n => z.gunzipSync(io.readFileSync(path.join(root, n + '.gz')));
  const sums = new Map();
  for (const line of io.readFileSync(path.join(root, 'SHA256SUMS'), 'utf8').trimEnd().split('\n')) {
    const m = line.match(/^([0-9a-f]{64})  ([A-Za-z0-9_.-]+)$/); a(m);
    a(!sums.has(m[2]), 'duplicate manifest entry'); sums.set(m[2], m[1]);
  }
  for (const n of ['audit.json', 'first-results.txt', 'repeat-results.txt'])
    a.equal(sha(io.readFileSync(path.join(root, n + '.gz'))), sums.get(n + '.gz'));
  const report = JSON.parse(read('audit.json'));
  a.equal(report.recordsPerRun, rows); a.equal(report.observations.length, rows);
  a.equal(report.documentedFieldsRepeatEqual, true); a.equal(report.rawByteEqual, true);
  for (const run of ['first', 'repeat']) {
    const raw = read(run + '-results.txt');
    a.equal(sha(raw), expected); a.equal(report[run + 'Sha256'], expected);
    a.equal(read(run + '.status').toString(), '0\n'); a.equal(read(run + '.stderr').length, 0);
    a.deepEqual(raw, read(run + '.out'));
  }
  return report;
}
function main(root) {
  const strHash = '13fba5c3663d1d44f0cf514d481abfc66e16063e3e300d1c8e33502b10282c6c';
  const crtHash = '60a1b217bbe305ccb61012cb4324d7bb944d9e413bd920803debd17e36e8c54d';
  const str = load(path.join(root, 'str-middle-captures1'), strHash, 528);
  const crt = load(path.join(root, 'crt-nativeonly-captures1'), crtHash, 4176);
  for (const flag of ['originalAndNativeParentControls', 'parentStrBeforeAfterExact',
    'inlineMetadataAndBRSExact', 'publicStrTypesAndWidths']) a.equal(str[flag], true);
  for (const flag of ['metadataAndBRSExact', 'settingsAndRuntimeModuleExact']) a.equal(crt[flag], true);
  // Hash/report binding is not independent replay of the native readers.
  // Before accepting this inventory, replay the frozen readers separately.
  return {...inventory(str.observations, crt.observations), sourceHashes: {str: strHash, crt: crtHash},
    nativeInvoked: false, qualifiedJoinOrDigitEquality: false,
    limitation: 'Coverage only. Native-reader replay, metadata/runtime equivalence and digit comparison remain separate gates.'};
}
module.exports = {load, main};
if (require.main === module) console.log(JSON.stringify(main(__dirname), null, 2));
