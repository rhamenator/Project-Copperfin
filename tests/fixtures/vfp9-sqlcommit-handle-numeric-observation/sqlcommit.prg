LOCAL lcExpression, luResult, loError
? 'VERSION|' + VERSION()
lcExpression = "SQLCOMMIT(0)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(2)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(1.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(1.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(1.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-1.1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(2147483647)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-2147483649)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(4294967295)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(4294967296)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(4294967297)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(4294967298)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-4294967296)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-4294967295)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-4294967294)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(4294967296.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-4294967295.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(1E20)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-1E20)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(9007199254740992)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(32767)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(32768)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(65535)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(65536)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(65537)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-65535)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(-65536)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT($0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT($1.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(.T.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(.F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT(.NULL.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT('0')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLCOMMIT('1')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'SESSION|' + TRANSFORM(SET('DATASESSION'))
