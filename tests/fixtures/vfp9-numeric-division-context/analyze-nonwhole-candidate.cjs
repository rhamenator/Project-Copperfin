// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// POST-CAPTURE HYPOTHESIS, not a native expected-value emitter or product oracle.
// Extends the prior native whole-operand contract; imports no Copperfin code.
const fs = require('node:fs'), a = require('node:assert/strict');
const inventory = JSON.parse(fs.readFileSync(__dirname + '/nonwhole-grid-cases.json'));
const observed = JSON.parse(fs.readFileSync(process.argv[2]));
const index = new Map(inventory.rows.map(r => [r.index, r]));
const bits = n => { const b = Buffer.alloc(8); b.writeDoubleBE(n); return b.toString('hex').toUpperCase(); };
const payload = hex => Buffer.from(hex, 'hex').readDoubleBE();
const literal = new Map(), literalParseDifferences = [];
for (const row of inventory.rows.filter(r => r.kind === 'literal-control')) {
    const samples = observed.observations.filter(o => o.index === row.index);
    a.equal(samples.length, 20);
    const first = samples[0].result;
    for (const s of samples) a.deepEqual(s.result, first);
    literal.set(row.literal, {...first, number:payload(first.bits)});
    if (bits(Number(row.literal)) !== first.bits)
        literalParseDifferences.push({literal:row.literal,native:first.bits,independentBinary64:bits(Number(row.literal))});
}
function negate(v) {
    const n = v.type === 'I' ? (v.number === 0 ? 0 : -v.number)
        : v.number === 0 ? -0 : -v.number;
    return {...v,width:v.width+1,number:n,bits:bits(n)};
}
function intrinsic(node) {
    if (node.kind === 'literal') { a(literal.has(node.text)); return literal.get(node.text); }
    if (node.kind === 'negate') return negate(intrinsic(node.operand));
    const l = intrinsic(node.left), r = intrinsic(node.right);
    const number = l.number / r.number;
    const integer = l.type === 'I' && r.type === 'I' && Number.isInteger(number) && Math.abs(number) <= 2147483647;
    const dl = l.type === 'I' ? 0 : l.decimals, dr = r.type === 'I' ? 0 : r.decimals;
    const d = integer ? 0 : Math.min(18, dl + dr);
    const whole = Math.abs(number) >= 1 ? String(Math.trunc(Math.abs(number))).length : 0;
    const width = Math.max(l.width-dl-(dl>0?1:0),whole+(number<0?1:0))+d+(d>0?1:0);
    return {type:integer?'I':'N',width,...(!integer?{decimals:d}:{}),
        number:number===0?0:number,bits:bits(number===0?0:number),cw:'027F'};
}
function materialize(v, setting) {
    if (v.type === 'I') return v;
    const d = Math.max(v.decimals, setting);
    return {...v,width:v.width+d-v.decimals+(v.decimals===0&&d>0?1:0),decimals:d};
}
function candidate(core, wrapper, setting) {
    const v = intrinsic(core.tree);
    if (wrapper === 'minus') return negate(v);
    if (wrapper === 'double-minus') return negate(negate(v));
    const m = materialize(v, setting);
    return ['minus-iif','minus-evaluate'].includes(wrapper) ? negate(m) : m;
}
const differences = [], cores = new Map(inventory.cores.map(c => [c.id,c]));
let compared = 0, descriptorDifferences = 0, payloadDifferences = 0;
for (const observation of observed.observations) {
    const row = index.get(observation.index);
    if (row.kind !== 'core') continue;
    const predicted = candidate(cores.get(row.core), row.wrapper, observation.setting);
    const actual = observation.result;
    ++compared;
    const descriptor = ['type','width','decimals'].some(k => predicted[k] !== actual[k]);
    const value = predicted.bits !== actual.bits;
    if (descriptor) ++descriptorDifferences;
    if (value) ++payloadDifferences;
    if (descriptor || value) differences.push({expression:row.expression,setting:observation.setting,
        fixed:observation.fixed,mode:observation.mode,predicted,actual,descriptor,value});
}
a.equal(compared, 68*8*20);
console.log(JSON.stringify({candidate:'Prior native intrinsic width/scale/storage rule extended to non-whole operands; sequential binary64 payload hypothesis',
    compared, descriptorDifferences,payloadDifferences,literalParseDifferences,
    limitations:'Hypothesis only; separately captured literal inputs, not standalone child division descriptors. Runtime mixtures/outside-scope controls not modeled. Zero side-effect/continuation or product verification claim.',differences},null,2));
