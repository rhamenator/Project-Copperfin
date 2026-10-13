// Full BOTH-fixed-suite audit; expectations originate in frozen recovered tests.
const fs=require('fs'),crypto=require('crypto');
const paths=process.argv.slice(2);if(paths.length!==2)throw Error('Expected BOTH complete fixed85 logs');
let bad=0;
for(const path of paths){
 const raw=fs.readFileSync(path,'utf8'),lines=raw.split(/\r?\n/);
 const subjects=lines.map(l=>l.match(/^\s*(\d+)\/85 Test\s+#\d+:\s+(\S+)\s+\.{3,}\s*(Passed|\*\*\*Failed)/)).filter(Boolean);
 const failed=subjects.filter(m=>m[3]!=='Passed').map(m=>m[2]);
 const assertions=lines.filter(l=>/^\d+: FAIL:/.test(l));
 const diagnostics=lines.filter(l=>/AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|SUMMARY:.*Sanitizer|LeakSanitizer/.test(l));
 const counters=lines.map(l=>l.match(/^\d+: AddItem frozen cases: direct=(\d+) public=(\d+)$/)).filter(Boolean).map(m=>[+m[1],+m[2]]);
 const complete=subjects.length===85&&new Set(subjects.map(m=>m[2])).size===85&&
  subjects.some(m=>m[1]==='85')&&failed.length===0&&assertions.length===0&&diagnostics.length===0&&
  /100% tests passed, 0 tests failed out of 85/.test(raw)&&counters.length===5&&
  counters.filter(c=>c[0]===1246&&c[1]===0).length===1&&
  counters.filter(c=>c[0]===0&&c[1]===1102).length===4;
 if(!complete)++bad;
 console.log(JSON.stringify({path,subjects:subjects.length,failed,assertions,diagnostics,counters,complete,
  rawSHA256:crypto.createHash('sha256').update(raw).digest('hex')}));
}
console.log(JSON.stringify({auditFailures:bad,note:'BOTH complete fixed85/current statuses/source hashes and samecompiler supplemental288 comparisons required. Not native shared-success/globalcapacity/release qualification.'}));
process.exitCode=bad?1:0;
