// Full BOTH-original audit; retain entire multiline assertion text, not prefixes.
// Evidence must complete before any caller wiring. Never execute hostile corpus.
const fs=require('fs'),crypto=require('crypto');
const paths=process.argv.slice(2);if(paths.length!==2)throw Error('Expected GNU and Clang full original90 logs');
const prefix='test_prg_engine_additem_column_numeric';
const expected=[prefix+'_copperfin_session1',prefix+'_copperfin_session2',prefix+'_vfp9_session1',prefix+'_vfp9_session2'].sort();
const oldNames=fs.readFileSync('tests/fixtures/vfp9-additem-numeric/fixed-gcc-test-list.log','utf8')
 .split(/\r?\n/).map(l=>l.match(/Test\s+#\d+:\s+(\S+)/)?.[1]).filter(Boolean).sort();
if(oldNames.length!==85)throw Error('Prior85 discovery evidence incomplete');
let bad=0;const results=[];
for(const path of paths){
 const raw=fs.readFileSync(path,'utf8'),lines=raw.split(/\r?\n/);
 const subjects=lines.map(l=>l.match(/^\s*(\d+)\/90 Test\s+#\d+:\s+(\S+)\s+\.{3,}\s*(Passed|\*\*\*Failed)/)).filter(Boolean);
 const names=subjects.map(m=>m[2]),failed=subjects.filter(m=>m[3]==='***Failed').map(m=>m[2]).sort();
 const prior=names.filter(n=>!n.startsWith(prefix)).sort();
 const failures=[];let current=null,testNumber=null;
 const flush=()=>{if(current!==null)failures.push(current.join('\n'));current=null;testNumber=null;};
 for(const line of lines){
  const start=line.match(/^(\d+): FAIL: (.*)$/);
  if(start){flush();testNumber=start[1];current=[start[2]];continue;}
  if(current!==null){const next=line.match(/^(\d+): (.*)$/);
   if(next&&next[1]===testNumber&&!/^AddItem COLUMN frozen cases:/.test(next[2]))current.push(next[2]);else flush();
  }
 }
 flush();
 const unrelated=failures.filter(x=>!/^AddItem COLUMN (admission error|invocation continuation|complete rejection property state|coherent count|retained ID|retained primary cell|retained cells|retained payload|independently observed visible placement|derived argument-driven row bound|rejected selection retained|localized11) /.test(x));
 const counters=lines.map(l=>l.match(/^\d+: AddItem COLUMN frozen cases: direct=(\d+) public=(\d+)$/))
  .filter(Boolean).map(m=>[Number(m[1]),Number(m[2])]);
 const diagnostics=lines.filter(l=>/AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|SUMMARY:.*Sanitizer|LeakSanitizer/.test(l));
 const complete=subjects.length===90&&new Set(names).size===90&&subjects.some(m=>m[1]==='90')&&
  /4 tests failed out of 90/.test(raw)&&JSON.stringify(failed)===JSON.stringify(expected)&&
  JSON.stringify(prior)===JSON.stringify(oldNames)&&unrelated.length===0&&diagnostics.length===0&&failures.length>0&&
  counters.length===5&&counters.filter(c=>c[0]===276&&c[1]===0).length===1&&
  counters.filter(c=>c[0]===0&&c[1]===1224).length===4;
 if(!complete)++bad;
 const result={path,subjects:subjects.length,prior85Unchanged:JSON.stringify(prior)===JSON.stringify(oldNames),
  failed,counters,assertionFailures:failures.length,unrelated,diagnostics,complete,
  rawSHA256:crypto.createHash('sha256').update(raw).digest('hex'),
  failureSHA256:crypto.createHash('sha256').update(failures.slice().sort().join('\n')).digest('hex')};
 results.push(result);console.log(JSON.stringify(result));
 for(const failure of failures)console.log(JSON.stringify({path,failure}));
}
const identical=results[0].failureSHA256===results[1].failureSHA256;if(!identical)++bad;
console.log(JSON.stringify({auditFailures:bad,identicalSelectedFailures:identical,
 note:'Both complete original90/allprior85 plus new276 direct must pass; only four safe-original1224 public subjects fail. Fixed-only hostile corpus is separately authored and NOT executed or qualified here.'}));
process.exitCode=bad?1:0;
