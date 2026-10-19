// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// AFTER capture: literal native observations only, not a rounding model/expected emitter.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const root=process.argv[2];a(root);const bytes=n=>{const p=path.join(root,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
const report=JSON.parse(bytes('audit.json')),hash='1d09046e3665a9d190684ff010b1b406b21add51a014f9e005b37cac75ff6469';
a.equal(report.recordsPerRun,12);a.equal(report.observations.length,12);
for(const k of ['documentedFieldsRepeatEqual','argumentAndSettingsExact','typedResultAndControls','exactRuntimeIdentity','rawByteEqual'])a.equal(report[k],true);
for(const n of ['first','repeat']){a.equal(report[n+'Sha256'],hash);a.equal(c.createHash('sha256').update(bytes(n+'-results.txt')).digest('hex'),hash);}
const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})*$/);return Buffer.from(h,'hex').toString('ascii');},pairs=[];
for(let index=1;index<=6;index++){
 const rows=['OFF','ON'].map(fixed=>{const r=report.observations.find(r=>r.index===index&&r.fixed===fixed);a(r);return r;});
 a.equal(rows[0].bits,rows[1].bits);a.deepEqual(rows[0].resultMeta,rows[1].resultMeta);a.equal(rows[0].resultBits,rows[1].resultBits);
 a.equal(rows[0].resultDisplayHex,rows[1].resultDisplayHex);a.equal(rows[0].resultStrHex,rows[1].resultStrHex);a.equal(rows[0].originalStrHex,rows[1].originalStrHex);
 pairs.push({index,bits:rows[0].bits,originalMeta:rows[0].originalMeta,resultMeta:rows[0].resultMeta,resultBits:rows[0].resultBits,
  rows:rows.map(r=>({fixed:r.fixed,originalDisplayHex:r.originalDisplayHex,originalDisplay:ascii(r.originalDisplayHex),originalStrHex:r.originalStrHex,
   originalStr:ascii(r.originalStrHex),resultDisplayHex:r.resultDisplayHex,resultDisplay:ascii(r.resultDisplayHex),resultStrHex:r.resultStrHex,resultStr:ascii(r.resultStrHex)}))});
}
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,sourceSha256:hash,contexts:12,pairs,
 originalStrEqualsResultStr:report.observations.filter(r=>r.originalStrHex===r.resultStrHex).length,
 onOriginalDisplayEqualsResultDisplay:report.observations.filter(r=>r.fixed==='ON'&&r.originalDisplayHex===r.resultDisplayHex).length,
 offOriginalDisplayDiffersFromResultDisplay:report.observations.filter(r=>r.fixed==='OFF'&&r.originalDisplayHex!==r.resultDisplayHex).length,
 distinctResultPayloads:[...new Set(report.observations.map(r=>r.resultBits))],
 limitation:'Only six explicit half-neighbor payloads/D0/INLINE N40/18/bothFIXED. Literal equality is finite evidence, not an algorithm/tolerance/cap/internal-call or Windows proof. Negative mirrors are explicit stimuli; no new strings or payloads predicted.'},null,2));
