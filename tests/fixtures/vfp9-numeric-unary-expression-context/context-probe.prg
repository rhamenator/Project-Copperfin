* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-UNARY-EXPRESSION-CONTEXT-001: bounded context recovery, not a compiler model.
LOCAL ARRAY expressions[48], settings[2]
expressions[1]='-(0.00)'
expressions[2]='-(-0.00)'
expressions[3]='-nZero'
expressions[4]='-(-nZero)'
expressions[5]='-(nZero-nZero)'
expressions[6]='-(0.00-0.00)'
expressions[7]='-ABS(0.00)'
expressions[8]='-ABS(nZero)'
expressions[9]='-IIF(.T.,0.00,1.00)'
expressions[10]='-IIF(.T.,nZero,1.00)'
expressions[11]='-ZeroValue()'
expressions[12]='-VAL("0.00")'
expressions[13]='-(-VAL("0.00"))'
expressions[14]='-(0.00+0.00)'
expressions[15]='-(0.00*1.00)'
expressions[16]='-(0.00/1.00)'
expressions[17]='-(1.00-1.00)'
expressions[18]='-(nSource-nSource)'
expressions[19]='-(-nMinusZero)'
expressions[20]='-(0.00+nZero)'
expressions[21]='-(nZero+0.00)'
expressions[22]='-EVALUATE("0.00")'
expressions[23]='-EVALUATE("-0.00")'
expressions[24]='-(-(-0.00))'
expressions[25]='5/2'
expressions[26]='(5/2)'
expressions[27]='-(5/2)'
expressions[28]='-5/2'
expressions[29]='+(5/2)'
expressions[30]='-((5/2))'
expressions[31]='-(+(5/2))'
expressions[32]='-(-5/2)'
expressions[33]='5.0/2'
expressions[34]='-(5.0/2)'
expressions[35]='5/2.0'
expressions[36]='-(5/2.0)'
expressions[37]='5.00/2.00'
expressions[38]='-(5.00/2.00)'
expressions[39]='5.00-2.00'
expressions[40]='-(5.00-2.00)'
expressions[41]='nSource/2'
expressions[42]='-(nSource/2)'
expressions[43]='5/nDivisor'
expressions[44]='-(5/nDivisor)'
expressions[45]='-IIF(.T.,5/2,1.00)'
expressions[46]='-EVALUATE("5/2")'
expressions[47]='-ABS(5/2)'
expressions[48]='-(1.00+ZeroValue())'
settings[1]=2
settings[2]=14
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught
PRIVATE nSource,nZero,nMinusZero,nDivisor
outputPath='Z:\home\rich\.codex\worktrees\numeric-unary-zero-context-5611\Project-Copperfin\build\numeric-unary-expression-native\context-results.txt'
libraryPath='Z:\home\rich\.local\share\Trash\files\listitem-column-numeric-5611-build\listitem-column-native\cause\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
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
SET LIBRARY TO

