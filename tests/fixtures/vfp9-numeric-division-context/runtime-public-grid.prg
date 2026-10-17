* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DIVISION-CONTEXT-001: runtime metadata intervention.
* Explicit native-only descriptor variants; no inferred division formula.
LOCAL ARRAY expressions[60], settings[5]
expressions[1]='iFive/iTwo'
expressions[2]='nFive/nTwo'
expressions[3]='iFive/nTwo'
expressions[4]='nFive/iTwo'
expressions[5]='iFive/2'
expressions[6]='5/iTwo'
expressions[7]='nFive/2.00'
expressions[8]='5.00/nTwo'
expressions[9]='-iFive/iTwo'
expressions[10]='iFive/-iTwo'
expressions[11]='-(iFive/iTwo)'
expressions[12]='+iFive/+iTwo'
expressions[13]='iZero/iTwo'
expressions[14]='iZero/-iTwo'
expressions[15]='nNegZero/iTwo'
expressions[16]='nZero/-iTwo'
expressions[17]='nNegZero/-iTwo'
expressions[18]='iBoundary/iTwo'
expressions[19]='nBoundary/nTwo'
expressions[20]='(iFive/iTwo)/2'
expressions[21]='iFive/(iFive/iTwo)'
expressions[22]='(nFive/nTwo)/iTwo'
expressions[23]='IIF(.T.,5,0)/2'
expressions[24]='EVALUATE("5")/2'
expressions[25]='5/EVALUATE("2")'
expressions[26]='IIF(.T.,iFive,0)/2'
expressions[27]='5/(IIF(.T.,iTwo,0))'
expressions[28]='nTiny/iTwo'
expressions[29]='-nTiny/iTwo'
expressions[30]='nBig/iTwo'
expressions[31]='-nBig/iTwo'
expressions[32]='nTiny/nTiny'
expressions[33]='nBig/nBig'
expressions[34]='nScale1/2'
expressions[35]='nScale2/2'
expressions[36]='nScale3/2'
expressions[37]='nScale4/2'
expressions[38]='nScale5/2'
expressions[39]='nScale6/2'
expressions[40]='nScale7/2'
expressions[41]='nScale8/2'
expressions[42]='nScale9/2'
expressions[43]='nScale10/2'
expressions[44]='nScale11/2'
expressions[45]='nScale12/2'
expressions[46]='nScale13/2'
expressions[47]='nScale14/2'
expressions[48]='nScale15/2'
expressions[49]='nScale16/2'
expressions[50]='nScale17/2'
expressions[51]='nScale18/2'
expressions[52]='nScale18/nScale1'
expressions[53]='nScale1/nScale18'
expressions[54]='nNegFive/nTwo'
expressions[55]='nFive/nNegTwo'
expressions[56]='nNegFive/nNegTwo'
expressions[57]='-(nFive/nTwo)'
expressions[58]='-0.0/iTwo'
expressions[59]='iFive/-2.00'
expressions[60]='-5.00/iTwo'
settings[1]=0
settings[2]=2
settings[3]=6
settings[4]=14
settings[5]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught
PRIVATE iFive,iTwo,nFive,nTwo,iZero,nZero,nNegZero,iBoundary,nBoundary,nTiny,nBig,nNegFive,nNegTwo
PRIVATE nScale1,nScale2,nScale3,nScale4,nScale5,nScale6,nScale7,nScale8,nScale9,nScale10,nScale11,nScale12,nScale13,nScale14,nScale15,nScale16,nScale17,nScale18

