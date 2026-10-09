* GETBAR independent position/identity observation; never activate.
LOCAL lcOut, lcValues, lcValue, lcCommand, lnLayout, lnCase, lnError, loError, lnResult
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcValues = '0|0.5|0.9|1|1.4|1.5|1.9|2|2.9|3|3.9|4|4.9|5.9|-1|-0.9|-2|-2.9|-3|-100|-16384|-32768|-2147483647|-2147483648'
lcValues = lcValues + '|2147483646.9|2147483647|2147483647.9|2147483648|2147483649|4294967293|4294967294|4294967295|4294967296|4294967297|4294967298|4294967299|-4294967295|-4294967294|-4294967293|-4294967296|1E20|1E300|-1E300'
FOR lnLayout = 1 TO 3
    DEFINE POPUP cfparent RELATIVE
    IF lnLayout = 3
        DEFINE BAR 4 OF cfparent PROMPT 'apply'
    ENDIF
    IF lnLayout >= 2
        DEFINE BAR 13 OF cfparent PROMPT 'first'
        DEFINE BAR 47 OF cfparent PROMPT 'second'
    ENDIF
    IF lnLayout = 3
        DEFINE BAR 100 OF cfparent PROMPT 'finish'
    ENDIF
    DEFINE BAR 2147483647 OF cfparent PROMPT 'cap'
    lcOut = lcOut + 'LAYOUT=' + TRANSFORM(lnLayout) + CHR(10)
    FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
        lcValue = GETWORDNUM(lcValues, lnCase, '|')
        lcCommand = "lnResult = GETBAR('cfparent', " + lcValue + ")"
        lnError = 0
        lnResult = -999
        TRY
            &lcCommand
        CATCH TO loError
            lnError = loError.ErrorNo
        ENDTRY
        lcOut = lcOut + lcValue + '|ERR=' + TRANSFORM(lnError) + '|BAR=' + TRANSFORM(lnResult) + '|COUNT=' + TRANSFORM(CNTBAR('cfparent')) + '|FIRST=' + TRANSFORM(GETBAR('cfparent', 1)) + '|LAST=' + TRANSFORM(GETBAR('cfparent', CNTBAR('cfparent'))) + '|CONTROL=' + PRMBAR('cfparent', 2147483647) + CHR(10)
    ENDFOR
    RELEASE POPUP cfparent
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'probe.out')
RETURN
