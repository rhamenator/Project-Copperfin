LOCAL lcArgument, luValue, loError, lcOutcome
? 'VERSION|' + VERSION()
SET FWEEK TO 2
lcArgument = "0"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "1"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "2"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "3"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "4"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-1"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "0.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "0.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "1.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "1.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "1.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "2.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "3.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "4.1"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-0.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-0.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-1.1"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "2147483647"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "2147483648"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-2147483648"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-2147483649"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "4294967295"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "4294967296"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "4294967297"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "4294967299"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-4294967296"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-4294967295"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-4294967293"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "4294967296.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-4294967295.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "1E20"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-1E20"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "1E300"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-1E300"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "9007199254740992"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-9007199254740992"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "9223372036854774784"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "9223372036854775808"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "-9223372036854775808"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "(1E300 * 1E300)"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "(-1E300 * 1E300)"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "$0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "$1.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = ".T."
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = ".F."
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = ".NULL."
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "'2'"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO 2
lcArgument = "'abc'"
TRY
    luValue = EVALUATE(lcArgument)
    SET FWEEK TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|FWEEK' + TRANSFORM(SET('FWEEK'))
SET FWEEK TO
? 'OMITTED|' + ALLTRIM(TRANSFORM(SET('FWEEK')))
SET FWEEK TO 1
? 'WEEK2021|FWEEK1|' + TRANSFORM(WEEK({^2021-01-01}, 0, 1))
SET FWEEK TO 2
? 'WEEK2021|FWEEK2|' + TRANSFORM(WEEK({^2021-01-01}, 0, 1))
SET FWEEK TO 3
? 'WEEK2021|FWEEK3|' + TRANSFORM(WEEK({^2021-01-01}, 0, 1))
SET FWEEK TO 1
? 'FINAL|' + TRANSFORM(SET('FWEEK'))
