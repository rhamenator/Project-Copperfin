// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// SYNTHETIC parser-tool checks ONLY; never runtime/original/fixed evidence.
const fs=require('fs'),assert=require('assert'),{auditRaw}=require('./audit-fixed-complete.cjs');
const inventory=fs.readFileSync('tests/fixtures/vfp9-addlistitem-new-column-numeric/pre-freeze-original100-test-list.log','utf8').split(/\r?\n/).map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]).filter(Boolean);
const prefix='test_prg_engine_addlistitem_new_column_numeric';
const records=inventory.map((name,i)=>'        Start '+(i+1)+': '+name+'\n'+(name.startsWith(prefix)?(i+1)+': AddListItem NEW COLUMN frozen cases: direct='+(name===prefix?'192 public=0':'0 public=432')+'\n':'')+(i+1)+'/100 Test #'+(i+1)+': '+name+' .... Passed 0.01 sec');
const good=records.join('\n')+'\n100% tests passed, 0 tests failed out of 100\n';
assert.equal(auditRaw(good).complete,true);
const bad=[
 records.slice(1).join('\n'),
 good.replace('100% tests passed','96% tests passed'),
 good.replace('public=432','public=624'),
 good.replace('direct=192','direct=191'),
 good.replace('Passed','***Failed'),
 good+'1: FAIL: selected assertion\n',
 good+'AddressSanitizer: failure\n',
 good+'runtime error: float cast overflow\n',
 good.replace('Start 1:','Start 2:'),
 good.replace('1/100 Test','2/100 Test'),
 good+records[0]+'\n',
];
for(const input of bad)assert.throws(()=>auditRaw(input));
console.log(JSON.stringify({syntheticGoodAccepted:true,malformedVariantsRejected:bad.length,note:'PARSER TOOL SELF-TEST ONLY, not runtime, original failures, fixed acceptance, or generated runtime error11.'}));
