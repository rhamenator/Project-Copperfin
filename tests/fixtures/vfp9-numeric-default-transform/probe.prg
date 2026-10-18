* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: immutable native stimulus, not an oracle model.
LOCAL ARRAY expressions[82],settings[6]
expressions[1]='2'
expressions[2]='3'
expressions[3]='7'
expressions[4]='5'
expressions[5]='1'
expressions[6]='0'
expressions[7]='0.5'
expressions[8]='0.25'
expressions[9]='2.50'
expressions[10]='0.50'
expressions[11]='0.3'
expressions[12]='0.000000000000000001'
expressions[13]='1.234567890123456789'
expressions[14]='00.50'
expressions[15]='0.125'
expressions[16]='0.0'
expressions[17]='0.00'
expressions[18]='-0.00'
expressions[19]='-0.5'
expressions[20]='-2.50'
expressions[21]='1.00'
expressions[22]='123.4500'
expressions[23]='00123.4500'
expressions[24]='1.234567'
expressions[25]='1.23456789'
expressions[26]='1.23456789012345'
expressions[27]='1.234567890123456'
expressions[28]='1.2345678901234567'
expressions[29]='1.23456789012345678'
expressions[30]='0.00001'
expressions[31]='0.0000000001'
expressions[32]='0.00000000000001'
expressions[33]='0.000000000000001'
expressions[34]='0.0000000000000001'
expressions[35]='0.00000000000000001'
expressions[36]='0.0000000000000000010'
expressions[37]='-0.000000000000000001'
expressions[38]='9.99999999999999'
expressions[39]='9.999999999999999'
expressions[40]='99.99999999999999'
expressions[41]='999999999.5'
expressions[42]='9999999999.5'
expressions[43]='999999999999999.5'
expressions[44]='2147483647'
expressions[45]='2147483648'
expressions[46]='-2147483648'
expressions[47]='-2147483649'
expressions[48]='9999999999999999'
expressions[49]='99999999999999999'
expressions[50]='999999999999999999'
expressions[51]='1E-1'
expressions[52]='1E-14'
expressions[53]='1E-18'
expressions[54]='1E-20'
expressions[55]='1.25E-2'
expressions[56]='1E+20'
expressions[57]='1E+100'
expressions[58]='1E+308'
expressions[59]='(0.5)'
expressions[60]='-(0.5)'
expressions[61]='IIF(.T.,0.5,0)'
expressions[62]='(2.50)'
expressions[63]='-(2.50)'
expressions[64]='IIF(.T.,2.50,0)'
expressions[65]='(0.00)'
expressions[66]='-(0.00)'
expressions[67]='IIF(.T.,0.00,0)'
expressions[68]='(0.000000000000000001)'
expressions[69]='-(0.000000000000000001)'
expressions[70]='IIF(.T.,0.000000000000000001,0)'
expressions[71]='(1.234567890123456789)'
expressions[72]='-(1.234567890123456789)'
expressions[73]='IIF(.T.,1.234567890123456789,0)'
expressions[74]='(2147483648)'
expressions[75]='-(2147483648)'
expressions[76]='IIF(.T.,2147483648,0)'
expressions[77]='(1E-18)'
expressions[78]='-(1E-18)'
expressions[79]='IIF(.T.,1E-18,0)'
expressions[80]='(1E+20)'
expressions[81]='-(1E+20)'
expressions[82]='IIF(.T.,1E+20,0)'
settings[1]=0
settings[2]=1
settings[3]=2
settings[4]=6
settings[5]=14
settings[6]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,routeNo,route
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-native\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
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
  FOR index=1 TO ALEN(expressions)
   expression=expressions[index]
   FOR routeNo=1 TO 4
    route=GETWORDNUM('EVAL COMPILED STORE-EVAL STORE-COMPILED',routeNo)
    TRY
     DO CASE
     CASE routeNo=1
      observation=CFNMETA(EVALUATE(expression))+'|'+HexBytes(BINTOC(EVALUATE(expression),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE(expression)))+'|'+VARTYPE(TRANSFORM(EVALUATE(expression)))
     CASE routeNo=2
      observation=CompiledObservation(index,.F.)
     CASE routeNo=3
      observation=StoredEvalObservation(expression)
     CASE routeNo=4
      observation=CompiledObservation(index,.T.)
     ENDCASE
     line=route+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
    CATCH TO caught
     line='ERROR-'+route+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
    ENDTRY
    =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
    total=total+1
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION StoredEvalObservation
 LPARAMETERS expression
 LOCAL stored
 stored=EVALUATE(expression)
 RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))

