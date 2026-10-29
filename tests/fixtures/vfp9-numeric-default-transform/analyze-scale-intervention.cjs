// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// AFTER-capture bounded route/scale comparisons; no formatter/expected-output model.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const dir=path.join(__dirname,'scale-intervention-captures1'),bytes=n=>z.gunzipSync(fs.readFileSync(path.join(dir,n+'.gz'))),sha=b=>c.createHash('sha256').update(b).digest('hex');
const manifest=JSON.parse(fs.readFileSync(path.join(dir,'manifest.json')));
a.equal(manifest.sourceCommit,'1a933a5beb097dcd0ad38b4d2adac57f24742b43');a.equal(manifest.sourceStatusClean,true);a.equal(manifest.pairedQualified,true);
for(const n of ['audit.json','first-results.txt','repeat-results.txt']){const r=manifest.files.find(r=>r.name===n);a(r);a.equal(sha(bytes(n)),r.sha256);}
const report=JSON.parse(bytes('audit.json')),digest='d6fef944a8f3fd2732703b7f8a81190a5743be78dcad758341cdd97702cb0fc2';
for(const k of ['documentedFieldsRepeatEqual','parentControlsExact','targetMetadataAndBRSExact','exactRuntimeIdentity','rawByteEqual'])a.equal(report[k],true);
for(const run of ['first','repeat']){a.equal(report[run+'Sha256'],digest);a.equal(sha(bytes(run+'-results.txt')),digest);a.deepEqual(bytes(run+'-results.txt'),bytes(run+'.out'));a.equal(bytes(run+'.status').toString(),'0\n');a.equal(bytes(run+'.stderr').length,0);}
a.equal(bytes('first-structure.status').toString(),'0\n');a.equal(bytes('audit.status').toString(),'0\n');a.equal(bytes('audit.stderr').length,0);
const lines=bytes('first-results.txt').toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|40');a.equal(lines.length,40);a.equal(report.recordsPerRun,40);a.equal(report.observations.length,40);
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
for(const r of rows)if(r.setting!==18&&r.scale===18){const s=rows.find(s=>s.parentIndex===r.parentIndex&&s.fixed===r.fixed&&s.kind===r.kind&&s.scale===r.setting);a(s);a.equal(r.bits,s.bits);a.equal(r.targetMeta.type,s.targetMeta.type);a.equal(r.targetMeta.width,s.targetMeta.width);interventions.push({parentIndex:r.parentIndex,setting:r.setting,fixed:r.fixed,kind:r.kind,bits:r.bits,scale18Index:r.index,scaleDIndex:s.index,transform:compare(r.targetDisplayHex,s.targetDisplayHex),str:compare(r.targetStrHex,s.targetStrHex)});}
a.equal(interventions.length,16);
const originalBaselines=rows.filter(r=>r.kind==='original'&&r.scale===18).map(r=>({parentIndex:r.parentIndex,setting:r.setting,fixed:r.fixed,bits:r.bits,transform:compare(r.targetDisplayHex,r.parentOriginalDisplayHex),str:compare(r.targetStrHex,r.parentOriginalStrHex)}));
const roundBaselines=rows.filter(r=>r.kind==='round'&&r.scale===r.setting).map(r=>({parentIndex:r.parentIndex,setting:r.setting,fixed:r.fixed,bits:r.bits,transform:compare(r.targetDisplayHex,r.parentRoundDisplayHex),str:compare(r.targetStrHex,r.parentRoundStrHex)}));
a.equal(originalBaselines.length,12);a.equal(roundBaselines.length,12);
const targetTransformVsStr=rows.map(r=>({index:r.index,parentIndex:r.parentIndex,setting:r.setting,fixed:r.fixed,kind:r.kind,scale:r.scale,comparison:compare(r.targetDisplayHex,r.targetStrHex)}));
function summary(items,field){const cs=items.map(r=>r[field]);return {comparisons:cs.length,literalDifferent:cs.filter(c=>!c.literalEqual).length,trimmedLiteralDifferent:cs.filter(c=>!c.trimmedLiteralEqual).length,valueDifferent:cs.filter(c=>!c.valueEqual).length,zeroSignDifferent:cs.filter(c=>c.zeroSignDifferent).length};}
const pairedSummary=items=>({transform:summary(items,'transform'),str:summary(items,'str')});
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,rawSha256:digest,metricSelfChecks,contextsChecked:40,
 summary:{scaleInterventions:pairedSummary(interventions),originalReconstructionBaselines:pairedSummary(originalBaselines),roundReconstructionBaselines:pairedSummary(roundBaselines),targetTransformVsStr:summary(targetTransformVsStr,'comparison')},
 interventions,originalBaselines,roundBaselines,targetTransformVsStr,
 limitation:'Finite reconstructed routes at D16/17 and unchanged-scale D18 controls only. Signed-zero spelling/padding kept separate from decimal value. No binary payload/decimal text representation comparison, precision/tolerance/formatter algorithm, hidden-state absence or universal route equivalence is inferred.'},null,2));
