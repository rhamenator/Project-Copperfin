// Capture framing/completeness/repeat audit, NOT a conversion acceptance table.
const fs=require('fs'),crypto=require('crypto');
const [kind,first,repeat]=process.argv.slice(2);
const plan={high:{rows:32,groups:2,size:16},sweep:{rows:50,groups:2,size:25},boundary:{rows:216,groups:18,size:12},refinement:{rows:288,groups:18,size:16},neighbors:{rows:144,groups:18,size:8}}[kind];
if(!plan)throw Error('unknown capture kind');
const raw=fs.readFileSync(first),again=fs.readFileSync(repeat),lines=raw.toString().trim().split(/\r?\n/),failures=[],groups={},observations={};
if(!raw.equals(again))failures.push('independent repeat mismatch');
if(lines.length!==plan.rows+2||!lines[0].includes('09.00.0000.7423')||lines.at(-1)!=='DONE|'+plan.rows)failures.push('VERSION/DONE/complete row count');
for(const line of lines.slice(1,-1)){
 const a=line.split('|'),get=k=>a.find(f=>f.startsWith(k+'='))?.slice(k.length+1),key=[a[0],get('rows')??'3',get('targetRow')??'2',get('targetID')??'22'].join('|');
 if(!['ListBox','ComboBox'].includes(a[0])||!a.includes('invoke')||Number(get('error'))!==0||Number(get('expandError'))!==0||get('return')!=='.T.')failures.push('invocation framing '+key);
 if(kind==='boundary'||kind==='refinement'||kind==='neighbors'){
  if(get('type')!=='N'||! /^[0-9A-F]{16}$/.test(get('BRS')))failures.push('actual Numeric payload '+key);
 }
 groups[key]=(groups[key]||0)+1;
 const cells=s=>[...s.matchAll(/\[(?:column=(\d+),)?indexError=(\d+),index=([^\]]*?),itemError=(\d+),item=([^\]]*)\]/g)].map(m=>({column:m[1]?+m[1]:null,indexError:+m[2],index:m[3],itemError:+m[4],item:m[5]}));
 const immediate=cells(get('after')),expanded=cells(get('expanded'));
 if(immediate.length!==(kind==='high'?0:1)||expanded.length!==(kind==='high'?14:3))failures.push('readback completeness '+key);
 for(const c of [...immediate,...expanded])if(c.indexError||c.itemError||c.index!==c.item)failures.push('readback framing/route disagreement '+key);
 const col=get('column')??a[2],obs={key,column:col,BRS:get('BRS')??get('preBRS'),before:get('before'),after:get('after'),expanded:get('expanded'),immediateCandidate:immediate[0]?.index==='candidate',expandedCandidates:expanded.map((c,i)=>({...c,column:c.column??(i===0?+col:i===1?1:2)})).filter(c=>c.index==='candidate').map(c=>c.column)};
 const outcome=kind==='high'?JSON.stringify(expanded.filter(c=>c.index==='candidate').map(c=>c.column)):JSON.stringify([immediate[0].index,expanded[0].index]);
 (observations[col]??={})[outcome]=((observations[col]??{})[outcome]??0)+1;
 console.log(JSON.stringify(obs));
}
if(Object.keys(groups).length!==plan.groups||Object.values(groups).some(n=>n!==plan.size))failures.push('complete planned contexts');
console.log(JSON.stringify({kind,rows:lines.length-2,groups,observations,failures,rawSHA256:crypto.createHash('sha256').update(raw).digest('hex'),note:'Native sampled cell observations only. No physical allocation cause, global capacity, default/exact64 extension requirement or helper inferred.'}));
process.exitCode=failures.length?1:0;
