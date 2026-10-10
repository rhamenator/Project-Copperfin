LOCAL lcOut, lcExpr, luResult, loError, i
LOCAL ARRAY aValues[12], aAction[12]
lcOut = 'Z:\home\rich\.codex\worktrees\getfont-size-numeric-5611\Project-Copperfin\build\getfont-native\probe.out'
=STRTOFILE('VERSION|' + VERSION() + CHR(13)+CHR(10), lcOut, 0)
aValues[1]='12'
aValues[2]='12.49'
aValues[3]='12.5'
aValues[4]='12.9'
aValues[5]='4'
aValues[6]='127'
aValues[7]='0'
aValues[8]='-1'
aValues[9]='3'
aValues[10]='128'
aValues[11]='1E20'
aValues[12]='EXP(1000)'
FOR i=1 TO 12
    aAction[i]=IIF(i<=6,'Return','Escape')
    lcExpr="GETFONT('Arial',"+aValues[i]+",'B')"
    =STRTOFILE('BEGIN|'+lcExpr+'|'+aAction[i]+CHR(13)+CHR(10),lcOut,1)
    TRY
        luResult=EVALUATE(lcExpr)
        =STRTOFILE('RESULT|'+lcExpr+'|'+VARTYPE(luResult)+':'+TRANSFORM(luResult)+CHR(13)+CHR(10),lcOut,1)
    CATCH TO loError
        =STRTOFILE('RESULT|'+lcExpr+'|ERR'+TRANSFORM(loError.ErrorNo)+CHR(13)+CHR(10),lcOut,1)
    ENDTRY
ENDFOR
=STRTOFILE('DONE'+CHR(13)+CHR(10),lcOut,1)
QUIT
