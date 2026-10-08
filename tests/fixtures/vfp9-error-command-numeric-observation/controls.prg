* RQ-CF-PRG-ERROR-COMMAND-NUMERIC-001: parameter ordering and Currency controls.
PUBLIC nParameterCalls
LOCAL loError, nCode, lAfter
? 'VERSION|'+VERSION()
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR -1, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '-1|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 0, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '0|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 0.49, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '0.49|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 0.5, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '0.5|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 0.9, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '0.9|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 1, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '1|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 1.49, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '1.49|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 1.5, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '1.5|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 1.9, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '1.9|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 11, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '11|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 11.5, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '11.5|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 12, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '12|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 99, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '99|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 1098, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '1098|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 2000, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '2000|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 2001, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '2001|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 9999, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '9999|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 65535, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '65535|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 2147483647, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '2147483647|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 2147483648, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '2147483648|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR -2147483648, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '-2147483648|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 4294967295, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '4294967295|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 4294967296, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '4294967296|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 4294967297, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '4294967297|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR -4294967295, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '-4294967295|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 9007199254740992, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '9007199254740992|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 1E20, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '1E20|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR -1E20, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '-1E20|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 1E300, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '1E300|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR -1E300, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '-1E300|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR 1E300*1E300, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '1E300*1E300|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR -1E300*1E300, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '-1E300*1E300|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR $11.5, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '$11.5|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR $0.5, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '$0.5|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR $922337203685477.5807, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '$922337203685477.5807|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nParameterCalls=0
nCode=-99
lAfter=.F.
TRY
    ERROR -$11.5, ParameterText()
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? '-$11.5|'+TRANSFORM(nCode)+'|'+TRANSFORM(nParameterCalls)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
? 'CLEANUP|'+TRANSFORM(SET('DATASESSION'))
RETURN
FUNCTION ParameterText
nParameterCalls=nParameterCalls+1
RETURN 'guard'
ENDFUNC
