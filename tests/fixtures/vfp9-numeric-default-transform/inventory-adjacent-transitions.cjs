// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: retained adjacent observations ONLY.
// No interpolation, fitted threshold, formatter or new native capture.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),cp=require('node:child_process'),a=require('node:assert/strict'),c=require('node:crypto');
const sha=b=>c.createHash('sha256').update(b).digest('hex'),dir=path.join(__dirname,'round-decomposition-analysis1');
const manifest=JSON.parse(fs.readFileSync(path.join(dir,'manifest.json')));
function bound(name){const r=manifest.files.find(r=>r.name===name);a(r);const b=z.gunzipSync(fs.readFileSync(path.join(dir,name+'.gz')));a.equal(b.length,r.bytes);a.equal(sha(b),r.sha256);return b;}
a.deepEqual(bound('source-decompose-round-observations.cjs'),fs.readFileSync(path.join(__dirname,'decompose-round-observations.cjs')));
const run=cp.spawnSync(process.execPath,[path.join(__dirname,'decompose-round-observations.cjs')],{maxBuffer:3*1024*1024});a.equal(run.status,0);a.equal(run.stderr.length,0);a.deepEqual(run.stdout,bound('decomposition.json'));
const controls=JSON.parse(run.stdout);a.equal(controls.nativeInvoked,false);a.equal(controls.sharedContextsChecked,528);a.equal(controls.onContexts,264);a.equal(controls.metricSelfChecks,8);
const original=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,'str-middle-captures1/audit.json.gz'))));
const originalRows=new Map(original.observations.filter(r=>r.fixed==='ON').map(r=>[r.index,r]));a.equal(originalRows.size,264);
const mask=(1n<<63n)-1n;
function payload(bits){a.match(bits,/^[0-9A-F]{16}$/);const b=BigInt('0x'+bits);a.notEqual((b>>52n)&2047n,2047n);return {magnitude:b&mask,negative:!!(b>>63n)};}
const valueKey=v=>v.coefficient+':'+v.exponent;
let metricSelfChecks=0;
a.equal(payload('BFE0000000000000').magnitude,payload('3FE0000000000000').magnitude);++metricSelfChecks;
a.equal(payload('BFE0000000000000').negative,true);++metricSelfChecks;
a.equal(payload('3FE0000000000000').magnitude-payload('3FDFFFFFFFFFFFFF').magnitude,1n);++metricSelfChecks;
a.equal(payload('3FE0000000000001').magnitude-payload('3FE0000000000000').magnitude,1n);++metricSelfChecks;
a.equal(payload('0000000000000001').magnitude-payload('0000000000000000').magnitude,1n);++metricSelfChecks;
a.equal(payload('0010000000000000').magnitude-payload('000FFFFFFFFFFFFF').magnitude,1n);++metricSelfChecks;
a.equal(payload('8000000000000000').magnitude,0n);++metricSelfChecks;
a.throws(()=>payload('7FF0000000000000'));++metricSelfChecks;
const groups=new Map();
for(const r of controls.rows){const p=originalRows.get(r.index);a(p);a.equal(p.bits,r.bits);a.equal(p.setting,r.setting);a.equal(p.originalMeta.type,'4E');a.equal(p.originalMeta.width,'0028');a.equal(p.originalMeta.decimals,'00000012');a.equal(p.originalMeta.real,r.bits);a.equal(p.originalMeta.cw,'027F');
 const b=payload(r.bits),key=r.setting+':'+b.negative;if(!groups.has(key))groups.set(key,{setting:r.setting,negative:b.negative,nodes:new Map()});const g=groups.get(key);
 const n={index:r.index,bits:r.bits,transformHex:r.originalVsRoundTransform.leftHex,strHex:r.originalStrVsRoundStr.leftHex,transformValue:r.originalVsRoundTransform.leftValue,strValue:r.originalStrVsRoundStr.leftValue,publicRoundBits:r.inlineBits};
 a.equal(n.transformHex,p.originalDisplayHex);a.equal(n.strHex,p.originalStrHex);
 const mag=String(b.magnitude);if(!g.nodes.has(mag))g.nodes.set(mag,[]);g.nodes.get(mag).push(n);
}
const components=[],adjacentPairs=[],coincidentPayloadConflicts=[];let observedNodes=0,nonAdjacentGaps=0;
for(const g of groups.values()){
 const nodes=[...g.nodes.entries()].map(([m,observations])=>({m:BigInt(m),observations})).sort((x,y)=>x.m<y.m?-1:x.m>y.m?1:0);observedNodes+=nodes.length;
 let component=null,previous=null;
 for(const n of nodes){
  const signatures=new Set(n.observations.map(r=>r.transformHex+':'+r.strHex));n.unambiguous=signatures.size===1;
  if(!n.unambiguous)coincidentPayloadConflicts.push({setting:g.setting,negative:g.negative,observations:n.observations});
  if(!previous||n.m-previous.m!==1n){if(previous)++nonAdjacentGaps;component={id:components.length+1,setting:g.setting,negative:g.negative,nodes:[]};components.push(component);}
  component.nodes.push({magnitudePayloadHex:n.m.toString(16).toUpperCase().padStart(16,'0'),unambiguous:n.unambiguous,observations:n.observations});
  if(previous&&n.m-previous.m===1n){const left=previous.observations[0],right=n.observations[0],unambiguous=previous.unambiguous&&n.unambiguous;
   adjacentPairs.push({component:component.id,setting:g.setting,negative:g.negative,increasingMagnitude:true,unambiguous,left,right,
    transformValueChanges:unambiguous?valueKey(left.transformValue)!==valueKey(right.transformValue):null,
    strValueChanges:unambiguous?valueKey(left.strValue)!==valueKey(right.strValue):null,
    transformLiteralChanges:unambiguous?left.transformHex!==right.transformHex:null,strLiteralChanges:unambiguous?left.strHex!==right.strHex:null});
  }previous=n;
 }
}
a.equal(components.reduce((n,c)=>n+c.nodes.reduce((m,r)=>m+r.observations.length,0),0),264);
const changed=adjacentPairs.filter(r=>r.transformValueChanges===true||r.strValueChanges===true);
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,metricSelfChecks,reusedValidatorMetricChecks:8,sharedContextsChecked:528,onContexts:264,sourceHashes:controls.sourceHashes,
 summary:{settingSignGroups:groups.size,observedDistinctPayloadNodes:observedNodes,connectedComponents:components.length,nonAdjacentGaps,adjacentPairs:adjacentPairs.length,transformValueChanges:adjacentPairs.filter(r=>r.transformValueChanges===true).length,strValueChanges:adjacentPairs.filter(r=>r.strValueChanges===true).length,consumerBoundaryDisagreements:adjacentPairs.filter(r=>r.unambiguous&&r.transformValueChanges!==r.strValueChanges).length,coincidentPayloadConflicts:coincidentPayloadConflicts.length,componentsWithoutAdjacentValueChange:components.filter(c=>!changed.some(r=>r.component===c.id)).map(c=>c.id)},
 components,adjacentPairs,changed,coincidentPayloadConflicts,
 limitation:'Only observed ORIGINAL N40/18 FIXED ON inputs at the same applied D/sign are grouped, ordered by finite IEEE payload magnitude. An adjacent pair is one raw payload step, not proof of global monotonicity/unique threshold. Uncaptured gaps are excluded, conflicting coincident payloads cannot qualify a transition. No extension to older-runtime evidence, interpolation/fitted tolerance/formatter/expected output or new native execution.'},null,2));
