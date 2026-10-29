* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: ONE binding call, NO CRT conversions.
LOCAL outputPath,libraryPath,crtPath,payload,beforeMeta,beforeBits,beforeDisplay,record
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-crt-binding-diagnostic\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
crtPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-crt-binding-diagnostic\crt-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
SET LIBRARY TO (crtPath) ADDITIVE
SET DECIMALS TO 2
SET FIXED ON
payload=0.5
beforeMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))
beforeBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))
beforeDisplay=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
record=CFNBIND()
=STRTOFILE('BIND|'+record+'|'+beforeMeta+'|'+beforeBits+'|'+beforeDisplay+'|'+CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))+'|'+HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))+'|'+HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))+CHR(13)+CHR(10),outputPath,1)
* Completion means the one diagnostic returned, NOT successful identity.
=STRTOFILE('DONE|1'+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO
RETURN

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,position,byteValue
 result=''
 FOR position=1 TO LEN(value)
  byteValue=ASC(SUBSTR(value,position,1))
  result=result+SUBSTR('0123456789ABCDEF',INT(byteValue/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(byteValue,16)+1,1)
 ENDFOR
 RETURN result
