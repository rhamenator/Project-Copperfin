// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER-capture discriminator only.
// Preserving an exact sum is a hypothesis, NOT observed formatter execution.
const fs = require('node:fs'), path = require('node:path'), z = require('node:zlib');
const cp = require('node:child_process'), a = require('node:assert/strict'), c = require('node:crypto');
const sha = b => c.createHash('sha256').update(b).digest('hex');
function bound(dir, name) {
  const root = path.join(__dirname, dir), m = JSON.parse(fs.readFileSync(path.join(root, 'manifest.json')));
  const r = m.files.find(r => r.name === name); a(r);
  const b = z.gunzipSync(fs.readFileSync(path.join(root, name + '.gz')));
  a.equal(b.length, r.bytes); a.equal(sha(b), r.sha256); return b;
}
function replay(dir, source, snapshot, result) {
  const b = bound(dir, snapshot); a.deepEqual(b, fs.readFileSync(path.join(__dirname, source)));
  const run = cp.spawnSync(process.execPath, [path.join(__dirname, source)], {maxBuffer: 8 * 1024 * 1024});
  a.equal(run.status, 0); a.equal(run.stderr.length, 0); a.deepEqual(run.stdout, bound(dir, result));
  return JSON.parse(run.stdout);
}
const previous = replay('collapsed-operands-analysis1', 'compare-collapsed-stage-operands.cjs',
  'pass2-source-compare-collapsed-stage-operands.cjs', 'pass2-analysis.json');
a.equal(previous.nativeInvoked, false); a.equal(previous.metricSelfChecks, 14);
a.equal(previous.onContexts, 264); a.equal(previous.adjacentPairs, 220); a.equal(previous.excludedNonAdjacentGaps, 6);
// The replay above binds source+raw stage and original controls transitively.
// Bind the graph classification as well, keeping ALL18 rejected contexts visible.
const classified = replay('graph-rejection-classification1', 'classify-graph-rejections.cjs',
  'pass2-source-classify-graph-rejections.cjs', 'pass2-classification.json');
a.equal(classified.sharedContextsChecked, 528); a.equal(classified.onMagnitudesCompared, 264);
a.deepEqual(classified.sourceHashes, {graph: previous.sourceHashes.arithmetic, str: previous.sourceHashes.str});
const classes = new Map(classified.comparisons.map(r => [r.index, r])); a.equal(classes.size, 264);
const rejected = new Set(classified.rejected.map(r => r.index)); a.equal(rejected.size, 18);

function binary(bits) {
  a.match(bits, /^[0-9A-F]{16}$/); const b = BigInt('0x' + bits), e = (b >> 52n) & 2047n;
  a.notEqual(e, 2047n); let n = (b & ((1n << 52n) - 1n)) + (e ? 1n << 52n : 0n);
  if (b >> 63n) n = -n;
  const p = e ? e - 1075n : -1074n;
  return p >= 0n ? {n: n << p, d: 1n} : {n, d: 1n << -p};
}
const add = (x, y) => ({n: x.n * y.d + y.n * x.d, d: x.d * y.d});
const subtract = (x, y) => add(x, {n: -y.n, d: y.d});
const multiply = (x, y) => ({n: x.n * y.n, d: x.d * y.d});
const abs = x => ({n: x.n < 0n ? -x.n : x.n, d: x.d});
const cmp = (x, y) => x.n * y.d < y.n * x.d ? 'less' : x.n * y.d > y.n * x.d ? 'greater' : 'equal';
const half = {n: 1n, d: 2n}, zero = {n: 0n, d: 1n};
function packed(x) {
  let n = x.n < 0n ? -x.n : x.n, d = x.d;
  while (d) { const t = n % d; n = d; d = t; }
  const gcd = n || 1n; return {numerator: String(x.n / gcd), denominator: String(x.d / gcd)};
}
function floor(x) { a(x.n >= 0n && x.d > 0n); return String(x.n / x.d); }
const decision = x => floor(add(x, half));
function decimalValue(v) {
  const e = BigInt(v.exponent), n = BigInt(v.coefficient);
  return e >= 0n ? {n: n * 10n ** e, d: 1n} : {n, d: 10n ** -e};
}
function pairResult(l, r) {
  return {ordering: cmp(l, r), leftDecision: decision(l), rightDecision: decision(r), decisionDiffers: decision(l) !== decision(r)};
}
let metricSelfChecks = 0;
function check(fn) { fn(); ++metricSelfChecks; }
for (const [bits, want] of [['3FDFFFFFFFFFFFFF', 'less'], ['3FE0000000000000', 'equal'], ['3FE0000000000001', 'greater']])
  check(() => a.equal(cmp(binary(bits), half), want));
