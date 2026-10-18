* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Independent Character type/length check for a retained scientific control.
LOCAL outputPath,settingNo,fixedNo,decimalCount,fixedMode,line
outputPath='Z:\home\rich\.codex\worktrees\numeric-nonwhole-division-5611\Project-Copperfin\build\nonwhole-native\empty-display-results.txt'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
FOR settingNo=1 TO 2
 IF settingNo=1
  decimalCount=0
 ELSE
  decimalCount=18
 ENDIF
 SET DECIMALS TO decimalCount
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  line=ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+VARTYPE(TRANSFORM(1E-14/2))+'|'+ALLTRIM(STR(LEN(TRANSFORM(1E-14/2)),6,0))+'|['+TRANSFORM(1E-14/2)+']'
  =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
 ENDFOR
ENDFOR
=STRTOFILE('DONE|4'+CHR(13)+CHR(10),outputPath,1)
