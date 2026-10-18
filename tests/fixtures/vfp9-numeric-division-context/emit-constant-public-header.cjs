// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Emit/check captured native constants only. No Copperfin helper import or
// width/scale algorithm; public producer scope cites earlier literal contracts.
const fs = require('node:fs'), path = require('node:path'), cp = require('node:child_process');
const a = require('node:assert/strict');
const root = __dirname;
const read = name => JSON.parse(fs.readFileSync(path.join(root, name)));
const run = (name, args) => JSON.parse(cp.execFileSync(process.execPath, [path.join(root, name), ...args], {maxBuffer: 32 * 1024 * 1024}));
function knownLiteral(text) {
    const unsigned = text.replace(/^-/, '');
    // Existing recovered literal producer domains, not type inference from a double.
    if (/^[0-9]{1,10}$/.test(unsigned)) return Number(unsigned) <= 2147483647;
    return /^[0-9]+\.[0-9]{1,18}$/.test(unsigned) && unsigned.length <= 40;
}
const collections = ['a', 'b'].map(batch => ({label: `payload-${batch}`,
    inventory: read(`constant-payload-${batch}-cases.json`).rows,
    capture: run('audit-constant-payload.cjs', [path.join(root, `constant-payload-${batch}-captures`), batch])}));
collections.push({label: 'grid', inventory: read('constant-grid-cases.json').rows.filter(row => row.group === 'five-over-two'),
    capture: run('audit-constant-grid.cjs', [path.join(root, 'constant-captures')])});
const cores = new Set(['5/2', '5.00/2.00', '10/2', '0.00/2', '(10/2)/2', '5/(2/2)']);
const wrappers = new Set(['plain', 'plus', 'minus', 'double-minus', 'iif', 'minus-iif', 'evaluate', 'minus-evaluate']);
collections.push({label: 'boundary', inventory: read('constant-boundary-cases.json').rows.filter(row =>
    (cores.has(row.inner) && wrappers.has(row.wrapper)) ||
    (row.wrapper === 'runtime-boundary' && !row.expression.includes('CFNCLONE'))),
    capture: run('audit-constant-boundary.cjs', [path.join(root, 'constant-boundary-captures')])});
const lines = ['// Copyright © 2026 Richard M. Hamilton.', '// SPDX-License-Identifier: GPL-3.0-only',
    '// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.',
    '// Actual native fields, independently reconstructed; no helper-derived expectations.',
    '#pragma once', '#include "copperfin/runtime/prg_engine.h"', '#include <array>', '#include <cstdint>',
    'namespace copperfin::test {', 'struct NativeConstantPublicRow {',
    '    const char* label; const char* expression; std::uint64_t bits;',
    '    std::array<runtime::PrgNumericMetadata, 5> metadata; bool known_literal_producers;',
    '};', 'inline constexpr NativeConstantPublicRow native_constant_public_rows[] = {'];
let count = 0, publicCount = 0;
for (const collection of collections) for (const row of collection.inventory) {
    const observed = [0, 2, 6, 14, 18].map(setting => {
        const matches = collection.capture.observations.filter(x => x.index === row.index && x.setting === setting && x.fixed === 'OFF' && x.mode === 'EVAL');
        a.equal(matches.length, 1);
        const x = matches[0];
        const fixed = collection.capture.observations.find(y => y.index === row.index && y.setting === setting && y.fixed === 'ON' && y.mode === 'COMPILED');
        a.deepEqual(x.result, fixed.result);
        return x.result;
    });
    a(observed.every(x => x.bits === observed[0].bits));
    const metadata = observed.map(x => `{${x.width},${x.decimals || 0},runtime::PrgNumericStorage::${x.type === 'I' ? 'integer' : 'real'}}`);
    const eligible = collection.label === 'boundary' || (knownLiteral(row.left) && knownLiteral(row.right));
    lines.push(`    {${JSON.stringify(collection.label + ':' + row.index)},${JSON.stringify(row.expression)},0x${observed[0].bits}ULL,{{${metadata.join(',')}}},${eligible}},`);
    ++count; if (eligible) ++publicCount;
}
// 48 rows require still-unmigrated large-whole or >10-digit Integer producers.
a.equal(count, 846); a.equal(publicCount, 798);
lines.push('};', '} // namespace copperfin::test', '');
const generated = lines.join('\n');
if (process.argv.includes('--check')) {
    a.equal(fs.readFileSync(path.join(root, 'constant-public-expected.h'), 'utf8'), generated);
    console.log(`${count} captured rows/five settings exactly match header; ${publicCount} within declared current producer domains.`);
} else process.stdout.write(generated);
