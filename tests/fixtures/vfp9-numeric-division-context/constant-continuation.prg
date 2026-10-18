* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-MATERIALIZATION-001: ungrouped prefix timing.
LOCAL ARRAY expressions[4],settings[5]
expressions[1]='5/2/CFMark(0)'
expressions[2]='5/2/CFMark(14)'
expressions[3]='(10/2)/2/CFMark(0)'
expressions[4]='(10/2)/2/CFMark(14)'
settings[1]=0
settings[2]=2
settings[3]=6
settings[4]=14
settings[5]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,modeNo,mode
PRIVATE gCalls,gSeen
outputPath='Z:\home\rich\.codex\worktrees\numeric-constant-division-5611\Project-Copperfin\build\constant-division-native\continuation-results.txt'
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
  RETURN CFNMETA(5/2/CFMark(0))
 CASE index=2
  RETURN CFNMETA(5/2/CFMark(14))
 CASE index=3
  RETURN CFNMETA((10/2)/2/CFMark(0))
 CASE index=4
  RETURN CFNMETA((10/2)/2/CFMark(14))
 ENDCASE
 RETURN ''

FUNCTION CFMark
 LPARAMETERS nextSetting
 gCalls=gCalls+1
 SET DECIMALS TO nextSetting
 RETURN 2.00

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,index,byteValue
 result=''
 FOR index=1 TO LEN(value)
  byteValue=ASC(SUBSTR(value,index,1))
  result=result+SUBSTR('0123456789ABCDEF',INT(byteValue/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(byteValue,16)+1,1)
 ENDFOR
 RETURN result
