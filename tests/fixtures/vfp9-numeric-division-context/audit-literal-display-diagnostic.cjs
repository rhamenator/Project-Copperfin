// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Product readback only; expected display bytes come from independent native captures.
const fs=require('node:fs'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2];
for(const name of ['compile','run']){
    a.equal(fs.readFileSync(root+'/'+name+'.status','utf8'),'0\n');
    a.equal(fs.statSync(root+'/'+name+'.stderr').size,0);
}
a.equal(fs.readFileSync(root+'/library-check-before.log','utf8'),fs.readFileSync(root+'/library-check-after.log','utf8'));
const inv=JSON.parse(fs.readFileSync(__dirname+'/small-divisor-grid-cases.json'));
const native=JSON.parse(z.gunzipSync(fs.readFileSync(__dirname+'/small-divisor-captures/audit-qualified.json.gz')));
const literals=new Map(inv.rows.filter(r=>r.kind==='literal-control').map(r=>[r.expression,r.index]));a.equal(literals.size,15);
const raw=fs.readFileSync(root+'/result.txt'),lines=raw.toString().trimEnd().split('\n');
a.equal(lines.pop(),'DONE|300|mismatches=70');a.equal(lines.length,300);
const seen=new Set(),differences=[];
for(const line of lines){
    const f=line.split('|');a.equal(f.length,8);
    const [mode,ds,fixed,expression,nativeHex,actualHex,metadata,bits]=f,d=Number(ds);
    a(['COPPERFIN','VFP9'].includes(mode));a(inv.settings.includes(d));a(['OFF','ON'].includes(fixed));a(literals.has(expression));
    const key=JSON.stringify([mode,d,fixed,expression]);a(!seen.has(key));seen.add(key);
    a.equal(metadata,'1');a.equal(bits,'1');a.match(actualHex,/^(?:[0-9A-F]{2})*$/);
    const controls=native.observations.filter(o=>o.index===literals.get(expression)&&o.setting===d&&o.fixed===fixed);a.equal(controls.length,2);
    for(const c of controls)a.equal(nativeHex,c.result.displayHex);
    if(nativeHex!==actualHex)differences.push({mode,decimals:d,fixed,expression,nativeHex,actualHex,
        nativeText:Buffer.from(nativeHex,'hex').toString(),actualText:Buffer.from(actualHex,'hex').toString()});
}
a.equal(differences.length,70);
console.log(JSON.stringify({requirement:'RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001',
    separateTrackingIssue:7210,actualReadbacks:300,displayMismatches:70,metadataMismatches:0,payloadMismatches:0,
    nativeSource:'Existing independent paired compiled/EVALUATE small-divisor literal controls, not a new native run or a product-derived expected table.',
    sha256:crypto.createHash('sha256').update(raw).digest('hex'),differences},null,2));
