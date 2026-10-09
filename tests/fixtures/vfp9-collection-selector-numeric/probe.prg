* Independent Collection numeric selector observation; fresh object per call.
LOCAL lcOut, lcValues, lcValue, lcCommand, lcMethod, lnMethod, lnCase, lnError, loError, loItems, lcResult, lcState, lnItem
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcValues = '0|0.5|0.9|1|1.4|1.5|1.9|2|2.5|2.9|3|3.5|3.9|4|-0.5|-0.9|-1|-1.5|-2'
lcValues = lcValues + '|2147483647|2147483648|4294967293|4294967294|4294967295|4294967296|4294967297|4294967298|4294967299|-4294967295|-4294967294|-4294967293|-4294967296|1E20|1E300|-1E300|$0.5|$1.5|$2.5'
FOR lnMethod = 1 TO 2
    lcMethod = IIF(lnMethod = 1, 'Item', 'Remove')
    FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
        loItems = CREATEOBJECT('Collection')
        loItems.Add('alpha', 'first')
        loItems.Add('beta', 'second')
        loItems.Add('gamma', 'third')
        lcValue = GETWORDNUM(lcValues, lnCase, '|')
        lcCommand = 'lcResult = TRANSFORM(loItems.' + lcMethod + '(' + lcValue + '))'
        lnError = 0
        lcResult = '<not-returned>'
        TRY
            &lcCommand
        CATCH TO loError
            lnError = loError.ErrorNo
        ENDTRY
        lcState = ''
        FOR lnItem = 1 TO loItems.Count
            lcState = lcState + '[' + loItems.Item(lnItem) + ']'
        ENDFOR
        lcOut = lcOut + lcMethod + '|' + lcValue + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + '|COUNT=' + TRANSFORM(loItems.Count) + '|STATE=' + lcState + CHR(10)
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'probe.out')
RETURN
