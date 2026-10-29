// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: independent SOURCE checks, not native results.
const fs=require('node:fs'),p=require('node:path'),a=require('node:assert/strict');
const inv=JSON.parse(fs.readFileSync(p.join(__dirname,'original-setting-consumer-cases.json'))),source=fs.readFileSync(p.join(__dirname,'original-setting-consumer.prg'),'utf8');
const expected=[];for(const [parentIndex,bits]of [[159,'3D2C1E8DBBA38061'],[160,'3D2C1E8DBBA38062'],[165,'BD2C1E8DBBA38061'],[166,'BD2C1E8DBBA38062']])for(const setting of [12,14])for(const scale of [13,14])expected.push({index:expected.length+1,parentIndex,originalBits:bits,bits,setting,scale,width:40,kind:'scale'+scale});
let checks=0;const check=(actual,want)=>{a.deepEqual(actual,want);++checks;};
check(inv.inputs,expected);check(inv.inputCount,16);check(inv.recordsPerRun,32);check(inv.fieldsPerRecord,42);check(inv.settings,[12,14]);check(inv.scales,[13,14]);check(inv.width,40);check(inv.runtimeEnvironment,'WINEDLLOVERRIDES=msvcr71=n');check(inv.moduleSha256,'8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe');check(inv.parentNativeSha256,'254b46dadbb71fa51b10e8439d51785b2c2aa3eb871c8c948fd3b8bdd3da4873');
for(const r of expected)for(const [n,v]of [['inputOriginal',`'${r.bits}'`],['inputHex',`'${r.bits}'`],['inputParent',r.parentIndex],['inputD',r.setting],['inputScale',r.scale],['inputKind',`'${r.kind}'`],['inputWidth',40]])check(source.split('\n').filter(l=>l===`${n}[${r.index}]=${v}`).length,1);
const orig='CFNDEC(CFNWIDTH(originalPayload,40),18)',rounded=`ROUND(${orig},inputD[index])`,target='CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index])';
for(const [prefix,expr,typed,bound]of [['original',orig,false,true],['round',rounded,false,false],['target',target,true,true]])for(const suffix of ['','After']){
 const n=prefix+suffix;for(const [name,e]of [[n+'Meta',`CFNMETA(${expr})`],[n+'Bits',`HexBytes(BINTOC(${expr},'BRS'))`],[n+'Display',`HexBytes(TRANSFORM(${expr}))`],[n+'Str',`HexBytes(STR(${expr},40,inputD[index]))`],...(typed?[[n+'Type',`VARTYPE(${expr})`]]:[]),...(bound?[[n+'Binding',`CFNFCVT(${expr},17)`]]:[])])check(source.split('\n').filter(l=>l===`   ${name}=${e}`).length,1);
 if(bound)check(source.includes(`ERROR-${n}Binding|`),true);
}
for(const s of ['decimalCount=inputD[index]','SET DECIMALS TO decimalCount','SET FIXED OFF','SET FIXED ON','ERROR-LOAD|','FOR index=1 TO ALEN(inputHex)','FOR fixedNo=1 TO 2'])check(source.includes(s),true);
const line=source.split('\n').filter(l=>l.startsWith('   line=fixedMode'));check(line.length,1);check((line[0].match(/\+'\|'/g)||[]).length,41);
check(source.includes('default-transform-original-setting-consumer\\results.txt'),true);
check(source.split('\n').filter(l=>/^\s*\w+=(?:CFNDEC|CFNWIDTH|ROUND)\(/i.test(l)),[]);
// No prediction of target or changed-setting parent/ROUND display values.
check(inv.limitation.includes('not copied D13 expectations'),true);
console.log(JSON.stringify({sourceOnly:true,sourceChecks:checks,inputs:16,records:32,fields:42,nativeInvoked:false,readerQualified:false,newControlValuesUnpredicted:true}));
