// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Structural/type-aware observation audit, NOT an inferred precision oracle.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2],source=fs.readFileSync(process.argv[3]||path.join(__dirname,'division-probe.prg'),'utf8');
const expressions=new Map();
for(const line of source.split('\n').filter(l=>l.startsWith('expressions['))) {
 const m=line.match(/^expressions\[(\d+)\]='([^']*)'\r?$/);a(m,line);
 const index=Number(m[1]),e=m[2];a(!expressions.has(index));expressions.set(index,e);
 a(source.includes(' CASE index='+index+'\n  RETURN CFNMETA('+e+")+'|'+HexBytes(BINTOC("+e+",'BRS'))+'|'+HexBytes(TRANSFORM("+e+'))'));
}
const declared=Number(source.match(/expressions\[(\d+)\], settings/)[1]);
a([48,81].includes(declared));a.equal(expressions.size,declared);const expectedCount=declared*8;
function descriptor(text,bits) {
 a.match(text,/^type=[0-9A-F]{2};width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=[0-9A-F]{4}$/);
 const m=Object.fromEntries(text.split(';').map(s=>s.split('=')));a(['4E','49'].includes(m.type));a.equal(m.cw,'027F');
 if(m.type==='4E') {a.equal(m.real,bits);return {type:'N',width:parseInt(m.width,16),decimals:parseInt(m.decimals,16),bits,cw:m.cw};}
 const integer=parseInt(m.integer,16)|0;a.equal(Buffer.from(bits,'hex').readDoubleBE(),integer);
 return {type:'I',width:parseInt(m.width,16),integer,bits,cw:m.cw}; // unused real/decimal slots deliberately excluded
}
function capture(name) {
 a.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');a.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
 const raw=fs.readFileSync(path.join(root,name+'-results.txt'));a.deepEqual(raw,fs.readFileSync(path.join(root,name+'.out')));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|'+expectedCount);a.equal(lines.length,expectedCount);
 const rows=new Map();let errors=0;const types={I:0,N:0};
 for(const line of lines) {
  const f=line.split('|'),decimals=Number(f[1]),fixed=f[2],index=Number(f[3]),mode=f[0].replace(/^ERROR-/,'');
  a([2,14].includes(decimals));a(['ON','OFF'].includes(fixed));a(['EVAL','COMPILED'].includes(mode));
  const expression=Buffer.from(f[4],'hex').toString('ascii');a.equal(expression,expressions.get(index));
  const key=JSON.stringify([decimals,fixed,index,mode]);a(!rows.has(key));
  if(f[0].startsWith('ERROR-')) {a.equal(f.length,7);a.match(f[5],/^\d+$/);a.match(f[6],/^(?:[0-9A-F]{2})+$/);++errors;rows.set(key,{expression,error:Number(f[5]),message:f[6]});continue;}
  a.equal(f.length,8);a.match(f[6],/^[0-9A-F]{16}$/);a.match(f[7],/^(?:[0-9A-F]{2})+$/);
  const d=descriptor(f[5],f[6]);++types[d.type];rows.set(key,{expression,...d,display:Buffer.from(f[7],'hex').toString('ascii')});
 }
 for(const d of [2,14])for(const fixed of ['OFF','ON'])for(const index of expressions.keys())for(const mode of ['EVAL','COMPILED'])a(rows.has(JSON.stringify([d,fixed,index,mode])));
 return {raw,rows,errors,types};
}
const first=capture('first'),repeat=capture('repeat');a.deepEqual(first.rows,repeat.rows);
// Raw repeat is NOT required for fields that the published tagged interface
// does not define. Prove every byte difference is confined to those slots.
const rawDifferences={};const firstLines=first.raw.toString('ascii').split('\r\n'),repeatLines=repeat.raw.toString('ascii').split('\r\n');
a.equal(firstLines.length,repeatLines.length);
for(let i=0;i<firstLines.length;++i)if(firstLines[i]!==repeatLines[i]) {
 const x=firstLines[i].split('|'),y=repeatLines[i].split('|');a.equal(x.length,8);a.equal(y.length,8);
 for(let j=0;j<8;++j)if(j!==5)a.equal(x[j],y[j]);
 const mx=Object.fromEntries(x[5].split(';').map(s=>s.split('='))),my=Object.fromEntries(y[5].split(';').map(s=>s.split('=')));
 for(const key of Object.keys(mx))if(mx[key]!==my[key]) {
  a((mx.type==='49'&&['decimals','real'].includes(key))||(mx.type==='4E'&&key==='integer'));
  const label=mx.type+':'+key;rawDifferences[label]=(rawDifferences[label]||0)+1;
 }
}
const hash=b=>crypto.createHash('sha256').update(b).digest('hex'),differences=[];
for(const decimals of [2,14])for(const fixed of ['OFF','ON'])for(const index of expressions.keys()) {
 const evalRow=first.rows.get(JSON.stringify([decimals,fixed,index,'EVAL'])),compiledRow=first.rows.get(JSON.stringify([decimals,fixed,index,'COMPILED']));
 if(JSON.stringify(evalRow)!==JSON.stringify(compiledRow))differences.push({decimals,fixed,index,evalRow,compiledRow});
}
process.stdout.write(JSON.stringify({casesPerRun:first.rows.size,successfulValues:first.rows.size-first.errors,recordedErrors:first.errors,typeCounts:first.types,documentedFieldsRepeatEqual:true,rawByteEqual:first.raw.equals(repeat.raw),rawDifferencesConfinedToUnusedSlots:rawDifferences,firstSha256:hash(first.raw),repeatSha256:hash(repeat.raw),compiledVersusEvalDifferences:differences,observations:[...expressions.keys()].map(index=>({index,decimals2:first.rows.get(JSON.stringify([2,'OFF',index,'EVAL'])),decimals14:first.rows.get(JSON.stringify([14,'OFF',index,'EVAL']))}))},null,2)+'\n');
