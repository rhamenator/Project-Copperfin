* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: bounded D13 ORIGINAL decimal-metadata14..17 routes, no expected strings.
LOCAL ARRAY inputOriginal[16],inputHex[16],inputD[16],inputScale[16],inputParent[16],inputKind[16],inputWidth[16]
inputOriginal[1]='3D2C1E8DBBA38061'
inputHex[1]='3D2C1E8DBBA38061'
inputD[1]=13
inputScale[1]=14
inputParent[1]=159
inputKind[1]='scale14'
inputWidth[1]=40
inputOriginal[2]='3D2C1E8DBBA38061'
inputHex[2]='3D2C1E8DBBA38061'
inputD[2]=13
inputScale[2]=15
inputParent[2]=159
inputKind[2]='scale15'
inputWidth[2]=40
inputOriginal[3]='3D2C1E8DBBA38061'
inputHex[3]='3D2C1E8DBBA38061'
inputD[3]=13
inputScale[3]=16
inputParent[3]=159
inputKind[3]='scale16'
inputWidth[3]=40
inputOriginal[4]='3D2C1E8DBBA38061'
inputHex[4]='3D2C1E8DBBA38061'
inputD[4]=13
inputScale[4]=17
inputParent[4]=159
inputKind[4]='scale17'
inputWidth[4]=40
inputOriginal[5]='3D2C1E8DBBA38062'
inputHex[5]='3D2C1E8DBBA38062'
inputD[5]=13
inputScale[5]=14
inputParent[5]=160
inputKind[5]='scale14'
inputWidth[5]=40
inputOriginal[6]='3D2C1E8DBBA38062'
inputHex[6]='3D2C1E8DBBA38062'
inputD[6]=13
inputScale[6]=15
inputParent[6]=160
inputKind[6]='scale15'
inputWidth[6]=40
inputOriginal[7]='3D2C1E8DBBA38062'
inputHex[7]='3D2C1E8DBBA38062'
inputD[7]=13
inputScale[7]=16
inputParent[7]=160
inputKind[7]='scale16'
inputWidth[7]=40
inputOriginal[8]='3D2C1E8DBBA38062'
inputHex[8]='3D2C1E8DBBA38062'
inputD[8]=13
inputScale[8]=17
inputParent[8]=160
inputKind[8]='scale17'
inputWidth[8]=40
inputOriginal[9]='BD2C1E8DBBA38061'
inputHex[9]='BD2C1E8DBBA38061'
inputD[9]=13
inputScale[9]=14
inputParent[9]=165
inputKind[9]='scale14'
inputWidth[9]=40
inputOriginal[10]='BD2C1E8DBBA38061'
inputHex[10]='BD2C1E8DBBA38061'
inputD[10]=13
inputScale[10]=15
inputParent[10]=165
inputKind[10]='scale15'
inputWidth[10]=40
inputOriginal[11]='BD2C1E8DBBA38061'
inputHex[11]='BD2C1E8DBBA38061'
inputD[11]=13
inputScale[11]=16
inputParent[11]=165
inputKind[11]='scale16'
inputWidth[11]=40
inputOriginal[12]='BD2C1E8DBBA38061'
inputHex[12]='BD2C1E8DBBA38061'
inputD[12]=13
inputScale[12]=17
inputParent[12]=165
inputKind[12]='scale17'
inputWidth[12]=40
inputOriginal[13]='BD2C1E8DBBA38062'
inputHex[13]='BD2C1E8DBBA38062'
inputD[13]=13
inputScale[13]=14
inputParent[13]=166
inputKind[13]='scale14'
inputWidth[13]=40
inputOriginal[14]='BD2C1E8DBBA38062'
inputHex[14]='BD2C1E8DBBA38062'
inputD[14]=13
inputScale[14]=15
inputParent[14]=166
inputKind[14]='scale15'
inputWidth[14]=40
inputOriginal[15]='BD2C1E8DBBA38062'
inputHex[15]='BD2C1E8DBBA38062'
inputD[15]=13
inputScale[15]=16
inputParent[15]=166
inputKind[15]='scale16'
inputWidth[15]=40
inputOriginal[16]='BD2C1E8DBBA38062'
inputHex[16]='BD2C1E8DBBA38062'
inputD[16]=13
inputScale[16]=17
inputParent[16]=166
inputKind[16]='scale17'
inputWidth[16]=40
LOCAL outputPath,libraryPath,crtPath,index,fixedNo,fixedMode,originalPayload,targetPayload,decimalCount,caught,line,total,failed
LOCAL originalMeta,originalBits,originalDisplay,originalStr,originalBinding,roundMeta,roundBits,roundDisplay,roundStr,decType,decText,fixedType,fixedText,targetType,targetMeta,targetBits,targetDisplay,targetStr,targetBinding,originalAfterMeta,originalAfterBits,originalAfterDisplay,originalAfterStr,originalAfterBinding,roundAfterMeta,roundAfterBits,roundAfterDisplay,roundAfterStr,targetAfterType,targetAfterMeta,targetAfterBits,targetAfterDisplay,targetAfterStr,targetAfterBinding
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-original-scale-middle-consumer\results.txt'
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
