// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Synthetic parser tests only; no native or compatibility expected output.
'use strict';
const a=require('node:assert/strict'),{readRecord,paddedHex}=require('./read-sprintf-record.cjs');
const bits='3FF0000000000000',text='1.000000000000000',hex=s=>Buffer.from(s,'ascii').toString('hex').toUpperCase(),expected=paddedHex(text);
const inv={modulePath:'C:\\windows\\system32\\MSVCR71.dll',moduleSha256:'a'.repeat(64),moduleFileSize:0x55000,modulePe:{machine:0x14c,magic:0x10b,stamp:0x3e561eac,imageSize:0x56000,checksum:0x5f1e9}};
const id=`real=${bits};cw0=027F;cw1=027F;module=${hex(inv.modulePath)};fileSample=ONCE;fileSize=00055000;fileSha256=${'A'.repeat(64)};machine=014C;magic=010B;stamp=3E561EAC;imageSize=00056000;checksum=0005F1E9`;
const meta=`type=4E;width=0028;decimals=00000012;integer=00000000;real=${bits};cw=027F`,control=`kind=F;count=00000011;${id};decimal=00000001;sign=00000000;digits=3130`;
// Put fields in the production order required by the strict regular expression.
const ctl=`kind=F;count=00000011;real=${bits};decimal=00000001;sign=00000000;cw0=027F;cw1=027F;module=${hex(inv.modulePath)};fileSample=ONCE;fileSize=00055000;fileSha256=${'A'.repeat(64)};machine=014C;magic=010B;stamp=3E561EAC;imageSize=00056000;checksum=0005F1E9;digits=3130`;
const result=(s=text,n=text,sg=1,ng=1)=>`kind=S;count=0000000F;real=${bits};cw0=027F;cw1=027F;module=${hex(inv.modulePath)};fileSample=ONCE;fileSize=00055000;fileSha256=${'A'.repeat(64)};machine=014C;magic=010B;stamp=3E561EAC;imageSize=00056000;checksum=0005F1E9;capacity=00000800;canary=PASS;sprintfGrammar=${sg};snprintfGrammar=${ng};equal=${s===n?1:0};sprintf=${hex(s)};snprintf=${hex(n)}`;
const row={index:1,bits,setting:0,expectedStrHex:{15:expected}},line=r=>['1',bits,'0','15',meta,bits,expected,ctl,r,meta,bits,expected,ctl,'N',hex('0'),'C',hex('OFF'),'OFF'].join('|');
let checks=0;function accept(r=result()){const o=readRecord(line(r),row,15,inv);++checks;return o;}function reject(mutator){const f=line(result()).split('|');mutator(f);a.throws(()=>readRecord(f.join('|'),row,15,inv));++checks;}
let o=accept();a.equal(o.mismatches.length,0);o=accept(result('1.000000000000001',text));a.equal(o.mismatches.length,2);a.equal(o.mismatches[0].rawText,'1.000000000000001');
o=accept(result('1,000000000000000','1,000000000000000',0,0));a.equal(o.crt.sprintfGrammar,'0');a.equal(o.mismatches.length,4);
o=accept(result('1'.repeat(41),'1'.repeat(41),0,0));a.equal(o.mismatches.length,4);a(o.mismatches.every(m=>m.overWidth));
for(const [column,value]of [[0,'2'],[1,'0000000000000000'],[2,'1'],[3,'16'],[4,meta.replace('width=0028','width=0020')],[5,'0000000000000000'],[6,hex('2.000000000000000')],[7,ctl.replace('cw0=027F','cw0=037F')],[9,meta.replace('decimals=00000012','decimals=00000011')],[10,'0000000000000000'],[11,hex('2.000000000000000')],[12,ctl.replace('fileSample=ONCE','fileSample=OTHER')],[13,'C'],[14,hex('1')],[15,'N'],[16,hex('ON')],[17,'ON']])reject(f=>{f[column]=value;});
reject(f=>{f[8]=result(text,text).replace('equal=1','equal=0');});reject(f=>{f[8]=result().replace('capacity=00000800','capacity=00000400');});reject(f=>f.push('EXTRA'));
reject(f=>{f[8]=result().replace('sprintfGrammar=1','sprintfGrammar=0');});
console.log(JSON.stringify({syntheticRecordReaderChecks:checks,nativeInvoked:false,mismatchesPreserved:true,nonGrammarTextPreserved:true}));
