// Source-matched independent native observations versus authored literals.
// No helper call, production execution or computed conversion oracle.
const fs=require('fs');
const source=fs.readFileSync('tests/test_prg_engine_addlistitem_column_numeric.cpp','utf8');
const block=source.split(' numbers{')[1].split('\n};')[0];
const table=new Map([...block.matchAll(/\{"([^"]+)",(.+?),(\{\}|\d+),(\{\}|\d+)(,true)?\}/g)].map(x=>[x[1],{modern:x[3]==='{}'?null:+x[3],legacy:x[4]==='{}'?null:+x[4],originalSafe:!!x[5]}]));
const exactBlock=source.split(' exacts{')[1].split('\n};')[0];
const exactCount=[...exactBlock.matchAll(/\{"/g)].length,exactSafe=[...exactBlock.matchAll(/,true\}/g)].length;
let failures=0,matched=0,unmatched=0;const used=new Set();
if(table.size!==69||[...table.values()].filter(x=>x.originalSafe).length!==46||exactCount!==19||exactSafe!==12)++failures;
for(const [capture,kind]of [['native-first.out','visible'],['visibility-first.out','deferred'],['high-storage-first.out','high'],['storage-sweep-first.out','request'],['storage-boundary-first.out','request'],['storage-refinement-first.out','request'],['storage-neighbors-first.out','request'],['precision-first.out','dual'],['configuration-first.out','dual']]){
 const lines=fs.readFileSync('tests/fixtures/vfp9-addlistitem-column-numeric/'+capture,'utf8').trim().split(/\r?\n/);
 for(const l of lines.slice(1,-1)){
  const a=l.split('|'),get=k=>a.find(f=>f.startsWith(k+'='))?.slice(k.length+1);
  if(capture==='native-first.out'&&get('shape')!=='1')continue; // NOT missing-ID creation.
  const expr=kind==='visible'||kind==='dual'?a[4]:kind==='deferred'?a[3]:kind==='high'?a[2]:get('column');
  const r=table.get(expr);if(!r){++unmatched;continue;}
  const actualError=+get('error'),expectedError=r.legacy===null?11:0;
  const snapshot=kind==='deferred'||kind==='high'?get('expanded'):get('after');
  let fields,actual;
  if(kind==='visible'||kind==='deferred'){
   fields=Array.from({length:Math.max(1,+(kind==='deferred'?9:get('columns')))},(_,i)=>i+1);
   actual=[...snapshot.matchAll(/,cell(\d+)=candidate(?:,|;|$)/g)].map(m=>+m[1]);
  }else{
   const reads=[...snapshot.matchAll(/\[(?:column=(\d+),)?indexError=(\d+),index=([^\]]*?),itemError=(\d+),item=([^\]]*)\]/g)];
   fields=kind==='request'?[+expr]:[...new Set(reads.map(m=>+m[1]))];
   actual=reads.filter(m=>m[3]==='candidate').map(m=>kind==='request'?+expr:+m[1]);
   if(reads.some(m=>+m[2]||+m[4]||m[3]!==m[5]))++failures;
  }
  const wanted=r.legacy!==null&&r.legacy>=1&&fields.includes(r.legacy)?[r.legacy]:[];
  const ok=a.includes('invoke')&&actualError===expectedError&&JSON.stringify(actual)===JSON.stringify(wanted)&&
   (actualError!==11||get('before')===get('after'));
  if(!ok)++failures;++matched;used.add(expr);
  console.log(JSON.stringify({capture,source:expr,context:a.slice(0,3),actualError,expectedError,sampledColumns:fields,actual,literalSampledColumns:wanted,consistent:ok}));
 }
}
console.log(JSON.stringify({numericLiterals:table.size,originalSafeNumeric:46,exactLiterals:exactCount,originalSafeExact:exactSafe,plannedDirect:210,plannedPublicPerShard:660,plannedFixedOnlyHostilePerShard:182,matchedNativeRows:matched,unmatchedNativeRows:unmatched,matchedNumericLiterals:used.size,numericLiteralsNotSourceMatched:[...table.keys()].filter(k=>!used.has(k)),failures,note:'Source-matched native legacy effects in declared sampled fields only, not source/binary identity or full hidden storage/metadata/return/Copperfin parity. Default/exact64/NaN/binary extensions have named owner parents. Missing-ID creation excluded. Not an original hostile-runtime qualification or executed counter report.'}));
process.exitCode=failures?1:0;
