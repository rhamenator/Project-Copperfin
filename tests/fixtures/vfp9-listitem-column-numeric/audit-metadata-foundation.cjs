// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Structural/repeat audit of native observations, not product acceptance expectations.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const root = process.argv[2];
const sha = data => crypto.createHash('sha256').update(data).digest('hex');
const operands = ['1','1.0','1.00','001.2300','0','0.0000','-0.00','-12.3400',
  '1E-14','1.00E2','1.23E-2','1-1E-14','1-90*2^(-53)','2.5','5/2','123456789.123456'];
function decode(text) {
  assert.match(text,/^type=[0-9A-F]{2};width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=[0-9A-F]{4}$/);
  const m = Object.fromEntries(text.split(';').map(s => s.split('=')));
  assert(['4E','49'].includes(m.type));
  assert.equal(m.cw,'027F');
  // Only these fields are documented for each type. Unused slots are retained
  // verbatim in raw evidence, never treated as Numeric precision or payload.
  return m.type === '4E' ? {type:m.type,width:m.width,decimals:m.decimals,real:m.real,cw:m.cw}
    : {type:m.type,width:m.width,integer:m.integer,cw:m.cw};
}
function read(run) {
  const raw = fs.readFileSync(path.join(root,`metadata-foundation-${run}.txt`));
  assert.equal(fs.readFileSync(path.join(root,`metadata-foundation-${run}.status`),'utf8'),'0\n');
  assert.equal(fs.statSync(path.join(root,`metadata-foundation-${run}.stderr`)).size,0);
  assert.deepEqual(raw,fs.readFileSync(path.join(root,`metadata-foundation-${run}.stdout`)));
  const lines = raw.toString('ascii').split('\r\n');
  assert.equal(lines.pop(),'');
  assert.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
  const footer=lines.pop();
  // The probe deliberately leaves FIXED ON / DECIMALS 14 active, so its own
  // TRANSFORM(total) is formatted too. Validate framing and exact numeric count.
  assert.match(footer,/^DONE\|96(?:\.0+)?$/);
  assert.equal(Number(footer.slice(5)),96);
  assert.equal(lines.length,96);
  const rows = new Map();
  for(const line of lines) {
    const f = line.split('|');
    assert.equal(f.length,18); assert.equal(f[0],'ROW');
    const decimals=Number(f[1]), fixed=Number(f[2]), operand=f[3];
    assert([2,6,14].includes(decimals)); assert([0,1].includes(fixed)); assert(operands.includes(operand));
    const meta=f.slice(4,10).map(decode), bits=f.slice(10,15);
    for(const b of bits) { assert.match(b,/^[0-9A-F]{16}$/); assert.equal(b,bits[0]); }
    for(let i=0;i<meta.length;i++) {
      const m=meta[i];
      if(m.type==='4E') assert.equal(m.real,bits[0]);
      else assert.equal(Buffer.from(bits[0],'hex').readDoubleBE(),parseInt(m.integer,16)|0);
    }
    const displays=f.slice(15).map(s=>{assert.match(s,/^(?:[0-9A-F]{2})+$/);return Buffer.from(s,'hex').toString('ascii');});
    assert.equal(displays[2].length,30);
    const key=JSON.stringify([decimals,fixed,operand]); assert(!rows.has(key));
    rows.set(key,{meta,bits,displays});
  }
  for(const d of [2,6,14]) for(const f of [0,1]) for(const op of operands)
    assert(rows.has(JSON.stringify([d,f,op])));
  return {raw,rows};
}
const first=read('first'), repeat=read('repeat');
assert.deepEqual(first.rows,repeat.rows);
const get=op=>first.rows.get(JSON.stringify([2,0,op]));
// Summarize observations rather than using them as Copperfin expected outputs.
const summary={readsPerRun:first.rows.size,documentedFieldsRepeatEqual:true,
  rawByteEqual:first.raw.equals(repeat.raw),firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),
  fractionDisplay:{literal:get('2.5').displays[1],division:get('5/2').displays[1],
    sameBits:get('2.5').bits[0]===get('5/2').bits[0],
    literalDecimals:parseInt(get('2.5').meta[1].decimals,16),divisionDecimals:parseInt(get('5/2').meta[1].decimals,16)},
  nearOneDisplay:{scientific:get('1-1E-14').displays[1],power:get('1-90*2^(-53)').displays[1]},
  literalWidth:{direct:parseInt(get('1.00').meta[0].width,16),assigned:parseInt(get('1.00').meta[1].width,16)}};
process.stdout.write(JSON.stringify(summary,null,2)+'\n');
