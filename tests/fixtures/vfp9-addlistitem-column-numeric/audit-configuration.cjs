// Native capture framing and independent context consistency; not helper acceptance.
const fs=require('fs'),crypto=require('crypto');
const [first,repeat]=process.argv.slice(2),raw=fs.readFileSync(first),again=fs.readFileSync(repeat),lines=raw.toString().trim().split(/\r?\n/),failures=[],groups={},observations={};
if(!raw.equals(again))failures.push('fresh pair mismatch');
if(lines.length!==290||!lines[0].includes('09.00.0000.7423')||lines.at(-1)!=='DONE|288')failures.push('VERSION/DONE288/full capture');
let rejected=0,admitted=0;
for(const l of lines.slice(1,-1)){
 const a=l.split('|'),get=k=>a.find(x=>x.startsWith(k+'='))?.slice(k.length+1),key=[a[0],get('columns'),get('shape'),get('target')].join('|'),err=+get('error');
 groups[key]=(groups[key]??0)+1;
 if(get('rst')!=='0'||get('binding')!==(['3','4'].includes(get('shape'))?'   ':'')||get('sorted')!==(get('shape')==='4'?'.T.':'.F.'))failures.push('configuration framing '+key);
 if(!['ListBox','ComboBox'].includes(a[0])||!['0','2','8'].includes(get('columns'))||!['1','2','3','4'].includes(get('shape'))||get('target')!=='22'||a[5]!=='invoke')failures.push('context/phase '+key);
 if(get('preType')!=='N'||get('postType')!=='N'||! /^[0-9A-F]{16}$/.test(get('preBRS'))||get('preBRS')!==get('postBRS'))failures.push('actual Numeric payload preservation '+a[4]);
 if(err===11){rejected++;if(get('before')!==get('after'))failures.push('rejection sampled state changed '+a[4]);}
 else if(err===0){admitted++;if(get('return')!=='.T.')failures.push('admitted return '+a[4]);}
 else failures.push('unexpected error '+err);
 const reads=s=>[...s.matchAll(/\[column=(\d+),indexError=(\d+),index=([^\]]*?),itemError=(\d+),item=([^\]]*)\]/g)].map(m=>({column:+m[1],indexError:+m[2],index:m[3],itemError:+m[4],item:m[5]}));
 const before=reads(get('before')),after=reads(get('after'));
 if(before.length!==21||after.length!==21)failures.push('complete3x7 dual route readback '+key);
 for(const c of [...before,...after])if(c.indexError||c.itemError||c.index!==c.item)failures.push('readback errors/disagreement '+key);
 const obs={error:err,BRS:get('preBRS'),candidate:after.filter(c=>c.index==='candidate').map(c=>c.column),sampledStateUnchanged:get('before')===get('after')};
 const serialized=JSON.stringify(obs);
 (observations[a[4]]??={})[serialized]=((observations[a[4]]??{})[serialized]??0)+1;
 console.log(JSON.stringify({key,source:a[4],...obs}));
}
if(Object.keys(groups).length!==24||Object.values(groups).some(n=>n!==12)||Object.keys(observations).length!==12)failures.push('24 contexts x12 distinct operands');
for(const [expr,outcomes]of Object.entries(observations))if(Object.keys(outcomes).length!==1||Object.values(outcomes)[0]!==24)failures.push('context outcome disagreement '+expr);
console.log(JSON.stringify({rows:lines.length-2,groups,rejected,admitted,observations,failures,rawSHA256:crypto.createHash('sha256').update(raw).digest('hex'),note:'Native sampled existing-ID observations only; no parser defect, hidden physical layout, exact64/default/NaN extension oracle or shared-success/native-return parity implementation inferred.'}));
process.exitCode=failures.length?1:0;
