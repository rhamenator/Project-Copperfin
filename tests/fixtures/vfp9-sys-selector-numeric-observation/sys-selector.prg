LOCAL cOut, i, cExpr, x, y, oEx, cTag, cFive, cSeven
LOCAL ARRAY aExpr[24]
SET SAFETY OFF
cFive = SYS(5)
cSeven = SYS(7)
aExpr[1] = '0'
aExpr[2] = '5'
aExpr[3] = '5.49'
aExpr[4] = '5.5'
aExpr[5] = '5.9'
aExpr[6] = '7.9'
aExpr[7] = '-0.9'
aExpr[8] = '-1'
aExpr[9] = '2147483647'
aExpr[10] = '2147483648'
aExpr[11] = '-2147483648'
aExpr[12] = '-2147483649'
aExpr[13] = '4294967295'
aExpr[14] = '4294967296'
aExpr[15] = '4294967301'
aExpr[16] = '4294967303'
aExpr[17] = '-4294967291'
aExpr[18] = '-4294967289'
aExpr[19] = '9007199254740992'
aExpr[20] = '9223372036854775808.0'
aExpr[21] = '1E300'
aExpr[22] = '-1E300'
aExpr[23] = 'EXP(1000)'
aExpr[24] = '-EXP(1000)'
cOut = 'VERSION|' + VERSION() + CHR(10) + 'BASE7-LENGTH|' + ALLTRIM(STR(LEN(cSeven))) + CHR(10)
=STRTOFILE(cOut, 'sys-selector.out', 0)
FOR i = 1 TO ALEN(aExpr)
    cExpr = aExpr[i]
    ? cExpr
    TRY
        x = EVALUATE(cExpr)
        y = SYS(x)
        DO CASE
        CASE VARTYPE(y) != 'C'
            cTag = VARTYPE(y) + ':non-character'
        CASE y == cFive
            cTag = 'C:matches-5'
        CASE y == cSeven
            cTag = 'C:matches-7'
        CASE y == '0'
            cTag = 'C:zero-text'
        OTHERWISE
            cTag = 'C:other-length-' + ALLTRIM(STR(LEN(y)))
        ENDCASE
        cOut = cOut + cExpr + '|' + cTag + CHR(10)
    CATCH TO oEx
        cOut = cOut + cExpr + '|ERR' + ALLTRIM(STR(oEx.ErrorNo)) + CHR(10)
    ENDTRY
    =STRTOFILE(cOut, 'sys-selector.out', 0)
ENDFOR
=STRTOFILE(cOut, 'sys-selector.out', 0)
