LOCAL lcCommand, loError, lcOutcome
? 'VERSION|' + VERSION()
lcCommand = "SET EPOCH TO 1950"
TRY
    EXECSCRIPT(lcCommand)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcCommand + '|' + lcOutcome
lcCommand = "SET EPOCH 1950"
TRY
    EXECSCRIPT(lcCommand)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcCommand + '|' + lcOutcome
lcCommand = "SET EPOCH TO"
TRY
    EXECSCRIPT(lcCommand)
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? lcCommand + '|' + lcOutcome
lcCommand = "SET EPOCH TO 1950"
TRY
    &lcCommand
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'MACRO:' + lcCommand + '|' + lcOutcome
lcCommand = "SET EPOCH 1950"
TRY
    &lcCommand
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'MACRO:' + lcCommand + '|' + lcOutcome
lcCommand = "SET EPOCH TO"
TRY
    &lcCommand
    lcOutcome = 'OK'
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'MACRO:' + lcCommand + '|' + lcOutcome
TRY
    lcOutcome = 'OK:' + TRANSFORM(SET('EPOCH'))
CATCH TO loError
    lcOutcome = 'ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
? "SET('EPOCH')|" + lcOutcome
SET CENTURY TO 19 ROLLOVER 50
? "CENTURY|" + TRANSFORM(SET('CENTURY', 1)) + '|' + TRANSFORM(SET('CENTURY', 2))
SET DATE TO MDY
SET CENTURY OFF
? 'CONSUMER|' + TRANSFORM(YEAR(CTOD('01/01/49'))) + '|' + TRANSFORM(YEAR(CTOD('01/01/50')))
