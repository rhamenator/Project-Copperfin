// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: AFTER-capture exact decimal/text decomposition.
// Never format a payload or use binary64(decimalText) as a compatibility oracle.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const bytes=(dir,n)=>z.gunzipSync(fs.readFileSync(path.join(dir,n+'.gz')));
function load(name,digest,fields,flags){
 const dir=path.join(__dirname,name),report=JSON.parse(bytes(dir,'audit.json'));
 a.equal(report.recordsPerRun,528);a.equal(report.observations.length,528);
 for(const k of ['documentedFieldsRepeatEqual','rawByteEqual',...flags])a.equal(report[k],true);
 const sums=new Map();for(const line of fs.readFileSync(path.join(dir,'SHA256SUMS'),'utf8').trimEnd().split('\n')){
  const m=line.match(/^([0-9a-f]{64})  ([A-Za-z0-9_.-]+)$/);a(m);a(!sums.has(m[2]));sums.set(m[2],m[1]);
 }
 for(const n of ['audit.json','first-results.txt','repeat-results.txt']){a(sums.has(n+'.gz'));a.equal(sha(fs.readFileSync(path.join(dir,n+'.gz'))),sums.get(n+'.gz'));}
 for(const run of ['first','repeat']){
  a.equal(report[run+'Sha256'],digest);a.equal(sha(bytes(dir,run+'-results.txt')),digest);
  a.deepEqual(bytes(dir,run+'-results.txt'),bytes(dir,run+'.out'));a.equal(bytes(dir,run+'.status').toString(),'0\n');a.equal(bytes(dir,run+'.stderr').length,0);
 }
 const lines=bytes(dir,'first-results.txt').toString('ascii').split('\r\n');a.equal(lines.pop(),'');
 a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|528');a.equal(lines.length,528);
 const raw=lines.map(s=>{const f=s.split('|');a.equal(f.length,fields);return f;});return {report,raw};
}
const hashes={round:'254b46dadbb71fa51b10e8439d51785b2c2aa3eb871c8c948fd3b8bdd3da4873',
 arithmetic:'1979626f4fec992924791da5cbc89ed3abef1583810690300fff1fd3cf95ea32',
 str:'13fba5c3663d1d44f0cf514d481abfc66e16063e3e300d1c8e33502b10282c6c'};
