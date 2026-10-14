// RQ-CF-PRG-ADDLISTITEM-COLUMN-NUMERIC-001 / VR-5611-ADDLISTITEM-COLUMN-NUMERIC-001/002.
// Future fixed95 gate tooling only. Do not execute hostile vectors on old caller.
const fs=require('fs'),path=require('path'),assert=require('assert'),crypto=require('crypto');
const prefix='test_prg_engine_addlistitem_column_numeric';
const shards=['copperfin','vfp9'].flatMap(m=>[1,2].map(s=>prefix+'_'+m+'_session'+s)).sort();
const same=(a,b)=>JSON.stringify(a)===JSON.stringify(b);
const digest=b=>crypto.createHash('sha256').update(b).digest('hex');
const listNames=raw=>raw.split(/\r?\n/).map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]).filter(Boolean).sort();
function auditRaw(raw,inventory){
 const lines=raw.split(/\r?\n/),errors=[],starts=new Map(),counters=[];
 let startCount=0;
 for(const line of lines){
  const start=line.match(/^\s*Start\s+(\d+):\s+(\S+)$/);
  if(start){++startCount;starts.set(start[1],start[2]);}
  const counter=line.match(/^(\d+): AddListItem COLUMN frozen cases: direct=(\d+) public=(\d+)$/);
  if(counter)counters.push({subject:starts.get(counter[1]),direct:Number(counter[2]),public:Number(counter[3])});
 }
 const subjects=lines.map(l=>l.match(/^\s*(\d+)\/95 Test\s+#(\d+):\s+(\S+)\s+\.{3,}\s*(Passed|\*\*\*\S+)\s+[\d.]+ sec$/)).filter(Boolean);
 const names=subjects.map(m=>m[3]).sort();
 const failures=lines.filter(l=>/^\d+: FAIL:/.test(l));
 const diagnostics=lines.filter(l=>/AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|SUMMARY:.*Sanitizer|LeakSanitizer/.test(l));
 if(inventory.length!==95||new Set(inventory).size!==95)errors.push('expected inventory95');
 if(subjects.length!==95||new Set(names).size!==95||!same(names,inventory))errors.push('complete unique execution95');
 if(startCount!==95||starts.size!==95||subjects.some(m=>starts.get(m[2])!==m[3]))errors.push('complete matching starts/results');
 if(!same(subjects.map(m=>Number(m[1])),Array.from({length:95},(_,i)=>i+1)))errors.push('complete ordered ordinals');
 if(subjects.some(m=>m[4]!=='Passed')||!/^100% tests passed, 0 tests failed out of 95$/m.test(raw)||failures.length)errors.push('every subject/assertion must pass');
 const wanted=[{subject:prefix,direct:210,public:0},...shards.map(subject=>({subject,direct:0,public:660}))].sort((a,b)=>a.subject.localeCompare(b.subject));
 if(!same(counters.sort((a,b)=>String(a.subject).localeCompare(String(b.subject))),wanted))errors.push('all210/four660 exact subject counters');
 if(diagnostics.length)errors.push('runtime sanitizer diagnostics');
 return {subjects:subjects.length,counters,failures,diagnostics,errors,complete:errors.length===0,rawSHA256:digest(raw)};
}
function auditEvidence(evidence){
 const fixture='tests/fixtures/vfp9-addlistitem-column-numeric';
 const read=f=>fs.readFileSync(path.join(evidence,f),'utf8');
 const status=f=>read(f).trim();
 assert.strictEqual(status('originals-complete-audit.status'),'0','completed BEFORE-wiring original audit');
 const gate=read('originals-complete-audit.jsonl').trim().split('\n').map(l=>JSON.parse(l)).at(-1);
 assert.strictEqual(gate.complete,true);assert.strictEqual(gate.identicalSelectedFullFailures,true);
 const parseManifest=raw=>raw.trim().split('\n').map(l=>{const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m,'manifest syntax');return {hash:m[1],file:m[2]};});
 const original=parseManifest(read('frozen-original-inputs.sha256'));
 const fixed=parseManifest(read('fixed-inputs.sha256'));
 assert.strictEqual(original.length,259);assert.strictEqual(fixed.length,259);
 assert.strictEqual(new Set(fixed.map(e=>e.file)).size,259);
 assert(same(original.map(e=>e.file),fixed.map(e=>e.file)),'all original paths retained');
 assert(same(fixed.filter((e,i)=>e.hash!==original[i].hash).map(e=>e.file),['src/runtime/prg_engine_runtime_surface_object_methods.inl']),'only bounded caller changed');
 for(const e of fixed)assert.strictEqual(digest(fs.readFileSync(e.file)),e.hash,'current fixed input '+e.file);
 const inventory=listNames(fs.readFileSync(fixture+'/pre-freeze-original95-test-list.log','utf8'));
 assert.strictEqual(status('fixed-driver.status'),'0','BOTH fixed driver completed');
 const results=[];
 for(const compiler of ['gcc','clang']){
  assert.strictEqual(status('fixed-compile-'+compiler+'.status'),'0','compile');
  assert.strictEqual(status('fixed-'+compiler+'.status'),'0','actual CTest status');
  assert(/^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\dZ$/.test(status('fixed-'+compiler+'.completed-at')));
  for(const phase of ['before','after'])assert(same(status('fixed-'+compiler+'-inputs-'+phase+'.log').split('\n'),fixed.map(e=>e.file+': OK')),'all259 '+phase);
  assert(same(listNames(read('fixed-'+compiler+'-test-list.log')),inventory),'unchanged inventory');
  assert.strictEqual(status('fixed-'+compiler+'-test-metadata.stderr'),'');
  assert.strictEqual(status('fixed-'+compiler+'-metadata-audit.stderr'),'');
  const metadata=status('fixed-'+compiler+'-metadata-audit.jsonl').split('\n').map(l=>JSON.parse(l));
  assert.strictEqual(metadata.length,6);assert.strictEqual(metadata[5].subjects,5);assert.strictEqual(metadata[5].requireCommands,'yes');assert(same(metadata[5].failures,[]));
  const raw=read('fixed-'+compiler+'.log'),result=auditRaw(raw,inventory);
  assert.strictEqual(status('fixed-'+compiler+'.log.sha256').split(/\s+/)[0],result.rawSHA256,'full raw hash');
  // Prior counter output is execution-coverage corroboration, not a behavioral oracle.
  const priorCounters=text=>text.split(/\r?\n/).filter(l=>/^\d+: .* frozen cases:/.test(l)&&!/^\d+: AddListItem COLUMN frozen cases:/.test(l)).map(l=>l.replace(/^\d+: /,'')).sort();
  assert(same(priorCounters(raw),priorCounters(read('original-'+compiler+'.log'))),'all prior counters retained');
  results.push({compiler,...result});
 }
 return {results,complete:results.every(r=>r.complete)};
}
module.exports={auditRaw,auditEvidence};
if(require.main===module){
 try{
  assert.strictEqual(process.argv.length,3,'Expected completed fixed95 evidence directory');
  const audit=auditEvidence(process.argv[2]);for(const result of audit.results)console.log(JSON.stringify(result));
  console.log(JSON.stringify({complete:audit.complete,note:'BOTH fixed95 source/counter/raw gate only; hostile182, preservation304, contracts, DQ/DV and exact-head integration remain separate required evidence.'}));
  process.exitCode=audit.complete?0:1;
 }catch(error){console.error(error.stack);process.exitCode=1;}
}
