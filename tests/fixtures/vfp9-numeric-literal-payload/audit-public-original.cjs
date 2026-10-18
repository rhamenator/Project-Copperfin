// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Diagnose actual ORIGINAL output independently of product/helper code.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const root=process.argv[2];
a.equal(fs.readFileSync(path.join(root,'build.status'),'utf8'),'0\n');
a.equal(fs.statSync(path.join(root,'build.stderr')).size,0);
a.equal(fs.readFileSync(path.join(root,'test.status'),'utf8'),'1\n');
const stdout=fs.readFileSync(path.join(root,'test.out'),'utf8');
const match=/^literal payload: descriptor=(\d+) bits=(\d+) evaluation=(\d+) controls=(\d+) total=(\d+)\n$/.exec(stdout);a(match);
const [descriptor,bits,evaluation,controls,total]=match.slice(1).map(Number);
a.equal(descriptor,0);a.equal(evaluation,0);a.equal(controls,0);a(bits>0);a.equal(total,bits);
const lines=fs.readFileSync(path.join(root,'test.stderr'),'utf8').trimEnd().split('\n');a.equal(lines.length,bits);
const inventories=[['profile','cases.json','audit-grid.cjs','captures'],
    ['heldout','holdout-cases.json','audit-holdout.cjs','holdout-captures']];
let classified=0;const counts={};
const toBits=n=>{const b=Buffer.alloc(8);b.writeDoubleBE(n);return b.toString('hex').toUpperCase();};
for(const [name,caseFile,auditor,capture] of inventories) {
    const inventory=JSON.parse(fs.readFileSync(path.join(__dirname,caseFile)));
    const actualNative=JSON.parse(execFileSync(process.execPath,[path.join(__dirname,auditor),path.join(__dirname,capture)],{maxBuffer:32*1024*1024}));
    counts[name]={rows:0,assertions:0};
    for(const row of inventory.rows) {
        const observed=lines.filter(line=>line.includes(` ${name}:${row.index} ${row.expression} bits `));
        if(!observed.length)continue;
        a.equal(observed.length,8);++counts[name].rows;counts[name].assertions+=observed.length;
        const nativeRows=actualNative.observations.filter(o=>o.index===row.index);a.equal(nativeRows.length,8);
        let value=Number(row.literal);
        if(['lexical-minus','group-minus'].includes(row.wrapper))value=-value;
        for(const line of observed) {
            a.match(line,/^FAIL: (?:COPPERFIN|VFP9)\/D(?:0|18)\/FIXED(?:OFF|ON) /);
            const pair=/ bits actual=([0-9A-F]{16}) native=([0-9A-F]{16})$/.exec(line);a(pair);
            a.equal(pair[1],toBits(value));a.notEqual(pair[1],pair[2]);
            for(const nativeRow of nativeRows)a.equal(pair[2],nativeRow.result.bits);
        }
        classified+=observed.length;
    }
}
a.equal(classified,bits);
console.log(JSON.stringify({baseline:'ef3ea326f78d640546363abe97bbe27b6208bf45',
    publicFreeze:'e2e59d020e404202a736ee96e907b57a01ecff5a',
    descriptor,bits,evaluation,controls,total,counts,
    result:'Actual public ORIGINAL fails only native payload assertions; all recorded actual bits agree with the old direct decimal parser counterfactual.',
    limitation:'Manual public object/link/run against verified unchanged-main libraries; not a new runtime rebuild, CTest run or SAME/native parity pass.'},null,2));
