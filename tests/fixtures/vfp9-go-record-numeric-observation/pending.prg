* RQ-CF-PRG-GO-RECORD-NUMERIC-001: pending negative record identities.
LOCAL loError, nCode, lAfter, cOldMultilocks
? 'VERSION|'+VERSION()
cOldMultilocks=SET('MULTILOCKS')
SET MULTILOCKS ON
CREATE CURSOR pendguard (label C(8))
INSERT INTO pendguard VALUES ('one')
INSERT INTO pendguard VALUES ('two')
INSERT INTO pendguard VALUES ('three')
=CURSORSETPROP('Buffering',4,'pendguard')
APPEND BLANK
REPLACE label WITH 'pending1'
APPEND BLANK
REPLACE label WITH 'pending2'
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-1]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1.49) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-1.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-2) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-2]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-2.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-2.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-3) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-3]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-4294967297) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-4294967298) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-4294967298]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-4294967295) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (0) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|0]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (0.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (1E300) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1E300) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1E300*1E300) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [4|-1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
=TABLEREVERT(.T.,'pendguard')
USE IN pendguard
CREATE CURSOR pendguard (label C(8))
INSERT INTO pendguard VALUES ('one')
INSERT INTO pendguard VALUES ('two')
INSERT INTO pendguard VALUES ('three')
=CURSORSETPROP('Buffering',5,'pendguard')
APPEND BLANK
REPLACE label WITH 'pending1'
APPEND BLANK
REPLACE label WITH 'pending2'
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-1]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1.49) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-1.49]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-1.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-2) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-2]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-2.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-2.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-3) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-3]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-4294967297) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-4294967297]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-4294967298) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-4294967298]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-4294967295) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-4294967295]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (0) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|0]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (0.9) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|0.9]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (1.5) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|1.5]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (1E300) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1E300) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
GO -2
nCode=0
lAfter=.F.
TRY
    GO (-1E300*1E300) IN pendguard
    lAfter=.T.
CATCH TO loError
    nCode=loError.ErrorNo
ENDTRY
? [5|-1E300*1E300]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(RECNO('pendguard'))+'|'+ALLTRIM(pendguard.label)+'|'+TRANSFORM(BOF('pendguard'))+'|'+TRANSFORM(EOF('pendguard'))+'|'+TRANSFORM(RECCOUNT('pendguard'))+'|'+TRANSFORM(SET('DATASESSION'))
=TABLEREVERT(.T.,'pendguard')
USE IN pendguard
IF cOldMultilocks=='OFF'
    SET MULTILOCKS OFF
ENDIF
? 'CLEANUP|'+TRANSFORM(USED('pendguard'))+'|'+SET('MULTILOCKS')+'|'+TRANSFORM(SET('DATASESSION'))
