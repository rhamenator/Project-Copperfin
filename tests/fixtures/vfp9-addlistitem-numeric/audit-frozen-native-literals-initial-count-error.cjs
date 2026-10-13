// Audit frozen literal expectations against independent native raws, never helper output.
const fs=require('fs'),path=require('path');
const root=process.argv[2],test=process.argv[3];
const text=fs.readFileSync(test,'utf8');
const block=text.split('const std::vector<NumberCase> numbers{')[1].split('\n};')[0];
const rows=[...block.matchAll(/\{"([^"]+)",([\s\S]*?),(\{\}|\d+),(\{\}|\d+)\}/g)]
 .map(m=>({source:m[1],modern:m[3],legacy:m[4]==='{}'?null:Number(m[4])}));
let failures=rows.length===58?0:1,matched=0,rejected=0,accepted=0;
const map=new Map(rows.map(r=>[r.source,r]));
for(const name of ['native','boundary']){
 const lines=fs.readFileSync(path.join(root,name+'-first.out'),'utf8').split(/\r?\n/).filter(Boolean);
 for(const line of lines.filter(x=>x.startsWith('ListBox|')||x.startsWith('ComboBox|'))){
  const fields=line.split('|'),r=map.get(fields[2]);if(!r)continue;
  ++matched;const error=Number(line.match(/\|error=(\d+)/)[1]);
  const [prefix,states]=line.split('|before='),[before,after]=states.split('|after=');
  let passed;
  if(r.legacy===null){++rejected;passed=error===11&&before===after;}
  else{
   ++accepted;
   const candidate=after.split(';').slice(1).map(x=>x.replace(/^row=/,'').split(',')).filter(x=>x[1]==='candidate');
   passed=error===0&&candidate.length===1&&Number(candidate[0][0])===r.legacy;
  }
  if(!passed)++failures;
  console.log(JSON.stringify({pair:name,class:fields[0],shape:fields[1],source:r.source,expectedID:r.legacy,error,passed}));
 }
}
if(matched!==672)++failures; // 440 native plus 29 matching boundary sources * 8.
console.log(JSON.stringify({literalRows:rows.length,matched,rejected,accepted,failures,note:'Explicit VFP9 native ID admission only; default/exact64/NaN remain named-parent derived. No duplicate-caption implementation claim.'}));
process.exitCode=failures?1:0;
