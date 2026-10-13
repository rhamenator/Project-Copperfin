* Fresh independent installed VFP9 RemoveItem observation. No Copperfin oracle.
LOCAL lcOut, lcClass, lcShape, lcExpr, lnClass, lnShape, lnMode, lnCase
LOCAL loList, luInput, luResult, loError, lcStatus, lnCalls, lcBefore, lnRow
LOCAL ARRAY aExpr[20]
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
aExpr[1] = [32769]
aExpr[2] = [32769.9]
aExpr[3] = [49999]
aExpr[4] = [50000]
aExpr[5] = [50000.9]
aExpr[6] = [50001]
aExpr[7] = [65000]
aExpr[8] = [65001]
aExpr[9] = [65500]
aExpr[10] = [65501]
aExpr[11] = [65502]
aExpr[12] = [65532]
aExpr[13] = [65533]
aExpr[14] = [65533.9]
aExpr[15] = [65534.1]
aExpr[16] = [65535.1]
aExpr[17] = [4295000064]
aExpr[18] = [4295032831]
aExpr[19] = [-4294934528]
aExpr[20] = [-4294901761]
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
=STRTOFILE(lcOut, 'range-results.txt')
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
