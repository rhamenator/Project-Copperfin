// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Mutation checks for the AUDIT PARSER ONLY. Synthetic records are NEVER
// runtime test evidence, original failures, a fake error11 or a wiring gate.
const fs=require('fs'),assert=require('assert');const {auditRaw}=require('./audit-originals-complete.cjs');
const prefix='test_prg_engine_addlistitem_new_column_numeric';
const names=fs.readFileSync('tests/fixtures/vfp9-addlistitem-new-column-numeric/pre-freeze-original100-test-list.log','utf8').split('\n').map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]).filter(Boolean).sort();assert.equal(names.length,100);
const lines=[];
for(const [i,name] of names.entries()){
 const id=i+1,publicShard=name.startsWith(prefix+'_');lines.push(`Start ${id}: ${name}`);
 if(publicShard)lines.push(`${id}: FAIL: AddListItem NEW COLUMN admission synthetic-parser-only`,`${id}: retained multiline diagnostic A`,`${id}: retained multiline diagnostic B`);
 if(name===prefix||publicShard)lines.push(`${id}: AddListItem NEW COLUMN frozen cases: direct=${publicShard?0:192} public=${publicShard?624:0}`);
 lines.push(`${id}/100 Test #${id}: ${name} ... ${publicShard?'***Failed':'Passed'} 0.01 sec`);
}
lines.push('96% tests passed, 4 tests failed out of 100');const raw=lines.join('\n')+'\n';
const good=auditRaw(raw,names);assert.equal(good.complete,true);assert.equal(good.failures.length,4);assert(good.failures.every(f=>f.text.endsWith('retained multiline diagnostic B')));
const mutations={
 truncated:raw.slice(0,raw.length/2),
 missingCounter:raw.replace(/\d+: AddListItem NEW COLUMN frozen cases: direct=192 public=0\n/,''),
 wrongCounter:raw.replace('direct=192 public=0','direct=191 public=0'),
 omittedPrior:raw.replace(/\d+\/100 Test #\d+: test_native_test_isolation_contract .*\n/,''),
 duplicateStart:raw.replace('Start 1:','Start 2:'),
 falseSummary:raw.replace('4 tests failed out of 100','3 tests failed out of 100'),
 unrelatedAssertion:raw.replace('FAIL: AddListItem NEW COLUMN admission','FAIL: AddListItem NEW COLUMN context'),
 sanitizerDiagnostic:raw+'runtime error: synthetic parser diagnostic, NOT runtime execution\n',
 malformedOrdinal:raw.replace('1/100 Test','2/100 Test'),
 wrongSubject:raw.replace('test_native_test_isolation_contract ... Passed','unexpected_subject ... Passed')
};
for(const [name,mutated] of Object.entries(mutations)){const result=auditRaw(mutated,names);assert.equal(result.complete,false,name);console.log(JSON.stringify({mutation:name,rejected:true,errors:result.errors}));}
console.log(JSON.stringify({parserPositiveShape:true,fullMultilineRetention:true,rejectedMutations:Object.keys(mutations).length,note:'AUDIT tooling selftest ONLY. No product runtime execution, original/fixed qualification, synthetic11 acceptance or wiring gate.'}));
