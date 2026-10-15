* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
LOCAL outputPath,value,libraryPath
outputPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\cause\smoke-results.txt'
libraryPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\cause\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
=STRTOFILE('LOADED|'+TRANSFORM(CFNCW())+CHR(13)+CHR(10),outputPath,1)
value=0.9
=STRTOFILE('SMOKE|0.9|'+CFNMETA(value)+CHR(13)+CHR(10),outputPath,1)
value=1-1E-14
=STRTOFILE('SMOKE|1-1E-14|'+CFNMETA(value)+CHR(13)+CHR(10),outputPath,1)
value=1-90*2^(-53)
=STRTOFILE('SMOKE|1-90*2^(-53)|'+CFNMETA(value)+CHR(13)+CHR(10),outputPath,1)
=STRTOFILE('CLONE|'+CFNMETA(CFNCLONE(value))+CHR(13)+CHR(10),outputPath,1)
=STRTOFILE('DEC0|'+CFNMETA(CFNDEC(value,0))+CHR(13)+CHR(10),outputPath,1)
=STRTOFILE('DEC14|'+CFNMETA(CFNDEC(value,14))+CHR(13)+CHR(10),outputPath,1)
=STRTOFILE('WIDTH20|'+CFNMETA(CFNWIDTH(value,20))+CHR(13)+CHR(10),outputPath,1)
=STRTOFILE('DONE|7'+CHR(13)+CHR(10),outputPath,1)
RETURN
