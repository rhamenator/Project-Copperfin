// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native-only source/framing/typed-input audit; no production helper import.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),crypto=require('node:crypto');
const zlib=require('node:zlib');
function nativeBytes(name){const p=path.join(root,name),plain=fs.existsSync(p),packed=fs.existsSync(p+'.gz');a.notEqual(plain,packed,'exactly one capture representation: '+name);return plain?fs.readFileSync(p):zlib.gunzipSync(fs.readFileSync(p+'.gz'));}
const root=process.argv[2],mode=process.argv[3];a(['payload','public'].includes(mode));
const source=fs.readFileSync(path.join(__dirname,'runtime-'+mode+'-grid.prg'),'utf8');
const settings=[0,2,6,14,18],count=mode==='payload'?648:60,total=count*20;
const widths=[1,4,10,25,4,13,25,25,40],scales=[0,0,0,0,2,2,0,14,18];
const payloads=[[5,2],[-5,2],[5,-2],[-5,-2],[0,2],[-0,2],[7,3],[2147483647,3]];
const expressions=new Map([...source.matchAll(/^expressions\[(\d+)\]='([^']*)'$/gm)].map(m=>[Number(m[1]),m[2]]));a.equal(expressions.size,count);
a(source.includes('expressions['+count+'], settings[5]'));
for(let i=0;i<settings.length;++i)a(source.includes('settings['+(i+1)+']='+settings[i]));
for(const [i,e]of expressions) {
 let line=' CASE index='+i+'\n  RETURN CFNMETA('+e+")+'|'+HexBytes(BINTOC("+e+",'BRS'))+'|'+HexBytes(TRANSFORM("+e+'))';
 if(mode==='payload'){const [l,r]=e.split('/');line+="+'|'+CFNMETA("+l+")+'|'+CFNMETA("+r+')';}
 a(source.includes(line),'source/compiler pairing '+i);
}
const bits=n=>{const b=Buffer.alloc(8);b.writeDoubleBE(n);return b.toString('hex').toUpperCase();};
function meta(text){a.match(text,/^type=(?:49|4E);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);const raw=Object.fromEntries(text.split(';').map(s=>s.split('=')));const normalized={type:raw.type,width:parseInt(raw.width,16),cw:raw.cw};if(raw.type==='49')normalized.integer=parseInt(raw.integer,16)|0;else{normalized.decimals=parseInt(raw.decimals,16);normalized.bits=raw.real;}return{raw,normalized};}
function capture(name){
 a.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');a.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
 const raw=nativeBytes(name+'-results.txt');a.deepEqual(raw,nativeBytes(name+'.out'));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|'+total);a.equal(lines.length,total);
 const rows=new Map();
 for(const line of lines){const f=line.split('|');a.equal(f.length,mode==='payload'?10:8);a(['EVAL','COMPILED'].includes(f[0]));const s=Number(f[1]),i=Number(f[3]);a(settings.includes(s));a(['ON','OFF'].includes(f[2]));a.equal(Buffer.from(f[4],'hex').toString('ascii'),expressions.get(i));const out=meta(f[5]);a.equal(out.raw.type,'4E');a.equal(out.raw.real,f[6]);a.match(f[7],/^(?:[0-9A-F]{2})+$/);
  if(mode==='payload'){
   const group=Math.floor((i-1)/81),li=Math.floor(((i-1)%81)/9),ri=(i-1)%9;const inputs=[meta(f[8]),meta(f[9])],indices=[li,ri];
   for(let k=0;k<2;++k){const v=inputs[k].normalized,j=indices[k],p=payloads[group][k];a.equal(v.type,j<4?'49':'4E');a.equal(v.width,widths[j]);if(j<4)a.equal(v.integer,p===0?0:p);else{a.equal(v.decimals,scales[j]);a.equal(v.bits,bits(p));}}
   const d=Math.min(18,Math.max(s+2,scales[li],scales[ri])),w=Math.min(40,widths[li]+widths[ri]-scales[li]-scales[ri]+d-(scales[li]>0||scales[ri]>0?1:0));a.equal(out.normalized.width,w,'native width '+i);a.equal(out.normalized.decimals,d,'native scale '+i);
   const l=li<4&&payloads[group][0]===0?0:payloads[group][0],r=payloads[group][1];a.equal(f[6],bits(l/r),'quotient bits '+i);
  }
  const key=JSON.stringify([s,f[2],i,f[0]]);a(!rows.has(key));rows.set(key,{fields:f,out});
 }
 for(const s of settings)for(const fixed of ['OFF','ON'])for(let i=1;i<=count;++i){const x=rows.get(JSON.stringify([s,fixed,i,'EVAL'])),y=rows.get(JSON.stringify([s,fixed,i,'COMPILED']));a(x&&y);a.deepEqual(x.out.normalized,y.out.normalized);a.equal(x.fields[7],y.fields[7]);if(mode==='payload')for(const j of [8,9])a.deepEqual(meta(x.fields[j]).normalized,meta(y.fields[j]).normalized);}
 return{raw,rows};
}
const first=capture('first'),repeat=capture('repeat'),unused={};let different=0;
for(const [key,x]of first.rows){const y=repeat.rows.get(key);a(y);for(let i=0;i<x.fields.length;++i){if(![5,8,9].includes(i)){a.equal(x.fields[i],y.fields[i]);continue;}const p=meta(x.fields[i]),q=meta(y.fields[i]);a.deepEqual(p.normalized,q.normalized);for(const f of Object.keys(p.raw))if(p.raw[f]!==q.raw[f]){a(p.raw.type==='49'?['decimals','real'].includes(f):f==='integer');const label=i+':'+p.raw.type+':'+f;unused[label]=(unused[label]||0)+1;}}if(x.fields.join('|')!==y.fields.join('|'))++different;}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
let headerMatched=0;const header=path.join(__dirname,'runtime-public-expected.h');
if(mode==='public'&&fs.existsSync(header)){
 const expected=new Map([...fs.readFileSync(header,'utf8').matchAll(/^    \{"((?:\\.|[^"\\])*)", \{\{(.+)\}\}, 0x([0-9A-F]+)ULL\},$/gm)].map(m=>[JSON.parse('"'+m[1]+'"'),{pairs:[...m[2].matchAll(/\{(\d+), (\d+)\}/g)].map(x=>[+x[1],+x[2]]),bits:m[3].padStart(16,'0')}]));a.equal(expected.size,count);
 for(const [key,row]of first.rows){const [s,,i]=JSON.parse(key),v=expected.get(expressions.get(i));a(v);a.equal(v.pairs.length,5);a.deepEqual(v.pairs[settings.indexOf(s)],[row.out.normalized.width,row.out.normalized.decimals]);a.equal(v.bits,row.fields[6]);++headerMatched;}
}
console.log(JSON.stringify({mode,expressions:count,casesPerRun:total,successfulValues:total,errors:0,actualStatuses:[0,0],compiledVersusEvaluateDifferences:0,documentedFieldsRepeatEqual:true,rawByteEqual:first.raw.equals(repeat.raw),differentRawRows:different,rawDifferencesConfinedToUnusedSlots:unused,firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),nativePublicHeaderRowsMatched:headerMatched,productionHelperImported:false},null,2));
