* Fresh independent installed VFP9 RemoveItem observation. No Copperfin oracle.
LOCAL lcOut, lcClass, lcShape, lcExpr, lnClass, lnShape, lnMode, lnCase
LOCAL loList, luInput, luResult, loError, lcStatus, lnCalls, lcBefore, lnRow
LOCAL ARRAY aExpr[33]
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
aExpr[1] = [32766]
aExpr[2] = [32767]
aExpr[3] = [32767.1]
aExpr[4] = [32767.9]
aExpr[5] = [32768]
aExpr[6] = [32768.9]
aExpr[7] = [65534]
aExpr[8] = [65535]
aExpr[9] = [65535.9]
aExpr[10] = [65536]
aExpr[11] = [65537]
aExpr[12] = [65538.9]
aExpr[13] = [131073]
aExpr[14] = [2147483646]
aExpr[15] = [2147483647.1]
aExpr[16] = [2147483647.5]
aExpr[17] = [2147483649]
aExpr[18] = [4294967297]
aExpr[19] = [4294967298.9]
aExpr[20] = [4294967299]
aExpr[21] = [-32767]
aExpr[22] = [-32768]
aExpr[23] = [-32769]
aExpr[24] = [-65535]
aExpr[25] = [-65534.9]
aExpr[26] = [-65533]
aExpr[27] = [-131071]
aExpr[28] = [9007199254740991]
aExpr[29] = [9007199254740992]
aExpr[30] = [-9007199254740991]
aExpr[31] = [-9007199254740992]
aExpr[32] = [1E-300]
aExpr[33] = [-1E-300]
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
=STRTOFILE(lcOut, 'boundary-results.txt')
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
