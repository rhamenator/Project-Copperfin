LOCAL lcExpression, luResult, loError
? "VERSION|" + VERSION()
lcExpression = "SET('TEXTMERGE', 0)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 2)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 3)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 4)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', -1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 1.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 1.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 1.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 2.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 3.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 3.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 2147483647)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', -2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', -2147483649)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 4294967297)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 4294967299)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', -4294967295)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', -4294967293)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', -1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', -EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 4.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 4.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 4.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 4294967300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', -4294967292)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SET('TEXTMERGE', 4294967296)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult)
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
