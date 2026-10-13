* Independent installed VFP9 boundary for review discussion4253248168.
LOCAL lcClass, lcSource, lcExpr, lcOut, loList, loError, lcStatus
LOCAL lnClass, lnSource, lnCase, lnCalls, lnBefore
LOCAL ARRAY aExpr[6]
aExpr[1] = [2.9]
aExpr[2] = [0]
aExpr[3] = [4]
aExpr[4] = [32769]
aExpr[5] = [4294967297]
aExpr[6] = [-4294967294.5]
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    FOR lnSource = 1 TO 3
        lcSource = IIF(lnSource = 1, '', IIF(lnSource = 2, ' ', CHR(9) + '  '))
        FOR lnCase = 1 TO ALEN(aExpr)
            loList = CREATEOBJECT(lcClass)
            loList.RowSourceType = 0
            loList.ControlSource = lcSource
            loList.ColumnCount = 2
            loList.AddListItem('first', 11)
            loList.AddListItem('second', 22)
            loList.AddListItem('third', 33)
            loList.List(1, 2) = 'cell1'
            loList.List(2, 2) = 'cell2'
            loList.List(3, 2) = 'cell3'
            loList.ItemData(1) = 101
            loList.ItemData(2) = 202
            loList.ItemData(3) = 303
            lnBefore = loList.ListCount
            lcStatus = 'OK'
            TRY
                loList.RemoveItem(EVALUATE(aExpr[lnCase]))
            CATCH TO loError
                lcStatus = 'ERR:' + TRANSFORM(loError.ErrorNo)
            ENDTRY
            lcOut = lcOut + lcClass + '|' + TRANSFORM(lnSource) + '|' + ;
                aExpr[lnCase] + '|before=' + TRANSFORM(lnBefore) + '|' + lcStatus + '|count=' + TRANSFORM(loList.ListCount)
            FOR lnRow = 1 TO loList.ListCount
                lcOut = lcOut + '|row=' + TRANSFORM(loList.IndexToItemID(lnRow)) + ',' + ;
                    loList.List(lnRow, 1) + ',' + loList.List(lnRow, 2) + ',' + ;
                    TRANSFORM(loList.ItemData(lnRow))
            ENDFOR
            lcOut = lcOut + CHR(13) + CHR(10)
            lnCalls = lnCalls + 1
        ENDFOR
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE|' + TRANSFORM(lnCalls) + CHR(13) + CHR(10)
=STRTOFILE(lcOut, 'review2-populated-rst0-results.txt')
