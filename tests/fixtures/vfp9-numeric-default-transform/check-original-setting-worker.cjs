// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: SYNTHETIC worker gate tests only.
// Extract ONLY checked(); never execute the native capture script or native paths.
const fs = require('node:fs'), path = require('node:path'), cp = require('node:child_process');
const a = require('node:assert/strict');
const worker = fs.readFileSync(path.join(__dirname, 'original-setting-consumer-capture.sh'), 'utf8');
const begin = worker.indexOf('checked() {\n'), end = worker.indexOf('\nchecked regenerate-source ');
a(begin >= 0 && end > begin); const checked = worker.slice(begin, end);
a(!checked.includes('vfp9-probe')); a(!checked.includes('WINEDLLOVERRIDES'));
const sq = s => "'" + s.replaceAll("'", "'\\''") + "'";
const root = fs.mkdtempSync(path.join(__dirname, '../../../build/original-setting-worker-synthetic-'));
let checks = 0;
function test(name, prefixStatus, pressure, child, expected, childRan, extra = () => {}) {
  const capture = path.join(root, name); fs.mkdirSync(capture);
  const input = path.join(capture, 'input.txt'); fs.writeFileSync(input, 'immutable input\n');
  const sha = cp.spawnSync('sha256sum', [input], {encoding: 'utf8'}); a.equal(sha.status, 0);
  fs.writeFileSync(path.join(capture, 'inputs.sha256'), sha.stdout);
  // pgrep and the /proc read are explicit synthetic functions; no native lookup
  // or host-pressure assertion is made by these tests.
  const shell = 'set -euo pipefail\n' + 'capture_root=' + sq(capture) + '\n' +
    'pgrep() { return ' + prefixStatus + '; }\n' +
    'cat() { if [ "$1" = /proc/pressure/io ]; then printf "%s\\n" ' + sq(pressure) + '; else command cat "$@"; fi; }\n' +
    checked + '\nchecked ' + sq(name.startsWith('partial') ? 'first' : 'probe') + ' child.out bash -c ' +
    sq('printf ran > ' + sq(path.join(capture, 'child-ran.txt')) + '\n' + child) + '\n';
  const r = cp.spawnSync('bash', ['-c', shell], {encoding: 'utf8'});
  a.equal(r.signal, null); a.equal(r.status, expected, name + ': ' + r.stderr);
  a.equal(fs.existsSync(path.join(capture, 'child-ran.txt')), childRan, name);
  extra(capture); ++checks;
}
try {
  test('clear', 1, 'full avg10=0.00 avg60=0.99 avg300=0.00 total=0', 'printf ok', 0, true,
    p => { a.equal(fs.readFileSync(path.join(p, 'probe.guard.status'), 'utf8'), '0\n'); a(fs.existsSync(path.join(p, 'probe.inputs-after.txt'))); });
  test('busy-prefix', 0, 'full avg10=0.00 avg60=0.00', 'exit 0', 76, false);
  test('prefix-error', 2, 'full avg10=0.00 avg60=0.00', 'exit 0', 76, false);
  test('avg10-equal-limit', 1, 'full avg10=1.00 avg60=0.00', 'exit 0', 75, false);
  test('avg60-equal-limit', 1, 'full avg10=0.00 avg60=1.00', 'exit 0', 75, false);
  test('avg10-above-limit', 1, 'full avg10=25.00 avg60=0.00', 'exit 0', 75, false);
  test('avg60-above-limit', 1, 'full avg10=0.00 avg60=5.00', 'exit 0', 75, false);
  test('full-missing', 1, 'some avg10=0.00 avg60=0.00', 'exit 0', 75, false);
  test('child-failure', 1, 'full avg10=0.00 avg60=0.00', 'printf partial; exit 7', 7, true,
    p => { a.equal(fs.readFileSync(path.join(p, 'probe.status'), 'utf8'), '7\n'); a.equal(fs.readFileSync(path.join(p, 'child.out'), 'utf8'), 'partial'); a(!fs.existsSync(path.join(p, 'probe.inputs-after.txt'))); });
  test('child-stderr', 1, 'full avg10=0.00 avg60=0.00', 'printf error >&2', 98, true);
  const changed = path.join(root, 'changed-input', 'input.txt');
  test('changed-input', 1, 'full avg10=0.00 avg60=0.00', 'printf changed > ' + sq(changed), 1, true);
  const partial = path.join(root, 'partial-result', 'results.txt');
  test('partial-result', 1, 'full avg10=0.00 avg60=0.00', 'printf partial-result > ' + sq(partial) + '; exit 7', 7, true,
    p => a.equal(fs.readFileSync(path.join(p, 'first-results.txt'), 'utf8'), 'partial-result'));
  // Source-order contract, not a native execution or cleanup claim.
  a(worker.indexOf('test ! -e "$capture_root"') < worker.indexOf('mkdir "$capture_root"'));
  a(worker.includes('test -z "${WINEDLLOVERRIDES-}" || exit 94'));
  a(worker.includes('for name in first repeat; do'));
  a(worker.includes('checked first-structure first-structure.json node "$fixture/audit-original-setting-consumer.cjs" "$capture_root" --single'));
  a(worker.indexOf('checked first-structure ') < worker.indexOf('\nchecked audit '));
  a(worker.includes('checked "$name" "$name.out" /usr/bin/env WINEDLLOVERRIDES=msvcr71=n /home/rich/bin/vfp9-probe'));
  ++checks;
  console.log(JSON.stringify({syntheticWorkerSelfTests: checks, nativeInvoked: false, vmInvoked: false,
    actualHostClearanceClaimed: false, checkedFunctionOnly: true, nativeWorkerExecuted: false}));
} finally {
  for (const dir of fs.readdirSync(root)) {
    const p = path.join(root, dir); for (const name of fs.readdirSync(p)) fs.unlinkSync(path.join(p, name)); fs.rmdirSync(p);
  }
  fs.rmdirSync(root);
}
