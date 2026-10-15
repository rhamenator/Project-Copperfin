* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-UNARY-EXPRESSION-CONTEXT-001: bounded context recovery, not a compiler model.
LOCAL ARRAY expressions[24], settings[2]
expressions[1]='-&cZeroExp'
expressions[2]='-(-&cZeroExp)'
expressions[3]='-&cMinusZeroExp'
expressions[4]='-(&cZeroExp)'
expressions[5]='-oZero.z'
expressions[6]='-(-oZero.z)'
expressions[7]='-aZero[1]'
expressions[8]='-(-aZero[1])'
expressions[9]='-cfUnaryZero.z'
expressions[10]='-(-cfUnaryZero.z)'
expressions[11]='-IIF(.T.,0.00,ZeroValue())'
expressions[12]='-IIF(.F.,ZeroValue(),0.00)'
expressions[13]='-ICASE(.T.,0.00,1.00)'
expressions[14]='-MAX(0.00,0.00)'
expressions[15]='-MIN(0.00,0.00)'
expressions[16]='-ReturnArgument(0.00)'
expressions[17]='-ReturnArgument(-0.00)'
expressions[18]='-(-ReturnArgument(0.00))'
expressions[19]='-EVALUATE(cZeroExp)'
expressions[20]='-EVALUATE(cMinusZeroExp)'
expressions[21]='-(0.00)'
expressions[22]='-(-0.00)'
expressions[23]='-nZero'
expressions[24]='-nMinusZero'
settings[1]=2
settings[2]=14
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught
PRIVATE nSource,nZero,nMinusZero,nDivisor,cZeroExp,cMinusZeroExp,oZero,aZero
DIMENSION aZero[1]
outputPath='Z:\home\rich\.codex\worktrees\numeric-unary-zero-context-5611\Project-Copperfin\build\numeric-unary-expression-native\primary-results.txt'
libraryPath='Z:\home\rich\.local\share\Trash\files\listitem-column-numeric-5611-build\listitem-column-native\cause\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
oZero=CREATEOBJECT('Custom')
=oZero.AddProperty('z',0.00)
CREATE CURSOR cfUnaryZero (z N(8,2))
APPEND BLANK
REPLACE z WITH 0.00
cZeroExp='0.00'
cMinusZeroExp='-0.00'
total=0
FOR settingNo=1 TO ALEN(settings)
 decimalCount=settings[settingNo]
 SET DECIMALS TO decimalCount
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  nSource=5.00
  nDivisor=2.00
  aZero[1]=0.00
  nZero=0.00
  nMinusZero=-0.00
  FOR index=1 TO ALEN(expressions)
   expression=expressions[index]
   TRY
    observation=CFNMETA(EVALUATE(expression))+'|'+HexBytes(BINTOC(EVALUATE(expression),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE(expression)))
    line='EVAL|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
   CATCH TO caught
    line='ERROR-EVAL|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
   ENDTRY
   =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   total=total+1
   TRY
    observation=CompiledObservation(index)
    line='COMPILED|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
   CATCH TO caught
    line='ERROR-COMPILED|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
   ENDTRY
   =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   total=total+1
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
USE IN cfUnaryZero
SET LIBRARY TO

FUNCTION ReturnArgument
 LPARAMETERS argument
 RETURN argument
ENDFUNC