check(() => a.equal(cmp(binary('0000000000000001'), {n: 1n, d: 1n << 1074n}), 'equal'));
check(() => a.equal(cmp(abs(binary('BFE0000000000000')), half), 'equal'));
check(() => a.equal(cmp(abs(binary('8000000000000000')), zero), 'equal'));
check(() => a.deepEqual(packed({n: -6n, d: 8n}), {numerator: '-3', denominator: '4'}));
check(() => a.deepEqual(packed({n: 0n, d: 8n}), {numerator: '0', denominator: '1'}));
check(() => a.deepEqual(packed(subtract({n: 3n, d: 4n}, half)), {numerator: '1', denominator: '4'}));
check(() => a.equal(cmp(add(binary('3FF0000000000000'), binary('3CA0000000000000')), binary('3FF0000000000000')), 'greater'));
check(() => a.equal(cmp(multiply({n: 1n, d: 4n}, {n: 2n, d: 1n}), half), 'equal'));
check(() => a.equal(decision(binary('3FDFFFFFFFFFFFFF')), '0'));
check(() => a.equal(decision(half), '1'));
check(() => a.equal(decision({n: 201n, d: 2n}), '101'));
check(() => a.equal(pairResult(binary('3FDFFFFFFFFFFFFF'), half).decisionDiffers, true));
check(() => a.equal(pairResult(half, half).decisionDiffers, false));
check(() => a.equal(cmp(decimalValue({coefficient: '-123', exponent: '-2'}), {n: -123n, d: 100n}), 'equal'));
check(() => a.equal(cmp(decimalValue({coefficient: '123', exponent: '2'}), {n: 12300n, d: 1n}), 'equal'));
check(() => a.throws(() => binary('7FF0000000000000')));

const exact = new Map();
const nodes = previous.nodes.map(r => {
  const s = r.stages, old = classes.get(r.index); a(old); a.equal(old.bits, r.bits); a.equal(old.setting, r.setting);
  a.equal(old.displayHex, r.transformHex); a.equal(old.currentDStrHex, r.strHex); a.equal(old.equal, !rejected.has(r.index));
  const original = abs(binary(r.bits)), bias = binary(s.bias.bits), shifted = binary(s.shifted.bits), powUp = binary(s.powUp.bits);
  a(original.n >= 0n && bias.n > 0n && shifted.n > 0n && powUp.n > 0n);
  const sum = add(original, bias), pre = multiply(sum, powUp), post = multiply(shifted, powUp), error = subtract(sum, shifted);
  // Same post-addition product as the already completed comparison, NOT a new finding.
  a.deepEqual(packed(post), r.exactMeasuredShiftedTimesMeasuredPowUp);
  const q = binary(s.effectiveCount.bits); a.equal(q.n % q.d, 0n); const count = q.n / q.d; a(count >= 0n && count <= 18n);
  a.equal(String(count), old.observedCount);
  const observed = abs(decimalValue(r.transformValue)), str = abs(decimalValue(r.strValue));
  a.equal(cmp(observed, str), 'equal');
  const scaledDecision = n => ({n: BigInt(n), d: 10n ** count});
  const preDecision = decision(pre), postDecision = decision(post);
  exact.set(r.index, {pre, post, sum, shifted});
  return {...r, rejectedDiagnosticGraph: rejected.has(r.index),
    exactAbsOriginalPlusMeasuredBias: packed(sum), exactSumMinusMeasuredShifted: packed(error),
    exactSumVsMeasuredShifted: cmp(sum, shifted), exactSumTimesMeasuredPowUp: packed(pre),
    exactPreAdditionHalfUpDecision: preDecision, exactPostAdditionHalfUpDecision: postDecision,
    additionRoundingChangesDecision: preDecision !== postDecision,
    preAdditionDecisionMatchesObservedMagnitude: cmp(scaledDecision(preDecision), observed) === 'equal',
    postAdditionDecisionMatchesObservedMagnitude: cmp(scaledDecision(postDecision), observed) === 'equal'};
});
a.equal(exact.size, 264);
const pairs = previous.pairs.map(p => {
  const l = exact.get(p.leftIndex), r = exact.get(p.rightIndex); a(l && r);
  return {...p, preAddition: pairResult(l.pre, r.pre), postAddition: pairResult(l.post, r.post),
    exactSumOrdering: cmp(l.sum, r.sum), measuredShiftedOrdering: cmp(l.shifted, r.shifted)};
});
a.equal(pairs.length, 220);
for (const p of pairs) a.deepEqual(p.postAddition, {
  ordering: p.exactProduct.ordering, leftDecision: p.exactProduct.leftDecision,
  rightDecision: p.exactProduct.rightDecision, decisionDiffers: p.exactProduct.decisionDiffers});
