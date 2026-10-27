// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: independent exact-rational readback.
'use strict';
const fs=require('node:fs'),a=require('node:assert/strict'),z=require('node:zlib'),path=require('node:path');
const report=JSON.parse(fs.readFileSync(process.argv[2]));
const read=n=>JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,n,'audit.json.gz')))).observations;
const str=read('str-middle-captures1'),old=read('crt-nativeonly-captures1'),more=read('crt-precision-captures1');
// Compare rationals by cross multiplication, independently of the analyzer's
// normalized decimal-coefficient representation. No binary floating conversion.
function fraction(coefficient,exponent) {
  const e=BigInt(exponent);a(e>=-2048n&&e<=2048n);
  return e<0n?[BigInt(coefficient),10n**(-e)]:[BigInt(coefficient)*10n**e,1n];
}
function parse(text) {
  const m=text.trim().match(/^([+-]?)(\d+)(?:\.(\d*))?(?:[eE]([+-]?\d+))?$/);a(m);
  const f=m[3]||'';return fraction((m[1]==='-'?'-':'')+m[2]+f,BigInt(m[4]||'0')-BigInt(f.length));
}
const seen=new Set();
for(const r of report.rows) {
  const key=JSON.stringify([r.index,r.fixed,r.bits,r.setting,r.count]);a(!seen.has(key));seen.add(key);
  a.equal(r.offset,r.count-r.setting);a.equal(Buffer.from(r.strHex,'hex').toString('ascii'),r.strText);
  a.equal(Buffer.from(r.nativeControl.digits,'hex').toString('ascii'),r.crtDigits);
  const s=str.find(x=>x.index===r.index&&x.fixed===r.fixed);a(s);a.equal(s.bits,r.bits);
  a.equal(s.setting,r.setting);a.equal(s['str'+r.count+'Hex'],r.strHex);
  const source=r.offset===0?old:more;
  a.equal(r.source,r.offset===0?'crt-nativeonly-captures1':'crt-precision-captures1');
  const actual=source.filter(x=>x.route==='ORIGINAL'&&x.kind==='F'&&x.index===r.index&&
    x.fixed===r.fixed&&x.bits===r.bits&&x.setting===r.setting&&x.count===r.count);
  a.equal(actual.length,1);a.deepEqual(actual[0].crt,r.nativeControl);
  let different=null;
  let zeroSign=null;
  if(r.crtDigits!=='') {
    let point=BigInt('0x'+r.nativeControl.decimal);if(point>=0x80000000n)point-=0x100000000n;
    const sign=BigInt('0x'+r.nativeControl.sign)!==0n?'-':'';
    a.equal(r.crtPoint,String(point));a.equal(r.crtNegative,sign==='-');
    const x=parse(r.strText),y=fraction(sign+r.crtDigits,point-BigInt(r.crtDigits.length));
    different=x[0]*y[1]!==y[0]*x[1];
    if(x[0]===0n&&y[0]===0n)zeroSign=r.strText.trim().startsWith('-')!==(sign==='-');
  }
  a.equal(r.exactValueDifferent,different);
  a.equal(r.zeroSignDifferent,zeroSign);
}
a.equal(report.available,report.rows.length);
for(const g of report.groups) {
  const rows=report.rows.filter(r=>r.offset===g.offset&&r.fixed===g.fixed);
  a.equal(g.contexts,rows.length);a.equal(g.emptyReturns,rows.filter(r=>r.crtDigits==='').length);
  a.equal(g.exactValueDifferent,rows.filter(r=>r.exactValueDifferent===true).length);
  a.equal(g.zeroSignDifferent,rows.filter(r=>r.zeroSignDifferent===true).length);
}
console.log(JSON.stringify({independentRationalReadback:true,rows:seen.size,groups:report.groups,nativeInvoked:false}));
