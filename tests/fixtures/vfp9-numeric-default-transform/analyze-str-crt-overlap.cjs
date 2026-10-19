// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER capture, exact offline join only.
// This does not format input values, create expected strings or execute native code.
const fs = require('node:fs'), path = require('node:path'), z = require('node:zlib');
const a = require('node:assert/strict'), crypto = require('node:crypto');
const fixture = __dirname;
const bytes = (dir, name) => z.gunzipSync(fs.readFileSync(path.join(dir, name + '.gz')));
const sha = b => crypto.createHash('sha256').update(b).digest('hex');
const ascii = h => { a.match(h, /^(?:[0-9A-F]{2})*$/); return Buffer.from(h, 'hex').toString('ascii'); };
function load(name, expected, rows, fields) {
  const dir = path.join(fixture, name), report = JSON.parse(bytes(dir, 'audit.json'));
  a.equal(report.recordsPerRun, rows); a.equal(report.observations.length, rows);
  a.equal(report.documentedFieldsRepeatEqual, true); a.equal(report.rawByteEqual, true);
  for (const run of ['first', 'repeat']) {
    const raw = bytes(dir, run + '-results.txt');
    a.equal(sha(raw), expected); a.equal(report[run + 'Sha256'], expected);
    a.equal(bytes(dir, run + '.status').toString(), '0\n');
    a.equal(bytes(dir, run + '.stderr').length, 0); a.deepEqual(raw, bytes(dir, run + '.out'));
  }
  const lines = bytes(dir, 'first-results.txt').toString('ascii').split('\r\n');
  a.equal(lines.pop(), ''); a.equal(lines.shift(), 'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
  a.equal(lines.pop(), 'DONE|' + rows); a.equal(lines.length, rows);
  const rawRows = lines.map(s => { const f = s.split('|'); a.equal(f.length, fields); return f; });
  // Both retained archives publish SHA256SUMS; the older CRT archive predates
  // the additional JSON manifest used by the newer STR archive.
  const inventory = new Map();
  for (const line of fs.readFileSync(path.join(dir, 'SHA256SUMS'), 'utf8').trimEnd().split('\n')) {
    const m = line.match(/^([0-9a-f]{64})  ([A-Za-z0-9_.-]+)$/); a(m); a(!inventory.has(m[2])); inventory.set(m[2], m[1]);
  }
  for (const n of ['audit.json', 'first-results.txt', 'repeat-results.txt']) {
    a(inventory.has(n + '.gz'));
    a.equal(sha(fs.readFileSync(path.join(dir, n + '.gz'))), inventory.get(n + '.gz'));
  }
  return {report, rawRows};
}
const strHash = '13fba5c3663d1d44f0cf514d481abfc66e16063e3e300d1c8e33502b10282c6c';
const crtHash = '60a1b217bbe305ccb61012cb4324d7bb944d9e413bd920803debd17e36e8c54d';
const str = load('str-middle-captures1', strHash, 528, 32);
const crt = load('crt-nativeonly-captures1', crtHash, 4176, 20);
for (const k of ['originalAndNativeParentControls', 'parentStrBeforeAfterExact', 'inlineMetadataAndBRSExact', 'publicStrTypesAndWidths']) a.equal(str.report[k], true);
for (const k of ['metadataAndBRSExact', 'settingsAndRuntimeModuleExact']) a.equal(crt.report[k], true);
const key = r => JSON.stringify([r.index, r.fixed]);
const strRows = new Map(), overlap = [], checked = new Set();
const metadata = s => { const m = Object.fromEntries(s.split(';').map(x => x.split('='))); delete m.integer; return m; };
for (const [i, r] of str.report.observations.entries()) {
  const f = str.rawRows[i]; a.equal(f[0], r.fixed); a.equal(f[1], String(r.index));
  a.equal(f[2], r.bits); a.equal(f[3], String(r.setting)); a.equal(f[6], r.originalDisplayHex);
  a.equal(f[7], r.originalStrHex); a.deepEqual(metadata(f[4]), r.originalMeta);
  for (const [count, field] of [[15, 15], [16, 25], [17, 27], [18, 17]]) a.equal(f[field], r['str' + count + 'Hex']);
  a(!strRows.has(key(r))); strRows.set(key(r), {r, f});
}
a.equal(strRows.size, 528);
const runtimeKeys = ['module', 'fileSample', 'fileSize', 'fileSha256', 'machine', 'magic', 'stamp', 'imageSize', 'checksum', 'cw0', 'cw1'];
for (const [i, r] of crt.report.observations.entries()) {
  if (r.route !== 'ORIGINAL') continue;
  const f = crt.rawRows[i], current = strRows.get(key(r)); a(current); const {r: s, f: sf} = current;
  a.equal(f[0], r.fixed); a.equal(f[1], String(r.index)); a.equal(f[2], r.bits);
  a.equal(f[3], String(r.setting)); a.equal(f[4], 'ORIGINAL'); a.equal(f[5], r.argumentBits);
  a.equal(f[6], r.kind); a.equal(f[7], String(r.count));
  a.equal(r.argumentBits, s.bits); a.equal(r.bits, s.bits); a.equal(r.setting, s.setting);
  a.deepEqual(metadata(f[8]), s.originalMeta); a.deepEqual(metadata(f[17]), s.originalMeta);
  a.equal(f[9], s.bits); a.equal(f[18], s.bits);
  a.equal(f[10], r.displayHex); a.equal(r.displayHex, s.originalDisplayHex); a.equal(f[19], r.displayHex);
  a.equal(f[11], r.strHex); a.equal(r.strHex, s.originalStrHex);
  a.equal(f[12], 'N'); a.equal(f[13], sf[9]); a.equal(f[13], r.appliedDecimalHex);
  a.equal(f[14], 'C'); a.equal(f[15], sf[11]); a.equal(f[15], r.appliedFixedHex);
  const control = Object.fromEntries(f[16].split(';').map(x => x.split('='))); a.deepEqual(control, r.crt);
  a.equal(control.real, s.bits); for (const k of runtimeKeys) a.equal(control[k], s.crt[k]);
  checked.add(key(r));
  if (r.kind !== 'F' || ![15, 16, 17, 18].includes(r.count)) continue;
  a.equal(r.count, r.setting); a.equal(r.strHex, s['str' + r.count + 'Hex']);
  a.equal(control.kind, 'F'); a.equal(parseInt(control.count, 16), r.count);
  if (r.count === 17) a.deepEqual(control, s.crt);
  overlap.push({r, s});
}
a.equal(checked.size, 528); a.equal(overlap.length, 96);
function normalize(n, e, negative) {
  const zero = n === 0n; if (negative) n = -n;
  if (zero) e = 0n; else while (n % 10n === 0n) { n /= 10n; ++e; }
  return {coefficient: String(n), exponent: String(e), zero, negative};
}
function decimal(text) {
  const m = text.trim().match(/^([+-]?)(\d+)(?:\.(\d*))?(?:[eE]([+-]?\d+))?$/); a(m);
  return normalize(BigInt(m[2] + (m[3] || '')), BigInt(m[4] || '0') - BigInt((m[3] || '').length), m[1] === '-');
}
function crtValue(c) {
  const digits = ascii(c.digits); a.match(digits, /^[0-9]*$/);
  a.match(c.decimal, /^[0-9A-F]{8}$/); a.match(c.sign, /^[0-9A-F]{8}$/);
  let dec = BigInt('0x' + c.decimal); if (dec >= 0x80000000n) dec -= 0x100000000n;
  const negative = BigInt('0x' + c.sign) !== 0n;
  // Preserve empty strings as a distinct native observation. Do not invent digits
  // or infer an expected zero result from a return without digits.
  return {digits, decimal: String(dec), negative, value: digits === '' ? null : normalize(BigInt(digits), dec - BigInt(digits.length), negative)};
}
const sameValue = (x, y) => x.coefficient === y.coefficient && x.exponent === y.exponent;
// Sanity checks for the comparison representation only, not a VFP formatter.
let metricSelfChecks = 0;
for (const [text, coefficient, exponent, negative] of [
  ['0', '0', '0', false], [' -0.000 ', '0', '0', true],
  ['3.1415927', '31415927', '-7', false], ['-12.3400e+2', '-1234', '0', true],
  ['0.0000000000000005', '5', '-16', false]
]) { const v = decimal(text); a.equal(v.coefficient, coefficient); a.equal(v.exponent, exponent); a.equal(v.negative, negative); ++metricSelfChecks; }
for (const [digits, dec, sign, text] of [
  ['31415927', '00000001', '00000000', '3.1415927'],
  ['50', 'FFFFFFF1', '00000001', '-0.0000000000000005'],
  ['000', '00000001', '00000001', '-0']
]) { const v = crtValue({digits: Buffer.from(digits).toString('hex').toUpperCase(), decimal: dec, sign});
  a.deepEqual(v.value, decimal(text)); ++metricSelfChecks; }
a.equal(crtValue({digits: '', decimal: 'FFFFFFF1', sign: '00000000'}).value, null); ++metricSelfChecks;
const comparisons = [];
for (const count of [15, 16, 17, 18]) for (const fixed of ['OFF', 'ON']) {
  const rows = overlap.filter(x => x.r.count === count && x.r.fixed === fixed).map(({r, s}) => {
    const native = crtValue(r.crt), text = ascii(s['str' + count + 'Hex']), display = ascii(s.originalDisplayHex);
    const strValue = decimal(text), displayValue = decimal(display);
    return {index: r.index, bits: r.bits, setting: r.setting, strHex: s['str' + count + 'Hex'], displayHex: s.originalDisplayHex,
      native, strValue, displayValue, leadingStrSpaces: text.length - text.trimStart().length,
      strVsDisplayTrimmedLiteralDifferent: text.trimStart() !== display,
      strVsDisplayValueDifferent: !sameValue(strValue, displayValue),
      strVsCrtValueDifferent: native.value === null ? null : !sameValue(strValue, native.value),
      displayVsCrtValueDifferent: native.value === null ? null : !sameValue(displayValue, native.value),
      strVsCrtZeroSignDifferent: native.value && native.value.zero && strValue.zero ? native.negative !== strValue.negative : null};
  });
  a.equal(rows.length, 12);
  comparisons.push({count, fixed, contexts: rows.length, emptyCrt: rows.filter(r => r.native.value === null).length,
    nonemptyCrt: rows.filter(r => r.native.value !== null).length,
    strVsCrtValueDifferent: rows.filter(r => r.strVsCrtValueDifferent === true).length,
    displayVsCrtValueDifferent: rows.filter(r => r.displayVsCrtValueDifferent === true).length,
    strVsCrtZeroSignDifferent: rows.filter(r => r.strVsCrtZeroSignDifferent === true).length,
    strVsDisplayValueDifferent: rows.filter(r => r.strVsDisplayValueDifferent).length,
    strVsDisplayTrimmedLiteralDifferent: rows.filter(r => r.strVsDisplayTrimmedLiteralDifferent).length, rows});
}
console.log(JSON.stringify({analysisOnly: true, nativeInvoked: false, ordinaryShellNotReaEvidence: true,
  sourceHashes: {str: strHash, crt: crtHash}, metricSelfChecks, originalContextsChecked: checked.size, fixedCountOverlap: overlap.length,
  abiInterpretation: 'Microsoft public _fcvt digits/decimal-point/sign contract; modern documentation is interpretation, not installed-runtime behavior proof. Empty native digit returns remain separate without a numeric equality claim.',
  documentation: 'https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fcvt?view=msvc-170', comparisons,
  limitations: 'Only 12 inputs per count/FIXED at applied D15..18, not all counts at every payload or new runtime qualification. Exact decimal value metric separately ignores padding and signed-zero spelling. No tolerance, binary Number(text), native replay, formatter, expected strings or internal-call attribution.'}, null, 2));
