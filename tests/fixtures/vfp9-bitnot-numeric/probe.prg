* Independent BITNOT first-operand conversion observation; no native binary inspection.
LOCAL lcOut, lcValues, lcValue, lcCommand, lcResult, lnCase, lnError, loError, vResult
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcValues = '0|1|-1|0.49|0.5|0.9|1.49|1.5|1.9|-0.49|-0.5|-0.9|-1.49|-1.5|-1.9|2.5|-2.5'
lcValues = lcValues + '|2147483647|2147483647.9|2147483648|-2147483648|-2147483648.9|-2147483649|4294967295|4294967296|4294967297|-4294967295|-4294967296|-4294967297'
lcValues = lcValues + '|9007199254740992|1E20|-1E20|1E300|-1E300|$0.5|$1.5|$2.5|-$1.5|.T.|.F.|"1.9"|"x"|.NULL.|{^2026-10-09}|CREATEOBJECT("Empty")'
FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
    lcValue = GETWORDNUM(lcValues, lnCase, '|')
    lcCommand = 'vResult = BITNOT(' + lcValue + ')'
    lnError = 0
    lcResult = '<not-returned>'
    TRY
        &lcCommand
        lcResult = IIF(ISNULL(vResult), '.NULL.', ALLTRIM(STR(vResult, 30, 0)))
    CATCH TO loError
        lnError = loError.ErrorNo
    ENDTRY
    lcOut = lcOut + lcValue + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
ENDFOR
FOR lnCase = 1 TO 2
    lcCommand = IIF(lnCase = 1, 'lcResult = TRANSFORM(BITNOT())', 'lcResult = TRANSFORM(BITNOT(1,2))')
    lnError = 0
    lcResult = '<not-returned>'
    TRY
        &lcCommand
    CATCH TO loError
        lnError = loError.ErrorNo
    ENDTRY
    lcOut = lcOut + lcCommand + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'probe.out')
RETURN
