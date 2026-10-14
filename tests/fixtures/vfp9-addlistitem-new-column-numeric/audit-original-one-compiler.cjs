// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Completed ONE-compiler evidence only, NEVER the required BOTH wiring gate.
const fs=require('fs'),path=require('path'),assert=require('assert'),crypto=require('crypto'),{execFileSync}=require('child_process');
const {auditRaw}=require('./audit-originals-counter-v2-complete.cjs');
const [evidence,compiler]=process.argv.slice(2);assert.equal(process.argv.length,4);assert(['gcc','clang'].includes(compiler));
const fixture='tests/fixtures/vfp9-addlistitem-new-column-numeric/';
const read=f=>fs.readFileSync(path.join(evidence,f),'utf8'),status=f=>read(f).trim(),hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const names=raw=>raw.split(/\r?\n/).map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]).filter(Boolean).sort();
const inventory=names(fs.readFileSync(fixture+'pre-freeze-original100-test-list.log','utf8'));
const prior=names(fs.readFileSync('tests/fixtures/vfp9-addlistitem-column-numeric/pre-freeze-original95-test-list.log','utf8'));
assert.equal(inventory.length,100);assert.equal(prior.length,95);assert.deepEqual(inventory.filter(n=>!n.startsWith('test_prg_engine_addlistitem_new_column_numeric')),prior);
const entries=fs.readFileSync(fixture+'counter-v2-frozen-original-inputs.sha256','utf8').trim().split('\n').map(l=>{const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m);return {hash:m[1],file:m[2]};});
assert.equal(entries.length,270);assert.equal(new Set(entries.map(e=>e.file)).size,270);for(const e of entries)assert.equal(hash(fs.readFileSync(e.file)),e.hash,e.file);
for(const phase of ['before','after'])assert.deepEqual(status('original-'+compiler+'-inputs-'+phase+'.log').split('\n'),entries.map(e=>e.file+': OK'),'all270 '+phase);
for(const step of ['configure','compile'])assert.equal(status('original-'+step+'-'+compiler+'.status'),'0');
assert.equal(status('original-'+compiler+'.status'),'8');assert(/^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\dZ$/.test(status('original-'+compiler+'.completed-at')));
assert.deepEqual(names(read('original-'+compiler+'-test-list.log')),inventory);assert.equal(status('original-'+compiler+'-test-metadata.stderr'),'');assert.equal(status('original-'+compiler+'-metadata-audit.stderr'),'');
execFileSync(process.execPath,[fixture+'audit-test-metadata.cjs',path.join(evidence,'original-'+compiler+'-test-metadata.json')]);
const result=auditRaw(read('original-'+compiler+'.log'),inventory);assert(result.complete,JSON.stringify(result.errors));assert.equal(status('original-'+compiler+'.log.sha256').split(/\s+/)[0],result.rawSHA256);
const {failures,...summary}=result;console.log(JSON.stringify({compiler,...summary,sourceInputs:270,bothCompilerGate:false,note:'Completed ONE original100 only: allprior95/helper192 PASS/onlyfour432 selected public failures. NOT fixed acceptance, BOTH gate or permission to wire.'}));
for(const failure of failures)console.log(JSON.stringify({compiler,...failure}));