const round=load('round-inline-captures1',hashes.round,35,['typedResultAndControls','inlineResultAndStoredParentControls','exactRuntimeIdentity']);
const arithmetic=load('arithmetic-inline-captures1',hashes.arithmetic,66,['typedResultAndControls','inlineArithmeticAndStoredParentControls','exactRuntimeIdentity']);
const str=load('str-middle-captures1',hashes.str,32,['originalAndNativeParentControls','parentStrBeforeAfterExact','inlineMetadataAndBRSExact','publicStrTypesAndWidths']);
const ascii=h=>{a.match(h,/^(?:[0-9A-F]{2})*$/);const b=Buffer.from(h,'hex');a(b.every(n=>n>0&&n<128));return b.toString('ascii');};
function meta(s,bits){
 a.match(bits,/^[0-9A-F]{16}$/);a.notEqual((BigInt('0x'+bits)>>52n)&2047n,2047n);
 a.match(s,/^type=(?:4E|49);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);
 const m=Object.fromEntries(s.split(';').map(x=>x.split('=')));
 if(m.type==='4E'){a.equal(m.real,bits);delete m.integer;}
 else{const b=Buffer.alloc(8);b.writeDoubleBE(parseInt(m.integer,16)|0);a.equal(b.toString('hex').toUpperCase(),bits);delete m.real;delete m.decimals;}
 return m;
}
const key=r=>JSON.stringify([r.index,r.fixed]);
function keyed(data){const map=new Map();for(const[i,r]of data.report.observations.entries()){
 const f=data.raw[i];a.equal(f[0],r.fixed);a.equal(f[1],String(r.index));a.equal(f[2],r.bits);a.equal(f[3],String(r.setting));
 a.equal(f[6],r.originalDisplayHex);a.equal(f[7],r.originalStrHex);a.equal(f[5],r.bits);a.equal(f[19],r.bits);a.equal(f[20],r.originalDisplayHex);
 a.deepEqual(meta(f[4],r.bits),meta(f[18],r.bits));a(!map.has(key(r)));map.set(key(r),{r,f});
}a.equal(map.size,528);return map;}
const rounds=keyed(round),arithmetics=keyed(arithmetic),strs=keyed(str);
for(const {r,f}of rounds.values()){
 const {r:g,f:gf}=arithmetics.get(key(r)),{r:s,f:sf}=strs.get(key(r));a(g&&s);
 for(const other of [g,s])for(const n of ['bits','setting','originalDisplayHex','originalStrHex'])a.equal(r[n],other[n]);
 a.deepEqual(meta(f[4],r.bits),s.originalMeta);a.deepEqual(meta(gf[4],g.bits),s.originalMeta);
 a.deepEqual(r.crt,g.crt);a.deepEqual(r.crt,s.crt);
 for(const j of [12,21])a.deepEqual(Object.fromEntries(f[j].split(';').map(x=>x.split('='))),r.crt);
 for(const j of [12,21])a.deepEqual(Object.fromEntries(gf[j].split(';').map(x=>x.split('='))),g.crt);
 for(const j of [12,22])a.deepEqual(Object.fromEntries(sf[j].split(';').map(x=>x.split('='))),s.crt);
 for(const[j,k]of [[8,8],[9,9],[10,10],[11,11]]){a.equal(f[j],gf[k]);a.equal(f[j],sf[k]);}
 a.equal(f[8],'N');a.equal(f[10],'C');a.equal(ascii(f[11]),r.fixed);a.match(ascii(f[9]),/^\s*\d+(?:\.0+)?\s*$/);
 a.equal(BigInt(ascii(f[9]).trim().split('.')[0]),BigInt(r.setting));
 // Public VARTYPE=N includes Numeric and Integer: active FLL tag selects slots.
 a.equal(f[13],'N');a.equal(f[26],'N');a.equal(f[15],r.resultBits);a.equal(f[23],r.resultBits);
 for(const j of [14,22])a.deepEqual(meta(f[j],r.resultBits),r.resultMeta);
 for(const[j,n]of [[16,'resultDisplayHex'],[24,'resultDisplayHex'],[17,'resultStrHex'],[25,'resultStrHex']])a.equal(f[j],r[n]);
 a.equal(f[28],r.inlineBits);a.equal(f[32],r.inlineBits);for(const j of [27,31])a.deepEqual(meta(f[j],r.inlineBits),r.inlineMeta);
 for(const[j,n]of [[29,'inlineDisplayHex'],[33,'inlineDisplayHex'],[30,'inlineStrHex'],[34,'inlineStrHex']])a.equal(f[j],r[n]);
 for(const n of ['resultBits','resultDisplayHex','resultStrHex'])a.equal(r[n],g[n]);a.deepEqual(r.resultMeta,g.resultMeta);
 for(const j of [14,22])a.deepEqual(meta(gf[j],g.resultBits),g.resultMeta);
 for(const[j,n]of [[15,'resultBits'],[23,'resultBits'],[16,'resultDisplayHex'],[24,'resultDisplayHex'],[17,'resultStrHex'],[25,'resultStrHex']])a.equal(gf[j],g[n]);
 a.equal(r.inlineBits,r.resultBits);a.equal(r.inlineDisplayHex,r.resultDisplayHex);a.equal(r.inlineStrHex,r.resultStrHex);
}
function decimal(text){
 a(text.length<=127);const m=text.trim().match(/^([+-]?)(?:(\d+)(?:\.(\d*))?|\.(\d+))(?:[eE]([+-]?\d+))?$/);a(m);
 const fraction=m[3]===undefined?(m[4]||''):m[3];let n=BigInt((m[2]||'0')+fraction),e=BigInt(m[5]||'0')-BigInt(fraction.length);
 const negative=m[1]==='-';if(negative)n=-n;const zero=n===0n;
 if(zero)e=0n;else while(n%10n===0n){n/=10n;++e;}
 return {coefficient:String(n),exponent:String(e),zero,negative};
}
const same=(x,y)=>x.coefficient===y.coefficient&&x.exponent===y.exponent;
let metricSelfChecks=0;
for(const[left,right,equal]of [['0','-0.000',true],['1.20','1.2e0',true],['.0001','1e-4',true],['-12.3400','-1234e-2',true],['0.000000001','0',false],['1','-1',false]]){a.equal(same(decimal(left),decimal(right)),equal);++metricSelfChecks;}
a.equal(decimal('-0.00').negative,true);++metricSelfChecks;a.equal(decimal(' 0.00 ').negative,false);++metricSelfChecks;
function comparison(leftHex,rightHex){
 const left=ascii(leftHex),right=ascii(rightHex),x=decimal(left),y=decimal(right),valueEqual=same(x,y);
 return {leftHex,rightHex,valueEqual,literalEqual:leftHex===rightHex,trimmedLiteralEqual:left.trimStart()===right.trimStart(),
  bothZero:x.zero&&y.zero,zeroSignDifferent:x.zero&&y.zero&&x.negative!==y.negative,leftValue:x,rightValue:y};
}
const rows=[];
for(const {r}of rounds.values()){
 if(r.fixed!=='ON')continue;
 const payload=BigInt('0x'+r.inlineBits),payloadZero=(payload&((1n<<63n)-1n))===0n;
 rows.push({index:r.index,setting:r.setting,bits:r.bits,inlineBits:r.inlineBits,inlineMeta:r.inlineMeta,storedMeta:r.resultMeta,
  payloadZero,payloadNegative:(payload>>63n)===1n,
  originalVsRoundTransform:comparison(r.originalDisplayHex,r.inlineDisplayHex),
  originalStrVsRoundStr:comparison(r.originalStrHex,r.inlineStrHex),
  originalTransformVsOriginalStr:comparison(r.originalDisplayHex,r.originalStrHex),
  roundTransformVsRoundStr:comparison(r.inlineDisplayHex,r.inlineStrHex)});
}
a.equal(rows.length,264);
const fields=['originalVsRoundTransform','originalStrVsRoundStr','originalTransformVsOriginalStr','roundTransformVsRoundStr'];
function summarize(items,field){
 const cs=items.map(r=>r[field]),valueDifferent=cs.filter(r=>!r.valueEqual).length;
 const sameValueLiteralDifferent=cs.filter(r=>r.valueEqual&&!r.literalEqual).length;
 const sameValueSameLiteral=cs.filter(r=>r.valueEqual&&r.literalEqual).length;
 a.equal(valueDifferent+sameValueLiteralDifferent+sameValueSameLiteral,items.length);
 return {contexts:items.length,literalDifferent:cs.filter(r=>!r.literalEqual).length,trimmedLiteralDifferent:cs.filter(r=>!r.trimmedLiteralEqual).length,
  valueDifferent,sameValueLiteralDifferent,sameValueSameLiteral,zeroSignDifferent:cs.filter(r=>r.zeroSignDifferent).length,
  valueDifferenceIndices:items.filter(r=>!r[field].valueEqual).map(r=>r.index)};
}
const summary=Object.fromEntries(fields.map(f=>[f,summarize(rows,f)]));
const bySetting=[...new Set(rows.map(r=>r.setting))].sort((a,b)=>a-b).map(setting=>({setting,...summarize(rows.filter(r=>r.setting===setting),'originalVsRoundTransform')}));
const metaFieldsDifferent={};for(const {r}of rounds.values())for(const n of new Set([...Object.keys(r.inlineMeta),...Object.keys(r.resultMeta)]))if(r.inlineMeta[n]!==r.resultMeta[n])metaFieldsDifferent[n]=(metaFieldsDifferent[n]||0)+1;
const valueDifferenceIndices=summary.originalVsRoundTransform.valueDifferenceIndices;
const zeroSpelling=rows.filter(r=>r.originalVsRoundTransform.bothZero).map(r=>({index:r.index,setting:r.setting,bits:r.bits,resultBits:r.inlineBits,payloadZero:r.payloadZero,payloadNegative:r.payloadNegative,
  originalHex:r.originalVsRoundTransform.leftHex,roundHex:r.originalVsRoundTransform.rightHex,zeroSignDifferent:r.originalVsRoundTransform.zeroSignDifferent}));
console.log(JSON.stringify({analysisOnly:true,nativeInvoked:false,ordinaryShellNotReaEvidence:true,sourceHashes:hashes,metricSelfChecks,
 sharedContextsChecked:rounds.size,onContexts:rows.length,inlineVsStored:{bitsAndTextEqualContexts:528,metaFieldsDifferent},summary,bySetting,
 valueDifferenceIndices,zeroSpelling,rows,
 limitations:'Only retained264 signed ON contexts with528 OFF/ON raw/control joins. Exact DECIMAL TEXT values ignore padding and signed-zero spelling for value equality only; literal/sign metrics remain separate. No exact binary payload-versus-decimal discrepancy is called a conversion failure. No native replay, expected emitter, formatter, accepted tolerance/cap/internal call sequence, Windows parity or retroqualification.'},null,2));
