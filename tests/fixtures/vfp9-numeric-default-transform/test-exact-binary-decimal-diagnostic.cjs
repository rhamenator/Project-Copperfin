// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{scaled,candidates}=require('./exact-binary-decimal-diagnostic.cjs');
let checks=0;const check=f=>{f();++checks;};
for(const [bits,count,trunc,even,away,tie] of [
 ['3FF0000000000000',0,'1','1','1',false],
 ['3FE0000000000000',0,'0','0','1',true],
 ['3FF8000000000000',0,'1','2','2',true],
 ['4004000000000000',0,'2','2','3',true],
 ['BFE0000000000000',0,'0','0','-1',true],
 ['BFF8000000000000',0,'-1','-2','-2',true],
 ['3FF4000000000000',1,'12','12','13',true],
 ['3FEFFFFFFFFFFFFF',0,'0','1','1',false],
 ['3FF0000000000001',16,'10000000000000002','10000000000000002','10000000000000002',false],
 ['3FB999999999999A',18,'100000000000000005','100000000000000006','100000000000000006',false]])
 check(()=>{const r=candidates(bits,count);a.deepEqual([r.truncated,r.nearestEven,r.nearestAway,r.tie],[trunc,even,away,tie]);});
for(const bits of ['0000000000000000','8000000000000000'])check(()=>{
 const r=candidates(bits,21);a.equal(r.exact,true);a.equal(r.nearestEven,'0');a.equal(r.negative,bits[0]==='8');});
check(()=>{const r=scaled('0000000000000001',0);a.equal(r.numerator,1n);a.equal(r.denominator,1n<<1074n);});
check(()=>{const r=scaled('0010000000000000',0);a.equal(r.numerator,1n<<52n);a.equal(r.denominator,1n<<1074n);});
check(()=>{const r=scaled('7FEFFFFFFFFFFFFF',0);a.equal(r.denominator,1n);a.equal(r.numerator,((1n<<53n)-1n)<<971n);});
// Independent platform-endian oracle for known payloads; never used for expected digits.
for(const [bits,value]of [['3FF0000000000000',1],['3FE0000000000000',0.5],['8000000000000000',-0]])
 check(()=>a(Object.is(Buffer.from(bits,'hex').readDoubleBE(),value)));
for(const bits of ['7FF0000000000000','FFF0000000000000','7FF8000000000001','bad','3ff0000000000000'])
 check(()=>a.throws(()=>scaled(bits,1)));
for(const count of [-1,22,0.5,NaN,'1'])check(()=>a.throws(()=>scaled('3FF0000000000000',count)));
console.log(JSON.stringify({checks,passed:true,nativeInvoked:false}));
