* RQ-CF-PRG-SKIP-COUNT-NUMERIC-001: selected count conversion recovery.
LOCAL loError, nCode, lAfter
? 'VERSION|'+VERSION()
CREATE CURSOR skipguard (label C(8))
INSERT INTO skipguard VALUES ('one')
INSERT INTO skipguard VALUES ('two')
INSERT INTO skipguard VALUES ('three')
SELECT 0
CREATE CURSOR observer (label C(8))
INSERT INTO observer VALUES ('watch')
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-1.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-1.49)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-1.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-0.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-0.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-0.49)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-0.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (0)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|0]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (0.49)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|0.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (0.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (0.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (1)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|1]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (1.49)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|1.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (1.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (2.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|2.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (2.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|2.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (2147483646.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|2147483646.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (2147483647)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|2147483647]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (2147483647.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|2147483647.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (2147483648)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (2147483648.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|2147483648.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (2147483649)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|2147483649]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (4294967295)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (4294967296)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (4294967297)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (4294967298)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|4294967298]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-2147483648)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-2147483648.9)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-2147483648.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-2147483649)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-2147483649]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967295)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967296)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967297)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967298)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-4294967298]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (9223372036854775808)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|9223372036854775808]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-9223372036854775808)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-9223372036854775808]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (1E100)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|1E100]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-1E100)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-1E100]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP ($1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|$1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP ($-1.5)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|$-1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP ('1.5')
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|'1.5']+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (.T.)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|.T.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (.F.)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|.F.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (.NULL.)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|.NULL.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (1E300*1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-1E300*1E300)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (9223372036854774784)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|9223372036854774784]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
nCode=0
lAfter=.F.
TRY
    SKIP (-9223372036854774784)
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIP|-9223372036854774784]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1.9) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1.5) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1.49) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-1.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-0.9) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-0.5) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-0.49) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-0.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|0]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0.49) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|0.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0.5) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0.9) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|1]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1.49) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|1.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1.5) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1.9) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2.5) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|2.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2.9) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|2.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2147483646.9) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|2147483646.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2147483647) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|2147483647]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2147483647.9) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|2147483647.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2147483648) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2147483648.9) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|2147483648.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2147483649) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|2147483649]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967295) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967296) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967297) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967298) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|4294967298]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-2147483648) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-2147483648.9) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-2147483648.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-2147483649) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-2147483649]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967295) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967296) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967297) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967298) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-4294967298]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (9223372036854775808) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|9223372036854775808]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-9223372036854775808) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-9223372036854775808]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1E100) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|1E100]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1E100) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-1E100]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1E300) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1E300) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP ($1.5) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|$1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP ($-1.5) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|$-1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP ('1.5') IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|'1.5']+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (.T.) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|.T.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (.F.) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|.F.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (.NULL.) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|.NULL.]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1E300*1E300) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1E300*1E300) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (9223372036854774784) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|9223372036854774784]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT skipguard
GO 2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-9223372036854774784) IN skipguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [SKIPIN|-9223372036854774784]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('skipguard'))+'|'+TRANSFORM(BOF('skipguard'))+'|'+TRANSFORM(EOF('skipguard'))+'|'+TRANSFORM(RECCOUNT('skipguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
USE IN skipguard
USE IN observer
? 'CLEANUP|'+TRANSFORM(USED('skipguard'))+'|'+TRANSFORM(USED('observer'))+'|'+TRANSFORM(SET('DATASESSION'))
