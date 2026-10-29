// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: strict public observation parser.
'use strict';
const a=require('node:assert/strict');
function ascii(hex){a.match(hex,/^(?:[0-9A-F]{2})*$/);const b=Buffer.from(hex,'hex');a(b.every(x=>x>0&&x<128));return b.toString('ascii');}
function fields(value){const entries=value.split(';').map(part=>{const i=part.indexOf('=');a(i>0);return[part.slice(0,i),part.slice(i+1)];});const out=Object.fromEntries(entries);a.equal(Object.keys(out).length,entries.length);return out;}
function identity(b,bits,inv){
 a.equal(b.real,bits);a.equal(b.cw0,'027F');a.equal(b.cw1,'027F');a.equal(b.fileSample,'ONCE');
 a.equal(ascii(b.module).toLowerCase(),inv.modulePath.toLowerCase());a.equal(b.fileSha256.toLowerCase(),inv.moduleSha256);a.equal(parseInt(b.fileSize,16),inv.moduleFileSize);
 for(const [k,v]of Object.entries(inv.modulePe))a.equal(parseInt(b[k],16),v);
}
function meta(value,bits){
 a.match(value,/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
 const m=fields(value);a.equal(m.real,bits);a.notEqual((BigInt('0x'+bits)>>52n)&2047n,2047n);delete m.integer;return m;
}
function control(value,bits,inv){
 a.match(value,/^kind=F;count=00000011;real=[0-9A-F]{16};decimal=[0-9A-F]{8};sign=[0-9A-F]{8};cw0=027F;cw1=027F;module=(?:[0-9A-F]{2})+;fileSample=ONCE;fileSize=[0-9A-F]{8};fileSha256=[0-9A-F]{64};machine=[0-9A-F]{4};magic=[0-9A-F]{4};stamp=[0-9A-F]{8};imageSize=[0-9A-F]{8};checksum=[0-9A-F]{8};digits=(?:3[0-9])*$/);
 const b=fields(value);identity(b,bits,inv);return b;
}
function crt(value,bits,count,inv){
 a.match(value,/^kind=S;count=[0-9A-F]{8};real=[0-9A-F]{16};cw0=027F;cw1=027F;module=(?:[0-9A-F]{2})+;fileSample=ONCE;fileSize=[0-9A-F]{8};fileSha256=[0-9A-F]{64};machine=[0-9A-F]{4};magic=[0-9A-F]{4};stamp=[0-9A-F]{8};imageSize=[0-9A-F]{8};checksum=[0-9A-F]{8};capacity=00000800;canary=PASS;sprintfGrammar=[01];snprintfGrammar=[01];equal=[01];sprintf=(?:[0-9A-F]{2})+;snprintf=(?:[0-9A-F]{2})+$/);
 const b=fields(value);identity(b,bits,inv);a.equal(parseInt(b.count,16),count);
 const sprintfText=ascii(b.sprintf),snprintfText=ascii(b.snprintf);a(sprintfText.length<384);a(snprintfText.length<384);
 const grammar=text=>{const m=/^[+-]?[0-9]+\.([0-9]+)$/.exec(text);return Number(!!m&&m[1].length===count);};
 a.equal(Number(b.sprintfGrammar),grammar(sprintfText));a.equal(Number(b.snprintfGrammar),grammar(snprintfText));
 a.equal(Number(b.equal),Number(sprintfText===snprintfText));return {...b,sprintfText,snprintfText};
}
function paddedHex(text){return Buffer.from(text.length<=40?text.padStart(40,' '):text,'ascii').toString('hex').toUpperCase();}
function readRecord(line,row,count,inv){
 const f=line.split('|');a.equal(f.length,18);a.equal(f[0],String(row.index));a.equal(f[1],row.bits);a.equal(f[2],String(row.setting));a.equal(f[3],String(count));
 a.deepEqual(meta(f[4],row.bits),meta(f[9],row.bits));a.equal(f[5],row.bits);a.equal(f[10],row.bits);a.equal(f[6],f[11]);
 const expectedOff=row.expectedStrHex[String(count)],expectedOn=row.expectedStrHex[String(count)];a.equal(f[6],expectedOff);
 const before=control(f[7],row.bits,inv),parsed=crt(f[8],row.bits,count,inv);a.deepEqual(control(f[12],row.bits,inv),before);
 a.equal(f[13],'N');const decimalText=ascii(f[14]);a.match(decimalText,/^\s*[0-9]+(?:\.0+)?\s*$/);a.equal(Number(decimalText),row.setting);a.equal(f[15],'C');a.equal(ascii(f[16]),'OFF');a.equal(f[17],'OFF');
 const actual={sprintf:paddedHex(parsed.sprintfText),snprintf:paddedHex(parsed.snprintfText)},mismatches=[];
 for(const [name,hex]of Object.entries(actual))for(const [fixed,expected]of [['OFF',expectedOff],['ON',expectedOn]])if(hex!==expected)mismatches.push({export:name,fixed,expectedHex:expected,actualHex:hex,rawText:parsed[name+'Text'],overWidth:parsed[name+'Text'].length>40});
 return {index:row.index,bits:row.bits,setting:row.setting,count,meta:meta(f[4],row.bits),freshStrHex:f[6],control:before,crt:parsed,actual,mismatches,fields:f};
}
module.exports={ascii,fields,identity,meta,control,crt,paddedHex,readRecord};
