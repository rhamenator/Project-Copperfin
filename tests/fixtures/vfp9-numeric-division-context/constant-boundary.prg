* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DIVISION-CONTEXT-001: nested materialization boundary observations.
LOCAL ARRAY expressions[150], settings[5]
expressions[1]='5/2'
expressions[2]='+(5/2)'
expressions[3]='-(5/2)'
expressions[4]='-(-(5/2))'
expressions[5]='CFNCLONE(5/2)'
expressions[6]='-(CFNCLONE(5/2))'
expressions[7]='CFNCLONE(-(5/2))'
expressions[8]='ABS(5/2)'
expressions[9]='-(ABS(5/2))'
expressions[10]='IIF(.T.,5/2,0)'
expressions[11]='-(IIF(.T.,5/2,0))'
expressions[12]='EVALUATE("5/2")'
expressions[13]='-(EVALUATE("5/2"))'
expressions[14]='5.00/2.00'
expressions[15]='+(5.00/2.00)'
expressions[16]='-(5.00/2.00)'
expressions[17]='-(-(5.00/2.00))'
expressions[18]='CFNCLONE(5.00/2.00)'
expressions[19]='-(CFNCLONE(5.00/2.00))'
expressions[20]='CFNCLONE(-(5.00/2.00))'
expressions[21]='ABS(5.00/2.00)'
expressions[22]='-(ABS(5.00/2.00))'
expressions[23]='IIF(.T.,5.00/2.00,0)'
expressions[24]='-(IIF(.T.,5.00/2.00,0))'
expressions[25]='EVALUATE("5.00/2.00")'
expressions[26]='-(EVALUATE("5.00/2.00"))'
expressions[27]='10/2'
expressions[28]='+(10/2)'
expressions[29]='-(10/2)'
expressions[30]='-(-(10/2))'
expressions[31]='CFNCLONE(10/2)'
expressions[32]='-(CFNCLONE(10/2))'
expressions[33]='CFNCLONE(-(10/2))'
expressions[34]='ABS(10/2)'
expressions[35]='-(ABS(10/2))'
expressions[36]='IIF(.T.,10/2,0)'
expressions[37]='-(IIF(.T.,10/2,0))'
expressions[38]='EVALUATE("10/2")'
expressions[39]='-(EVALUATE("10/2"))'
expressions[40]='0.00/2'
expressions[41]='+(0.00/2)'
expressions[42]='-(0.00/2)'
expressions[43]='-(-(0.00/2))'
expressions[44]='CFNCLONE(0.00/2)'
expressions[45]='-(CFNCLONE(0.00/2))'
expressions[46]='CFNCLONE(-(0.00/2))'
expressions[47]='ABS(0.00/2)'
expressions[48]='-(ABS(0.00/2))'
expressions[49]='IIF(.T.,0.00/2,0)'
expressions[50]='-(IIF(.T.,0.00/2,0))'
expressions[51]='EVALUATE("0.00/2")'
expressions[52]='-(EVALUATE("0.00/2"))'
expressions[53]='(5/2)/(5/2)'
expressions[54]='+((5/2)/(5/2))'
expressions[55]='-((5/2)/(5/2))'
expressions[56]='-(-((5/2)/(5/2)))'
expressions[57]='CFNCLONE((5/2)/(5/2))'
expressions[58]='-(CFNCLONE((5/2)/(5/2)))'
expressions[59]='CFNCLONE(-((5/2)/(5/2)))'
expressions[60]='ABS((5/2)/(5/2))'
expressions[61]='-(ABS((5/2)/(5/2)))'
expressions[62]='IIF(.T.,(5/2)/(5/2),0)'
expressions[63]='-(IIF(.T.,(5/2)/(5/2),0))'
expressions[64]='EVALUATE("(5/2)/(5/2)")'
expressions[65]='-(EVALUATE("(5/2)/(5/2)"))'
expressions[66]='(5.00/2)/(5/2.00)'
expressions[67]='+((5.00/2)/(5/2.00))'
expressions[68]='-((5.00/2)/(5/2.00))'
expressions[69]='-(-((5.00/2)/(5/2.00)))'
expressions[70]='CFNCLONE((5.00/2)/(5/2.00))'
expressions[71]='-(CFNCLONE((5.00/2)/(5/2.00)))'
expressions[72]='CFNCLONE(-((5.00/2)/(5/2.00)))'
expressions[73]='ABS((5.00/2)/(5/2.00))'
expressions[74]='-(ABS((5.00/2)/(5/2.00)))'
expressions[75]='IIF(.T.,(5.00/2)/(5/2.00),0)'
expressions[76]='-(IIF(.T.,(5.00/2)/(5/2.00),0))'
expressions[77]='EVALUATE("(5.00/2)/(5/2.00)")'
expressions[78]='-(EVALUATE("(5.00/2)/(5/2.00)"))'
expressions[79]='(10/2)/2'
expressions[80]='+((10/2)/2)'
expressions[81]='-((10/2)/2)'
expressions[82]='-(-((10/2)/2))'
expressions[83]='CFNCLONE((10/2)/2)'
expressions[84]='-(CFNCLONE((10/2)/2))'
expressions[85]='CFNCLONE(-((10/2)/2))'
expressions[86]='ABS((10/2)/2)'
expressions[87]='-(ABS((10/2)/2))'
expressions[88]='IIF(.T.,(10/2)/2,0)'
expressions[89]='-(IIF(.T.,(10/2)/2,0))'
expressions[90]='EVALUATE("(10/2)/2")'
expressions[91]='-(EVALUATE("(10/2)/2"))'
expressions[92]='5/(2/2)'
expressions[93]='+(5/(2/2))'
expressions[94]='-(5/(2/2))'
expressions[95]='-(-(5/(2/2)))'
expressions[96]='CFNCLONE(5/(2/2))'
expressions[97]='-(CFNCLONE(5/(2/2)))'
expressions[98]='CFNCLONE(-(5/(2/2)))'
expressions[99]='ABS(5/(2/2))'
expressions[100]='-(ABS(5/(2/2)))'
expressions[101]='IIF(.T.,5/(2/2),0)'
expressions[102]='-(IIF(.T.,5/(2/2),0))'
expressions[103]='EVALUATE("5/(2/2)")'
expressions[104]='-(EVALUATE("5/(2/2)"))'
expressions[105]='(1/7)/3'
expressions[106]='+((1/7)/3)'
expressions[107]='-((1/7)/3)'
expressions[108]='-(-((1/7)/3))'
expressions[109]='CFNCLONE((1/7)/3)'
expressions[110]='-(CFNCLONE((1/7)/3))'
expressions[111]='CFNCLONE(-((1/7)/3))'
expressions[112]='ABS((1/7)/3)'
expressions[113]='-(ABS((1/7)/3))'
expressions[114]='IIF(.T.,(1/7)/3,0)'
expressions[115]='-(IIF(.T.,(1/7)/3,0))'
expressions[116]='EVALUATE("(1/7)/3")'
expressions[117]='-(EVALUATE("(1/7)/3"))'
expressions[118]='5/(5/2)'
expressions[119]='+(5/(5/2))'
expressions[120]='-(5/(5/2))'
expressions[121]='-(-(5/(5/2)))'
expressions[122]='CFNCLONE(5/(5/2))'
expressions[123]='-(CFNCLONE(5/(5/2)))'
expressions[124]='CFNCLONE(-(5/(5/2)))'
expressions[125]='ABS(5/(5/2))'
expressions[126]='-(ABS(5/(5/2)))'
expressions[127]='IIF(.T.,5/(5/2),0)'
expressions[128]='-(IIF(.T.,5/(5/2),0))'
expressions[129]='EVALUATE("5/(5/2)")'
expressions[130]='-(EVALUATE("5/(5/2)"))'
expressions[131]='iFive/(5/2)'
expressions[132]='(5/2)/iTwo'
expressions[133]='iFive/-(5/2)'
expressions[134]='-(iFive/(5/2))'
expressions[135]='-((5/2)/iTwo)'
expressions[136]='(5/2)/(iFive/iTwo)'
expressions[137]='(iFive/iTwo)/(5/2)'
expressions[138]='5/(CFNCLONE(5)/2)'
expressions[139]='CFNCLONE(5)/(5/2)'
expressions[140]='-(CFNCLONE(5)/(5/2))'
expressions[141]='CFNCLONE(5)/-(5/2)'
expressions[142]='(5/2)/CFNCLONE(2)'
expressions[143]='-(5/2)/CFNCLONE(2)'
expressions[144]='-((5/2)/CFNCLONE(2))'
expressions[145]='(5/2)/(CFNCLONE(5)/2)'
expressions[146]='(CFNCLONE(5)/2)/(5/2)'
expressions[147]='(5/2)/(IIF(.T.,5,0)/2)'
expressions[148]='IIF(.T.,5,0)/(5/2)'
expressions[149]='(5/2)/IIF(.T.,2,0)'
expressions[150]='-(IIF(.T.,5,0)/(5/2))'
settings[1]=0
settings[2]=2
settings[3]=6
settings[4]=14
settings[5]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught
PRIVATE iFive,iTwo
outputPath='Z:\home\rich\.codex\worktrees\numeric-constant-division-5611\Project-Copperfin\build\constant-division-native\boundary-results.txt'
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
  iFive=5
  iTwo=2
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
  RETURN CFNMETA(5/2)+'|'+HexBytes(BINTOC(5/2,'BRS'))+'|'+HexBytes(TRANSFORM(5/2))
 CASE index=2
  RETURN CFNMETA(+(5/2))+'|'+HexBytes(BINTOC(+(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(+(5/2)))
 CASE index=3
  RETURN CFNMETA(-(5/2))+'|'+HexBytes(BINTOC(-(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/2)))
 CASE index=4
  RETURN CFNMETA(-(-(5/2)))+'|'+HexBytes(BINTOC(-(-(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(5/2))))
 CASE index=5
  RETURN CFNMETA(CFNCLONE(5/2))+'|'+HexBytes(BINTOC(CFNCLONE(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(5/2)))
 CASE index=6
  RETURN CFNMETA(-(CFNCLONE(5/2)))+'|'+HexBytes(BINTOC(-(CFNCLONE(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE(5/2))))
 CASE index=7
  RETURN CFNMETA(CFNCLONE(-(5/2)))+'|'+HexBytes(BINTOC(CFNCLONE(-(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-(5/2))))
 CASE index=8
  RETURN CFNMETA(ABS(5/2))+'|'+HexBytes(BINTOC(ABS(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(ABS(5/2)))
 CASE index=9
  RETURN CFNMETA(-(ABS(5/2)))+'|'+HexBytes(BINTOC(-(ABS(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS(5/2))))
 CASE index=10
  RETURN CFNMETA(IIF(.T.,5/2,0))+'|'+HexBytes(BINTOC(IIF(.T.,5/2,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,5/2,0)))
 CASE index=11
  RETURN CFNMETA(-(IIF(.T.,5/2,0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,5/2,0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,5/2,0))))
 CASE index=12
  RETURN CFNMETA(EVALUATE("5/2"))+'|'+HexBytes(BINTOC(EVALUATE("5/2"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("5/2")))
 CASE index=13
  RETURN CFNMETA(-(EVALUATE("5/2")))+'|'+HexBytes(BINTOC(-(EVALUATE("5/2")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("5/2"))))
 CASE index=14
  RETURN CFNMETA(5.00/2.00)+'|'+HexBytes(BINTOC(5.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(5.00/2.00))
 CASE index=15
  RETURN CFNMETA(+(5.00/2.00))+'|'+HexBytes(BINTOC(+(5.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(+(5.00/2.00)))
 CASE index=16
  RETURN CFNMETA(-(5.00/2.00))+'|'+HexBytes(BINTOC(-(5.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.00/2.00)))
 CASE index=17
  RETURN CFNMETA(-(-(5.00/2.00)))+'|'+HexBytes(BINTOC(-(-(5.00/2.00)),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(5.00/2.00))))
 CASE index=18
  RETURN CFNMETA(CFNCLONE(5.00/2.00))+'|'+HexBytes(BINTOC(CFNCLONE(5.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(5.00/2.00)))
 CASE index=19
  RETURN CFNMETA(-(CFNCLONE(5.00/2.00)))+'|'+HexBytes(BINTOC(-(CFNCLONE(5.00/2.00)),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE(5.00/2.00))))
 CASE index=20
  RETURN CFNMETA(CFNCLONE(-(5.00/2.00)))+'|'+HexBytes(BINTOC(CFNCLONE(-(5.00/2.00)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-(5.00/2.00))))
 CASE index=21
  RETURN CFNMETA(ABS(5.00/2.00))+'|'+HexBytes(BINTOC(ABS(5.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(ABS(5.00/2.00)))
 CASE index=22
  RETURN CFNMETA(-(ABS(5.00/2.00)))+'|'+HexBytes(BINTOC(-(ABS(5.00/2.00)),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS(5.00/2.00))))
 CASE index=23
  RETURN CFNMETA(IIF(.T.,5.00/2.00,0))+'|'+HexBytes(BINTOC(IIF(.T.,5.00/2.00,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,5.00/2.00,0)))
 CASE index=24
  RETURN CFNMETA(-(IIF(.T.,5.00/2.00,0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,5.00/2.00,0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,5.00/2.00,0))))
 CASE index=25
  RETURN CFNMETA(EVALUATE("5.00/2.00"))+'|'+HexBytes(BINTOC(EVALUATE("5.00/2.00"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("5.00/2.00")))
 CASE index=26
  RETURN CFNMETA(-(EVALUATE("5.00/2.00")))+'|'+HexBytes(BINTOC(-(EVALUATE("5.00/2.00")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("5.00/2.00"))))
 CASE index=27
  RETURN CFNMETA(10/2)+'|'+HexBytes(BINTOC(10/2,'BRS'))+'|'+HexBytes(TRANSFORM(10/2))
 CASE index=28
  RETURN CFNMETA(+(10/2))+'|'+HexBytes(BINTOC(+(10/2),'BRS'))+'|'+HexBytes(TRANSFORM(+(10/2)))
 CASE index=29
  RETURN CFNMETA(-(10/2))+'|'+HexBytes(BINTOC(-(10/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(10/2)))
 CASE index=30
  RETURN CFNMETA(-(-(10/2)))+'|'+HexBytes(BINTOC(-(-(10/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(10/2))))
 CASE index=31
  RETURN CFNMETA(CFNCLONE(10/2))+'|'+HexBytes(BINTOC(CFNCLONE(10/2),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(10/2)))
 CASE index=32
  RETURN CFNMETA(-(CFNCLONE(10/2)))+'|'+HexBytes(BINTOC(-(CFNCLONE(10/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE(10/2))))
 CASE index=33
  RETURN CFNMETA(CFNCLONE(-(10/2)))+'|'+HexBytes(BINTOC(CFNCLONE(-(10/2)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-(10/2))))
 CASE index=34
  RETURN CFNMETA(ABS(10/2))+'|'+HexBytes(BINTOC(ABS(10/2),'BRS'))+'|'+HexBytes(TRANSFORM(ABS(10/2)))
 CASE index=35
  RETURN CFNMETA(-(ABS(10/2)))+'|'+HexBytes(BINTOC(-(ABS(10/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS(10/2))))
 CASE index=36
  RETURN CFNMETA(IIF(.T.,10/2,0))+'|'+HexBytes(BINTOC(IIF(.T.,10/2,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,10/2,0)))
 CASE index=37
  RETURN CFNMETA(-(IIF(.T.,10/2,0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,10/2,0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,10/2,0))))
 CASE index=38
  RETURN CFNMETA(EVALUATE("10/2"))+'|'+HexBytes(BINTOC(EVALUATE("10/2"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("10/2")))
 CASE index=39
  RETURN CFNMETA(-(EVALUATE("10/2")))+'|'+HexBytes(BINTOC(-(EVALUATE("10/2")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("10/2"))))
 CASE index=40
  RETURN CFNMETA(0.00/2)+'|'+HexBytes(BINTOC(0.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(0.00/2))
 CASE index=41
  RETURN CFNMETA(+(0.00/2))+'|'+HexBytes(BINTOC(+(0.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(+(0.00/2)))
 CASE index=42
  RETURN CFNMETA(-(0.00/2))+'|'+HexBytes(BINTOC(-(0.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00/2)))
 CASE index=43
  RETURN CFNMETA(-(-(0.00/2)))+'|'+HexBytes(BINTOC(-(-(0.00/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(0.00/2))))
 CASE index=44
  RETURN CFNMETA(CFNCLONE(0.00/2))+'|'+HexBytes(BINTOC(CFNCLONE(0.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(0.00/2)))
 CASE index=45
  RETURN CFNMETA(-(CFNCLONE(0.00/2)))+'|'+HexBytes(BINTOC(-(CFNCLONE(0.00/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE(0.00/2))))
 CASE index=46
  RETURN CFNMETA(CFNCLONE(-(0.00/2)))+'|'+HexBytes(BINTOC(CFNCLONE(-(0.00/2)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-(0.00/2))))
 CASE index=47
  RETURN CFNMETA(ABS(0.00/2))+'|'+HexBytes(BINTOC(ABS(0.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(ABS(0.00/2)))
 CASE index=48
  RETURN CFNMETA(-(ABS(0.00/2)))+'|'+HexBytes(BINTOC(-(ABS(0.00/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS(0.00/2))))
 CASE index=49
  RETURN CFNMETA(IIF(.T.,0.00/2,0))+'|'+HexBytes(BINTOC(IIF(.T.,0.00/2,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,0.00/2,0)))
 CASE index=50
  RETURN CFNMETA(-(IIF(.T.,0.00/2,0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,0.00/2,0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,0.00/2,0))))
 CASE index=51
  RETURN CFNMETA(EVALUATE("0.00/2"))+'|'+HexBytes(BINTOC(EVALUATE("0.00/2"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("0.00/2")))
 CASE index=52
  RETURN CFNMETA(-(EVALUATE("0.00/2")))+'|'+HexBytes(BINTOC(-(EVALUATE("0.00/2")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("0.00/2"))))
 CASE index=53
  RETURN CFNMETA((5/2)/(5/2))+'|'+HexBytes(BINTOC((5/2)/(5/2),'BRS'))+'|'+HexBytes(TRANSFORM((5/2)/(5/2)))
 CASE index=54
  RETURN CFNMETA(+((5/2)/(5/2)))+'|'+HexBytes(BINTOC(+((5/2)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+((5/2)/(5/2))))
 CASE index=55
  RETURN CFNMETA(-((5/2)/(5/2)))+'|'+HexBytes(BINTOC(-((5/2)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/2)/(5/2))))
 CASE index=56
  RETURN CFNMETA(-(-((5/2)/(5/2))))+'|'+HexBytes(BINTOC(-(-((5/2)/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((5/2)/(5/2)))))
 CASE index=57
  RETURN CFNMETA(CFNCLONE((5/2)/(5/2)))+'|'+HexBytes(BINTOC(CFNCLONE((5/2)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE((5/2)/(5/2))))
 CASE index=58
  RETURN CFNMETA(-(CFNCLONE((5/2)/(5/2))))+'|'+HexBytes(BINTOC(-(CFNCLONE((5/2)/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE((5/2)/(5/2)))))
 CASE index=59
  RETURN CFNMETA(CFNCLONE(-((5/2)/(5/2))))+'|'+HexBytes(BINTOC(CFNCLONE(-((5/2)/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-((5/2)/(5/2)))))
 CASE index=60
  RETURN CFNMETA(ABS((5/2)/(5/2)))+'|'+HexBytes(BINTOC(ABS((5/2)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(ABS((5/2)/(5/2))))
 CASE index=61
  RETURN CFNMETA(-(ABS((5/2)/(5/2))))+'|'+HexBytes(BINTOC(-(ABS((5/2)/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS((5/2)/(5/2)))))
 CASE index=62
  RETURN CFNMETA(IIF(.T.,(5/2)/(5/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(5/2)/(5/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(5/2)/(5/2),0)))
 CASE index=63
  RETURN CFNMETA(-(IIF(.T.,(5/2)/(5/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(5/2)/(5/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(5/2)/(5/2),0))))
 CASE index=64
  RETURN CFNMETA(EVALUATE("(5/2)/(5/2)"))+'|'+HexBytes(BINTOC(EVALUATE("(5/2)/(5/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(5/2)/(5/2)")))
 CASE index=65
  RETURN CFNMETA(-(EVALUATE("(5/2)/(5/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(5/2)/(5/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(5/2)/(5/2)"))))
 CASE index=66
  RETURN CFNMETA((5.00/2)/(5/2.00))+'|'+HexBytes(BINTOC((5.00/2)/(5/2.00),'BRS'))+'|'+HexBytes(TRANSFORM((5.00/2)/(5/2.00)))
 CASE index=67
  RETURN CFNMETA(+((5.00/2)/(5/2.00)))+'|'+HexBytes(BINTOC(+((5.00/2)/(5/2.00)),'BRS'))+'|'+HexBytes(TRANSFORM(+((5.00/2)/(5/2.00))))
 CASE index=68
  RETURN CFNMETA(-((5.00/2)/(5/2.00)))+'|'+HexBytes(BINTOC(-((5.00/2)/(5/2.00)),'BRS'))+'|'+HexBytes(TRANSFORM(-((5.00/2)/(5/2.00))))
 CASE index=69
  RETURN CFNMETA(-(-((5.00/2)/(5/2.00))))+'|'+HexBytes(BINTOC(-(-((5.00/2)/(5/2.00))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((5.00/2)/(5/2.00)))))
 CASE index=70
  RETURN CFNMETA(CFNCLONE((5.00/2)/(5/2.00)))+'|'+HexBytes(BINTOC(CFNCLONE((5.00/2)/(5/2.00)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE((5.00/2)/(5/2.00))))
 CASE index=71
  RETURN CFNMETA(-(CFNCLONE((5.00/2)/(5/2.00))))+'|'+HexBytes(BINTOC(-(CFNCLONE((5.00/2)/(5/2.00))),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE((5.00/2)/(5/2.00)))))
 CASE index=72
  RETURN CFNMETA(CFNCLONE(-((5.00/2)/(5/2.00))))+'|'+HexBytes(BINTOC(CFNCLONE(-((5.00/2)/(5/2.00))),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-((5.00/2)/(5/2.00)))))
 CASE index=73
  RETURN CFNMETA(ABS((5.00/2)/(5/2.00)))+'|'+HexBytes(BINTOC(ABS((5.00/2)/(5/2.00)),'BRS'))+'|'+HexBytes(TRANSFORM(ABS((5.00/2)/(5/2.00))))
 CASE index=74
  RETURN CFNMETA(-(ABS((5.00/2)/(5/2.00))))+'|'+HexBytes(BINTOC(-(ABS((5.00/2)/(5/2.00))),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS((5.00/2)/(5/2.00)))))
 CASE index=75
  RETURN CFNMETA(IIF(.T.,(5.00/2)/(5/2.00),0))+'|'+HexBytes(BINTOC(IIF(.T.,(5.00/2)/(5/2.00),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(5.00/2)/(5/2.00),0)))
 CASE index=76
  RETURN CFNMETA(-(IIF(.T.,(5.00/2)/(5/2.00),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(5.00/2)/(5/2.00),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(5.00/2)/(5/2.00),0))))
 CASE index=77
  RETURN CFNMETA(EVALUATE("(5.00/2)/(5/2.00)"))+'|'+HexBytes(BINTOC(EVALUATE("(5.00/2)/(5/2.00)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(5.00/2)/(5/2.00)")))
 CASE index=78
  RETURN CFNMETA(-(EVALUATE("(5.00/2)/(5/2.00)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(5.00/2)/(5/2.00)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(5.00/2)/(5/2.00)"))))
 CASE index=79
  RETURN CFNMETA((10/2)/2)+'|'+HexBytes(BINTOC((10/2)/2,'BRS'))+'|'+HexBytes(TRANSFORM((10/2)/2))
 CASE index=80
  RETURN CFNMETA(+((10/2)/2))+'|'+HexBytes(BINTOC(+((10/2)/2),'BRS'))+'|'+HexBytes(TRANSFORM(+((10/2)/2)))
 CASE index=81
  RETURN CFNMETA(-((10/2)/2))+'|'+HexBytes(BINTOC(-((10/2)/2),'BRS'))+'|'+HexBytes(TRANSFORM(-((10/2)/2)))
 CASE index=82
  RETURN CFNMETA(-(-((10/2)/2)))+'|'+HexBytes(BINTOC(-(-((10/2)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((10/2)/2))))
 CASE index=83
  RETURN CFNMETA(CFNCLONE((10/2)/2))+'|'+HexBytes(BINTOC(CFNCLONE((10/2)/2),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE((10/2)/2)))
 CASE index=84
  RETURN CFNMETA(-(CFNCLONE((10/2)/2)))+'|'+HexBytes(BINTOC(-(CFNCLONE((10/2)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE((10/2)/2))))
 CASE index=85
  RETURN CFNMETA(CFNCLONE(-((10/2)/2)))+'|'+HexBytes(BINTOC(CFNCLONE(-((10/2)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-((10/2)/2))))
 CASE index=86
  RETURN CFNMETA(ABS((10/2)/2))+'|'+HexBytes(BINTOC(ABS((10/2)/2),'BRS'))+'|'+HexBytes(TRANSFORM(ABS((10/2)/2)))
 CASE index=87
  RETURN CFNMETA(-(ABS((10/2)/2)))+'|'+HexBytes(BINTOC(-(ABS((10/2)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS((10/2)/2))))
 CASE index=88
  RETURN CFNMETA(IIF(.T.,(10/2)/2,0))+'|'+HexBytes(BINTOC(IIF(.T.,(10/2)/2,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(10/2)/2,0)))
 CASE index=89
  RETURN CFNMETA(-(IIF(.T.,(10/2)/2,0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(10/2)/2,0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(10/2)/2,0))))
 CASE index=90
  RETURN CFNMETA(EVALUATE("(10/2)/2"))+'|'+HexBytes(BINTOC(EVALUATE("(10/2)/2"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(10/2)/2")))
 CASE index=91
  RETURN CFNMETA(-(EVALUATE("(10/2)/2")))+'|'+HexBytes(BINTOC(-(EVALUATE("(10/2)/2")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(10/2)/2"))))
 CASE index=92
  RETURN CFNMETA(5/(2/2))+'|'+HexBytes(BINTOC(5/(2/2),'BRS'))+'|'+HexBytes(TRANSFORM(5/(2/2)))
 CASE index=93
  RETURN CFNMETA(+(5/(2/2)))+'|'+HexBytes(BINTOC(+(5/(2/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+(5/(2/2))))
 CASE index=94
  RETURN CFNMETA(-(5/(2/2)))+'|'+HexBytes(BINTOC(-(5/(2/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/(2/2))))
 CASE index=95
  RETURN CFNMETA(-(-(5/(2/2))))+'|'+HexBytes(BINTOC(-(-(5/(2/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(5/(2/2)))))
 CASE index=96
  RETURN CFNMETA(CFNCLONE(5/(2/2)))+'|'+HexBytes(BINTOC(CFNCLONE(5/(2/2)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(5/(2/2))))
 CASE index=97
  RETURN CFNMETA(-(CFNCLONE(5/(2/2))))+'|'+HexBytes(BINTOC(-(CFNCLONE(5/(2/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE(5/(2/2)))))
 CASE index=98
  RETURN CFNMETA(CFNCLONE(-(5/(2/2))))+'|'+HexBytes(BINTOC(CFNCLONE(-(5/(2/2))),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-(5/(2/2)))))
 CASE index=99
  RETURN CFNMETA(ABS(5/(2/2)))+'|'+HexBytes(BINTOC(ABS(5/(2/2)),'BRS'))+'|'+HexBytes(TRANSFORM(ABS(5/(2/2))))
 CASE index=100
  RETURN CFNMETA(-(ABS(5/(2/2))))+'|'+HexBytes(BINTOC(-(ABS(5/(2/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS(5/(2/2)))))
 CASE index=101
  RETURN CFNMETA(IIF(.T.,5/(2/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,5/(2/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,5/(2/2),0)))
 CASE index=102
  RETURN CFNMETA(-(IIF(.T.,5/(2/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,5/(2/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,5/(2/2),0))))
 CASE index=103
  RETURN CFNMETA(EVALUATE("5/(2/2)"))+'|'+HexBytes(BINTOC(EVALUATE("5/(2/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("5/(2/2)")))
 CASE index=104
  RETURN CFNMETA(-(EVALUATE("5/(2/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("5/(2/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("5/(2/2)"))))
 CASE index=105
  RETURN CFNMETA((1/7)/3)+'|'+HexBytes(BINTOC((1/7)/3,'BRS'))+'|'+HexBytes(TRANSFORM((1/7)/3))
 CASE index=106
  RETURN CFNMETA(+((1/7)/3))+'|'+HexBytes(BINTOC(+((1/7)/3),'BRS'))+'|'+HexBytes(TRANSFORM(+((1/7)/3)))
 CASE index=107
  RETURN CFNMETA(-((1/7)/3))+'|'+HexBytes(BINTOC(-((1/7)/3),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/7)/3)))
 CASE index=108
  RETURN CFNMETA(-(-((1/7)/3)))+'|'+HexBytes(BINTOC(-(-((1/7)/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((1/7)/3))))
 CASE index=109
  RETURN CFNMETA(CFNCLONE((1/7)/3))+'|'+HexBytes(BINTOC(CFNCLONE((1/7)/3),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE((1/7)/3)))
 CASE index=110
  RETURN CFNMETA(-(CFNCLONE((1/7)/3)))+'|'+HexBytes(BINTOC(-(CFNCLONE((1/7)/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE((1/7)/3))))
 CASE index=111
  RETURN CFNMETA(CFNCLONE(-((1/7)/3)))+'|'+HexBytes(BINTOC(CFNCLONE(-((1/7)/3)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-((1/7)/3))))
 CASE index=112
  RETURN CFNMETA(ABS((1/7)/3))+'|'+HexBytes(BINTOC(ABS((1/7)/3),'BRS'))+'|'+HexBytes(TRANSFORM(ABS((1/7)/3)))
 CASE index=113
  RETURN CFNMETA(-(ABS((1/7)/3)))+'|'+HexBytes(BINTOC(-(ABS((1/7)/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS((1/7)/3))))
 CASE index=114
  RETURN CFNMETA(IIF(.T.,(1/7)/3,0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/7)/3,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/7)/3,0)))
 CASE index=115
  RETURN CFNMETA(-(IIF(.T.,(1/7)/3,0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/7)/3,0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/7)/3,0))))
 CASE index=116
  RETURN CFNMETA(EVALUATE("(1/7)/3"))+'|'+HexBytes(BINTOC(EVALUATE("(1/7)/3"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(1/7)/3")))
 CASE index=117
  RETURN CFNMETA(-(EVALUATE("(1/7)/3")))+'|'+HexBytes(BINTOC(-(EVALUATE("(1/7)/3")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(1/7)/3"))))
 CASE index=118
  RETURN CFNMETA(5/(5/2))+'|'+HexBytes(BINTOC(5/(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(5/(5/2)))
 CASE index=119
  RETURN CFNMETA(+(5/(5/2)))+'|'+HexBytes(BINTOC(+(5/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+(5/(5/2))))
 CASE index=120
  RETURN CFNMETA(-(5/(5/2)))+'|'+HexBytes(BINTOC(-(5/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/(5/2))))
 CASE index=121
  RETURN CFNMETA(-(-(5/(5/2))))+'|'+HexBytes(BINTOC(-(-(5/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(5/(5/2)))))
 CASE index=122
  RETURN CFNMETA(CFNCLONE(5/(5/2)))+'|'+HexBytes(BINTOC(CFNCLONE(5/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(5/(5/2))))
 CASE index=123
  RETURN CFNMETA(-(CFNCLONE(5/(5/2))))+'|'+HexBytes(BINTOC(-(CFNCLONE(5/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE(5/(5/2)))))
 CASE index=124
  RETURN CFNMETA(CFNCLONE(-(5/(5/2))))+'|'+HexBytes(BINTOC(CFNCLONE(-(5/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(-(5/(5/2)))))
 CASE index=125
  RETURN CFNMETA(ABS(5/(5/2)))+'|'+HexBytes(BINTOC(ABS(5/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(ABS(5/(5/2))))
 CASE index=126
  RETURN CFNMETA(-(ABS(5/(5/2))))+'|'+HexBytes(BINTOC(-(ABS(5/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(ABS(5/(5/2)))))
 CASE index=127
  RETURN CFNMETA(IIF(.T.,5/(5/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,5/(5/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,5/(5/2),0)))
 CASE index=128
  RETURN CFNMETA(-(IIF(.T.,5/(5/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,5/(5/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,5/(5/2),0))))
 CASE index=129
  RETURN CFNMETA(EVALUATE("5/(5/2)"))+'|'+HexBytes(BINTOC(EVALUATE("5/(5/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("5/(5/2)")))
 CASE index=130
  RETURN CFNMETA(-(EVALUATE("5/(5/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("5/(5/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("5/(5/2)"))))
 CASE index=131
  RETURN CFNMETA(iFive/(5/2))+'|'+HexBytes(BINTOC(iFive/(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(iFive/(5/2)))
 CASE index=132
  RETURN CFNMETA((5/2)/iTwo)+'|'+HexBytes(BINTOC((5/2)/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM((5/2)/iTwo))
 CASE index=133
  RETURN CFNMETA(iFive/-(5/2))+'|'+HexBytes(BINTOC(iFive/-(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(iFive/-(5/2)))
 CASE index=134
  RETURN CFNMETA(-(iFive/(5/2)))+'|'+HexBytes(BINTOC(-(iFive/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(iFive/(5/2))))
 CASE index=135
  RETURN CFNMETA(-((5/2)/iTwo))+'|'+HexBytes(BINTOC(-((5/2)/iTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/2)/iTwo)))
 CASE index=136
  RETURN CFNMETA((5/2)/(iFive/iTwo))+'|'+HexBytes(BINTOC((5/2)/(iFive/iTwo),'BRS'))+'|'+HexBytes(TRANSFORM((5/2)/(iFive/iTwo)))
 CASE index=137
  RETURN CFNMETA((iFive/iTwo)/(5/2))+'|'+HexBytes(BINTOC((iFive/iTwo)/(5/2),'BRS'))+'|'+HexBytes(TRANSFORM((iFive/iTwo)/(5/2)))
 CASE index=138
  RETURN CFNMETA(5/(CFNCLONE(5)/2))+'|'+HexBytes(BINTOC(5/(CFNCLONE(5)/2),'BRS'))+'|'+HexBytes(TRANSFORM(5/(CFNCLONE(5)/2)))
 CASE index=139
  RETURN CFNMETA(CFNCLONE(5)/(5/2))+'|'+HexBytes(BINTOC(CFNCLONE(5)/(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(5)/(5/2)))
 CASE index=140
  RETURN CFNMETA(-(CFNCLONE(5)/(5/2)))+'|'+HexBytes(BINTOC(-(CFNCLONE(5)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(CFNCLONE(5)/(5/2))))
 CASE index=141
  RETURN CFNMETA(CFNCLONE(5)/-(5/2))+'|'+HexBytes(BINTOC(CFNCLONE(5)/-(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(CFNCLONE(5)/-(5/2)))
 CASE index=142
  RETURN CFNMETA((5/2)/CFNCLONE(2))+'|'+HexBytes(BINTOC((5/2)/CFNCLONE(2),'BRS'))+'|'+HexBytes(TRANSFORM((5/2)/CFNCLONE(2)))
 CASE index=143
  RETURN CFNMETA(-(5/2)/CFNCLONE(2))+'|'+HexBytes(BINTOC(-(5/2)/CFNCLONE(2),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/2)/CFNCLONE(2)))
 CASE index=144
  RETURN CFNMETA(-((5/2)/CFNCLONE(2)))+'|'+HexBytes(BINTOC(-((5/2)/CFNCLONE(2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/2)/CFNCLONE(2))))
 CASE index=145
  RETURN CFNMETA((5/2)/(CFNCLONE(5)/2))+'|'+HexBytes(BINTOC((5/2)/(CFNCLONE(5)/2),'BRS'))+'|'+HexBytes(TRANSFORM((5/2)/(CFNCLONE(5)/2)))
 CASE index=146
  RETURN CFNMETA((CFNCLONE(5)/2)/(5/2))+'|'+HexBytes(BINTOC((CFNCLONE(5)/2)/(5/2),'BRS'))+'|'+HexBytes(TRANSFORM((CFNCLONE(5)/2)/(5/2)))
 CASE index=147
  RETURN CFNMETA((5/2)/(IIF(.T.,5,0)/2))+'|'+HexBytes(BINTOC((5/2)/(IIF(.T.,5,0)/2),'BRS'))+'|'+HexBytes(TRANSFORM((5/2)/(IIF(.T.,5,0)/2)))
 CASE index=148
  RETURN CFNMETA(IIF(.T.,5,0)/(5/2))+'|'+HexBytes(BINTOC(IIF(.T.,5,0)/(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,5,0)/(5/2)))
 CASE index=149
  RETURN CFNMETA((5/2)/IIF(.T.,2,0))+'|'+HexBytes(BINTOC((5/2)/IIF(.T.,2,0),'BRS'))+'|'+HexBytes(TRANSFORM((5/2)/IIF(.T.,2,0)))
 CASE index=150
  RETURN CFNMETA(-(IIF(.T.,5,0)/(5/2)))+'|'+HexBytes(BINTOC(-(IIF(.T.,5,0)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,5,0)/(5/2))))
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
