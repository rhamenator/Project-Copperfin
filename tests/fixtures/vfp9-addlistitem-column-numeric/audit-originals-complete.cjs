// RQ-CF-PRG-ADDLISTITEM-COLUMN-NUMERIC-001 / VR-5611-ADDLISTITEM-COLUMN-NUMERIC-001/002.
// Audit tooling authored AFTER the original259 freeze, WITHOUT changing inputs.
// Completed original failures are recovery evidence, never product acceptance.
const fs=require('fs'),path=require('path'),crypto=require('crypto'),assert=require('assert');
const prefix='test_prg_engine_addlistitem_column_numeric';
const shards=['copperfin','vfp9'].flatMap(m=>[1,2].map(s=>prefix+'_'+m+'_session'+s)).sort();
const digest=s=>crypto.createHash('sha256').update(s).digest('hex');
const same=(a,b)=>JSON.stringify(a)===JSON.stringify(b);
const listNames=raw=>raw.split(/\r?\n/).map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]).filter(Boolean).sort();
function auditRaw(raw,inventory){
 const lines=raw.split(/\r?\n/),failures=[],counters=[],starts=new Map();
 let block=null,number=null,startCount=0;
 const flush=()=>{if(block!==null)failures.push({subject:starts.get(number),text:block.join('\n')});block=null;number=null;};
 for(const line of lines){
  const start=line.match(/^\s*Start\s+(\d+):\s+(\S+)$/);
  if(start){++startCount;starts.set(start[1],start[2]);}
  const fail=line.match(/^(\d+): FAIL: (.*)$/);
  if(fail){flush();number=fail[1];block=[fail[2]];continue;}
  if(block!==null){
   const continuation=line.match(/^(\d+): (.*)$/);
   if(continuation&&continuation[1]===number&&!/^AddListItem COLUMN frozen cases:/.test(continuation[2]))block.push(continuation[2]);
   else flush();
  }
  const count=line.match(/^(\d+): AddListItem COLUMN frozen cases: direct=(\d+) public=(\d+)$/);
  if(count)counters.push({subject:starts.get(count[1]),direct:Number(count[2]),public:Number(count[3])});
 }
 flush();
 const subjects=lines.map(l=>l.match(/^\s*(\d+)\/95 Test\s+#(\d+):\s+(\S+)\s+\.{3,}\s*(Passed|\*\*\*Failed)\s+[\d.]+ sec$/)).filter(Boolean);
 const names=subjects.map(m=>m[3]).sort(),failed=subjects.filter(m=>m[4]==='***Failed').map(m=>m[3]).sort();
 const prior=names.filter(n=>n!==prefix&&!shards.includes(n));
 const expectedPrior=inventory.filter(n=>n!==prefix&&!shards.includes(n));
 const allowed=/^AddListItem COLUMN (admission|invocation continuation|localized11|rejected modeled metadata|derived argument-driven row bound|complete retained\/selected row model|candidate\/no-write count|selected stored column|rejected selection) /;
 const unrelated=failures.filter(f=>!shards.includes(f.subject)||!allowed.test(f.text));
 const diagnostics=lines.filter(l=>/AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|SUMMARY:.*Sanitizer|LeakSanitizer/.test(l));
 const errors=[];
 if(inventory.length!==95||new Set(inventory).size!==95||expectedPrior.length!==90)errors.push('configured inventory95/prior90');
 if(subjects.length!==95||new Set(names).size!==95||!same(names,inventory))errors.push('complete unique execution95');
 if(startCount!==95||starts.size!==95||subjects.some(m=>starts.get(m[2])!==m[3]))errors.push('unique starts match every result ID/name');
 if(!same(subjects.map(m=>Number(m[1])),Array.from({length:95},(_,i)=>i+1)))errors.push('complete ordered CTest ordinals');
 if(!same(prior,expectedPrior)||subjects.filter(m=>expectedPrior.includes(m[3])).some(m=>m[4]!=='Passed'))errors.push('all prior90 must pass');
 if(!same(failed,shards)||!/^96% tests passed, 4 tests failed out of 95$/m.test(raw))errors.push('only four new public failures');
 if(!subjects.some(m=>m[3]===prefix&&m[4]==='Passed'))errors.push('new unused helper direct must pass');
 const wanted=[{subject:prefix,direct:210,public:0},...shards.map(subject=>({subject,direct:0,public:660}))].sort((a,b)=>a.subject.localeCompare(b.subject));
 if(!same(counters.sort((a,b)=>String(a.subject).localeCompare(String(b.subject))),wanted))errors.push('all five subject-specific210/660 counters');
 if(failures.length===0||unrelated.length)errors.push('full selected failure blocks, no unrelated assertions');
 if(diagnostics.length)errors.push('unexpected runtime sanitizer diagnostic');
 const canonicalFailures=failures.map(f=>f.subject+'\n'+f.text).sort().join('\n');
 return {subjects:subjects.length,failed,counters,assertionFailures:failures.length,failures,unrelated,diagnostics,errors,
  rawSHA256:digest(raw),failureSHA256:digest(canonicalFailures),complete:errors.length===0};
}
function auditEvidence(evidence){
 const fixture='tests/fixtures/vfp9-addlistitem-column-numeric';
 const inventory=listNames(fs.readFileSync(fixture+'/pre-freeze-original95-test-list.log','utf8'));
 const manifest=fs.readFileSync(fixture+'/frozen-original-inputs.sha256','utf8');
 assert.strictEqual(manifest,fs.readFileSync(path.join(evidence,'frozen-original-inputs.sha256'),'utf8'),'frozen manifest copies');
 const entries=manifest.trim().split('\n').map(l=>{const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m,'manifest syntax');return {hash:m[1],file:m[2]};});
 assert.strictEqual(entries.length,259,'original259 inventory');
 assert.strictEqual(new Set(entries.map(e=>e.file)).size,259,'unique frozen paths');
 for(const e of entries)assert.strictEqual(digest(fs.readFileSync(e.file)),e.hash,'current frozen input '+e.file);
 const callerManifest=fs.readFileSync(fixture+'/unchanged-three-caller-inputs.sha256','utf8').trim().split('\n');
 assert.strictEqual(callerManifest.length,3,'three original caller inputs');
 for(const l of callerManifest){const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m);assert.strictEqual(digest(fs.readFileSync(m[2])),m[1],'unchanged caller '+m[2]);}
 const status=f=>fs.readFileSync(path.join(evidence,f),'utf8').trim();
 assert.strictEqual(status('originals-driver.status'),'0','BOTH driver completed');
 const results=[];
 for(const compiler of ['gcc','clang']){
  for(const step of ['configure','compile'])assert.strictEqual(status('original-'+step+'-'+compiler+'.status'),'0',compiler+' '+step);
  assert.strictEqual(status('original-'+compiler+'.status'),'8',compiler+' actual expected failing CTest status');
  assert(/^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\dZ$/.test(status('original-'+compiler+'.completed-at')),'completion timestamp');
  for(const phase of ['before','after']){
   const checks=status('original-'+compiler+'-inputs-'+phase+'.log').split('\n');
   assert(same(checks,entries.map(e=>e.file+': OK')),compiler+' exact259 '+phase);
  }
  assert(same(listNames(fs.readFileSync(path.join(evidence,'original-'+compiler+'-test-list.log'),'utf8')),inventory),compiler+' configured inventory unchanged');
  assert.strictEqual(status('original-'+compiler+'-test-metadata.stderr'),'','metadata stderr');
  assert.strictEqual(status('original-'+compiler+'-metadata-audit.stderr'),'','built command/isolation stderr');
  const metadata=status('original-'+compiler+'-metadata-audit.jsonl').split('\n').map(l=>JSON.parse(l));
  assert.strictEqual(metadata.length,6,'five built subject records plus summary');
  assert.strictEqual(metadata[5].subjects,5);assert.strictEqual(metadata[5].requireCommands,'yes');assert(same(metadata[5].failures,[]));
  const raw=fs.readFileSync(path.join(evidence,'original-'+compiler+'.log'),'utf8');
  const result=auditRaw(raw,inventory);
  assert.strictEqual(status('original-'+compiler+'.log.sha256').split(/\s+/)[0],result.rawSHA256,'raw log hash');
  results.push({compiler,...result});
 }
 const identical=results[0].failureSHA256===results[1].failureSHA256;
 return {results,identicalSelectedFullFailures:identical,complete:results.every(r=>r.complete)&&identical};
}
module.exports={auditRaw,auditEvidence};
if(require.main===module){
 try{
  assert.strictEqual(process.argv.length,3,'Expected completed original95 evidence directory');
  const audit=auditEvidence(process.argv[2]);
  for(const r of audit.results){const {failures,...summary}=r;console.log(JSON.stringify(summary));for(const failure of failures)console.log(JSON.stringify({compiler:r.compiler,...failure}));}
  console.log(JSON.stringify({complete:audit.complete,identicalSelectedFullFailures:audit.identicalSelectedFullFailures,
   note:'BOTH completed original95/source259/caller3 audit only. Original failures are NOT fixed acceptance; hostile corpus is NOT executed here.'}));
  process.exitCode=audit.complete?0:1;
 }catch(error){console.error(error.stack);process.exitCode=1;}
}
