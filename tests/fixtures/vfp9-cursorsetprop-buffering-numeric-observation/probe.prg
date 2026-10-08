* Clean-room installed VFP9 observation: temporary cursors only.
* #5611/#6776 numeric BUFFERING-mode boundary; no table files or network.
? 'VERSION|' + VERSION()
SET MULTILOCKS ON
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',0,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '0|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',1,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '1|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',2,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '2|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',3,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '3|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',5,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '5|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',6,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '6|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',-1,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '-1|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',0.49,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '0.49|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',0.5,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '0.5|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',0.9,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '0.9|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',1.49,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '1.49|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',1.5,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '1.5|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',1.9,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '1.9|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',2.5,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '2.5|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',3.5,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '3.5|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4.5,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4.5|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4.9,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4.9|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',5.9,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '5.9|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',-0.5,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '-0.5|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',-1.5,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '-1.5|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',2147483647,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '2147483647|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',2147483648,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '2147483648|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',-2147483648,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '-2147483648|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4294967295,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4294967295|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4294967296,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4294967296|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4294967297,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4294967297|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4294967298,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4294967298|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4294967299,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4294967299|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4294967300,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4294967300|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',4294967301,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '4294967301|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',-4294967295,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '-4294967295|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',9007199254740992,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '9007199254740992|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',1E20,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '1E20|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',-1E20,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '-1E20|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',1E300,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '1E300|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',-1E300,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '-1E300|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',1E300*1E300,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '1E300*1E300|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',-1E300*1E300,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '-1E300*1E300|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',.NULL.,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '.NULL.|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',.T.,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '.T.|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING','3','flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? "'3'|"+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
CREATE CURSOR flagguard (label C(8))
INSERT INTO flagguard VALUES ('guard')
GO TOP
LOCAL x, cStatus
x=-99
cStatus='OK'
TRY
    x=CURSORSETPROP('BUFFERING',$3.5,'flagguard')
CATCH TO oError
    cStatus='ERR'+ALLTRIM(STR(oError.ErrorNo))
ENDTRY
? '$3.5|'+cStatus+'|'+TRANSFORM(x)+'|'+TRANSFORM(CURSORGETPROP('BUFFERING','flagguard'))+'|'+TRANSFORM(RECNO('flagguard'))+':'+TRANSFORM(RECCOUNT('flagguard'))+':'+ALLTRIM(flagguard.label)+'|'+TRANSFORM(SET('DATASESSION'))
USE IN flagguard
SET MULTILOCKS OFF
? 'CLEANUP|'+TRANSFORM(USED('flagguard'))+'|'+SET('MULTILOCKS')+'|'+TRANSFORM(SET('DATASESSION'))
