// Pre-code capture consistency only; not Copperfin/helper acceptance expectations.
const fs=require('fs'),path=require('path'),crypto=require('crypto');
const root=process.argv[2];let failures=0,total=0,rejections=0,inserts=0;
for(const [name,count] of [['native',440],['configuration',96],['boundary',224],['sorted',48],['count',220]]){
 const raw=fs.readFileSync(path.join(root,name+'-first.out'));
 const repeat=fs.readFileSync(path.join(root,name+'-repeat.out'));
 if(!raw.equals(repeat))++failures;
 const lines=raw.toString().trim().split(/\r?\n/);
 if(lines.length!==count+2||!lines[0].includes('09.00.0000.7423')||lines.at(-1)!=='DONE|'+count)++failures;
 for(const line of lines.slice(1,-1)){
  const fields=line.split('|'),get=k=>fields.find(x=>x.startsWith(k+'='))?.slice(k.length+1);
  const before=get('before'),after=get('after');
  const n=+before.match(/^count=(\d+)/)[1],m=+after.match(/^count=(\d+)/)[1];
  const error=+get('error');let ok=fields[3]==='invoke';
  if(error===11){++rejections;ok&&=before===after;}
  else if(error===0){++inserts;ok&&=m===n+1;}
  else ok=false;
  if(!ok)++failures;++total;
  console.log(JSON.stringify({pair:name,cls:fields[0],shape:fields[1],source:fields[2],beforeCount:n,afterCount:m,error,consistent:ok}));
 }
 console.log(JSON.stringify({pair:name,rows:count,sha256:crypto.createHash('sha256').update(raw).digest('hex')}));
}
const original=fs.readFileSync(path.join(root,'boundary-first.out'),'utf8');
const post=fs.readFileSync(path.join(root,'boundary-postbits-first.out'),'utf8');
const identical=original===post.replace(/\|postcallBRS=[0-9A-F]+/g,'');
if(!identical)++failures;
console.log(JSON.stringify({total,rejections,inserts,failures,postcallStrippedExactlyOriginal:identical,
 note:'Capture consistency only, no global capacity/shared-success parity or implementation acceptance.'}));
process.exitCode=failures?1:0;
