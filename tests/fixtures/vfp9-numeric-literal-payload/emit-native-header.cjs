// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Expected fields come ONLY from independently audited actual native records.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const output=[];
for(const [name,caseFile,auditor,capture] of [
    ['profile','cases.json','audit-grid.cjs','captures'],
    ['heldout','holdout-cases.json','audit-holdout.cjs','holdout-captures']]) {
    const inventory=JSON.parse(fs.readFileSync(path.join(__dirname,caseFile)));
    const actual=JSON.parse(execFileSync(process.execPath,[path.join(__dirname,auditor),path.join(__dirname,capture)],{maxBuffer:32*1024*1024}));
    a.equal(actual.compilerEvaluateDifferences.length,0);
    for(const row of inventory.rows) {
        const records=actual.observations.filter(o=>o.index===row.index);a.equal(records.length,8);
        const values=new Set(records.map(o=>o.result.bits));a.equal(values.size,1);
        const descriptors=[0,18].map(setting=>{
            const samples=records.filter(o=>o.setting===setting);a.equal(samples.length,4);
            for(const sample of samples)a.deepEqual(sample.result,samples[0].result);
            const r=samples[0].result;
            return `{${r.width},${r.type==='I'?0:r.decimals},runtime::PrgNumericStorage::${r.type==='I'?'integer':'real'}}`;
        });
        const plain=name==='heldout'||row.wrapper==='literal';
        output.push(`    {${JSON.stringify(name+':'+row.index)},${JSON.stringify(row.expression)},0x${[...values][0]}ULL,{{${descriptors.join(',')}}},${plain}},`);
    }
}
a.equal(output.length,1193);
const header=`// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001: actual native fields, no candidate imports.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <array>
#include <cstdint>
namespace copperfin::test {
struct NativeLiteralPayloadRow {
    const char* label; const char* expression; std::uint64_t bits;
    std::array<runtime::PrgNumericMetadata,2> metadata;
    bool plain_literal; // Helper-input shape only. Public test checks EVERY row.
};
inline constexpr NativeLiteralPayloadRow native_literal_payload_rows[]={
${output.join('\n')}
};
} // namespace copperfin::test
`;
if(process.argv.includes('--check')) {
    a.equal(fs.readFileSync(path.join(__dirname,'native-expected.h'),'utf8'),header);
    console.log('All1193 native rows/settings match exactly; no predictions or product helpers imported.');
} else process.stdout.write(header);
