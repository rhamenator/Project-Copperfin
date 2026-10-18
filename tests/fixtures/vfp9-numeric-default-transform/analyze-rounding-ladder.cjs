// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Observed sampled intervals, not an output model or monotonicity proof.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict');
const report=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-ladder-captures/audit.json.gz'))));
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-ladder-cases.json')));
a.equal(report.firstSha256,'e7cbfd0a45549b69bbfcebc2d924dc20bb7f895e1405ad95136f4359b4eb134f');
a.equal(report.repeatSha256,report.firstSha256);a.equal(report.rawByteEqual,true);
const groups=[];
for(const group of inv.groups)for(const fixed of ['OFF','ON']){
    const rows=report.observations.filter(r=>r.center===group.center&&r.negative===group.negative&&r.fixed===fixed);
    a.deepEqual(rows.map(r=>r.offset),inv.offsets);const segments=[],brackets=[];
    for(const row of rows){
        const text=Buffer.from(row.displayHex,'hex').toString('ascii');
        if(segments.length&&segments.at(-1).text===text){
            segments.at(-1).lastOffset=row.offset;segments.at(-1).lastBits=row.bits;
        }else{
            if(segments.length){const prev=segments.at(-1);brackets.push({
                lowOffset:prev.lastOffset,highOffset:row.offset,lowBits:prev.lastBits,
                highBits:row.bits,lowText:prev.text,highText:text,adjacent:row.offset-prev.lastOffset===1});}
            segments.push({firstOffset:row.offset,lastOffset:row.offset,firstBits:row.bits,lastBits:row.bits,text});
        }
    }
    groups.push({center:group.center,negative:group.negative,fixed,setting:group.setting,
        sampledTextSegments:segments.length,segments,brackets});
}
console.log(JSON.stringify({requirement:inv.requirement,nativeSha256:report.firstSha256,
    limitation:'Equal strings only at sampled ladder points, not continuous intervals. Different adjacent sampled outputs bracket at least one change, not uniqueness or monotonicity. No output synthesis, tolerance, general conversion or implementation parity claim.',groups},null,2));
