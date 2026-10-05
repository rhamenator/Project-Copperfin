LOCAL cOut, i, cExpr, x, y, oEx, cTag, nDefault, nHost, nOem
LOCAL ARRAY aExpr[32]
SET SAFETY OFF
nDefault = CPCURRENT()
nHost = CPCURRENT(1)
nOem = CPCURRENT(2)
aExpr[1] = '0'
aExpr[2] = '1'
aExpr[3] = '2'
aExpr[4] = '3'
aExpr[5] = '-1'
aExpr[6] = '99'
aExpr[7] = '0.49'
aExpr[8] = '0.5'
aExpr[9] = '0.9'
aExpr[10] = '1.49'
aExpr[11] = '1.5'
aExpr[12] = '1.9'
aExpr[13] = '2.49'
aExpr[14] = '2.5'
aExpr[15] = '2.9'
aExpr[16] = '-0.9'
aExpr[17] = '2147483647'
aExpr[18] = '2147483648'
aExpr[19] = '-2147483648'
aExpr[20] = '-2147483649'
aExpr[21] = '4294967298'
aExpr[22] = '-4294967294'
aExpr[23] = '1E300'
aExpr[24] = '-1E300'
aExpr[25] = 'EXP(1000)'
aExpr[26] = '-EXP(1000)'
aExpr[27] = '$1.9'
aExpr[28] = '$2.9'
aExpr[29] = '.T.'
aExpr[30] = "'2'"
aExpr[31] = '.NULL.'
aExpr[32] = "''"
cOut = 'VERSION|' + VERSION() + CHR(10)
cOut = cOut + 'BASE|' + TRANSFORM(nDefault) + '|' + TRANSFORM(nHost) + '|' + TRANSFORM(nOem) + CHR(10)
=STRTOFILE(cOut, 'cpcurrent.out', 0)
FOR i = 1 TO ALEN(aExpr)
    cExpr = aExpr[i]
    ? cExpr
    TRY
        x = EVALUATE(cExpr)
        y = CPCURRENT(x)
        cTag = VARTYPE(y) + ':' + TRANSFORM(y)
        cOut = cOut + cExpr + '|' + cTag + CHR(10)
    CATCH TO oEx
        cOut = cOut + cExpr + '|ERR' + ALLTRIM(STR(oEx.ErrorNo)) + CHR(10)
    ENDTRY
    =STRTOFILE(cOut, 'cpcurrent.out', 0)
ENDFOR
