// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Structural/type-aware repeat audit; not a Copperfin acceptance oracle.
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2],source=fs.readFileSync(process.argv[3]||path.join(__dirname,'unary-probe.prg'),'utf8');
const expressions=new Map();
for(const line of source.split('\n').filter(l=>l.startsWith('expressions['))) {
 const m=line.match(/^expressions\[(\d+)\]='([^']*)'(?:\+REPLICATE\('0',(\d+)\)(?:\+'([^']*)')?)?\r?$/);
 assert(m,'unsupported probe source expression: '+line);assert(!expressions.has(Number(m[1])));
 expressions.set(Number(m[1]),m[2]+(m[3]?'0'.repeat(Number(m[3])):'')+(m[4]||''));
}
assert.equal(expressions.size,Number(source.match(/expressions\[(\d+)\], settings/)[1]));
function meta(text) {
 assert.match(text,/^type=[0-9A-F]{2};width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=[0-9A-F]{4}$/);
 const m=Object.fromEntries(text.split(';').map(s=>s.split('=')));
 assert(['4E','49'].includes(m.type));assert.equal(m.cw,'027F');
 return m.type==='4E'?{type:m.type,width:m.width,decimals:m.decimals,real:m.real,cw:m.cw}
  :{type:m.type,width:m.width,integer:m.integer,cw:m.cw};
}
function capture(name) {
 assert.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');
 assert.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
 const raw=fs.readFileSync(path.join(root,name+'-results.txt'));assert.deepEqual(raw,fs.readFileSync(path.join(root,name+'.out')));
 const lines=raw.toString('ascii').split('\r\n');assert.equal(lines.pop(),'');
 assert.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
 assert.equal(lines.pop(),'DONE|'+expressions.size*4);assert.equal(lines.length,expressions.size*4);
 const rows=new Map();let errors=0;
 for(const line of lines) {
  const f=line.split('|'),s=Number(f[1]),fixed=f[2],index=Number(f[3]);assert([2,14].includes(s));assert(['ON','OFF'].includes(fixed));
  const expression=Buffer.from(f[4],'hex').toString('ascii');assert.equal(expression,expressions.get(index));
  const key=JSON.stringify([s,fixed,index]);assert(!rows.has(key));
  if(f[0]==='ERROR') {assert.equal(f.length,7);assert.match(f[5],/^\d+$/);assert.match(f[6],/^(?:[0-9A-F]{2})+$/);++errors;rows.set(key,{expression,error:Number(f[5]),message:f[6]});continue;}
  assert.equal(f[0],'UNARY');assert.equal(f.length,11);const direct=meta(f[5]),assigned=meta(f[6]);
  assert.match(f[7],/^[0-9A-F]{16}$/);assert.equal(f[7],f[8]);
  for(const m of [direct,assigned])if(m.type==='4E')assert.equal(m.real,f[7]);
   else assert.equal(Buffer.from(f[7],'hex').readDoubleBE(),parseInt(m.integer,16)|0);
  for(const text of [f[9],f[10]])assert.match(text,/^(?:[0-9A-F]{2})+$/);
  rows.set(key,{expression,direct,assigned,bits:f[7],directDisplay:f[9],assignedDisplay:f[10]});
 }
 for(const s of [2,14])for(const fixed of ['OFF','ON'])for(const index of expressions.keys())assert(rows.has(JSON.stringify([s,fixed,index])));
 return {raw,rows,errors};
}
const first=capture('corrected-first'),repeat=capture('corrected-repeat');assert.deepEqual(first.rows,repeat.rows);
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
process.stdout.write(JSON.stringify({casesPerRun:first.rows.size,successfulValues:first.rows.size-first.errors,recordedErrors:first.errors,
 documentedFieldsRepeatEqual:true,rawByteEqual:first.raw.equals(repeat.raw),firstSha256:hash(first.raw),repeatSha256:hash(repeat.raw),
 samples:[2,10,17,19,24,27,29,38].map(index=>first.rows.get(JSON.stringify([2,'OFF',index])))},null,2)+'\n');
