* Independent installed BITOR Numeric conversion observation; no binary inspection.
LOCAL lcOut, lcValues, lcValue, lcArgs, lcCommand, lcResult, lnCase, lnSlot, lnIndex, lnError, loError, vResult
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcValues = '0|1|-1|0.49|0.5|0.9|1.49|1.5|1.9|-0.49|-0.5|-0.9|-1.49|-1.5|-1.9|2.5|-2.5'
lcValues = lcValues + '|2147483647|2147483647.9|2147483648|-2147483648|-2147483648.9|-2147483649|4294967295|4294967296|4294967297|-4294967295|-4294967296|-4294967297'
lcValues = lcValues + '|9007199254740992|1E20|-1E20|1E300|-1E300|EXP(1000)|-EXP(1000)'
FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
    lcValue = GETWORDNUM(lcValues, lnCase, '|')
    FOR lnSlot = 1 TO 2
        lcArgs = IIF(lnSlot = 1, lcValue + ',0', '0,' + lcValue)
        lcCommand = 'vResult = BITOR(' + lcArgs + ')'
        lnError = 0
        lcResult = '<not-returned>'
        TRY
            &lcCommand
            lcResult = ALLTRIM(STR(vResult, 30, 0))
        CATCH TO loError
            lnError = loError.ErrorNo
        ENDTRY
        lcOut = lcOut + lcArgs + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
    ENDFOR
ENDFOR
FOR lnSlot = 1 TO 26
    lcArgs = ''
    FOR lnIndex = 1 TO 26
        lcArgs = lcArgs + IIF(lnIndex = 1, '', ',') + IIF(lnIndex = lnSlot, '1.9', '0')
    ENDFOR
    lcCommand = 'vResult = BITOR(' + lcArgs + ')'
    lnError = 0
    lcResult = '<not-returned>'
    TRY
        &lcCommand
        lcResult = ALLTRIM(STR(vResult, 30, 0))
    CATCH TO loError
        lnError = loError.ErrorNo
    ENDTRY
    lcOut = lcOut + 'SLOT=' + TRANSFORM(lnSlot) + '|ARGS=' + lcArgs + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
ENDFOR
lcValues = '3,6|7.9,3.9,1.9|-2.9,-3.9|-2147483648,0|-1,1E300|0,-1,1E300|1E300,-1|0,4294967297,3'
FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
    lcArgs = GETWORDNUM(lcValues, lnCase, '|')
    lcCommand = 'vResult = BITOR(' + lcArgs + ')'
    lnError = 0
    lcResult = '<not-returned>'
    TRY
        &lcCommand
        lcResult = ALLTRIM(STR(vResult, 30, 0))
    CATCH TO loError
        lnError = loError.ErrorNo
    ENDTRY
    lcOut = lcOut + lcArgs + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'probe.out')
RETURN
