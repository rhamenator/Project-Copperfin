// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: new STR16/17 stimuli, exact15/18 controls.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const read=n=>fs.readFileSync(path.join(__dirname,n)),base=JSON.parse(read('str-fixed-cases.json')),parent=JSON.parse(z.gunzipSync(read('str-fixed-captures1/audit.json.gz'))),digest='89a420760a2332430158427778908a952d84966a6d4eee14f090c1a9a6332192';
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],digest);a.equal(c.createHash('sha256').update(z.gunzipSync(read('str-fixed-captures1/'+n+'-results.txt.gz'))).digest('hex'),digest);}
a.equal(parent.recordsPerRun,528);for(const k of ['documentedFieldsRepeatEqual','originalAndNativeParentControls','inlineMetadataAndBRSExact','publicStrTypesAndWidths'])a.equal(parent[k],true);
const inputs=parent.observations.filter(r=>r.fixed==='OFF').map(({index,setting,bits})=>({index,setting,bits}));a.equal(inputs.length,264);a.deepEqual(inputs,base.inputs);
const inv={...base,parentNativeSha256:digest,fieldsPerRecord:32,counts:[16,17],parentStrCounts:[15,18],inputs,
 limitation:'New explicit fixed STR16/17 on same264 ORIGINAL INLINE N40/18/bothFIXED/applied D0..18; prior STR15/18 plus original/F17/G17 are exact parent controls. Not a full settings Cartesian experiment. Counts are stimuli, not inferred caps. No computed strings, rounding, tolerance, formatter acceptance, internal-call/Windows/atomic mapped-code proof.'};
let source=read('str-fixed.prg').toString();a(source.includes('LOCAL str15Type,str15Hex,str18Type,str18Hex,afterMeta'));
source=source.replace('LOCAL str15Type,str15Hex,str18Type,str18Hex,afterMeta','LOCAL str16Type,str16Hex,str17Type,str17Hex\nLOCAL str15AfterType,str15AfterHex,str18AfterType,str18AfterHex\nLOCAL str15Type,str15Hex,str18Type,str18Hex,afterMeta');
const marker='     str18Hex=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,18))';a.equal(source.split(marker).length,2);
source=source.replace(marker,marker+'\n'+[16,17].flatMap(n=>[`     str${n}Type=VARTYPE(STR(CFNDEC(CFNWIDTH(payload,40),18),40,${n}))`,`     str${n}Hex=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,${n}))`]).join('\n')+'\n'+[15,18].flatMap(n=>[`     str${n}AfterType=VARTYPE(STR(CFNDEC(CFNWIDTH(payload,40),18),40,${n}))`,`     str${n}AfterHex=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,${n}))`]).join('\n'));
const lines=source.split('\n'),indices=lines.flatMap((l,i)=>l.startsWith("       line=fixedMode+'|'")?[i]:[]);a.equal(indices.length,1);a.equal((lines[indices[0]].match(/\+'\|'/g)||[]).length,23);
lines[indices[0]]+="+'|'+str16Type+'|'+str16Hex+'|'+str17Type+'|'+str17Hex+'|'+str15AfterType+'|'+str15AfterHex+'|'+str18AfterType+'|'+str18AfterHex";a.equal((lines[indices[0]].match(/\+'\|'/g)||[]).length,31);
source=lines.join('\n').replaceAll('default-transform-str-fixed\\results.txt','default-transform-str-middle\\results.txt').replace('fixed-count public STR observations.','fixed STR16/17 observations; accepted STR15/18 parent controls.');a(source.includes('default-transform-str-middle\\results.txt'));
if(process.argv[2]==='source')process.stdout.write(source);else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');else console.log(JSON.stringify({sourceOnly:true,inputs:264,records:528,fields:32,counts:inv.counts,parentStrCounts:inv.parentStrCounts}));
