* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: bounded scale interventions, no expected strings.
LOCAL ARRAY inputOriginal[20],inputHex[20],inputD[20],inputScale[20],inputParent[20],inputKind[20]
inputOriginal[1]='3CC203398FC21737'
inputHex[1]='3CC203398FC21737'
inputD[1]=16
inputScale[1]=18
inputParent[1]=193
inputKind[1]='original'
inputOriginal[2]='3CC203398FC21737'
inputHex[2]='3CC203398FC21737'
inputD[2]=16
inputScale[2]=16
inputParent[2]=193
inputKind[2]='original'
inputOriginal[3]='3CC203398FC21737'
inputHex[3]='3CC203AF9EE75616'
inputD[3]=16
inputScale[3]=18
inputParent[3]=193
inputKind[3]='round'
inputOriginal[4]='3CC203398FC21737'
inputHex[4]='3CC203AF9EE75616'
inputD[4]=16
inputScale[4]=16
inputParent[4]=193
inputKind[4]='round'
inputOriginal[5]='BCC203398FC21737'
inputHex[5]='BCC203398FC21737'
inputD[5]=16
inputScale[5]=18
inputParent[5]=199
inputKind[5]='original'
inputOriginal[6]='BCC203398FC21737'
inputHex[6]='BCC203398FC21737'
inputD[6]=16
inputScale[6]=16
inputParent[6]=199
inputKind[6]='original'
inputOriginal[7]='BCC203398FC21737'
inputHex[7]='BCC203AF9EE75616'
inputD[7]=16
inputScale[7]=18
inputParent[7]=199
inputKind[7]='round'
inputOriginal[8]='BCC203398FC21737'
inputHex[8]='BCC203AF9EE75616'
inputD[8]=16
inputScale[8]=16
inputParent[8]=199
inputKind[8]='round'
inputOriginal[9]='3CC203A3D096CFC9'
inputHex[9]='3CC203A3D096CFC9'
inputD[9]=17
inputScale[9]=18
inputParent[9]=205
inputKind[9]='original'
inputOriginal[10]='3CC203A3D096CFC9'
inputHex[10]='3CC203A3D096CFC9'
inputD[10]=17
inputScale[10]=17
inputParent[10]=205
inputKind[10]='original'
inputOriginal[11]='3CC203A3D096CFC9'
inputHex[11]='3CC203AF9EE75616'
inputD[11]=17
inputScale[11]=18
inputParent[11]=205
inputKind[11]='round'
inputOriginal[12]='3CC203A3D096CFC9'
inputHex[12]='3CC203AF9EE75616'
inputD[12]=17
inputScale[12]=17
inputParent[12]=205
inputKind[12]='round'
inputOriginal[13]='BCC203A3D096CFC9'
inputHex[13]='BCC203A3D096CFC9'
inputD[13]=17
inputScale[13]=18
inputParent[13]=211
inputKind[13]='original'
inputOriginal[14]='BCC203A3D096CFC9'
inputHex[14]='BCC203A3D096CFC9'
inputD[14]=17
inputScale[14]=17
inputParent[14]=211
inputKind[14]='original'
inputOriginal[15]='BCC203A3D096CFC9'
inputHex[15]='BCC203AF9EE75616'
inputD[15]=17
inputScale[15]=18
inputParent[15]=211
inputKind[15]='round'
inputOriginal[16]='BCC203A3D096CFC9'
inputHex[16]='BCC203AF9EE75616'
inputD[16]=17
inputScale[16]=17
inputParent[16]=211
inputKind[16]='round'
inputOriginal[17]='3CC203AE70AC1571'
inputHex[17]='3CC203AE70AC1571'
inputD[17]=18
inputScale[17]=18
inputParent[17]=217
inputKind[17]='original'
inputOriginal[18]='3CC203AE70AC1571'
inputHex[18]='3CC203AF9EE75616'
inputD[18]=18
inputScale[18]=18
inputParent[18]=217
inputKind[18]='round'
inputOriginal[19]='BCC203AE70AC1571'
inputHex[19]='BCC203AE70AC1571'
inputD[19]=18
inputScale[19]=18
inputParent[19]=223
inputKind[19]='original'
inputOriginal[20]='BCC203AE70AC1571'
inputHex[20]='BCC203AF9EE75616'
inputD[20]=18
inputScale[20]=18
inputParent[20]=223
inputKind[20]='round'
LOCAL outputPath,libraryPath,crtPath,index,fixedNo,fixedMode,originalPayload,targetPayload,decimalCount,caught,line,total,failed
LOCAL originalMeta,originalBits,originalDisplay,originalStr,originalBinding,roundMeta,roundBits,roundDisplay,roundStr,decType,decText,fixedType,fixedText,targetType,targetMeta,targetBits,targetDisplay,targetStr,targetBinding,originalAfterMeta,originalAfterBits,originalAfterDisplay,originalAfterStr,originalAfterBinding,roundAfterMeta,roundAfterBits,roundAfterDisplay,roundAfterStr,targetAfterType,targetAfterMeta,targetAfterBits,targetAfterDisplay,targetAfterStr,targetAfterBinding
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-scale-intervention\results.txt'
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
 originalPayload=CTOBIN(Unhex(inputOriginal[index]),'BS')
 targetPayload=CTOBIN(Unhex(inputHex[index]),'BS')
 decimalCount=inputD[index]
 SET DECIMALS TO decimalCount
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
   originalMeta=CFNMETA(CFNDEC(CFNWIDTH(originalPayload,40),18))
   originalBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(originalPayload,40),18),'BRS'))
   originalDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(originalPayload,40),18)))
   originalStr=HexBytes(STR(CFNDEC(CFNWIDTH(originalPayload,40),18),40,inputD[index]))
   originalBinding=CFNFCVT(CFNDEC(CFNWIDTH(originalPayload,40),18),17)
   IF LEFT(originalBinding,6)=='ERROR:'
    =STRTOFILE('ERROR-originalBinding|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(originalBinding)+CHR(13)+CHR(10),outputPath,1)
    SET LIBRARY TO
    RETURN
   ENDIF
   roundMeta=CFNMETA(ROUND(CFNDEC(CFNWIDTH(originalPayload,40),18),inputD[index]))
   roundBits=HexBytes(BINTOC(ROUND(CFNDEC(CFNWIDTH(originalPayload,40),18),inputD[index]),'BRS'))
   roundDisplay=HexBytes(TRANSFORM(ROUND(CFNDEC(CFNWIDTH(originalPayload,40),18),inputD[index])))
   roundStr=HexBytes(STR(ROUND(CFNDEC(CFNWIDTH(originalPayload,40),18),inputD[index]),40,inputD[index]))
   decType=VARTYPE(SET('DECIMALS'))
   decText=HexBytes(TRANSFORM(SET('DECIMALS')))
   fixedType=VARTYPE(SET('FIXED'))
   fixedText=HexBytes(TRANSFORM(SET('FIXED')))
   targetType=VARTYPE(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]))
   targetMeta=CFNMETA(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]))
   targetBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]),'BRS'))
   targetDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index])))
   targetStr=HexBytes(STR(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]),40,inputD[index]))
   targetBinding=CFNFCVT(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]),17)
   IF LEFT(targetBinding,6)=='ERROR:'
    =STRTOFILE('ERROR-targetBinding|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(targetBinding)+CHR(13)+CHR(10),outputPath,1)
    SET LIBRARY TO
    RETURN
   ENDIF
   originalAfterMeta=CFNMETA(CFNDEC(CFNWIDTH(originalPayload,40),18))
   originalAfterBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(originalPayload,40),18),'BRS'))
   originalAfterDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(originalPayload,40),18)))
   originalAfterStr=HexBytes(STR(CFNDEC(CFNWIDTH(originalPayload,40),18),40,inputD[index]))
   originalAfterBinding=CFNFCVT(CFNDEC(CFNWIDTH(originalPayload,40),18),17)
   IF LEFT(originalAfterBinding,6)=='ERROR:'
    =STRTOFILE('ERROR-originalAfterBinding|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(originalAfterBinding)+CHR(13)+CHR(10),outputPath,1)
    SET LIBRARY TO
    RETURN
   ENDIF
   roundAfterMeta=CFNMETA(ROUND(CFNDEC(CFNWIDTH(originalPayload,40),18),inputD[index]))
   roundAfterBits=HexBytes(BINTOC(ROUND(CFNDEC(CFNWIDTH(originalPayload,40),18),inputD[index]),'BRS'))
   roundAfterDisplay=HexBytes(TRANSFORM(ROUND(CFNDEC(CFNWIDTH(originalPayload,40),18),inputD[index])))
   roundAfterStr=HexBytes(STR(ROUND(CFNDEC(CFNWIDTH(originalPayload,40),18),inputD[index]),40,inputD[index]))
   targetAfterType=VARTYPE(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]))
   targetAfterMeta=CFNMETA(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]))
   targetAfterBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]),'BRS'))
   targetAfterDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index])))
   targetAfterStr=HexBytes(STR(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]),40,inputD[index]))
   targetAfterBinding=CFNFCVT(CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]),17)
   IF LEFT(targetAfterBinding,6)=='ERROR:'
    =STRTOFILE('ERROR-targetAfterBinding|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(targetAfterBinding)+CHR(13)+CHR(10),outputPath,1)
    SET LIBRARY TO
    RETURN
   ENDIF
   line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+ALLTRIM(STR(inputParent[index],6,0))+'|'+inputOriginal[index]+'|'+inputHex[index]+'|'+ALLTRIM(STR(inputD[index],6,0))+'|'+ALLTRIM(STR(inputScale[index],6,0))+'|'+inputKind[index]+'|'+originalMeta+'|'+originalBits+'|'+originalDisplay+'|'+originalStr+'|'+originalBinding+'|'+roundMeta+'|'+roundBits+'|'+roundDisplay+'|'+roundStr+'|'+decType+'|'+decText+'|'+fixedType+'|'+fixedText+'|'+targetType+'|'+targetMeta+'|'+targetBits+'|'+targetDisplay+'|'+targetStr+'|'+targetBinding+'|'+originalAfterMeta+'|'+originalAfterBits+'|'+originalAfterDisplay+'|'+originalAfterStr+'|'+originalAfterBinding+'|'+roundAfterMeta+'|'+roundAfterBits+'|'+roundAfterDisplay+'|'+roundAfterStr+'|'+targetAfterType+'|'+targetAfterMeta+'|'+targetAfterBits+'|'+targetAfterDisplay+'|'+targetAfterStr+'|'+targetAfterBinding
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
