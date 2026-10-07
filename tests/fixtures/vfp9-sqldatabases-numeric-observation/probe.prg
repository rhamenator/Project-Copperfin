LOCAL lcExpression, luResult, loError
? 'VERSION|' + VERSION()
lcExpression = "SQLTABLES(0, 'TABLE', 'native_control')"
TRY
    luResult = EVALUATE(lcExpression)
    ? 'CONTROL|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? 'CONTROL|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(0, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
    ? 'FIRST_ERROR|' + loError.Message
ENDTRY
lcExpression = "SQLDATABASES(1, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(2, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-1, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(0.49, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(0.5, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(0.9, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(1.49, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(1.5, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(1.9, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-0.49, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-0.5, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-0.9, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-1.1, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(2147483647, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(2147483648, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-2147483648, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-2147483649, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(4294967295, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(4294967296, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(4294967297, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(4294967298, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-4294967296, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-4294967295, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-4294967294, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(4294967296.9, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-4294967295.9, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(1E20, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-1E20, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(1E300, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-1E300, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(EXP(1000), 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-EXP(1000), 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(9007199254740992, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(32767, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(32768, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(65535, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(65536, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(65537, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-65535, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(-65536, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES($0.5, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES($1.5, 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(.T., 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(.F., 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES(.NULL., 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES('0', 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLDATABASES('1', 'native_databases')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'CURSOR|' + IIF(USED('native_databases'), 'T', 'F')
? 'SESSION|' + TRANSFORM(SET('DATASESSION'))
