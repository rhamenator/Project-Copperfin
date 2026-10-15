* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-INTEGER-DESCRIPTOR-001: native Integer prerequisite recovery.
LOCAL ARRAY expressions[48], settings[2]
expressions[1]='0'
expressions[2]='1'
expressions[3]='5'
expressions[4]='9'
expressions[5]='10'
expressions[6]='99'
expressions[7]='100'
expressions[8]='0005'
expressions[9]='0000000005'
expressions[10]='00000000005'
expressions[11]='2147483646'
expressions[12]='2147483647'
expressions[13]='2147483648'
expressions[14]='4294967295'
expressions[15]='4294967296'
expressions[16]='-0'
expressions[17]='-1'
expressions[18]='-2147483647'
expressions[19]='-2147483648'
expressions[20]='-2147483649'
expressions[21]='-(-0)'
expressions[22]='-(-5)'
expressions[23]='-(-2147483647)'
expressions[24]='+0'
expressions[25]='+(5)'
expressions[26]='(2147483647)'
expressions[27]='INT(-2147483648.00)'
expressions[28]='-INT(-2147483648.00)'
expressions[29]='INT(2147483647.00)'
expressions[30]='-INT(2147483647.00)'
expressions[31]='-iZero'
expressions[32]='-(-iZero)'
expressions[33]='-iMin'
expressions[34]='-(-iMin)'
expressions[35]='iMin'
expressions[36]='iZero'
expressions[37]='0000000000'
expressions[38]='00000000000'
expressions[39]='00000000000000000000'
expressions[40]='-0005'
expressions[41]='-0000000005'
expressions[42]='-00000000005'
expressions[43]='-(-0005)'
expressions[44]='+0005'
expressions[45]='0.0'
expressions[46]='-0.0'
expressions[47]='2147483647.0'
expressions[48]='-2147483647.0'
settings[1]=2
settings[2]=14
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught
PRIVATE iFive,iTwo,nFive,nTwo,nOne,iOne,iZero,iMin
outputPath='Z:\home\rich\.codex\worktrees\numeric-division-context-5611\Project-Copperfin\build\numeric-division-native\integer-results.txt'
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
  iFive=5
  iTwo=2
  nFive=5.00
  nTwo=2.00
  nOne=1.00
  iOne=1
  iZero=0
  iMin=INT(-2147483648.00)
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
  RETURN CFNMETA(0)+'|'+HexBytes(BINTOC(0,'BRS'))+'|'+HexBytes(TRANSFORM(0))
 CASE index=2
  RETURN CFNMETA(1)+'|'+HexBytes(BINTOC(1,'BRS'))+'|'+HexBytes(TRANSFORM(1))
 CASE index=3
  RETURN CFNMETA(5)+'|'+HexBytes(BINTOC(5,'BRS'))+'|'+HexBytes(TRANSFORM(5))
 CASE index=4
  RETURN CFNMETA(9)+'|'+HexBytes(BINTOC(9,'BRS'))+'|'+HexBytes(TRANSFORM(9))
 CASE index=5
  RETURN CFNMETA(10)+'|'+HexBytes(BINTOC(10,'BRS'))+'|'+HexBytes(TRANSFORM(10))
 CASE index=6
  RETURN CFNMETA(99)+'|'+HexBytes(BINTOC(99,'BRS'))+'|'+HexBytes(TRANSFORM(99))
 CASE index=7
  RETURN CFNMETA(100)+'|'+HexBytes(BINTOC(100,'BRS'))+'|'+HexBytes(TRANSFORM(100))
 CASE index=8
  RETURN CFNMETA(0005)+'|'+HexBytes(BINTOC(0005,'BRS'))+'|'+HexBytes(TRANSFORM(0005))
 CASE index=9
  RETURN CFNMETA(0000000005)+'|'+HexBytes(BINTOC(0000000005,'BRS'))+'|'+HexBytes(TRANSFORM(0000000005))
 CASE index=10
  RETURN CFNMETA(00000000005)+'|'+HexBytes(BINTOC(00000000005,'BRS'))+'|'+HexBytes(TRANSFORM(00000000005))
 CASE index=11
  RETURN CFNMETA(2147483646)+'|'+HexBytes(BINTOC(2147483646,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646))
 CASE index=12
  RETURN CFNMETA(2147483647)+'|'+HexBytes(BINTOC(2147483647,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647))
 CASE index=13
  RETURN CFNMETA(2147483648)+'|'+HexBytes(BINTOC(2147483648,'BRS'))+'|'+HexBytes(TRANSFORM(2147483648))
 CASE index=14
  RETURN CFNMETA(4294967295)+'|'+HexBytes(BINTOC(4294967295,'BRS'))+'|'+HexBytes(TRANSFORM(4294967295))
 CASE index=15
  RETURN CFNMETA(4294967296)+'|'+HexBytes(BINTOC(4294967296,'BRS'))+'|'+HexBytes(TRANSFORM(4294967296))
 CASE index=16
  RETURN CFNMETA(-0)+'|'+HexBytes(BINTOC(-0,'BRS'))+'|'+HexBytes(TRANSFORM(-0))
 CASE index=17
  RETURN CFNMETA(-1)+'|'+HexBytes(BINTOC(-1,'BRS'))+'|'+HexBytes(TRANSFORM(-1))
 CASE index=18
  RETURN CFNMETA(-2147483647)+'|'+HexBytes(BINTOC(-2147483647,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647))
 CASE index=19
  RETURN CFNMETA(-2147483648)+'|'+HexBytes(BINTOC(-2147483648,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483648))
 CASE index=20
  RETURN CFNMETA(-2147483649)+'|'+HexBytes(BINTOC(-2147483649,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483649))
 CASE index=21
  RETURN CFNMETA(-(-0))+'|'+HexBytes(BINTOC(-(-0),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0)))
 CASE index=22
  RETURN CFNMETA(-(-5))+'|'+HexBytes(BINTOC(-(-5),'BRS'))+'|'+HexBytes(TRANSFORM(-(-5)))
 CASE index=23
  RETURN CFNMETA(-(-2147483647))+'|'+HexBytes(BINTOC(-(-2147483647),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647)))
 CASE index=24
  RETURN CFNMETA(+0)+'|'+HexBytes(BINTOC(+0,'BRS'))+'|'+HexBytes(TRANSFORM(+0))
 CASE index=25
  RETURN CFNMETA(+(5))+'|'+HexBytes(BINTOC(+(5),'BRS'))+'|'+HexBytes(TRANSFORM(+(5)))
 CASE index=26
  RETURN CFNMETA((2147483647))+'|'+HexBytes(BINTOC((2147483647),'BRS'))+'|'+HexBytes(TRANSFORM((2147483647)))
 CASE index=27
  RETURN CFNMETA(INT(-2147483648.00))+'|'+HexBytes(BINTOC(INT(-2147483648.00),'BRS'))+'|'+HexBytes(TRANSFORM(INT(-2147483648.00)))
 CASE index=28
  RETURN CFNMETA(-INT(-2147483648.00))+'|'+HexBytes(BINTOC(-INT(-2147483648.00),'BRS'))+'|'+HexBytes(TRANSFORM(-INT(-2147483648.00)))
 CASE index=29
  RETURN CFNMETA(INT(2147483647.00))+'|'+HexBytes(BINTOC(INT(2147483647.00),'BRS'))+'|'+HexBytes(TRANSFORM(INT(2147483647.00)))
 CASE index=30
  RETURN CFNMETA(-INT(2147483647.00))+'|'+HexBytes(BINTOC(-INT(2147483647.00),'BRS'))+'|'+HexBytes(TRANSFORM(-INT(2147483647.00)))
 CASE index=31
  RETURN CFNMETA(-iZero)+'|'+HexBytes(BINTOC(-iZero,'BRS'))+'|'+HexBytes(TRANSFORM(-iZero))
 CASE index=32
  RETURN CFNMETA(-(-iZero))+'|'+HexBytes(BINTOC(-(-iZero),'BRS'))+'|'+HexBytes(TRANSFORM(-(-iZero)))
 CASE index=33
  RETURN CFNMETA(-iMin)+'|'+HexBytes(BINTOC(-iMin,'BRS'))+'|'+HexBytes(TRANSFORM(-iMin))
 CASE index=34
  RETURN CFNMETA(-(-iMin))+'|'+HexBytes(BINTOC(-(-iMin),'BRS'))+'|'+HexBytes(TRANSFORM(-(-iMin)))
 CASE index=35
  RETURN CFNMETA(iMin)+'|'+HexBytes(BINTOC(iMin,'BRS'))+'|'+HexBytes(TRANSFORM(iMin))
 CASE index=36
  RETURN CFNMETA(iZero)+'|'+HexBytes(BINTOC(iZero,'BRS'))+'|'+HexBytes(TRANSFORM(iZero))
 CASE index=37
  RETURN CFNMETA(0000000000)+'|'+HexBytes(BINTOC(0000000000,'BRS'))+'|'+HexBytes(TRANSFORM(0000000000))
 CASE index=38
  RETURN CFNMETA(00000000000)+'|'+HexBytes(BINTOC(00000000000,'BRS'))+'|'+HexBytes(TRANSFORM(00000000000))
 CASE index=39
  RETURN CFNMETA(00000000000000000000)+'|'+HexBytes(BINTOC(00000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(00000000000000000000))
 CASE index=40
  RETURN CFNMETA(-0005)+'|'+HexBytes(BINTOC(-0005,'BRS'))+'|'+HexBytes(TRANSFORM(-0005))
 CASE index=41
  RETURN CFNMETA(-0000000005)+'|'+HexBytes(BINTOC(-0000000005,'BRS'))+'|'+HexBytes(TRANSFORM(-0000000005))
 CASE index=42
  RETURN CFNMETA(-00000000005)+'|'+HexBytes(BINTOC(-00000000005,'BRS'))+'|'+HexBytes(TRANSFORM(-00000000005))
 CASE index=43
  RETURN CFNMETA(-(-0005))+'|'+HexBytes(BINTOC(-(-0005),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0005)))
 CASE index=44
  RETURN CFNMETA(+0005)+'|'+HexBytes(BINTOC(+0005,'BRS'))+'|'+HexBytes(TRANSFORM(+0005))
 CASE index=45
  RETURN CFNMETA(0.0)+'|'+HexBytes(BINTOC(0.0,'BRS'))+'|'+HexBytes(TRANSFORM(0.0))
 CASE index=46
  RETURN CFNMETA(-0.0)+'|'+HexBytes(BINTOC(-0.0,'BRS'))+'|'+HexBytes(TRANSFORM(-0.0))
 CASE index=47
  RETURN CFNMETA(2147483647.0)+'|'+HexBytes(BINTOC(2147483647.0,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647.0))
 CASE index=48
  RETURN CFNMETA(-2147483647.0)+'|'+HexBytes(BINTOC(-2147483647.0,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647.0))
 ENDCASE
 RETURN ''
ENDFUNC

FUNCTION HexBytes
 LPARAMETERS text
 LOCAL answer,i,value
 answer=''
 FOR i=1 TO LEN(text)
  value=ASC(SUBSTR(text,i,1))
  answer=answer+SUBSTR('0123456789ABCDEF',INT(value/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(value,16)+1,1)
 ENDFOR
 RETURN answer
ENDFUNC
