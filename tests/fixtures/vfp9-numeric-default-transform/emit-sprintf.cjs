// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: public fixed-format CRT stimuli only.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=__dirname,read=n=>fs.readFileSync(path.join(root,n)),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const archive='str-middle-captures1/',gzip=read(archive+'audit.json.gz'),sums=new Map(read(archive+'SHA256SUMS').toString().trim().split('\n').map(line=>{const i=line.indexOf('  ');return[line.slice(i+2),line.slice(0,i)];}));
a.equal(sha(gzip),sums.get('audit.json.gz'));const raw=z.gunzipSync(gzip),audit=JSON.parse(raw);a.equal(sha(raw),'57d15b3231e2fb881ab632479e456711005f4df106664064aeb3c7bd6a3c44f5');
const {plan}=require('./plan-sprintf-discriminator.cjs'),selected=plan(audit.observations);a.equal(selected.inputs,264);a.equal(selected.crtCalls,1056);a.deepEqual(selected.counts,[15,16,17,18]);
const crt=JSON.parse(read('crt-cases.json'));a.equal(crt.moduleSha256,'8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe');
const inv={requirement:'RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001',parentDecodedSha256:sha(raw),inputCount:264,recordsPerRun:1056,fieldsPerRecord:18,
 counts:selected.counts,inputs:selected.cases,modulePath:crt.modulePath,moduleSha256:crt.moduleSha256,moduleFileSize:crt.moduleFileSize,modulePe:crt.modulePe,runtimeEnvironment:crt.runtimeEnvironment,
 format:'%.*f',bufferBytes:2048,exports:['sprintf','_snprintf'],fixedMode:'OFF',vfpControlComparisons:2112,
 inputRoute:'Inline CFNDEC(CFNWIDTH(payload,40),18) at every numeric consumer; no stored numeric argument.',
 limitation:'Public MSVCR71 fixed-format comparison only. Retained OFF/ON VFP expected hex and fresh STR controls are observations, not an internal-call attribution, formatter acceptance, Windows parity or production algorithm.'};
const prior=read('crt.prg').toString(),functions=prior.slice(prior.indexOf('FUNCTION Unhex'));a(functions.startsWith('FUNCTION Unhex'));a.equal(prior.split('FUNCTION Unhex').length,2);
const header=prior.split('\n').slice(0,3).join('\n')+'\n* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: guarded public sprintf/_snprintf observations.\n';
const assignments=selected.cases.map(r=>`inputHex[${r.index}]='${r.bits}'\ntargetDecimals[${r.index}]=${r.setting}`).join('\n');
const source=header+`LOCAL ARRAY inputHex[264],targetDecimals[264]\n${assignments}`+String.raw`
LOCAL outputPath,libraryPath,crtPath,index,decimalCount,conversionCount,payload,argumentHex
LOCAL beforeMeta,beforeBits,beforeStr,beforeControl,crtResult,afterMeta,afterBits,afterStr,afterControl
LOCAL decimalType,decimalHex,fixedType,fixedHex,line,caught,total,failed
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-sprintf\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
crtPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-sprintf\sprintf-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
TRY
 SET LIBRARY TO (libraryPath) ADDITIVE
 SET LIBRARY TO (crtPath) ADDITIVE
CATCH TO caught
 =STRTOFILE('ERROR-LOAD|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)+CHR(13)+CHR(10),outputPath,1)
 SET LIBRARY TO
 RETURN
ENDTRY
total=0
FOR index=1 TO ALEN(inputHex)
 decimalCount=targetDecimals[index]
 SET DECIMALS TO decimalCount
 SET FIXED OFF
 argumentHex=inputHex[index]
 payload=CTOBIN(Unhex(argumentHex),'BS')
 FOR conversionCount=15 TO 18
  failed=.F.
  TRY
   beforeMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
   beforeBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
   beforeStr=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,conversionCount))
   beforeControl=CFNFCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
   IF LEFT(beforeControl,6)=='ERROR:'
    failed=.T.
    line='ERROR-CONTROL-BEFORE|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(beforeControl)
   ELSE
    crtResult=CFNSPRTF(CFNDEC(CFNWIDTH(payload,40),18),conversionCount)
    IF LEFT(crtResult,6)=='ERROR:'
     failed=.T.
     line='ERROR-SPRINTF|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(crtResult)
    ELSE
     afterMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
     afterBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
     afterStr=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,conversionCount))
     afterControl=CFNFCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
     decimalType=VARTYPE(SET('DECIMALS'))
     decimalHex=HexBytes(TRANSFORM(SET('DECIMALS')))
     fixedType=VARTYPE(SET('FIXED'))
     fixedHex=HexBytes(TRANSFORM(SET('FIXED')))
     IF LEFT(afterControl,6)=='ERROR:'
      failed=.T.
      line='ERROR-CONTROL-AFTER|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(afterControl)
     ELSE
      line=ALLTRIM(STR(index,6,0))+'|'+argumentHex+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+ALLTRIM(STR(conversionCount,4,0))+'|'+beforeMeta+'|'+beforeBits+'|'+beforeStr+'|'+beforeControl+'|'+crtResult+'|'+afterMeta+'|'+afterBits+'|'+afterStr+'|'+afterControl+'|'+decimalType+'|'+decimalHex+'|'+fixedType+'|'+fixedHex+'|OFF'
     ENDIF
    ENDIF
   ENDIF
  CATCH TO caught
   failed=.T.
   line='ERROR-RECORD|'+ALLTRIM(STR(index,6,0))+'|'+argumentHex+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
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
else process.stdout.write(JSON.stringify({sourceOnly:true,nativeInvoked:false,inputs:inv.inputCount,records:inv.recordsPerRun,fields:inv.fieldsPerRecord,counts:inv.counts})+'\n');
