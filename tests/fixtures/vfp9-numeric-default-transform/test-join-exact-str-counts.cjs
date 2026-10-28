// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{join}=require('./join-exact-str-counts.cjs');
function grid(negative=false){return [15,16,17,18].map((count,i)=>{
 const sign=negative?-1n:1n,observed=sign*(10n**BigInt(i)+BigInt(i));
 const nearest=sign*10n**BigInt(i),delta=String(observed-nearest);
 return {index:1,fixed:'OFF',bits:negative?'BFF0000000000000':'3FF0000000000000',setting:13,count,
  observedScaled:String(observed),candidate:{nearestEven:String(nearest),nearestAway:String(nearest)},
  residuals:{nearestEven:{signedUnits:delta},nearestAway:{signedUnits:delta}}};
});}
let checks=0;const check=f=>{f();++checks;};
for(const negative of [false,true])check(()=>{
 const rows=grid(negative),copy=structuredClone(rows),pairs=join(rows),sign=negative?-1:1;
 a.equal(pairs.length,3);a.deepEqual(rows,copy);
 a.deepEqual(pairs.map(p=>p.observedStep),[String(sign),String(-8*sign),String(-17*sign)]);
 for(const pair of pairs){a.equal(pair.transitions.nearestEven.nearestStep,'0');a.equal(pair.transitions.nearestEven.residualStep,pair.observedStep);}
});
check(()=>{const rows=grid();for(const r of rows){r.observedScaled='0';for(const m of ['nearestEven','nearestAway']){r.candidate[m]='0';r.residuals[m].signedUnits='0';}}a(join(rows).every(p=>p.observedStep==='0'));});
for(const mutate of [r=>r.pop(),r=>r.push(structuredClone(r[0])),r=>r[1].bits='4000000000000000',
 r=>r[1].setting=14,r=>r[1].count=19,r=>r[1].fixed='ON',r=>r[1].residuals.nearestEven.signedUnits='99'])
 check(()=>{const rows=grid();mutate(rows);a.throws(()=>join(rows));});
check(()=>{const rows=grid(),second=grid(true).map(r=>({...r,index:2}));a.equal(join([...rows,...second]).length,6);});
check(()=>a.deepEqual(join([]),[]));
console.log(JSON.stringify({checks,passed:true,nativeInvoked:false}));
