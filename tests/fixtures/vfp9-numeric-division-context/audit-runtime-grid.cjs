// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Standalone native recovery audit: no Copperfin runtime/helper import.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2],source=fs.readFileSync(process.argv[3]||path.join(__dirname,'runtime-grid.prg'),'utf8');
const settings=[0,2,6,14,18],widths=[1,4,10,25,4,13,25,25,40],scales=[0,0,0,0,2,2,0,14,18];
const expressions=new Map();
for(const match of source.matchAll(/^expressions\[(\d+)\]='([^']*)'$/gm)) {
 const i=Number(match[1]),e=match[2],[l,r]=e.split('/');a(!expressions.has(i));a(l&&r);
 a(source.includes(' CASE index='+i+'\n  RETURN CFNMETA('+e+")+'|'+HexBytes(BINTOC("+e+",'BRS'))+'|'+HexBytes(TRANSFORM("+e+"))+'|'+CFNMETA("+l+")+'|'+CFNMETA("+r+')'));
 expressions.set(i,e);
}
a.equal(expressions.size,81);a(source.includes('expressions[81], settings[5]'));
for(let i=0;i<settings.length;++i)a(source.includes('settings['+(i+1)+']='+settings[i]));
function meta(text) {
 a.match(text,/^type=(?:49|4E);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
 const fields=Object.fromEntries(text.split(';').map(s=>s.split('=')));
 const result={type:fields.type==='49'?'I':'N',width:parseInt(fields.width,16),cw:fields.cw};
 if(result.type==='I') result.integer=parseInt(fields.integer,16)|0;
 else {result.decimals=parseInt(fields.decimals,16);result.bits=fields.real;}
 return {raw:fields,normalized:result};
}
function input(raw,index,payload) {
 const m=meta(raw),v=m.normalized;
 a.equal(v.type,index<4?'I':'N');a.equal(v.width,widths[index]);
 if(v.type==='I')a.equal(v.integer,payload);
 else {a.equal(v.decimals,scales[index]);const bytes=Buffer.alloc(8);bytes.writeDoubleBE(payload);a.equal(v.bits,bytes.toString('hex').toUpperCase());}
 return m;
}
function capture(name) {
 a.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');a.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
 const raw=fs.readFileSync(path.join(root,name+'-results.txt'));a.deepEqual(raw,fs.readFileSync(path.join(root,name+'.out')));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|1620');a.equal(lines.length,1620);
 const rows=new Map();let rejectedTypeBasedCandidate=0;const recoveredCandidateMismatches=[];
 for(const line of lines) {
  const f=line.split('|');a.equal(f.length,10);a(['EVAL','COMPILED'].includes(f[0]));
  const setting=Number(f[1]),i=Number(f[3]);a(settings.includes(setting));a(['OFF','ON'].includes(f[2]));a(expressions.has(i));
  a.equal(Buffer.from(f[4],'hex').toString('ascii'),expressions.get(i));
  const li=Math.floor((i-1)/9),ri=(i-1)%9,left=input(f[8],li,5),right=input(f[9],ri,2),out=meta(f[5]);
  a.equal(out.normalized.type,'N');a.equal(out.normalized.bits,'4004000000000000');a.equal(f[6],out.normalized.bits);a.match(f[7],/^(?:[0-9A-F]{2})+$/);
  const d=Math.min(18,Math.max(setting+2,scales[li],scales[ri]));
  const sum=widths[li]+widths[ri]-scales[li]-scales[ri]+d;
  const w=Math.min(40,sum-(scales[li]>0||scales[ri]>0?1:0));
  const old=Math.min(40,sum-(li>=4||ri>=4?1:0));
  if(old!==out.normalized.width)++rejectedTypeBasedCandidate;
  if(w!==out.normalized.width||d!==out.normalized.decimals)recoveredCandidateMismatches.push({setting,index:i,mode:f[0],fixed:f[2],predicted:{w,d},actual:out.normalized});
  const key=JSON.stringify([setting,f[2],i,f[0]]);a(!rows.has(key));
  rows.set(key,{line,fields:f,setting,fixed:f[2],index:i,mode:f[0],left,right,out,display:f[7]});
 }
 for(const setting of settings)for(const fixed of ['OFF','ON'])for(let i=1;i<=81;++i) {
  const x=rows.get(JSON.stringify([setting,fixed,i,'EVAL'])),y=rows.get(JSON.stringify([setting,fixed,i,'COMPILED']));a(x&&y);
  for(const field of ['left','right','out'])a.deepEqual(x[field].normalized,y[field].normalized);
  a.equal(x.display,y.display);
 }
 a.equal(recoveredCandidateMismatches.length,0,JSON.stringify(recoveredCandidateMismatches.slice(0,10)));
 a.equal(rejectedTypeBasedCandidate,64);
 return {raw,rows,rejectedTypeBasedCandidate};
}
const first=capture('first'),repeat=capture('repeat');a.equal(first.rows.size,repeat.rows.size);
const unused={};let rawDifferentRows=0;
for(const [key,x]of first.rows) {
 const y=repeat.rows.get(key);a(y);
 for(const field of ['left','right','out'])a.deepEqual(x[field].normalized,y[field].normalized);
 a.equal(x.display,y.display);
 if(x.line!==y.line) {
  ++rawDifferentRows;
  for(let i=0;i<10;++i) {
   if(![5,8,9].includes(i)) {a.equal(x.fields[i],y.fields[i]);continue;}
   const p=meta(x.fields[i]).raw,q=meta(y.fields[i]).raw;
   for(const field of Object.keys(p))if(p[field]!==q[field]) {
    a(p.type==='49'?['real','decimals'].includes(field):field==='integer');
    const label=i+':'+p.type+':'+field;unused[label]=(unused[label]||0)+1;
   }
  }
 }
}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
console.log(JSON.stringify({casesPerRun:1620,actualSuccessfulValues:1620,recordedErrors:0,operandVariants:9,pairs:81,settings,fixedModes:['OFF','ON'],compiledVersusEvaluateDifferences:0,documentedFieldsRepeatEqual:true,rawByteEqual:first.raw.equals(repeat.raw),rawDifferentRows,rawDifferencesConfinedToUnusedSlots:unused,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),rejectedTypeBasedCandidateMismatches:first.rejectedTypeBasedCandidate,recoveredPositiveFiveOverTwoCandidateMismatches:0,limitation:'Native-only intervention recovery at fixed payloads 5/2. No general payload/context algorithm or Copperfin production acceptance.',observations:[...first.rows.values()].map(x=>({setting:x.setting,fixed:x.fixed,index:x.index,mode:x.mode,left:x.left.normalized,right:x.right.normalized,result:x.out.normalized,displayHex:x.display}))},null,2));
