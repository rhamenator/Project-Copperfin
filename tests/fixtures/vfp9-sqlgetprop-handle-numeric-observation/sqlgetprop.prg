LOCAL lcExpression, luResult, loError
? 'VERSION|' + VERSION()
lcExpression = "SQLGETPROP(0, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(1, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(2, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-1, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(0.49, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(0.5, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(0.9, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(1.49, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(1.5, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(1.9, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-0.49, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-0.5, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-0.9, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-1.1, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(2147483647, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(2147483648, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-2147483648, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-2147483649, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(4294967295, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(4294967296, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(4294967297, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(4294967298, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-4294967296, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-4294967295, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-4294967294, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(4294967296.9, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-4294967295.9, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(1E20, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-1E20, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(1E300, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-1E300, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(EXP(1000), 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-EXP(1000), 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(9007199254740992, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(32767, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(32768, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(65535, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(65536, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(65537, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-65535, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(-65536, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP($0.5, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP($1.5, 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(.T., 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(.F., 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP(.NULL., 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP('0', 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLGETPROP('1', 'Asynchronous')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'SESSION|' + TRANSFORM(SET('DATASESSION'))
