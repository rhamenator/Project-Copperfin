LOCAL lcOut, lcExpr, luResult, loError, i, lcFont
LOCAL ARRAY aValues[28], aAction[28], aFonts[1]
SET SAFETY OFF
lcOut = 'Z:\home\rich\.codex\worktrees\getfont-size-numeric-5611\Project-Copperfin\build\getfont-native\probe.out'
=STRTOFILE('VERSION|' + VERSION() + CHR(13)+CHR(10), lcOut, 0)
=AFONT(aFonts)
lcFont=aFonts[1]
FOR i=1 TO ALEN(aFonts)
    IF LEFT(aFonts[i],1)!='@'
        lcFont=aFonts[i]
        EXIT
    ENDIF
ENDFOR
=STRTOFILE('FONT|'+lcFont+CHR(13)+CHR(10),lcOut,1)
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
aValues[13]='2147483647'
aValues[14]='2147483647.49'
aValues[15]='2147483647.5'
aValues[16]='2147483647.9'
aValues[17]='2147483648'
aValues[18]='-2147483648'
aValues[19]='-2147483648.9'
aValues[20]='-2147483649'
aValues[21]='4294967295'
aValues[22]='4294967296'
aValues[23]='-4294967296'
aValues[24]='1E300'
aValues[25]='-1E300'
aValues[26]='-EXP(1000)'
aValues[27]='.T.'
aValues[28]='.NULL.'
FOR i=1 TO 28
    aAction[i]=IIF(i<=6,'Return','Escape')
    lcExpr="GETFONT('"+lcFont+"',"+aValues[i]+",'B')"
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
