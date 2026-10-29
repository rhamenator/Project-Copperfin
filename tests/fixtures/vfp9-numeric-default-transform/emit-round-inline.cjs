// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Isolate the public ROUND result assignment boundary, not a formatting model.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const fixture=__dirname,parentRoot=path.join(fixture,'round-result-captures1'),parentHash='0be810176af4c28eca3864eae70dc8adbfe409986ca227cdde8ccaa7c8013607';
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(parentRoot,'audit.json.gz'))));
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],parentHash);a.equal(c.createHash('sha256').update(z.gunzipSync(fs.readFileSync(path.join(parentRoot,n+'-results.txt.gz')))).digest('hex'),parentHash);}
const base=JSON.parse(fs.readFileSync(path.join(fixture,'round-result-cases.json')));a.equal(base.inputCount,264);
const selected=parent.observations.filter(r=>r.fixed==='OFF').map(r=>({index:r.index,setting:r.setting,bits:r.bits}));a.deepEqual(selected,base.inputs);
const inv={...base,storedParentSha256:parentHash,fieldsPerRecord:35,
 inlineResultRoute:'Every consumer evaluates ROUND(CFNDEC(CFNWIDTH(payload,40),18),decimalCount) inline; only resulting character observations are assigned. No numeric result assignment on this route.',
 limitation:'Same264 original inputs/D0..18/bothFIXED. Compare public inline ROUND result descriptor/BRS/display/STR with independently observed stored result and unchanged original controls. No expected arithmetic/text, TRANSFORM internal-call attribution, accepted formatter, atomic mapped-code proof or Windows parity.'};
const expression='ROUND(CFNDEC(CFNWIDTH(payload,40),18),decimalCount)';
const before=[`inlineType=VARTYPE(${expression})`,`inlineMeta=CFNMETA(${expression})`,`inlineBits=HexBytes(BINTOC(${expression},'BRS'))`,`inlineDisplay=HexBytes(TRANSFORM(${expression}))`,`inlineStr=HexBytes(STR(${expression},40,decimalCount))`].map(v=>'    '+v+'\n').join('');
let source=fs.readFileSync(path.join(fixture,'round-result.prg'),'utf8');
a.equal(source.split('    rounded='+expression).length,2);source=source.replace('    rounded='+expression,before+'    rounded='+expression);
source=source.replace('LOCAL roundedDisplay,roundedStr','LOCAL roundedDisplay,roundedStr\nLOCAL inlineType,inlineMeta,inlineBits,inlineDisplay,inlineStr');
const lines=source.split('\n'),indices=lines.flatMap((v,i)=>v.startsWith("     line=fixedMode+'|'")?[i]:[]);a.equal(indices.length,1);
lines[indices[0]]+="+'|'+inlineType+'|'+inlineMeta+'|'+inlineBits+'|'+inlineDisplay+'|'+inlineStr"+`+'|'+CFNMETA(${expression})+'|'+HexBytes(BINTOC(${expression},'BRS'))+'|'+HexBytes(TRANSFORM(${expression}))+'|'+HexBytes(STR(${expression},40,decimalCount))`;
a.equal((lines[indices[0]].match(/\+'\|'/g)||[]).length,34);
source=lines.join('\n').replaceAll('default-transform-round-result\\results.txt','default-transform-round-inline\\results.txt');a(source.includes('default-transform-round-inline\\results.txt'));
source=source.replace('independent public ROUND discriminator.','independent inline/stored public ROUND result discriminator.');
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({inputs:264,records:528,fields:35,inlineExpression:expression,sourceOnly:true}));
