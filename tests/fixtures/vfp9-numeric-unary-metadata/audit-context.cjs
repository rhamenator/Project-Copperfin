// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native structural/repeat audit, not a general unary implementation oracle.
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2],source=fs.readFileSync(process.argv[3]||path.join(__dirname,'context-probe.prg'),'utf8');
const expressions=new Map();
for(const line of source.split('\n').filter(l=>l.startsWith('expressions['))) {
 const m=line.match(/^expressions\[(\d+)\]='([^']*)'\r?$/);assert(m,line);
 const index=Number(m[1]),e=m[2];assert(!expressions.has(index));expressions.set(index,e);
 assert(source.includes(' CASE index='+index+'\n  RETURN CFNMETA('+e+")+'|'+HexBytes(BINTOC("+e+",'BRS'))+'|'+HexBytes(TRANSFORM("+e+'))'));
}
assert.equal(expressions.size,Number(source.match(/expressions\[(\d+)\], settings/)[1]));
function capture(name) {
 assert.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');
 assert.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
 const raw=fs.readFileSync(path.join(root,name+'-results.txt'));assert.deepEqual(raw,fs.readFileSync(path.join(root,name+'.out')));
 const lines=raw.toString('ascii').split('\r\n');assert.equal(lines.pop(),'');
 assert.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
 assert.equal(lines.pop(),'DONE|'+expressions.size*8);assert.equal(lines.length,expressions.size*8);
 const rows=new Map();let errors=0;
 for(const line of lines) {
  const f=line.split('|'),s=Number(f[1]),fixed=f[2],index=Number(f[3]);assert([2,14].includes(s));assert(['ON','OFF'].includes(fixed));
  const mode=f[0].replace(/^ERROR-/,''),expression=Buffer.from(f[4],'hex').toString('ascii');
  assert(['EVAL','COMPILED'].includes(mode));assert.equal(expression,expressions.get(index));
  const key=JSON.stringify([s,fixed,index,mode]);assert(!rows.has(key));
  if(f[0].startsWith('ERROR-')){assert.equal(f.length,7);assert.match(f[5],/^\d+$/);assert.match(f[6],/^(?:[0-9A-F]{2})+$/);++errors;rows.set(key,{expression,error:Number(f[5]),message:f[6]});continue;}
  assert.equal(f.length,8);assert.match(f[5],/^type=[0-9A-F]{2};width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=[0-9A-F]{4}$/);
  const m=Object.fromEntries(f[5].split(';').map(s=>s.split('=')));assert.equal(m.type,'4E');assert.equal(m.cw,'027F');
  assert.match(f[6],/^[0-9A-F]{16}$/);assert.equal(m.real,f[6]);assert.match(f[7],/^(?:[0-9A-F]{2})+$/);
  rows.set(key,{expression,type:m.type,width:parseInt(m.width,16),decimals:parseInt(m.decimals,16),bits:f[6],display:f[7],cw:m.cw});
 }
 for(const s of [2,14])for(const fixed of ['OFF','ON'])for(const index of expressions.keys())for(const mode of ['EVAL','COMPILED'])assert(rows.has(JSON.stringify([s,fixed,index,mode])));
 return {raw,rows,errors};
}
const first=capture('context-first'),repeat=capture('context-repeat');assert.deepEqual(first.rows,repeat.rows);
const hash=b=>crypto.createHash('sha256').update(b).digest('hex'),differences=[];
for(const s of [2,14])for(const fixed of ['OFF','ON'])for(const index of expressions.keys()) {
 const evalRow=first.rows.get(JSON.stringify([s,fixed,index,'EVAL'])),compiledRow=first.rows.get(JSON.stringify([s,fixed,index,'COMPILED']));
 if(JSON.stringify(evalRow)!==JSON.stringify(compiledRow))differences.push({s,fixed,index,evalRow,compiledRow});
}
process.stdout.write(JSON.stringify({casesPerRun:first.rows.size,successfulValues:first.rows.size-first.errors,recordedErrors:first.errors,
 documentedFieldsRepeatEqual:true,rawByteEqual:first.raw.equals(repeat.raw),firstSha256:hash(first.raw),repeatSha256:hash(repeat.raw),
 compiledVersusEvalDifferences:differences,samples:[...expressions.keys()].map(index=>({index,...first.rows.get(JSON.stringify([2,'OFF',index,'EVAL']))}))},null,2)+'\n');
