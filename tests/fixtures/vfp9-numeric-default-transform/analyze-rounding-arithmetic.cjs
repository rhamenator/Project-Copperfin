// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Reject a public-language graph from actual observations, never emit expectations.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib'),a=require('node:assert/strict');
const report=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-arithmetic-captures/audit.json.gz'))));
a.equal(report.firstSha256,'2917ff9167e4a2752520614776580d08dda8fa4ba1520615154912bd937bd1c9');
a.equal(report.repeatSha256,report.firstSha256);a.equal(report.rawByteEqual,true);
a.equal(report.documentedFieldsRepeatEqual,true);a.equal(report.observations.length,528);
const number=hex=>Buffer.from(hex,'hex').readDoubleBE();
const hex=value=>{const b=Buffer.alloc(8);b.writeDoubleBE(value);return b.toString('hex').toUpperCase();};
const mismatches=[],hostStepDifferences=[],byInput=new Map();let compared=0;
for(const row of report.observations){
    if(row.fixed==='OFF'){a(!byInput.has(row.index));byInput.set(row.index,row);continue;}
    a.deepEqual(row.arithmetic,byInput.get(row.index).arithmetic);
    const d=row.setting,q=Math.min(d,15),actualText=Buffer.from(row.displayHex,'hex').toString('ascii');
    a.match(actualText,/^-?[0-9]+(?:\.[0-9]+)?$/);
    const [whole,fraction='']=actualText.replace(/^-/,'').split('.');
    // Compare exact decimal integer units, not a rounded Number(string).
    let actualUnits=BigInt(whole+fraction);const shift=q-fraction.length;
    if(shift>=0)actualUnits*=10n**BigInt(shift);
    else {const divisor=10n**BigInt(-shift);a.equal(actualUnits%divisor,0n);actualUnits/=divisor;}
    const nativeQuantized=number(row.arithmetic.quantized);a(Number.isSafeInteger(nativeQuantized));
    ++compared;
    if(actualUnits!==BigInt(nativeQuantized))mismatches.push({
        index:row.index,group:row.group,setting:d,bits:row.bits,offset:row.offset,
        observedHex:row.displayHex,observedStrHex:row.strHex,actualMagnitudeUnits:String(actualUnits),
        nativeGraphUnits:String(nativeQuantized),arithmetic:row.arithmetic});
    const powDown=10**(-d),bias=.0005*powDown,shifted=Math.abs(number(row.bits))+bias,
        powUp=10**q,scaled=shifted*powUp,raised=scaled+.5;
    const host={powDown,bias,shifted,effectiveCount:q,powUp,scaled,raised,quantized:Math.floor(raised)};
    for(const [field,value]of Object.entries(host)){
        const hostHex=hex(value),nativeHex=row.arithmetic[field];
        if(hostHex!==nativeHex)hostStepDifferences.push({index:row.index,setting:d,field,hostHex,nativeHex});
    }
}
a.equal(compared,264);a.equal(byInput.size,264);a.equal(mismatches.length,18);
const counts={};for(const r of hostStepDifferences)counts[r.field]=(counts[r.field]||0)+1;
a.deepEqual(counts,{powDown:36,bias:36});
console.log(JSON.stringify({requirement:report.requirement,nativeSha256:report.firstSha256,
    compared,arithmeticEqualAcrossFixedModes:true,publicScalarGraphRejected:true,mismatches,
    hostNativeStepDifferenceCounts:counts,hostStepDifferences,
    limitation:'The observed native public-language scalar graph disagrees with18/264 independently observed ON display magnitudes. Host/native power and bias bits differ36 times each; later recorded steps coincide for these inputs, so power differences alone do not explain the formatter discrepancy. This is rejection evidence, NOT formatter call attribution, a recovered precision/cap/epsilon/order, sign/padding/negative-zero/text equivalence, or an expected-output producer. No Copperfin implementation is imported.'},null,2));