function summarize(rows) {
  return {pairs: rows.length, ...Object.fromEntries(['preAddition', 'postAddition'].map(name => [name, {
    distinct: rows.filter(r => r[name].ordering !== 'equal').length,
    decisionDiffers: rows.filter(r => r[name].decisionDiffers).length,
    boundaryAgreesWithObserved: rows.filter(r => r[name].decisionDiffers === r.observedDisplayChanges).length}])),
    additionRoundingChangesBoundaryDecision: rows.filter(r => r.preAddition.decisionDiffers !== r.postAddition.decisionDiffers).length};
}
const subsets = {observedChanges: pairs.filter(p => p.observedDisplayChanges), observedPlateaus: pairs.filter(p => !p.observedDisplayChanges),
  scaledTupleCollapses: pairs.filter(p => p.stages.scaled.collapseAcrossObservedChange),
  raisedTupleCollapses: pairs.filter(p => p.stages.raised.collapseAcrossObservedChange)};
a.equal(subsets.observedChanges.length, 44); a.equal(subsets.observedPlateaus.length, 176);
a.equal(subsets.scaledTupleCollapses.length, 2); a.equal(subsets.raisedTupleCollapses.length, 12);
const tally = rows => ({contexts: rows.length,
  exactSumVsMeasuredShifted: Object.fromEntries(['less', 'equal', 'greater'].map(k => [k, rows.filter(r => r.exactSumVsMeasuredShifted === k).length])),
  additionRoundingChangesDecision: rows.filter(r => r.additionRoundingChangesDecision).length,
  preAdditionMagnitudeGaps: rows.filter(r => !r.preAdditionDecisionMatchesObservedMagnitude).length,
  postAdditionMagnitudeGaps: rows.filter(r => !r.postAdditionDecisionMatchesObservedMagnitude).length});
console.log(JSON.stringify({analysisOnly: true, nativeInvoked: false, ordinaryShellNotReaEvidence: true,
  metricSelfChecks, replayedMeasuredOperandMetricChecks: 14, replayedStageMetricChecks: 9,
  replayedInventoryMetricChecks: 8, replayedClassificationMetricChecks: 9, reusedRawValidatorMetricChecks: 8,
  sharedContextsChecked: 528, onContexts: 264, adjacentPairs: 220, excludedNonAdjacentGaps: 6,
  sourceHashes: previous.sourceHashes, nodeSummary: {all: tally(nodes), rejectedGraph: tally(nodes.filter(r => r.rejectedDiagnosticGraph))},
  summary: Object.fromEntries(Object.entries(subsets).map(([name, rows]) => [name, summarize(rows)])), nodes, pairs,
  limitation: 'Exact ABS(original)+MEASURED BINARY bias, preserving that sum before multiplying by MEASURED powUp, is a mathematical discriminator of addition loss, not the rejected exact DECIMAL bias candidate or previously completed rounded-shifted-product analysis. All264 ON endpoints/220 adjacent pairs/176 plateaus, signed Character literals and active tuples,528 OFF/ON raw/control joins and18 graph rejections retained; six uncaptured gaps excluded. Hypothetical half-up decisions/count cap belong to independently evaluated REJECTED graph stimuli, NOT recovered TRANSFORM internals/order/precision. Magnitude equality excludes padding/sign, which remain in full retained nodes. No tolerance/fit, native replay, expected emitter, accepted formatter, production change, Windows/internal-call/atomic-image/cleanup proof or original/SAME relaxation.'}, null, 2));
