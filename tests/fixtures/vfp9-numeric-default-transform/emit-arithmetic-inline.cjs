// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Route discriminator for an already REJECTED graph, never a formatter rule.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const parentRoot=path.join(__dirname,'round-inline-captures1'),parentHash='254b46dadbb71fa51b10e8439d51785b2c2aa3eb871c8c948fd3b8bdd3da4873';
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(parentRoot,'audit.json.gz'))));
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],parentHash);a.equal(c.createHash('sha256').update(z.gunzipSync(fs.readFileSync(path.join(parentRoot,n+'-results.txt.gz')))).digest('hex'),parentHash);}
const base=JSON.parse(fs.readFileSync(path.join(__dirname,'round-result-cases.json'))),prior=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-arithmetic-cases.json')));
const inputs=parent.observations.filter(r=>r.fixed==='OFF').map(r=>({index:r.index,setting:r.setting,bits:r.bits}));a.deepEqual(inputs,base.inputs);a.deepEqual(inputs,prior.inputs.map(({index,setting,bits})=>({index,setting,bits})));
const names=['powDown','bias','shifted','effectiveCount','powUp','scaled','raised','quantized'];a.deepEqual(names,prior.arithmeticFields);
const powDown='10^(-decimalCount)',bias=`0.0005*(${powDown})`,shifted=`ABS(payload)+(${bias})`,effectiveCount='MIN(decimalCount,15)',powUp=`10^(${effectiveCount})`,scaled=`(${shifted})*(${powUp})`,raised=`(${scaled})+0.5`,quantized=`INT(${raised})`;
const expressions=[powDown,bias,shifted,effectiveCount,powUp,scaled,raised,quantized];
for(const e of expressions){let depth=0;for(const ch of e){if(ch==='(')++depth;if(ch===')')--depth;a(depth>=0);}a.equal(depth,0);}
const inv={...base,storedParentSha256:parentHash,fieldsPerRecord:66,stageNames:names,stageExpressions:expressions,
 arithmeticRoute:'Each rejected-graph stage is independently evaluated inline for VARTYPE/CFNMETA/BINTOC before and CFNMETA/BINTOC after the CRT control; only character observations are assigned. No numeric intermediate assignment.',
 limitation:'Same264 original inputs/D0..18/bothFIXED, inline counterpart of earlier REJECTED scalar graph ONLY. Constants0.0005/0.5 and MIN(D,15) are diagnostic stimulus, NOT requirements, tolerance/cap/operation-order or TRANSFORM internals. Observe typed finite results without predicted bits/text; independent original and stored-ROUND controls. No accepted formatter/atomic image proof/Windows parity.'};
let source=fs.readFileSync(path.join(__dirname,'round-result.prg'),'utf8');
source=source.replace('LOCAL outputPath,','LOCAL ARRAY stageType[8],stageMeta[8],stageBits[8]\nLOCAL outputPath,');
const before=expressions.map((e,i)=>`    stageType[${i+1}]=VARTYPE(${e})\n    stageMeta[${i+1}]=CFNMETA(${e})\n    stageBits[${i+1}]=HexBytes(BINTOC(${e},'BRS'))\n`).join('');
const marker='    rounded=ROUND(CFNDEC(CFNWIDTH(payload,40),18),decimalCount)';a.equal(source.split(marker).length,2);source=source.replace(marker,before+marker);
const lines=source.split('\n'),indices=lines.flatMap((s,i)=>s.startsWith("     line=fixedMode+'|'")?[i]:[]);a.equal(indices.length,1);
const additions=expressions.map((e,i)=>`     line=line+'|'+stageType[${i+1}]+'|'+stageMeta[${i+1}]+'|'+stageBits[${i+1}]+'|'+CFNMETA(${e})+'|'+HexBytes(BINTOC(${e},'BRS'))`);
lines.splice(indices[0]+1,0,...additions);a.equal((lines[indices[0]].match(/\+'\|'/g)||[]).length+additions.reduce((n,s)=>n+(s.match(/\+'\|'/g)||[]).length,0),65);
source=lines.join('\n').replaceAll('default-transform-round-result\\results.txt','default-transform-arithmetic-inline\\results.txt');a(source.includes('default-transform-arithmetic-inline\\results.txt'));
source=source.replace('independent public ROUND discriminator.','inline evaluation of REJECTED arithmetic graph, observation ONLY.');
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({inputs:264,records:528,fields:66,stages:8,sourceOnly:true}));
