// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Capture/framing/state corroboration ONLY, not an acceptance conversion oracle.
const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const root=process.argv[2]??'build/addlistitem-new-column-native';
const source=fs.readFileSync('tests/fixtures/vfp9-addlistitem-new-column-numeric/independent-probe.prg','utf8');
const operands=[...source.matchAll(/^args\[\d+\]='([^']+)'$/gm)].map(m=>m[1]);
assert.equal(operands.length,65);assert.equal(new Set(operands).size,65);
for(const f of ['first.status','repeat.status','pair.cmp.status'])assert.equal(fs.readFileSync(root+'/'+f,'utf8').trim(),'0',f);
for(const f of ['first.stderr','repeat.stderr'])assert.equal(fs.readFileSync(root+'/'+f,'utf8'),'',f);
const first=fs.readFileSync(root+'/first-results.txt'),repeat=fs.readFileSync(root+'/repeat-results.txt');assert(first.equals(repeat),'complete fresh native pair');
const lines=first.toString('utf8').trim().split(/\r?\n/);assert.equal(lines.length,782);assert.equal(lines[0],'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');assert.equal(lines.at(-1),'DONE|780');
const rows=lines.slice(1,-1).map(l=>l.split('|'));assert(rows.every(r=>r.length===17&&r[0]==='CASE'));
const groups=new Map(),counts={};let rejected=0,admitted=0,unchangedRejections=0,createdRows=0;
const count=s=>Number(s.match(/^count=(\d+);/)[1]);
for(const r of rows){
 const [tag,kind,column,shape,id,operand,type,bits,afterType,afterBits,phase,error,message,returned,before,after,expanded]=r;
 assert(['ListBox','ComboBox'].includes(kind));assert(['0','2','9'].includes(column));assert(['0','1'].includes(shape));assert.equal(id,shape==='0'?'11':'44');
 const group=[kind,column,shape,id].join('|');if(!groups.has(group))groups.set(group,[]);groups.get(group).push(operand);
 assert.equal(phase,'invoke');assert.equal(type,'N');assert.equal(afterType,type);assert(/^[0-9A-F]{16}$/.test(bits));assert.equal(afterBits,bits);
 assert.equal(count(before),shape==='0'?0:3);assert(![...before.matchAll(/;row=\d+,id=(\d+),/g)].some(m=>m[1]===id),'target missing before call');
 assert(!expanded.startsWith('EXPANDERROR'));assert(expanded.includes(';columns=9'));
 counts[error]=(counts[error]??0)+1;
 if(error==='11'){rejected++;assert.equal(after,before,'full sampled rejection state');assert.equal(returned,'<not-called>');unchangedRejections++;}
 else if(error==='0'){admitted++;assert.equal(message,'');if(count(after)===count(before)+1)createdRows++;}
 else assert.fail('Unexpected captured error '+error);
}
assert.equal(groups.size,12);for(const group of groups.values())assert.deepEqual(group,operands);
console.log(JSON.stringify({completeCapture:true,freshPairEqual:true,subjects:rows.length,groups:groups.size,operands:operands.length,errors:counts,rejected,admitted,unchangedRejections,observedOneRowCreations:createdRows,sourceSHA256:crypto.createHash('sha256').update(source).digest('hex'),rawSHA256:crypto.createHash('sha256').update(first).digest('hex'),note:'Native capture/state framing corroboration only, no conversion expected table, hidden-storage/global capacity, Copperfin behavior or completed requirements/implementation claim.'}));
