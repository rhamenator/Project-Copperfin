LOCAL lcExpression, luResult, loError, lcFont
LOCAL ARRAY aFonts[1], aProbe[1]
? 'VERSION|' + VERSION()
=AFONT(aFonts)
lcFont = aFonts[1]
? 'FONT|' + lcFont
lcExpression = "AFONT(aProbe, lcFont, 0)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 1)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 8)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 12)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -1)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 0.49)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 0.5)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 0.9)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 1.49)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 1.5)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 1.9)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 12.49)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 12.5)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 12.9)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -0.49)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -0.5)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -0.9)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -1.1)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 2147483647)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 2147483648)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -2147483648)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -2147483649)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 4294967295)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 4294967296)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 4294967297)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 4294967308)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -4294967296)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -4294967295)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -4294967284)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 1E20)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -1E20)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 1E300)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -1E300)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, EXP(1000))"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -EXP(1000))"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 4294967308.9)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -4294967284.9)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 32767)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 32768)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 65535)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 65536)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -32768)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -65536)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -1.49)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -1.5)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -1.9)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -2)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -2.1)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 4294967295.9)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -4294967297)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -4294967297.9)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 9007199254740992)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, 9223372036854775808)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
lcExpression = "AFONT(aProbe, lcFont, -9223372036854775808)"
aProbe[1] = 'sentinel'
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALEN:' + TRANSFORM(ALEN(aProbe)) + '|FIRST:' + TRANSFORM(aProbe[1])
ENDTRY
