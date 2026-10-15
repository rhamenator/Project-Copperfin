// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Structural native recovery audit, not a Copperfin acceptance oracle.
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2];
const source=fs.readFileSync(path.join(__dirname,'literal-probe.prg'),'utf8');
const expressions=[...source.matchAll(/^expressions\[\d+\]='([^']+)'$/gm)].map(x=>x[1]);
assert.equal(expressions.length,48); assert.equal(new Set(expressions).size,48);
function metadata(text) {
 assert.match(text,/^type=[0-9A-F]{2};width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=[0-9A-F]{4}$/);
 const m=Object.fromEntries(text.split(';').map(s=>s.split('=')));
 assert(['4E','49'].includes(m.type)); assert.equal(m.cw,'027F');
 return m.type==='4E'?{type:m.type,width:m.width,decimals:m.decimals,real:m.real,cw:m.cw}
  :{type:m.type,width:m.width,integer:m.integer,cw:m.cw};
}
function capture(run) {
 const raw=fs.readFileSync(path.join(root,`${run}-results.txt`));
 assert.equal(fs.readFileSync(path.join(root,`${run}.status`),'utf8'),'0\n');
 assert.equal(fs.statSync(path.join(root,`${run}.stderr`)).size,0);
 assert.deepEqual(raw,fs.readFileSync(path.join(root,`${run}.out`)));
 const lines=raw.toString('ascii').split('\r\n');assert.equal(lines.pop(),'');
 assert.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
 assert.equal(lines.pop(),'DONE|144');assert.equal(lines.length,144);
 const rows=new Map();
 for(const line of lines) {
  const f=line.split('|');assert.equal(f.length,9);assert.equal(f[0],'LITERAL');
  const setting=Number(f[1]),expression=f[2];assert([2,6,14].includes(setting));assert(expressions.includes(expression));
  const direct=metadata(f[3]),assigned=metadata(f[4]);
  assert.match(f[5],/^[0-9A-F]{16}$/);assert.equal(f[6],f[5]);
  for(const m of [direct,assigned]) {
   if(m.type==='4E')assert.equal(m.real,f[5]);
   else assert.equal(Buffer.from(f[5],'hex').readDoubleBE(),parseInt(m.integer,16)|0);
  }
  for(const text of [f[7],f[8]])assert.match(text,/^(?:[0-9A-F]{2})+$/);
  const key=JSON.stringify([setting,expression]);assert(!rows.has(key));
  rows.set(key,{direct,assigned,bits:f[5],directDisplay:Buffer.from(f[7],'hex').toString('ascii'),assignedDisplay:Buffer.from(f[8],'hex').toString('ascii')});
 }
 for(const s of [2,6,14])for(const e of expressions)assert(rows.has(JSON.stringify([s,e])));
 return {raw,rows};
}
const first=capture('first'),repeat=capture('repeat');assert.deepEqual(first.rows,repeat.rows);
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const samples=['1.00','001.2300','0.0000000000000000001','1E-14','1.00E2','1E-300'];
process.stdout.write(JSON.stringify({observationsPerRun:first.rows.size,documentedFieldsRepeatEqual:true,
 rawByteEqual:first.raw.equals(repeat.raw),firstSha256:hash(first.raw),repeatSha256:hash(repeat.raw),
 samples:samples.map(expression=>({expression,...first.rows.get(JSON.stringify([2,expression]))}))},null,2)+'\n');
