// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent structural/type-aware audit. No production helper or inferred normalization formula.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2],source=fs.readFileSync(process.argv[3]||path.join(__dirname,'assignment-probe.prg'),'utf8');
const expressions=new Map();
for(const line of source.split('\n').filter(l=>l.startsWith('expressions['))){
 const m=line.match(/^expressions\[(\d+)\]='([^']*)'\r?$/);a(m,line);const index=Number(m[1]),e=m[2];a(!expressions.has(index));expressions.set(index,e);
 const block=source.split('  CASE index='+index+'\n')[1].split('  CASE index=')[0];
 a(block.includes('answer=CFNMETA('+e+")+'|'+HexBytes(BINTOC("+e+",'BRS'))+'|'+HexBytes(TRANSFORM("+e+'))'));
 for(const name of ['assignedLocal','assignedPrivate','assignedPublic'])a(block.includes(name+'='+e+'\n'));
 a(block.includes('copiedLocal=assignedLocal\n'));
}
a.equal(expressions.size,24);a(source.includes('evaluatedLocal=EVALUATE(expression)'));
function decode(meta,bits){
 a.match(meta,/^type=[0-9A-F]{2};width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=[0-9A-F]{4}$/);
 const m=Object.fromEntries(meta.split(';').map(s=>s.split('=')));a.equal(m.cw,'027F');a(['49','4E'].includes(m.type));
 const common={width:parseInt(m.width,16),bits,cw:m.cw};
 if(m.type==='4E'){a.equal(m.real,bits);return {type:'N',...common,decimals:parseInt(m.decimals,16)};}
 const integer=parseInt(m.integer,16)|0;a.equal(Buffer.from(bits,'hex').readDoubleBE(),integer);
 return {type:'I',...common,integer}; // Native Integer decimal/real slots deliberately ignored.
}
function capture(name){
 a.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');a.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
 const raw=fs.readFileSync(path.join(root,name+'-results.txt'));a.deepEqual(raw,fs.readFileSync(path.join(root,name+'.out')));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|672');a.equal(lines.length,672);
 const rows=new Map(),types={I:0,N:0};let errors=0;
 for(const line of lines){const f=line.split('|'),decimals=Number(f[1]),fixed=f[2],index=Number(f[3]),mode=Number(f[4]);
  a([2,14].includes(decimals));a(['OFF','ON'].includes(fixed));a(mode>=1&&mode<=7);
  const expression=Buffer.from(f[5],'hex').toString('ascii');a.equal(expression,expressions.get(index));const key=JSON.stringify([decimals,fixed,index,mode]);a(!rows.has(key));
  if(f[0]==='ERROR'){a.equal(f.length,8);++errors;rows.set(key,{expression,error:Number(f[6]),message:f[7]});continue;}
  a.equal(f[0],'VALUE');a.equal(f.length,9);a.match(f[7],/^[0-9A-F]{16}$/);a.match(f[8],/^(?:[0-9A-F]{2})+$/);
  const d=decode(f[6],f[7]);++types[d.type];rows.set(key,{expression,...d,display:Buffer.from(f[8],'hex').toString('ascii')});
 }
 for(const d of [2,14])for(const fixed of ['OFF','ON'])for(const i of expressions.keys())for(let mode=1;mode<=7;++mode)a(rows.has(JSON.stringify([d,fixed,i,mode])));
 return {raw,rows,types,errors};
}
const first=capture('first'),repeat=capture('repeat');a.deepEqual(first.rows,repeat.rows);
const x=first.raw.toString('ascii').split('\r\n'),y=repeat.raw.toString('ascii').split('\r\n'),unusedDifferences={};a.equal(x.length,y.length);
for(let i=0;i<x.length;++i)if(x[i]!==y[i]){const f=x[i].split('|'),g=y[i].split('|');a.equal(f.length,9);a.equal(g.length,9);
 for(let j=0;j<9;++j)if(j!==6)a.equal(f[j],g[j]);
 const m=Object.fromEntries(f[6].split(';').map(s=>s.split('='))),n=Object.fromEntries(g[6].split(';').map(s=>s.split('=')));
 for(const key of Object.keys(m))if(m[key]!==n[key]){a((m.type==='49'&&['decimals','real'].includes(key))||(m.type==='4E'&&key==='integer'));const label=m.type+':'+key;unusedDifferences[label]=(unusedDifferences[label]||0)+1;}
}
const differences=[];
for(const d of [2,14])for(const fixed of ['OFF','ON'])for(const i of expressions.keys()){
 const row=mode=>first.rows.get(JSON.stringify([d,fixed,i,mode]));
 for(const [left,right] of [[1,2],[3,4],[4,5],[4,6],[4,7]])if(JSON.stringify(row(left))!==JSON.stringify(row(right)))differences.push({decimals:d,fixed,index:i,left,right,leftValue:row(left),rightValue:row(right)});
}
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
process.stdout.write(JSON.stringify({casesPerRun:first.rows.size,successfulValues:first.rows.size-first.errors,recordedErrors:first.errors,typeCounts:first.types,documentedFieldsRepeatEqual:true,rawByteEqual:first.raw.equals(repeat.raw),rawDifferencesConfinedToUnusedSlots:unusedDifferences,firstSha256:hash(first.raw),repeatSha256:hash(repeat.raw),comparisonDifferences:differences,observations:[...expressions].map(([index,expression])=>({index,expression,direct:first.rows.get(JSON.stringify([2,'OFF',index,2])),local:first.rows.get(JSON.stringify([2,'OFF',index,4])),localD14:first.rows.get(JSON.stringify([14,'OFF',index,4]))}))},null,2)+'\n');
