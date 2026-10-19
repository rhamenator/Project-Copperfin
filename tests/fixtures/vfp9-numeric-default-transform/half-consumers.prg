* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: public near-half consumers, no expected strings.
LOCAL ARRAY inputHex[6]
inputHex[1]='3FDFFFFFFFFFFFFF'
inputHex[2]='3FE0000000000000'
inputHex[3]='3FE0000000000001'
inputHex[4]='BFDFFFFFFFFFFFFF'
inputHex[5]='BFE0000000000000'
inputHex[6]='BFE0000000000001'
LOCAL outputPath,libraryPath,crtPath,index,fixedNo,fixedMode,payload,caught,line,total,failed
LOCAL inputMeta,inputBits,inputDisplay,inputStr,decType,decText,fixedType,fixedText,bindingBefore,roundType,roundMeta,roundBits,roundDisplay,roundStr,inputAfterMeta,inputAfterBits,inputAfterDisplay,inputAfterStr,bindingAfter,roundAfterType,roundAfterMeta,roundAfterBits,roundAfterDisplay,roundAfterStr
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-half-consumers\results.txt'
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
SET DECIMALS TO 0
FOR index=1 TO ALEN(inputHex)
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
   inputMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
   inputBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
   inputDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
   inputStr=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,0))
   decType=VARTYPE(SET('DECIMALS'))
   decText=HexBytes(TRANSFORM(SET('DECIMALS')))
   fixedType=VARTYPE(SET('FIXED'))
   fixedText=HexBytes(TRANSFORM(SET('FIXED')))
   bindingBefore=CFNFCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
   IF LEFT(bindingBefore,6)=='ERROR:'
    =STRTOFILE('ERROR-bindingBefore|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(bindingBefore)+CHR(13)+CHR(10),outputPath,1)
    SET LIBRARY TO
    RETURN
   ENDIF
   roundType=VARTYPE(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0))
   roundMeta=CFNMETA(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0))
   roundBits=HexBytes(BINTOC(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0),'BRS'))
   roundDisplay=HexBytes(TRANSFORM(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0)))
   roundStr=HexBytes(STR(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0),40,0))
   inputAfterMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
   inputAfterBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
   inputAfterDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
   inputAfterStr=HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,0))
   bindingAfter=CFNFCVT(CFNDEC(CFNWIDTH(payload,40),18),17)
   IF LEFT(bindingAfter,6)=='ERROR:'
    =STRTOFILE('ERROR-bindingAfter|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(bindingAfter)+CHR(13)+CHR(10),outputPath,1)
    SET LIBRARY TO
    RETURN
   ENDIF
   roundAfterType=VARTYPE(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0))
   roundAfterMeta=CFNMETA(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0))
   roundAfterBits=HexBytes(BINTOC(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0),'BRS'))
   roundAfterDisplay=HexBytes(TRANSFORM(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0)))
   roundAfterStr=HexBytes(STR(ROUND(CFNDEC(CFNWIDTH(payload,40),18),0),40,0))
   line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+inputHex[index]+'|0'+'|'+inputMeta+'|'+inputBits+'|'+inputDisplay+'|'+inputStr+'|'+decType+'|'+decText+'|'+fixedType+'|'+fixedText+'|'+bindingBefore+'|'+roundType+'|'+roundMeta+'|'+roundBits+'|'+roundDisplay+'|'+roundStr+'|'+inputAfterMeta+'|'+inputAfterBits+'|'+inputAfterDisplay+'|'+inputAfterStr+'|'+bindingAfter+'|'+roundAfterType+'|'+roundAfterMeta+'|'+roundAfterBits+'|'+roundAfterDisplay+'|'+roundAfterStr
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

FUNCTION Unhex
 LPARAMETERS value
 LOCAL result,position,high,low
 result=''
 FOR position=1 TO LEN(value) STEP 2
  high=AT(SUBSTR(value,position,1),'0123456789ABCDEF')-1
  low=AT(SUBSTR(value,position+1,1),'0123456789ABCDEF')-1
  result=result+CHR(high*16+low)
 ENDFOR
 RETURN result

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,position,byteValue
 result=''
 FOR position=1 TO LEN(value)
  byteValue=ASC(SUBSTR(value,position,1))
  result=result+SUBSTR('0123456789ABCDEF',INT(byteValue/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(byteValue,16)+1,1)
 ENDFOR
 RETURN result
