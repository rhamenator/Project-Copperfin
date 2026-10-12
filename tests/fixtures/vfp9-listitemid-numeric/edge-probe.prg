* Independent ListItemID assignment edge/state probe, no Copperfin expectations.
LOCAL lcOut, lcClass, lcShape, lcExpr, lnClass, lnShape, lnMode, lnCase, loList
LOCAL luInput, loError, lcStatus, lnCalls, lcBits, lcIDs, lcCells, lcData, lnRow
LOCAL ARRAY aExpr[30]
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
aExpr[1] = '0'
aExpr[2] = '0.9'
aExpr[3] = '1'
aExpr[4] = '1.9'
aExpr[5] = '2.9'
aExpr[6] = '3.9'
aExpr[7] = '4'
aExpr[8] = '22.9'
aExpr[9] = '-0.9'
aExpr[10] = '-1.9'
aExpr[11] = '2147483647.9'
aExpr[12] = '2147483648'
aExpr[13] = '4294967294'
aExpr[14] = '4294967295'
aExpr[15] = '4294967295.1'
aExpr[16] = '4294967295.5'
aExpr[17] = '4294967295.9'
aExpr[18] = '4294967296'
aExpr[19] = '4294967296.1'
aExpr[20] = '-2147483649'
aExpr[21] = '-4294967293'
aExpr[22] = '-4294967294.5'
aExpr[23] = '-4294967295.9'
aExpr[24] = '-4294967296'
aExpr[25] = '-4294967297'
aExpr[26] = '-8589934590'
aExpr[27] = '-8589934591.9'
aExpr[28] = '8589934593'
aExpr[29] = '-1e300'
aExpr[30] = '-EXP(1000)'
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    FOR lnShape = 0 TO 2
        lcShape = IIF(lnShape = 0, 'empty', IIF(lnShape = 1, 'sparse', 'dense'))
        FOR lnMode = 0 TO IIF(lnClass = 1, 1, 0)
            FOR lnCase = 1 TO ALEN(aExpr)
                loList = CREATEOBJECT(lcClass)
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
                    loList.ItemData(1) = 101
                    loList.ItemData(2) = 202
                    loList.ItemData(3) = 303
                ENDIF
                IF lnMode = 1
                    loList.MultiSelect = .T.
                    IF lnShape > 0
                        loList.Selected(1) = .T.
                        loList.Selected(2) = .T.
                    ENDIF
                ENDIF
                loList.ListIndex = IIF(lnShape = 0, 0, 2)
                lcExpr = aExpr[lnCase]
                lcStatus = 'OK'
                TRY
                    luInput = EVALUATE(lcExpr)
                    loList.ListItemID = luInput
                CATCH TO loError
                    lcStatus = 'ERR' + TRANSFORM(loError.ErrorNo)
                ENDTRY
                lnCalls = lnCalls + 1
                lcBits = ''
                lcIDs = ''
                lcCells = ''
                lcData = ''
                FOR lnRow = 1 TO loList.ListCount
                    lcBits = lcBits + TRANSFORM(loList.Selected(lnRow)) + ','
                    lcIDs = lcIDs + LTRIM(STR(loList.IndexToItemID(lnRow), 20, 0)) + ','
                    lcCells = lcCells + loList.List(lnRow) + ','
                    lcData = lcData + TRANSFORM(loList.ItemData(lnRow)) + ','
                ENDFOR
                lcOut = lcOut + lcClass + '|' + lcShape + '|multi' + TRANSFORM(lnMode) + '|' + lcExpr + '|' + lcStatus + '|'
                lcOut = lcOut + VARTYPE(loList.ListIndex) + ':' + TRANSFORM(loList.ListIndex) + '|' + TRANSFORM(loList.ListItemID) + '|'
                lcOut = lcOut + VARTYPE(loList.Value) + ':' + TRANSFORM(loList.Value) + '|' + VARTYPE(loList.DisplayValue) + ':' + TRANSFORM(loList.DisplayValue) + '|'
                lcOut = lcOut + TRANSFORM(loList.ListCount) + '|' + lcCells + '|' + lcData + '|' + lcBits + '|' + lcIDs + CHR(13) + CHR(10)
                loList = .NULL.
            ENDFOR
        ENDFOR
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE|' + TRANSFORM(lnCalls) + CHR(13) + CHR(10)
=STRTOFILE(lcOut, 'Z:\home\rich\.codex\worktrees\listitemid-numeric-5611\Project-Copperfin\build\listitemid-native\edge.out', 0)
