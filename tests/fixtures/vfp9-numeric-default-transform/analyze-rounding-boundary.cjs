// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Summarize actual native text intervals, never synthesize an expected string.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict');
const report=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-captures/audit.json.gz'))));
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-cases.json')));
a.equal(report.firstSha256,'3c6601c1d769cd9bf662adf3068da1486a4f48e9993fb2b13524dfabcf627980');
a.equal(report.repeatSha256,report.firstSha256);a.equal(report.rawByteEqual,true);
const groups=[];
for(const center of inv.centers)for(const fixed of ['OFF','ON']){
    const rows=report.observations.filter(r=>r.center===center.id&&r.fixed===fixed);
    a.equal(rows.length,center.high-center.low+1);const segments=[];
    for(const row of rows){
        const text=Buffer.from(row.displayHex,'hex').toString('ascii');
        if(segments.length&&segments.at(-1).text===text){
            segments.at(-1).lastOffset=row.offset;segments.at(-1).lastBits=row.bits;
        }else segments.push({firstOffset:row.offset,lastOffset:row.offset,
            firstBits:row.bits,lastBits:row.bits,text});
    }
    groups.push({expression:center.expression,criticalSetting:center.setting,fixed,
        sampledLow:center.low,sampledHigh:center.high,distinctTextIntervals:segments.length,segments});
}
console.log(JSON.stringify({requirement:inv.requirement,nativeSha256:report.firstSha256,
    limitation:'Observed equal-text intervals on the finite frozen grid only. One interval means no transition was bracketed, NOT a general tolerance or algorithm. No native syntax/producer, negative-value or unmeasured magnitude parity claim.',groups},null,2));
