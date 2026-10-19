// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: public STR stimuli, NOT expected text.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const read=n=>fs.readFileSync(path.join(__dirname,n)),base=JSON.parse(read('gcvt-cases.json')),parent=JSON.parse(z.gunzipSync(read('gcvt-captures1/audit.json.gz'))),digest='0b7950e0725e76d15742428b0b03614563ddcc15a1ac088a28f207b7788e7712';
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],digest);a.equal(c.createHash('sha256').update(z.gunzipSync(read('gcvt-captures1/'+n+'-results.txt.gz'))).digest('hex'),digest);}
for(const k of ['documentedFieldsRepeatEqual','originalParentAndTypedControls','exactRuntimeIdentity','guardedBufferAndGrammar'])a.equal(parent[k],true);
const inputs=parent.observations.filter(r=>r.fixed==='OFF'&&r.count===17).map(({index,setting,bits})=>({index,setting,bits}));a.equal(inputs.length,264);a.deepEqual(inputs,base.inputs);
const inv={...base,parentNativeSha256:digest,recordsPerRun:528,fieldsPerRecord:24,counts:[15,18],inputs,
 controlCount:17,observerSha256:'0b1fcd2ba81b95631cb52e6ecd0d4afa721b450a967402f504d1852a49e9e9ac',
 limitation:'Public STR at fixed15/18 decimals on same ORIGINAL INLINE N40/18 inputs across applied settings/bothFIXED; counts are stimuli, not inferred VFP caps. Unchanged parent original/current-D STR/F17/G17 controls required before+after. New strings are native observations, no computed expectations/rounding/tolerance/formatter acceptance or internal-call/Windows/atomic mapped-code proof.'};
const prior=read('gcvt.prg').toString(),functions=prior.slice(prior.indexOf('FUNCTION Unhex'));a.equal(prior.split('FUNCTION Unhex').length,2);
const source=prior.split('\n').slice(0,3).join('\n')+'\n* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: fixed-count public STR observations.\nLOCAL ARRAY inputHex[264],targetDecimals[264]\n'+inputs.map(r=>`inputHex[${r.index}]='${r.bits}'\ntargetDecimals[${r.index}]=${r.setting}`).join('\n')+String.raw`
LOCAL outputPath,libraryPath,crtPath,index,decimalCount,fixedNo,fixedMode
LOCAL payload,argumentHex,line,caught,total,failed
LOCAL beforeMeta,beforeBits,beforeDisplay,beforeStr,beforeF,beforeG,afterF,afterG
LOCAL str15Type,str15Hex,str18Type,str18Hex,afterMeta,afterBits,afterDisplay,afterStr
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-str-fixed\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
crtPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-gcvt-captures1\gcvt-probe.fll'
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
  failed=.F.
  TRY
   beforeMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
   beforeBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
   beforeDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
   beforeStr=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,decimalCount))
   beforeF=CFNFCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
   IF LEFT(beforeF,6)=='ERROR:'
    failed=.T.
    line='ERROR-F-BEFORE|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(beforeF)
   ELSE
    beforeG=CFNGCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
    IF LEFT(beforeG,6)=='ERROR:'
     failed=.T.
     line='ERROR-G-BEFORE|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(beforeG)
    ELSE
     str15Type=VARTYPE(STR(CFNDEC(CFNWIDTH(payload,40),18),40,15))
     str15Hex=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,15))
     str18Type=VARTYPE(STR(CFNDEC(CFNWIDTH(payload,40),18),40,18))
     str18Hex=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,18))
     afterMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
     afterBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
     afterDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
     afterStr=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,decimalCount))
     afterF=CFNFCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
     IF LEFT(afterF,6)=='ERROR:'
      failed=.T.
      line='ERROR-F-AFTER|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(afterF)
     ELSE
      afterG=CFNGCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
      IF LEFT(afterG,6)=='ERROR:'
       failed=.T.
       line='ERROR-G-AFTER|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(afterG)
      ELSE
       line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+argumentHex+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+beforeMeta+'|'+beforeBits+'|'+beforeDisplay+'|'+beforeStr+'|'+VARTYPE(SET('DECIMALS'))+'|'+HexBytes(TRANSFORM(SET('DECIMALS')))+'|'+VARTYPE(SET('FIXED'))+'|'+HexBytes(TRANSFORM(SET('FIXED')))+'|'+beforeF+'|'+beforeG+'|'+str15Type+'|'+str15Hex+'|'+str18Type+'|'+str18Hex+'|'+afterMeta+'|'+afterBits+'|'+afterDisplay+'|'+afterStr+'|'+afterF+'|'+afterG
      ENDIF
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
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

`+functions;
if(process.argv[2]==='source')process.stdout.write(source);else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');else console.log(JSON.stringify({sourceOnly:true,inputs:264,records:528,fields:24,counts:inv.counts}));
