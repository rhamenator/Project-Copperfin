// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// A rejected host-binary64 hypothesis, never an expected-output producer.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict');
const report=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-decimals3-captures/audit.json.gz'))));
a.equal(report.firstSha256,'6972cb22232ddd776ec25491bb461b051e88c297d5e98d1bf9eea5bd519e1a3f');
a.equal(report.repeatSha256,report.firstSha256);a.equal(report.rawByteEqual,true);
const mismatches=[];let compared=0;
for(const row of report.observations.filter(x=>x.fixed==='ON')){
    const value=Math.abs(Buffer.from(row.bits,'hex').readDoubleBE()),d=row.setting;
    const actual=Math.abs(Number(Buffer.from(row.displayHex,'hex').toString('ascii')));
    const q=Math.min(d,15),factor=10**q,bias=Number('5e-'+(d+4));
    const proposed=Math.floor((value+bias)*factor+.5)/factor;++compared;
    if(proposed!==actual)mismatches.push({bits:row.bits,setting:d,observedHex:row.displayHex,observedMagnitude:actual,proposedMagnitude:proposed});
}
a.equal(compared,2052);a(mismatches.length>0);
console.log(JSON.stringify({requirement:report.requirement,nativeSha256:report.firstSha256,compared,rejected:true,mismatches,
    limitation:'Post-hoc HOST-BINARY64 pre-add/capped-decimals magnitude hypothesis is REJECTED. It does not establish native arithmetic order, a precision cap, epsilon, exact text/sign/padding, or any acceptable formatter. No Copperfin/source/expected-string input.'},null,2));
