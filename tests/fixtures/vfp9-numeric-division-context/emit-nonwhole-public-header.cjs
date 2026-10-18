// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Reconstruct expected fields ONLY from audited actual native records.
const fs = require('node:fs'), path = require('node:path'), a = require('node:assert/strict');
const {execFileSync} = require('node:child_process');
const root = process.argv.slice(2).find(arg => arg !== '--check') || path.join(__dirname,'nonwhole-captures');
const inventory = JSON.parse(fs.readFileSync(path.join(__dirname,'nonwhole-grid-cases.json')));
const native = JSON.parse(execFileSync(process.execPath,[path.join(__dirname,'audit-nonwhole-grid.cjs'),root],{maxBuffer:32*1024*1024}));
const rows = [];
for (const row of inventory.rows) {
    const actual = native.observations.filter(o => o.index === row.index);
    a.equal(actual.length,20);
    const bitValues = new Set(actual.map(o=>o.result.bits)); a.equal(bitValues.size,1);
    const descriptors = inventory.settings.map(setting => {
        const samples = actual.filter(o=>o.setting===setting); a.equal(samples.length,4);
        for (const s of samples) a.deepEqual(s.result,samples[0].result);
        const n = samples[0].result;
        return `{${n.width},${n.type==='I'?0:n.decimals},runtime::PrgNumericStorage::${n.type==='I'?'integer':'real'}}`;
    });
    rows.push(`    {${JSON.stringify(row.kind+':'+row.index)},${JSON.stringify(row.expression)},0x${[...bitValues][0]}ULL,{{${descriptors.join(',')}}},${row.kind!=='retained-outside-scope'}},`);
}
a.equal(rows.length,578);
const header = `// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001: actual native fields.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <array>
#include <cstdint>
namespace copperfin::test {
struct NativeNonwholeDivisionRow {
    const char* label; const char* expression; std::uint64_t bits;
    std::array<runtime::PrgNumericMetadata,5> metadata;
    bool admitted;
};
inline constexpr NativeNonwholeDivisionRow native_nonwhole_division_rows[] = {
${rows.join('\n')}
};
} // namespace copperfin::test
`;
if (process.argv.includes('--check')) {
    a.equal(fs.readFileSync(path.join(__dirname,'nonwhole-public-expected.h'),'utf8'),header);
    console.log('578 native rows/five settings exactly match the header; 574 admitted, four retained outside-scope controls.');
} else process.stdout.write(header);
