// Independent literal/source audit against installed-VFP raw; no production/helper execution.
const fs=require('fs'),path=require('path'),crypto=require('crypto');
const root=process.argv[2],test=fs.readFileSync(path.join(root,'tests/test_prg_engine_additem_numeric.cpp'),'utf8');
const block=test.slice(test.indexOf('const std::vector<NumberCase> numbers{'),test.indexOf('struct ExactCase'));
const rows=[...block.matchAll(/\{"([^"]+)",[^\n]*?,(\{\}|\d+),(\{\}|\d+)\}/g)];
if(rows.length!==82)throw Error('Expected 82 independent literals');
const literals=new Map(rows.map(r=>[r[1],{modern:r[2]==='{}'?null:+r[2],legacy:r[3]==='{}'?null:+r[3]}]));
let checked=0,positions=0,failures=0,excluded=0;
for(const name of ['native','configuration','boundary','sorted','count','count-alias']){
 const raw=fs.readFileSync(path.join(root,'tests/fixtures/vfp9-additem-numeric',name+'-first.out'),'utf8');
 for(const line of raw.trim().split(/\r?\n/).slice(1,-1)){
  const fields=line.split('|'),source=fields[2],get=k=>fields.find(f=>f.startsWith(k+'='))?.slice(k.length+1);
  if(source==='0.9999999999999999'){++excluded;continue;}
  const entry=literals.get(source);if(!entry)throw Error('Unmapped raw source '+source);
  const count=+get('before').match(/^count=(\d+)/)[1];
  const wanted=entry.legacy!==null&&entry.legacy<=count+1?entry.legacy:null;
  let ok=+get('error')===(wanted===null?11:0);
  if(wanted!==null&&name!=='sorted'){
   const cells=get('after').split(';row=').slice(1);
   const candidate=cells.findIndex(c=>c.split(',')[1]==='candidate')+1;
   ok&&=candidate===wanted;++positions;
  }
  if(!ok)++failures;++checked;
  console.log(JSON.stringify({pair:name,cls:fields[0],shape:fields[1],source,count,wanted,error:+get('error'),ok}));
 }
}
if(!test.includes("TRY\\no.AddItem('candidate',")||test.includes("TRY\\no.AddListItem('candidate',"))throw Error('Wrong consumer in test scaffold');
console.log(JSON.stringify({literals:rows.length,checked,positions,excludedDecimalParserRows:excluded,failures,
 attribution:'VFP9 expected admission/insertion only; modern/exact/host requirements derive from named owner parent. No shared-state/generated-ID oracle.'}));
process.exitCode=failures?1:0;
