// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),{readRecord}=require('./read-str-ambient-record.cjs');
const hex=s=>Buffer.from(s).toString('hex').toUpperCase();
function fixture(){
 const bits='3CC1FF130772E184',metadata='type=4E;width=0028;decimals=00000012;integer=00000000;real='+bits+';cw=027F';
 const originalMeta=Object.fromEntries(metadata.split(';').map(s=>s.split('=')));delete originalMeta.integer;
 const p={originalMeta,originalDisplayHex:hex('0'),originalStrHex:hex(' '.repeat(39)+'0'),crt:{kind:'F',digits:'30'},gcvt:{kind:'G',text:'30'}};
 const input={index:184,bits,controls:{OFF:p}};
 const f=Array(44).fill('');Object.assign(f,{0:'OFF',1:'1',2:bits,3:'15',4:metadata,5:bits,6:p.originalDisplayHex,7:p.originalStrHex,
 8:'N',9:hex('15'),10:'C',11:hex('OFF'),12:'kind=F;digits=30',13:'kind=G;text=30',
 18:metadata,19:bits,20:p.originalDisplayHex,21:p.originalStrHex,22:'kind=F;digits=30',23:'kind=G;text=30',
 32:'184',33:'13',34:'N',35:hex('13'),36:'C',37:hex('OFF'),38:metadata,39:bits,40:metadata,41:bits,42:hex('13'),43:hex('OFF')});
 for(const i of [14,16,24,26,28,30])f[i]='C';for(const i of [15,17,25,27,29,31])f[i]=hex(' '.repeat(39)+'0');
 return {f,input};
}
module.exports={fixture};
if(require.main===module){
let checks=0;const good=fixture();a.equal(readRecord(good.f,good.input,'OFF',13,1).fields.length,44);++checks;
for(const [i,value]of [[0,'ON'],[1,'2'],[2,'0000000000000000'],[3,'13'],[4,'bad'],[5,'bad'],[6,'31'],[7,'31'],[8,'C'],[9,hex('18')],[10,'N'],[11,hex('ON')],[12,'ERROR'],[13,'ERROR'],[14,'N'],[15,'30'],[18,'bad'],[19,'bad'],[20,'31'],[21,'31'],[22,'ERROR'],[23,'ERROR'],[24,'N'],[25,'00'],[26,'N'],[27,'FF'],[28,'N'],[29,hex(' '.repeat(39)+'1')],[30,'N'],[31,hex(' '.repeat(39)+'1')],[32,'185'],[33,'0'],[34,'C'],[35,hex('0')],[36,'N'],[37,hex('ON')],[38,'bad'],[39,'bad'],[40,'bad'],[41,'bad'],[42,hex('0')],[43,hex('ON')]]){
 const {f,input}=fixture();f[i]=value;a.throws(()=>readRecord(f,input,'OFF',13,1));++checks;
}
const short=fixture();short.f.pop();a.throws(()=>readRecord(short.f,short.input,'OFF',13,1));++checks;
console.log(JSON.stringify({checks,passed:true,syntheticOnly:true,nativeInvoked:false}));
}
