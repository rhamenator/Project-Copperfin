LOCAL lcExpression, luResult, loError
? 'VERSION|' + VERSION()
lcExpression = "SQLTABLES(0, 'TABLE', 'native_control')"
TRY
    luResult = EVALUATE(lcExpression)
    ? 'CONTROL|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? 'CONTROL|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(0, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
    ? 'FIRST_ERROR|' + loError.Message
ENDTRY
lcExpression = "SQLFOREIGNKEYS(1, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(2, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-1, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(0.49, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(0.5, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(0.9, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(1.49, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(1.5, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(1.9, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-0.49, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-0.5, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-0.9, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-1.1, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(2147483647, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(2147483648, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-2147483648, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-2147483649, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(4294967295, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(4294967296, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(4294967297, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(4294967298, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-4294967296, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-4294967295, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-4294967294, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(4294967296.9, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-4294967295.9, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(1E20, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-1E20, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(1E300, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-1E300, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(EXP(1000), 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-EXP(1000), 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(9007199254740992, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(32767, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(32768, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(65535, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(65536, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(65537, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-65535, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(-65536, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS($0.5, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS($1.5, 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(.T., 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(.F., 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS(.NULL., 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS('0', 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
lcExpression = "SQLFOREIGNKEYS('1', 'ORD*', 'native_foreignkeys')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'CURSOR|' + IIF(USED('native_foreignkeys'), 'T', 'F')
? 'SESSION|' + TRANSFORM(SET('DATASESSION'))
