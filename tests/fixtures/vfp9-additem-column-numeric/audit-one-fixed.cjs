// RQ-CF-PRG-ADDITEM-COLUMN-NUMERIC-001 / VR001/002.
// Audit ONE completed compiler, never substitute GNU-only for the BOTH gate.
const fs=require('fs'),crypto=require('crypto');
const compiler=process.argv[2];
if(!['gcc','clang'].includes(compiler))throw Error('Expected gcc or clang');
const root='build/additem-column-native/', read=p=>fs.readFileSync(root+p,'utf8');
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
let failures=[];const check=(b,label)=>{if(!b)failures.push(label);};
const status=p=>check(read(p).trim()==='0',p);
const checksum=p=>{
 const entries=read(p).trimEnd().split('\n');
 for(const line of entries){const m=line.match(/^([a-f0-9]{64})  (.+)$/);
  check(!!m,'manifest syntax '+p);if(m)check(sha(fs.readFileSync(m[2]))===m[1],'hash '+m[2]);}
 return entries.length;
};
const hashReport=(p,n)=>{const lines=read(p).trimEnd().split('\n');
 check(lines.length===n&&lines.every(l=>l.endsWith(': OK')),p);};
for(const p of [`fixed-configure-${compiler}.status`,`fixed-compile-${compiler}.status`,`fixed-${compiler}.status`])status(p);
const raw=read(`fixed-${compiler}.log`),lines=raw.split(/\r?\n/);
const subjects=lines.map(l=>l.match(/^\s*(\d+)\/90 Test\s+#\d+:\s+(\S+)\s+\.{3,}\s*(Passed|\*\*\*Failed)/)).filter(Boolean);
const prefix='test_prg_engine_additem_column_numeric';
const oldNames=fs.readFileSync('tests/fixtures/vfp9-additem-numeric/fixed-gcc-test-list.log','utf8')
 .split(/\r?\n/).map(l=>l.match(/Test\s+#\d+:\s+(\S+)/)?.[1]).filter(Boolean).sort();
const names=subjects.map(m=>m[2]);
check(subjects.length===90&&new Set(names).size===90&&subjects.every(m=>m[3]==='Passed')&&subjects.some(m=>m[1]==='90'),'90 unique completed PASS');
check(oldNames.length===85&&JSON.stringify(names.filter(n=>!n.startsWith(prefix)).sort())===JSON.stringify(oldNames),'prior85 discovery unchanged');
check(/100% tests passed, 0 tests failed out of 90/.test(raw),'CTest complete summary');
check(!/FAIL:|AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|SUMMARY:.*Sanitizer|LeakSanitizer/.test(raw),'assertion/runtime diagnostics');
const counters=lines.map(l=>l.match(/^\d+: AddItem COLUMN frozen cases: direct=(\d+) public=(\d+)$/)).filter(Boolean).map(m=>[+m[1],+m[2]]);
check(counters.length===5&&counters.filter(c=>c[0]===276&&c[1]===0).length===1&&counters.filter(c=>c[0]===0&&c[1]===1224).length===4,'all frozen counters');
for(const phase of ['before','after'])hashReport(`fixed-${compiler}-inputs-${phase}.log`,256);
check(checksum('fixed-inputs.sha256')===256,'fixed256 manifest count');
check(checksum('initial-five.sha256')===5,'initialfive manifest count');
checksum(`fixed-${compiler}.log.sha256`);
const hostile=[];
for(const mode of ['COPPERFIN','VFP9'])for(const session of [1,2]){
 const stem=`hostile-fixed-${compiler}-${mode}-session${session}`;
 status(stem+'.status');checksum(stem+'.log.sha256');
 const out=read(stem+'.log');check(out.trim()==='AddItem COLUMN frozen cases: direct=0 public=542',stem+' complete hostile542/no diagnostics');
 hostile.push({mode,session,calls:542,rawSHA256:sha(out)});
}
hashReport(`hostile-fixed-${compiler}-inputs-after.log`,256);
const stem=`preservation-fixed-${compiler}`;
for(const p of ['.status','-compile.status','-driver.status','.cmp.status'])status(stem+p);
check(read(stem+'.stderr')==='','preservation stderr empty');
check(read(stem+'.cmp.log')==='','full samecompiler cmp output empty');
check(checksum('preservation-source-frozen.sha256')===3,'preservation source count');
check(checksum(stem+'-libraries.sha256')===9,'preservation nine libraries');
checksum(stem+'.out.sha256');
for(const phase of ['before','after'])hashReport(stem+'-source-'+phase+'.log',3);
hashReport(stem+'-libraries-after.log',9);
const preserved=read(stem+'.out'),baseline=read(`preservation-baseline-${compiler}.out`);
check(preserved===baseline,'samecompiler fullraw equality');
const rows=preserved.trimEnd().split(/\r?\n/),done=rows.pop();
check(rows.length===296&&done==='DONE|calls=296|scaffold_failures=0','preservation296 complete');
const groups={};for(const row of rows){const key=row.split('|').slice(0,3).join('|');groups[key]=(groups[key]||0)+1;
 check(/\|completed=1\|/.test(row),'preservation invocation complete');}
for(const cls of ['ListBox','ComboBox'])for(const mode of ['COPPERFIN','VFP9'])for(const session of [1,2])check(groups[`${cls}|${mode}|${session}`]===37,'preservation context '+cls+mode+session);
check(Object.keys(groups).length===8,'exactly eight preservation contexts');
console.log(JSON.stringify({compiler,subjects:subjects.length,counters,totalSeconds:raw.match(/Total Test time \(real\) = ([\d.]+) sec/)?.[1],
 completedAt:read(`fixed-${compiler}.completed-at`).trim(),rawSHA256:sha(raw),hostile,
 preservation:{rows:rows.length,groups,rawSHA256:sha(preserved),sameCompilerFullRawEqual:preserved===baseline},failures,
 note:'ONE compiler only. Fixed-only hostile corroboration is not original hostile qualification. NON-NATIVE sampled preservation is not a native oracle, full property/global atomicity/leak/warning-free/system/release qualification. Both complete compiler audits plus contracts/DQ-DV/review/CI gates remain necessary.'},null,2));
process.exitCode=failures.length?1:0;
