// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER-capture spelling classification, no normalization.
const fs=require('node:fs'),p=require('node:path'),z=require('node:zlib'),c=require('node:crypto'),a=require('node:assert/strict');
const sha=b=>c.createHash('sha256').update(b).digest('hex'),dir=p.join(__dirname,'original-setting-str-captures1');
const manifest=JSON.parse(fs.readFileSync(p.join(dir,'manifest.json'))),bytes=z.gunzipSync(fs.readFileSync(p.join(dir,'analysis.json.gz'))),entry=manifest.files.find(r=>r.name==='analysis.json.gz');a(entry);a.equal(sha(bytes),entry.sha256);
const report=JSON.parse(bytes);a.equal(report.nativeInvoked,false);a.equal(report.contextsChecked,48);a.equal(report.metricSelfChecks,8);a.equal(report.joinSelfChecks,6);a.equal(report.selectorSelfChecks,2);a.equal(report.comparisons.length,144);
function profile(raw){
 a.match(raw,/^ *[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?$/);
 const leadingSpaces=raw.length-raw.trimStart().length,body=raw.slice(leadingSpaces),m=body.match(/^([+-]?)(?:(\d+)(?:\.(\d*))?|\.(\d+))(?:[eE]([+-]?\d+))?$/);a(m);
 const fraction=m[3]===undefined?(m[4]||''):m[3];let n=BigInt((m[2]||'0')+fraction),e=BigInt(m[5]||0)-BigInt(fraction.length);if(m[1]==='-')n=-n;const zero=n===0n;if(zero)e=0n;else while(n%10n===0n){n/=10n;++e;}
 return {raw,body,leadingSpaces,sign:m[1],negative:m[1]==='-',zero,coefficient:String(n),exponent:String(e),fractionDigits:fraction.length,trailingFractionZeros:(fraction.match(/0*$/)||[''])[0].length,exponentSpelling:m[5]===undefined?null:m[5]};
}
const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})+$/);const b=Buffer.from(h,'hex');a(b.every(n=>n>0&&n<128));return b.toString('ascii');};
function classify(left,right){const l=profile(left),r=profile(right),valueEqual=l.coefficient===r.coefficient&&l.exponent===r.exponent,trimmedEqual=l.body===r.body;return {left:l,right:r,literalEqual:left===right,trimmedLiteralEqual:trimmedEqual,valueEqual,zeroSignDifferent:l.zero&&r.zero&&l.negative!==r.negative,category:left===right?'literal-equal':trimmedEqual?'padding-only':!valueEqual?'different-value':l.zero&&r.zero?'zero-spelling':'equal-nonzero-spelling'};}
let metricSelfChecks=0;for(const [l,r,category,zeroSign]of [['1','1','literal-equal',false],['1','  1','padding-only',false],['-0.00','0.00','zero-spelling',true],['0','0.000','zero-spelling',false],['1.20','1.2','equal-nonzero-spelling',false],['1','2','different-value',false],['1e-4','0.0001','equal-nonzero-spelling',false],['-1','1','different-value',false]]){const t=classify(l,r);a.equal(t.category,category);a.equal(t.zeroSignDifferent,zeroSign);++metricSelfChecks;}
for(const bad of ['','  ','1x','0.0 ','NaN','Infinity']){a.throws(()=>profile(bad));++metricSelfChecks;}
const key=r=>[r.parentIndex,r.fixed,r.setting,r.scale].join(':');
function select(comparisons){
 const selected=new Map(),allKeys=new Set();for(const r of comparisons){const l=r.transform,s=r.str;allKeys.add(key(l));a.equal(l.parentIndex,s.parentIndex);a.equal(l.fixed,s.fixed);a.equal(l.scale,s.scale);a.equal(l.bits,s.bits);a.equal(s.setting,s.publicCount);a.equal(r.sameSettingCount,l.setting===s.publicCount);a.equal(r.metadataCount,l.scale===s.publicCount);if(s.publicCount!==(l.fixed==='ON'?l.setting:l.scale))continue;a(!selected.has(key(l)),'duplicate selected context');selected.set(key(l),r);}
 a.equal(allKeys.size,48);a.equal(selected.size,48);for(const parent of [159,160,165,166])for(const fixed of ['OFF','ON'])for(const setting of [12,13,14])for(const scale of [13,14])a(selected.has([parent,fixed,setting,scale].join(':')),'missing selected context');return selected;
}
const selected=select(report.comparisons);let rejectionSelfChecks=0;
const duplicate=[...report.comparisons,...selected.values()];a.throws(()=>select(duplicate));++rejectionSelfChecks;
const missing=report.comparisons.filter(r=>key(r.transform)!==key(report.comparisons[0].transform));a.throws(()=>select(missing));++rejectionSelfChecks;
for(const mutate of [r=>{r.str.bits='0000000000000000';},r=>{r.str.publicCount=99;},r=>{r.str.scale=99;},r=>{r.sameSettingCount=!r.sameSettingCount;}]){const changed=structuredClone(report.comparisons);mutate(changed[0]);a.throws(()=>select(changed));++rejectionSelfChecks;}
// Recompute every parent metric, not merely its selected aggregate.
for(const r of report.comparisons){const t=classify(ascii(r.comparison.leftHex),ascii(r.comparison.rightHex));for(const k of ['literalEqual','trimmedLiteralEqual','valueEqual','zeroSignDifferent'])a.equal(t[k],r.comparison[k]);}
const observations=[...selected.values()].map(r=>({transform:r.transform,str:r.str,rawInputNegative:(BigInt('0x'+r.transform.bits)>>63n)===1n,...classify(ascii(r.comparison.leftHex),ascii(r.comparison.rightHex))}));
const summary=rs=>({contexts:rs.length,literalDifferent:rs.filter(r=>!r.literalEqual).length,trimmedDifferent:rs.filter(r=>!r.trimmedLiteralEqual).length,valueDifferent:rs.filter(r=>!r.valueEqual).length,zeroSignDifferent:rs.filter(r=>r.zeroSignDifferent).length,categories:Object.fromEntries(['literal-equal','padding-only','different-value','zero-spelling','equal-nonzero-spelling'].map(k=>[k,rs.filter(r=>r.category===k).length]))});
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,parentReportSha256:sha(bytes),sources:report.sources,metricSelfChecks,rejectionSelfChecks,contextsChecked:48,summary:{all:summary(observations),byFixed:Object.fromEntries(['OFF','ON'].map(f=>[f,summary(observations.filter(r=>r.transform.fixed===f))]))},observations,limitation:'Classification of actual48 selected TF/public STR observations only. Raw padding, exact decimal value, zero sign and fraction/exponent spelling preserved; no normalization, replacement target strings, internal call graph, general formatter rule, fitted tolerance/cap or parity acceptance.'},null,2));
