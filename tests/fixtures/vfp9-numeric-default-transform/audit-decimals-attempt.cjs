// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Replay the native feedback path, not a formatter; reject incomplete boundaries.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-decimals-cases.json')));
const source=fs.readFileSync(path.join(__dirname,'rounding-decimals.prg'),'utf8');
a.equal(inv.groups.length,38);a.equal(inv.lowOffset,'-4503599627370496');a.equal(inv.highOffset,'0');
a.equal(inv.maxQueriesPerGroup,54);a.equal(inv.width,40);a.equal(inv.scale,18);
for(const g of inv.groups){a(source.includes(`baseHex[${g.id}]='${g.bits}'`));a(source.includes(`negative[${g.id}]=${g.negative?'.T.':'.F.'}`));a(source.includes(`setting[${g.id}]=${g.setting}`));}
const bytes=name=>{const p=path.join(process.argv[2],name);return fs.existsSync(p)?fs.readFileSync(p):zlib.gunzipSync(fs.readFileSync(p+'.gz'));};
const sha=x=>crypto.createHash('sha256').update(x).digest('hex');
const hex=(g,offset)=>(BigInt('0x'+g.bits)+offset|(g.negative?0x8000000000000000n:0n)).toString(16).padStart(16,'0').toUpperCase();
function read(name){
 a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);
 const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
 a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
 const observations=[],boundaries=[],rawMeta=[],unbracketed=[];let cursor=0;
 for(const g of inv.groups){let query=0;
  function observe(offset){++query;a(query<=54);let onText;
   for(const fixed of ['OFF','ON']){
    const f=lines[cursor++].split('|');a.equal(f.length,10);
    a.deepEqual(f.slice(0,5),[String(g.id),String(query),fixed,hex(g,offset),String(g.setting)]);
    a.match(f[5],/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
    const meta=Object.fromEntries(f[5].split(';').map(s=>s.split('=')));a.equal(meta.real,hex(g,offset));a.equal(f[6],hex(g,offset));
    a.match(f[7],/^(?:[0-9A-F]{2})*$/);a.match(f[8],/^(?:[0-9A-F]{2})*$/);a.equal(f[9],'C');
    observations.push({group:g.id,center:g.center,negative:g.negative,setting:g.setting,query,fixed,offset:String(offset),bits:f[3],displayHex:f[7],strHex:f[8]});
    rawMeta.push(f[5]);if(fixed==='ON')onText=f[7];
   }return onText;
  }
  let lo=BigInt(inv.lowOffset),hi=0n;
  const highText=observe(hi);let lowText=observe(lo);if(g.id>=33){a.equal(lowText,highText);a.equal(lines[cursor++],'UNBRACKETED|'+g.id);unbracketed.push({group:g.id,setting:g.setting,negative:g.negative,initialTextHex:highText,queries:query});continue;}a.notEqual(lowText,highText);
  while(hi-lo>1n){const mid=(lo+hi)/2n;const text=observe(mid);if(text===highText)hi=mid;else{lo=mid;lowText=text;}}
  a.deepEqual(lines[cursor++].split('|'),['EDGE',String(g.id),hex(g,lo),hex(g,hi),lowText,highText]);
  boundaries.push({group:g.id,center:g.center,negative:g.negative,setting:g.setting,queries:query,
   lowOffset:String(lo),highOffset:String(hi),lowBits:hex(g,lo),highBits:hex(g,hi),lowTextHex:lowText,highTextHex:highText,adjacent:true});
 }
 a.equal(lines[cursor++],'DONE|'+observations.length);a.equal(cursor,lines.length);
 return {raw,observations,boundaries,rawMeta,unbracketed};
}
const first=read('first');a.equal(first.observations.length,3480);a.equal(first.boundaries.length,32);a.equal(first.unbracketed.length,6);
console.log(JSON.stringify({requirement:inv.requirement,originalQualificationFailed:true,pairedQualified:false,diagnosticOnly:true,records:3480,firstSha256:sha(first.raw),metadataAndBRSIdentityExact:true,unbracketed:first.unbracketed,boundaries:first.boundaries,observations:first.observations,limitation:'Forensic structure/path readback of an UNQUALIFIED first-only experiment. Frozen original strict reader remains failed1; repeat was not invoked. Does not retroactively qualify a pair, synthesize expected strings or establish the applied settings. Six observed equal endpoint controls require refined stimulus.'},null,2));
