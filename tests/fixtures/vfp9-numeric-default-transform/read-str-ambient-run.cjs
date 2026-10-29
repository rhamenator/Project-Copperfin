// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: strict complete native framing.
'use strict';
const a=require('node:assert/strict'),{readRecord}=require('./read-str-ambient-record.cjs');
function readRun(raw,selection){
 a(Buffer.isBuffer(raw));a(raw.length<1024*1024);a(raw.every(x=>x<128));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
 a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
 a.equal(lines.pop(),'DONE|36');a.equal(lines.length,36);a.equal(selection.inputs.length,6);
 const observations=[];let ordinal=0;
 for(const [i,input]of selection.inputs.entries())for(const fixed of ['OFF','ON'])for(const ambientD of [0,13,18]){
  observations.push(readRecord(lines[ordinal++].split('|'),input,fixed,ambientD,i+1));
 }
 return observations;
}
function pair(firstRaw,repeatRaw,selection){
 const first=readRun(firstRaw,selection),repeat=readRun(repeatRaw,selection);let inactiveSlotDifferences=0;
 for(let i=0;i<36;++i)for(let field=0;field<44;++field){
  const x=first[i].fields[field],y=repeat[i].fields[field];if(x===y)continue;
  a([4,18,38,40].includes(field),'unexpected paired field difference');
  // readRecord already validates every active metadata field against parent.
  const strip=s=>s.replace(/;integer=[0-9A-F]{8};/,';');a.equal(strip(x),strip(y));++inactiveSlotDifferences;
 }
 return {recordsPerRun:36,observationsPerRun:144,documentedFieldsRepeatEqual:true,
  rawByteEqual:firstRaw.equals(repeatRaw),inactiveSlotDifferences,observations:first};
}
module.exports={readRun,pair};
