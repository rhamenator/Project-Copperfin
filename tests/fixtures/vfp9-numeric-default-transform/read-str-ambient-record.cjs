// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: strict public observation reader.
'use strict';
const a=require('node:assert/strict');
function ascii(hex){a.match(hex,/^(?:[0-9A-F]{2})*$/);const b=Buffer.from(hex,'hex');a(b.every(x=>x>0&&x<128));return b.toString('ascii');}
function meta(text,parent,bits){
 a.match(text,/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
 const m=Object.fromEntries(text.split(';').map(s=>s.split('=')));a.equal(m.real,bits);delete m.integer;a.deepEqual(m,parent);return m;
}
function readRecord(fields,input,fixed,ambientD,ordinal){
 a.equal(fields.length,44);const f=fields,p=input.controls[fixed];a(p);
 a.equal(f[0],fixed);a.equal(f[1],String(ordinal));a.equal(f[2],input.bits);a.equal(f[3],'15');
 a.equal(f[32],String(input.index));a.equal(f[33],String(ambientD));
 for(const i of [4,18,38,40])meta(f[i],p.originalMeta,input.bits);
 for(const i of [5,19,39,41])a.equal(f[i],input.bits);
 for(const i of [6,20])a.equal(f[i],p.originalDisplayHex);
 for(const i of [7,21])a.equal(f[i],p.originalStrHex);
 a.equal(f[8],'N');a.equal(f[10],'C');a.equal(f[34],'N');a.equal(f[36],'C');
 const decimal=h=>{const text=ascii(h);a.match(text,/^\s*\d+(?:\.0+)?\s*$/);return Number(text);};
 a.equal(decimal(f[9]),15);a.equal(decimal(f[35]),ambientD);a.equal(f[42],f[35]);
 for(const i of [11,37,43])a.equal(ascii(f[i]),fixed);
 for(const [i,k]of [[12,'crt'],[22,'crt'],[13,'gcvt'],[23,'gcvt']])
  a.equal(f[i],Object.entries(p[k]).map(([key,value])=>key+'='+value).join(';'));
 for(const i of [14,16,24,26,28,30])a.equal(f[i],'C');
 for(const i of [15,17,25,27,29,31])a.equal(ascii(f[i]).length,40);
 a.equal(f[15],f[29]);a.equal(f[17],f[31]);
 // New ambient STR strings are observations, never compared to invented text.
 return {index:input.index,bits:input.bits,fixed,ambientD,originalSetting:15,
  str15Hex:f[15],str16Hex:f[25],str17Hex:f[27],str18Hex:f[17],fields:[...f]};
}
module.exports={readRecord};
