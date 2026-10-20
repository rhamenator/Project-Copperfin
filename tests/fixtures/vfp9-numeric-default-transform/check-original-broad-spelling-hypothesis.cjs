// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: unwired finite spelling hypothesis, NOT numeric conversion.
const fs=require('node:fs'),p=require('node:path'),z=require('node:zlib'),c=require('node:crypto'),a=require('node:assert/strict');
const sha=b=>c.createHash('sha256').update(b).digest('hex');
function candidate(raw,bits,fixed,setting,scale,count,width=40){
 a.match(bits,/^[0-9A-F]{16}$/);const b=BigInt('0x'+bits);a.notEqual((b>>52n)&2047n,2047n);a(['OFF','ON'].includes(fixed));a(Number.isInteger(setting)&&setting>=0&&setting<=18);a(Number.isInteger(scale)&&scale>=0&&scale<=18);a.equal(count,fixed==='ON'?setting:scale);a.equal(width,40);a(raw.length>0&&raw.length<=width);
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
 ['0','bad','ON',12,13,12],['0','7FF0000000000000','ON',12,13,12],['0',pos,'OTHER',12,13,12],['0',pos,'ON',19,13,19],['0',pos,'ON',12,19,12],['0',pos,'ON',12,13,13],['0',pos,'OFF',12,13,12],['0',pos,'OFF',12,13,13,32],['-1',pos,'ON',12,13,12],['1',neg,'ON',12,13,12]
 ]){a.throws(()=>candidate(...args));++hypothesisSelfChecks;}
const dir=p.join(__dirname,'str-fixed-captures1'),read=n=>z.gunzipSync(fs.readFileSync(p.join(dir,n+'.gz'))),report=JSON.parse(read('audit.json')),digest='89a420760a2332430158427778908a952d84966a6d4eee14f090c1a9a6332192';
for(const k of ['documentedFieldsRepeatEqual','originalAndNativeParentControls','inlineMetadataAndBRSExact','publicStrTypesAndWidths','rawByteEqual'])a.equal(report[k],true);for(const n of ['first','repeat']){a.equal(report[n+'Sha256'],digest);a.equal(sha(read(n+'-results.txt')),digest);}a.equal(report.observations.length,528);
const observations=[],seen=new Set();for(const r of report.observations){const key=r.index+':'+r.fixed;a(!seen.has(key));seen.add(key);a.equal(r.originalMeta.type,'4E');a.equal(r.originalMeta.width,'0028');a.equal(r.originalMeta.decimals,'00000012');a.equal(r.originalMeta.real,r.bits);a.equal(r.originalMeta.cw,'027F');const count=r.fixed==='ON'?r.setting:18,inputHex=r.fixed==='ON'?r.originalStrHex:r.str18Hex,raw=Buffer.from(inputHex,'hex').toString('ascii');a.equal(raw.length,40);const predicted=candidate(raw,r.bits,r.fixed,r.setting,18,count),predictedHex=Buffer.from(predicted,'ascii').toString('hex').toUpperCase();observations.push({index:r.index,fixed:r.fixed,setting:r.setting,scale:18,bits:r.bits,publicCount:count,inputHex,predictedHex,expectedHex:r.originalDisplayHex,exact:predictedHex===r.originalDisplayHex});}
for(let i=1;i<=264;++i)for(const fixed of ['OFF','ON'])a(seen.has(i+':'+fixed));const mismatches=observations.filter(r=>!r.exact);console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,productionWired:false,ordinaryShellNotReaEvidence:true,rawSha256:digest,hypothesisSelfChecks,contextsChecked:528,exactMatches:528-mismatches.length,exactMismatches:mismatches.length,byFixed:Object.fromEntries(['OFF','ON'].map(fixed=>[fixed,{contexts:264,mismatches:mismatches.filter(r=>r.fixed===fixed).length}])),mismatches,observations,limitation:'UNCHANGED rendering logic over retained original N40/18 public STR inputs, only validator range expanded to observed0..18. Every exact mismatch retained; not numeric conversion, internal call graph, general formatter/parity acceptance, fitted count cap/tolerance or production hook.'},null,2));
