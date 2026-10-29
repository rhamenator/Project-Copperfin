// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER-capture indexed join ONLY.
// Equal totals do not establish equal endpoint decisions or boundary membership.
const fs = require('node:fs'), path = require('node:path'), z = require('node:zlib');
const cp = require('node:child_process'), a = require('node:assert/strict'), c = require('node:crypto');
const sha = b => c.createHash('sha256').update(b).digest('hex');
function bound(dir, name) {
  const root = path.join(__dirname, dir), manifest = JSON.parse(fs.readFileSync(path.join(root, 'manifest.json')));
  const item = manifest.files.find(r => r.name === name); a(item);
  const gzip = fs.readFileSync(path.join(root, name + '.gz')), bytes = z.gunzipSync(gzip);
  a.equal(bytes.length, item.bytes); a.equal(sha(bytes), item.sha256);
  if (item.gzipSha256) { a.equal(gzip.length, item.gzipBytes); a.equal(sha(gzip), item.gzipSha256); }
  return bytes;
}
function replay(dir, source, snapshot, report) {
  a.deepEqual(bound(dir, snapshot), fs.readFileSync(path.join(__dirname, source)));
  const run = cp.spawnSync(process.execPath, [path.join(__dirname, source)], {maxBuffer: 12 * 1024 * 1024});
  a.equal(run.status, 0); a.equal(run.stderr.length, 0); a.deepEqual(run.stdout, bound(dir, report));
  return JSON.parse(run.stdout);
}
const pre = replay('preaddition-analysis2', 'compare-preaddition-operands.cjs', 'source-compare-preaddition-operands.cjs', 'analysis.json');
const decimal = replay('half-grid-adjacency-analysis1', 'join-half-grid-adjacency.cjs', 'source-join-half-grid-adjacency.cjs', 'analysis.json');
for (const r of [pre, decimal]) {
  a.equal(r.nativeInvoked, false); a.equal(r.onContexts, 264); a.equal(r.adjacentPairs, 220);
  a.equal(r.sharedContextsChecked, 528); a.equal(r.excludedNonAdjacentGaps, 6);
}
a.equal(pre.metricSelfChecks, 19); a.equal(decimal.metricSelfChecks, 4); a.deepEqual(pre.sourceHashes, decimal.sourceHashes);
const models = decimal.models.filter(r => r.name === 'decimal_prebias_cap15'); a.equal(models.length, 1);
const model = models[0]; a.equal(model.bias, true); a.equal(model.cap, true);
function units(text) { a.match(text, /^(?:0|[1-9][0-9]*)$/); return BigInt(text); }
function indexed(rows, key) {
  const result = new Map();
  for (const row of rows) { const id = key(row); a(!result.has(id)); result.set(id, row); }
  return result;
}
const category = (observed, candidate) => observed ? (candidate ? 'coincidentChangeOnly' : 'missedObservedChange') :
  (candidate ? 'inventedChangeOnObservedPlateau' : 'sharedPlateau');
function membership(left, right) {
  const x = new Set(left), y = new Set(right); a.equal(x.size, left.length); a.equal(y.size, right.length);
  return {leftOnly: [...x].filter(v => !y.has(v)).sort(), rightOnly: [...y].filter(v => !x.has(v)).sort(),
    shared: [...x].filter(v => y.has(v)).sort()};
}
let metricSelfChecks = 0;
function check(fn) { fn(); ++metricSelfChecks; }
for (const [o, p, want] of [[true, true, 'coincidentChangeOnly'], [true, false, 'missedObservedChange'],
  [false, true, 'inventedChangeOnObservedPlateau'], [false, false, 'sharedPlateau']]) check(() => a.equal(category(o, p), want));
check(() => a.equal(units('1000'), 1000n));
check(() => a.equal(units('0'), 0n));
check(() => a.throws(() => units('01')));
check(() => a.throws(() => units('-1')));
check(() => a.deepEqual(membership(['a', 'b'], ['b', 'a']), {leftOnly: [], rightOnly: [], shared: ['a', 'b']}));
check(() => a.deepEqual(membership(['a', 'b'], ['b', 'c']), {leftOnly: ['a'], rightOnly: ['c'], shared: ['b']}));
check(() => a.throws(() => membership(['a', 'a'], ['a'])));
check(() => a.throws(() => indexed([{index: 1}, {index: 1}], r => r.index)));
check(() => a.equal(indexed([{index: 2}, {index: 1}], r => r.index).get(1).index, 1));

