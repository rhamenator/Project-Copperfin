// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native-only conversion stimulus from QUALIFIED input/intermediate bits.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict');
const native=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-arithmetic-captures/audit.json.gz'))));
a.equal(native.firstSha256,'2917ff9167e4a2752520614776580d08dda8fa4ba1520615154912bd937bd1c9');
a.equal(native.repeatSha256,native.firstSha256);a.equal(native.rawByteEqual,true);
const inputs=native.observations.filter(r=>r.fixed==='ON').map(r=>({
    index:r.index,setting:r.setting,bits:r.bits,shiftedBits:r.arithmetic.shifted}));
a.equal(inputs.length,264);a.equal(inputs.filter(r=>r.setting===0).length,12);
const inv={requirement:native.requirement,parentNativeSha256:native.firstSha256,
    inputCount:264,recordsPerRun:4176,width:40,scale:18,inputs,
    modulePath:'C:\\windows\\syswow64\\msvcr71.dll',
    moduleSha256:'8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe',
    limitation:'Observe installed32-bit CRT conversion of original/qualified shifted bits, not recomputed model inputs. Both FIXED modes, applied fcvt count1..18 (D0 omitted), ecvt counts15/16/17. No DLL load/replacement/CW modification, inferred VFP call attribution, expected formatter or production linkage.'};
const prior=fs.readFileSync(path.join(__dirname,'rounding-arithmetic.prg'),'utf8');
const functions=prior.slice(prior.indexOf('FUNCTION Unhex'));
a(functions.startsWith('FUNCTION Unhex'));a.equal(prior.split('FUNCTION Unhex').length,2);
const source=prior.split('\n').slice(0,3).join('\n')+'\n* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: native CRT boundary observations, not formatter attribution.\n'+
    'LOCAL ARRAY inputHex[264],shiftedHex[264],targetDecimals[264]\n'+
    inputs.map(r=>`inputHex[${r.index}]='${r.bits}'\nshiftedHex[${r.index}]='${r.shiftedBits}'\ntargetDecimals[${r.index}]=${r.setting}`).join('\n')+String.raw`
LOCAL outputPath,libraryPath,crtPath,index,decimalCount,fixedNo,fixedMode,targetNo,route
LOCAL payload,argumentHex,kindNo,kind,countNo,conversionCount,crtResult,line,caught,total
LOCAL beforeMeta,beforeBits,beforeDisplay,beforeStr
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-crt\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
crtPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-crt\crt-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
SET LIBRARY TO (crtPath) ADDITIVE
total=0
FOR index=1 TO ALEN(inputHex)
 decimalCount=targetDecimals[index]
 SET DECIMALS TO decimalCount
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  FOR targetNo=1 TO 2
   IF targetNo=1
    argumentHex=inputHex[index]
    route='ORIGINAL'
   ELSE
    argumentHex=shiftedHex[index]
    route='SHIFTED'
   ENDIF
   payload=CTOBIN(Unhex(argumentHex),'BS')
   beforeMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
   beforeBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
   beforeDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
   beforeStr=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,decimalCount))
   FOR kindNo=1 TO 2
    IF kindNo=1
     kind='F'
    ELSE
     kind='E'
    ENDIF
    FOR countNo=1 TO 3
     IF kindNo=1
      IF decimalCount=0 OR countNo>1
       LOOP
      ENDIF
      conversionCount=decimalCount
     ELSE
      conversionCount=14+countNo
     ENDIF
     TRY
      IF kindNo=1
       crtResult=CFNFCVT(payload,conversionCount)
      ELSE
       crtResult=CFNECVT(payload,conversionCount)
      ENDIF
      line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+inputHex[index]+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+route+'|'+argumentHex+'|'+kind+'|'+ALLTRIM(STR(conversionCount,4,0))+'|'+beforeMeta+'|'+beforeBits+'|'+beforeDisplay+'|'+beforeStr+'|'+VARTYPE(SET('DECIMALS'))+'|'+HexBytes(TRANSFORM(SET('DECIMALS')))+'|'+VARTYPE(SET('FIXED'))+'|'+HexBytes(TRANSFORM(SET('FIXED')))+'|'+crtResult+'|'+CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))+'|'+HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))+'|'+HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
     CATCH TO caught
      line='ERROR-'+fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+argumentHex+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
     ENDTRY
     =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
     total=total+1
    ENDFOR
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

`+functions;
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({inputs:264,records:4176}));
