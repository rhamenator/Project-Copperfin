// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Structural/provenance checkpoint only, NOT runtime acceptance qualification.
const fs = require('fs'), assert = require('assert'), crypto = require('crypto');
const {execFileSync} = require('child_process');
const base = 'tests/fixtures/vfp9-addlistitem-new-column-numeric/';
const hash = b => crypto.createHash('sha256').update(b).digest('hex');
const entries = file => fs.readFileSync(base + file, 'utf8').trim().split('\n').map(line => {
  const m = line.match(/^([0-9a-f]{64})  (.+)$/); assert(m, line); return {hash:m[1], path:m[2]};
});
const check = file => { const rows = entries(file); for(const row of rows) assert.equal(hash(fs.readFileSync(row.path)), row.hash, row.path); return rows.length; };
assert.equal(check('first-recovery-evidence.sha256'), 38);
assert.equal(check('pre-code-map-inputs.sha256'), 3);
assert.equal(check('unused-helper-seven-inputs.sha256'), 7);
const original = entries('selection-main-inputs.sha256');
const changed = original.filter(row => hash(fs.readFileSync(row.path)) !== row.hash).map(row => row.path).sort();
assert.deepEqual(changed, ['src/runtime/prg_engine_helpers.cpp','src/runtime/prg_engine_helpers.h']);
const preserved = original.filter(row => !changed.includes(row.path)); assert.equal(preserved.length, 5);
for(const row of preserved) {
  assert.equal(hash(execFileSync('git',['show','337156b342ff5e59db88859ededf6580066f3355:'+row.path])), row.hash, row.path);
}
const sourcePaths = changed;
const sourceDiff = execFileSync('git', ['diff','--unified=0','337156b342ff5e59db88859ededf6580066f3355','--',...sourcePaths], {encoding:'utf8'});
assert(!sourceDiff.split('\n').some(line => line.startsWith('-') && !line.startsWith('---')), 'only helper/header additions; no original code deletion');
const name = 'checked_addlistitem_new_column_argument';
const matches = execFileSync('rg',['-n',name,'src'],{encoding:'utf8'}).trim().split('\n');
assert.equal(matches.length,2); assert(matches.some(line=>line.startsWith(sourcePaths[0]+':'))); assert(matches.some(line=>line.startsWith(sourcePaths[1]+':')));
for(const compiler of ['gcc','clang']) {
  assert.equal(fs.readFileSync(base+'unused-helper-'+compiler+'-syntax.status','utf8').trim(),'0');
  assert.equal(fs.readFileSync(base+'unused-helper-'+compiler+'-syntax.log','utf8'),'');
}
for(const prefix of ['configuration','boundary']) {
  for(const suffix of ['first','repeat']) {
    assert.equal(fs.readFileSync(base+prefix+'-'+suffix+'.status','utf8').trim(),'0');
    assert.equal(fs.readFileSync(base+prefix+'-'+suffix+'.stderr','utf8'),'');
  }
  assert.equal(fs.readFileSync(base+prefix+'.cmp.status','utf8').trim(),'0');
  assert(fs.readFileSync(base+prefix+'-first-results.txt').equals(fs.readFileSync(base+prefix+'-repeat-results.txt')));
}
const configuration = JSON.parse(fs.readFileSync(base+'configuration-audit.jsonl','utf8').trim());
assert.equal(configuration.subjects,384); assert.equal(configuration.operands,24); assert.equal(configuration.unchangedRejections,112); assert.equal(configuration.observedOneRowCreations,272);
const boundary = JSON.parse(fs.readFileSync(base+'boundary-audit-v2.jsonl','utf8').trim().split('\n').at(-1));
assert.equal(boundary.subjects,144); assert.equal(boundary.operands,36); assert.equal(boundary.admitted,140); assert.equal(boundary.rejected,4); assert.equal(boundary.bothReadRoutesEqual,true);
const oldBoundary = JSON.parse(fs.readFileSync(base+'boundary-audit.jsonl','utf8').trim().split('\n').at(-1)); assert.equal(oldBoundary.operands,25);
const oldAudit = fs.readFileSync(base+'audit-boundary.before-metadata.txt','utf8');
assert.equal(fs.readFileSync(base+'audit-boundary.cjs','utf8'), oldAudit.replace('groups:4,operands:25,','groups:4,operands:36,'), 'metadata correction only');
console.log(JSON.stringify({historicalEvidenceEntries:38,preCodeMapEntries:3,currentPhaseInputs:7,deliberatelyChanged:changed,preservedMainInputs:preserved.map(row=>row.path),productionNameOccurrences:2,syntaxOnly:{gcc:0,clang:0},matchingNativePairs:[384,144],staleBoundaryReportRetained:true,note:'Structural/provenance and syntax feedback only. No runtime qualification, assertion/registration/isolation/original freeze/caller migration, #7186 repair or assurance closure.'}));
