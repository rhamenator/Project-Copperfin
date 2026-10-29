// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: stimuli only, no expected text.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const read=n=>fs.readFileSync(path.join(__dirname,n)),parentRoot='arithmetic-inline-captures1/';
const parent=JSON.parse(z.gunzipSync(read(parentRoot+'audit.json.gz'))),parentHash='1979626f4fec992924791da5cbc89ed3abef1583810690300fff1fd3cf95ea32';
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],parentHash);a.equal(c.createHash('sha256').update(z.gunzipSync(read(parentRoot+n+'-results.txt.gz'))).digest('hex'),parentHash);}
a.equal(parent.exactRuntimeIdentity,true);a.equal(parent.typedResultAndControls,true);a.equal(parent.documentedFieldsRepeatEqual,true);
const base=JSON.parse(read('crt-cases.json')),inputs=parent.observations.filter(r=>r.fixed==='OFF').map(({index,setting,bits})=>({index,setting,bits}));
a.equal(inputs.length,264);a.deepEqual(inputs,base.inputs.map(({index,setting,bits})=>({index,setting,bits})));
for(const [i,r]of inputs.entries()){a.equal(r.index,i+1);a(Number.isInteger(r.setting)&&r.setting>=0&&r.setting<=18);a.match(r.bits,/^[0-9A-F]{16}$/);a.notEqual((BigInt('0x'+r.bits)>>52n)&2047n,2047n);}
const inv={requirement:base.requirement,parentNativeSha256:parentHash,inputCount:264,recordsPerRun:1584,fieldsPerRecord:20,width:40,scale:18,counts:[15,16,17],inputs,
 modulePath:base.modulePath,moduleSha256:base.moduleSha256,moduleFileSize:base.moduleFileSize,modulePe:base.modulePe,runtimeEnvironment:base.runtimeEnvironment,
 inputRoute:'Inline CFNDEC(CFNWIDTH(payload,40),18) at every numeric consumer; no stored numeric argument.',
 limitation:'Explicit ORIGINAL payloads/count15..17 are stimuli, not a VFP precision rule. Native _gcvt text is observation only; unchanged original parent display/STR and public _fcvt17 before/after controls required. Own2048-byte guarded buffer, bounded127-byte ASCII report. No formatter acceptance, internal-call attribution, atomic mapped-code proof or Windows parity.'};
const prior=read('crt.prg').toString(),functions=prior.slice(prior.indexOf('FUNCTION Unhex'));a(functions.startsWith('FUNCTION Unhex'));a.equal(prior.split('FUNCTION Unhex').length,2);
const source=prior.split('\n').slice(0,3).join('\n')+'\n* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: guarded public _gcvt observations.\n'+
 'LOCAL ARRAY inputHex[264],targetDecimals[264]\n'+inputs.map(r=>`inputHex[${r.index}]='${r.bits}'\ntargetDecimals[${r.index}]=${r.setting}`).join('\n')+String.raw`
LOCAL outputPath,libraryPath,crtPath,index,decimalCount,fixedNo,fixedMode
LOCAL payload,argumentHex,conversionCount,crtResult,line,caught,total,failed
LOCAL beforeMeta,beforeBits,beforeDisplay,beforeStr,beforeControl,afterControl
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-gcvt\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
crtPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-gcvt\gcvt-probe.fll'
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
  FOR conversionCount=15 TO 17
   failed=.F.
   TRY
    beforeMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
    beforeBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
    beforeDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
    beforeStr=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,decimalCount))
    beforeControl=CFNFCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
    IF LEFT(beforeControl,6)=='ERROR:'
     failed=.T.
     line='ERROR-CONTROL-BEFORE|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(beforeControl)
    ELSE
     crtResult=CFNGCVT(CFNDEC(CFNWIDTH(payload,40),18),conversionCount)
     IF LEFT(crtResult,6)=='ERROR:'
      failed=.T.
      line='ERROR-GCVT|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(crtResult)
     ELSE
      afterControl=CFNFCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
      IF LEFT(afterControl,6)=='ERROR:'
       failed=.T.
       line='ERROR-CONTROL-AFTER|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(afterControl)
      ELSE
       line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+argumentHex+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+beforeMeta+'|'+beforeBits+'|'+beforeDisplay+'|'+beforeStr+'|'+VARTYPE(SET('DECIMALS'))+'|'+HexBytes(TRANSFORM(SET('DECIMALS')))+'|'+VARTYPE(SET('FIXED'))+'|'+HexBytes(TRANSFORM(SET('FIXED')))+'|'+beforeControl+'|'+ALLTRIM(STR(conversionCount,4,0))+'|'+crtResult+'|'+CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))+'|'+HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))+'|'+HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))+'|'+HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,decimalCount))+'|'+afterControl
      ENDIF
     ENDIF
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
else console.log(JSON.stringify({sourceOnly:true,inputs:264,records:1584,fields:20,counts:inv.counts}));
