LOCAL lcExpression, luResult, loError
LOCAL ARRAY aProbe[1]
? "VERSION|" + VERSION()
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 0)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 2)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 3)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 1.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 1.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 1.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 2.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 2.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 2.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 3.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 2147483647)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -2147483649)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 4294967296)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 4294967297)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 4294967298)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 4294967299)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -4294967296)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -4294967295)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -4294967294)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -4294967293)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -1.1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 4294967298.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -4294967294.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 3.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 4)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 4.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 8)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 15)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 31)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 32)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', 4294967300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "ADIR(aProbe, 'MiXeD.txt', '', -4294967292)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + "|" + VARTYPE(luResult) + ":" + TRANSFORM(luResult) + "|[" + aProbe[1, 1] + "]"
CATCH TO loError
    ? lcExpression + "|ERR" + TRANSFORM(loError.ErrorNo)
ENDTRY
