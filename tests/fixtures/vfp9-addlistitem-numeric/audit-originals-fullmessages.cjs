// Full original-suite audit. Failure messages originate in independent frozen tests, not helpers.
const fs=require('fs'),crypto=require('crypto');
const paths=process.argv.slice(2);
// Retain COMPLETE multiline assertion messages, including configuration/source continuations.
if(paths.length!==2){console.error('Expected full GNU and Clang refrozen original logs');process.exit(2);}
const expected=['test_prg_engine_addlistitem_numeric_copperfin_session1',
 'test_prg_engine_addlistitem_numeric_copperfin_session2',
 'test_prg_engine_addlistitem_numeric_vfp9_session1',
 'test_prg_engine_addlistitem_numeric_vfp9_session2'].sort();
let bad=0;
const summaries=[];
for(const path of paths){
 const raw=fs.readFileSync(path,'utf8'),lines=raw.split(/\r?\n/);
 const subjectRows=lines.map(l=>l.match(/^\s*(\d+)\/80 Test\s+#\d+:\s+(\S+)\s+\.\.\.\s+(Passed|\*\*\*Failed)/)).filter(Boolean);
 const failed=subjectRows.filter(m=>m[3]==='***Failed').map(m=>m[2]).sort();
 const errors=[];let current=null,number=null;
 const flush=()=>{if(current!==null)errors.push(current.join('\n'));current=null;number=null;};
 for(const line of lines){
  const start=line.match(/^(\d+): FAIL: (.*)$/);
  if(start){flush();number=start[1];current=[start[2]];continue;}
  if(current!==null){
   const next=line.match(/^(\d+): (.*)$/);
   if(next&&next[1]===number&&!/^AddListItem frozen cases:/.test(next[2]))current.push(next[2]);
   else flush();
  }
 }
 flush();
 const unrelated=errors.filter(x=>!/^AddListItem (admission error|invocation continuation|complete rejection property state|coherent materialized count|independently expected stable IDs|rejected selection retained|localized11) /.test(x));
 const counters=lines.map(l=>l.match(/^\d+: AddListItem frozen cases: direct=(\d+) public=(\d+)$/)).filter(Boolean).map(m=>[Number(m[1]),Number(m[2])]);
 const diagnostics=lines.filter(l=>/AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|SUMMARY:.*Sanitizer|LeakSanitizer/.test(l));
 const complete=subjectRows.length===80&&subjectRows.some(m=>m[1]==='80')&&
  /95% tests passed, 4 tests failed out of 80/.test(raw)&&
  JSON.stringify(failed)===JSON.stringify(expected)&&unrelated.length===0&&
  counters.length===5&&counters.filter(c=>c[0]===198&&c[1]===0).length===1&&
  counters.filter(c=>c[0]===0&&c[1]===810).length===4&&
  errors.length>0&&diagnostics.length===0;
 if(!complete)++bad;
 const normalized=errors.slice().sort();
 const summary={path,subjects:subjectRows.length,failed,assertionFailures:errors.length,
  unrelated,diagnostics,counters,complete,rawSHA256:crypto.createHash('sha256').update(raw).digest('hex'),
  failureSHA256:crypto.createHash('sha256').update(normalized.join('\n')).digest('hex')};
 summaries.push(summary);console.log(JSON.stringify(summary));
 for(const error of errors)console.log(JSON.stringify({path,failure:error}));
}
if(summaries[0].failureSHA256!==summaries[1].failureSHA256)++bad;
console.log(JSON.stringify({auditFailures:bad,identicalSelectedFailures:summaries[0].failureSHA256===summaries[1].failureSHA256,
 note:'Both complete original80 required. Metadata-only refreeze; original runtime arm remains unchanged. No fixed implementation acceptance.'}));
process.exitCode=bad?1:0;
