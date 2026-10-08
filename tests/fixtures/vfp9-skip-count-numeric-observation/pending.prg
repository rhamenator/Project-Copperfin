* RQ-CF-PRG-SKIP-COUNT-NUMERIC-001: pending rows and explicit-IN count conversion.
LOCAL loError, nCode, lAfter
? 'VERSION|'+VERSION()
SET MULTILOCKS ON
SELECT 0
CREATE CURSOR pendguard (label C(8))
INSERT INTO pendguard VALUES ('one')
INSERT INTO pendguard VALUES ('two')
INSERT INTO pendguard VALUES ('three')
=CURSORSETPROP('Buffering',4,'pendguard')
APPEND BLANK
REPLACE label WITH 'pending1'
APPEND BLANK
REPLACE label WITH 'pending2'
SELECT 0
CREATE CURSOR observer (label C(8))
INSERT INTO observer VALUES ('watch')
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-0.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-0.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|0]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|2.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2147483648) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967295) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967296) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967297) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967295) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967297) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1E300) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP ($1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|$1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
=TABLEREVERT(.T.,'pendguard')
USE IN pendguard
USE IN observer
? 'CLEAN4|'+TRANSFORM(USED('pendguard'))+'|'+TRANSFORM(USED('observer'))+'|'+TRANSFORM(SET('DATASESSION'))
SELECT 0
CREATE CURSOR pendguard (label C(8))
INSERT INTO pendguard VALUES ('one')
INSERT INTO pendguard VALUES ('two')
INSERT INTO pendguard VALUES ('three')
=CURSORSETPROP('Buffering',5,'pendguard')
APPEND BLANK
REPLACE label WITH 'pending1'
APPEND BLANK
REPLACE label WITH 'pending2'
SELECT 0
CREATE CURSOR observer (label C(8))
INSERT INTO observer VALUES ('watch')
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-0.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-0.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|0]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|0.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (0.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|2.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (2147483648) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|2147483648]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967295) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967296) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|4294967296]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (4294967297) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967295) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (-4294967297) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP (1E300) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
SELECT pendguard
GO -2
SELECT observer
nCode=0
lAfter=.F.
TRY
    SKIP ($1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|$1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+ALIAS()+'|'+TRANSFORM(SET('DATASESSION'))
=TABLEREVERT(.T.,'pendguard')
USE IN pendguard
USE IN observer
? 'CLEAN5|'+TRANSFORM(USED('pendguard'))+'|'+TRANSFORM(USED('observer'))+'|'+TRANSFORM(SET('DATASESSION'))
SET MULTILOCKS OFF
? 'CLEANUP|'+SET('MULTILOCKS')+'|'+TRANSFORM(SET('DATASESSION'))
