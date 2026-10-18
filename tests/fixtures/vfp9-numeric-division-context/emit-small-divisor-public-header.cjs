// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Expected fields ONLY from audited actual native records, never a candidate/product.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const inv=JSON.parse(fs.readFileSync(__dirname+'/small-divisor-grid-cases.json'));
const native=JSON.parse(execFileSync(process.execPath,[__dirname+'/audit-small-divisor-grid.cjs',__dirname+'/small-divisor-captures'],{maxBuffer:16*1024*1024}));
const metadata=m=>`{${m.width},${m.type==='I'?0:m.decimals},runtime::PrgNumericStorage::${m.type==='I'?'integer':'real'}}`;
const rows=inv.rows.map(row=>{
    const all=native.observations.filter(o=>o.index===row.index);a.equal(all.length,20);
    const bitValues=new Set(all.map(o=>o.result.bits));a.equal(bitValues.size,1);
    const descriptors=inv.settings.map(s=>{
        const samples=all.filter(o=>o.setting===s);a.equal(samples.length,4);
        const fields=m=>({type:m.type,width:m.width,decimals:m.decimals,bits:m.bits});
        for(const v of samples)a.deepEqual(fields(v.result),fields(samples[0].result));
        return metadata(samples[0].result);
    });
    const displays=[];
    for(const s of inv.settings)for(const f of ['OFF','ON']){
        const samples=all.filter(o=>o.setting===s&&o.fixed===f);a.equal(samples.length,2);
        a.equal(samples[0].result.displayHex,samples[1].result.displayHex);
        const v=Buffer.from(samples[0].result.displayHex,'hex');a(v.every(c=>c>=32&&c<=126));
        displays.push(JSON.stringify(v.toString('ascii')));
    }
    return `    {${JSON.stringify(row.kind+':'+row.index)},${JSON.stringify(row.expression)},0x${[...bitValues][0]}ULL,{{${descriptors.join(',')}}},{{${displays.join(',')}}},${row.kind!=='retained-boundary'}},`;
});
a.equal(rows.length,253);a.equal(inv.rows.filter(r=>r.kind!=='retained-boundary').length,245);
const out=`// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001: actual native roots.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <array>
#include <cstdint>
namespace copperfin::test {
struct NativeSmallDivisorRow {
    const char* label; const char* expression; std::uint64_t bits;
    std::array<runtime::PrgNumericMetadata,5> metadata;
    std::array<const char*,10> display; bool admitted;
};
inline constexpr NativeSmallDivisorRow native_small_divisor_rows[] = {
${rows.join('\n')}
};
} // namespace copperfin::test
`;
if(process.argv.includes('--check')){
    a.equal(fs.readFileSync(path.join(__dirname,'small-divisor-public-expected.h'),'utf8'),out);
    console.log('253 actual native rows:245 admitted/eight retained, five descriptors/ten displays each.');
}else process.stdout.write(out);
