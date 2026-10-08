* RQ-CF-PRG-ERROR-COMMAND-NUMERIC-001: installed VFP9 clean-room evidence.
LOCAL loError, lcStatus, lnCode, llAfter
? 'VERSION|'+VERSION()
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -1
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-1 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 0
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '0 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 0.49
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '0.49 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 0.5
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '0.5 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 0.9
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '0.9 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1.49
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1.49 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1.5
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1.5 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1.9
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1.9 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 11
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '11 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 11.5
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '11.5 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 12
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '12 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 99
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '99 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1098
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1098 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 2000
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '2000 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 2001
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '2001 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 9999
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '9999 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 65535
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '65535 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 2147483647
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '2147483647 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 2147483648
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '2147483648 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -2147483648
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-2147483648 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 4294967295
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '4294967295 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 4294967296
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '4294967296 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 4294967297
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '4294967297 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -4294967295
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-4294967295 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 9007199254740992
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '9007199254740992 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1E20
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1E20 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -1E20
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-1E20 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1E300
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1E300 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -1E300
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-1E300 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1E300*1E300
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1E300*1E300 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -1E300*1E300
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-1E300*1E300 bare|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -1, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-1 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 0, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '0 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 0.49, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '0.49 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 0.5, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '0.5 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 0.9, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '0.9 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1.49, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1.49 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1.5, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1.5 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1.9, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1.9 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 11, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '11 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 11.5, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '11.5 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 12, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '12 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 99, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '99 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1098, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1098 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 2000, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '2000 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 2001, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '2001 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 9999, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '9999 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 65535, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '65535 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 2147483647, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '2147483647 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 2147483648, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '2147483648 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -2147483648, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-2147483648 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 4294967295, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '4294967295 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 4294967296, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '4294967296 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 4294967297, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '4294967297 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -4294967295, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-4294967295 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 9007199254740992, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '9007199254740992 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1E20, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1E20 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -1E20, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-1E20 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1E300, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1E300 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -1E300, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-1E300 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR 1E300*1E300, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '1E300*1E300 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
lcStatus='OK'
lnCode=-99
llAfter=.F.
TRY
    ERROR -1E300*1E300, 'guard'
    llAfter=.T.
CATCH TO loError
    lcStatus='ERR'
    lnCode=loError.ErrorNo
ENDTRY
? '-1E300*1E300 parameter|'+lcStatus+'|'+TRANSFORM(lnCode)+'|'+TRANSFORM(llAfter)+'|'+TRANSFORM(SET('DATASESSION'))
? 'CLEANUP|'+TRANSFORM(SET('DATASESSION'))
