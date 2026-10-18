// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Reject hypotheses against qualified observations; never emit expected output.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict');
const r=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-edge-captures/audit.json.gz'))));
a.equal(r.firstSha256,'b6e43abea8d3290d46e231a72e0ec6807c25551b266cfa156d91f393f4baacc2');
a.equal(r.repeatSha256,r.firstSha256);a.equal(r.rawByteEqual,true);
const hypotheses={binary64Half:[],binary64Add5005:[],binary64PreRound3:[]};let compared=0;
for(const row of r.observations.filter(x=>x.fixed==='ON')){
    const magnitude=Math.abs(Buffer.from(row.bits,'hex').readDoubleBE()),factor=10**row.setting;
    const observed=Math.abs(Number(Buffer.from(row.displayHex,'hex').toString('ascii')));
    const proposed={binary64Half:Math.floor(magnitude*factor+.5)/factor,
        binary64Add5005:Math.floor(magnitude*factor+.5005)/factor,
        binary64PreRound3:Math.floor(Math.round(magnitude*factor*1000)/1000+.5)/factor};
    ++compared;
    for(const [hypothesis,value]of Object.entries(proposed))if(value!==observed)
        hypotheses[hypothesis].push({bits:row.bits,setting:row.setting,proposedMagnitude:value,
            observedMagnitude:observed,observedHex:row.displayHex});
}
for(const mismatches of Object.values(hypotheses))a(mismatches.length>0);
console.log(JSON.stringify({requirement:r.requirement,nativeSha256:r.firstSha256,compared,hypotheses,
    limitation:'All three host-binary64 magnitude hypotheses are REJECTED, not formatter candidates or oracle strings. This numeric comparison deliberately excludes sign, negative-zero text, padding and width. It says nothing about an x87 implementation or untested algorithms. No tolerated mismatch or inferred epsilon is accepted.'},null,2));
