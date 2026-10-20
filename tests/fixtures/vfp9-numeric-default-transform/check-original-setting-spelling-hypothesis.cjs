// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: unwired finite spelling hypothesis, NOT numeric conversion.
const fs=require('node:fs'),p=require('node:path'),z=require('node:zlib'),c=require('node:crypto'),a=require('node:assert/strict');
const sha=b=>c.createHash('sha256').update(b).digest('hex');
function candidate(raw,bits,fixed,setting,scale,count,width=40){
 a.match(bits,/^[0-9A-F]{16}$/);const b=BigInt('0x'+bits);a.notEqual((b>>52n)&2047n,2047n);a(['OFF','ON'].includes(fixed));a([12,13,14].includes(setting));a([13,14].includes(scale));a.equal(count,fixed==='ON'?setting:scale);a.equal(width,40);a(raw.length>0&&raw.length<=width);
 const m=raw.match(/^ *([+-]?)(?:(\d+)(?:\.(\d*))?|\.(\d+))(?:[eE]([+-]?\d+))?$/);a(m);const body=raw.trimStart(),digits=(m[2]||'0')+(m[3]===undefined?(m[4]||''):m[3]),zero=/^0+$/.test(digits),negative=(b>>63n)===1n;
 if(zero)return (negative?'-':'')+(fixed==='OFF'?'0':body.replace(/^[+-]/,''));
 a.equal(m[1]==='-',negative,'nonzero observed STR sign must match source payload');return body;
}
let hypothesisSelfChecks=0;
const pos='3FF0000000000000',neg='BFF0000000000000',pz='0000000000000000',nz='8000000000000000';
for(const [raw,bits,fixed,setting,scale,count,want]of [
 [' 0.000',pos,'OFF',12,13,13,'0'],[' 0.000',neg,'OFF',14,13,13,'-0'],[' -0.000',pz,'OFF',13,14,14,'0'],[' 0',nz,'OFF',12,14,14,'-0'],
 [' 0.000',pos,'ON',12,13,12,'0.000'],[' 0.000',neg,'ON',14,13,14,'-0.000'],[' -0.00',pz,'ON',13,14,13,'0.00'],[' 0.00',nz,'ON',13,13,13,'-0.00'],
 [' 1.20',pos,'OFF',12,13,13,'1.20'],[' -1.20',neg,'ON',14,13,14,'-1.20'],[' 1e-4',pos,'OFF',14,14,14,'1e-4'],[' -1E+4',neg,'ON',12,13,12,'-1E+4'],[' 0e-4',neg,'ON',13,14,13,'-0e-4'],[' +0.000',pos,'ON',13,13,13,'0.000']
 ]){a.equal(candidate(raw,bits,fixed,setting,scale,count),want);++hypothesisSelfChecks;}
for(const args of [
 ['',pos,'ON',12,13,12],['x',pos,'ON',12,13,12],['0.0 ',pos,'ON',12,13,12],['NaN',pos,'ON',12,13,12],['Infinity',pos,'ON',12,13,12],['1'.repeat(41),pos,'ON',12,13,12],
 ['0','bad','ON',12,13,12],['0','7FF0000000000000','ON',12,13,12],['0',pos,'OTHER',12,13,12],['0',pos,'ON',11,13,11],['0',pos,'ON',12,15,12],['0',pos,'ON',12,13,13],['0',pos,'OFF',12,13,12],['0',pos,'OFF',12,13,13,32],['-1',pos,'ON',12,13,12],['1',neg,'ON',12,13,12]
 ]){a.throws(()=>candidate(...args));++hypothesisSelfChecks;}
const dir=p.join(__dirname,'original-setting-spelling-captures1'),manifest=JSON.parse(fs.readFileSync(p.join(dir,'manifest.json'))),bytes=z.gunzipSync(fs.readFileSync(p.join(dir,'analysis.json.gz'))),entry=manifest.files.find(r=>r.name==='analysis.json.gz');a(entry);a.equal(sha(bytes),entry.sha256);
const report=JSON.parse(bytes);a.equal(report.nativeInvoked,false);a.equal(report.metricSelfChecks,14);a.equal(report.rejectionSelfChecks,6);a.equal(report.contextsChecked,48);a.equal(report.observations.length,48);
const seen=new Set(),observations=[];
for(const r of report.observations){const l=r.transform,s=r.str,key=[l.parentIndex,l.fixed,l.setting,l.scale].join(':');a(!seen.has(key));seen.add(key);a.equal(l.parentIndex,s.parentIndex);a.equal(l.fixed,s.fixed);a.equal(l.scale,s.scale);a.equal(l.bits,s.bits);a.equal(s.setting,s.publicCount);a.equal(r.rawInputNegative,(BigInt('0x'+l.bits)>>63n)===1n);const predicted=candidate(r.right.raw,l.bits,l.fixed,l.setting,l.scale,s.publicCount),predictedHex=Buffer.from(predicted,'ascii').toString('hex').toUpperCase(),expectedHex=Buffer.from(r.left.raw,'ascii').toString('hex').toUpperCase();observations.push({transform:l,str:s,predicted,predictedHex,observed:r.left.raw,expectedHex,exact:predictedHex===expectedHex,sourceCategory:r.category});}
for(const parent of [159,160,165,166])for(const fixed of ['OFF','ON'])for(const setting of [12,13,14])for(const scale of [13,14])a(seen.has([parent,fixed,setting,scale].join(':')));
const mismatches=observations.filter(r=>!r.exact);
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,productionWired:false,ordinaryShellNotReaEvidence:true,parentReportSha256:sha(bytes),sources:report.sources,hypothesisSelfChecks,contextsChecked:48,exactMatches:48-mismatches.length,exactMismatches:mismatches.length,mismatches,observations,limitation:'Finite unwired spelling hypothesis over observed public STR inputs, NOT a numeric conversion algorithm. No newly fabricated native expectations, general rounding/internal-call/default formatter policy, tolerance/cap, production hook or compatibility acceptance. Every exact mismatch retained; unchanged ORIGINAL/SAME/hosted/review still required.'},null,2));
