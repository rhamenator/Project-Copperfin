// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),fs=require('node:fs'),z=require('node:zlib'),path=require('node:path');
const report=JSON.parse(fs.readFileSync(process.argv[2]));
const source=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,'str-middle-captures1/audit.json.gz'))));
const modes=['truncated','nearestEven','nearestAway'],seen=new Set();a.equal(report.rows.length,2112);
for(const row of report.rows){
 const key=JSON.stringify([row.index,row.fixed,row.count]);a(!seen.has(key));seen.add(key);
 const original=source.observations.find(s=>s.index===row.index&&s.fixed===row.fixed);a(original);
 a.equal(row.bits,original.bits);a.equal(row.setting,original.setting);a.equal(row.strHex,original['str'+row.count+'Hex']);
 const text=Buffer.from(row.strHex,'hex').toString('ascii');a.equal(row.text,text);
 // Independently decode the bit string and construct an exact rational.
 const binary=BigInt('0x'+row.bits).toString(2).padStart(64,'0');
 const negative=binary[0]==='1',e=parseInt(binary.slice(1,12),2);
 let n=BigInt('0b'+(e===0?'0':'1')+binary.slice(12)),d=1n;
 const shift=(e===0?1:e)-1075;
 if(shift<0)d=2n**BigInt(-shift);else n*=2n**BigInt(shift);
 n*=10n**BigInt(row.count);
 const floor=n/d,remainder=n-floor*d,half=remainder*2n;
 const expected=[floor,floor+(half>d||(half===d&&floor%2n===1n)?1n:0n),floor+(half>=d?1n:0n)];
 const m=text.trim().match(/^(-?)(\d+)\.(\d+)$/);a(m);a.equal(m[3].length,row.count);
 const observed=BigInt(m[2]+m[3])*(m[1]?-1n:1n);a.equal(row.observedScaled,String(observed));
 a.equal(row.observedNegative,m[1]==='-');a.equal(row.candidate.negative,negative);
 a.equal(row.candidate.exact,remainder===0n);a.equal(row.candidate.tie,half===d);
 a.equal(row.candidate.remainder,String(remainder));a.equal(row.candidate.denominator,String(d));
 for(const [i,mode]of modes.entries()){const value=expected[i]*(negative?-1n:1n);
  a.equal(row.candidate[mode],String(value));a.equal(row.mismatches[mode],value!==observed);}
 a.equal(row.zeroSignDifferent,observed===0n?negative!==(m[1]==='-'):null);
}
a.deepEqual(report.mismatches,report.rows.filter(r=>modes.some(m=>r.mismatches[m])||r.zeroSignDifferent===true));
for(const g of report.groups){const rows=report.rows.filter(r=>r.fixed===g.fixed&&r.count===g.count);a.equal(rows.length,264);a.equal(g.contexts,264);
 for(const mode of modes)a.equal(g[mode],rows.filter(r=>r.mismatches[mode]).length);
 a.equal(g.zeroSignDifferent,rows.filter(r=>r.zeroSignDifferent===true).length);}
console.log(JSON.stringify({independentReadback:true,rows:report.rows.length,groups:report.groups,mismatchRows:report.mismatches.length,nativeInvoked:false},null,2));
