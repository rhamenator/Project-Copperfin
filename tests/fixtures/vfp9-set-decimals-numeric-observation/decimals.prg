LOCAL lcArgument, luValue, loError, lcOutcome
? 'VERSION|' + VERSION()
SET DECIMALS TO 5
lcArgument = "0"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "1"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "2"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "18"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "19"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-1"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "0.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "0.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "1.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "1.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "1.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "17.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "18.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "19.1"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-0.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-0.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-1.1"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "2147483647"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "2147483648"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-2147483648"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-2147483649"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "4294967295"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "4294967296"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "4294967297"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "4294967314"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-4294967296"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-4294967295"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-4294967278"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "4294967296.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-4294967295.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "1E20"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-1E20"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "1E300"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-1E300"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "9007199254740992"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-9007199254740992"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "9223372036854774784"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "9223372036854775808"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "-9223372036854775808"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "(1E300 * 1E300)"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "(-1E300 * 1E300)"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "$0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "$1.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = ".T."
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = ".F."
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = ".NULL."
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "'2'"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
lcArgument = "'abc'"
TRY
    luValue = EVALUATE(lcArgument)
    SET DECIMALS TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 5
TRY
    SET DECIMALS TO
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'OMITTED|' + lcOutcome + '|DECIMALS' + TRANSFORM(SET('DECIMALS'))
SET DECIMALS TO 2
? 'FINAL|' + TRANSFORM(SET('DECIMALS'))
