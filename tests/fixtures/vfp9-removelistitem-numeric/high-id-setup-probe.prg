* High-ID setup is measured separately, not confused with RemoveListItem lookup.
LOCAL lnClass, lnCase, loList, loError, lcClass, lcOut, lcPhase, lcStatus, lnCalls
LOCAL ARRAY aID[4]
aID[1] = 32766
aID[2] = 32767
aID[3] = 32768
aID[4] = 2147483647
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass=1, 'ListBox', 'ComboBox')
    FOR lnCase = 1 TO ALEN(aID)
        loList = CREATEOBJECT(lcClass)
        loList.RowSourceType = 0
        lcPhase = 'setup'
        lcStatus = 'OK'
        TRY
            loList.AddListItem('high', aID[lnCase])
            lcOut = lcOut + lcClass + '|id=' + TRANSFORM(aID[lnCase]) + ;
                '|setupCount=' + TRANSFORM(loList.ListCount)
            lcPhase = 'invoke'
            loList.RemoveListItem(aID[lnCase])
            lcPhase = 'returned'
        CATCH TO loError
            lcStatus = 'ERR:' + TRANSFORM(loError.ErrorNo)
            IF lcPhase = 'setup'
                lcOut = lcOut + lcClass + '|id=' + TRANSFORM(aID[lnCase]) + ;
                    '|setupCount=' + TRANSFORM(loList.ListCount)
            ENDIF
        ENDTRY
        lcOut = lcOut + '|' + lcPhase + '|' + lcStatus + ;
            '|after=' + TRANSFORM(loList.ListCount) + CHR(13) + CHR(10)
        lnCalls = lnCalls + 1
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE|' + TRANSFORM(lnCalls) + CHR(13) + CHR(10)
=STRTOFILE(lcOut, 'high-id-setup-results.txt')
