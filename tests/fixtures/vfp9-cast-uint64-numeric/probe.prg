* Installed VFP9 CAST controls and unsigned64 extension boundary; no binary inspection.
LOCAL lcOut, lcTypes, lcValues, lcType, lcValue, lcCommand, lcResult
LOCAL lnType, lnValue, lnError, loError, vResult
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcTypes = 'INTEGER|INT|I|UINT64|ULONGLONG|UBIGINT'
lcValues = '0|1.9|-0.9|-1.9|2147483647|-2147483648'
FOR lnType = 1 TO GETWORDCOUNT(lcTypes, '|')
    lcType = GETWORDNUM(lcTypes, lnType, '|')
    FOR lnValue = 1 TO GETWORDCOUNT(lcValues, '|')
        lcValue = GETWORDNUM(lcValues, lnValue, '|')
        lcCommand = 'vResult = CAST(' + lcValue + ' AS ' + lcType + ')'
        lnError = 0
        lcResult = '<not-returned>'
        TRY
            &lcCommand
            lcResult = ALLTRIM(STR(vResult, 30, 0))
        CATCH TO loError
            lnError = loError.ErrorNo
        ENDTRY
        lcOut = lcOut + lcValue + ' AS ' + lcType + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'probe.out')
QUIT
