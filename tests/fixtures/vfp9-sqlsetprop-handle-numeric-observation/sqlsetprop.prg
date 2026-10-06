LOCAL lcExpression, luResult, loError
? 'VERSION|' + VERSION()
? 'DEFAULT-BEFORE|' + VARTYPE(SQLGETPROP(0, 'Asynchronous')) + ':[' + TRANSFORM(SQLGETPROP(0, 'Asynchronous')) + ']'
lcExpression = "SQLSETPROP(0, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(1, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(2, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-1, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(0.49, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(0.5, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(0.9, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(1.49, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(1.5, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(1.9, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-0.49, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-0.5, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-0.9, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-1.1, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(2147483647, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(2147483648, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-2147483648, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-2147483649, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(4294967295, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(4294967296, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(4294967297, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(4294967298, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-4294967296, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-4294967295, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-4294967294, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(4294967296.9, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-4294967295.9, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(1E20, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-1E20, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(1E300, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-1E300, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(EXP(1000), 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-EXP(1000), 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(9007199254740992, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(32767, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(32768, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(65535, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(65536, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(65537, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-65535, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(-65536, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP($0.5, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP($1.5, 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(.T., 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(.F., 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP(.NULL., 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP('0', 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLSETPROP('1', 'Asynchronous', .F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'DEFAULT-AFTER|' + VARTYPE(SQLGETPROP(0, 'Asynchronous')) + ':[' + TRANSFORM(SQLGETPROP(0, 'Asynchronous')) + ']'
? 'SESSION|' + TRANSFORM(SET('DATASESSION'))
