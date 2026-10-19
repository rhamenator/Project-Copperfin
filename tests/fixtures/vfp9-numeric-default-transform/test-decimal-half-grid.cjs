// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER-capture hypothesis rejection ONLY.
// No native expected strings, formatter, fitted tolerance or accepted precision cap.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),cp=require('node:child_process');
const a=require('node:assert/strict'),c=require('node:crypto'),sha=b=>c.createHash('sha256').update(b).digest('hex');
const archived=(dir,name)=>z.gunzipSync(fs.readFileSync(path.join(__dirname,dir,name+'.gz')));
function bound(dir,name){const m=JSON.parse(fs.readFileSync(path.join(__dirname,dir,'manifest.json'))),r=m.files.find(r=>r.name===name);a(r);const b=archived(dir,name);a.equal(b.length,r.bytes);a.equal(sha(b),r.sha256);return b;}
// Reuse the already qualified raw/control validator, not another native capture.
const validationSource=bound('round-decomposition-analysis1','source-decompose-round-observations.cjs');
a.deepEqual(validationSource,fs.readFileSync(path.join(__dirname,'decompose-round-observations.cjs')));
const validation=cp.spawnSync(process.execPath,[path.join(__dirname,'decompose-round-observations.cjs')],{maxBuffer:3*1024*1024});a.equal(validation.status,0);a.equal(validation.stderr.length,0);
a.deepEqual(validation.stdout,bound('round-decomposition-analysis1','decomposition.json'));
const controls=JSON.parse(validation.stdout);a.equal(controls.analysisOnly,true);a.equal(controls.nativeInvoked,false);a.equal(controls.sharedContextsChecked,528);a.equal(controls.onContexts,264);a.equal(controls.metricSelfChecks,8);
const classified=JSON.parse(bound('graph-rejection-classification1','pass2-classification.json'));
a.equal(classified.analysisOnly,true);a.equal(classified.nativeInvoked,false);a.equal(classified.sharedContextsChecked,528);a.equal(classified.rejectedMagnitudes,18);a.deepEqual(classified.sourceHashes,{graph:controls.sourceHashes.arithmetic,str:controls.sourceHashes.str});
const graphRows=new Map(classified.comparisons.map(r=>[r.index,r]));a.equal(graphRows.size,264);
function binary(bits){a.match(bits,/^[0-9A-F]{16}$/);const b=BigInt('0x'+bits),e=(b>>52n)&2047n;a.notEqual(e,2047n);let n=(b&((1n<<52n)-1n))+(e?1n<<52n:0n);const p=e?e-1075n:-1074n;return {n:p>=0n?n<<p:n,d:p>=0n?1n:1n<<-p,negative:!!(b>>63n)};}
function decimal(text){const m=text.trim().match(/^([+-]?)(?:(\d+)(?:\.(\d*))?|\.(\d+))(?:[eE]([+-]?\d+))?$/);a(m);const f=m[3]===undefined?(m[4]||''):m[3];let n=BigInt((m[2]||'0')+f),e=BigInt(m[5]||'0')-BigInt(f.length);return {n:e>=0n?n*10n**e:n,d:e>=0n?1n:10n**-e,negative:m[1]==='-'};}
const cmp=(x,y)=>x.n*y.d<y.n*x.d?'less':x.n*y.d>y.n*x.d?'greater':'equal';
const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})*$/);const b=Buffer.from(h,'hex');a(b.every(n=>n>0&&n<128));return b.toString('ascii');};
function quantize(x,D,q,bias){a(Number.isInteger(D)&&D>=0&&D<=18);a(Number.isInteger(q)&&q>=0&&q<=18);a(x.n>=0n&&x.d>0n);
 let n=x.n,d=x.d;if(bias){const t=2000n*10n**BigInt(D);n=n*t+d;d*=t;}
 n*=10n**BigInt(q);const units=(2n*n+d)/(2n*d);return {units,n:units,d:10n**BigInt(q)};
}
let metricSelfChecks=0;
for(const[bits,expected]of [['3FDFFFFFFFFFFFFF',0n],['3FE0000000000000',1n],['3FE0000000000001',1n]]){a.equal(quantize(binary(bits),0,0,false).units,expected);++metricSelfChecks;}
a.equal(cmp(binary('0000000000000001'),{n:1n,d:1n<<1074n}),'equal');++metricSelfChecks;
a.equal(binary('8000000000000000').negative,true);++metricSelfChecks;a.equal(binary('8000000000000000').n,0n);++metricSelfChecks;
a.equal(quantize({n:999n,d:2000n},0,0,true).units,1n);++metricSelfChecks;a.equal(quantize({n:998n,d:2000n},0,0,true).units,0n);++metricSelfChecks;
a.equal(quantize({n:1n,d:2n*10n**18n},18,18,false).units,1n);++metricSelfChecks;
a.equal(quantize({n:1n,d:2n*10n**18n},18,15,false).units,0n);++metricSelfChecks;
a.equal(cmp(decimal('0.000000000000001'),decimal('1e-15')),'equal');++metricSelfChecks;a.equal(decimal('-0.000').negative,true);++metricSelfChecks;
a.throws(()=>binary('7FF0000000000000'));++metricSelfChecks;a.throws(()=>binary('7FF8000000000000'));++metricSelfChecks;
const specs=[{name:'unbiased_at_D',bias:false,cap:false},{name:'unbiased_cap15',bias:false,cap:true},{name:'decimal_prebias_at_D',bias:true,cap:false},{name:'decimal_prebias_cap15',bias:true,cap:true}];
const models=specs.map(spec=>{
 const rows=[];for(const r of controls.rows){a(Number.isInteger(r.setting)&&r.setting>=0&&r.setting<=18);const g=graphRows.get(r.index);a(g);a.equal(g.bits,r.bits);a.equal(g.setting,r.setting);a.equal(g.displayHex,r.originalVsRoundTransform.leftHex);
  const q=spec.cap&&r.setting>15?15:r.setting,candidate=quantize(binary(r.bits),r.setting,q,spec.bias);
  const observations={originalTransform:r.originalVsRoundTransform.leftHex,originalStr:r.originalStrVsRoundStr.leftHex,roundTransform:r.originalVsRoundTransform.rightHex,roundStr:r.originalStrVsRoundStr.rightHex};
  const comparisons=Object.fromEntries(Object.entries(observations).map(([name,hex])=>{const value=decimal(ascii(hex));return [name,{hex,magnitudeVsCandidate:cmp(value,candidate),zero:value.n===0n,negative:value.negative}];}));
  rows.push({index:r.index,setting:r.setting,bits:r.bits,publicRoundBits:r.inlineBits,quantumCount:q,candidateMagnitudeUnits:String(candidate.units),priorGraphRejected:!g.equal,comparisons});
 }a.equal(rows.length,264);
 const summary=Object.fromEntries(['originalTransform','originalStr','roundTransform','roundStr'].map(name=>{const failed=rows.filter(r=>r.comparisons[name].magnitudeVsCandidate!=='equal');return [name,{comparisons:264,magnitudeDifferent:failed.length,failedIndices:failed.map(r=>r.index),amongPriorGraphRejections:failed.filter(r=>r.priorGraphRejected).length}];}));
 return {...spec,definition:spec.bias?'floor((abs(binaryInput)+1/(2000*10^D))*10^q+1/2)/10^q, all operations exact rational':'floor(abs(binaryInput)*10^q+1/2)/10^q, all operations exact rational',summary,rows};
});
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,metricSelfChecks,reusedValidatorMetricChecks:8,sharedContextsChecked:528,onMagnitudesCompared:264,sourceHashes:controls.sourceHashes,priorGraphRejections:18,models,
 limitation:'Four explicitly proposed mathematical candidates only, on retained ORIGINAL N40/18 FIXED ON contexts. Decimal prebias and cap15 originate in rejected diagnostic stimuli, NOT recovered requirements or fitted parameters. Signed-zero/padding/literal bytes are observations, not predicted. Public ROUND payloads are retained but NOT compared to decimal text as a defect metric. No accepted formatter, expected emitter, internal-call/precision/Windows proof or production change.'},null,2));
