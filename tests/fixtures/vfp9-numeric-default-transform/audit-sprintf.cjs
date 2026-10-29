// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: strict actual-observation reader.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto'),{readRecord,meta}=require('./read-sprintf-record.cjs');
const fixture=__dirname,inv=JSON.parse(fs.readFileSync(path.join(fixture,'sprintf-cases.json'))),source=fs.readFileSync(path.join(fixture,'sprintf.prg'),'utf8'),root=process.argv[2];a(root);
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),bytes=(name)=>{const p=path.join(root,name);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
a.equal(inv.inputCount,264);a.equal(inv.recordsPerRun,1056);a.equal(inv.fieldsPerRecord,18);a.deepEqual(inv.counts,[15,16,17,18]);a.equal(inv.inputs.length,264);a.equal(inv.vfpControlComparisons,2112);
a.equal(inv.inputRoute,'Inline CFNDEC(CFNWIDTH(payload,40),18) at every numeric consumer; no stored numeric argument.');a(!/\bargument\s*=/i.test(source));a(source.includes('crtResult=CFNSPRTF(CFNDEC(CFNWIDTH(payload,40),18),conversionCount)'));
for(const [i,row]of inv.inputs.entries()){a.equal(row.index,i+1);a(source.includes(`inputHex[${row.index}]='${row.bits}'`));a(source.includes(`targetDecimals[${row.index}]=${row.setting}`));}
a.equal(bytes('supplied-runtime-env.txt').toString(),inv.runtimeEnvironment+'\n');a(['UNSET\n','\n'].includes(bytes('inherited-dll-overrides.txt').toString()));
function readRun(name){
 a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));a(raw.every(x=>x<128));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|1056');a.equal(lines.length,1056);
 const observations=[];let ordinal=0;for(const row of inv.inputs)for(const count of inv.counts)observations.push(readRecord(lines[ordinal++],row,count,inv));
 return {raw,observations,mismatches:observations.flatMap(o=>o.mismatches.map(m=>({index:o.index,bits:o.bits,setting:o.setting,count:o.count,...m})))};
}
const first=readRun('first');
if(process.argv[3]==='--single'){
 console.log(JSON.stringify({firstComplete:true,records:1056,firstSha256:sha(first.raw),mismatchCount:first.mismatches.length,mismatches:first.mismatches,pairedQualified:false}));process.exit(0);
}
a.equal(bytes('first-structure.status').toString(),'0\n');a.equal(bytes('first-structure.stderr').length,0);const receipt=JSON.parse(bytes('first-structure.json'));
a.equal(receipt.firstComplete,true);a.equal(receipt.pairedQualified,false);a.equal(receipt.firstSha256,sha(first.raw));a.equal(receipt.records,1056);a.equal(receipt.mismatchCount,first.mismatches.length);a.deepEqual(receipt.mismatches,first.mismatches);
const repeat=readRun('repeat');a.deepEqual(first.mismatches,repeat.mismatches);let inactiveSlotDifferences=0;
for(let i=0;i<1056;++i){const x=first.observations[i],y=repeat.observations[i];a.deepEqual({...x,fields:undefined},{...y,fields:undefined});for(let k=0;k<18;++k){if(x.fields[k]===y.fields[k])continue;a([4,9].includes(k));a.deepEqual(meta(x.fields[k],x.bits),meta(y.fields[k],y.bits));++inactiveSlotDifferences;}}
console.log(JSON.stringify({requirement:inv.requirement,recordsPerRun:1056,firstComplete:true,pairedQualified:true,documentedFieldsRepeatEqual:true,exactRuntimeIdentity:true,
 firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),rawByteEqual:first.raw.equals(repeat.raw),inactiveSlotDifferences,mismatchCount:first.mismatches.length,mismatches:first.mismatches,
 limitation:inv.limitation},null,2));
