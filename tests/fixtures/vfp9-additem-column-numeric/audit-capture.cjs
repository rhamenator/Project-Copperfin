// Independent capture consistency only, no conversion helper or acceptance oracle.
const fs=require('fs'),crypto=require('crypto');
const [first,repeat,expected]=process.argv.slice(2),count=Number(expected);
const raw=fs.readFileSync(first),again=fs.readFileSync(repeat);
const lines=raw.toString().trim().split(/\r?\n/);
let bad=raw.equals(again)?0:1,rejected=0,inserted=0;
if(lines.length!==count+2||!lines[0].includes('09.00.0000.7423')||lines.at(-1)!=='DONE|'+count)++bad;
for(const line of lines.slice(1,-1)){
 const a=line.split('|'),get=k=>a.find(f=>f.startsWith(k+'='))?.slice(k.length+1);
 const before=get('before'),after=get('after'),error=Number(get('error'));
 const n=Number(before.match(/^count=(\d+)/)[1]),m=Number(after.match(/^count=(\d+)/)[1]);
 const same=before===after;
 const candidates=[...after.matchAll(/,cell(\d+)=candidate(?:,|;|$)/g)].map(x=>Number(x[1]));
 let ok=a[4]==='invoke';
 if(error===11){++rejected;ok&&=same;}
 else if(error===0){++inserted;ok&&=m===n+1;}
 else ok=false;
 if(!ok)++bad;
 console.log(JSON.stringify({cls:a[0],columns:get('columns'),shape:get('shape'),source:a[3],phase:a[4],
  error,beforeCount:n,afterCount:m,unchanged:same,candidateVisibleColumns:candidates,consistent:ok}));
}
console.log(JSON.stringify({rows:lines.length-2,rejected,inserted,failures:bad,
 rawSHA256:crypto.createHash('sha256').update(raw).digest('hex'),
 note:'Capture consistency and visible cells only. Not native global capacity, hidden storage, default policy or implementation acceptance.'}));
process.exitCode=bad?1:0;
