// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Public ROUND result observations only; no assumed TRANSFORM implementation.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const base=JSON.parse(fs.readFileSync(path.join(__dirname,'crt-precision-cases.json'))),parentRoot=path.join(__dirname,'crt-precision-captures1'),parentHash='ba189979b2d7cd67ab15f1d719af586f815f28f98427d30344629760f1fbabd3';
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(parentRoot,'audit.json.gz'))));
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],parentHash);a.equal(crypto.createHash('sha256').update(z.gunzipSync(fs.readFileSync(path.join(parentRoot,n+'-results.txt.gz')))).digest('hex'),parentHash);}
const inputs=parent.observations.filter(r=>r.fixed==='ON'&&r.count===r.setting+1).map(r=>({index:r.index,setting:r.setting,bits:r.bits}));
a.equal(inputs.length,264);a.deepEqual(inputs,base.inputs);
const inv={requirement:base.requirement,parentNativeSha256:parentHash,inputCount:264,recordsPerRun:528,width:40,scale:18,inputs,
 modulePath:base.modulePath,moduleSha256:base.moduleSha256,moduleFileSize:base.moduleFileSize,modulePe:base.modulePe,runtimeEnvironment:base.runtimeEnvironment,
 observerSha256:'a4d38aaecf67837c06b0d640746bc44b56b126d697cd0519c1b25c1b7934b090',
 limitation:'Independent public ROUND(payload,D) at D0..18 on explicit ORIGINAL payloads/metadata40scale18; typed result metadata/BRS/display/STR observed, NOT TRANSFORM internal-call attribution or accepted rounding algorithm. Existing bound CRT17 controls bracket each public call; no CRT count0, new compiler/observer, atomic image proof or Windows parity.'};
const prior=fs.readFileSync(path.join(__dirname,'crt-precision.prg'),'utf8'),functions=prior.slice(prior.indexOf('FUNCTION Unhex'));a.equal(prior.split('FUNCTION Unhex').length,2);
const source=prior.split('\n').slice(0,3).join('\n')+'\n* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: independent public ROUND discriminator.\nLOCAL ARRAY inputHex[264],targetDecimals[264]\n'+inputs.map(r=>`inputHex[${r.index}]='${r.bits}'\ntargetDecimals[${r.index}]=${r.setting}`).join('\n')+String.raw`
LOCAL outputPath,libraryPath,crtPath,index,decimalCount,fixedNo,fixedMode
LOCAL payload,argument,rounded,bindingBefore,bindingAfter,failed,line,caught,total
LOCAL beforeMeta,beforeBits,beforeDisplay,beforeStr,roundedMeta,roundedBits
LOCAL roundedDisplay,roundedStr
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-round-result\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
crtPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-crt-precision-captures1\crt-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
SET LIBRARY TO (crtPath) ADDITIVE
total=0
FOR index=1 TO ALEN(inputHex)
 decimalCount=targetDecimals[index]
 SET DECIMALS TO decimalCount
 payload=CTOBIN(Unhex(inputHex[index]),'BS')
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  failed=.F.
  TRY
   argument=CFNDEC(CFNWIDTH(payload,40),18)
   beforeMeta=CFNMETA(argument)
   beforeBits=HexBytes(BINTOC(argument,'BRS'))
   beforeDisplay=HexBytes(TRANSFORM(argument))
   beforeStr=HexBytes(STR(argument,40,decimalCount))
   bindingBefore=CFNFCVT(argument,17)
   IF LEFT(bindingBefore,6)=='ERROR:'
    failed=.T.
    line='ERROR-BINDING-BEFORE|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(bindingBefore)
   ELSE
    rounded=ROUND(argument,decimalCount)
    roundedMeta=CFNMETA(rounded)
    roundedBits=HexBytes(BINTOC(rounded,'BRS'))
    roundedDisplay=HexBytes(TRANSFORM(rounded))
    roundedStr=HexBytes(STR(rounded,40,decimalCount))
    bindingAfter=CFNFCVT(argument,17)
    IF LEFT(bindingAfter,6)=='ERROR:'
     failed=.T.
     line='ERROR-BINDING-AFTER|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(bindingAfter)
    ELSE
     line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+inputHex[index]+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+beforeMeta+'|'+beforeBits+'|'+beforeDisplay+'|'+beforeStr+'|'+VARTYPE(SET('DECIMALS'))+'|'+HexBytes(TRANSFORM(SET('DECIMALS')))+'|'+VARTYPE(SET('FIXED'))+'|'+HexBytes(TRANSFORM(SET('FIXED')))+'|'+bindingBefore+'|'+VARTYPE(rounded)+'|'+roundedMeta+'|'+roundedBits+'|'+roundedDisplay+'|'+roundedStr+'|'+CFNMETA(argument)+'|'+HexBytes(BINTOC(argument,'BRS'))+'|'+HexBytes(TRANSFORM(argument))+'|'+bindingAfter+'|'+CFNMETA(rounded)+'|'+HexBytes(BINTOC(rounded,'BRS'))+'|'+HexBytes(TRANSFORM(rounded))+'|'+HexBytes(STR(rounded,40,decimalCount))
    ENDIF
   ENDIF
  CATCH TO caught
   failed=.T.
   line='ERROR-'+fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
  ENDTRY
  =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
  IF failed
   SET LIBRARY TO
   RETURN
  ENDIF
  total=total+1
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

`+functions;
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({inputs:264,records:528,publicRoundDecimals:[0,18],controlCrtCount:17}));
