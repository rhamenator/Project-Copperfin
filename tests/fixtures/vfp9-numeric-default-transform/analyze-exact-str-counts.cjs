// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: retained count transitions only.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),crypto=require('node:crypto'),a=require('node:assert/strict');
const {join}=require('./join-exact-str-counts.cjs');
const root=path.join(__dirname,'exact-str-residual-captures1');
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const compressed=fs.readFileSync(path.join(root,'analysis.out.gz'));
const hash='ed986ab02cb0aaf5500c87b7b8bd5e169cad465946f5383929cddc5ae3b5c202';a.equal(sha(compressed),hash);
const manifest=JSON.parse(fs.readFileSync(path.join(root,'manifest.json')));
const entries=manifest.filter(r=>r.name==='analysis.out.gz');a.equal(entries.length,1);
a.equal(entries[0].sha256,hash);a.equal(entries[0].bytes,compressed.length);
const raw=z.gunzipSync(compressed);a.equal(raw.length,entries[0].decodedBytes);a.equal(sha(raw),entries[0].decodedSha256);
const parent=JSON.parse(raw);a.equal(parent.rows.length,2112);
const pairs=join(parent.rows);a.equal(pairs.length,1584);
const groups=[];
for(const fixed of ['OFF','ON'])for(const lowCount of [15,16,17]){
 const selected=pairs.filter(r=>r.fixed===fixed&&r.lowCount===lowCount);a.equal(selected.length,264);
 const histogram=new Map();for(const r of selected)histogram.set(r.observedStep,(histogram.get(r.observedStep)||0)+1);
 groups.push({fixed,lowCount,highCount:lowCount+1,contexts:264,
  observedStepHistogram:[...histogram].sort(([x],[y])=>BigInt(x)<BigInt(y)?-1:BigInt(x)>BigInt(y)?1:0).map(([units,contexts])=>({units,contexts}))});
}
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,parentCompressedSha256:hash,
 parentDecodedSha256:sha(raw),groups,pairs,
 nonzeroObservedSteps:pairs.filter(r=>r.observedStep!=='0'),
 limitation:'Exact adjacent-count observed/nearest/residual transitions; all endpoints retained. Not a cap, rounding repair, tolerance or production formatter.'},null,2));
