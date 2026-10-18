* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001: single-evaluation timing.
LOCAL ARRAY expressions[147],settings[5]
expressions[1]='CFObserve((0.5/2),0)'
expressions[2]='-(CFObserve((0.5/2),0))'
expressions[3]='CFObserve(-((0.5/2)),0)'
expressions[4]='(0.5/2)/CFMark(0)'
expressions[5]='-((0.5/2))/CFMark(0)'
expressions[6]='-(((0.5/2))/CFMark(0))'
expressions[7]='CFMark(0)/(0.5/2)'
expressions[8]='0.5/2/CFMark(0)'
expressions[9]='IIF(.T.,(0.5/2),0)/CFMark(0)'
expressions[10]='CFObserve((0.5/2),14)'
expressions[11]='-(CFObserve((0.5/2),14))'
expressions[12]='CFObserve(-((0.5/2)),14)'
expressions[13]='(0.5/2)/CFMark(14)'
expressions[14]='-((0.5/2))/CFMark(14)'
expressions[15]='-(((0.5/2))/CFMark(14))'
expressions[16]='CFMark(14)/(0.5/2)'
expressions[17]='0.5/2/CFMark(14)'
expressions[18]='IIF(.T.,(0.5/2),0)/CFMark(14)'
expressions[19]='CFObserve(((1/7)/3),0)'
expressions[20]='-(CFObserve(((1/7)/3),0))'
expressions[21]='CFObserve(-(((1/7)/3)),0)'
expressions[22]='((1/7)/3)/CFMark(0)'
expressions[23]='-(((1/7)/3))/CFMark(0)'
expressions[24]='-((((1/7)/3))/CFMark(0))'
expressions[25]='CFMark(0)/((1/7)/3)'
expressions[26]='(1/7)/3/CFMark(0)'
expressions[27]='IIF(.T.,((1/7)/3),0)/CFMark(0)'
expressions[28]='CFObserve(((1/7)/3),14)'
expressions[29]='-(CFObserve(((1/7)/3),14))'
expressions[30]='CFObserve(-(((1/7)/3)),14)'
expressions[31]='((1/7)/3)/CFMark(14)'
expressions[32]='-(((1/7)/3))/CFMark(14)'
expressions[33]='-((((1/7)/3))/CFMark(14))'
expressions[34]='CFMark(14)/((1/7)/3)'
expressions[35]='(1/7)/3/CFMark(14)'
expressions[36]='IIF(.T.,((1/7)/3),0)/CFMark(14)'
expressions[37]='CFObserve(((5/2)/(5/2)),0)'
expressions[38]='-(CFObserve(((5/2)/(5/2)),0))'
expressions[39]='CFObserve(-(((5/2)/(5/2))),0)'
expressions[40]='((5/2)/(5/2))/CFMark(0)'
expressions[41]='-(((5/2)/(5/2)))/CFMark(0)'
expressions[42]='-((((5/2)/(5/2)))/CFMark(0))'
expressions[43]='CFMark(0)/((5/2)/(5/2))'
expressions[44]='(5/2)/(5/2)/CFMark(0)'
expressions[45]='IIF(.T.,((5/2)/(5/2)),0)/CFMark(0)'
expressions[46]='CFObserve(((5/2)/(5/2)),14)'
expressions[47]='-(CFObserve(((5/2)/(5/2)),14))'
expressions[48]='CFObserve(-(((5/2)/(5/2))),14)'
expressions[49]='((5/2)/(5/2))/CFMark(14)'
expressions[50]='-(((5/2)/(5/2)))/CFMark(14)'
expressions[51]='-((((5/2)/(5/2)))/CFMark(14))'
expressions[52]='CFMark(14)/((5/2)/(5/2))'
expressions[53]='(5/2)/(5/2)/CFMark(14)'
expressions[54]='IIF(.T.,((5/2)/(5/2)),0)/CFMark(14)'
expressions[55]='CFObserve((5/(5/2)),0)'
expressions[56]='-(CFObserve((5/(5/2)),0))'
expressions[57]='CFObserve(-((5/(5/2))),0)'
expressions[58]='(5/(5/2))/CFMark(0)'
expressions[59]='-((5/(5/2)))/CFMark(0)'
expressions[60]='-(((5/(5/2)))/CFMark(0))'
expressions[61]='CFMark(0)/(5/(5/2))'
expressions[62]='5/(5/2)/CFMark(0)'
expressions[63]='IIF(.T.,(5/(5/2)),0)/CFMark(0)'
expressions[64]='CFObserve((5/(5/2)),14)'
expressions[65]='-(CFObserve((5/(5/2)),14))'
expressions[66]='CFObserve(-((5/(5/2))),14)'
expressions[67]='(5/(5/2))/CFMark(14)'
expressions[68]='-((5/(5/2)))/CFMark(14)'
expressions[69]='-(((5/(5/2)))/CFMark(14))'
expressions[70]='CFMark(14)/(5/(5/2))'
expressions[71]='5/(5/2)/CFMark(14)'
expressions[72]='IIF(.T.,(5/(5/2)),0)/CFMark(14)'
expressions[73]='CFObserve((1.234567890123456789/3),0)'
expressions[74]='-(CFObserve((1.234567890123456789/3),0))'
expressions[75]='CFObserve(-((1.234567890123456789/3)),0)'
expressions[76]='(1.234567890123456789/3)/CFMark(0)'
expressions[77]='-((1.234567890123456789/3))/CFMark(0)'
expressions[78]='-(((1.234567890123456789/3))/CFMark(0))'
expressions[79]='CFMark(0)/(1.234567890123456789/3)'
expressions[80]='1.234567890123456789/3/CFMark(0)'
expressions[81]='IIF(.T.,(1.234567890123456789/3),0)/CFMark(0)'
expressions[82]='CFObserve((1.234567890123456789/3),14)'
expressions[83]='-(CFObserve((1.234567890123456789/3),14))'
expressions[84]='CFObserve(-((1.234567890123456789/3)),14)'
expressions[85]='(1.234567890123456789/3)/CFMark(14)'
expressions[86]='-((1.234567890123456789/3))/CFMark(14)'
expressions[87]='-(((1.234567890123456789/3))/CFMark(14))'
expressions[88]='CFMark(14)/(1.234567890123456789/3)'
expressions[89]='1.234567890123456789/3/CFMark(14)'
expressions[90]='IIF(.T.,(1.234567890123456789/3),0)/CFMark(14)'
expressions[91]='CFObserve((0.9999999999999999/2.5),0)'
expressions[92]='-(CFObserve((0.9999999999999999/2.5),0))'
expressions[93]='CFObserve(-((0.9999999999999999/2.5)),0)'
expressions[94]='(0.9999999999999999/2.5)/CFMark(0)'
expressions[95]='-((0.9999999999999999/2.5))/CFMark(0)'
expressions[96]='-(((0.9999999999999999/2.5))/CFMark(0))'
expressions[97]='CFMark(0)/(0.9999999999999999/2.5)'
expressions[98]='0.9999999999999999/2.5/CFMark(0)'
expressions[99]='IIF(.T.,(0.9999999999999999/2.5),0)/CFMark(0)'
expressions[100]='CFObserve((0.9999999999999999/2.5),14)'
expressions[101]='-(CFObserve((0.9999999999999999/2.5),14))'
expressions[102]='CFObserve(-((0.9999999999999999/2.5)),14)'
expressions[103]='(0.9999999999999999/2.5)/CFMark(14)'
expressions[104]='-((0.9999999999999999/2.5))/CFMark(14)'
expressions[105]='-(((0.9999999999999999/2.5))/CFMark(14))'
expressions[106]='CFMark(14)/(0.9999999999999999/2.5)'
expressions[107]='0.9999999999999999/2.5/CFMark(14)'
expressions[108]='IIF(.T.,(0.9999999999999999/2.5),0)/CFMark(14)'
expressions[109]='CFObserve((-(0.00/2)),0)'
expressions[110]='-(CFObserve((-(0.00/2)),0))'
expressions[111]='CFObserve(-((-(0.00/2))),0)'
expressions[112]='(-(0.00/2))/CFMark(0)'
expressions[113]='-((-(0.00/2)))/CFMark(0)'
expressions[114]='-(((-(0.00/2)))/CFMark(0))'
expressions[115]='CFMark(0)/(-(0.00/2))'
expressions[116]='-(0.00/2)/CFMark(0)'
expressions[117]='IIF(.T.,(-(0.00/2)),0)/CFMark(0)'
expressions[118]='CFObserve((-(0.00/2)),14)'
expressions[119]='-(CFObserve((-(0.00/2)),14))'
expressions[120]='CFObserve(-((-(0.00/2))),14)'
expressions[121]='(-(0.00/2))/CFMark(14)'
expressions[122]='-((-(0.00/2)))/CFMark(14)'
expressions[123]='-(((-(0.00/2)))/CFMark(14))'
expressions[124]='CFMark(14)/(-(0.00/2))'
expressions[125]='-(0.00/2)/CFMark(14)'
expressions[126]='IIF(.T.,(-(0.00/2)),0)/CFMark(14)'
expressions[127]='CFObserve(((0.00/2)/2),0)'
expressions[128]='-(CFObserve(((0.00/2)/2),0))'
expressions[129]='CFObserve(-(((0.00/2)/2)),0)'
expressions[130]='((0.00/2)/2)/CFMark(0)'
expressions[131]='-(((0.00/2)/2))/CFMark(0)'
expressions[132]='-((((0.00/2)/2))/CFMark(0))'
expressions[133]='CFMark(0)/((0.00/2)/2)'
expressions[134]='(0.00/2)/2/CFMark(0)'
expressions[135]='IIF(.T.,((0.00/2)/2),0)/CFMark(0)'
expressions[136]='CFObserve(((0.00/2)/2),14)'
expressions[137]='-(CFObserve(((0.00/2)/2),14))'
expressions[138]='CFObserve(-(((0.00/2)/2)),14)'
expressions[139]='((0.00/2)/2)/CFMark(14)'
expressions[140]='-(((0.00/2)/2))/CFMark(14)'
expressions[141]='-((((0.00/2)/2))/CFMark(14))'
expressions[142]='CFMark(14)/((0.00/2)/2)'
expressions[143]='(0.00/2)/2/CFMark(14)'
expressions[144]='IIF(.T.,((0.00/2)/2),0)/CFMark(14)'
expressions[145]='(5/2)/(5/2)'
expressions[146]='(1/7)/3'
expressions[147]='5/(5/2)'
settings[1]=0
settings[2]=2
settings[3]=6
settings[4]=14
settings[5]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,modeNo,mode
PRIVATE gCalls,gSeen
outputPath='Z:\home\rich\.codex\worktrees\numeric-nonwhole-division-5611\Project-Copperfin\build\nonwhole-native\timing-results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR settingNo=1 TO ALEN(settings)
 decimalCount=settings[settingNo]
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
   FOR modeNo=1 TO 2
    SET DECIMALS TO decimalCount
    gCalls=0
    gSeen=''
    IF modeNo=1
     mode='EVAL'
    ELSE
     mode='COMPILED'
    ENDIF
    TRY
     IF modeNo=1
      observation=CFNMETA(EVALUATE(expression))
     ELSE
      observation=CompiledObservation(index)
     ENDIF
     line=mode+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation+'|'+HexBytes(gSeen)+'|'+ALLTRIM(STR(gCalls,6,0))+'|'+ALLTRIM(STR(SET('DECIMALS'),4,0))
    CATCH TO caught
     line='ERROR-'+mode+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
    ENDTRY
    =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
    total=total+1
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO


