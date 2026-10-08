* RQ-CF-PRG-ON-SELECTION-BAR-NUMERIC-001: independent sparse recovery.
* One parent bar only; no activation/UI/ordinal allocation or handler execution.
LOCAL lcOut, lcValues, lcValue, lcCommand, lnSeed, lnCase, lnError, loError, lnKind
PUBLIC nNativeGuard
nNativeGuard = 0
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcValues = '0|0.9|1|1.4|1.5|1.9|2|2.9|-1|-0.9|-2|-2.9|-3|-100|-1000|-10000|-16383|-16384|-16385|-32766|-32767|-32768|-2147483647|-2147483648'
lcValues = lcValues + '|2147483646.9|2147483647|2147483647.9|2147483648|2147483649|4294967293|4294967294|4294967295|4294967296|4294967297|-4294967295|-4294967294|-4294967296|1E20|1E300|-1E300'
FOR lnKind = 1 TO 2
    lcOut = lcOut + 'KIND=' + IIF(lnKind=1, 'DO', 'ACTION') + CHR(10)
    FOR lnSeed = 1 TO 3
        DEFINE POPUP cfparent RELATIVE
        DO CASE
        CASE lnSeed = 1
            DEFINE BAR 1 OF cfparent PROMPT 'seed'
        CASE lnSeed = 2
            DEFINE BAR 2 OF cfparent PROMPT 'seed'
        OTHERWISE
            DEFINE BAR 2147483647 OF cfparent PROMPT 'seed'
        ENDCASE
        lcOut = lcOut + 'SEED=' + TRANSFORM(GETBAR('cfparent', 1)) + CHR(10)
        FOR lnCase = 1 TO GETWORDCOUNT(lcValues, '|')
            lcValue = GETWORDNUM(lcValues, lnCase, '|')
            lcCommand = 'ON SELECTION BAR ' + lcValue + ' OF cfparent '
            lcCommand = lcCommand + IIF(lnKind=1, 'DO cfHandler', 'nNativeGuard = 2')
            lnError = 0
            TRY
                &lcCommand
            CATCH TO loError
                lnError = loError.ErrorNo
            ENDTRY
            lcOut = lcOut + lcValue + '|ERR=' + TRANSFORM(lnError) + '|COUNT=' + TRANSFORM(CNTBAR('cfparent')) + '|ID=' + TRANSFORM(GETBAR('cfparent', 1)) + '|GUARD=' + TRANSFORM(nNativeGuard) + CHR(10)
        ENDFOR
        RELEASE POPUP cfparent
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'probe.out')
RETURN
PROCEDURE cfHandler
    nNativeGuard = 1
    RETURN
ENDPROC
