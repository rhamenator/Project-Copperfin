LOCAL lcExpression, luResult, loError
LOCAL ARRAY aProbe[1]
? "VERSION|" + VERSION()
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 0)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 2)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 3)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 4)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 31)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 32)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 1.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 1.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 1.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 2.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 31.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 31.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 32.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 2147483647)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -2147483649)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 4294967297)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -4294967295)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 4294967327)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -4294967265)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -1.1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', 4294967328)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ALINES(aProbe, ' a ' + CHR(13) + ' b ', -4294967264)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
