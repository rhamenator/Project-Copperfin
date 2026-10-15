// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independently link the frozen public table to retained native observations;
// never call the production classifier to manufacture an expected descriptor.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),crypto=require('node:crypto');
const cpp=fs.readFileSync(process.argv[2]||path.resolve(__dirname,'../../../tests/test_prg_engine_numeric_integer_metadata.cpp'),'utf8');
const native=JSON.parse(fs.readFileSync(path.join(__dirname,'integer-captures/audit.json'),'utf8'));
a.equal(native.documentedFieldsRepeatEqual,true);a.equal(native.recordedErrors,0);a.equal(native.compiledVersusEvalDifferences.length,0);
const matches=[...cpp.matchAll(/\{"([^"]+)",(\d+),(0x[0-9A-F]+ULL|0)\}/g)];a.equal(matches.length,18);
for(const [,expression,width,bitsText] of matches) {
 const n=native.observations.find(o=>o.decimals2.expression===expression);a(n,expression);
 const bits=BigInt(bitsText.replace('ULL','')).toString(16).toUpperCase().padStart(16,'0');
 for(const row of[n.decimals2,n.decimals14]){a.equal(row.type,'I',expression);a.equal(row.width,Number(width),expression);a.equal(row.bits,bits,expression);}
}
const real=native.observations.find(o=>o.decimals2.expression==='-0.0');
for(const row of[real.decimals2,real.decimals14]){a.equal(row.type,'N');a.equal(row.width,4);a.equal(row.decimals,1);a.equal(row.bits,'8000000000000000');}
process.stdout.write(JSON.stringify({publicRows:matches.length,nativeSettingsPerRow:2,documentedTypedRepeat:true,expectedRowsMatchNative:true,
 publicSourceSha256:crypto.createHash('sha256').update(cpp).digest('hex'),
 limitation:'Independent table correspondence, not runtime execution or assignment/default-display acceptance.'},null,2)+'\n');
