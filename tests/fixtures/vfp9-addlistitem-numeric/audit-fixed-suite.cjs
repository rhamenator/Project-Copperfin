// Verification-only completion audit, never a source of behavior expectations.
const fs=require('fs'),crypto=require('crypto');
if(process.argv.length!==3){console.error('Expected ONE completed fixed80 raw');process.exit(2);}
const path=process.argv[2],raw=fs.readFileSync(path),text=raw.toString();
const lines=text.split(/\r?\n/);
const rows=lines.map(l=>l.match(/^\s*(\d+)\/80 Test\s+#\d+:\s+(\S+)\s+\.{3,}\s*(Passed|\*\*\*Failed)/)).filter(Boolean);
const counters=lines.map(l=>l.match(/^\d+: AddListItem frozen cases: direct=(\d+) public=(\d+)$/)).filter(Boolean).map(m=>[+m[1],+m[2]]);
const diagnostics=lines.filter(l=>/FAIL:|runtime error:|AddressSanitizer|UndefinedBehaviorSanitizer|LeakSanitizer|SUMMARY:.*Sanitizer/.test(l));
const indices=rows.map(m=>+m[1]);
const pass=rows.length===80&&new Set(indices).size===80&&indices.every((n,i)=>n===i+1)&&
 rows.every(m=>m[3]==='Passed')&&new Set(rows.map(m=>m[2])).size===80&&
 /100% tests passed, 0 tests failed out of 80/.test(text)&&diagnostics.length===0&&
 counters.length===5&&counters.filter(c=>c[0]===198&&c[1]===0).length===1&&
 counters.filter(c=>c[0]===0&&c[1]===810).length===4;
console.log(JSON.stringify({path,pass,subjects:rows.length,counters,diagnostics,
 rawSHA256:crypto.createHash('sha256').update(raw).digest('hex'),
 note:'ONE compiler completion only; other compiler, input hashes, preservation and integration gates remain separate.'}));
process.exitCode=pass?0:1;
