* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-UNARY-ZERO-CONTEXT-001: native cache-test expression oracle.
LOCAL outputPath,decimalCount,fixedNo,fixedMode,runtimeZero,constantZero,negativeZero,line
PRIVATE callCount
outputPath='Z:\home\rich\.codex\worktrees\numeric-unary-zero-context-5611\Project-Copperfin\build\numeric-unary-expression-native\cache-results.txt'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
FOR decimalCount=2 TO 14 STEP 12
 SET DECIMALS TO decimalCount
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  callCount=0
  runtimeZero=-(IIF(.T.,0.00,1.00))*LaterOne()
  constantZero=-(0.00)*LaterOne()
  negativeZero=-(-0.00)*LaterOne()
  line='CACHE|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+;
   HexBytes(BINTOC(runtimeZero,'BRS'))+'|'+HexBytes(BINTOC(constantZero,'BRS'))+'|'+;
   HexBytes(BINTOC(negativeZero,'BRS'))+'|'+ALLTRIM(STR(callCount,4,0))
  =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
 ENDFOR
ENDFOR
=STRTOFILE('DONE|4'+CHR(13)+CHR(10),outputPath,1)

FUNCTION LaterOne
 callCount=callCount+1
 RETURN 1.00
ENDFUNC

FUNCTION HexBytes
 LPARAMETERS source
 LOCAL result,index,byte,digits
 result=''
 digits='0123456789ABCDEF'
 FOR index=1 TO LEN(source)
  byte=ASC(SUBSTR(source,index,1))
  result=result+SUBSTR(digits,INT(byte/16)+1,1)+SUBSTR(digits,MOD(byte,16)+1,1)
 ENDFOR
 RETURN result
ENDFUNC
