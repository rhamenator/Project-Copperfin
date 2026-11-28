// RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR-5611-LISTITEM-COLUMN-NUMERIC-001/002.
// Completed original failures are recovery evidence, never product acceptance.
const fs=require('fs'),path=require('path'),crypto=require('crypto'),assert=require('assert');
const direct='test_prg_engine_listitem_column_numeric',prefix='test_prg_engine_listitem_column_numeric_public';
const shards=['copperfin','vfp9'].flatMap(m=>[1,2].map(s=>prefix+'_'+m+'_session'+s)).sort();
const current=[direct,prefix,...shards].sort(),digest=s=>crypto.createHash('sha256').update(s).digest('hex');
const same=(a,b)=>JSON.stringify(a)===JSON.stringify(b);
const listNames=raw=>raw.split(/\r?\n/).map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]??l.match(/^(test_\S+)$/)?.[1]).filter(Boolean).sort();
function auditRaw(raw,inventory){
 const lines=raw.split(/\r?\n/),failures=[],counters=[],directPass=[],starts=new Map();
 let block=null,number=null,startCount=0;
 const flush=()=>{if(block!==null)failures.push({subject:starts.get(number),text:block.join('\n')});block=null;number=null;};
 for(const line of lines){
  const start=line.match(/^\s*Start\s+(\d+):\s+(\S+)$/);if(start){++startCount;starts.set(start[1],start[2]);}
  const fail=line.match(/^(\d+): FAIL: (.*)$/);if(fail){flush();number=fail[1];block=[fail[2]];continue;}
  if(block!==null){const continuation=line.match(/^(\d+): (.*)$/);if(continuation&&continuation[1]===number&&!/^ListItem supplied-column frozen public cases=/.test(continuation[2]))block.push(continuation[2]);else flush();}
  const count=line.match(/^(\d+): ListItem supplied-column frozen public cases=(\d+)$/);if(count)counters.push({subject:starts.get(count[1]),public:Number(count[2])});
  const passed=line.match(/^(\d+): ListItem supplied-column decimal conversion foundation passed$/);if(passed)directPass.push(starts.get(passed[1]));
 }
 flush();
 const subjects=lines.map(l=>l.match(/^\s*(\d+)\/106 Test\s+#(\d+):\s+(\S+)\s+\.{3,}\s*(Passed|\*\*\*Failed)\s+[\d.]+ sec$/)).filter(Boolean);
 const names=subjects.map(m=>m[3]).sort(),failed=subjects.filter(m=>m[4]==='***Failed').map(m=>m[3]).sort();
 const prior=names.filter(n=>!current.includes(n)),expectedPrior=inventory.filter(n=>!current.includes(n));
 const unrelated=failures.filter(f=>!shards.includes(f.subject)||!/^ListItem COLUMN /.test(f.text));
 const diagnostics=lines.filter(l=>/AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|SUMMARY:.*Sanitizer|LeakSanitizer/.test(l));
 const errors=[];
 if(inventory.length!==106||new Set(inventory).size!==106||expectedPrior.length!==100)errors.push('configured inventory106/prior100');
 if(subjects.length!==106||new Set(names).size!==106||!same(names,inventory))errors.push('complete unique execution106');
 if(startCount!==106||starts.size!==106||subjects.some(m=>starts.get(m[2])!==m[3]))errors.push('unique starts match every result');
 if(!same(subjects.map(m=>Number(m[1])),Array.from({length:106},(_,i)=>i+1)))errors.push('complete ordered CTest ordinals');
 if(!same(prior,expectedPrior)||subjects.filter(m=>expectedPrior.includes(m[3])).some(m=>m[4]!=='Passed'))errors.push('all prior100 must pass');
 if(!same(failed,shards)||!/^96% tests passed, 4 tests failed out of 106$/m.test(raw))errors.push('only four public shards fail');
 if(!subjects.some(m=>m[3]===direct&&m[4]==='Passed')||!subjects.some(m=>m[3]===prefix&&m[4]==='Passed'))errors.push('direct and bare inventory pass');
 const wanted=[{subject:prefix,public:0},...shards.map(subject=>({subject,public:92}))].sort((a,b)=>a.subject.localeCompare(b.subject));
 if(!same(counters.sort((a,b)=>String(a.subject).localeCompare(String(b.subject))),wanted)||!same(directPass,[direct]))errors.push('direct/bare/four shard counters');
 if(failures.length===0||unrelated.length)errors.push('selected full failure blocks only');
 if(diagnostics.length)errors.push('unexpected sanitizer diagnostic');
 const canonicalFailures=failures.map(f=>f.subject+'\n'+f.text).sort().join('\n');
 return {subjects:subjects.length,failed,counters,directPass,assertionFailures:failures.length,failures,unrelated,diagnostics,errors,rawSHA256:digest(raw),failureSHA256:digest(canonicalFailures),complete:errors.length===0};
}
function auditEvidence(evidence){
 const fixture='tests/fixtures/vfp9-listitem-column-numeric';
 const inventory=listNames(fs.readFileSync(fixture+'/pre-freeze-original106-test-names.txt','utf8'));
 const manifest=fs.readFileSync(fixture+'/frozen-original-inputs.sha256','utf8');
 assert.strictEqual(manifest,fs.readFileSync(path.join(evidence,'frozen-original-inputs.sha256'),'utf8'),'frozen manifest copies');
 const entries=manifest.trim().split('\n').map(l=>{const m=l.match(/^([a-f0-9]{64})  (.+)$/);assert(m,'manifest syntax');return {hash:m[1],file:m[2]};});
 assert.strictEqual(new Set(entries.map(e=>e.file)).size,entries.length,'unique frozen paths');
 for(const e of entries)assert.strictEqual(digest(fs.readFileSync(e.file)),e.hash,'current frozen input '+e.file);
 const status=f=>fs.readFileSync(path.join(evidence,f),'utf8').trim();
 assert.strictEqual(status('originals-driver.status'),'0','BOTH driver completed');
 const results=[];
 for(const compiler of ['gcc','clang']){
  for(const step of ['configure','compile'])assert.strictEqual(status('original-'+step+'-'+compiler+'.status'),'0',compiler+' '+step);
  assert.strictEqual(status('original-'+compiler+'.status'),'8',compiler+' expected failing CTest status');
  for(const phase of ['before','after'])assert(same(status('original-'+compiler+'-inputs-'+phase+'.log').split('\n'),entries.map(e=>e.file+': OK')),compiler+' exact inputs '+phase);
  assert(same(listNames(fs.readFileSync(path.join(evidence,'original-'+compiler+'-test-list.log'),'utf8')),inventory),compiler+' inventory unchanged');
  assert.strictEqual(status('original-'+compiler+'-test-metadata.stderr'),'','metadata stderr');
  assert.strictEqual(status('original-'+compiler+'-metadata-audit.stderr'),'','metadata audit stderr');
  const metadata=status('original-'+compiler+'-metadata-audit.jsonl').split('\n').map(l=>JSON.parse(l));
  assert.strictEqual(metadata.length,7,'six records plus summary');assert.strictEqual(metadata[6].subjects,6);assert(same(metadata[6].failures,[]));
  const raw=fs.readFileSync(path.join(evidence,'original-'+compiler+'.log'),'utf8'),result=auditRaw(raw,inventory);
  assert.strictEqual(status('original-'+compiler+'.log.sha256').split(/\s+/)[0],result.rawSHA256,'raw log hash');results.push({compiler,...result});
 }
 const identical=results[0].failureSHA256===results[1].failureSHA256;
 return {results,identicalSelectedFullFailures:identical,complete:results.every(r=>r.complete)&&identical};
}
module.exports={auditRaw,auditEvidence};
if(require.main===module){try{assert.strictEqual(process.argv.length,3,'Expected completed original106 evidence directory');const audit=auditEvidence(process.argv[2]);for(const r of audit.results){const {failures,...summary}=r;console.log(JSON.stringify(summary));for(const failure of failures)console.log(JSON.stringify({compiler:r.compiler,...failure}));}console.log(JSON.stringify({complete:audit.complete,identicalSelectedFullFailures:audit.identicalSelectedFullFailures,note:'BOTH completed original106 audit only; failures are not fixed acceptance.'}));process.exitCode=audit.complete?0:1;}catch(error){console.error(error.stack);process.exitCode=1;}}
