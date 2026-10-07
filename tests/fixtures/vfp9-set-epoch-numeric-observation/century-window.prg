LOCAL nCentury, nRollover, nYear, lcYear, lcWindow, lcDisplay, lcDate, lcCommand, nWindow
? 'VERSION|' + VERSION()
SET DATE TO MDY
FOR nWindow = 1 TO 3
    DO CASE
    CASE nWindow = 1
        nCentury = 19
        nRollover = 50
    CASE nWindow = 2
        nCentury = 19
        nRollover = 75
    OTHERWISE
        nCentury = 20
        nRollover = 25
    ENDCASE
    lcCommand = 'SET CENTURY TO ' + TRANSFORM(nCentury) + ' ROLLOVER ' + TRANSFORM(nRollover)
    &lcCommand
    lcWindow = TRANSFORM(nCentury * 100 + nRollover)
    FOR nDisplay = 1 TO 2
        IF nDisplay = 1
            SET CENTURY OFF
        ELSE
            SET CENTURY ON
        ENDIF
        lcDisplay = SET('CENTURY')
        ? 'STATE|' + lcWindow + '|' + lcDisplay + '|' + TRANSFORM(SET('CENTURY',1)) + '|' + TRANSFORM(SET('CENTURY',2))
        FOR nYear = 0 TO 99
            IF INLIST(nYear, 0, 24, 25, 49, 50, 74, 75, 99)
                lcYear = RIGHT('0' + TRANSFORM(nYear), 2)
                lcDate = '01/02/' + lcYear
                ? 'YEAR|' + lcWindow + '|' + lcDisplay + '|' + lcYear + '|' + TRANSFORM(YEAR(CTOD(lcDate))) + '|' + TRANSFORM(YEAR(TTOD(CTOT(lcDate + ' 13:45:56'))))
                ? 'FORMAT|' + lcWindow + '|' + lcDisplay + '|' + lcYear + '|' + DTOC(CTOD(lcDate)) + '|' + DTOC(CTOD(lcDate),1) + '|' + TTOC(CTOT(lcDate + ' 13:45:56'),1) + '|' + TTOC(DTOT(CTOD(lcDate)),1)
            ENDIF
        ENDFOR
    ENDFOR
ENDFOR
