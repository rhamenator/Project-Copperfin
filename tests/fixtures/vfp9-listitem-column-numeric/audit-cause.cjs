// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Audits native diagnostic interventions, NOT a Copperfin conversion acceptance table.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const root = process.argv[2];
const metadata = text => Object.fromEntries(text.split(';').map(part => part.split('=')));
const documented = m => ({type:m.type, width:m.width, decimals:m.decimals, real:m.real, cw:m.cw});
const noDecimals = m => ({type:m.type, width:m.width, real:m.real, cw:m.cw});
const doubleFromBits = hex => Buffer.from(hex, 'hex').readDoubleBE(0);
function modeledOutcome(row) {
  const value = doubleFromBits(row.argument.real);
  const decimalPlaces = Number.parseInt(row.argument.decimals, 16);
  assert(decimalPlaces <= 18, 'retained diagnostic decimal range');
  // All retained operands are positive. toFixed performs decimal
  // quantization; Number then removes presentation-only trailing zeroes.
  const quantized = Number(value.toFixed(decimalPlaces + 2));
  const column = Math.trunc(quantized);
  return column >= 1 && column <= 0x7fffffff
    ? `0/C:id${row.id}c${column}` : '31/<not-called>';
}
function capture(phase, count) {
  const first = fs.readFileSync(path.join(root, `${phase}-first-results.txt`));
  assert.deepEqual(first, fs.readFileSync(path.join(root, `${phase}-repeat-results.txt`)));
  for (const run of ['first', 'repeat']) {
    assert.equal(fs.readFileSync(path.join(root, `${phase}-${run}.status`), 'utf8'), '0\n');
    assert.equal(fs.statSync(path.join(root, `${phase}-${run}.stderr`)).size, 0);
    assert.deepEqual(first, fs.readFileSync(path.join(root, `${phase}-${run}.out`)));
  }
  const lines = first.toString('utf8').split('\r\n');
  assert.equal(lines.pop(), '');
  assert.equal(lines.shift(), 'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
  assert.equal(lines.pop(), `DONE|${count}`);
  assert.equal(lines.length, count);
  const rows = new Map();
  for (const line of lines) {
    const f = line.split('|');
    assert.equal(f.length, 19);
    const [tag,kind,id,operand,variant,originalText,argumentText,bits,argBits,stage,
      error,message,returned,afterText,afterBits,context,afterContext,before,after] = f;
    assert.equal(tag,'CAUSE');
    assert(['ListBox','ComboBox'].includes(kind) && ['11','22'].includes(id));
    assert.equal(stage,'invoke');
    const original=metadata(originalText), argument=metadata(argumentText), originalAfter=metadata(afterText);
    assert.equal(original.type,'4E'); assert.equal(argument.type,'4E');
    assert.equal(original.cw,'027F'); assert.equal(argument.cw,'027F');
    assert.equal(original.real,bits); assert.equal(argument.real,bits);
    assert.equal(argBits,bits); assert.equal(afterBits,bits);
    // ev_long is unused for Numeric according to the documented interface;
    // retain it in raw evidence but do not interpret it as Numeric data.
    assert.deepEqual(documented(originalAfter),documented(original));
    assert.equal(context,'1;;1'); assert.equal(afterContext,context); assert.equal(before,after);
    assert(before.startsWith('count=3;columns=9;rst=N:0;binding=C:[];newid=33;newindex=3;index=2;itemid=22;'));
    const key=JSON.stringify([kind,id,operand,variant]); assert(!rows.has(key));
    if(error==='0') { assert.equal(message,''); assert(returned.startsWith('C:')); }
    else { assert.equal(error,'31'); assert.equal(message,'Invalid subscript reference.'); assert.equal(returned,'<not-called>'); }
    rows.set(key,{kind,id,operand,variant,original,argument,error,returned});
  }
  return {rows,sha256:crypto.createHash('sha256').update(first).digest('hex')};
}
const factorial=capture('factorial',748), settings=capture('settings',160);
const lookup=(set,kind,id,op,v)=>{const row=set.rows.get(JSON.stringify([kind,id,op,v])); assert(row); return row;};
const outcome=r=>`${r.error}/${r.returned}`;
let decimalPairs=0,widthControls=0,cloneControls=0;
for(const r of factorial.rows.values()) {
  const v=r.variant, d=Number.parseInt(v.replace(/^dec/,''),10);
  const expected={...documented(r.original)};
  if(v.startsWith('width')) expected.width=Number(v.slice(5)).toString(16).toUpperCase().padStart(4,'0');
  else if(v.startsWith('dec')) expected.decimals=d.toString(16).toUpperCase().padStart(8,'0');
  else if(/^d(2|14)w(25|30)$/.test(v)) {
    const [,dec,width]=v.match(/^d(2|14)w(25|30)$/);
    expected.decimals=Number(dec).toString(16).toUpperCase().padStart(8,'0');
    expected.width=Number(width).toString(16).toUpperCase().padStart(4,'0');
  } else assert(['raw','clone'].includes(v));
  assert.deepEqual(documented(r.argument),expected,'actual returned descriptor matches intended intervention');
  const raw=lookup(factorial,r.kind,r.id,r.operand,'raw');
  if(v==='clone') {assert.equal(outcome(r),outcome(raw)); ++cloneControls;}
  if(v.startsWith('width')) {assert.equal(outcome(r),outcome(raw)); ++widthControls;}
  assert.equal(outcome(r),modeledOutcome(r),
    'round to value decimal places plus two, then truncate');
}
for(const r of settings.rows.values()) assert.equal(outcome(r),modeledOutcome(r),
  'settings row follows attached-metadata conversion model');
for(const kind of ['ListBox','ComboBox']) for(const id of ['11','22']) {
  const decimal=lookup(factorial,kind,id,'1-1E-14','raw');
  const power=lookup(factorial,kind,id,'1-90*2^(-53)','raw');
  assert.equal(decimal.original.real,'3FEFFFFFFFFFFFA6');
  assert.equal(power.original.real,decimal.original.real);
  assert.equal(decimal.original.decimals,'0000000E'); assert.equal(power.original.decimals,'00000002');
  assert.equal(decimal.error,'31'); assert.equal(power.returned,`C:id${id}c1`);
  for(const op of ['1-1E-14','1-90*2^(-53)','2-1E-14','2-45*2^(-52)','9-1E-12','9-563*2^(-49)'])
    for(const width of [25,30]) {
      const low=lookup(factorial,kind,id,op,`d2w${width}`), high=lookup(factorial,kind,id,op,`d14w${width}`);
      assert.deepEqual(noDecimals(low.argument),noDecimals(high.argument));
      assert.notEqual(outcome(low),outcome(high)); ++decimalPairs;
    }
  for(const op of ['1-1E-14','1-90*2^(-53)','2.5','5/2'])
    for(const [evalDec,readDec] of [[0,18],[2,18],[6,18],[14,0],[18,0]]) {
      const unchanged=lookup(settings,kind,id,op,`${evalDec}/${evalDec}`);
      const changed=lookup(settings,kind,id,op,`${evalDec}/${readDec}`);
      assert.deepEqual(documented(unchanged.argument),documented(changed.argument));
      assert.equal(outcome(unchanged),outcome(changed));
      assert.deepEqual(documented(changed.argument),documented(changed.original));
      if(op==='2.5'||op==='5/2') assert.equal(changed.returned,`C:id${id}c2`);
      if(op==='1-90*2^(-53)') {
        assert.equal(Number.parseInt(changed.argument.decimals,16),evalDec);
        assert.equal(changed.error,evalDec>=14?'31':'0');
      }
    }
}
assert.equal(decimalPairs,48); assert.equal(widthControls,88); assert.equal(cloneControls,44);
console.log(JSON.stringify({nativeDiagnosticOnly:true,factorialRows:748,settingsRows:160,
  sameBitsSameWidthSameCWDecimalOnlyOutcomePairs:decimalPairs,widthOnlyControls:widthControls,
  cloneControls,factorialRawSHA256:factorial.sha256,settingsRawSHA256:settings.sha256,
  conversionModel:'round the Numeric value to its attached decimal-place count plus two, then truncate',
  conclusion:'The conversion model explains every retained factorial and settings row; display width and current SET DECIMALS at read time do not explain the controlled outcomes. Product wiring remains separately gated.'}));