FUNCTION CompiledObservation
 LPARAMETERS index
 DO CASE
 CASE index=1
  RETURN CFNMETA(-(0.00))+'|'+HexBytes(BINTOC(-(0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00)))
 CASE index=2
  RETURN CFNMETA(-(-0.00))+'|'+HexBytes(BINTOC(-(-0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.00)))
 CASE index=3
  RETURN CFNMETA(-nZero)+'|'+HexBytes(BINTOC(-nZero,'BRS'))+'|'+HexBytes(TRANSFORM(-nZero))
 CASE index=4
  RETURN CFNMETA(-(-nZero))+'|'+HexBytes(BINTOC(-(-nZero),'BRS'))+'|'+HexBytes(TRANSFORM(-(-nZero)))
 CASE index=5
  RETURN CFNMETA(-(nZero-nZero))+'|'+HexBytes(BINTOC(-(nZero-nZero),'BRS'))+'|'+HexBytes(TRANSFORM(-(nZero-nZero)))
 CASE index=6
  RETURN CFNMETA(-(0.00-0.00))+'|'+HexBytes(BINTOC(-(0.00-0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00-0.00)))
 CASE index=7
  RETURN CFNMETA(-ABS(0.00))+'|'+HexBytes(BINTOC(-ABS(0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-ABS(0.00)))
 CASE index=8
  RETURN CFNMETA(-ABS(nZero))+'|'+HexBytes(BINTOC(-ABS(nZero),'BRS'))+'|'+HexBytes(TRANSFORM(-ABS(nZero)))
 CASE index=9
  RETURN CFNMETA(-IIF(.T.,0.00,1.00))+'|'+HexBytes(BINTOC(-IIF(.T.,0.00,1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-IIF(.T.,0.00,1.00)))
 CASE index=10
  RETURN CFNMETA(-IIF(.T.,nZero,1.00))+'|'+HexBytes(BINTOC(-IIF(.T.,nZero,1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-IIF(.T.,nZero,1.00)))
 CASE index=11
  RETURN CFNMETA(-ZeroValue())+'|'+HexBytes(BINTOC(-ZeroValue(),'BRS'))+'|'+HexBytes(TRANSFORM(-ZeroValue()))
 CASE index=12
  RETURN CFNMETA(-VAL("0.00"))+'|'+HexBytes(BINTOC(-VAL("0.00"),'BRS'))+'|'+HexBytes(TRANSFORM(-VAL("0.00")))
 CASE index=13
  RETURN CFNMETA(-(-VAL("0.00")))+'|'+HexBytes(BINTOC(-(-VAL("0.00")),'BRS'))+'|'+HexBytes(TRANSFORM(-(-VAL("0.00"))))
 CASE index=14
  RETURN CFNMETA(-(0.00+0.00))+'|'+HexBytes(BINTOC(-(0.00+0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00+0.00)))
 CASE index=15
  RETURN CFNMETA(-(0.00*1.00))+'|'+HexBytes(BINTOC(-(0.00*1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00*1.00)))
 CASE index=16
  RETURN CFNMETA(-(0.00/1.00))+'|'+HexBytes(BINTOC(-(0.00/1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00/1.00)))
 CASE index=17
  RETURN CFNMETA(-(1.00-1.00))+'|'+HexBytes(BINTOC(-(1.00-1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00-1.00)))
 CASE index=18
  RETURN CFNMETA(-(nSource-nSource))+'|'+HexBytes(BINTOC(-(nSource-nSource),'BRS'))+'|'+HexBytes(TRANSFORM(-(nSource-nSource)))
 CASE index=19
  RETURN CFNMETA(-(-nMinusZero))+'|'+HexBytes(BINTOC(-(-nMinusZero),'BRS'))+'|'+HexBytes(TRANSFORM(-(-nMinusZero)))
 CASE index=20
  RETURN CFNMETA(-(0.00+nZero))+'|'+HexBytes(BINTOC(-(0.00+nZero),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00+nZero)))
 CASE index=21
  RETURN CFNMETA(-(nZero+0.00))+'|'+HexBytes(BINTOC(-(nZero+0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(nZero+0.00)))
 CASE index=22
  RETURN CFNMETA(-EVALUATE("0.00"))+'|'+HexBytes(BINTOC(-EVALUATE("0.00"),'BRS'))+'|'+HexBytes(TRANSFORM(-EVALUATE("0.00")))
 CASE index=23
  RETURN CFNMETA(-EVALUATE("-0.00"))+'|'+HexBytes(BINTOC(-EVALUATE("-0.00"),'BRS'))+'|'+HexBytes(TRANSFORM(-EVALUATE("-0.00")))
 CASE index=24
  RETURN CFNMETA(-(-(-0.00)))+'|'+HexBytes(BINTOC(-(-(-0.00)),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(-0.00))))
 CASE index=25
  RETURN CFNMETA(5/2)+'|'+HexBytes(BINTOC(5/2,'BRS'))+'|'+HexBytes(TRANSFORM(5/2))
 CASE index=26
  RETURN CFNMETA((5/2))+'|'+HexBytes(BINTOC((5/2),'BRS'))+'|'+HexBytes(TRANSFORM((5/2)))
 CASE index=27
  RETURN CFNMETA(-(5/2))+'|'+HexBytes(BINTOC(-(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/2)))
 CASE index=28
  RETURN CFNMETA(-5/2)+'|'+HexBytes(BINTOC(-5/2,'BRS'))+'|'+HexBytes(TRANSFORM(-5/2))
 CASE index=29
  RETURN CFNMETA(+(5/2))+'|'+HexBytes(BINTOC(+(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(+(5/2)))
 CASE index=30
  RETURN CFNMETA(-((5/2)))+'|'+HexBytes(BINTOC(-((5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/2))))
 CASE index=31
  RETURN CFNMETA(-(+(5/2)))+'|'+HexBytes(BINTOC(-(+(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(+(5/2))))
 CASE index=32
  RETURN CFNMETA(-(-5/2))+'|'+HexBytes(BINTOC(-(-5/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-5/2)))
 CASE index=33
  RETURN CFNMETA(5.0/2)+'|'+HexBytes(BINTOC(5.0/2,'BRS'))+'|'+HexBytes(TRANSFORM(5.0/2))
 CASE index=34
  RETURN CFNMETA(-(5.0/2))+'|'+HexBytes(BINTOC(-(5.0/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.0/2)))
 CASE index=35
  RETURN CFNMETA(5/2.0)+'|'+HexBytes(BINTOC(5/2.0,'BRS'))+'|'+HexBytes(TRANSFORM(5/2.0))
 CASE index=36
  RETURN CFNMETA(-(5/2.0))+'|'+HexBytes(BINTOC(-(5/2.0),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/2.0)))
 CASE index=37
  RETURN CFNMETA(5.00/2.00)+'|'+HexBytes(BINTOC(5.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(5.00/2.00))
 CASE index=38
  RETURN CFNMETA(-(5.00/2.00))+'|'+HexBytes(BINTOC(-(5.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.00/2.00)))
 CASE index=39
  RETURN CFNMETA(5.00-2.00)+'|'+HexBytes(BINTOC(5.00-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(5.00-2.00))
 CASE index=40
  RETURN CFNMETA(-(5.00-2.00))+'|'+HexBytes(BINTOC(-(5.00-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.00-2.00)))
 CASE index=41
  RETURN CFNMETA(nSource/2)+'|'+HexBytes(BINTOC(nSource/2,'BRS'))+'|'+HexBytes(TRANSFORM(nSource/2))
 CASE index=42
  RETURN CFNMETA(-(nSource/2))+'|'+HexBytes(BINTOC(-(nSource/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(nSource/2)))
 CASE index=43
  RETURN CFNMETA(5/nDivisor)+'|'+HexBytes(BINTOC(5/nDivisor,'BRS'))+'|'+HexBytes(TRANSFORM(5/nDivisor))
 CASE index=44
  RETURN CFNMETA(-(5/nDivisor))+'|'+HexBytes(BINTOC(-(5/nDivisor),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/nDivisor)))
 CASE index=45
  RETURN CFNMETA(-IIF(.T.,5/2,1.00))+'|'+HexBytes(BINTOC(-IIF(.T.,5/2,1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-IIF(.T.,5/2,1.00)))
 CASE index=46
  RETURN CFNMETA(-EVALUATE("5/2"))+'|'+HexBytes(BINTOC(-EVALUATE("5/2"),'BRS'))+'|'+HexBytes(TRANSFORM(-EVALUATE("5/2")))
 CASE index=47
  RETURN CFNMETA(-ABS(5/2))+'|'+HexBytes(BINTOC(-ABS(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(-ABS(5/2)))
 CASE index=48
  RETURN CFNMETA(-(1.00+ZeroValue()))+'|'+HexBytes(BINTOC(-(1.00+ZeroValue()),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00+ZeroValue())))
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
