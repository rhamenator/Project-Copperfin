// Independently compare frozen Numeric literals to installed VFP9 observations.
// No checked helper, Copperfin execution or helper-produced acceptance oracle.
const fs=require('fs');
const source=fs.readFileSync('tests/test_prg_engine_additem_column_numeric.cpp','utf8');
const block=source.split(' numbers{')[1].split('\n};')[0];
const table=new Map([...block.matchAll(/\{"([^"]+)",(.+?),(\{\}|\d+),(\{\}|\d+)(,true)?\}/g)]
 .map(x=>[x[1],{modern:x[3]==='{}'?null:Number(x[3]),legacy:x[4]==='{}'?null:Number(x[4]),originalSafe:!!x[5]}]));
let failures=0,matched=0,unmatched=0;const used=new Set();
if(table.size!==98)++failures;
for(const name of ['native-repeat.out','boundary-first.out','configuration-first.out','projection-first.out']){
 const lines=fs.readFileSync('tests/fixtures/vfp9-additem-column-numeric/'+name,'utf8').trim().split(/\r?\n/);
 for(const line of lines.slice(1,-1)){
  const a=line.split('|'),r=table.get(a[3]);if(!r){++unmatched;continue;}
  const get=k=>a.find(f=>f.startsWith(k+'='))?.slice(k.length+1);
  const error=Number(get('error')),columns=Number(get('columns'));
  const observed=[...get('after').matchAll(/,cell(\d+)=candidate(?:,|;|$)/g)].map(x=>Number(x[1]));
  const wanted=r.legacy!==null&&r.legacy>=1&&r.legacy<=Math.max(1,columns)?[r.legacy]:[];
  const expectedError=r.legacy===null?11:0;
  const ok=a[4]==='invoke'&&error===expectedError&&JSON.stringify(observed)===JSON.stringify(wanted)&&
   (error!==11||get('before')===get('after'));
  if(!ok)++failures;++matched;used.add(a[3]);
  console.log(JSON.stringify({capture:name,source:a[3],context:a.slice(0,3),error,expectedError,
   observedVisibleColumns:observed,literalVisibleColumns:wanted,consistent:ok}));
 }
}
console.log(JSON.stringify({numericLiterals:table.size,originalSafeNumeric:[...table.values()].filter(x=>x.originalSafe).length,
 matchedRows:matched,unmatchedNativeRows:unmatched,matchedLiterals:used.size,
 numericLiteralsNotDirectlyCaptured:[...table.keys()].filter(k=>!used.has(k)),failures,
 note:'Native legacy numeric observations only. Default bound/exact64/NaN/binary extensions are named-parent derived; successful hidden storage/IDs/focus/return and original hostile-runtime qualification are excluded.'}));
process.exitCode=failures?1:0;
