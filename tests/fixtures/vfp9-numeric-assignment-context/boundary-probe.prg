* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-ASSIGNMENT-CONTEXT-001: type-aware assignment recovery.
LOCAL ARRAY expressions[24],settings[2]
expressions[1]='-999999999'
expressions[2]='-1000000000'
expressions[3]='999999999'
expressions[4]='1000000000'
expressions[5]='-2147483646'
expressions[6]='-0000000001'
expressions[7]='-0000000000'
expressions[8]='-(-2147483647)'
expressions[9]='-(-00005)'
expressions[10]='999999999.0'
expressions[11]='-999999999.0'
expressions[12]='1000000000.00'
expressions[13]='-1000000000.00'
expressions[14]='9999999999.0'
expressions[15]='-9999999999.0'
expressions[16]='10000000000.0'
expressions[17]='-10000000000.0'
expressions[18]='0.000000000000000001'
expressions[19]='-0.000000000000000001'
expressions[20]='-0.000000000000000000'
expressions[21]='999999999.999999999'
expressions[22]='-999999999.999999999'
expressions[23]='123456789012345.00'
expressions[24]='-123456789012345.00'
settings[1]=2
settings[2]=14
PRIVATE outputPath
outputPath='Z:\home\rich\.codex\worktrees\numeric-assignment-context-5611\Project-Copperfin\build\numeric-assignment-native\boundary-results.txt'
LOCAL libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,mode,expression,line,caught
libraryPath='Z:\home\rich\.local\share\Trash\files\listitem-column-numeric-5611-build\listitem-column-native\cause\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
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
  FOR index=1 TO ALEN(expressions)
   expression=expressions[index]
   FOR mode=1 TO 7
    TRY
     line='VALUE|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+ALLTRIM(STR(mode,2,0))+'|'+HexBytes(expression)+'|'+Capture(index,mode,expression)
    CATCH TO caught
     line='ERROR|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+ALLTRIM(STR(mode,2,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
    ENDTRY
    =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|672'+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION Capture
 LPARAMETERS index,mode,expression
 LOCAL assignedLocal,copiedLocal,evaluatedLocal,answer
 PRIVATE assignedPrivate
 PUBLIC assignedPublic
 assignedPublic=.F.
 DO CASE
 CASE mode=1
  answer=CFNMETA(EVALUATE(expression))+'|'+HexBytes(BINTOC(EVALUATE(expression),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE(expression)))
 CASE mode=3
  evaluatedLocal=EVALUATE(expression)
  answer=CFNMETA(evaluatedLocal)+'|'+HexBytes(BINTOC(evaluatedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(evaluatedLocal))
 OTHERWISE
  DO CASE
  CASE index=1
   DO CASE
   CASE mode=2
    answer=CFNMETA(-999999999)+'|'+HexBytes(BINTOC(-999999999,'BRS'))+'|'+HexBytes(TRANSFORM(-999999999))
   CASE mode=4 OR mode=5
    assignedLocal=-999999999
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-999999999
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-999999999
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=2
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1000000000)+'|'+HexBytes(BINTOC(-1000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=3
   DO CASE
   CASE mode=2
    answer=CFNMETA(999999999)+'|'+HexBytes(BINTOC(999999999,'BRS'))+'|'+HexBytes(TRANSFORM(999999999))
   CASE mode=4 OR mode=5
    assignedLocal=999999999
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=999999999
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=999999999
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=4
   DO CASE
   CASE mode=2
    answer=CFNMETA(1000000000)+'|'+HexBytes(BINTOC(1000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=5
   DO CASE
   CASE mode=2
    answer=CFNMETA(-2147483646)+'|'+HexBytes(BINTOC(-2147483646,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646))
   CASE mode=4 OR mode=5
    assignedLocal=-2147483646
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-2147483646
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-2147483646
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=6
   DO CASE
   CASE mode=2
    answer=CFNMETA(-0000000001)+'|'+HexBytes(BINTOC(-0000000001,'BRS'))+'|'+HexBytes(TRANSFORM(-0000000001))
   CASE mode=4 OR mode=5
    assignedLocal=-0000000001
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-0000000001
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-0000000001
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=7
   DO CASE
   CASE mode=2
    answer=CFNMETA(-0000000000)+'|'+HexBytes(BINTOC(-0000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-0000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-0000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-0000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-0000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=8
   DO CASE
   CASE mode=2
    answer=CFNMETA(-(-2147483647))+'|'+HexBytes(BINTOC(-(-2147483647),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647)))
   CASE mode=4 OR mode=5
    assignedLocal=-(-2147483647)
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-(-2147483647)
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-(-2147483647)
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=9
   DO CASE
   CASE mode=2
    answer=CFNMETA(-(-00005))+'|'+HexBytes(BINTOC(-(-00005),'BRS'))+'|'+HexBytes(TRANSFORM(-(-00005)))
   CASE mode=4 OR mode=5
    assignedLocal=-(-00005)
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-(-00005)
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-(-00005)
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=10
   DO CASE
   CASE mode=2
    answer=CFNMETA(999999999.0)+'|'+HexBytes(BINTOC(999999999.0,'BRS'))+'|'+HexBytes(TRANSFORM(999999999.0))
   CASE mode=4 OR mode=5
    assignedLocal=999999999.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=999999999.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=999999999.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=11
   DO CASE
   CASE mode=2
    answer=CFNMETA(-999999999.0)+'|'+HexBytes(BINTOC(-999999999.0,'BRS'))+'|'+HexBytes(TRANSFORM(-999999999.0))
   CASE mode=4 OR mode=5
    assignedLocal=-999999999.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-999999999.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-999999999.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=12
   DO CASE
   CASE mode=2
    answer=CFNMETA(1000000000.00)+'|'+HexBytes(BINTOC(1000000000.00,'BRS'))+'|'+HexBytes(TRANSFORM(1000000000.00))
   CASE mode=4 OR mode=5
    assignedLocal=1000000000.00
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1000000000.00
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1000000000.00
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=13
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1000000000.00)+'|'+HexBytes(BINTOC(-1000000000.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1000000000.00))
   CASE mode=4 OR mode=5
    assignedLocal=-1000000000.00
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1000000000.00
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1000000000.00
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=14
   DO CASE
   CASE mode=2
    answer=CFNMETA(9999999999.0)+'|'+HexBytes(BINTOC(9999999999.0,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.0))
   CASE mode=4 OR mode=5
    assignedLocal=9999999999.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=9999999999.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=9999999999.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=15
   DO CASE
   CASE mode=2
    answer=CFNMETA(-9999999999.0)+'|'+HexBytes(BINTOC(-9999999999.0,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.0))
   CASE mode=4 OR mode=5
    assignedLocal=-9999999999.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-9999999999.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-9999999999.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=16
   DO CASE
   CASE mode=2
    answer=CFNMETA(10000000000.0)+'|'+HexBytes(BINTOC(10000000000.0,'BRS'))+'|'+HexBytes(TRANSFORM(10000000000.0))
   CASE mode=4 OR mode=5
    assignedLocal=10000000000.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=10000000000.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=10000000000.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=17
   DO CASE
   CASE mode=2
    answer=CFNMETA(-10000000000.0)+'|'+HexBytes(BINTOC(-10000000000.0,'BRS'))+'|'+HexBytes(TRANSFORM(-10000000000.0))
   CASE mode=4 OR mode=5
    assignedLocal=-10000000000.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-10000000000.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-10000000000.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=18
   DO CASE
   CASE mode=2
    answer=CFNMETA(0.000000000000000001)+'|'+HexBytes(BINTOC(0.000000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000000000000001))
   CASE mode=4 OR mode=5
    assignedLocal=0.000000000000000001
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=0.000000000000000001
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=0.000000000000000001
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=19
   DO CASE
   CASE mode=2
    answer=CFNMETA(-0.000000000000000001)+'|'+HexBytes(BINTOC(-0.000000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(-0.000000000000000001))
   CASE mode=4 OR mode=5
    assignedLocal=-0.000000000000000001
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-0.000000000000000001
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-0.000000000000000001
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=20
   DO CASE
   CASE mode=2
    answer=CFNMETA(-0.000000000000000000)+'|'+HexBytes(BINTOC(-0.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-0.000000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-0.000000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-0.000000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-0.000000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=21
   DO CASE
   CASE mode=2
    answer=CFNMETA(999999999.999999999)+'|'+HexBytes(BINTOC(999999999.999999999,'BRS'))+'|'+HexBytes(TRANSFORM(999999999.999999999))
   CASE mode=4 OR mode=5
    assignedLocal=999999999.999999999
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=999999999.999999999
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=999999999.999999999
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=22
   DO CASE
   CASE mode=2
    answer=CFNMETA(-999999999.999999999)+'|'+HexBytes(BINTOC(-999999999.999999999,'BRS'))+'|'+HexBytes(TRANSFORM(-999999999.999999999))
   CASE mode=4 OR mode=5
    assignedLocal=-999999999.999999999
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-999999999.999999999
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-999999999.999999999
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=23
   DO CASE
   CASE mode=2
    answer=CFNMETA(123456789012345.00)+'|'+HexBytes(BINTOC(123456789012345.00,'BRS'))+'|'+HexBytes(TRANSFORM(123456789012345.00))
   CASE mode=4 OR mode=5
    assignedLocal=123456789012345.00
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=123456789012345.00
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=123456789012345.00
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=24
   DO CASE
   CASE mode=2
    answer=CFNMETA(-123456789012345.00)+'|'+HexBytes(BINTOC(-123456789012345.00,'BRS'))+'|'+HexBytes(TRANSFORM(-123456789012345.00))
   CASE mode=4 OR mode=5
    assignedLocal=-123456789012345.00
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-123456789012345.00
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-123456789012345.00
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  ENDCASE
 ENDCASE
 RELEASE assignedPublic
 RETURN answer
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


