// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER-capture payload-route comparisons only.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const dir=path.join(__dirname,'shifted-consumer-captures1'),bytes=n=>z.gunzipSync(fs.readFileSync(path.join(dir,n+'.gz'))),sha=b=>c.createHash('sha256').update(b).digest('hex');
const manifest=JSON.parse(fs.readFileSync(path.join(dir,'manifest.json')));
a.equal(manifest.sourceCommit,'1b248f7a474e92fe24c13392fd6c95fe0b154828');a.equal(manifest.sourceStatusClean,true);a.equal(manifest.pairedQualified,true);
for(const n of ['audit.json','first-results.txt','repeat-results.txt']){const r=manifest.files.find(r=>r.name===n);a(r);a.equal(sha(bytes(n)),r.sha256);}
const report=JSON.parse(bytes('audit.json')),digest='764bc67a4feedce531033ac8b1fa52adcca9bd5844af00c4d18575f0140667e5';
for(const k of ['documentedFieldsRepeatEqual','parentControlsExact','targetMetadataAndBRSExact','exactRuntimeIdentity','rawByteEqual'])a.equal(report[k],true);
for(const run of ['first','repeat']){a.equal(report[run+'Sha256'],digest);a.equal(sha(bytes(run+'-results.txt')),digest);a.deepEqual(bytes(run+'-results.txt'),bytes(run+'.out'));a.equal(bytes(run+'.status').toString(),'0\n');a.equal(bytes(run+'.stderr').length,0);}
a.equal(bytes('first-structure.status').toString(),'0\n');a.equal(bytes('audit.status').toString(),'0\n');a.equal(bytes('audit.stderr').length,0);
const lines=bytes('first-results.txt').toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|16');a.equal(lines.length,16);a.equal(report.recordsPerRun,16);a.equal(report.observations.length,16);
const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})*$/);const b=Buffer.from(h,'hex');a(b.every(n=>n>0&&n<128));return b.toString('ascii');};
function decimal(text){const m=text.trim().match(/^([+-]?)(?:(\d+)(?:\.(\d*))?|\.(\d+))(?:[eE]([+-]?\d+))?$/);a(m);const fraction=m[3]===undefined?(m[4]||''):m[3];let n=BigInt((m[2]||'0')+fraction),e=BigInt(m[5]||'0')-BigInt(fraction.length);const negative=m[1]==='-';if(negative)n=-n;const zero=n===0n;if(zero)e=0n;else while(n%10n===0n){n/=10n;++e;}return {coefficient:String(n),exponent:String(e),zero,negative};}
const equal=(x,y)=>x.coefficient===y.coefficient&&x.exponent===y.exponent;
let metricSelfChecks=0;for(const[l,r,eq]of [['0','-0.000',true],['1.20','1.2e0',true],['.0001','1e-4',true],['-12.3400','-1234e-2',true],['0.000000001','0',false],['1','-1',false]]){a.equal(equal(decimal(l),decimal(r)),eq);++metricSelfChecks;}a.equal(decimal('-0.00').negative,true);++metricSelfChecks;a.equal(decimal(' 0.00 ').negative,false);++metricSelfChecks;
function compare(lh,rh){const l=ascii(lh),r=ascii(rh),x=decimal(l),y=decimal(r);return {leftHex:lh,rightHex:rh,literalEqual:lh===rh,trimmedLiteralEqual:l.trimStart()===r.trimStart(),valueEqual:equal(x,y),zeroSignDifferent:x.zero&&y.zero&&x.negative!==y.negative,leftValue:x,rightValue:y};}
const rows=report.observations;
const activeMeta=s=>{a.match(s,/^type=4E;width=0028;decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);const m=Object.fromEntries(s.split(';').map(x=>x.split('=')));delete m.integer;return m;};
for(const[i,r]of rows.entries()){const f=lines[i].split('|');a.equal(f.length,42);a.deepEqual(f.slice(0,8),[r.fixed,String(r.index),String(r.parentIndex),r.originalBits,r.bits,String(r.setting),String(r.scale),r.kind]);
 for(const[j,n]of [[10,'parentOriginalDisplayHex'],[11,'parentOriginalStrHex'],[15,'parentRoundDisplayHex'],[16,'parentRoundStrHex'],[24,'targetDisplayHex'],[25,'targetStrHex']])a.equal(f[j],r[n]);
 for(const[j,k]of [[8,27],[9,28],[10,29],[11,30],[12,31],[13,32],[14,33],[15,34],[16,35],[21,36],[22,37],[23,38],[24,39],[25,40],[26,41]])if([8,13,22].includes(j))a.deepEqual(activeMeta(f[j]),activeMeta(f[k]));else a.equal(f[j],f[k]);
 const meta=Object.fromEntries(f[22].split(';').map(x=>x.split('=')));delete meta.integer;a.deepEqual(meta,r.targetMeta);a.equal(meta.type,'4E');a.equal(meta.width,'0028');a.equal(parseInt(meta.decimals,16),r.scale);a.equal(meta.real,r.bits);a.equal(meta.cw,'027F');a.equal(f[23],r.bits);a.deepEqual(Object.fromEntries(f[26].split(';').map(x=>x.split('='))),r.targetCrt);
}
const interventions=[];
for(const r of rows)if(r.kind==='original'){const shifted=rows.find(s=>s.parentIndex===r.parentIndex&&s.fixed===r.fixed&&s.kind==='shifted');a(shifted);a.equal(r.setting,13);a.equal(shifted.setting,13);a.equal(r.scale,18);a.equal(shifted.scale,18);for(const k of ['type','width','decimals','cw'])a.equal(r.targetMeta[k],shifted.targetMeta[k]);a.equal(r.bits,r.originalBits);
 interventions.push({parentIndex:r.parentIndex,fixed:r.fixed,setting:r.setting,originalTargetIndex:r.index,shiftedTargetIndex:shifted.index,originalBits:r.bits,shiftedBits:shifted.bits,transform:compare(r.targetDisplayHex,shifted.targetDisplayHex),str:compare(r.targetStrHex,shifted.targetStrHex)});
}a.equal(interventions.length,8);
const originalBaselines=rows.filter(r=>r.kind==='original').map(r=>({parentIndex:r.parentIndex,fixed:r.fixed,bits:r.bits,transform:compare(r.targetDisplayHex,r.parentOriginalDisplayHex),str:compare(r.targetStrHex,r.parentOriginalStrHex)}));a.equal(originalBaselines.length,8);
const adjacent=[];
for(const [leftParent,rightParent]of [[159,160],[165,166]])for(const fixed of ['OFF','ON'])for(const kind of ['original','shifted']){
 const left=rows.find(r=>r.parentIndex===leftParent&&r.fixed===fixed&&r.kind===kind),right=rows.find(r=>r.parentIndex===rightParent&&r.fixed===fixed&&r.kind===kind);a(left&&right);const mask=(1n<<63n)-1n;a.equal((BigInt('0x'+right.bits)&mask)-(BigInt('0x'+left.bits)&mask),1n);for(const k of ['type','width','decimals','cw'])a.equal(left.targetMeta[k],right.targetMeta[k]);
 adjacent.push({leftParent,rightParent,fixed,kind,leftIndex:left.index,rightIndex:right.index,leftBits:left.bits,rightBits:right.bits,transform:compare(left.targetDisplayHex,right.targetDisplayHex),str:compare(left.targetStrHex,right.targetStrHex)});
}a.equal(adjacent.length,8);
const targetTransformVsStr=rows.map(r=>({index:r.index,parentIndex:r.parentIndex,setting:r.setting,fixed:r.fixed,kind:r.kind,scale:r.scale,comparison:compare(r.targetDisplayHex,r.targetStrHex)}));
function summary(items,field){const cs=items.map(r=>r[field]);return {comparisons:cs.length,literalDifferent:cs.filter(c=>!c.literalEqual).length,trimmedLiteralDifferent:cs.filter(c=>!c.trimmedLiteralEqual).length,valueDifferent:cs.filter(c=>!c.valueEqual).length,zeroSignDifferent:cs.filter(c=>c.zeroSignDifferent).length};}
const pairedSummary=items=>({transform:summary(items,'transform'),str:summary(items,'str')});
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,rawSha256:digest,metricSelfChecks,contextsChecked:16,
 summary:{payloadInterventions:pairedSummary(interventions),originalReconstructionBaselines:pairedSummary(originalBaselines),adjacentByKindAndFixed:Object.fromEntries(['original','shifted'].map(kind=>[kind,Object.fromEntries(['OFF','ON'].map(fixed=>[fixed,pairedSummary(adjacent.filter(r=>r.kind===kind&&r.fixed===fixed))]))])),targetTransformVsStr:summary(targetTransformVsStr,'comparison')},
 interventions,originalBaselines,adjacent,targetTransformVsStr,
 limitation:'Finite D13 original/sign-copied measured-shift payloads at deliberately controlled N40/18 only. Native measured ABS-shift stage was N32/18; normalization and sign-copy are NEW routes, NOT assumed signed-expression equivalence. Signed-zero spelling/padding kept separate from exact decimal value. No binary payload/decimal text defect metric, fitted tolerance/cap, internal operation/precision, universal route equivalence, accepted formatter or parity claim.'},null,2));
