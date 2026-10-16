* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-REAL-ASSIGNMENT-001: bounded behavioral recovery.
LOCAL ARRAY expressions[48],settings[2]
expressions[1]='1.0'
expressions[2]='-1.0'
expressions[3]='1.00'
expressions[4]='-1.00'
expressions[5]='1.000'
expressions[6]='-1.000'
expressions[7]='1.0000'
expressions[8]='-1.0000'
expressions[9]='1.00000'
expressions[10]='-1.00000'
expressions[11]='1.000000'
expressions[12]='-1.000000'
expressions[13]='1.0000000'
expressions[14]='-1.0000000'
expressions[15]='1.00000000'
expressions[16]='-1.00000000'
expressions[17]='1.000000000'
expressions[18]='-1.000000000'
expressions[19]='1.0000000000'
expressions[20]='-1.0000000000'
expressions[21]='1.00000000000'
expressions[22]='-1.00000000000'
expressions[23]='1.000000000000'
expressions[24]='-1.000000000000'
expressions[25]='1.0000000000000'
expressions[26]='-1.0000000000000'
expressions[27]='1.00000000000000'
expressions[28]='-1.00000000000000'
expressions[29]='1.000000000000000'
expressions[30]='-1.000000000000000'
expressions[31]='1.0000000000000000'
expressions[32]='-1.0000000000000000'
expressions[33]='1.00000000000000000'
expressions[34]='-1.00000000000000000'
expressions[35]='1.000000000000000000'
expressions[36]='-1.000000000000000000'
expressions[37]='0.0'
expressions[38]='-0.0'
expressions[39]='0.000000000000000000'
expressions[40]='-0.000000000000000000'
expressions[41]='0.000000000000000001'
expressions[42]='-0.000000000000000001'
expressions[43]='9999999999.0'
expressions[44]='-9999999999.0'
expressions[45]='000000000000000000000000000000000005.00'
expressions[46]='-000000000000000000000000000000000005.00'
expressions[47]='9999999999.999999999'
expressions[48]='-9999999999.999999999'
settings[1]=2
settings[2]=14
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,mode,expression,line,caught
outputPath='Z:\home\rich\.codex\worktrees\numeric-real-assignment-5611\Project-Copperfin\build\numeric-real-assignment-native\results.txt'
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
=STRTOFILE('DONE|1344'+CHR(13)+CHR(10),outputPath,1)
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
    answer=CFNMETA(1.0)+'|'+HexBytes(BINTOC(1.0,'BRS'))+'|'+HexBytes(TRANSFORM(1.0))
   CASE mode=4 OR mode=5
    assignedLocal=1.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=2
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.0)+'|'+HexBytes(BINTOC(-1.0,'BRS'))+'|'+HexBytes(TRANSFORM(-1.0))
   CASE mode=4 OR mode=5
    assignedLocal=-1.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=3
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.00)+'|'+HexBytes(BINTOC(1.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.00))
   CASE mode=4 OR mode=5
    assignedLocal=1.00
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.00
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.00
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=4
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.00)+'|'+HexBytes(BINTOC(-1.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00))
   CASE mode=4 OR mode=5
    assignedLocal=-1.00
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.00
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.00
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=5
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.000)+'|'+HexBytes(BINTOC(1.000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000))
   CASE mode=4 OR mode=5
    assignedLocal=1.000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=6
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.000)+'|'+HexBytes(BINTOC(-1.000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=7
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.0000)+'|'+HexBytes(BINTOC(1.0000,'BRS'))+'|'+HexBytes(TRANSFORM(1.0000))
   CASE mode=4 OR mode=5
    assignedLocal=1.0000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.0000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.0000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=8
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.0000)+'|'+HexBytes(BINTOC(-1.0000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.0000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.0000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.0000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.0000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=9
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.00000)+'|'+HexBytes(BINTOC(1.00000,'BRS'))+'|'+HexBytes(TRANSFORM(1.00000))
   CASE mode=4 OR mode=5
    assignedLocal=1.00000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.00000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.00000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=10
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.00000)+'|'+HexBytes(BINTOC(-1.00000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.00000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.00000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.00000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=11
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.000000)+'|'+HexBytes(BINTOC(1.000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=12
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.000000)+'|'+HexBytes(BINTOC(-1.000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=13
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.0000000)+'|'+HexBytes(BINTOC(1.0000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.0000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.0000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.0000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.0000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=14
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.0000000)+'|'+HexBytes(BINTOC(-1.0000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.0000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.0000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.0000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.0000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=15
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.00000000)+'|'+HexBytes(BINTOC(1.00000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.00000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.00000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.00000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.00000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=16
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.00000000)+'|'+HexBytes(BINTOC(-1.00000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.00000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.00000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.00000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=17
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.000000000)+'|'+HexBytes(BINTOC(1.000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=18
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.000000000)+'|'+HexBytes(BINTOC(-1.000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=19
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.0000000000)+'|'+HexBytes(BINTOC(1.0000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.0000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.0000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.0000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.0000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=20
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.0000000000)+'|'+HexBytes(BINTOC(-1.0000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.0000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.0000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.0000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.0000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=21
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.00000000000)+'|'+HexBytes(BINTOC(1.00000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.00000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.00000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.00000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.00000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=22
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.00000000000)+'|'+HexBytes(BINTOC(-1.00000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.00000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.00000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.00000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=23
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.000000000000)+'|'+HexBytes(BINTOC(1.000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=24
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.000000000000)+'|'+HexBytes(BINTOC(-1.000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=25
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.0000000000000)+'|'+HexBytes(BINTOC(1.0000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.0000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.0000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.0000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.0000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=26
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.0000000000000)+'|'+HexBytes(BINTOC(-1.0000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.0000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.0000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.0000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.0000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=27
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.00000000000000)+'|'+HexBytes(BINTOC(1.00000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.00000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.00000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.00000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.00000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=28
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.00000000000000)+'|'+HexBytes(BINTOC(-1.00000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.00000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.00000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.00000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=29
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.000000000000000)+'|'+HexBytes(BINTOC(1.000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=30
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.000000000000000)+'|'+HexBytes(BINTOC(-1.000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=31
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.0000000000000000)+'|'+HexBytes(BINTOC(1.0000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.0000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.0000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.0000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.0000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=32
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.0000000000000000)+'|'+HexBytes(BINTOC(-1.0000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.0000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.0000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.0000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.0000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=33
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.00000000000000000)+'|'+HexBytes(BINTOC(1.00000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.00000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.00000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.00000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.00000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=34
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.00000000000000000)+'|'+HexBytes(BINTOC(-1.00000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.00000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.00000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.00000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=35
   DO CASE
   CASE mode=2
    answer=CFNMETA(1.000000000000000000)+'|'+HexBytes(BINTOC(1.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=1.000000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=1.000000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=1.000000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=36
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1.000000000000000000)+'|'+HexBytes(BINTOC(-1.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=-1.000000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-1.000000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-1.000000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=37
   DO CASE
   CASE mode=2
    answer=CFNMETA(0.0)+'|'+HexBytes(BINTOC(0.0,'BRS'))+'|'+HexBytes(TRANSFORM(0.0))
   CASE mode=4 OR mode=5
    assignedLocal=0.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=0.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=0.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=38
   DO CASE
   CASE mode=2
    answer=CFNMETA(-0.0)+'|'+HexBytes(BINTOC(-0.0,'BRS'))+'|'+HexBytes(TRANSFORM(-0.0))
   CASE mode=4 OR mode=5
    assignedLocal=-0.0
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-0.0
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-0.0
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=39
   DO CASE
   CASE mode=2
    answer=CFNMETA(0.000000000000000000)+'|'+HexBytes(BINTOC(0.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000000000000000))
   CASE mode=4 OR mode=5
    assignedLocal=0.000000000000000000
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=0.000000000000000000
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=0.000000000000000000
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=40
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
  CASE index=41
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
  CASE index=42
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
  CASE index=43
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
  CASE index=44
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
  CASE index=45
   DO CASE
   CASE mode=2
    answer=CFNMETA(000000000000000000000000000000000005.00)+'|'+HexBytes(BINTOC(000000000000000000000000000000000005.00,'BRS'))+'|'+HexBytes(TRANSFORM(000000000000000000000000000000000005.00))
   CASE mode=4 OR mode=5
    assignedLocal=000000000000000000000000000000000005.00
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=000000000000000000000000000000000005.00
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=000000000000000000000000000000000005.00
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=46
   DO CASE
   CASE mode=2
    answer=CFNMETA(-000000000000000000000000000000000005.00)+'|'+HexBytes(BINTOC(-000000000000000000000000000000000005.00,'BRS'))+'|'+HexBytes(TRANSFORM(-000000000000000000000000000000000005.00))
   CASE mode=4 OR mode=5
    assignedLocal=-000000000000000000000000000000000005.00
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-000000000000000000000000000000000005.00
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-000000000000000000000000000000000005.00
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=47
   DO CASE
   CASE mode=2
    answer=CFNMETA(9999999999.999999999)+'|'+HexBytes(BINTOC(9999999999.999999999,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.999999999))
   CASE mode=4 OR mode=5
    assignedLocal=9999999999.999999999
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=9999999999.999999999
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=9999999999.999999999
    answer=CFNMETA(assignedPublic)+'|'+HexBytes(BINTOC(assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPublic))
   ENDCASE
  CASE index=48
   DO CASE
   CASE mode=2
    answer=CFNMETA(-9999999999.999999999)+'|'+HexBytes(BINTOC(-9999999999.999999999,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.999999999))
   CASE mode=4 OR mode=5
    assignedLocal=-9999999999.999999999
    IF mode=4
     answer=CFNMETA(assignedLocal)+'|'+HexBytes(BINTOC(assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(assignedLocal))
    ELSE
     copiedLocal=assignedLocal
     answer=CFNMETA(copiedLocal)+'|'+HexBytes(BINTOC(copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(copiedLocal))
    ENDIF
   CASE mode=6
    assignedPrivate=-9999999999.999999999
    answer=CFNMETA(assignedPrivate)+'|'+HexBytes(BINTOC(assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(assignedPrivate))
   CASE mode=7
    assignedPublic=-9999999999.999999999
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
