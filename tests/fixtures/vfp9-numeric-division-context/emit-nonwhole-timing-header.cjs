// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native-only emitter; imports neither product code nor timing hypotheses.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const root=process.argv.slice(2).find(x=>x!=='--check')||path.join(__dirname,'nonwhole-timing-captures');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'nonwhole-timing-cases.json')));
const observed=JSON.parse(execFileSync(process.execPath,[path.join(__dirname,'audit-nonwhole-timing.cjs'),root],{maxBuffer:16*1024*1024}));
let selected=0,oldControls=0;
const rows=inv.rows.map(row=>{
    const observations=observed.observations.filter(o=>o.index===row.index);a.equal(observations.length,20);
    const bits=new Set(observations.map(o=>o.result.bits));a.equal(bits.size,1);
    const metadata=inv.settings.map(s=>{
        const samples=observations.filter(o=>o.setting===s);a.equal(samples.length,4);
        for(const o of samples)a.deepEqual(o.result,samples[0].result);
        const m=samples[0].result;
        return `{${m.width},${m.type==='I'?0:m.decimals},runtime::PrgNumericStorage::${m.type==='I'?'integer':'real'}}`;
    });
    const callback=!!row.core&&!row.core.includes('0.00')&&!row.observesArgument;
    if(callback)++selected;if(row.oldUnknownControl)++oldControls;
    return `    {${JSON.stringify(row.expression)},${row.changesTo??-1},${callback},${!!row.oldUnknownControl},0x${[...bits][0]}ULL,{{${metadata.join(',')}}}},`;
});
a.equal(rows.length,147);a.equal(selected,72);a.equal(oldControls,3);
const header=`// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001: actual native timing.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <array>
#include <cstdint>
namespace copperfin::test {
struct NativeNonwholeTimingRow {
    const char* expression; int changes_to; bool callback_admitted; bool old_unknown_control;
    std::uint64_t bits; std::array<runtime::PrgNumericMetadata,5> metadata;
};
inline constexpr NativeNonwholeTimingRow native_nonwhole_timing_rows[] = {
${rows.join('\n')}
};
} // namespace copperfin::test
`;
if(process.argv.includes('--check')){
    a.equal(fs.readFileSync(path.join(__dirname,'nonwhole-timing-expected.h'),'utf8'),header);
    console.log('147 native timing rows/five settings unchanged; 72 callback cases and three exact older controls selected. Other observations remain retained.');
}else process.stdout.write(header);
