* RQ-CF-PRG-SLEEP-DURATION-NUMERIC-001: safe command-presence recovery only.
LOCAL loError, nCode, lAfter, nControl
? 'VERSION|'+VERSION()
nControl=INT(1.9)
? 'CONTROL|INT|'+TRANSFORM(nControl)+'|'+TRANSFORM(SET('DATASESSION'))
nCode=0
lAfter=.F.
TRY
 =EXECSCRIPT('SLEEP 0')
 lAfter=.T.
CATCH TO loError
 nCode=loError.ErrorNo
ENDTRY
? 'SLEEP0|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
nCode=0
lAfter=.F.
TRY
 =EXECSCRIPT('SLEEP 0.0')
 lAfter=.T.
CATCH TO loError
 nCode=loError.ErrorNo
ENDTRY
? 'SLEEPZERO|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(SET('DATASESSION'))
? 'CLEANUP|'+TRANSFORM(SET('DATASESSION'))
RETURN
