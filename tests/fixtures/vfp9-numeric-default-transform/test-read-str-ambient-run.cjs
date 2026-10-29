// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{readRun,pair}=require('./read-str-ambient-run.cjs'),{fixture}=require('./test-read-str-ambient-record.cjs');
const hex=s=>Buffer.from(s).toString('hex').toUpperCase();
const selection={inputs:Array.from({length:6},(_,i)=>{const {input}=fixture();input.index=184+i;input.controls.ON=structuredClone(input.controls.OFF);return input;})};
const lines=[];
for(const [i,input]of selection.inputs.entries())for(const fixed of ['OFF','ON'])for(const ambientD of [0,13,18]){
 const {f}=fixture();f[0]=fixed;f[1]=String(i+1);f[32]=String(input.index);f[33]=String(ambientD);
 for(const field of [11,37,43])f[field]=hex(fixed);for(const field of [35,42])f[field]=hex(String(ambientD));lines.push(f.join('|'));
}
const raw=rows=>Buffer.from(['VERSION|Visual FoxPro 09.00.0000.7423 for Windows',...rows,'DONE|36',''].join('\r\n'));
const good=raw(lines);let checks=0;a.equal(readRun(good,selection).length,36);++checks;a.equal(pair(good,good,selection).rawByteEqual,true);++checks;
for(const mutate of [r=>r.pop(),r=>r.push(r[0]),r=>r.reverse(),r=>r[0]='ERROR-LOAD|1',r=>r[1]=r[0]]){const rows=[...lines];mutate(rows);a.throws(()=>readRun(raw(rows),selection));++checks;}
for(const bad of [Buffer.from(good.toString().replaceAll('\r\n','\n')),good.subarray(0,good.length-2),Buffer.concat([good,Buffer.from('x')]),Buffer.from(good.toString().replace('DONE|36','DONE|35'))]){a.throws(()=>readRun(bad,selection));++checks;}
const changed=[...lines];const fields=changed[0].split('|');fields[4]=fields[4].replace('integer=00000000','integer=12345678');changed[0]=fields.join('|');
const inactive=pair(good,raw(changed),selection);a.equal(inactive.inactiveSlotDifferences,1);a.equal(inactive.rawByteEqual,false);++checks;
const output=[...lines],f=output[0].split('|');f[25]=hex(' '.repeat(39)+'1');output[0]=f.join('|');a.throws(()=>pair(good,raw(output),selection));++checks;
console.log(JSON.stringify({checks,passed:true,syntheticOnly:true,nativeInvoked:false}));
