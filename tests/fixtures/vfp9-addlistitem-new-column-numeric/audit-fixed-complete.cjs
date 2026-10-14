// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-ADDLISTITEM-NEW-COLUMN-NUMERIC-001 / VR001+002.
// Full fixed100 gate only; hostile112/preservation328/contracts remain separate.
const fs=require('fs'),path=require('path'),assert=require('assert'),crypto=require('crypto'),{execFileSync}=require('child_process');
const fixture='tests/fixtures/vfp9-addlistitem-new-column-numeric/';
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const names=s=>s.split(/\r?\n/).map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]).filter(Boolean).sort();
const prefix='test_prg_engine_addlistitem_new_column_numeric';
const inventory=names(fs.readFileSync(fixture+'pre-freeze-original100-test-list.log','utf8'));
const prior=names(fs.readFileSync('tests/fixtures/vfp9-addlistitem-column-numeric/pre-freeze-original95-test-list.log','utf8'));
assert.equal(inventory.length,100);assert.equal(new Set(inventory).size,100);assert.equal(prior.length,95);
assert.deepEqual(inventory.filter(n=>!n.startsWith(prefix)),prior);
function auditRaw(raw){
 const lines=raw.split(/\r?\n/),starts=lines.map(l=>l.match(/^\s*Start\s+(\d+):\s+(\S+)$/)).filter(Boolean);
 const results=lines.map(l=>l.match(/^\s*(\d+)\/100 Test\s+#(\d+):\s+(\S+)\s+\.{3,}\s*(Passed|\*\*\*Failed)\s+[\d.]+ sec$/)).filter(Boolean);
 assert.equal(starts.length,100);assert.equal(new Set(starts.map(m=>m[1])).size,100);
 assert.equal(results.length,100);assert.equal(new Set(results.map(m=>m[2])).size,100);
 assert.deepEqual(starts.map(m=>m[2]).sort(),inventory);assert.deepEqual(results.map(m=>m[3]).sort(),inventory);
 assert.deepEqual(results.map(m=>Number(m[1])),Array.from({length:100},(_,i)=>i+1));
 const byID=new Map(starts.map(m=>[m[1],m[2]]));for(const m of results){assert.equal(byID.get(m[2]),m[3]);assert.equal(m[4],'Passed',m[3]);}
 assert(/^100% tests passed, 0 tests failed out of 100$/m.test(raw));
 const failures=lines.filter(l=>/FAIL:|AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|SUMMARY:.*Sanitizer|LeakSanitizer/.test(l));assert.deepEqual(failures,[]);
 const counters=lines.map(l=>l.match(/^(\d+): AddListItem NEW COLUMN frozen cases: direct=(\d+) public=(\d+)$/)).filter(Boolean).map(m=>({subject:byID.get(m[1]),direct:Number(m[2]),public:Number(m[3])})).sort((a,b)=>a.subject.localeCompare(b.subject));
 const expected=[{subject:prefix,direct:192,public:0},...['copperfin','vfp9'].flatMap(mode=>[1,2].map(s=>({subject:prefix+'_'+mode+'_session'+s,direct:0,public:432})))].sort((a,b)=>a.subject.localeCompare(b.subject));
 assert.deepEqual(counters,expected);
 return {subjects:100,priorSubjects:95,counters,failures,rawSHA256:hash(raw),complete:true};
}
module.exports={auditRaw};
if(require.main===module){try{
 assert.equal(process.argv.length,3,'Expected completed BOTH fixed100 evidence directory');
 const evidence=process.argv[2],read=f=>fs.readFileSync(path.join(evidence,f),'utf8'),status=f=>read(f).trim();
 assert.equal(fs.readFileSync(path.join(path.dirname(evidence),'fixed-driver.status'),'utf8').trim(),'0','actual BOTH driver harvested');
 const entries=fs.readFileSync(fixture+'fixed-inputs.sha256','utf8').trim().split('\n').map(l=>{const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m);return [m[1],m[2]];});
 assert.equal(entries.length,270);for(const [h,f] of entries)assert.equal(hash(fs.readFileSync(f)),h,f);
 execFileSync(process.execPath,[fixture+'audit-wiring.cjs']);
 for(const compiler of ['gcc','clang']){
  assert.equal(status('fixed-compile-'+compiler+'.status'),'0');assert.equal(status('fixed-'+compiler+'.status'),'0');
  assert(/^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\dZ$/.test(status('fixed-'+compiler+'.completed-at')));
  for(const phase of ['before','after'])assert.deepEqual(status('fixed-'+compiler+'-inputs-'+phase+'.log').split('\n'),entries.map(e=>e[1]+': OK'));
  assert.deepEqual(names(read('fixed-'+compiler+'-test-list.log')),inventory);
  assert.equal(status('fixed-'+compiler+'-test-metadata.stderr'),'');assert.equal(status('fixed-'+compiler+'-metadata-audit.stderr'),'');
  execFileSync(process.execPath,[fixture+'audit-test-metadata.cjs',path.join(evidence,'fixed-'+compiler+'-test-metadata.json')]);
  const result=auditRaw(read('fixed-'+compiler+'.log'));assert.equal(status('fixed-'+compiler+'.log.sha256').split(/\s+/)[0],result.rawSHA256);
  console.log(JSON.stringify({compiler,...result,sourceInputs:270}));
 }
 console.log(JSON.stringify({complete:true,note:'BOTH full fixed100/counters/prior95/source270/commands only; separate hostile, SAME-compiler preservation, contracts, DQDV and exact-head integration remain required.'}));
}catch(error){console.error(error.stack);process.exitCode=1;}}
