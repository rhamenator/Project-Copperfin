// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Only captured native literals and complete root results supply expected fields.
// Postorder nodes describe source syntax, never intermediate expected metadata.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'nonwhole-grid-cases.json')));
const raw=JSON.parse(execFileSync(process.execPath,[path.join(__dirname,'audit-nonwhole-grid.cjs'),path.join(__dirname,'nonwhole-captures')],{maxBuffer:32*1024*1024}));
const literal=new Map();
const metadata=m=>`{${m.width},${m.type==='I'?0:m.decimals},runtime::PrgNumericStorage::${m.type==='I'?'integer':'real'}}`;
for(const row of inv.rows.filter(r=>r.kind==='literal-control')){
    const samples=raw.observations.filter(o=>o.index===row.index);a.equal(samples.length,20);
    for(const s of samples)a.deepEqual(s.result,samples[0].result);
    literal.set(row.literal,samples[0].result);
}
const nodes=[],roots=[];
function emit(tree){
    let node;
    if(tree.kind==='literal'){
        a(literal.has(tree.text));const v=literal.get(tree.text);
        node=`{NativeNonwholeOp::literal,0,0,0x${v.bits}ULL,${metadata(v)}}`;
    }else if(tree.kind==='negate'){
        const operand=emit(tree.operand);node=`{NativeNonwholeOp::negate,${operand},0,0,{}}`;
    }else{
        a.equal(tree.kind,'divide');const left=emit(tree.left),right=emit(tree.right);
        node=`{NativeNonwholeOp::divide,${left},${right},0,{}}`;
    }
    const index=nodes.length;nodes.push('    '+node+',');return index;
}
for(const core of inv.cores){
    const row=inv.rows.find(r=>r.kind==='core'&&r.core===core.id&&r.wrapper==='direct');a(row);
    const samples=raw.observations.filter(o=>o.index===row.index&&o.setting===0);a.equal(samples.length,4);
    for(const s of samples)a.deepEqual(s.result,samples[0].result);
    const v=samples[0].result;
    roots.push(`    {${core.id},${emit(core.tree)},0x${v.bits}ULL,${metadata(v)}},`);
}
a.equal(roots.length,68);
const out=`// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001: native-only direct roots.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <cstddef>
#include <cstdint>
namespace copperfin::test {
enum class NativeNonwholeOp { literal, negate, divide };
struct NativeNonwholeNode {
    NativeNonwholeOp op; std::size_t left,right; std::uint64_t bits;
    runtime::PrgNumericMetadata metadata;
};
struct NativeNonwholeRoot {
    unsigned core; std::size_t node; std::uint64_t bits;
    runtime::PrgNumericMetadata metadata;
};
inline constexpr NativeNonwholeNode native_nonwhole_nodes[] = {
${nodes.join('\n')}
};
inline constexpr NativeNonwholeRoot native_nonwhole_roots[] = {
${roots.join('\n')}
};
} // namespace copperfin::test
`;
if(process.argv.includes('--check')){
    a.equal(fs.readFileSync(path.join(__dirname,'nonwhole-direct-expected.h'),'utf8'),out);
    console.log(`68 actual native D0 roots, ${nodes.length} syntax nodes; captured literals only, no intermediate-result oracle.`);
}else process.stdout.write(out);
