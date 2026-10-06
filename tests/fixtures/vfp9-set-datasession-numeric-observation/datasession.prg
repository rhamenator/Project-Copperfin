LOCAL lcArgument, luValue, loError, loSession, lcOutcome
? 'VERSION|' + VERSION()
loSession = CREATEOBJECT('Session')
? 'PRIVATE|' + TRANSFORM(loSession.DataSessionId)
SET DATASESSION TO 1
lcArgument = "0"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "1"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "2"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "3"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-1"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "0.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "0.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "1.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "1.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "1.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "2.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "2.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "2.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-0.49"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-0.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-1.1"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "2147483647"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "2147483648"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-2147483648"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-2147483649"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "4294967295"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "4294967296"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "4294967297"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "4294967298"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-4294967296"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-4294967295"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-4294967294"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "4294967296.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-4294967295.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-4294967294.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "1E20"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-1E20"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "1E300"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-1E300"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "EXP(1000)"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-EXP(1000)"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "9007199254740992"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "32767"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "32768"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "65535"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "65536"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "65537"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "65538"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-65535"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "-65534"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "$0.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "$1.5"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = ".T."
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = ".F."
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = ".NULL."
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "'1'"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
lcArgument = "'2'"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 2
lcArgument = "0"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'FROM2:' + lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 2
lcArgument = "0.9"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'FROM2:' + lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 2
lcArgument = "3"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'FROM2:' + lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 2
lcArgument = "1E300"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'FROM2:' + lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 2
lcArgument = "-EXP(1000)"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'FROM2:' + lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 2
lcArgument = "4294967297"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'FROM2:' + lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 2
lcArgument = "-4294967294"
TRY
    luValue = EVALUATE(lcArgument)
    SET DATASESSION TO (luValue)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'FROM2:' + lcArgument + '|' + lcOutcome + '|SESSION' + TRANSFORM(SET('DATASESSION'))
SET DATASESSION TO 1
loSession = .NULL.
? 'FINAL|' + TRANSFORM(SET('DATASESSION'))
