* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DIVISION-CONTEXT-001: type-aware behavioral recovery only.
LOCAL ARRAY expressions[81], settings[2]
expressions[1]='5'
expressions[2]='2'
expressions[3]='5.0'
expressions[4]='2.0'
expressions[5]='5.00'
expressions[6]='2.00'
expressions[7]='iFive'
expressions[8]='iTwo'
expressions[9]='nFive'
expressions[10]='nTwo'
expressions[11]='INT(5.00)'
expressions[12]='INT(2.00)'
expressions[13]='VAL("5.00")'
expressions[14]='VAL("2.00")'
expressions[15]='EVALUATE("5")'
expressions[16]='EVALUATE("2")'
expressions[17]='EVALUATE("5.00")'
expressions[18]='EVALUATE("2.00")'
expressions[19]='5/2'
expressions[20]='5/2.0'
expressions[21]='5/2.00'
expressions[22]='5.0/2'
expressions[23]='5.0/2.0'
expressions[24]='5.0/2.00'
expressions[25]='5.00/2'
expressions[26]='5.00/2.0'
expressions[27]='5.00/2.00'
expressions[28]='iFive/iTwo'
expressions[29]='iFive/nTwo'
expressions[30]='nFive/iTwo'
expressions[31]='nFive/nTwo'
expressions[32]='nFive/2'
expressions[33]='5/nTwo'
expressions[34]='-(5/2)'
expressions[35]='-(5/2.0)'
expressions[36]='-(5/2.00)'
expressions[37]='-(5.0/2)'
expressions[38]='-(5.0/2.0)'
expressions[39]='-(5.0/2.00)'
expressions[40]='-(5.00/2)'
expressions[41]='-(5.00/2.0)'
expressions[42]='-(5.00/2.00)'
expressions[43]='-(iFive/iTwo)'
expressions[44]='-(iFive/nTwo)'
expressions[45]='-(nFive/iTwo)'
expressions[46]='-(nFive/nTwo)'
expressions[47]='-(nFive/2)'
expressions[48]='-(5/nTwo)'
expressions[49]='-5/2'
expressions[50]='-5/2.0'
expressions[51]='-5/2.00'
expressions[52]='-5.0/2'
expressions[53]='-5.0/2.0'
expressions[54]='-5.0/2.00'
expressions[55]='-5.00/2'
expressions[56]='-5.00/2.0'
expressions[57]='-5.00/2.00'
expressions[58]='EVALUATE("5/2")'
expressions[59]='-EVALUATE("5/2")'
expressions[60]='ABS(5/2)'
expressions[61]='-ABS(5/2)'
expressions[62]='IIF(.T.,5/2,1.00)'
expressions[63]='-IIF(.T.,5/2,1.00)'
expressions[64]='INT(5.00)/INT(2.00)'
expressions[65]='-(INT(5.00)/INT(2.00))'
expressions[66]='VAL("5.00")/VAL("2.00")'
expressions[67]='-(VAL("5.00")/VAL("2.00"))'
expressions[68]='EVALUATE("5")/EVALUATE("2")'
expressions[69]='-(EVALUATE("5")/EVALUATE("2"))'
expressions[70]='+(5/2)'
expressions[71]='-((5/2))'
expressions[72]='-(+(5/2))'
expressions[73]='-(-5/2)'
expressions[74]='(5/2)+0.00'
expressions[75]='-((5/2)+0.00)'
expressions[76]='(5.00/2.00)*1.00'
expressions[77]='-((5.00/2.00)*1.00)'
expressions[78]='(iFive/iTwo)+nOne'
expressions[79]='-((iFive/iTwo)+nOne)'
expressions[80]='(nFive/nTwo)/iOne'
expressions[81]='-((nFive/nTwo)/iOne)'
settings[1]=2
settings[2]=14
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught
PRIVATE iFive,iTwo,nFive,nTwo,nOne,iOne
outputPath='Z:\home\rich\.codex\worktrees\numeric-division-context-5611\Project-Copperfin\build\numeric-division-native\results.txt'
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
  RETURN CFNMETA(5)+'|'+HexBytes(BINTOC(5,'BRS'))+'|'+HexBytes(TRANSFORM(5))
 CASE index=2
  RETURN CFNMETA(2)+'|'+HexBytes(BINTOC(2,'BRS'))+'|'+HexBytes(TRANSFORM(2))
 CASE index=3
  RETURN CFNMETA(5.0)+'|'+HexBytes(BINTOC(5.0,'BRS'))+'|'+HexBytes(TRANSFORM(5.0))
 CASE index=4
  RETURN CFNMETA(2.0)+'|'+HexBytes(BINTOC(2.0,'BRS'))+'|'+HexBytes(TRANSFORM(2.0))
 CASE index=5
  RETURN CFNMETA(5.00)+'|'+HexBytes(BINTOC(5.00,'BRS'))+'|'+HexBytes(TRANSFORM(5.00))
 CASE index=6
  RETURN CFNMETA(2.00)+'|'+HexBytes(BINTOC(2.00,'BRS'))+'|'+HexBytes(TRANSFORM(2.00))
 CASE index=7
  RETURN CFNMETA(iFive)+'|'+HexBytes(BINTOC(iFive,'BRS'))+'|'+HexBytes(TRANSFORM(iFive))
 CASE index=8
  RETURN CFNMETA(iTwo)+'|'+HexBytes(BINTOC(iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(iTwo))
 CASE index=9
  RETURN CFNMETA(nFive)+'|'+HexBytes(BINTOC(nFive,'BRS'))+'|'+HexBytes(TRANSFORM(nFive))
 CASE index=10
  RETURN CFNMETA(nTwo)+'|'+HexBytes(BINTOC(nTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nTwo))
 CASE index=11
  RETURN CFNMETA(INT(5.00))+'|'+HexBytes(BINTOC(INT(5.00),'BRS'))+'|'+HexBytes(TRANSFORM(INT(5.00)))
 CASE index=12
  RETURN CFNMETA(INT(2.00))+'|'+HexBytes(BINTOC(INT(2.00),'BRS'))+'|'+HexBytes(TRANSFORM(INT(2.00)))
 CASE index=13
  RETURN CFNMETA(VAL("5.00"))+'|'+HexBytes(BINTOC(VAL("5.00"),'BRS'))+'|'+HexBytes(TRANSFORM(VAL("5.00")))
 CASE index=14
  RETURN CFNMETA(VAL("2.00"))+'|'+HexBytes(BINTOC(VAL("2.00"),'BRS'))+'|'+HexBytes(TRANSFORM(VAL("2.00")))
 CASE index=15
  RETURN CFNMETA(EVALUATE("5"))+'|'+HexBytes(BINTOC(EVALUATE("5"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("5")))
 CASE index=16
  RETURN CFNMETA(EVALUATE("2"))+'|'+HexBytes(BINTOC(EVALUATE("2"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("2")))
 CASE index=17
  RETURN CFNMETA(EVALUATE("5.00"))+'|'+HexBytes(BINTOC(EVALUATE("5.00"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("5.00")))
 CASE index=18
  RETURN CFNMETA(EVALUATE("2.00"))+'|'+HexBytes(BINTOC(EVALUATE("2.00"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("2.00")))
 CASE index=19
  RETURN CFNMETA(5/2)+'|'+HexBytes(BINTOC(5/2,'BRS'))+'|'+HexBytes(TRANSFORM(5/2))
 CASE index=20
  RETURN CFNMETA(5/2.0)+'|'+HexBytes(BINTOC(5/2.0,'BRS'))+'|'+HexBytes(TRANSFORM(5/2.0))
 CASE index=21
  RETURN CFNMETA(5/2.00)+'|'+HexBytes(BINTOC(5/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(5/2.00))
 CASE index=22
  RETURN CFNMETA(5.0/2)+'|'+HexBytes(BINTOC(5.0/2,'BRS'))+'|'+HexBytes(TRANSFORM(5.0/2))
 CASE index=23
  RETURN CFNMETA(5.0/2.0)+'|'+HexBytes(BINTOC(5.0/2.0,'BRS'))+'|'+HexBytes(TRANSFORM(5.0/2.0))
 CASE index=24
  RETURN CFNMETA(5.0/2.00)+'|'+HexBytes(BINTOC(5.0/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(5.0/2.00))
 CASE index=25
  RETURN CFNMETA(5.00/2)+'|'+HexBytes(BINTOC(5.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(5.00/2))
 CASE index=26
  RETURN CFNMETA(5.00/2.0)+'|'+HexBytes(BINTOC(5.00/2.0,'BRS'))+'|'+HexBytes(TRANSFORM(5.00/2.0))
 CASE index=27
  RETURN CFNMETA(5.00/2.00)+'|'+HexBytes(BINTOC(5.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(5.00/2.00))
 CASE index=28
  RETURN CFNMETA(iFive/iTwo)+'|'+HexBytes(BINTOC(iFive/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(iFive/iTwo))
 CASE index=29
  RETURN CFNMETA(iFive/nTwo)+'|'+HexBytes(BINTOC(iFive/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM(iFive/nTwo))
 CASE index=30
  RETURN CFNMETA(nFive/iTwo)+'|'+HexBytes(BINTOC(nFive/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nFive/iTwo))
 CASE index=31
  RETURN CFNMETA(nFive/nTwo)+'|'+HexBytes(BINTOC(nFive/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM(nFive/nTwo))
 CASE index=32
  RETURN CFNMETA(nFive/2)+'|'+HexBytes(BINTOC(nFive/2,'BRS'))+'|'+HexBytes(TRANSFORM(nFive/2))
 CASE index=33
  RETURN CFNMETA(5/nTwo)+'|'+HexBytes(BINTOC(5/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM(5/nTwo))
 CASE index=34
  RETURN CFNMETA(-(5/2))+'|'+HexBytes(BINTOC(-(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/2)))
 CASE index=35
  RETURN CFNMETA(-(5/2.0))+'|'+HexBytes(BINTOC(-(5/2.0),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/2.0)))
 CASE index=36
  RETURN CFNMETA(-(5/2.00))+'|'+HexBytes(BINTOC(-(5/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/2.00)))
 CASE index=37
  RETURN CFNMETA(-(5.0/2))+'|'+HexBytes(BINTOC(-(5.0/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.0/2)))
 CASE index=38
  RETURN CFNMETA(-(5.0/2.0))+'|'+HexBytes(BINTOC(-(5.0/2.0),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.0/2.0)))
 CASE index=39
  RETURN CFNMETA(-(5.0/2.00))+'|'+HexBytes(BINTOC(-(5.0/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.0/2.00)))
 CASE index=40
  RETURN CFNMETA(-(5.00/2))+'|'+HexBytes(BINTOC(-(5.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.00/2)))
 CASE index=41
  RETURN CFNMETA(-(5.00/2.0))+'|'+HexBytes(BINTOC(-(5.00/2.0),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.00/2.0)))
 CASE index=42
  RETURN CFNMETA(-(5.00/2.00))+'|'+HexBytes(BINTOC(-(5.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(5.00/2.00)))
 CASE index=43
  RETURN CFNMETA(-(iFive/iTwo))+'|'+HexBytes(BINTOC(-(iFive/iTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-(iFive/iTwo)))
 CASE index=44
  RETURN CFNMETA(-(iFive/nTwo))+'|'+HexBytes(BINTOC(-(iFive/nTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-(iFive/nTwo)))
 CASE index=45
  RETURN CFNMETA(-(nFive/iTwo))+'|'+HexBytes(BINTOC(-(nFive/iTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-(nFive/iTwo)))
 CASE index=46
  RETURN CFNMETA(-(nFive/nTwo))+'|'+HexBytes(BINTOC(-(nFive/nTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-(nFive/nTwo)))
 CASE index=47
  RETURN CFNMETA(-(nFive/2))+'|'+HexBytes(BINTOC(-(nFive/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(nFive/2)))
 CASE index=48
  RETURN CFNMETA(-(5/nTwo))+'|'+HexBytes(BINTOC(-(5/nTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/nTwo)))
 CASE index=49
  RETURN CFNMETA(-5/2)+'|'+HexBytes(BINTOC(-5/2,'BRS'))+'|'+HexBytes(TRANSFORM(-5/2))
 CASE index=50
  RETURN CFNMETA(-5/2.0)+'|'+HexBytes(BINTOC(-5/2.0,'BRS'))+'|'+HexBytes(TRANSFORM(-5/2.0))
 CASE index=51
  RETURN CFNMETA(-5/2.00)+'|'+HexBytes(BINTOC(-5/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-5/2.00))
 CASE index=52
  RETURN CFNMETA(-5.0/2)+'|'+HexBytes(BINTOC(-5.0/2,'BRS'))+'|'+HexBytes(TRANSFORM(-5.0/2))
 CASE index=53
  RETURN CFNMETA(-5.0/2.0)+'|'+HexBytes(BINTOC(-5.0/2.0,'BRS'))+'|'+HexBytes(TRANSFORM(-5.0/2.0))
 CASE index=54
  RETURN CFNMETA(-5.0/2.00)+'|'+HexBytes(BINTOC(-5.0/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-5.0/2.00))
 CASE index=55
  RETURN CFNMETA(-5.00/2)+'|'+HexBytes(BINTOC(-5.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(-5.00/2))
 CASE index=56
  RETURN CFNMETA(-5.00/2.0)+'|'+HexBytes(BINTOC(-5.00/2.0,'BRS'))+'|'+HexBytes(TRANSFORM(-5.00/2.0))
 CASE index=57
  RETURN CFNMETA(-5.00/2.00)+'|'+HexBytes(BINTOC(-5.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-5.00/2.00))
 CASE index=58
  RETURN CFNMETA(EVALUATE("5/2"))+'|'+HexBytes(BINTOC(EVALUATE("5/2"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("5/2")))
 CASE index=59
  RETURN CFNMETA(-EVALUATE("5/2"))+'|'+HexBytes(BINTOC(-EVALUATE("5/2"),'BRS'))+'|'+HexBytes(TRANSFORM(-EVALUATE("5/2")))
 CASE index=60
  RETURN CFNMETA(ABS(5/2))+'|'+HexBytes(BINTOC(ABS(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(ABS(5/2)))
 CASE index=61
  RETURN CFNMETA(-ABS(5/2))+'|'+HexBytes(BINTOC(-ABS(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(-ABS(5/2)))
 CASE index=62
  RETURN CFNMETA(IIF(.T.,5/2,1.00))+'|'+HexBytes(BINTOC(IIF(.T.,5/2,1.00),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,5/2,1.00)))
 CASE index=63
  RETURN CFNMETA(-IIF(.T.,5/2,1.00))+'|'+HexBytes(BINTOC(-IIF(.T.,5/2,1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-IIF(.T.,5/2,1.00)))
 CASE index=64
  RETURN CFNMETA(INT(5.00)/INT(2.00))+'|'+HexBytes(BINTOC(INT(5.00)/INT(2.00),'BRS'))+'|'+HexBytes(TRANSFORM(INT(5.00)/INT(2.00)))
 CASE index=65
  RETURN CFNMETA(-(INT(5.00)/INT(2.00)))+'|'+HexBytes(BINTOC(-(INT(5.00)/INT(2.00)),'BRS'))+'|'+HexBytes(TRANSFORM(-(INT(5.00)/INT(2.00))))
 CASE index=66
  RETURN CFNMETA(VAL("5.00")/VAL("2.00"))+'|'+HexBytes(BINTOC(VAL("5.00")/VAL("2.00"),'BRS'))+'|'+HexBytes(TRANSFORM(VAL("5.00")/VAL("2.00")))
 CASE index=67
  RETURN CFNMETA(-(VAL("5.00")/VAL("2.00")))+'|'+HexBytes(BINTOC(-(VAL("5.00")/VAL("2.00")),'BRS'))+'|'+HexBytes(TRANSFORM(-(VAL("5.00")/VAL("2.00"))))
 CASE index=68
  RETURN CFNMETA(EVALUATE("5")/EVALUATE("2"))+'|'+HexBytes(BINTOC(EVALUATE("5")/EVALUATE("2"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("5")/EVALUATE("2")))
 CASE index=69
  RETURN CFNMETA(-(EVALUATE("5")/EVALUATE("2")))+'|'+HexBytes(BINTOC(-(EVALUATE("5")/EVALUATE("2")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("5")/EVALUATE("2"))))
 CASE index=70
  RETURN CFNMETA(+(5/2))+'|'+HexBytes(BINTOC(+(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(+(5/2)))
 CASE index=71
  RETURN CFNMETA(-((5/2)))+'|'+HexBytes(BINTOC(-((5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/2))))
 CASE index=72
  RETURN CFNMETA(-(+(5/2)))+'|'+HexBytes(BINTOC(-(+(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(+(5/2))))
 CASE index=73
  RETURN CFNMETA(-(-5/2))+'|'+HexBytes(BINTOC(-(-5/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-5/2)))
 CASE index=74
  RETURN CFNMETA((5/2)+0.00)+'|'+HexBytes(BINTOC((5/2)+0.00,'BRS'))+'|'+HexBytes(TRANSFORM((5/2)+0.00))
 CASE index=75
  RETURN CFNMETA(-((5/2)+0.00))+'|'+HexBytes(BINTOC(-((5/2)+0.00),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/2)+0.00)))
 CASE index=76
  RETURN CFNMETA((5.00/2.00)*1.00)+'|'+HexBytes(BINTOC((5.00/2.00)*1.00,'BRS'))+'|'+HexBytes(TRANSFORM((5.00/2.00)*1.00))
 CASE index=77
  RETURN CFNMETA(-((5.00/2.00)*1.00))+'|'+HexBytes(BINTOC(-((5.00/2.00)*1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-((5.00/2.00)*1.00)))
 CASE index=78
  RETURN CFNMETA((iFive/iTwo)+nOne)+'|'+HexBytes(BINTOC((iFive/iTwo)+nOne,'BRS'))+'|'+HexBytes(TRANSFORM((iFive/iTwo)+nOne))
 CASE index=79
  RETURN CFNMETA(-((iFive/iTwo)+nOne))+'|'+HexBytes(BINTOC(-((iFive/iTwo)+nOne),'BRS'))+'|'+HexBytes(TRANSFORM(-((iFive/iTwo)+nOne)))
 CASE index=80
  RETURN CFNMETA((nFive/nTwo)/iOne)+'|'+HexBytes(BINTOC((nFive/nTwo)/iOne,'BRS'))+'|'+HexBytes(TRANSFORM((nFive/nTwo)/iOne))
 CASE index=81
  RETURN CFNMETA(-((nFive/nTwo)/iOne))+'|'+HexBytes(BINTOC(-((nFive/nTwo)/iOne),'BRS'))+'|'+HexBytes(TRANSFORM(-((nFive/nTwo)/iOne)))
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