FUNCTION CompiledObservation
 LPARAMETERS index
 DO CASE
 CASE index=1
  RETURN CFNMETA(CFObserve((0.5/2),0))
 CASE index=2
  RETURN CFNMETA(-(CFObserve((0.5/2),0)))
 CASE index=3
  RETURN CFNMETA(CFObserve(-((0.5/2)),0))
 CASE index=4
  RETURN CFNMETA((0.5/2)/CFMark(0))
 CASE index=5
  RETURN CFNMETA(-((0.5/2))/CFMark(0))
 CASE index=6
  RETURN CFNMETA(-(((0.5/2))/CFMark(0)))
 CASE index=7
  RETURN CFNMETA(CFMark(0)/(0.5/2))
 CASE index=8
  RETURN CFNMETA(0.5/2/CFMark(0))
 CASE index=9
  RETURN CFNMETA(IIF(.T.,(0.5/2),0)/CFMark(0))
 CASE index=10
  RETURN CFNMETA(CFObserve((0.5/2),14))
 CASE index=11
  RETURN CFNMETA(-(CFObserve((0.5/2),14)))
 CASE index=12
  RETURN CFNMETA(CFObserve(-((0.5/2)),14))
 CASE index=13
  RETURN CFNMETA((0.5/2)/CFMark(14))
 CASE index=14
  RETURN CFNMETA(-((0.5/2))/CFMark(14))
 CASE index=15
  RETURN CFNMETA(-(((0.5/2))/CFMark(14)))
 CASE index=16
  RETURN CFNMETA(CFMark(14)/(0.5/2))
 CASE index=17
  RETURN CFNMETA(0.5/2/CFMark(14))
 CASE index=18
  RETURN CFNMETA(IIF(.T.,(0.5/2),0)/CFMark(14))
 CASE index=19
  RETURN CFNMETA(CFObserve(((1/7)/3),0))
 CASE index=20
  RETURN CFNMETA(-(CFObserve(((1/7)/3),0)))
 CASE index=21
  RETURN CFNMETA(CFObserve(-(((1/7)/3)),0))
 CASE index=22
  RETURN CFNMETA(((1/7)/3)/CFMark(0))
 CASE index=23
  RETURN CFNMETA(-(((1/7)/3))/CFMark(0))
 CASE index=24
  RETURN CFNMETA(-((((1/7)/3))/CFMark(0)))
 CASE index=25
  RETURN CFNMETA(CFMark(0)/((1/7)/3))
 CASE index=26
  RETURN CFNMETA((1/7)/3/CFMark(0))
 CASE index=27
  RETURN CFNMETA(IIF(.T.,((1/7)/3),0)/CFMark(0))
 CASE index=28
  RETURN CFNMETA(CFObserve(((1/7)/3),14))
 CASE index=29
  RETURN CFNMETA(-(CFObserve(((1/7)/3),14)))
 CASE index=30
  RETURN CFNMETA(CFObserve(-(((1/7)/3)),14))
 CASE index=31
  RETURN CFNMETA(((1/7)/3)/CFMark(14))
 CASE index=32
  RETURN CFNMETA(-(((1/7)/3))/CFMark(14))
 CASE index=33
  RETURN CFNMETA(-((((1/7)/3))/CFMark(14)))
 CASE index=34
  RETURN CFNMETA(CFMark(14)/((1/7)/3))
 CASE index=35
  RETURN CFNMETA((1/7)/3/CFMark(14))
 CASE index=36
  RETURN CFNMETA(IIF(.T.,((1/7)/3),0)/CFMark(14))
 CASE index=37
  RETURN CFNMETA(CFObserve(((5/2)/(5/2)),0))
 CASE index=38
  RETURN CFNMETA(-(CFObserve(((5/2)/(5/2)),0)))
 CASE index=39
  RETURN CFNMETA(CFObserve(-(((5/2)/(5/2))),0))
 CASE index=40
  RETURN CFNMETA(((5/2)/(5/2))/CFMark(0))
 CASE index=41
  RETURN CFNMETA(-(((5/2)/(5/2)))/CFMark(0))
 CASE index=42
  RETURN CFNMETA(-((((5/2)/(5/2)))/CFMark(0)))
 CASE index=43
  RETURN CFNMETA(CFMark(0)/((5/2)/(5/2)))
 CASE index=44
  RETURN CFNMETA((5/2)/(5/2)/CFMark(0))
 CASE index=45
  RETURN CFNMETA(IIF(.T.,((5/2)/(5/2)),0)/CFMark(0))
 CASE index=46
  RETURN CFNMETA(CFObserve(((5/2)/(5/2)),14))
 CASE index=47
  RETURN CFNMETA(-(CFObserve(((5/2)/(5/2)),14)))
 CASE index=48
  RETURN CFNMETA(CFObserve(-(((5/2)/(5/2))),14))
 CASE index=49
  RETURN CFNMETA(((5/2)/(5/2))/CFMark(14))
 CASE index=50
  RETURN CFNMETA(-(((5/2)/(5/2)))/CFMark(14))
 CASE index=51
  RETURN CFNMETA(-((((5/2)/(5/2)))/CFMark(14)))
 CASE index=52
  RETURN CFNMETA(CFMark(14)/((5/2)/(5/2)))
 CASE index=53
  RETURN CFNMETA((5/2)/(5/2)/CFMark(14))
 CASE index=54
  RETURN CFNMETA(IIF(.T.,((5/2)/(5/2)),0)/CFMark(14))
 CASE index=55
  RETURN CFNMETA(CFObserve((5/(5/2)),0))
 CASE index=56
  RETURN CFNMETA(-(CFObserve((5/(5/2)),0)))
 CASE index=57
  RETURN CFNMETA(CFObserve(-((5/(5/2))),0))
 CASE index=58
  RETURN CFNMETA((5/(5/2))/CFMark(0))
 CASE index=59
  RETURN CFNMETA(-((5/(5/2)))/CFMark(0))
 CASE index=60
  RETURN CFNMETA(-(((5/(5/2)))/CFMark(0)))
 CASE index=61
  RETURN CFNMETA(CFMark(0)/(5/(5/2)))
 CASE index=62
  RETURN CFNMETA(5/(5/2)/CFMark(0))
 CASE index=63
  RETURN CFNMETA(IIF(.T.,(5/(5/2)),0)/CFMark(0))
 CASE index=64
  RETURN CFNMETA(CFObserve((5/(5/2)),14))
 CASE index=65
  RETURN CFNMETA(-(CFObserve((5/(5/2)),14)))
 CASE index=66
  RETURN CFNMETA(CFObserve(-((5/(5/2))),14))
 CASE index=67
  RETURN CFNMETA((5/(5/2))/CFMark(14))
 CASE index=68
  RETURN CFNMETA(-((5/(5/2)))/CFMark(14))
 CASE index=69
  RETURN CFNMETA(-(((5/(5/2)))/CFMark(14)))
 CASE index=70
  RETURN CFNMETA(CFMark(14)/(5/(5/2)))
 CASE index=71
  RETURN CFNMETA(5/(5/2)/CFMark(14))
 CASE index=72
  RETURN CFNMETA(IIF(.T.,(5/(5/2)),0)/CFMark(14))
 CASE index=73
  RETURN CFNMETA(CFObserve((1.234567890123456789/3),0))
 CASE index=74
  RETURN CFNMETA(-(CFObserve((1.234567890123456789/3),0)))
 CASE index=75
  RETURN CFNMETA(CFObserve(-((1.234567890123456789/3)),0))
 CASE index=76
  RETURN CFNMETA((1.234567890123456789/3)/CFMark(0))
 CASE index=77
  RETURN CFNMETA(-((1.234567890123456789/3))/CFMark(0))
 CASE index=78
  RETURN CFNMETA(-(((1.234567890123456789/3))/CFMark(0)))
 CASE index=79
  RETURN CFNMETA(CFMark(0)/(1.234567890123456789/3))
 CASE index=80
  RETURN CFNMETA(1.234567890123456789/3/CFMark(0))
 CASE index=81
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/3),0)/CFMark(0))
 CASE index=82
  RETURN CFNMETA(CFObserve((1.234567890123456789/3),14))
 CASE index=83
  RETURN CFNMETA(-(CFObserve((1.234567890123456789/3),14)))
 CASE index=84
  RETURN CFNMETA(CFObserve(-((1.234567890123456789/3)),14))
 CASE index=85
  RETURN CFNMETA((1.234567890123456789/3)/CFMark(14))
 CASE index=86
  RETURN CFNMETA(-((1.234567890123456789/3))/CFMark(14))
 CASE index=87
  RETURN CFNMETA(-(((1.234567890123456789/3))/CFMark(14)))
 CASE index=88
  RETURN CFNMETA(CFMark(14)/(1.234567890123456789/3))
 CASE index=89
  RETURN CFNMETA(1.234567890123456789/3/CFMark(14))
 CASE index=90
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/3),0)/CFMark(14))
 CASE index=91
  RETURN CFNMETA(CFObserve((0.9999999999999999/2.5),0))
 CASE index=92
  RETURN CFNMETA(-(CFObserve((0.9999999999999999/2.5),0)))
 CASE index=93
  RETURN CFNMETA(CFObserve(-((0.9999999999999999/2.5)),0))
 CASE index=94
  RETURN CFNMETA((0.9999999999999999/2.5)/CFMark(0))
 CASE index=95
  RETURN CFNMETA(-((0.9999999999999999/2.5))/CFMark(0))
 CASE index=96
  RETURN CFNMETA(-(((0.9999999999999999/2.5))/CFMark(0)))
 CASE index=97
  RETURN CFNMETA(CFMark(0)/(0.9999999999999999/2.5))
 CASE index=98
  RETURN CFNMETA(0.9999999999999999/2.5/CFMark(0))
 CASE index=99
  RETURN CFNMETA(IIF(.T.,(0.9999999999999999/2.5),0)/CFMark(0))
 CASE index=100
  RETURN CFNMETA(CFObserve((0.9999999999999999/2.5),14))
 CASE index=101
  RETURN CFNMETA(-(CFObserve((0.9999999999999999/2.5),14)))
 CASE index=102
  RETURN CFNMETA(CFObserve(-((0.9999999999999999/2.5)),14))
 CASE index=103
  RETURN CFNMETA((0.9999999999999999/2.5)/CFMark(14))
 CASE index=104
  RETURN CFNMETA(-((0.9999999999999999/2.5))/CFMark(14))
 CASE index=105
  RETURN CFNMETA(-(((0.9999999999999999/2.5))/CFMark(14)))
 CASE index=106
  RETURN CFNMETA(CFMark(14)/(0.9999999999999999/2.5))
 CASE index=107
  RETURN CFNMETA(0.9999999999999999/2.5/CFMark(14))
 CASE index=108
  RETURN CFNMETA(IIF(.T.,(0.9999999999999999/2.5),0)/CFMark(14))
 CASE index=109
  RETURN CFNMETA(CFObserve((-(0.00/2)),0))
 CASE index=110
  RETURN CFNMETA(-(CFObserve((-(0.00/2)),0)))
 CASE index=111
  RETURN CFNMETA(CFObserve(-((-(0.00/2))),0))
 CASE index=112
  RETURN CFNMETA((-(0.00/2))/CFMark(0))
 CASE index=113
  RETURN CFNMETA(-((-(0.00/2)))/CFMark(0))
 CASE index=114
  RETURN CFNMETA(-(((-(0.00/2)))/CFMark(0)))
 CASE index=115
  RETURN CFNMETA(CFMark(0)/(-(0.00/2)))
 CASE index=116
  RETURN CFNMETA(-(0.00/2)/CFMark(0))
 CASE index=117
  RETURN CFNMETA(IIF(.T.,(-(0.00/2)),0)/CFMark(0))
 CASE index=118
  RETURN CFNMETA(CFObserve((-(0.00/2)),14))
 CASE index=119
  RETURN CFNMETA(-(CFObserve((-(0.00/2)),14)))
 CASE index=120
  RETURN CFNMETA(CFObserve(-((-(0.00/2))),14))
 CASE index=121
  RETURN CFNMETA((-(0.00/2))/CFMark(14))
 CASE index=122
  RETURN CFNMETA(-((-(0.00/2)))/CFMark(14))
 CASE index=123
  RETURN CFNMETA(-(((-(0.00/2)))/CFMark(14)))
 CASE index=124
  RETURN CFNMETA(CFMark(14)/(-(0.00/2)))
 CASE index=125
  RETURN CFNMETA(-(0.00/2)/CFMark(14))
 CASE index=126
  RETURN CFNMETA(IIF(.T.,(-(0.00/2)),0)/CFMark(14))
 CASE index=127
  RETURN CFNMETA(CFObserve(((0.00/2)/2),0))
 CASE index=128
  RETURN CFNMETA(-(CFObserve(((0.00/2)/2),0)))
 CASE index=129
  RETURN CFNMETA(CFObserve(-(((0.00/2)/2)),0))
 CASE index=130
  RETURN CFNMETA(((0.00/2)/2)/CFMark(0))
 CASE index=131
  RETURN CFNMETA(-(((0.00/2)/2))/CFMark(0))
 CASE index=132
  RETURN CFNMETA(-((((0.00/2)/2))/CFMark(0)))
 CASE index=133
  RETURN CFNMETA(CFMark(0)/((0.00/2)/2))
 CASE index=134
  RETURN CFNMETA((0.00/2)/2/CFMark(0))
 CASE index=135
  RETURN CFNMETA(IIF(.T.,((0.00/2)/2),0)/CFMark(0))
 CASE index=136
  RETURN CFNMETA(CFObserve(((0.00/2)/2),14))
 CASE index=137
  RETURN CFNMETA(-(CFObserve(((0.00/2)/2),14)))
 CASE index=138
  RETURN CFNMETA(CFObserve(-(((0.00/2)/2)),14))
 CASE index=139
  RETURN CFNMETA(((0.00/2)/2)/CFMark(14))
 CASE index=140
  RETURN CFNMETA(-(((0.00/2)/2))/CFMark(14))
 CASE index=141
  RETURN CFNMETA(-((((0.00/2)/2))/CFMark(14)))
 CASE index=142
  RETURN CFNMETA(CFMark(14)/((0.00/2)/2))
 CASE index=143
  RETURN CFNMETA((0.00/2)/2/CFMark(14))
 CASE index=144
  RETURN CFNMETA(IIF(.T.,((0.00/2)/2),0)/CFMark(14))
 CASE index=145
  RETURN CFNMETA((5/2)/(5/2))
 CASE index=146
  RETURN CFNMETA((1/7)/3)
 CASE index=147
  RETURN CFNMETA(5/(5/2))
 ENDCASE
 RETURN ''

FUNCTION CFMark
 LPARAMETERS nextSetting
 gCalls=gCalls+1
 SET DECIMALS TO nextSetting
 RETURN 2.00

FUNCTION CFObserve
 LPARAMETERS observedValue,nextSetting
 gCalls=gCalls+1
 gSeen=CFNMETA(observedValue)
 SET DECIMALS TO nextSetting
 RETURN observedValue

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,index,byteValue
 result=''
 FOR index=1 TO LEN(value)
  byteValue=ASC(SUBSTR(value,index,1))
  result=result+SUBSTR('0123456789ABCDEF',INT(byteValue/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(byteValue,16)+1,1)
 ENDFOR
 RETURN result
