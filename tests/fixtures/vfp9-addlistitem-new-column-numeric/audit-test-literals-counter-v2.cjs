// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independently captured native observations vs authored literals; NO helper
// invocation, production execution or computed projection oracle.
const fs=require('fs'),assert=require('assert');
const source=fs.readFileSync('tests/test_prg_engine_addlistitem_new_column_numeric.cpp','utf8');
const block=source.split(' numbers{')[1].split('\n};')[0];
const table=new Map([...block.matchAll(/\{"([^"]+)",(.+?),(\{\}|\d+),(\{\}|\d+),(true|false)\}/g)].map(m=>[m[1],{modern:m[3]==='{}'?null:Number(m[3]),legacy:m[4]==='{}'?null:Number(m[4]),safe:m[5]==='true'}]));
assert.equal(table.size,65);assert.equal([...table.values()].filter(r=>r.safe).length,44);
const exact=source.split(' exacts{')[1].split('\n};')[0];assert.equal([...exact.matchAll(/\{"/g)].length,18);assert.equal([...exact.matchAll(/,true\}/g)].length,12);
let matched=0,unmatched=0;const used=new Set();
const base='tests/fixtures/vfp9-addlistitem-new-column-numeric/';
for(const [file,kind] of [['first-results.txt','case'],['configuration-first-results.txt','case'],['storage-first-results.txt','read'],['boundary-first-results.txt','read']]){
 for(const line of fs.readFileSync(base+file,'utf8').trim().split(/\r?\n/).slice(1,-1)){
  const row=line.split('|'),operand=row[kind==='case'?5:4],expected=table.get(operand);
  if(!expected){++unmatched;continue;}
  const error=Number(row[kind==='case'?11:6]);assert.equal(error,expected.legacy===null?11:0,file+' '+operand+' admission');
  if(kind==='read'&&error===0){
   const cells=[...row[12].matchAll(/\[(\d+);(\d+);([^;\]]*);(\d+);([^\]]*)\]/g)];assert.equal(cells.length,14);
   const fields=cells.map(m=>Number(m[1])),actual=cells.filter(m=>m[3]==='C:candidate').map(m=>Number(m[1]));
   for(const c of cells){assert.equal(c[2],'0');assert.equal(c[4],'0');assert.equal(c[3],c[5]);}
   const wanted=expected.legacy>0&&fields.includes(expected.legacy)?[expected.legacy]:[];
   assert.deepEqual(actual,wanted,file+' '+operand+' declared sampled candidate');
  }
  ++matched;used.add(operand);
 }
}
console.log(JSON.stringify({numeric:65,safeNumeric:44,exact:18,safeExact:12,additionalDirectPerMode:13,plannedDirect:192,plannedSafePublicPerShard:432,plannedFixedOnlyHostilePerShard:112,matchedNativeRows:matched,unmatchedNativeRows:unmatched,sourceMatchedLiterals:used.size,notSourceMatched:[...table.keys()].filter(k=>!used.has(k)),note:'Only source-matched native admission and declared dual-route sampled candidate effects. No source/binary identity, hidden-storage/global limit or Copperfin shared-creation parity. Default/exact64/NaN/binary parents are named separately. Planned counts, NOT executed acceptance.'}));
