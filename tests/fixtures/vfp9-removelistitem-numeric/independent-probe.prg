* Fresh installed VFP9 evidence; no Copperfin implementation as oracle.
LOCAL lcClass, loList, loError, lcOut, lnClass, lnShape, lnCase, lnCalls
LOCAL lnBase, lnBefore, lnRow, lcStatus, lnArg, lcPhase, lvReturn
LOCAL ARRAY aExpr[36]
aExpr[1] = [1]
aExpr[2] = [1.4]
aExpr[3] = [1.5]
aExpr[4] = [1.9]
aExpr[5] = [2.9]
aExpr[6] = [3.9]
aExpr[7] = [4]
aExpr[8] = [11]
aExpr[9] = [11.9]
aExpr[10] = [22.9]
aExpr[11] = [33.9]
aExpr[12] = [0]
aExpr[13] = [-0.9]
aExpr[14] = [-1]
aExpr[15] = [32768]
aExpr[16] = [32769]
aExpr[17] = [65535]
aExpr[18] = [2147483647]
aExpr[19] = [2147483648]
aExpr[20] = [4294967295]
aExpr[21] = [4294967297]
aExpr[22] = [4294967307.9]
aExpr[23] = [-4294967295.9]
aExpr[24] = [-4294967285.5]
aExpr[25] = [8589934593]
aExpr[26] = [-8589934591.9]
aExpr[27] = [9007199254740991]
aExpr[28] = [-9007199254740991]
aExpr[29] = [9007199254740992]
aExpr[30] = [-9007199254740992]
aExpr[31] = [1e300]
aExpr[32] = [-1e300]
aExpr[33] = [1e308 * 1e308]
aExpr[34] = [-1e308 * 1e308]
aExpr[35] = [4294967296]
aExpr[36] = [-4294967296]
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    FOR lnShape = 1 TO 2
        FOR lnCase = 1 TO ALEN(aExpr)
            loList = CREATEOBJECT(lcClass)
            * Configure before populating: RST assignment clears native rows.
            loList.RowSourceType = 0
            loList.ControlSource = ''
            loList.ColumnCount = 2
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
=STRTOFILE(lcOut, 'independent-results.txt')
