// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: offline classification AFTER capture.
// Observed graph operands are diagnostic stimuli, never a formatter or requirement.
const fs = require('node:fs'), path = require('node:path'), z = require('node:zlib');
const a = require('node:assert/strict'), crypto = require('node:crypto');
const sha = b => crypto.createHash('sha256').update(b).digest('hex');
const bytes = (dir, n) => z.gunzipSync(fs.readFileSync(path.join(dir, n + '.gz')));
const ascii = h => { a.match(h, /^(?:[0-9A-F]{2})*$/); const b = Buffer.from(h, 'hex'); a(b.every(n => n > 0 && n < 128)); return b.toString('ascii'); };
function load(name, digest, fields, extra = []) {
  const dir = path.join(__dirname, name), report = JSON.parse(bytes(dir, 'audit.json'));
  a.equal(report.recordsPerRun, 528); a.equal(report.observations.length, 528);
  a.equal(report.documentedFieldsRepeatEqual, true); a.equal(report.rawByteEqual, true);
  const inventory = new Map();
  for (const line of fs.readFileSync(path.join(dir, 'SHA256SUMS'), 'utf8').trimEnd().split('\n')) {
    const m = line.match(/^([0-9a-f]{64})  ([A-Za-z0-9_.-]+)$/); a(m); a(!inventory.has(m[2])); inventory.set(m[2], m[1]);
  }
  for (const n of ['audit.json', 'first-results.txt', 'repeat-results.txt', ...extra]) {
    a(inventory.has(n + '.gz')); a.equal(sha(fs.readFileSync(path.join(dir, n + '.gz'))), inventory.get(n + '.gz'));
  }
  for (const run of ['first', 'repeat']) {
    a.equal(report[run + 'Sha256'], digest); a.equal(sha(bytes(dir, run + '-results.txt')), digest);
    a.deepEqual(bytes(dir, run + '-results.txt'), bytes(dir, run + '.out'));
    a.equal(bytes(dir, run + '.status').toString(), '0\n'); a.equal(bytes(dir, run + '.stderr').length, 0);
  }
  const lines = bytes(dir, 'first-results.txt').toString('ascii').split('\r\n');
  a.equal(lines.pop(), ''); a.equal(lines.shift(), 'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
  a.equal(lines.pop(), 'DONE|528'); a.equal(lines.length, 528);
  const raw = lines.map(s => { const f = s.split('|'); a.equal(f.length, fields); return f; });
  return {dir, report, raw};
}
const graphHash = '1979626f4fec992924791da5cbc89ed3abef1583810690300fff1fd3cf95ea32';
const strHash = '13fba5c3663d1d44f0cf514d481abfc66e16063e3e300d1c8e33502b10282c6c';
const graph = load('arithmetic-inline-captures1', graphHash, 66, ['comparison.json']);
const str = load('str-middle-captures1', strHash, 32);
const retained = JSON.parse(bytes(graph.dir, 'comparison.json'));
a.equal(retained.analysisOnly, true); a.equal(retained.sourceSha256, graphHash);
a.equal(retained.comparedOnMagnitudes, 264); a.equal(retained.publicInlineGraphRejected, true);
a.equal(retained.mismatches.length, 18); a.equal(retained.allMagnitudeComparisons.length, 264);
for (const k of ['typedResultAndControls', 'inlineArithmeticAndStoredParentControls', 'exactRuntimeIdentity']) a.equal(graph.report[k], true);
for (const k of ['originalAndNativeParentControls', 'parentStrBeforeAfterExact', 'inlineMetadataAndBRSExact', 'publicStrTypesAndWidths']) a.equal(str.report[k], true);
function rational(bits) {
  a.match(bits, /^[0-9A-F]{16}$/); const b = BigInt('0x' + bits), e = (b >> 52n) & 2047n;
  a.notEqual(e, 2047n); let n = (b & ((1n << 52n) - 1n)) + (e ? 1n << 52n : 0n);
  if (b >> 63n) n = -n;
  const p = e ? e - 1075n : -1074n;
  return p >= 0n ? {n: n << p, d: 1n} : {n, d: 1n << -p};
}
const add = (x, y) => ({n: x.n * y.d + y.n * x.d, d: x.d * y.d});
const multiply = (x, y) => ({n: x.n * y.n, d: x.d * y.d});
const magnitude = x => ({n: x.n < 0n ? -x.n : x.n, d: x.d});
const compare = (x, y) => x.n * y.d < y.n * x.d ? 'less' : x.n * y.d > y.n * x.d ? 'greater' : 'equal';
const half = {n: 1n, d: 2n}, one = {n: 1n, d: 1n};
function integer(bits) { const x = rational(bits); a.equal(x.n % x.d, 0n); const n = x.n / x.d; a(n >= 0n && n <= 9007199254740991n); return n; }
function decimal(text) {
  const m = text.trim().match(/^(-?)(\d+)(?:\.(\d*))?$/); a(m);
  const n = BigInt(m[2] + (m[3] || '')); return {n: m[1] ? -n : n, d: 10n ** BigInt((m[3] || '').length), negative: m[1] === '-'};
}
function activeMeta(s, bits) {
  a.match(s, /^type=(?:4E|49);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
  const m = Object.fromEntries(s.split(';').map(x => x.split('='))); rational(bits);
  if (m.type === '4E') { a.equal(m.real, bits); delete m.integer; }
  else { let n = BigInt('0x' + m.integer); if (n >= 0x80000000n) n -= 0x100000000n;
    const x = rational(bits); a.equal(x.n, n * x.d); delete m.real; delete m.decimals; }
  return m;
}
let metricSelfChecks = 0;
for (const [bits, expected] of [['3FDFFFFFFFFFFFFF', 'less'], ['3FE0000000000000', 'equal'], ['3FE0000000000001', 'greater']]) {
  a.equal(compare(rational(bits), half), expected); ++metricSelfChecks;
}
a.equal(compare(rational('0000000000000001'), {n: 1n, d: 1n << 1074n}), 'equal'); ++metricSelfChecks;
a.equal(compare(rational('BFE0000000000000'), {n: -1n, d: 2n}), 'equal'); ++metricSelfChecks;
a.equal(integer('4022000000000000'), 9n); ++metricSelfChecks;
a.equal(compare(magnitude(decimal('-0.000000001')), {n: 1n, d: 1000000000n}), 'equal'); ++metricSelfChecks;
a.equal(decimal('-0.00').negative, true); ++metricSelfChecks;
a.equal(compare(add(rational('3FDFFFFFFFFFFFFF'), half), one), 'less'); ++metricSelfChecks;
const key = r => JSON.stringify([r.index, r.fixed]);
const stageNames = ['powDown', 'bias', 'shifted', 'effectiveCount', 'powUp', 'scaled', 'raised', 'quantized'];
const strRows = new Map();
for (const [i, r] of str.report.observations.entries()) {
  const f = str.raw[i]; a.equal(f[0], r.fixed); a.equal(f[1], String(r.index)); a.equal(f[2], r.bits); a.equal(f[3], String(r.setting));
  a.equal(f[6], r.originalDisplayHex); a.equal(f[7], r.originalStrHex); a.deepEqual(activeMeta(f[4], r.bits), r.originalMeta);
  for (const [count, field] of [[15, 15], [16, 25], [17, 27], [18, 17]]) a.equal(f[field], r['str' + count + 'Hex']);
  a(!strRows.has(key(r))); strRows.set(key(r), {r, f});
}
a.equal(strRows.size, 528);
const retainedRows = new Map(retained.allMagnitudeComparisons.map(r => [r.index, r])); a.equal(retainedRows.size, 264);
const seen = new Set(), comparisons = [], rejected = [];
for (const [i, r] of graph.report.observations.entries()) {
  const f = graph.raw[i], k = key(r); a(!seen.has(k)); seen.add(k); const current = strRows.get(k); a(current); const {r: s, f: sf} = current;
  a.equal(f[0], r.fixed); a.equal(f[1], String(r.index)); a.equal(f[2], r.bits); a.equal(f[3], String(r.setting));
  a.equal(r.bits, s.bits); a.equal(r.setting, s.setting);
  for (const n of ['originalDisplayHex', 'originalStrHex']) a.equal(r[n], s[n]);
  for (const j of [4, 18]) a.deepEqual(activeMeta(f[j], r.bits), s.originalMeta);
  for (const j of [5, 19]) a.equal(f[j], r.bits);
  a.equal(f[6], r.originalDisplayHex); a.equal(f[7], r.originalStrHex); a.equal(f[20], r.originalDisplayHex);
  for (const [j, sj] of [[8, 8], [9, 9], [10, 10], [11, 11]]) a.equal(f[j], sf[sj]);
  for (const j of [12, 21]) a.deepEqual(Object.fromEntries(f[j].split(';').map(x => x.split('='))), r.crt);
  a.deepEqual(r.crt, s.crt); a.equal(f[13], r.resultVarType); a.equal(f[15], r.resultBits); a.equal(f[23], r.resultBits);
  for (const j of [14, 22]) a.deepEqual(activeMeta(f[j], r.resultBits), r.resultMeta);
  for (const [j, name] of [[16, 'resultDisplayHex'], [17, 'resultStrHex'], [24, 'resultDisplayHex'], [25, 'resultStrHex']]) a.equal(f[j], r[name]);
  a.deepEqual(Object.keys(r.arithmetic), stageNames);
  for (const [j, name] of stageNames.entries()) {
    const offset = 26 + 5 * j, stage = r.arithmetic[name]; a.equal(f[offset], 'N'); a.equal(stage.varType, 'N');
    for (const b of [offset + 2, offset + 4]) a.equal(f[b], stage.bits);
    for (const m of [offset + 1, offset + 3]) a.deepEqual(activeMeta(f[m], stage.bits), stage.meta);
    a.equal(stage.type, stage.meta.type === '49' ? 'I' : 'N');
  }
  if (r.fixed !== 'ON') continue;
  const q = integer(r.arithmetic.effectiveCount.bits), quantized = integer(r.arithmetic.quantized.bits); a(q <= 18n);
  const graphMagnitude = {n: quantized, d: 10n ** q}, display = decimal(ascii(r.originalDisplayHex));
  const equal = compare(magnitude(display), graphMagnitude) === 'equal', old = retainedRows.get(r.index); a(old);
  a.equal(old.bits, r.bits); a.equal(old.setting, r.setting); a.equal(String(old.observedCount), String(q));
  a.equal(old.quantizedInteger, String(quantized)); a.equal(old.equal, equal); a.equal(old.observedHex, r.originalDisplayHex);
  const counts = [15, 16, 17, 18].map(count => {
    const value = decimal(ascii(s['str' + count + 'Hex']));
    return {count, hex: s['str' + count + 'Hex'], magnitudeVsDisplay: compare(magnitude(value), magnitude(display)),
      magnitudeVsGraph: compare(magnitude(value), graphMagnitude), negative: value.negative, zero: value.n === 0n};
  });
  const scaled = rational(r.arithmetic.scaled.bits), raised = rational(r.arithmetic.raised.bits);
  const exactSum = add(scaled, half), product = multiply(rational(r.arithmetic.shifted.bits), rational(r.arithmetic.powUp.bits));
  // Mathematical comparison of observed operands only. This is NOT observed
  // execution with a different precision or a newly accepted arithmetic graph.
  const sumProduct = multiply(add(magnitude(rational(r.bits)), rational(r.arithmetic.bias.bits)), rational(r.arithmetic.powUp.bits));
  const item = {index: r.index, setting: r.setting, bits: r.bits, equal, observedCount: String(q), quantizedInteger: String(quantized),
    displayHex: r.originalDisplayHex, currentDStrHex: r.originalStrHex, displayNegative: display.negative, displayZero: display.n === 0n,
    publicRoundBits: r.resultBits, publicRoundDisplayHex: r.resultDisplayHex,
    currentDStrMagnitudeVsDisplay: compare(magnitude(decimal(ascii(r.originalStrHex))), magnitude(display)),
    scaledVsHalf: compare(scaled, half), raisedVsOne: compare(raised, one), exactScaledPlusHalfVsOne: compare(exactSum, one),
    exactShiftedTimesObservedPowUpVsHalf: compare(product, half), counts};
  item.exactAbsInputPlusObservedBiasTimesObservedPowUpVsHalf = compare(sumProduct, half);
  comparisons.push(item); if (!equal) rejected.push({...item, arithmetic: r.arithmetic, crt: r.crt});
}
a.equal(seen.size, 528); a.equal(comparisons.length, 264); a.equal(rejected.length, 18);
a.deepEqual(rejected.map(r => r.index), retained.mismatches.map(r => r.index));
const tally = (rows, field) => Object.fromEntries(['less', 'equal', 'greater'].map(k => [k, rows.filter(r => r[field] === k).length]));
const perCount = [15, 16, 17, 18].map(count => ({count, rejectedContexts: rejected.length,
  magnitudeVsDisplay: tally(rejected.map(r => r.counts.find(c => c.count === count)), 'magnitudeVsDisplay'),
  magnitudeVsGraph: tally(rejected.map(r => r.counts.find(c => c.count === count)), 'magnitudeVsGraph')}));
console.log(JSON.stringify({analysisOnly: true, nativeInvoked: false, ordinaryShellNotReaEvidence: true,
  sourceHashes: {graph: graphHash, str: strHash}, metricSelfChecks, sharedContextsChecked: seen.size,
  onMagnitudesCompared: comparisons.length, rejectedMagnitudes: rejected.length,
  classification: {scaledVsHalf: tally(rejected, 'scaledVsHalf'), raisedVsOne: tally(rejected, 'raisedVsOne'),
    exactScaledPlusHalfVsOne: tally(rejected, 'exactScaledPlusHalfVsOne'),
    exactShiftedTimesObservedPowUpVsHalf: tally(rejected, 'exactShiftedTimesObservedPowUpVsHalf'),
    exactAbsInputPlusObservedBiasTimesObservedPowUpVsHalf: tally(rejected, 'exactAbsInputPlusObservedBiasTimesObservedPowUpVsHalf'),
    zeroDisplays: rejected.filter(r => r.displayZero).length,
    currentDStrMagnitudeEqualsDisplay: rejected.filter(r => r.currentDStrMagnitudeVsDisplay === 'equal').length,
    publicRoundPayloadZero: rejected.filter(r => rational(r.publicRoundBits).n === 0n).length, perCount},
  comparisons, rejected,
  limitations: 'Same264 ORIGINAL inputs/ON magnitudes only, joined528 OFF/ON controls. Exact operations compare already-observed operands, not a proposed input formatter. Diagnostic bias/powers/effective-count cap remain rejected-graph stimuli, not requirements. A below-half-only addition fix cannot explain equal/above-half cases. No fitted constants/tolerance, expected output, new runtime qualification, Windows parity, internal-call sequence or production change.'}, null, 2));
