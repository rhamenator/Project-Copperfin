// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Captured literals and complete native roots ONLY; syntax nodes are not child oracles.
const fs=require('node:fs'),a=require('node:assert/strict'),{execFileSync}=require('node:child_process');
const inv=JSON.parse(fs.readFileSync(__dirname+'/small-divisor-grid-cases.json'));
const raw=JSON.parse(execFileSync(process.execPath,[__dirname+'/audit-small-divisor-grid.cjs',__dirname+'/small-divisor-captures'],{maxBuffer:16*1024*1024}));
const metadata=m=>`{${m.width},${m.type==='I'?0:m.decimals},runtime::PrgNumericStorage::${m.type==='I'?'integer':'real'}}`;
const typed=m=>({type:m.type,width:m.width,decimals:m.decimals,bits:m.bits});
const literals=new Map();
for(const row of inv.rows.filter(r=>r.kind==='literal-control')){
    const samples=raw.observations.filter(o=>o.index===row.index);a.equal(samples.length,20);
    for(const s of samples)a.deepEqual(typed(s.result),typed(samples[0].result));
    literals.set(row.expression,samples[0].result);
}
const nodes=[],roots=[];
function parse(text){
    let i=0;
    function add(value){const n=nodes.length;nodes.push('    '+value+',');return n;}
    function atom(){
        if(text[i]==='-'){++i;return add(`{NativeSmallDivisorOp::negate,${atom()},0,0,{}}`);}
        if(text[i]==='('){++i;const v=expression();a.equal(text[i++],')');return v;}
        const m=/^\d+(?:\.\d+)?/.exec(text.slice(i));a(m);i+=m[0].length;a(literals.has(m[0]));
        const v=literals.get(m[0]);return add(`{NativeSmallDivisorOp::literal,0,0,0x${v.bits}ULL,${metadata(v)}}`);
    }
    function expression(){let v=atom();while(text[i]==='/'){++i;const r=atom();v=add(`{NativeSmallDivisorOp::divide,${v},${r},0,{}}`);}return v;}
    const n=expression();a.equal(i,text.length);return n;
}
for(const core of inv.cores){
    const row=inv.rows.find(r=>r.kind==='core'&&r.core===core.id&&r.wrapper==='direct');a(row);
    const samples=raw.observations.filter(o=>o.index===row.index&&o.setting===0);a.equal(samples.length,4);
    for(const s of samples)a.deepEqual(typed(s.result),typed(samples[0].result));
    const v=samples[0].result;roots.push(`    {${core.id},${parse(core.expression)},0x${v.bits}ULL,${metadata(v)}},`);
}
a.equal(roots.length,56);
const out=`// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001: actual native direct roots.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <cstddef>
#include <cstdint>
namespace copperfin::test {
enum class NativeSmallDivisorOp { literal, negate, divide };
struct NativeSmallDivisorNode {
    NativeSmallDivisorOp op; std::size_t left,right; std::uint64_t bits;
    runtime::PrgNumericMetadata metadata;
};
struct NativeSmallDivisorRoot { unsigned core; std::size_t node; std::uint64_t bits; runtime::PrgNumericMetadata metadata; };
inline constexpr NativeSmallDivisorNode native_small_divisor_nodes[] = {
${nodes.join('\n')}
};
inline constexpr NativeSmallDivisorRoot native_small_divisor_roots[] = {
${roots.join('\n')}
};
} // namespace copperfin::test
`;
if(process.argv.includes('--check')){
    a.equal(fs.readFileSync(__dirname+'/small-divisor-direct-expected.h','utf8'),out);
    console.log(`56 actual native D0 roots/${nodes.length} syntax nodes, no intermediate-result oracle.`);
}else process.stdout.write(out);
