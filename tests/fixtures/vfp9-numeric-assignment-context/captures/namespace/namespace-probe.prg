* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-ASSIGNMENT-CONTEXT-001: type-aware assignment recovery.
LOCAL ARRAY expressions[24],settings[2]
expressions[1]='0'
expressions[2]='-0'
expressions[3]='1'
expressions[4]='-1'
expressions[5]='5'
expressions[6]='-5'
expressions[7]='2147483647'
expressions[8]='-2147483647'
expressions[9]='00005'
expressions[10]='00000000000000000001'
expressions[11]='0.0'
expressions[12]='-0.0'
expressions[13]='5.00'
expressions[14]='-5.00'
expressions[15]='0.125'
expressions[16]='-0.125'
expressions[17]='5.000000000000000000'
expressions[18]='2147483648'
expressions[19]='-2147483648'
expressions[20]='1E0'
expressions[21]='1E-14'
expressions[22]='1-1E-14'
expressions[23]='1-90*2^(-53)'
expressions[24]='5/2'
settings[1]=2
settings[2]=14
PRIVATE outputPath
outputPath='Z:\home\rich\.codex\worktrees\numeric-assignment-context-5611\Project-Copperfin\build\numeric-assignment-native\namespace-results.txt'
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
 m.assignedPublic=.F.
 DO CASE
 CASE mode=1
  answer=CFNMETA(EVALUATE(expression))+'|'+HexBytes(BINTOC(EVALUATE(expression),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE(expression)))
 CASE mode=3
  m.evaluatedLocal=EVALUATE(expression)
  answer=CFNMETA(m.evaluatedLocal)+'|'+HexBytes(BINTOC(m.evaluatedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.evaluatedLocal))
 OTHERWISE
  DO CASE
  CASE index=1
   DO CASE
   CASE mode=2
    answer=CFNMETA(0)+'|'+HexBytes(BINTOC(0,'BRS'))+'|'+HexBytes(TRANSFORM(0))
   CASE mode=4 OR mode=5
    m.assignedLocal=0
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=0
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=0
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=2
   DO CASE
   CASE mode=2
    answer=CFNMETA(-0)+'|'+HexBytes(BINTOC(-0,'BRS'))+'|'+HexBytes(TRANSFORM(-0))
   CASE mode=4 OR mode=5
    m.assignedLocal=-0
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=-0
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=-0
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=3
   DO CASE
   CASE mode=2
    answer=CFNMETA(1)+'|'+HexBytes(BINTOC(1,'BRS'))+'|'+HexBytes(TRANSFORM(1))
   CASE mode=4 OR mode=5
    m.assignedLocal=1
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=1
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=1
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=4
   DO CASE
   CASE mode=2
    answer=CFNMETA(-1)+'|'+HexBytes(BINTOC(-1,'BRS'))+'|'+HexBytes(TRANSFORM(-1))
   CASE mode=4 OR mode=5
    m.assignedLocal=-1
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=-1
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=-1
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=5
   DO CASE
   CASE mode=2
    answer=CFNMETA(5)+'|'+HexBytes(BINTOC(5,'BRS'))+'|'+HexBytes(TRANSFORM(5))
   CASE mode=4 OR mode=5
    m.assignedLocal=5
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=5
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=5
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=6
   DO CASE
   CASE mode=2
    answer=CFNMETA(-5)+'|'+HexBytes(BINTOC(-5,'BRS'))+'|'+HexBytes(TRANSFORM(-5))
   CASE mode=4 OR mode=5
    m.assignedLocal=-5
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=-5
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=-5
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=7
   DO CASE
   CASE mode=2
    answer=CFNMETA(2147483647)+'|'+HexBytes(BINTOC(2147483647,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647))
   CASE mode=4 OR mode=5
    m.assignedLocal=2147483647
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=2147483647
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=2147483647
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=8
   DO CASE
   CASE mode=2
    answer=CFNMETA(-2147483647)+'|'+HexBytes(BINTOC(-2147483647,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647))
   CASE mode=4 OR mode=5
    m.assignedLocal=-2147483647
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=-2147483647
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=-2147483647
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=9
   DO CASE
   CASE mode=2
    answer=CFNMETA(00005)+'|'+HexBytes(BINTOC(00005,'BRS'))+'|'+HexBytes(TRANSFORM(00005))
   CASE mode=4 OR mode=5
    m.assignedLocal=00005
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=00005
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=00005
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=10
   DO CASE
   CASE mode=2
    answer=CFNMETA(00000000000000000001)+'|'+HexBytes(BINTOC(00000000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(00000000000000000001))
   CASE mode=4 OR mode=5
    m.assignedLocal=00000000000000000001
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=00000000000000000001
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=00000000000000000001
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=11
   DO CASE
   CASE mode=2
    answer=CFNMETA(0.0)+'|'+HexBytes(BINTOC(0.0,'BRS'))+'|'+HexBytes(TRANSFORM(0.0))
   CASE mode=4 OR mode=5
    m.assignedLocal=0.0
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=0.0
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=0.0
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=12
   DO CASE
   CASE mode=2
    answer=CFNMETA(-0.0)+'|'+HexBytes(BINTOC(-0.0,'BRS'))+'|'+HexBytes(TRANSFORM(-0.0))
   CASE mode=4 OR mode=5
    m.assignedLocal=-0.0
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=-0.0
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=-0.0
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=13
   DO CASE
   CASE mode=2
    answer=CFNMETA(5.00)+'|'+HexBytes(BINTOC(5.00,'BRS'))+'|'+HexBytes(TRANSFORM(5.00))
   CASE mode=4 OR mode=5
    m.assignedLocal=5.00
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=5.00
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=5.00
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=14
   DO CASE
   CASE mode=2
    answer=CFNMETA(-5.00)+'|'+HexBytes(BINTOC(-5.00,'BRS'))+'|'+HexBytes(TRANSFORM(-5.00))
   CASE mode=4 OR mode=5
    m.assignedLocal=-5.00
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=-5.00
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=-5.00
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=15
   DO CASE
   CASE mode=2
    answer=CFNMETA(0.125)+'|'+HexBytes(BINTOC(0.125,'BRS'))+'|'+HexBytes(TRANSFORM(0.125))
   CASE mode=4 OR mode=5
    m.assignedLocal=0.125
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=0.125
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=0.125
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=16
   DO CASE
   CASE mode=2
    answer=CFNMETA(-0.125)+'|'+HexBytes(BINTOC(-0.125,'BRS'))+'|'+HexBytes(TRANSFORM(-0.125))
   CASE mode=4 OR mode=5
    m.assignedLocal=-0.125
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=-0.125
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=-0.125
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=17
   DO CASE
   CASE mode=2
    answer=CFNMETA(5.000000000000000000)+'|'+HexBytes(BINTOC(5.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(5.000000000000000000))
   CASE mode=4 OR mode=5
    m.assignedLocal=5.000000000000000000
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=5.000000000000000000
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=5.000000000000000000
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=18
   DO CASE
   CASE mode=2
    answer=CFNMETA(2147483648)+'|'+HexBytes(BINTOC(2147483648,'BRS'))+'|'+HexBytes(TRANSFORM(2147483648))
   CASE mode=4 OR mode=5
    m.assignedLocal=2147483648
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=2147483648
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=2147483648
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=19
   DO CASE
   CASE mode=2
    answer=CFNMETA(-2147483648)+'|'+HexBytes(BINTOC(-2147483648,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483648))
   CASE mode=4 OR mode=5
    m.assignedLocal=-2147483648
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=-2147483648
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=-2147483648
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=20
   DO CASE
   CASE mode=2
    answer=CFNMETA(1E0)+'|'+HexBytes(BINTOC(1E0,'BRS'))+'|'+HexBytes(TRANSFORM(1E0))
   CASE mode=4 OR mode=5
    m.assignedLocal=1E0
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=1E0
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=1E0
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=21
   DO CASE
   CASE mode=2
    answer=CFNMETA(1E-14)+'|'+HexBytes(BINTOC(1E-14,'BRS'))+'|'+HexBytes(TRANSFORM(1E-14))
   CASE mode=4 OR mode=5
    m.assignedLocal=1E-14
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=1E-14
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=1E-14
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=22
   DO CASE
   CASE mode=2
    answer=CFNMETA(1-1E-14)+'|'+HexBytes(BINTOC(1-1E-14,'BRS'))+'|'+HexBytes(TRANSFORM(1-1E-14))
   CASE mode=4 OR mode=5
    m.assignedLocal=1-1E-14
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=1-1E-14
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=1-1E-14
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=23
   DO CASE
   CASE mode=2
    answer=CFNMETA(1-90*2^(-53))+'|'+HexBytes(BINTOC(1-90*2^(-53),'BRS'))+'|'+HexBytes(TRANSFORM(1-90*2^(-53)))
   CASE mode=4 OR mode=5
    m.assignedLocal=1-90*2^(-53)
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=1-90*2^(-53)
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=1-90*2^(-53)
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
   ENDCASE
  CASE index=24
   DO CASE
   CASE mode=2
    answer=CFNMETA(5/2)+'|'+HexBytes(BINTOC(5/2,'BRS'))+'|'+HexBytes(TRANSFORM(5/2))
   CASE mode=4 OR mode=5
    m.assignedLocal=5/2
    IF mode=4
     answer=CFNMETA(m.assignedLocal)+'|'+HexBytes(BINTOC(m.assignedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedLocal))
    ELSE
     m.copiedLocal=m.assignedLocal
     answer=CFNMETA(m.copiedLocal)+'|'+HexBytes(BINTOC(m.copiedLocal,'BRS'))+'|'+HexBytes(TRANSFORM(m.copiedLocal))
    ENDIF
   CASE mode=6
    m.assignedPrivate=5/2
    answer=CFNMETA(m.assignedPrivate)+'|'+HexBytes(BINTOC(m.assignedPrivate,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPrivate))
   CASE mode=7
    m.assignedPublic=5/2
    answer=CFNMETA(m.assignedPublic)+'|'+HexBytes(BINTOC(m.assignedPublic,'BRS'))+'|'+HexBytes(TRANSFORM(m.assignedPublic))
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

