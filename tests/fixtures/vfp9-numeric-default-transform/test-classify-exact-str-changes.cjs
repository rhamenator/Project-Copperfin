// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{classify}=require('./classify-exact-str-changes.cjs');
const grid=()=>[15,16,17].map(lowCount=>({index:1,fixed:'OFF',bits:'3FF0000000000000',setting:13,
 lowCount,highCount:lowCount+1,observedStep:lowCount===16?'1':'0',low:{count:lowCount},high:{count:lowCount+1}}));
let checks=0;const check=f=>{f();++checks;};
check(()=>{const rows=grid(),copy=structuredClone(rows),r=classify(rows);a.equal(r.changing.length,1);a.equal(r.unchanged.length,0);a.equal(r.contexts[0].changedSteps,1);a.deepEqual(rows,copy);});
check(()=>{const rows=grid().map(r=>({...r,observedStep:'0'}));a.equal(classify(rows).unchanged.length,1);});
check(()=>{const rows=grid().map(r=>({...r,observedStep:'-1'}));a.equal(classify(rows).changing[0].changedSteps,3);});
for(const mutate of [r=>r.pop(),r=>r.push(structuredClone(r[0])),r=>r[1].bits='4000000000000000',
 r=>r[1].setting=14,r=>r[1].low.extra=true,r=>r[1].highCount=19,r=>r[1].lowCount=14])
 check(()=>{const rows=grid();mutate(rows);a.throws(()=>classify(rows));});
check(()=>{const rows=grid();a.equal(classify([...rows,...grid().map(r=>({...r,index:2}))]).contexts.length,2);});
check(()=>a.deepEqual(classify([]),{contexts:[],changing:[],unchanged:[]}));
console.log(JSON.stringify({checks,passed:true,nativeInvoked:false}));
