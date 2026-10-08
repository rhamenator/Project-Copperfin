* RQ-CF-PRG-SKPBAR-NUMERIC-001: independent singleton skip query controls.
* Exactly one RELATIVE bar, never activate or allocate by numeric operand.
LOCAL lcOut, lcValues, lcValue, lcCommand, lnSeed, lnCase, lnError, loError, lnId, lnDisabledError, llEnabled, llDisabled
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcValues = '0|0.9|1|1.4|1.5|1.9|2|2.9|-1|-0.9|-2|-2.9|-3|-100|-1000|-10000|-16383|-16384|-16385|-32766|-32767|-32768|-2147483647|-2147483648'
lcValues = lcValues + '|2147483646.9|2147483647|2147483647.9|2147483648|2147483649|4294967293|4294967294|4294967295|4294967296|4294967297|-4294967295|-4294967294|-4294967296|1E20|1E300|-1E300'
FOR lnSeed = 1 TO 3
    DEFINE POPUP cfparent RELATIVE
    DO CASE
    CASE lnSeed = 1
        DEFINE BAR 1 OF cfparent PROMPT 'seed'
        lnId = 1
    CASE lnSeed = 2
        DEFINE BAR 2 OF cfparent PROMPT 'seed'
        lnId = 2
    OTHERWISE
        DEFINE BAR 2147483647 OF cfparent PROMPT 'seed'
        lnId = 2147483647
    ENDCASE
    lcOut = lcOut + 'SEED=' + TRANSFORM(lnId) + CHR(10)
    FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
        lcValue = GETWORDNUM(lcValues, lnCase, '|')
        SET SKIP OF BAR (lnId) OF cfparent .F.
        lcCommand = "llEnabled = SKPBAR('cfparent', " + lcValue + ")"
        lnError = 0
        llEnabled = .F.
        TRY
            &lcCommand
        CATCH TO loError
            lnError = loError.ErrorNo
        ENDTRY
        SET SKIP OF BAR (lnId) OF cfparent .T.
        lcCommand = "llDisabled = SKPBAR('cfparent', " + lcValue + ")"
        lnDisabledError = 0
        llDisabled = .F.
        TRY
            &lcCommand
        CATCH TO loError
            lnDisabledError = loError.ErrorNo
        ENDTRY
        lcOut = lcOut + lcValue + '|ENABLEERR=' + TRANSFORM(lnError) + '|ENABLED=' + TRANSFORM(llEnabled) + '|DISABLEERR=' + TRANSFORM(lnDisabledError) + '|DISABLED=' + TRANSFORM(llDisabled) + '|COUNT=' + TRANSFORM(CNTBAR('cfparent')) + '|ID=' + TRANSFORM(GETBAR('cfparent', 1)) + CHR(10)
    ENDFOR
    RELEASE POPUP cfparent
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'probe.out')
RETURN
