// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: arithmetic relations only.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),crypto=require('node:crypto'),a=require('node:assert/strict');
const {relation}=require('./exact-str-step-relation.cjs');
const root=path.join(__dirname,'exact-str-counts-captures1'),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const bytes=fs.readFileSync(path.join(root,'analysis.out.gz'));
const hash='d007657a0fc003559ea9b9e4b08d9e23c6060f23108cfdd4b9a23b2780e2c46e';a.equal(sha(bytes),hash);
const entries=JSON.parse(fs.readFileSync(path.join(root,'manifest.json'))).filter(r=>r.name==='analysis.out.gz');a.equal(entries.length,1);
a.equal(entries[0].sha256,hash);a.equal(entries[0].bytes,bytes.length);
const raw=z.gunzipSync(bytes);a.equal(raw.length,entries[0].decodedBytes);a.equal(sha(raw),entries[0].decodedSha256);
const parent=JSON.parse(raw);a.equal(parent.pairs.length,1584);
const modes=['nearestEven','nearestAway'];
const pairs=parent.pairs.map(pair=>({...pair,relations:Object.fromEntries(modes.map(mode=>[mode,
 relation(pair.observedStep,pair.transitions[mode].nearestStep,pair.transitions[mode].residualStep)]))}));
const groups=[];
for(const fixed of ['OFF','ON'])for(const lowCount of [15,16,17])for(const mode of modes){
 const selected=pairs.filter(r=>r.fixed===fixed&&r.lowCount===lowCount);a.equal(selected.length,264);
 groups.push({fixed,lowCount,mode,contexts:264,categories:Object.fromEntries(
 ['both-zero','cancelled','residual-only','equal-step','mixed'].map(category=>[category,selected.filter(r=>r.relations[mode].category===category).length]))});
}
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,parentCompressedSha256:hash,
 parentDecodedSha256:sha(raw),groups,pairs,changing:pairs.filter(r=>r.observedStep!=='0'),
 limitation:'Complete arithmetic relation of observed and exact-nearest transitions, not causal explanation, internal-call attribution, tolerance or accepted conversion.'},null,2));