FUNCTION CompiledObservation
 LPARAMETERS index
 DO CASE
 CASE index=1
  RETURN CFNMETA(-&cZeroExp)+'|'+HexBytes(BINTOC(-&cZeroExp,'BRS'))+'|'+HexBytes(TRANSFORM(-&cZeroExp))
 CASE index=2
  RETURN CFNMETA(-(-&cZeroExp))+'|'+HexBytes(BINTOC(-(-&cZeroExp),'BRS'))+'|'+HexBytes(TRANSFORM(-(-&cZeroExp)))
 CASE index=3
  RETURN CFNMETA(-&cMinusZeroExp)+'|'+HexBytes(BINTOC(-&cMinusZeroExp,'BRS'))+'|'+HexBytes(TRANSFORM(-&cMinusZeroExp))
 CASE index=4
  RETURN CFNMETA(-(&cZeroExp))+'|'+HexBytes(BINTOC(-(&cZeroExp),'BRS'))+'|'+HexBytes(TRANSFORM(-(&cZeroExp)))
 CASE index=5
  RETURN CFNMETA(-oZero.z)+'|'+HexBytes(BINTOC(-oZero.z,'BRS'))+'|'+HexBytes(TRANSFORM(-oZero.z))
 CASE index=6
  RETURN CFNMETA(-(-oZero.z))+'|'+HexBytes(BINTOC(-(-oZero.z),'BRS'))+'|'+HexBytes(TRANSFORM(-(-oZero.z)))
 CASE index=7
  RETURN CFNMETA(-aZero[1])+'|'+HexBytes(BINTOC(-aZero[1],'BRS'))+'|'+HexBytes(TRANSFORM(-aZero[1]))
 CASE index=8
  RETURN CFNMETA(-(-aZero[1]))+'|'+HexBytes(BINTOC(-(-aZero[1]),'BRS'))+'|'+HexBytes(TRANSFORM(-(-aZero[1])))
 CASE index=9
  RETURN CFNMETA(-cfUnaryZero.z)+'|'+HexBytes(BINTOC(-cfUnaryZero.z,'BRS'))+'|'+HexBytes(TRANSFORM(-cfUnaryZero.z))
 CASE index=10
  RETURN CFNMETA(-(-cfUnaryZero.z))+'|'+HexBytes(BINTOC(-(-cfUnaryZero.z),'BRS'))+'|'+HexBytes(TRANSFORM(-(-cfUnaryZero.z)))
 CASE index=11
  RETURN CFNMETA(-IIF(.T.,0.00,ZeroValue()))+'|'+HexBytes(BINTOC(-IIF(.T.,0.00,ZeroValue()),'BRS'))+'|'+HexBytes(TRANSFORM(-IIF(.T.,0.00,ZeroValue())))
 CASE index=12
  RETURN CFNMETA(-IIF(.F.,ZeroValue(),0.00))+'|'+HexBytes(BINTOC(-IIF(.F.,ZeroValue(),0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-IIF(.F.,ZeroValue(),0.00)))
 CASE index=13
  RETURN CFNMETA(-ICASE(.T.,0.00,1.00))+'|'+HexBytes(BINTOC(-ICASE(.T.,0.00,1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-ICASE(.T.,0.00,1.00)))
 CASE index=14
  RETURN CFNMETA(-MAX(0.00,0.00))+'|'+HexBytes(BINTOC(-MAX(0.00,0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-MAX(0.00,0.00)))
 CASE index=15
  RETURN CFNMETA(-MIN(0.00,0.00))+'|'+HexBytes(BINTOC(-MIN(0.00,0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-MIN(0.00,0.00)))
 CASE index=16
  RETURN CFNMETA(-ReturnArgument(0.00))+'|'+HexBytes(BINTOC(-ReturnArgument(0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-ReturnArgument(0.00)))
 CASE index=17
  RETURN CFNMETA(-ReturnArgument(-0.00))+'|'+HexBytes(BINTOC(-ReturnArgument(-0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-ReturnArgument(-0.00)))
 CASE index=18
  RETURN CFNMETA(-(-ReturnArgument(0.00)))+'|'+HexBytes(BINTOC(-(-ReturnArgument(0.00)),'BRS'))+'|'+HexBytes(TRANSFORM(-(-ReturnArgument(0.00))))
 CASE index=19
  RETURN CFNMETA(-EVALUATE(cZeroExp))+'|'+HexBytes(BINTOC(-EVALUATE(cZeroExp),'BRS'))+'|'+HexBytes(TRANSFORM(-EVALUATE(cZeroExp)))
 CASE index=20
  RETURN CFNMETA(-EVALUATE(cMinusZeroExp))+'|'+HexBytes(BINTOC(-EVALUATE(cMinusZeroExp),'BRS'))+'|'+HexBytes(TRANSFORM(-EVALUATE(cMinusZeroExp)))
 CASE index=21
  RETURN CFNMETA(-(0.00))+'|'+HexBytes(BINTOC(-(0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00)))
 CASE index=22
  RETURN CFNMETA(-(-0.00))+'|'+HexBytes(BINTOC(-(-0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.00)))
 CASE index=23
  RETURN CFNMETA(-nZero)+'|'+HexBytes(BINTOC(-nZero,'BRS'))+'|'+HexBytes(TRANSFORM(-nZero))
 CASE index=24
  RETURN CFNMETA(-nMinusZero)+'|'+HexBytes(BINTOC(-nMinusZero,'BRS'))+'|'+HexBytes(TRANSFORM(-nMinusZero))
 ENDCASE
 ERROR 11
ENDFUNC

FUNCTION ZeroValue
 RETURN 0.00
ENDFUNC

FUNCTION HexBytes
 LPARAMETERS source
 LOCAL result,index,byte,digits
 result=''
 digits='0123456789ABCDEF'
 FOR index=1 TO LEN(source)
  byte=ASC(SUBSTR(source,index,1))
  result=result+SUBSTR(digits,INT(byte/16)+1,1)+SUBSTR(digits,MOD(byte,16)+1,1)
 ENDFOR
 RETURN result
ENDFUNC