outputPath='Z:\home\rich\.codex\worktrees\numeric-division-recovery-5611\Project-Copperfin\build\division-recovery-native\public-results.txt'
libraryPath='Z:\home\rich\.codex\worktrees\numeric-division-recovery-5611\Project-Copperfin\build\division-recovery-native\diagnostic\cause-probe.fll'
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
  iZero=0
  nZero=0.0
  nNegZero=-0.0
  iBoundary=2147483647
  nBoundary=2147483647.00
  nTiny=0.000000000000000001
  nBig=9999999999.0
  nNegFive=-5.00
  nNegTwo=-2.00
  nScale1=5.0
  nScale2=5.00
  nScale3=5.000
  nScale4=5.0000
  nScale5=5.00000
  nScale6=5.000000
  nScale7=5.0000000
  nScale8=5.00000000
  nScale9=5.000000000
  nScale10=5.0000000000
  nScale11=5.00000000000
  nScale12=5.000000000000
  nScale13=5.0000000000000
  nScale14=5.00000000000000
  nScale15=5.000000000000000
  nScale16=5.0000000000000000
  nScale17=5.00000000000000000
  nScale18=5.000000000000000000
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
  RETURN CFNMETA(iFive/iTwo)+'|'+HexBytes(BINTOC(iFive/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(iFive/iTwo))
 CASE index=2
  RETURN CFNMETA(nFive/nTwo)+'|'+HexBytes(BINTOC(nFive/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nFive/nTwo))
 CASE index=3
  RETURN CFNMETA(iFive/nTwo)+'|'+HexBytes(BINTOC(iFive/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM(iFive/nTwo))
 CASE index=4
  RETURN CFNMETA(nFive/iTwo)+'|'+HexBytes(BINTOC(nFive/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nFive/iTwo))
 CASE index=5
  RETURN CFNMETA(iFive/2)+'|'+HexBytes(BINTOC(iFive/2,'BRS'))+'|'+HexBytes(TRANSFORM(iFive/2))
 CASE index=6
  RETURN CFNMETA(5/iTwo)+'|'+HexBytes(BINTOC(5/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(5/iTwo))
 CASE index=7
  RETURN CFNMETA(nFive/2.00)+'|'+HexBytes(BINTOC(nFive/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(nFive/2.00))
 CASE index=8
  RETURN CFNMETA(5.00/nTwo)+'|'+HexBytes(BINTOC(5.00/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM(5.00/nTwo))
 CASE index=9
  RETURN CFNMETA(-iFive/iTwo)+'|'+HexBytes(BINTOC(-iFive/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(-iFive/iTwo))
 CASE index=10
  RETURN CFNMETA(iFive/-iTwo)+'|'+HexBytes(BINTOC(iFive/-iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(iFive/-iTwo))
 CASE index=11
  RETURN CFNMETA(-(iFive/iTwo))+'|'+HexBytes(BINTOC(-(iFive/iTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-(iFive/iTwo)))
 CASE index=12
  RETURN CFNMETA(+iFive/+iTwo)+'|'+HexBytes(BINTOC(+iFive/+iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(+iFive/+iTwo))
 CASE index=13
  RETURN CFNMETA(iZero/iTwo)+'|'+HexBytes(BINTOC(iZero/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(iZero/iTwo))
 CASE index=14
  RETURN CFNMETA(iZero/-iTwo)+'|'+HexBytes(BINTOC(iZero/-iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(iZero/-iTwo))
 CASE index=15
  RETURN CFNMETA(nNegZero/iTwo)+'|'+HexBytes(BINTOC(nNegZero/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nNegZero/iTwo))
 CASE index=16
  RETURN CFNMETA(nZero/-iTwo)+'|'+HexBytes(BINTOC(nZero/-iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nZero/-iTwo))
 CASE index=17
  RETURN CFNMETA(nNegZero/-iTwo)+'|'+HexBytes(BINTOC(nNegZero/-iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nNegZero/-iTwo))
 CASE index=18
  RETURN CFNMETA(iBoundary/iTwo)+'|'+HexBytes(BINTOC(iBoundary/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(iBoundary/iTwo))
 CASE index=19
  RETURN CFNMETA(nBoundary/nTwo)+'|'+HexBytes(BINTOC(nBoundary/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nBoundary/nTwo))
 CASE index=20
  RETURN CFNMETA((iFive/iTwo)/2)+'|'+HexBytes(BINTOC((iFive/iTwo)/2,'BRS'))+'|'+HexBytes(TRANSFORM((iFive/iTwo)/2))
 CASE index=21
  RETURN CFNMETA(iFive/(iFive/iTwo))+'|'+HexBytes(BINTOC(iFive/(iFive/iTwo),'BRS'))+'|'+HexBytes(TRANSFORM(iFive/(iFive/iTwo)))
 CASE index=22
  RETURN CFNMETA((nFive/nTwo)/iTwo)+'|'+HexBytes(BINTOC((nFive/nTwo)/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM((nFive/nTwo)/iTwo))
 CASE index=23
  RETURN CFNMETA(IIF(.T.,5,0)/2)+'|'+HexBytes(BINTOC(IIF(.T.,5,0)/2,'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,5,0)/2))
 CASE index=24
  RETURN CFNMETA(EVALUATE("5")/2)+'|'+HexBytes(BINTOC(EVALUATE("5")/2,'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("5")/2))
 CASE index=25
  RETURN CFNMETA(5/EVALUATE("2"))+'|'+HexBytes(BINTOC(5/EVALUATE("2"),'BRS'))+'|'+HexBytes(TRANSFORM(5/EVALUATE("2")))
 CASE index=26
  RETURN CFNMETA(IIF(.T.,iFive,0)/2)+'|'+HexBytes(BINTOC(IIF(.T.,iFive,0)/2,'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,iFive,0)/2))
 CASE index=27
  RETURN CFNMETA(5/(IIF(.T.,iTwo,0)))+'|'+HexBytes(BINTOC(5/(IIF(.T.,iTwo,0)),'BRS'))+'|'+HexBytes(TRANSFORM(5/(IIF(.T.,iTwo,0))))
 CASE index=28
  RETURN CFNMETA(nTiny/iTwo)+'|'+HexBytes(BINTOC(nTiny/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nTiny/iTwo))
 CASE index=29
  RETURN CFNMETA(-nTiny/iTwo)+'|'+HexBytes(BINTOC(-nTiny/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(-nTiny/iTwo))
 CASE index=30
  RETURN CFNMETA(nBig/iTwo)+'|'+HexBytes(BINTOC(nBig/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nBig/iTwo))
 CASE index=31
  RETURN CFNMETA(-nBig/iTwo)+'|'+HexBytes(BINTOC(-nBig/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(-nBig/iTwo))
 CASE index=32
  RETURN CFNMETA(nTiny/nTiny)+'|'+HexBytes(BINTOC(nTiny/nTiny,'BRS'))+'|'+HexBytes(TRANSFORM(nTiny/nTiny))
 CASE index=33
  RETURN CFNMETA(nBig/nBig)+'|'+HexBytes(BINTOC(nBig/nBig,'BRS'))+'|'+HexBytes(TRANSFORM(nBig/nBig))
 CASE index=34
  RETURN CFNMETA(nScale1/2)+'|'+HexBytes(BINTOC(nScale1/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale1/2))
 CASE index=35
  RETURN CFNMETA(nScale2/2)+'|'+HexBytes(BINTOC(nScale2/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale2/2))
 CASE index=36
  RETURN CFNMETA(nScale3/2)+'|'+HexBytes(BINTOC(nScale3/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale3/2))
 CASE index=37
  RETURN CFNMETA(nScale4/2)+'|'+HexBytes(BINTOC(nScale4/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale4/2))
 CASE index=38
  RETURN CFNMETA(nScale5/2)+'|'+HexBytes(BINTOC(nScale5/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale5/2))
 CASE index=39
  RETURN CFNMETA(nScale6/2)+'|'+HexBytes(BINTOC(nScale6/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale6/2))
 CASE index=40
  RETURN CFNMETA(nScale7/2)+'|'+HexBytes(BINTOC(nScale7/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale7/2))
 CASE index=41
  RETURN CFNMETA(nScale8/2)+'|'+HexBytes(BINTOC(nScale8/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale8/2))
 CASE index=42
  RETURN CFNMETA(nScale9/2)+'|'+HexBytes(BINTOC(nScale9/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale9/2))
 CASE index=43
  RETURN CFNMETA(nScale10/2)+'|'+HexBytes(BINTOC(nScale10/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale10/2))
 CASE index=44
  RETURN CFNMETA(nScale11/2)+'|'+HexBytes(BINTOC(nScale11/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale11/2))
 CASE index=45
  RETURN CFNMETA(nScale12/2)+'|'+HexBytes(BINTOC(nScale12/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale12/2))
 CASE index=46
  RETURN CFNMETA(nScale13/2)+'|'+HexBytes(BINTOC(nScale13/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale13/2))
 CASE index=47
  RETURN CFNMETA(nScale14/2)+'|'+HexBytes(BINTOC(nScale14/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale14/2))
 CASE index=48
  RETURN CFNMETA(nScale15/2)+'|'+HexBytes(BINTOC(nScale15/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale15/2))
 CASE index=49
  RETURN CFNMETA(nScale16/2)+'|'+HexBytes(BINTOC(nScale16/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale16/2))
 CASE index=50
  RETURN CFNMETA(nScale17/2)+'|'+HexBytes(BINTOC(nScale17/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale17/2))
 CASE index=51
  RETURN CFNMETA(nScale18/2)+'|'+HexBytes(BINTOC(nScale18/2,'BRS'))+'|'+HexBytes(TRANSFORM(nScale18/2))
 CASE index=52
  RETURN CFNMETA(nScale18/nScale1)+'|'+HexBytes(BINTOC(nScale18/nScale1,'BRS'))+'|'+HexBytes(TRANSFORM(nScale18/nScale1))
 CASE index=53
  RETURN CFNMETA(nScale1/nScale18)+'|'+HexBytes(BINTOC(nScale1/nScale18,'BRS'))+'|'+HexBytes(TRANSFORM(nScale1/nScale18))
 CASE index=54
  RETURN CFNMETA(nNegFive/nTwo)+'|'+HexBytes(BINTOC(nNegFive/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nNegFive/nTwo))
 CASE index=55
  RETURN CFNMETA(nFive/nNegTwo)+'|'+HexBytes(BINTOC(nFive/nNegTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nFive/nNegTwo))
 CASE index=56
  RETURN CFNMETA(nNegFive/nNegTwo)+'|'+HexBytes(BINTOC(nNegFive/nNegTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nNegFive/nNegTwo))
 CASE index=57
  RETURN CFNMETA(-(nFive/nTwo))+'|'+HexBytes(BINTOC(-(nFive/nTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-(nFive/nTwo)))
 CASE index=58
  RETURN CFNMETA(-0.0/iTwo)+'|'+HexBytes(BINTOC(-0.0/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(-0.0/iTwo))
 CASE index=59
  RETURN CFNMETA(iFive/-2.00)+'|'+HexBytes(BINTOC(iFive/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(iFive/-2.00))
 CASE index=60
  RETURN CFNMETA(-5.00/iTwo)+'|'+HexBytes(BINTOC(-5.00/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(-5.00/iTwo))
 ENDCASE
ENDFUNC

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,position,byte,digits
 result=''
 digits='0123456789ABCDEF'
 FOR position=1 TO LEN(value)
  byte=ASC(SUBSTR(value,position,1))
  result=result+SUBSTR(digits,INT(byte/16)+1,1)+SUBSTR(digits,MOD(byte,16)+1,1)
 ENDFOR
 RETURN result
ENDFUNC

