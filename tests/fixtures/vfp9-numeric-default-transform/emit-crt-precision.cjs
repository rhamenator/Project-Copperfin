// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// ORIGINAL-only finite precision discriminator; no expected formatting rule.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),crypto=require('node:crypto'),a=require('node:assert/strict');
const base=JSON.parse(fs.readFileSync(path.join(__dirname,'crt-cases.json'))),parentRoot=path.join(__dirname,'crt-nativeonly-captures1');
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(parentRoot,'audit.json.gz'))));
const parentHash='60a1b217bbe305ccb61012cb4324d7bb944d9e413bd920803debd17e36e8c54d';
a.equal(parent.firstSha256,parentHash);a.equal(parent.repeatSha256,parentHash);a.equal(parent.rawByteEqual,true);
for(const name of ['first','repeat'])a.equal(crypto.createHash('sha256').update(z.gunzipSync(fs.readFileSync(path.join(parentRoot,name+'-results.txt.gz')))).digest('hex'),parentHash);
const inputs=parent.observations.filter(r=>r.fixed==='ON'&&r.route==='ORIGINAL'&&r.kind==='E'&&r.count===17).map(r=>({index:r.index,setting:r.setting,bits:r.bits}));
a.equal(inputs.length,264);a.deepEqual(inputs,base.inputs.map(({index,setting,bits})=>({index,setting,bits})));
for(const r of inputs){a(Number.isInteger(r.setting)&&r.setting>=0&&r.setting<=18);a.match(r.bits,/^[0-9A-F]{16}$/);}
const inv={requirement:base.requirement,parentNativeSha256:parentHash,inputCount:264,recordsPerRun:1584,width:40,scale:18,inputs,
 modulePath:base.modulePath,moduleSha256:base.moduleSha256,moduleFileSize:base.moduleFileSize,modulePe:base.modulePe,
 runtimeEnvironment:base.runtimeEnvironment,inputsAreExplicitPayloadStimuli:true,countOffsets:[1,2,3],experimentalCountMax:21,
 limitation:'ORIGINAL-only explicit264 payloads; public_fcvt at applied D+1/+2/+3 counts1..21, no assumed VFP extra-digit algorithm/cap/rounding rule. Exact sampled module identity/controls, first-error stop. No atomic mapped-code proof/internal-call attribution/Windows parity/production linkage.'};
const prior=fs.readFileSync(path.join(__dirname,'crt.prg'),'utf8'),functions=prior.slice(prior.indexOf('FUNCTION Unhex'));
a(functions.startsWith('FUNCTION Unhex'));a.equal(prior.split('FUNCTION Unhex').length,2);
const source=prior.split('\n').slice(0,3).join('\n')+'\n* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: diagnostic counts, NOT expected text.\n'+
 'LOCAL ARRAY inputHex[264],targetDecimals[264]\n'+inputs.map(r=>`inputHex[${r.index}]='${r.bits}'\ntargetDecimals[${r.index}]=${r.setting}`).join('\n')+String.raw`
LOCAL outputPath,libraryPath,crtPath,index,decimalCount,fixedNo,fixedMode
LOCAL payload,argumentHex,countNo,conversionCount,crtResult,line,caught,total
LOCAL beforeMeta,beforeBits,beforeDisplay,beforeStr,failed
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-crt-precision\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
crtPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-crt-precision\crt-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
SET LIBRARY TO (crtPath) ADDITIVE
total=0
FOR index=1 TO ALEN(inputHex)
 decimalCount=targetDecimals[index]
 SET DECIMALS TO decimalCount
 argumentHex=inputHex[index]
 payload=CTOBIN(Unhex(argumentHex),'BS')
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  beforeMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
  beforeBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
  beforeDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
  beforeStr=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,decimalCount))
  FOR countNo=1 TO 3
   conversionCount=decimalCount+countNo
   failed=.F.
   TRY
    crtResult=CFNFCVT(payload,conversionCount)
    IF LEFT(crtResult,6)=='ERROR:'
     failed=.T.
     line='ERROR-OBSERVER|'+ALLTRIM(STR(index,6,0))+'|'+argumentHex+'|'+HexBytes(crtResult)
    ELSE
     line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+argumentHex+'|'+ALLTRIM(STR(decimalCount,4,0))+'|ORIGINAL|'+argumentHex+'|F|'+ALLTRIM(STR(conversionCount,4,0))+'|'+beforeMeta+'|'+beforeBits+'|'+beforeDisplay+'|'+beforeStr+'|'+VARTYPE(SET('DECIMALS'))+'|'+HexBytes(TRANSFORM(SET('DECIMALS')))+'|'+VARTYPE(SET('FIXED'))+'|'+HexBytes(TRANSFORM(SET('FIXED')))+'|'+crtResult+'|'+CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))+'|'+HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))+'|'+HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
    ENDIF
   CATCH TO caught
    failed=.T.
    line='ERROR-'+fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+argumentHex+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
   ENDTRY
   =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   IF failed
    SET LIBRARY TO
    RETURN
   ENDIF
   total=total+1
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

`+functions;
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({inputs:264,records:1584,countOffsets:[1,2,3],max:21}));
