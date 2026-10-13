* Fresh installed VFP9 evidence; no Copperfin implementation as oracle.
LOCAL lcClass, loList, loError, lcOut, lnClass, lnShape, lnCase, lnCalls
LOCAL lnBase, lnBefore, lnRow, lcStatus, lnArg, lcPhase, lvReturn
LOCAL ARRAY aExpr[26]
aExpr[1] = [0.1]
aExpr[2] = [0.5]
aExpr[3] = [0.9]
aExpr[4] = [0.9999999999999999]
aExpr[5] = [1]
aExpr[6] = [32766]
aExpr[7] = [32766.9]
aExpr[8] = [32767]
aExpr[9] = [32767.1]
aExpr[10] = [32767.5]
aExpr[11] = [32767.9]
aExpr[12] = [32768]
aExpr[13] = [32768.9]
aExpr[14] = [32769]
aExpr[15] = [4295000062]
aExpr[16] = [4295000063]
aExpr[17] = [4295000063.9]
aExpr[18] = [4295000064]
aExpr[19] = [-4294934530]
aExpr[20] = [-4294934529]
aExpr[21] = [-4294934529.9]
aExpr[22] = [-4294934528]
aExpr[23] = [8589967359]
aExpr[24] = [8589967360]
aExpr[25] = [-8589901825]
aExpr[26] = [-8589901824]
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    FOR lnShape = 1 TO 3
        FOR lnCase = 1 TO ALEN(aExpr)
            loList = CREATEOBJECT(lcClass)
            * Configure before populating: RST assignment clears native rows.
            loList.RowSourceType = 0
            loList.ControlSource = ''
            loList.ColumnCount = 2
            IF lnShape < 3
            lnBase = IIF(lnShape = 1, 1, 11)
            loList.AddListItem('first', lnBase)
            loList.AddListItem('second', lnBase * 2)
            loList.AddListItem('third', lnBase * 3)
            loList.List(1, 2) = 'cell1'
            loList.List(2, 2) = 'cell2'
            loList.List(3, 2) = 'cell3'
            loList.ItemData(1) = 101
            loList.ItemData(2) = 202
            loList.ItemData(3) = 303
            ENDIF
            lnBefore = loList.ListCount
            lcStatus = 'OK'
            lcPhase = 'evaluate'
            lvReturn = 'not-returned'
            TRY
                lnArg = EVALUATE(aExpr[lnCase])
                lcPhase = 'invoke'
                lvReturn = loList.RemoveListItem(lnArg)
                lcPhase = 'returned'
            CATCH TO loError
                lcStatus = 'ERR:' + TRANSFORM(loError.ErrorNo)
            ENDTRY
            lcOut = lcOut + lcClass + '|shape=' + TRANSFORM(lnShape) + '|' + ;
                aExpr[lnCase] + '|before=' + TRANSFORM(lnBefore) + '|' + ;
                lcPhase + '|' + lcStatus + '|count=' + TRANSFORM(loList.ListCount) + ;
                '|returnType=' + VARTYPE(lvReturn) + '|return=' + TRANSFORM(lvReturn)
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
=STRTOFILE(lcOut, 'boundary-results.txt')
