LOCAL lcOut, lcExpr, luResult, loError, i, lcFont
LOCAL ARRAY aValues[47], aAction[47], aFonts[1]
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
aValues[29]='0'
aValues[30]='-1'
aValues[31]='-1E300'
aValues[32]='-EXP(1000)'
aValues[33]='0.5'
aValues[34]='0.9'
aValues[35]='4294967295.1'
aValues[36]='4294967295.5'
aValues[37]='4294967295.9'
aValues[38]='4294967296.1'
aValues[39]='4294967295.9999995'
aValues[40]='-0.5'
aValues[41]='-0.9'
aValues[42]='-2147483649'
aValues[43]='-4294967284'
aValues[44]='-4294967283.5'
aValues[45]='4294967295'
aValues[46]='2147483648'
aValues[47]='2147483648.5'
FOR i=1 TO 47
    aAction[i]=IIF(i<=6 OR BETWEEN(i,29,32) OR BETWEEN(i,40,41) OR BETWEEN(i,43,44),'Return','Escape')
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
