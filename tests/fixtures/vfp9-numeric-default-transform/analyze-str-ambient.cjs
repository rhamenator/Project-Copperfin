// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: retain every exact ambient STR mismatch.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),crypto=require('node:crypto'),a=require('node:assert/strict');
const {audit}=require('./audit-str-ambient.cjs');
const {readRun}=require('./read-str-ambient-run.cjs');
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
function analyze(observations){
 a.equal(observations.length,36);const byKey=new Map();
 for(const row of observations){const key=[row.index,row.fixed,row.ambientD].join('|');a(!byKey.has(key));byKey.set(key,row);}
 const mismatches=[];let comparisons=0;
 for(const index of [...new Set(observations.map(r=>r.index))])for(const fixed of ['OFF','ON']){
  const base=byKey.get([index,fixed,0].join('|'));a(base);
  for(const ambientD of [13,18]){const actual=byKey.get([index,fixed,ambientD].join('|'));a(actual);
   for(const count of [15,16,17,18]){++comparisons;const field='str'+count+'Hex';
    if(base[field]!==actual[field])mismatches.push({index,bits:base.bits,fixed,count,baselineAmbientD:0,actualAmbientD:ambientD,baselineHex:base[field],actualHex:actual[field]});
   }
  }
 }
 a.equal(comparisons,96);
 return {records:36,observations:144,exactAmbientComparisons:comparisons,ambientMismatchCount:mismatches.length,ambientMismatches:mismatches,
  ambientIndependentForObservedGrid:mismatches.length===0,
  limitation:'Six sign-paired payloads, FIXED OFF/ON, DECIMALS 0/13/18 and explicit STR counts 15..18; public observations only.'};
}
module.exports={analyze};
if(require.main===module){
 const root=process.argv[2];a(root);const qualified=audit(root,false);
 const selectionRaw=z.gunzipSync(fs.readFileSync(path.join(__dirname,'str-ambient-selection-captures1/selection.out.gz')));
 const selection=JSON.parse(selectionRaw),firstRaw=fs.readFileSync(path.join(root,'first-results.txt'));
 const report={sourceSha256:qualified.sourceSha256,selectionSha256:qualified.selectionSha256,firstSha256:sha(firstRaw),pairedQualified:qualified.pairedQualified,...analyze(readRun(firstRaw,selection))};
 process.stdout.write(JSON.stringify(report,null,2)+'\n');
}