const byIndex = indexed(model.endpoints, r => r.index); a.equal(byIndex.size, 264);
const nodes = pre.nodes.map(r => {
  const d = byIndex.get(r.index); a(d);
  for (const name of ['bits', 'setting', 'negative', 'transformHex', 'strHex']) a.equal(r[name], d[name]);
  for (const name of ['transformValue', 'strValue']) a.deepEqual(r[name], d[name]);
  const nativeCount = Buffer.alloc(8); nativeCount.writeDoubleBE(d.quantumCount);
  a.equal(nativeCount.toString('hex').toUpperCase(), r.stages.effectiveCount.bits);
  a.equal(d.quantumCount, Math.min(r.setting, 15));
  const same = units(r.exactPreAdditionHalfUpDecision) === units(d.candidateMagnitudeUnits);
  const preGap = !r.preAdditionDecisionMatchesObservedMagnitude, decimalGap = d.magnitudeVsCandidate !== 'equal';
  if (same) a.equal(preGap, decimalGap);
  return {...r, decimalCandidate: d, decisionsEqual: same, preAdditionMagnitudeGap: preGap, decimalMagnitudeGap: decimalGap};
});
a.equal(nodes.length, 264); a.equal(nodes.filter(r => r.rejectedDiagnosticGraph).length, 18);
const nodeMap = indexed(nodes, r => r.index), pairId = p => p.leftIndex + ':' + p.rightIndex;
const decimalPairs = indexed(model.pairs, pairId); a.equal(decimalPairs.size, 220);
const pairs = pre.pairs.map(p => {
  const d = decimalPairs.get(pairId(p)); a(d);
  for (const name of ['component', 'setting', 'negative', 'leftIndex', 'rightIndex', 'leftBits', 'rightBits']) a.equal(p[name], d[name]);
  a.equal(p.observedDisplayChanges, d.observedChanges);
  const left = nodeMap.get(p.leftIndex), right = nodeMap.get(p.rightIndex); a(left && right);
  a.equal(left.exactPreAdditionHalfUpDecision, p.preAddition.leftDecision);
  a.equal(right.exactPreAdditionHalfUpDecision, p.preAddition.rightDecision);
  const decimalChanges = units(left.decimalCandidate.candidateMagnitudeUnits) !== units(right.decimalCandidate.candidateMagnitudeUnits);
  a.equal(decimalChanges, d.candidateChanges);
  const preCategory = category(p.observedDisplayChanges, p.preAddition.decisionDiffers);
  a.equal(category(p.observedDisplayChanges, decimalChanges), d.category);
  return {...p, decimalCandidate: d, preCategory, categoryEqual: preCategory === d.category,
    bothEndpointDecisionsEqual: left.decisionsEqual && right.decisionsEqual};
});
a.equal(indexed(pairs, pairId).size, 220);
const categories = ['coincidentChangeOnly', 'missedObservedChange', 'inventedChangeOnObservedPlateau', 'sharedPlateau'];
const pairMembership = Object.fromEntries(categories.map(name => [name, membership(
  pairs.filter(r => r.preCategory === name).map(pairId), pairs.filter(r => r.decimalCandidate.category === name).map(pairId))]));
const failedEndpointMembership = membership(nodes.filter(r => r.preAdditionMagnitudeGap).map(r => String(r.index)),
  nodes.filter(r => r.decimalMagnitudeGap).map(r => String(r.index)));
const summarize = rows => ({contexts: rows.length, decisionDifferences: rows.filter(r => !r.decisionsEqual).length,
  magnitudeGapMembershipDifferences: rows.filter(r => r.preAdditionMagnitudeGap !== r.decimalMagnitudeGap).length,
  preAdditionMagnitudeGaps: rows.filter(r => r.preAdditionMagnitudeGap).length, decimalMagnitudeGaps: rows.filter(r => r.decimalMagnitudeGap).length});
console.log(JSON.stringify({analysisOnly: true, nativeInvoked: false, ordinaryShellNotReaEvidence: true,
  metricSelfChecks, replayedPreadditionMetricChecks: 19, replayedDecimalJoinMetricChecks: 4,
  replayedDecimalCandidateMetricChecks: 14, replayedMeasuredOperandMetricChecks: 14,
  replayedStageMetricChecks: 9, replayedInventoryMetricChecks: 8, replayedClassificationMetricChecks: 9,
  reusedRawValidatorMetricChecks: 8, sharedContextsChecked: 528, onContexts: 264, adjacentPairs: 220, excludedNonAdjacentGaps: 6,
  sourceHashes: pre.sourceHashes, summary: {endpoints: summarize(nodes), rejectedGraph: summarize(nodes.filter(r => r.rejectedDiagnosticGraph)),
    pairs: {comparisons: 220, categoryDifferences: pairs.filter(r => !r.categoryEqual).length,
      endpointDecisionDifferences: pairs.filter(r => !r.bothEndpointDecisionsEqual).length}, failedEndpointMembership, pairMembership},
  bySetting: [...new Set(nodes.map(r => r.setting))].sort((x, y) => x - y).map(setting => ({setting, ...summarize(nodes.filter(r => r.setting === setting))})),
  nodes, pairs, limitation: 'Index-exact comparison of two ALREADY REJECTED hypothetical quantizers, not a new candidate. Exact pre-addition measured BINARY bias/powUp versus true DECIMAL bias/powers with cap15; native observed count is independently matched. Matching decisions/membership only covers retained264 ON endpoints/220 pairs/176 plateaus and18 graph rejections, with528 OFF/ON raw controls and six excluded gaps. All original literal/sign/active metadata observations retained, not predicted. Equal decisions do NOT mean equal exact operands, internal algorithm/precision/order or global equivalence. No fitted threshold, native capture, expected emitter, formatter/production change, Windows/internal-call/atomic-image/cleanup proof, parent closure or ORIGINAL/SAME relaxation.'}, null, 2));
