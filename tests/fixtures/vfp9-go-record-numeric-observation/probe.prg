* RQ-CF-PRG-GO-RECORD-NUMERIC-001: numeric conversion versus record existence.
LOCAL loError, nCode, lAfter, nBeforeSession
? 'VERSION|'+VERSION()
CREATE CURSOR goguard (label C(8))
INSERT INTO goguard VALUES ('one')
INSERT INTO goguard VALUES ('two')
INSERT INTO goguard VALUES ('three')
nBeforeSession=SET('DATASESSION')
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-1)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-1]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-0.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-0.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (0)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|0]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (0.49)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|0.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (0.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (0.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (1)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|1]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (1.49)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|1.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (1.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (2)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|2]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (2.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|2.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (2.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|2.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (3)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|3]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (3.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|3.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (4)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|4]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (2147483647)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|2147483647]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (2147483648)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-2147483648)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (4294967295)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (4294967296)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (4294967297)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-4294967295)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-4294967296)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-4294967297)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (9007199254740992)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|9007199254740992]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (1E20)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|1E20]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-1E20)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-1E20]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (1E300*1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-1E300*1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO ($1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|$1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO ($0.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|$0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (-$1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|-$1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO ('2')
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|'2']+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (.T.)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|.T.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (.F.)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|.F.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GO (.NULL.)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GO|.NULL.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-1)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-1]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-0.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-0.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (0)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|0]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (0.49)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|0.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (0.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (0.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (1)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|1]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (1.49)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|1.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (1.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (2)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|2]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (2.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|2.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (2.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|2.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (3)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|3]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (3.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|3.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (4)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|4]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (2147483647)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|2147483647]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (2147483648)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-2147483648)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (4294967295)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (4294967296)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (4294967297)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-4294967295)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-4294967296)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-4294967297)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (9007199254740992)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|9007199254740992]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (1E20)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|1E20]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-1E20)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-1E20]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (1E300*1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-1E300*1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO ($1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|$1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO ($0.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|$0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (-$1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|-$1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO ('2')
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|'2']+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (.T.)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|.T.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (.F.)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|.F.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO 2
nCode=0
lAfter=.F.
TRY
    GOTO (.NULL.)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [GOTO|.NULL.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('goguard'))+'|'+TRANSFORM(BOF('goguard'))+'|'+TRANSFORM(EOF('goguard'))+'|'+TRANSFORM(RECCOUNT('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
USE IN goguard
? 'CLEANUP|'+TRANSFORM(USED('goguard'))+'|'+TRANSFORM(SET('DATASESSION'))
