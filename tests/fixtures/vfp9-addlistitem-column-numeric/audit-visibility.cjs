// Native deferred visibility observations only; no helper/default expectations.
const fs=require('fs'),crypto=require('crypto');
const [first,repeat]=process.argv.slice(2),raw=fs.readFileSync(first),again=fs.readFileSync(repeat);
const lines=raw.toString().trim().split(/\r?\n/);let failures=[];const groups={};
if(!raw.equals(again))failures.push('independent repeat mismatch');
if(lines.length!==62||!lines[0].includes('09.00.0000.7423')||lines.at(-1)!=='DONE|60')failures.push('complete VERSION/DONE60');
const visible=s=>[...s.matchAll(/;row=(\d+),[^;]*?(?=;row=|$)/g)].flatMap(m=>[...m[0].matchAll(/,cell(\d+)=candidate(?:,|;|$)/g)].map(c=>({id:+m[1],column:+c[1]})));
for(const line of lines.slice(1,-1)){
 const a=line.split('|'),get=k=>a.find(f=>f.startsWith(k+'='))?.slice(k.length+1),key=a.slice(0,3).join('|');
 groups[key]=(groups[key]||0)+1;
 if(a[4]!=='invoke'||Number(get('expandError'))!==0||!/^count=3,columns=9,/.test(get('expanded')))failures.push('invocation/expansion framing '+key+' '+a[3]);
 console.log(JSON.stringify({cls:a[0],columns:+get('columns'),target:+get('target'),source:a[3],phase:a[4],error:+get('error'),expandError:+get('expandError'),preBRS:get('preBRS'),beforeAfterEqual:get('before')===get('after'),immediateCandidate:visible(get('after')),expandedTo9Candidate:visible(get('expanded'))}));
}
if(Object.keys(groups).length!==6||Object.values(groups).some(n=>n!==10))failures.push('six contexts x10');
console.log(JSON.stringify({rows:lines.length-2,groups,failures,rawSHA256:crypto.createHash('sha256').update(raw).digest('hex'),note:'Deferred visible cells1..9 only; no all-high hidden-storage/native capacity, neighboring low16, default/exact extension or implementation requirement inferred.'}));
process.exitCode=failures.length?1:0;
