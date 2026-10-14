// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-ADDLISTITEM-NEW-COLUMN-NUMERIC-001 / VR001+002.
// Post-freeze audit tooling only; NEVER change frozen assertions/inputs to pass.
const fs=require('fs'),path=require('path'),crypto=require('crypto'),assert=require('assert');
const prefix='test_prg_engine_addlistitem_new_column_numeric';
const shards=['copperfin','vfp9'].flatMap(m=>[1,2].map(s=>prefix+'_'+m+'_session'+s)).sort();
const digest=s=>crypto.createHash('sha256').update(s).digest('hex');
const listNames=raw=>raw.split(/\r?\n/).map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]).filter(Boolean).sort();
function auditRaw(raw,inventory){
 const lines=raw.split(/\r?\n/),failures=[],counters=[],starts=new Map();
 let block=null,number=null,startCount=0;
 const flush=()=>{if(block!==null)failures.push({subject:starts.get(number),text:block.join('\n')});block=null;number=null;};
 for(const line of lines){
  const start=line.match(/^\s*Start\s+(\d+):\s+(\S+)$/);if(start){++startCount;starts.set(start[1],start[2]);}
  const fail=line.match(/^(\d+): FAIL: (.*)$/);if(fail){flush();number=fail[1];block=[fail[2]];continue;}
  if(block!==null){const continuation=line.match(/^(\d+): (.*)$/);if(continuation&&continuation[1]===number&&!/^AddListItem NEW COLUMN frozen cases:/.test(continuation[2]))block.push(continuation[2]);else flush();}
  const count=line.match(/^(\d+): AddListItem NEW COLUMN frozen cases: direct=(\d+) public=(\d+)$/);if(count)counters.push({subject:starts.get(count[1]),direct:Number(count[2]),public:Number(count[3])});
 }
 flush();
 const subjects=lines.map(l=>l.match(/^\s*(\d+)\/100 Test\s+#(\d+):\s+(\S+)\s+\.{3,}\s*(Passed|\*\*\*Failed)\s+[\d.]+ sec$/)).filter(Boolean);
 const names=subjects.map(m=>m[3]).sort(),failed=subjects.filter(m=>m[4]==='***Failed').map(m=>m[3]).sort();
 const prior=inventory.filter(n=>n!==prefix&&!shards.includes(n));
 const allowed=/^AddListItem NEW COLUMN (admission|continuation|localized11|preserved result|retained no-row metadata|modeled count\/coherence|modeled candidate row|candidate count) /;
 const unrelated=failures.filter(f=>!shards.includes(f.subject)||!allowed.test(f.text));
 const diagnostics=lines.filter(l=>/AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|SUMMARY:.*Sanitizer|LeakSanitizer/.test(l));
 const errors=[];
 if(inventory.length!==100||new Set(inventory).size!==100||prior.length!==95)errors.push('configured inventory100/prior95');
 if(subjects.length!==100||new Set(names).size!==100||JSON.stringify(names)!==JSON.stringify(inventory))errors.push('complete unique execution100');
 if(startCount!==100||starts.size!==100||subjects.some(m=>starts.get(m[2])!==m[3]))errors.push('unique starts match every result ID/name');
 if(JSON.stringify(subjects.map(m=>Number(m[1])))!==JSON.stringify(Array.from({length:100},(_,i)=>i+1)))errors.push('complete ordered ordinals');
 if(subjects.filter(m=>prior.includes(m[3])).length!==95||subjects.filter(m=>prior.includes(m[3])).some(m=>m[4]!=='Passed'))errors.push('all prior95 must pass');
 if(JSON.stringify(failed)!==JSON.stringify(shards)||!/^96% tests passed, 4 tests failed out of 100$/m.test(raw))errors.push('only four new public failures');
 if(!subjects.some(m=>m[3]===prefix&&m[4]==='Passed'))errors.push('new unused helper direct must pass');
 const wanted=[{subject:prefix,direct:192,public:0},...shards.map(subject=>({subject,direct:0,public:624}))].sort((a,b)=>a.subject.localeCompare(b.subject));
 if(JSON.stringify(counters.sort((a,b)=>String(a.subject).localeCompare(String(b.subject))))!==JSON.stringify(wanted))errors.push('all five actual192/four624 counters');
 if(failures.length===0||unrelated.length)errors.push('full selected failure blocks, no unrelated assertions');
 if(diagnostics.length)errors.push('unexpected sanitizer diagnostics');
 const canonicalFailures=failures.map(f=>f.subject+'\n'+f.text).sort().join('\n');
 return {subjects:subjects.length,failed,counters,assertionFailures:failures.length,failures,unrelated,diagnostics,errors,rawSHA256:digest(raw),failureSHA256:digest(canonicalFailures),complete:errors.length===0};
}
function auditEvidence(evidence){
 const fixture='tests/fixtures/vfp9-addlistitem-new-column-numeric';
 const inventory=listNames(fs.readFileSync(fixture+'/pre-freeze-original100-test-list.log','utf8'));
 const prior=listNames(fs.readFileSync('tests/fixtures/vfp9-addlistitem-column-numeric/pre-freeze-original95-test-list.log','utf8'));
 assert.equal(prior.length,95);assert.deepEqual(inventory.filter(n=>n!==prefix&&!shards.includes(n)),prior,'exact prior95');
 const manifest=fs.readFileSync(fixture+'/frozen-original-inputs.sha256','utf8');
 const entries=manifest.trim().split('\n').map(l=>{const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m);return {hash:m[1],file:m[2]};});
 assert.equal(entries.length,264);assert.equal(new Set(entries.map(e=>e.file)).size,264);
 for(const e of entries)assert.equal(digest(fs.readFileSync(e.file)),e.hash,'CURRENT frozen input '+e.file);
 const caller=fs.readFileSync(fixture+'/unchanged-three-caller-inputs.sha256','utf8').trim().split('\n');assert.equal(caller.length,3);
 for(const l of caller){const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m);assert.equal(digest(fs.readFileSync(m[2])),m[1],'still-original caller '+m[2]);}
 const status=f=>fs.readFileSync(path.join(evidence,f),'utf8').trim();assert.equal(status('originals-driver.status'),'0','BOTH driver harvested complete');
 const results=[];
 for(const compiler of ['gcc','clang']){
  for(const step of ['configure','compile'])assert.equal(status('original-'+step+'-'+compiler+'.status'),'0',compiler+' '+step);
  assert.equal(status('original-'+compiler+'.status'),'8','actual failing CTest');assert(/^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\dZ$/.test(status('original-'+compiler+'.completed-at')));
  for(const phase of ['before','after'])assert.deepEqual(status('original-'+compiler+'-inputs-'+phase+'.log').split('\n'),entries.map(e=>e.file+': OK'),'exact264 '+compiler+' '+phase);
  assert.deepEqual(listNames(fs.readFileSync(path.join(evidence,'original-'+compiler+'-test-list.log'),'utf8')),inventory,'configured inventory unchanged');
  assert.equal(status('original-'+compiler+'-test-metadata.stderr'),'');assert.equal(status('original-'+compiler+'-metadata-audit.stderr'),'');
  const metadata=status('original-'+compiler+'-metadata-audit.jsonl').split('\n').map(JSON.parse);assert.equal(metadata.length,6);assert.equal(metadata[5].subjects,5);assert.equal(metadata[5].commands,'yes');assert.equal(metadata[5].complete,true);
  // Independently rerun the exact built-command/isolation checks, not just a
  // retained summary claiming those checks passed.
  const {execFileSync}=require('child_process');execFileSync(process.execPath,[fixture+'/audit-test-metadata.cjs',path.join(evidence,'original-'+compiler+'-test-metadata.json')]);
  const result=auditRaw(fs.readFileSync(path.join(evidence,'original-'+compiler+'.log'),'utf8'),inventory);
  assert.equal(status('original-'+compiler+'.log.sha256').split(/\s+/)[0],result.rawSHA256,'full raw hash');results.push({compiler,...result});
 }
 const identical=results[0].failureSHA256===results[1].failureSHA256;
 return {results,identicalSelectedFullFailures:identical,complete:results.every(r=>r.complete)&&identical};
}
module.exports={auditRaw,auditEvidence};
if(require.main===module){try{
 assert.equal(process.argv.length,3,'Expected completed BOTH original100 evidence directory');const audit=auditEvidence(process.argv[2]);
 for(const r of audit.results){const {failures,...summary}=r;console.log(JSON.stringify(summary));for(const failure of failures)console.log(JSON.stringify({compiler:r.compiler,...failure}));}
 console.log(JSON.stringify({complete:audit.complete,identicalSelectedFullFailures:audit.identicalSelectedFullFailures,note:'BOTH full original100/counters/current-before-after264/caller3/built commands audit ONLY. Original failures are NOT fixed acceptance; no hostile public original execution.'}));process.exitCode=audit.complete?0:1;
}catch(error){console.error(error.stack);process.exitCode=1;}}
