* Independent installed BITTEST Numeric value and position observation.
* BITTEST outputs are observed fresh, not inferred from BITCLEAR or BITSET or Copperfin.
LOCAL lcOut, lcValues, lcValue, lcArgs, lcCommand, lcResult, lnCase, lnSlot, lnPosition, lnError, loError, vResult
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcValues = '0|1|-1|0.49|0.5|0.9|1.49|1.5|1.9|-0.49|-0.5|-0.9|-1.49|-1.5|-1.9|2.5|-2.5'
lcValues = lcValues + '|2147483647|2147483647.9|2147483648|-2147483648|-2147483648.9|-2147483649|4294967295|4294967296|4294967297|-4294967295|-4294967296|-4294967297'
lcValues = lcValues + '|9007199254740992|1E20|-1E20|1E300|-1E300|EXP(1000)|-EXP(1000)'
FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
    lcValue = GETWORDNUM(lcValues, lnCase, '|')
    FOR lnPosition = 0 TO 31
        lcArgs = lcValue + ',' + TRANSFORM(lnPosition)
        lcCommand = 'vResult = BITTEST(' + lcArgs + ')'
        lnError = 0
        lcResult = '<not-returned>'
        TRY
            &lcCommand
            lcResult = IIF(vResult, '.T.', '.F.')
        CATCH TO loError
            lnError = loError.ErrorNo
        ENDTRY
        lcOut = lcOut + 'VALUE|' + lcArgs + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
    ENDFOR
ENDFOR
lcValues = '-1.9|-1|-0.9|-0.5|0|0.49|0.5|0.9|1.49|1.5|1.9|2.5|30.9|31|31.9|32|32.1'
lcValues = lcValues + '|2147483647|2147483648|-2147483648|-2147483649|4294967295|4294967296|4294967297|4294967327|-4294967295|-4294967296|-4294967297|-4294967265'
lcValues = lcValues + '|9007199254740992|1E20|-1E20|1E300|-1E300|EXP(1000)|-EXP(1000)'
FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
    lcValue = GETWORDNUM(lcValues, lnCase, '|')
    FOR lnSlot = 1 TO 3
        lcArgs = IIF(lnSlot = 1, '0,', IIF(lnSlot = 2, '-1,', '1431655765,')) + lcValue
        lcCommand = 'vResult = BITTEST(' + lcArgs + ')'
        lnError = 0
        lcResult = '<not-returned>'
        TRY
            &lcCommand
            lcResult = IIF(vResult, '.T.', '.F.')
        CATCH TO loError
            lnError = loError.ErrorNo
        ENDTRY
        lcOut = lcOut + 'POSITION|' + lcArgs + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
    ENDFOR
ENDFOR
FOR lnPosition = 0 TO 31
    lcArgs = '1431655765,' + TRANSFORM(lnPosition)
    lcCommand = 'vResult = BITTEST(' + lcArgs + ')'
    lnError = 0
    lcResult = '<not-returned>'
    TRY
        &lcCommand
        lcResult = IIF(vResult, '.T.', '.F.')
    CATCH TO loError
        lnError = loError.ErrorNo
    ENDTRY
    lcOut = lcOut + 'BIT|' + lcArgs + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
ENDFOR
lcValues = '5,1|7.9,1.9|-2.9,0.9|0,31.9|0,32|0,1E300|-1,32|7,1E20|4294967297,4294967297|EXP(1000),EXP(1000)'
FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
    lcArgs = GETWORDNUM(lcValues, lnCase, '|')
    lcCommand = 'vResult = BITTEST(' + lcArgs + ')'
    lnError = 0
    lcResult = '<not-returned>'
    TRY
        &lcCommand
        lcResult = IIF(vResult, '.T.', '.F.')
    CATCH TO loError
        lnError = loError.ErrorNo
    ENDTRY
    lcOut = lcOut + 'MIXED|' + lcArgs + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'probe.out')
RETURN
