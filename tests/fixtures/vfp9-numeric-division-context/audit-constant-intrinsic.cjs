// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Post-capture model audit; no Copperfin helper import. --header emits only
// literal captured DECIMALS0 expectations, NEVER model-computed expectations.
const fs = require('node:fs'), path = require('node:path'), cp = require('node:child_process');
const a = require('node:assert/strict');
const root = __dirname;
function run(script, args) {
    return JSON.parse(cp.execFileSync(process.execPath, [path.join(root, script), ...args], {maxBuffer: 32 * 1024 * 1024}));
}
const captures = ['a', 'b'].map(batch => ({batch,
    inventory: JSON.parse(fs.readFileSync(path.join(root, `constant-payload-${batch}-cases.json`))).rows,
    capture: run('audit-constant-payload.cjs', [path.join(root, `constant-payload-${batch}-captures`), batch])}));
const first = {batch: 'grid', inventory: JSON.parse(fs.readFileSync(path.join(root, 'constant-grid-cases.json'))).rows,
    capture: run('audit-constant-grid.cjs', [path.join(root, 'constant-captures')])};
let checked = 0, rejectedSignWidthMismatches = 0;
const mismatches = [];
for (const data of [...captures, first]) for (const observed of data.capture.observations) {
    const row = data.inventory[observed.index - 1];
    if (data.batch === 'grid' && row.group !== 'five-over-two') continue;
    const left = observed.left, right = observed.right;
    let quotient = Number(row.left) / Number(row.right);
    if (quotient === 0) quotient = 0; // Intrinsic folded zero is +0; grouped unary is subsequent.
    const intrinsicScale = Math.min(18, (left.decimals || 0) + (right.decimals || 0));
    const integer = left.type === 'I' && right.type === 'I' && Number.isInteger(quotient) && Math.abs(quotient) <= 2147483647;
    // The rejected version counted a mandatory leading zero before negative
    // fractions. Native folded width can omit that position (e.g. 1/-7).
    const rejectedDigits = String(Math.trunc(Math.abs(quotient))).length + (quotient < 0 ? 1 : 0);
    const quotientDigits = (Math.abs(quotient) >= 1 ? String(Math.trunc(Math.abs(quotient))).length : 0) + (quotient < 0 ? 1 : 0);
    const sourceWhole = left.width - (left.decimals || 0) - ((left.decimals || 0) > 0 ? 1 : 0);
    const scale = integer ? 0 : row.mode === 'group-minus' ? intrinsicScale : Math.max(observed.setting, intrinsicScale);
    const width = Math.max(sourceWhole, quotientDigits) + (scale > 0 ? scale + 1 : 0) + (row.mode === 'group-minus' ? 1 : 0);
    const rejectedWidth = Math.max(sourceWhole, rejectedDigits) + (scale > 0 ? scale + 1 : 0) + (row.mode === 'group-minus' ? 1 : 0);
    if (data.batch !== 'grid' && rejectedWidth !== observed.result.width) ++rejectedSignWidthMismatches;
    const expectedType = integer ? 'I' : 'N';
    if (expectedType !== observed.result.type || width !== observed.result.width || (!integer && scale !== observed.result.decimals)) {
        mismatches.push({batch: data.batch, index: row.index, expression: row.expression,
            setting: observed.setting, predicted: {type: expectedType, width, scale}, actual: observed.result});
    }
    ++checked;
}
a.equal(checked, 15740);
a.equal(rejectedSignWidthMismatches, 560);
a.equal(mismatches.length, 0, JSON.stringify(mismatches.slice(0, 8)));
if (process.argv.includes('--header') || process.argv.includes('--check-header')) {
    const lines = [
        '// Copyright © 2026 Richard M. Hamilton.',
        '// SPDX-License-Identifier: GPL-3.0-only',
        '// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.',
        '// Captured native DECIMALS0/FIXED OFF/EVAL constants, not model-derived.',
        '#pragma once', '#include <cstdint>',
        'struct NativeConstantDivisionRow {',
        '    unsigned batch, index; bool grouped_minus;',
        '    std::uint64_t left_bits; unsigned left_width, left_scale; bool left_integer;',
        '    std::uint64_t right_bits; unsigned right_width, right_scale; bool right_integer;',
        '    std::uint64_t result_bits; unsigned result_width, result_scale; bool result_integer;',
        '};', 'inline constexpr NativeConstantDivisionRow native_constant_division_rows[] = {'
    ];
    for (const data of captures) for (const row of data.inventory) {
        const found = data.capture.observations.filter(x => x.index === row.index && x.setting === 0 && x.fixed === 'OFF' && x.mode === 'EVAL');
        a.equal(found.length, 1);
        const observed = found[0];
        const fields = [data.batch === 'a' ? 0 : 1, row.index, row.mode === 'group-minus',
            ...[observed.left, observed.right, observed.result].flatMap(v => [`0x${v.bits}ULL`, v.width, v.decimals || 0, v.type === 'I'])];
        lines.push('    {' + fields.join(',') + '},');
    }
    lines.push('};', '');
    const generated = lines.join('\n');
    if (process.argv.includes('--check-header')) {
        a.equal(fs.readFileSync(path.join(root, 'constant-intrinsic-expected.h'), 'utf8'), generated);
        console.log('640 captured DECIMALS0 rows exactly match the frozen C++ header; no helper imported.');
    } else process.stdout.write(generated);
} else {
    console.log(JSON.stringify({checked, rejectedSignWidthMismatches, revisedCandidateMismatches: 0,
        inputAudits: captures.map(x => ({batch: x.batch, casesPerRun: x.capture.casesPerRun,
            firstSha256: x.capture.firstSha256, repeatSha256: x.capture.repeatSha256})),
        limitation: 'Direct literal division and grouped minus in the retained domain only. Nested-folding/materialization integration and runtime acceptance remain unproved.'}, null, 2));
}