FUNCTION CompiledObservation
 LPARAMETERS index,storeFirst
 LOCAL stored
 DO CASE
 CASE index=1
  IF storeFirst
   stored=2
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(2)+'|'+HexBytes(BINTOC(2,'BRS'))+'|'+HexBytes(TRANSFORM(2))+'|'+VARTYPE(TRANSFORM(2))
 CASE index=2
  IF storeFirst
   stored=3
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(3)+'|'+HexBytes(BINTOC(3,'BRS'))+'|'+HexBytes(TRANSFORM(3))+'|'+VARTYPE(TRANSFORM(3))
 CASE index=3
  IF storeFirst
   stored=7
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(7)+'|'+HexBytes(BINTOC(7,'BRS'))+'|'+HexBytes(TRANSFORM(7))+'|'+VARTYPE(TRANSFORM(7))
 CASE index=4
  IF storeFirst
   stored=5
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(5)+'|'+HexBytes(BINTOC(5,'BRS'))+'|'+HexBytes(TRANSFORM(5))+'|'+VARTYPE(TRANSFORM(5))
 CASE index=5
  IF storeFirst
   stored=1
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1)+'|'+HexBytes(BINTOC(1,'BRS'))+'|'+HexBytes(TRANSFORM(1))+'|'+VARTYPE(TRANSFORM(1))
 CASE index=6
  IF storeFirst
   stored=0
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0)+'|'+HexBytes(BINTOC(0,'BRS'))+'|'+HexBytes(TRANSFORM(0))+'|'+VARTYPE(TRANSFORM(0))
 CASE index=7
  IF storeFirst
   stored=0.5
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.5)+'|'+HexBytes(BINTOC(0.5,'BRS'))+'|'+HexBytes(TRANSFORM(0.5))+'|'+VARTYPE(TRANSFORM(0.5))
 CASE index=8
  IF storeFirst
   stored=0.25
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.25)+'|'+HexBytes(BINTOC(0.25,'BRS'))+'|'+HexBytes(TRANSFORM(0.25))+'|'+VARTYPE(TRANSFORM(0.25))
 CASE index=9
  IF storeFirst
   stored=2.50
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(2.50)+'|'+HexBytes(BINTOC(2.50,'BRS'))+'|'+HexBytes(TRANSFORM(2.50))+'|'+VARTYPE(TRANSFORM(2.50))
 CASE index=10
  IF storeFirst
   stored=0.50
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.50)+'|'+HexBytes(BINTOC(0.50,'BRS'))+'|'+HexBytes(TRANSFORM(0.50))+'|'+VARTYPE(TRANSFORM(0.50))
 CASE index=11
  IF storeFirst
   stored=0.3
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.3)+'|'+HexBytes(BINTOC(0.3,'BRS'))+'|'+HexBytes(TRANSFORM(0.3))+'|'+VARTYPE(TRANSFORM(0.3))
 CASE index=12
  IF storeFirst
   stored=0.000000000000000001
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.000000000000000001)+'|'+HexBytes(BINTOC(0.000000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000000000000001))+'|'+VARTYPE(TRANSFORM(0.000000000000000001))
 CASE index=13
  IF storeFirst
   stored=1.234567890123456789
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1.234567890123456789)+'|'+HexBytes(BINTOC(1.234567890123456789,'BRS'))+'|'+HexBytes(TRANSFORM(1.234567890123456789))+'|'+VARTYPE(TRANSFORM(1.234567890123456789))
 CASE index=14
  IF storeFirst
   stored=00.50
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(00.50)+'|'+HexBytes(BINTOC(00.50,'BRS'))+'|'+HexBytes(TRANSFORM(00.50))+'|'+VARTYPE(TRANSFORM(00.50))
 CASE index=15
  IF storeFirst
   stored=0.125
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.125)+'|'+HexBytes(BINTOC(0.125,'BRS'))+'|'+HexBytes(TRANSFORM(0.125))+'|'+VARTYPE(TRANSFORM(0.125))
 CASE index=16
  IF storeFirst
   stored=0.0
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.0)+'|'+HexBytes(BINTOC(0.0,'BRS'))+'|'+HexBytes(TRANSFORM(0.0))+'|'+VARTYPE(TRANSFORM(0.0))
 CASE index=17
  IF storeFirst
   stored=0.00
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.00)+'|'+HexBytes(BINTOC(0.00,'BRS'))+'|'+HexBytes(TRANSFORM(0.00))+'|'+VARTYPE(TRANSFORM(0.00))
 CASE index=18
  IF storeFirst
   stored=-0.00
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-0.00)+'|'+HexBytes(BINTOC(-0.00,'BRS'))+'|'+HexBytes(TRANSFORM(-0.00))+'|'+VARTYPE(TRANSFORM(-0.00))
 CASE index=19
  IF storeFirst
   stored=-0.5
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-0.5)+'|'+HexBytes(BINTOC(-0.5,'BRS'))+'|'+HexBytes(TRANSFORM(-0.5))+'|'+VARTYPE(TRANSFORM(-0.5))
 CASE index=20
  IF storeFirst
   stored=-2.50
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-2.50)+'|'+HexBytes(BINTOC(-2.50,'BRS'))+'|'+HexBytes(TRANSFORM(-2.50))+'|'+VARTYPE(TRANSFORM(-2.50))
 CASE index=21
  IF storeFirst
   stored=1.00
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1.00)+'|'+HexBytes(BINTOC(1.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.00))+'|'+VARTYPE(TRANSFORM(1.00))
 CASE index=22
  IF storeFirst
   stored=123.4500
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(123.4500)+'|'+HexBytes(BINTOC(123.4500,'BRS'))+'|'+HexBytes(TRANSFORM(123.4500))+'|'+VARTYPE(TRANSFORM(123.4500))
 CASE index=23
  IF storeFirst
   stored=00123.4500
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(00123.4500)+'|'+HexBytes(BINTOC(00123.4500,'BRS'))+'|'+HexBytes(TRANSFORM(00123.4500))+'|'+VARTYPE(TRANSFORM(00123.4500))
 CASE index=24
  IF storeFirst
   stored=1.234567
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1.234567)+'|'+HexBytes(BINTOC(1.234567,'BRS'))+'|'+HexBytes(TRANSFORM(1.234567))+'|'+VARTYPE(TRANSFORM(1.234567))
 CASE index=25
  IF storeFirst
   stored=1.23456789
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1.23456789)+'|'+HexBytes(BINTOC(1.23456789,'BRS'))+'|'+HexBytes(TRANSFORM(1.23456789))+'|'+VARTYPE(TRANSFORM(1.23456789))
 CASE index=26
  IF storeFirst
   stored=1.23456789012345
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1.23456789012345)+'|'+HexBytes(BINTOC(1.23456789012345,'BRS'))+'|'+HexBytes(TRANSFORM(1.23456789012345))+'|'+VARTYPE(TRANSFORM(1.23456789012345))
 CASE index=27
  IF storeFirst
   stored=1.234567890123456
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1.234567890123456)+'|'+HexBytes(BINTOC(1.234567890123456,'BRS'))+'|'+HexBytes(TRANSFORM(1.234567890123456))+'|'+VARTYPE(TRANSFORM(1.234567890123456))
 CASE index=28
  IF storeFirst
   stored=1.2345678901234567
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1.2345678901234567)+'|'+HexBytes(BINTOC(1.2345678901234567,'BRS'))+'|'+HexBytes(TRANSFORM(1.2345678901234567))+'|'+VARTYPE(TRANSFORM(1.2345678901234567))
 CASE index=29
  IF storeFirst
   stored=1.23456789012345678
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1.23456789012345678)+'|'+HexBytes(BINTOC(1.23456789012345678,'BRS'))+'|'+HexBytes(TRANSFORM(1.23456789012345678))+'|'+VARTYPE(TRANSFORM(1.23456789012345678))
 CASE index=30
  IF storeFirst
   stored=0.00001
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.00001)+'|'+HexBytes(BINTOC(0.00001,'BRS'))+'|'+HexBytes(TRANSFORM(0.00001))+'|'+VARTYPE(TRANSFORM(0.00001))
 CASE index=31
  IF storeFirst
   stored=0.0000000001
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.0000000001)+'|'+HexBytes(BINTOC(0.0000000001,'BRS'))+'|'+HexBytes(TRANSFORM(0.0000000001))+'|'+VARTYPE(TRANSFORM(0.0000000001))
 CASE index=32
  IF storeFirst
   stored=0.00000000000001
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.00000000000001)+'|'+HexBytes(BINTOC(0.00000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(0.00000000000001))+'|'+VARTYPE(TRANSFORM(0.00000000000001))
 CASE index=33
  IF storeFirst
   stored=0.000000000000001
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.000000000000001)+'|'+HexBytes(BINTOC(0.000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000000000001))+'|'+VARTYPE(TRANSFORM(0.000000000000001))
 CASE index=34
  IF storeFirst
   stored=0.0000000000000001
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.0000000000000001)+'|'+HexBytes(BINTOC(0.0000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(0.0000000000000001))+'|'+VARTYPE(TRANSFORM(0.0000000000000001))
 CASE index=35
  IF storeFirst
   stored=0.00000000000000001
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.00000000000000001)+'|'+HexBytes(BINTOC(0.00000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(0.00000000000000001))+'|'+VARTYPE(TRANSFORM(0.00000000000000001))
 CASE index=36
  IF storeFirst
   stored=0.0000000000000000010
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(0.0000000000000000010)+'|'+HexBytes(BINTOC(0.0000000000000000010,'BRS'))+'|'+HexBytes(TRANSFORM(0.0000000000000000010))+'|'+VARTYPE(TRANSFORM(0.0000000000000000010))
 CASE index=37
  IF storeFirst
   stored=-0.000000000000000001
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-0.000000000000000001)+'|'+HexBytes(BINTOC(-0.000000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(-0.000000000000000001))+'|'+VARTYPE(TRANSFORM(-0.000000000000000001))
 CASE index=38
  IF storeFirst
   stored=9.99999999999999
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(9.99999999999999)+'|'+HexBytes(BINTOC(9.99999999999999,'BRS'))+'|'+HexBytes(TRANSFORM(9.99999999999999))+'|'+VARTYPE(TRANSFORM(9.99999999999999))
 CASE index=39
  IF storeFirst
   stored=9.999999999999999
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(9.999999999999999)+'|'+HexBytes(BINTOC(9.999999999999999,'BRS'))+'|'+HexBytes(TRANSFORM(9.999999999999999))+'|'+VARTYPE(TRANSFORM(9.999999999999999))
 CASE index=40
  IF storeFirst
   stored=99.99999999999999
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(99.99999999999999)+'|'+HexBytes(BINTOC(99.99999999999999,'BRS'))+'|'+HexBytes(TRANSFORM(99.99999999999999))+'|'+VARTYPE(TRANSFORM(99.99999999999999))
 CASE index=41
  IF storeFirst
   stored=999999999.5
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(999999999.5)+'|'+HexBytes(BINTOC(999999999.5,'BRS'))+'|'+HexBytes(TRANSFORM(999999999.5))+'|'+VARTYPE(TRANSFORM(999999999.5))
 CASE index=42
  IF storeFirst
   stored=9999999999.5
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(9999999999.5)+'|'+HexBytes(BINTOC(9999999999.5,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.5))+'|'+VARTYPE(TRANSFORM(9999999999.5))
 CASE index=43
  IF storeFirst
   stored=999999999999999.5
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(999999999999999.5)+'|'+HexBytes(BINTOC(999999999999999.5,'BRS'))+'|'+HexBytes(TRANSFORM(999999999999999.5))+'|'+VARTYPE(TRANSFORM(999999999999999.5))
 CASE index=44
  IF storeFirst
   stored=2147483647
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(2147483647)+'|'+HexBytes(BINTOC(2147483647,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647))+'|'+VARTYPE(TRANSFORM(2147483647))
 CASE index=45
  IF storeFirst
   stored=2147483648
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(2147483648)+'|'+HexBytes(BINTOC(2147483648,'BRS'))+'|'+HexBytes(TRANSFORM(2147483648))+'|'+VARTYPE(TRANSFORM(2147483648))
 CASE index=46
  IF storeFirst
   stored=-2147483648
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-2147483648)+'|'+HexBytes(BINTOC(-2147483648,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483648))+'|'+VARTYPE(TRANSFORM(-2147483648))
 CASE index=47
  IF storeFirst
   stored=-2147483649
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-2147483649)+'|'+HexBytes(BINTOC(-2147483649,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483649))+'|'+VARTYPE(TRANSFORM(-2147483649))
 CASE index=48
  IF storeFirst
   stored=9999999999999999
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(9999999999999999)+'|'+HexBytes(BINTOC(9999999999999999,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999999999))+'|'+VARTYPE(TRANSFORM(9999999999999999))
 CASE index=49
  IF storeFirst
   stored=99999999999999999
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(99999999999999999)+'|'+HexBytes(BINTOC(99999999999999999,'BRS'))+'|'+HexBytes(TRANSFORM(99999999999999999))+'|'+VARTYPE(TRANSFORM(99999999999999999))
 CASE index=50
  IF storeFirst
   stored=999999999999999999
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(999999999999999999)+'|'+HexBytes(BINTOC(999999999999999999,'BRS'))+'|'+HexBytes(TRANSFORM(999999999999999999))+'|'+VARTYPE(TRANSFORM(999999999999999999))
 CASE index=51
  IF storeFirst
   stored=1E-1
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1E-1)+'|'+HexBytes(BINTOC(1E-1,'BRS'))+'|'+HexBytes(TRANSFORM(1E-1))+'|'+VARTYPE(TRANSFORM(1E-1))
 CASE index=52
  IF storeFirst
   stored=1E-14
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1E-14)+'|'+HexBytes(BINTOC(1E-14,'BRS'))+'|'+HexBytes(TRANSFORM(1E-14))+'|'+VARTYPE(TRANSFORM(1E-14))
 CASE index=53
  IF storeFirst
   stored=1E-18
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1E-18)+'|'+HexBytes(BINTOC(1E-18,'BRS'))+'|'+HexBytes(TRANSFORM(1E-18))+'|'+VARTYPE(TRANSFORM(1E-18))
 CASE index=54
  IF storeFirst
   stored=1E-20
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1E-20)+'|'+HexBytes(BINTOC(1E-20,'BRS'))+'|'+HexBytes(TRANSFORM(1E-20))+'|'+VARTYPE(TRANSFORM(1E-20))
 CASE index=55
  IF storeFirst
   stored=1.25E-2
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1.25E-2)+'|'+HexBytes(BINTOC(1.25E-2,'BRS'))+'|'+HexBytes(TRANSFORM(1.25E-2))+'|'+VARTYPE(TRANSFORM(1.25E-2))
 CASE index=56
  IF storeFirst
   stored=1E+20
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1E+20)+'|'+HexBytes(BINTOC(1E+20,'BRS'))+'|'+HexBytes(TRANSFORM(1E+20))+'|'+VARTYPE(TRANSFORM(1E+20))
 CASE index=57
  IF storeFirst
   stored=1E+100
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1E+100)+'|'+HexBytes(BINTOC(1E+100,'BRS'))+'|'+HexBytes(TRANSFORM(1E+100))+'|'+VARTYPE(TRANSFORM(1E+100))
 CASE index=58
  IF storeFirst
   stored=1E+308
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(1E+308)+'|'+HexBytes(BINTOC(1E+308,'BRS'))+'|'+HexBytes(TRANSFORM(1E+308))+'|'+VARTYPE(TRANSFORM(1E+308))
 CASE index=59
  IF storeFirst
   stored=(0.5)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA((0.5))+'|'+HexBytes(BINTOC((0.5),'BRS'))+'|'+HexBytes(TRANSFORM((0.5)))+'|'+VARTYPE(TRANSFORM((0.5)))
 CASE index=60
  IF storeFirst
   stored=-(0.5)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-(0.5))+'|'+HexBytes(BINTOC(-(0.5),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.5)))+'|'+VARTYPE(TRANSFORM(-(0.5)))
 CASE index=61
  IF storeFirst
   stored=IIF(.T.,0.5,0)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(IIF(.T.,0.5,0))+'|'+HexBytes(BINTOC(IIF(.T.,0.5,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,0.5,0)))+'|'+VARTYPE(TRANSFORM(IIF(.T.,0.5,0)))
 CASE index=62
  IF storeFirst
   stored=(2.50)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA((2.50))+'|'+HexBytes(BINTOC((2.50),'BRS'))+'|'+HexBytes(TRANSFORM((2.50)))+'|'+VARTYPE(TRANSFORM((2.50)))
 CASE index=63
  IF storeFirst
   stored=-(2.50)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-(2.50))+'|'+HexBytes(BINTOC(-(2.50),'BRS'))+'|'+HexBytes(TRANSFORM(-(2.50)))+'|'+VARTYPE(TRANSFORM(-(2.50)))
 CASE index=64
  IF storeFirst
   stored=IIF(.T.,2.50,0)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(IIF(.T.,2.50,0))+'|'+HexBytes(BINTOC(IIF(.T.,2.50,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,2.50,0)))+'|'+VARTYPE(TRANSFORM(IIF(.T.,2.50,0)))
 CASE index=65
  IF storeFirst
   stored=(0.00)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA((0.00))+'|'+HexBytes(BINTOC((0.00),'BRS'))+'|'+HexBytes(TRANSFORM((0.00)))+'|'+VARTYPE(TRANSFORM((0.00)))
 CASE index=66
  IF storeFirst
   stored=-(0.00)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-(0.00))+'|'+HexBytes(BINTOC(-(0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00)))+'|'+VARTYPE(TRANSFORM(-(0.00)))
 CASE index=67
  IF storeFirst
   stored=IIF(.T.,0.00,0)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(IIF(.T.,0.00,0))+'|'+HexBytes(BINTOC(IIF(.T.,0.00,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,0.00,0)))+'|'+VARTYPE(TRANSFORM(IIF(.T.,0.00,0)))
 CASE index=68
  IF storeFirst
   stored=(0.000000000000000001)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA((0.000000000000000001))+'|'+HexBytes(BINTOC((0.000000000000000001),'BRS'))+'|'+HexBytes(TRANSFORM((0.000000000000000001)))+'|'+VARTYPE(TRANSFORM((0.000000000000000001)))
 CASE index=69
  IF storeFirst
   stored=-(0.000000000000000001)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-(0.000000000000000001))+'|'+HexBytes(BINTOC(-(0.000000000000000001),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.000000000000000001)))+'|'+VARTYPE(TRANSFORM(-(0.000000000000000001)))
 CASE index=70
  IF storeFirst
   stored=IIF(.T.,0.000000000000000001,0)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(IIF(.T.,0.000000000000000001,0))+'|'+HexBytes(BINTOC(IIF(.T.,0.000000000000000001,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,0.000000000000000001,0)))+'|'+VARTYPE(TRANSFORM(IIF(.T.,0.000000000000000001,0)))
 CASE index=71
  IF storeFirst
   stored=(1.234567890123456789)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA((1.234567890123456789))+'|'+HexBytes(BINTOC((1.234567890123456789),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789)))+'|'+VARTYPE(TRANSFORM((1.234567890123456789)))
 CASE index=72
  IF storeFirst
   stored=-(1.234567890123456789)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-(1.234567890123456789))+'|'+HexBytes(BINTOC(-(1.234567890123456789),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.234567890123456789)))+'|'+VARTYPE(TRANSFORM(-(1.234567890123456789)))
 CASE index=73
  IF storeFirst
   stored=IIF(.T.,1.234567890123456789,0)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(IIF(.T.,1.234567890123456789,0))+'|'+HexBytes(BINTOC(IIF(.T.,1.234567890123456789,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,1.234567890123456789,0)))+'|'+VARTYPE(TRANSFORM(IIF(.T.,1.234567890123456789,0)))
 CASE index=74
  IF storeFirst
   stored=(2147483648)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA((2147483648))+'|'+HexBytes(BINTOC((2147483648),'BRS'))+'|'+HexBytes(TRANSFORM((2147483648)))+'|'+VARTYPE(TRANSFORM((2147483648)))
 CASE index=75
  IF storeFirst
   stored=-(2147483648)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-(2147483648))+'|'+HexBytes(BINTOC(-(2147483648),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483648)))+'|'+VARTYPE(TRANSFORM(-(2147483648)))
 CASE index=76
  IF storeFirst
   stored=IIF(.T.,2147483648,0)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(IIF(.T.,2147483648,0))+'|'+HexBytes(BINTOC(IIF(.T.,2147483648,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,2147483648,0)))+'|'+VARTYPE(TRANSFORM(IIF(.T.,2147483648,0)))
 CASE index=77
  IF storeFirst
   stored=(1E-18)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA((1E-18))+'|'+HexBytes(BINTOC((1E-18),'BRS'))+'|'+HexBytes(TRANSFORM((1E-18)))+'|'+VARTYPE(TRANSFORM((1E-18)))
 CASE index=78
  IF storeFirst
   stored=-(1E-18)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-(1E-18))+'|'+HexBytes(BINTOC(-(1E-18),'BRS'))+'|'+HexBytes(TRANSFORM(-(1E-18)))+'|'+VARTYPE(TRANSFORM(-(1E-18)))
 CASE index=79
  IF storeFirst
   stored=IIF(.T.,1E-18,0)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(IIF(.T.,1E-18,0))+'|'+HexBytes(BINTOC(IIF(.T.,1E-18,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,1E-18,0)))+'|'+VARTYPE(TRANSFORM(IIF(.T.,1E-18,0)))
 CASE index=80
  IF storeFirst
   stored=(1E+20)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA((1E+20))+'|'+HexBytes(BINTOC((1E+20),'BRS'))+'|'+HexBytes(TRANSFORM((1E+20)))+'|'+VARTYPE(TRANSFORM((1E+20)))
 CASE index=81
  IF storeFirst
   stored=-(1E+20)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(-(1E+20))+'|'+HexBytes(BINTOC(-(1E+20),'BRS'))+'|'+HexBytes(TRANSFORM(-(1E+20)))+'|'+VARTYPE(TRANSFORM(-(1E+20)))
 CASE index=82
  IF storeFirst
   stored=IIF(.T.,1E+20,0)
   RETURN CFNMETA(stored)+'|'+HexBytes(BINTOC(stored,'BRS'))+'|'+HexBytes(TRANSFORM(stored))+'|'+VARTYPE(TRANSFORM(stored))
  ENDIF
  RETURN CFNMETA(IIF(.T.,1E+20,0))+'|'+HexBytes(BINTOC(IIF(.T.,1E+20,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,1E+20,0)))+'|'+VARTYPE(TRANSFORM(IIF(.T.,1E+20,0)))
 ENDCASE
 RETURN ''

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,index,byteValue
 result=''
 FOR index=1 TO LEN(value)
  byteValue=ASC(SUBSTR(value,index,1))
  result=result+SUBSTR('0123456789ABCDEF',INT(byteValue/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(byteValue,16)+1,1)
 ENDFOR
 RETURN result
