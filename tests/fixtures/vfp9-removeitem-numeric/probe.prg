* Fresh independent installed VFP9 RemoveItem observation. No Copperfin oracle.
LOCAL lcOut, lcClass, lcShape, lcExpr, lnClass, lnShape, lnMode, lnCase
LOCAL loList, luInput, luResult, loError, lcStatus, lnCalls, lcBefore, lnRow
LOCAL ARRAY aExpr[47]
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
aExpr[1] = [0]
aExpr[2] = [0.9]
aExpr[3] = [1]
aExpr[4] = [1.4]
aExpr[5] = [1.5]
aExpr[6] = [1.9]
aExpr[7] = [2]
aExpr[8] = [2.4]
aExpr[9] = [2.5]
aExpr[10] = [2.9]
aExpr[11] = [3]
aExpr[12] = [3.9]
aExpr[13] = [4]
aExpr[14] = [4.9]
aExpr[15] = [11]
aExpr[16] = [22.9]
aExpr[17] = [-0.9]
aExpr[18] = [-1]
aExpr[19] = [-1.9]
aExpr[20] = [2147483647]
aExpr[21] = [2147483647.9]
aExpr[22] = [2147483648]
aExpr[23] = [2147483648.5]
aExpr[24] = [4294967294]
aExpr[25] = [4294967295]
aExpr[26] = [4294967295.1]
aExpr[27] = [4294967295.5]
aExpr[28] = [4294967295.9]
aExpr[29] = [4294967296]
aExpr[30] = [-2147483649]
aExpr[31] = [-4294967293]
aExpr[32] = [-4294967294.5]
aExpr[33] = [-4294967295.9]
aExpr[34] = [-4294967296]
aExpr[35] = [-4294967297]
aExpr[36] = [-8589934590]
aExpr[37] = [-8589934591.9]
aExpr[38] = [8589934593]
aExpr[39] = [1E300]
aExpr[40] = [-1E300]
aExpr[41] = [EXP(1000)]
aExpr[42] = [-EXP(1000)]
aExpr[43] = [.T.]
aExpr[44] = [.F.]
aExpr[45] = [.NULL.]
aExpr[46] = ['2']
aExpr[47] = [$2.5]
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    FOR lnShape = 0 TO 2
        lcShape = IIF(lnShape = 0, 'empty', IIF(lnShape = 1, 'sparse', 'dense'))
        FOR lnMode = 0 TO IIF(lnClass = 1, 1, 0)
            FOR lnCase = 1 TO ALEN(aExpr)
                loList = CREATEOBJECT(lcClass)
                loList.ColumnCount = 2
                IF lnShape = 1
                    loList.AddListItem('first', 11)
                    loList.AddListItem('second', 22)
                    loList.AddListItem('third', 33)
                ENDIF
                IF lnShape = 2
                    loList.AddItem('first')
                    loList.AddItem('second')
                    loList.AddItem('third')
                ENDIF
                IF lnShape > 0
                    FOR lnRow = 1 TO 3
                        loList.ItemData(lnRow) = lnRow * 101
                        loList.List(lnRow, 2) = 'cell' + TRANSFORM(lnRow)
                    ENDFOR
                ENDIF
                IF lnMode = 1
                    loList.MultiSelect = .T.
                    IF lnShape > 0
                        loList.Selected(1) = .T.
                        loList.Selected(2) = .T.
                    ENDIF
                ENDIF
                IF lnShape > 0
                    loList.ListIndex = 2
                    IF lnClass = 1
                        loList.TopIndex = 2
                    ENDIF
                ENDIF
                lcBefore = Snapshot(loList, lcClass)
                lcExpr = aExpr[lnCase]
                lcStatus = 'OK'
                luResult = [not-returned]
                TRY
                    luInput = EVALUATE(lcExpr)
                    luResult = loList.RemoveItem(luInput)
                CATCH TO loError
                    lcStatus = 'ERR' + TRANSFORM(loError.ErrorNo)
                ENDTRY
                lnCalls = lnCalls + 1
                lcOut = lcOut + lcClass + '|' + lcShape + '|multi' + TRANSFORM(lnMode) + '|' + lcExpr + '|' + lcStatus + '|'
                lcOut = lcOut + VARTYPE(luResult) + ':' + TRANSFORM(luResult) + '|' + lcBefore + '|' + Snapshot(loList, lcClass) + CHR(13) + CHR(10)
                loList = .NULL.
            ENDFOR
        ENDFOR
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE|' + TRANSFORM(lnCalls) + CHR(13) + CHR(10)
=STRTOFILE(lcOut, 'removeitem-results.txt')
RETURN

FUNCTION Snapshot
LPARAMETERS loList, lcClass
LOCAL lcState, lnRow, lcIDs, lcCells, lcData, lcBits, lcTop
lcIDs = ''
lcCells = ''
lcData = ''
lcBits = ''
FOR lnRow = 1 TO loList.ListCount
    lcIDs = lcIDs + LTRIM(STR(loList.IndexToItemID(lnRow), 20, 0)) + ','
    lcCells = lcCells + loList.List(lnRow, 1) + ':' + loList.List(lnRow, 2) + ','
    lcData = lcData + TRANSFORM(loList.ItemData(lnRow)) + ','
    IF lcClass = 'ListBox'
        lcBits = lcBits + TRANSFORM(loList.Selected(lnRow)) + ','
    ENDIF
ENDFOR
lcTop = '-'
IF lcClass = 'ListBox'
    lcTop = TRANSFORM(loList.TopIndex) + ',' + TRANSFORM(loList.TopItemID)
ENDIF
lcState = 'count' + TRANSFORM(loList.ListCount) + ';focus' + TRANSFORM(loList.ListIndex) + ',' + TRANSFORM(loList.ListItemID) + ';top' + lcTop
lcState = lcState + ';value' + VARTYPE(loList.Value) + ':' + TRANSFORM(loList.Value) + ';display' + VARTYPE(loList.DisplayValue) + ':' + TRANSFORM(loList.DisplayValue)
lcState = lcState + ';ids' + lcIDs + ';cells' + lcCells + ';data' + lcData + ';bits' + lcBits
RETURN lcState
ENDFUNC
