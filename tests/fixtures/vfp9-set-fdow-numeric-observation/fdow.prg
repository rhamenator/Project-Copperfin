LOCAL lcArgument, luValue, loError, lcOutcome
? 'VERSION|' + VERSION()
SET FDOW TO 5
lcArgument = "0"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "1"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "2"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "7"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "8"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-1"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "0.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "0.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "1.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "1.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "1.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "6.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "7.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "8.1"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-0.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-0.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-1.1"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "2147483647"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "2147483648"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-2147483648"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-2147483649"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "4294967295"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "4294967296"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "4294967297"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "4294967303"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-4294967296"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-4294967295"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-4294967289"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "4294967296.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-4294967295.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "1E20"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-1E20"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "1E300"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-1E300"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "9007199254740992"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-9007199254740992"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "9223372036854774784"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "9223372036854775808"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "-9223372036854775808"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "(1E300 * 1E300)"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "(-1E300 * 1E300)"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "$0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "$1.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = ".T."
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = ".F."
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = ".NULL."
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "'2'"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO 5
lcArgument = "'abc'"
TRY
    luValue = EVALUATE(lcArgument)
    SET FDOW TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FDOW' + TRANSFORM(SET('FDOW'))
SET FDOW TO
? 'OMITTED|FDOW' + ALLTRIM(TRANSFORM(SET('FDOW')))
SET FDOW TO 1
? 'FINAL|FDOW' + TRANSFORM(SET('FDOW'))
