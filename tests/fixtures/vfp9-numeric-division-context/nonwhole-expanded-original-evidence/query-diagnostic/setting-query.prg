* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Diagnostic only: independently identify SET DECIMALS readback type.
LOCAL outputPath,libraryPath,decimalCount,valueType,observation
outputPath='Z:\home\rich\.codex\worktrees\numeric-nonwhole-division-5611\Project-Copperfin\build\nonwhole-expanded-original\setting-query-results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
SET LIBRARY TO (libraryPath) ADDITIVE
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
FOR decimalCount=0 TO 14 STEP 14
 SET DECIMALS TO decimalCount
 valueType=VARTYPE(SET('DECIMALS'))
 IF valueType='N'
  observation=CFNMETA(SET('DECIMALS'))
 ELSE
  observation='NOT-NUMERIC'
 ENDIF
 =STRTOFILE('QUERY|'+ALLTRIM(STR(decimalCount,4,0))+'|'+valueType+'|'+observation+'|'+ALLTRIM(STR(VAL(TRANSFORM(SET('DECIMALS'))),4,0))+CHR(13)+CHR(10),outputPath,1)
ENDFOR
=STRTOFILE('DONE|2'+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO
