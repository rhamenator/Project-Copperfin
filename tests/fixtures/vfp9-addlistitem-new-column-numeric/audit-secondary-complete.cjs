// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-ADDLISTITEM-NEW-COLUMN-NUMERIC-001 / VR001+002.
const fs=require('fs'),path=require('path'),assert=require('assert'),crypto=require('crypto'),{execFileSync}=require('child_process');
const fixture='tests/fixtures/vfp9-addlistitem-new-column-numeric/';
assert.equal(process.argv.length,3);const root=process.argv[2];
const read=f=>fs.readFileSync(path.join(root,f),'utf8'),status=f=>read(f).trim(),hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const parse=s=>s.trim().split('\n').map(l=>{const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m);return [m[1],m[2]];});
const fixed=parse(fs.readFileSync(fixture+'fixed-inputs.sha256','utf8')),preservation=parse(fs.readFileSync(fixture+'preservation-source-frozen.sha256','utf8'));
assert.equal(fixed.length,270);assert.equal(preservation.length,3);
for(const [h,f] of [...fixed,...preservation])assert.equal(hash(fs.readFileSync(f)),h,f);
assert.equal(status('fixed-driver.status'),'0');assert.equal(status('fixed/complete-audit.status'),'0');assert.equal(status('hostile-driver.status'),'0');
for(const compiler of ['gcc','clang']){
 for(const phase of ['before','after'])assert.deepEqual(status('hostile-'+compiler+'-inputs-'+phase+'.log').split('\n'),fixed.map(e=>e[1]+': OK'));
 const binary=parse(read('hostile-'+compiler+'-binary.sha256'));assert.equal(binary.length,1);assert.equal(hash(fs.readFileSync(binary[0][1])),binary[0][0]);
 assert.equal(status('hostile-'+compiler+'-binary-after.log'),binary[0][1]+': OK');
 for(const mode of ['COPPERFIN','VFP9'])for(const session of [1,2]){
  const base='hostile-'+compiler+'-'+mode+'-'+session;
  assert.equal(status(base+'.status'),'0');assert.equal(read(base+'.stderr'),'');
  assert.equal(read(base+'.log'),'AddListItem NEW COLUMN frozen cases: direct=0 public=112\n');
 }
 for(const phase of ['baseline','fixed']){
  const base='preservation-'+phase+'-'+compiler;
  for(const step of ['-driver','-compile','','-audit'])assert.equal(status(base+step+'.status'),'0');
  assert.equal(read(base+'.stderr'),'');assert.equal(read(base+'-audit.stderr'),'');
  for(const when of ['before','after'])assert.deepEqual(status(base+'-source-'+when+'.log').split('\n'),preservation.map(e=>e[1]+': OK'));
  const libraries=parse(read(base+'-libraries.sha256'));assert.equal(libraries.length,9);assert.equal(new Set(libraries.map(e=>e[1])).size,9);
  assert.deepEqual(status(base+'-libraries-after.log').split('\n'),libraries.map(e=>e[1]+': OK'));
  if(phase==='fixed')for(const [h,f] of libraries)assert.equal(hash(fs.readFileSync(f)),h,f);
  execFileSync(process.execPath,[fixture+'audit-preservation.cjs',path.join(root,base+'.out')]);
  assert.equal(parse(read(base+'.out.sha256'))[0][0],hash(fs.readFileSync(path.join(root,base+'.out'))));
 }
 const baseline=fs.readFileSync(path.join(root,'preservation-baseline-'+compiler+'.out')),current=fs.readFileSync(path.join(root,'preservation-fixed-'+compiler+'.out'));
 assert(baseline.equals(current),'full SAME-compiler328 comparison');assert.equal(status('preservation-fixed-'+compiler+'.cmp.status'),'0');assert.equal(read('preservation-fixed-'+compiler+'.cmp.log'),'');
 console.log(JSON.stringify({compiler,hostileShards:4,hostilePerShard:112,preservationCalls:328,fullSameCompilerEqual:true,preservationSHA256:hash(current),currentInputs:270,preservationSources:3,linkedLibrariesPerCapture:9,note:'Fixed-only hostile and NON-NATIVE modeled preservation, not native shared-creation, all-properties, leak or system qualification.'}));
}
console.log(JSON.stringify({complete:true}));
