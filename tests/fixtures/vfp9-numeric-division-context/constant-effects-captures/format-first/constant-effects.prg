* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DIVISION-CONTEXT-001: single-evaluation side-effect timing controls.
LOCAL ARRAY expressions[16], settings[5]
expressions[1]='CFObserve(5/2,14)'
expressions[2]='-(CFObserve(5/2,14))'
expressions[3]='CFObserve(-(5/2),14)'
expressions[4]='CFObserve(5.00/2.00,14)'
expressions[5]='CFObserve(5/2,0)'
expressions[6]='-(CFObserve(5/2,0))'
expressions[7]='(5/2)/CFMark(14)'
expressions[8]='-(5/2)/CFMark(14)'
expressions[9]='-((5/2)/CFMark(14))'
expressions[10]='CFMark(14)/(5/2)'
expressions[11]='(5/2)/CFMark(0)'
expressions[12]='-(5/2)/CFMark(0)'
expressions[13]='-((5/2)/CFMark(0))'
expressions[14]='CFMark(0)/(5/2)'
expressions[15]='CFObserve(10/2,14)'
expressions[16]='CFObserve(0.00/2,14)'
settings[1]=0
settings[2]=2
settings[3]=6
settings[4]=14
settings[5]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,modeNo,mode
PRIVATE gCalls,gSeen
outputPath='Z:\home\rich\.codex\worktrees\numeric-constant-division-5611\Project-Copperfin\build\constant-division-native\effects-results.txt'
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
     line=mode+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation+'|'+HexBytes(gSeen)+'|'+ALLTRIM(STR(gCalls,6,0))+'|'+ALLTRIM(SET('DECIMALS'))
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
  RETURN CFNMETA(CFObserve(5/2,14))
 CASE index=2
  RETURN CFNMETA(-(CFObserve(5/2,14)))
 CASE index=3
  RETURN CFNMETA(CFObserve(-(5/2),14))
 CASE index=4
  RETURN CFNMETA(CFObserve(5.00/2.00,14))
 CASE index=5
  RETURN CFNMETA(CFObserve(5/2,0))
 CASE index=6
  RETURN CFNMETA(-(CFObserve(5/2,0)))
 CASE index=7
  RETURN CFNMETA((5/2)/CFMark(14))
 CASE index=8
  RETURN CFNMETA(-(5/2)/CFMark(14))
 CASE index=9
  RETURN CFNMETA(-((5/2)/CFMark(14)))
 CASE index=10
  RETURN CFNMETA(CFMark(14)/(5/2))
 CASE index=11
  RETURN CFNMETA((5/2)/CFMark(0))
 CASE index=12
  RETURN CFNMETA(-(5/2)/CFMark(0))
 CASE index=13
  RETURN CFNMETA(-((5/2)/CFMark(0)))
 CASE index=14
  RETURN CFNMETA(CFMark(0)/(5/2))
 CASE index=15
  RETURN CFNMETA(CFObserve(10/2,14))
 CASE index=16
  RETURN CFNMETA(CFObserve(0.00/2,14))
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
