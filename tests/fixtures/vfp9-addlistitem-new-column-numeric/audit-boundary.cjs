// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native-only dual-route captured readback audit, NOT a conversion expected table.
const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const root=process.argv[2]??'build/addlistitem-new-column-native';
for(const f of ['boundary-first.status','boundary-repeat.status','boundary.cmp.status'])assert.equal(fs.readFileSync(root+'/'+f,'utf8').trim(),'0',f);
for(const f of ['boundary-first.stderr','boundary-repeat.stderr'])assert.equal(fs.readFileSync(root+'/'+f,'utf8'),'',f);
const first=fs.readFileSync(root+'/boundary-first-results.txt'),repeat=fs.readFileSync(root+'/boundary-repeat-results.txt');assert(first.equals(repeat));
const source=fs.readFileSync('tests/fixtures/vfp9-addlistitem-new-column-numeric/boundary-probe.prg','utf8');
const operands=[...source.matchAll(/^operands\[\d+\]='([^']+)'$/gm)].map(m=>m[1]),columns=[...source.matchAll(/^sampleColumns\[\d+\]=(\d+)$/gm)].map(m=>m[1]);assert.equal(operands.length,36);assert.equal(columns.length,14);
const lines=first.toString('utf8').trim().split(/\r?\n/);assert.equal(lines.length,146);assert.equal(lines[0],'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');assert.equal(lines.at(-1),'DONE|144');
const groups=new Map(),observations=[];let admitted=0,rejected=0;
for(const line of lines.slice(1,-1)){
 const r=line.split('|');assert.equal(r.length,13);const [tag,kind,shape,id,operand,phase,error,expandError,bits,before,after,row,cells]=r;
 assert.equal(tag,'READ');assert(['ListBox','ComboBox'].includes(kind));assert(['0','1'].includes(shape));assert.equal(id,shape==='0'?'11':'44');assert.equal(phase,'invoke');assert.equal(expandError,'0');assert(/^[A-F0-9]{16}$/.test(bits));
 const key=kind+'|'+shape;if(!groups.has(key))groups.set(key,[]);groups.get(key).push(operand);
 const beforeCount=Number(before.match(/^count=(\d+);/)[1]),afterCount=Number(after.match(/^count=(\d+);/)[1]);assert.equal(beforeCount,shape==='0'?0:3);
 if(error==='11'){rejected++;assert.equal(before,after);assert.equal(row,'0');assert.equal(cells,'<no-target>');}
 else{assert.equal(error,'0');admitted++;assert.equal(afterCount,beforeCount+1);assert(Number(row)>0);const reads=[...cells.matchAll(/\[(\d+);(\d+);([^;\]]*);(\d+);([^\]]*)\]/g)];assert.equal(reads.length,14);assert.deepEqual(reads.map(m=>m[1]),columns);for(const m of reads){assert.equal(m[2],'0');assert.equal(m[4],'0');assert.equal(m[3],m[5],'independent index/stable-ID read agreement');}observations.push({kind,shape,operand,bits,observedCandidateColumns:reads.filter(m=>m[3]==='C:candidate').map(m=>Number(m[1]))});}
}
assert.equal(groups.size,4);for(const operandsSeen of groups.values())assert.deepEqual(operandsSeen,operands);
for(const observation of observations)console.log(JSON.stringify(observation));
console.log(JSON.stringify({completeCapture:true,subjects:144,groups:4,operands:36,admitted,rejected,sampledColumns:14,bothReadRoutesEqual:true,sourceSHA256:crypto.createHash('sha256').update(source).digest('hex'),rawSHA256:crypto.createHash('sha256').update(first).digest('hex'),note:'Observation/capture-only audit. Expansion65538 is test reach, not a native global capacity/default allocation/hidden physical storage or Copperfin shared-creation parity claim.'}));
