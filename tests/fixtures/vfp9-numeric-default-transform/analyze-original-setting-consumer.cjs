// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER-capture decimal-metadata comparisons only.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const dir=path.join(__dirname,'original-setting-consumer-captures1'),bytes=n=>z.gunzipSync(fs.readFileSync(path.join(dir,n+'.gz'))),sha=b=>c.createHash('sha256').update(b).digest('hex');
const manifest=JSON.parse(fs.readFileSync(path.join(dir,'manifest.json')));
a.equal(manifest.sourceCommit,'886b8720d56fa770847c79a9dacba4dbdf1d0a5f');a.equal(manifest.sourceStatusClean,true);a.equal(manifest.pairedQualified,true);
for(const n of ['audit.json','first-results.txt','repeat-results.txt']){const r=manifest.files.find(r=>r.name===n);a(r);a.equal(sha(bytes(n)),r.sha256);}
const report=JSON.parse(bytes('audit.json')),digest='47232c78c28f7b7536b654b3c85fd94bd7a6eead587f719298f80762f02c2499';
for(const k of ['documentedFieldsRepeatEqual','parentPayloadAndRuntimeExact','newSettingControlInvariance','targetMetadataAndBRSExact','exactRuntimeIdentity','rawByteEqual'])a.equal(report[k],true);
for(const run of ['first','repeat']){a.equal(report[run+'Sha256'],digest);a.equal(sha(bytes(run+'-results.txt')),digest);a.deepEqual(bytes(run+'-results.txt'),bytes(run+'.out'));a.equal(bytes(run+'.status').toString(),'0\n');a.equal(bytes(run+'.stderr').length,0);}
a.equal(bytes('first-structure.status').toString(),'0\n');a.equal(bytes('audit.status').toString(),'0\n');a.equal(bytes('audit.stderr').length,0);
const lines=bytes('first-results.txt').toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|32');a.equal(lines.length,32);a.equal(report.recordsPerRun,32);a.equal(report.observations.length,32);
const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})*$/);const b=Buffer.from(h,'hex');a(b.every(n=>n>0&&n<128));return b.toString('ascii');};
function decimal(text){const m=text.trim().match(/^([+-]?)(?:(\d+)(?:\.(\d*))?|\.(\d+))(?:[eE]([+-]?\d+))?$/);a(m);const fraction=m[3]===undefined?(m[4]||''):m[3];let n=BigInt((m[2]||'0')+fraction),e=BigInt(m[5]||'0')-BigInt(fraction.length);const negative=m[1]==='-';if(negative)n=-n;const zero=n===0n;if(zero)e=0n;else while(n%10n===0n){n/=10n;++e;}return {coefficient:String(n),exponent:String(e),zero,negative};}
const equal=(x,y)=>x.coefficient===y.coefficient&&x.exponent===y.exponent;
let metricSelfChecks=0;for(const[l,r,eq]of [['0','-0.000',true],['1.20','1.2e0',true],['.0001','1e-4',true],['-12.3400','-1234e-2',true],['0.000000001','0',false],['1','-1',false]]){a.equal(equal(decimal(l),decimal(r)),eq);++metricSelfChecks;}a.equal(decimal('-0.00').negative,true);++metricSelfChecks;a.equal(decimal(' 0.00 ').negative,false);++metricSelfChecks;
function compare(lh,rh){const l=ascii(lh),r=ascii(rh),x=decimal(l),y=decimal(r);return {leftHex:lh,rightHex:rh,literalEqual:lh===rh,trimmedLiteralEqual:l.trimStart()===r.trimStart(),valueEqual:equal(x,y),zeroSignDifferent:x.zero&&y.zero&&x.negative!==y.negative,leftValue:x,rightValue:y};}
const rows=report.observations;
const activeMeta=s=>{a.match(s,/^type=4E;width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);const m=Object.fromEntries(s.split(';').map(x=>x.split('=')));delete m.integer;return m;};
for(const[i,r]of rows.entries()){const f=lines[i].split('|');a.equal(f.length,42);a.deepEqual(f.slice(0,8),[r.fixed,String(r.index),String(r.parentIndex),r.originalBits,r.bits,String(r.setting),String(r.scale),r.kind]);
 for(const[j,n]of [[10,'parentOriginalDisplayHex'],[11,'parentOriginalStrHex'],[15,'parentRoundDisplayHex'],[16,'parentRoundStrHex'],[24,'targetDisplayHex'],[25,'targetStrHex']])a.equal(f[j],r[n]);
 for(const[j,k]of [[8,27],[9,28],[10,29],[11,30],[12,31],[13,32],[14,33],[15,34],[16,35],[21,36],[22,37],[23,38],[24,39],[25,40],[26,41]])if([8,13,22].includes(j))a.deepEqual(activeMeta(f[j]),activeMeta(f[k]));else a.equal(f[j],f[k]);
 a.deepEqual(f.slice(8,21).map((v,i)=>[0,5].includes(i)?activeMeta(v):v),r.controlTuple);a.deepEqual(activeMeta(f[13]),r.parentRoundMeta);a.equal(f[14],r.parentRoundBits);
 const meta=Object.fromEntries(f[22].split(';').map(x=>x.split('=')));delete meta.integer;a.deepEqual(meta,r.targetMeta);a.equal(meta.type,'4E');a.equal(parseInt(meta.width,16),r.width);a.equal(parseInt(meta.decimals,16),r.scale);a.equal(meta.real,r.bits);a.equal(meta.cw,'027F');a.equal(f[23],r.bits);a.deepEqual(Object.fromEntries(f[26].split(';').map(x=>x.split('='))),r.targetCrt);
}
const key=r=>[r.parentIndex,r.fixed,r.setting,r.scale].join(':');const map=new Map();for(const r of rows){a(!map.has(key(r)));map.set(key(r),r);}a.equal(map.size,32);
const ref=r=>({index:r.index,parentIndex:r.parentIndex,fixed:r.fixed,setting:r.setting,scale:r.scale,bits:r.bits});
const pair=(l,r)=>({left:ref(l),right:ref(r),transform:compare(l.targetDisplayHex,r.targetDisplayHex),str:compare(l.targetStrHex,r.targetStrHex)});
const get=(parent,fixed,setting,scale)=>{const r=map.get([parent,fixed,setting,scale].join(':'));a(r);return r;};
const interventions=[],settingInterventions=[],adjacent=[];
for(const parent of [159,160,165,166])for(const fixed of ['OFF','ON'])for(const setting of [12,14]){
 const l=get(parent,fixed,setting,13),r=get(parent,fixed,setting,14);a.equal(l.bits,r.bits);a.equal(l.width,40);a.equal(r.width,40);for(const k of ['type','real','width','cw'])a.equal(l.targetMeta[k],r.targetMeta[k]);a.equal(parseInt(l.targetMeta.decimals,16),13);a.equal(parseInt(r.targetMeta.decimals,16),14);a.deepEqual(l.controlTuple,r.controlTuple);a.deepEqual(l.targetCrt,r.targetCrt);interventions.push(pair(l,r));
}a.equal(interventions.length,16);
for(const parent of [159,160,165,166])for(const fixed of ['OFF','ON'])for(const scale of [13,14]){
 const l=get(parent,fixed,12,scale),r=get(parent,fixed,14,scale);a.equal(l.bits,r.bits);a.deepEqual(l.targetMeta,r.targetMeta);a.deepEqual(l.targetCrt,r.targetCrt);
 // Controls may change with SET DECIMALS: preserve observations, never require equality across D.
 settingInterventions.push({...pair(l,r),original:{metadataEqual:JSON.stringify(l.controlTuple[0])===JSON.stringify(r.controlTuple[0]),bitsEqual:l.controlTuple[1]===r.controlTuple[1],transform:compare(l.parentOriginalDisplayHex,r.parentOriginalDisplayHex),str:compare(l.parentOriginalStrHex,r.parentOriginalStrHex)},round:{metadataEqual:JSON.stringify(l.parentRoundMeta)===JSON.stringify(r.parentRoundMeta),bitsEqual:l.parentRoundBits===r.parentRoundBits,transform:compare(l.parentRoundDisplayHex,r.parentRoundDisplayHex),str:compare(l.parentRoundStrHex,r.parentRoundStrHex)}});
}a.equal(settingInterventions.length,16);
for(const [lp,rp]of [[159,160],[165,166]])for(const fixed of ['OFF','ON'])for(const setting of [12,14])for(const scale of [13,14]){
 const l=get(lp,fixed,setting,scale),r=get(rp,fixed,setting,scale);a.equal(BigInt('0x'+r.bits)-BigInt('0x'+l.bits),1n);for(const k of ['type','width','decimals','cw'])a.equal(l.targetMeta[k],r.targetMeta[k]);adjacent.push(pair(l,r));
}a.equal(adjacent.length,16);
const targetTransformVsStr=rows.map(r=>({index:r.index,parentIndex:r.parentIndex,setting:r.setting,fixed:r.fixed,kind:r.kind,width:r.width,scale:r.scale,comparison:compare(r.targetDisplayHex,r.targetStrHex)}));
function summary(items,field){const cs=items.map(r=>r[field]);return {comparisons:cs.length,literalDifferent:cs.filter(c=>!c.literalEqual).length,trimmedLiteralDifferent:cs.filter(c=>!c.trimmedLiteralEqual).length,valueDifferent:cs.filter(c=>!c.valueEqual).length,zeroSignDifferent:cs.filter(c=>c.zeroSignDifferent).length};}
const pairedSummary=items=>({transform:summary(items,'transform'),str:summary(items,'str')});
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,rawSha256:digest,metricSelfChecks,contextsChecked:32,
 summary:{decimalMetadataInterventions:pairedSummary(interventions),settingInterventions:pairedSummary(settingInterventions),adjacentBySettingScaleFixed:Object.fromEntries([12,14].map(setting=>[setting,Object.fromEntries([13,14].map(scale=>[scale,Object.fromEntries(['OFF','ON'].map(fixed=>[fixed,pairedSummary(adjacent.filter(r=>r.left.setting===setting&&r.left.scale===scale&&r.left.fixed===fixed))]))]))])),targetTransformVsStr:summary(targetTransformVsStr,'comparison')},
 interventions,settingInterventions,adjacent,targetTransformVsStr,
 limitation:'Finite four signed ORIGINAL D13 payloads at decimal metadata13/14, DECIMALS12/14, width40 and both FIXED only. Metadata interventions check same-D control tuples; setting interventions retain observed changed controls without assuming across-D equality. No shift, sign-copy or arithmetic stimulus. Reconstruction remains a NEW route, not a universal producer equivalence claim. Signed-zero spelling and padding separate from exact decimal value. No binary payload/decimal text defect metric, fitted tolerance/cap, internal operation/precision, universal decimal-metadata independence, accepted formatter or parity claim.'},null,2));
