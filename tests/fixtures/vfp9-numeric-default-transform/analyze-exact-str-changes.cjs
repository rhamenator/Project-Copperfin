// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: retained changing-context partition.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),crypto=require('node:crypto'),a=require('node:assert/strict');
const {classify}=require('./classify-exact-str-changes.cjs');
const root=path.join(__dirname,'exact-str-counts-captures1'),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const bytes=fs.readFileSync(path.join(root,'analysis.out.gz'));
const hash='d007657a0fc003559ea9b9e4b08d9e23c6060f23108cfdd4b9a23b2780e2c46e';a.equal(sha(bytes),hash);
const entries=JSON.parse(fs.readFileSync(path.join(root,'manifest.json'))).filter(r=>r.name==='analysis.out.gz');a.equal(entries.length,1);
a.equal(entries[0].sha256,hash);a.equal(entries[0].bytes,bytes.length);
const raw=z.gunzipSync(bytes);a.equal(raw.length,entries[0].decodedBytes);a.equal(sha(raw),entries[0].decodedSha256);
const parent=JSON.parse(raw);a.equal(parent.pairs.length,1584);
const result=classify(parent.pairs);a.equal(result.contexts.length,528);
const groups=[];for(const fixed of ['OFF','ON'])for(const changedSteps of [0,1,2,3]){
 const selected=result.contexts.filter(r=>r.fixed===fixed&&r.changedSteps===changedSteps);
 groups.push({fixed,changedSteps,contexts:selected.length,indices:selected.map(r=>r.index)});
}
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,parentCompressedSha256:hash,
 parentDecodedSha256:sha(raw),groups,...result,
 limitation:'Complete finite changing/unchanged partition; raw endpoints and nearest residuals retained. No causal claim, cap, tolerance or accepted conversion rule.'},null,2));
