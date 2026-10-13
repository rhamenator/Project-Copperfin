* Independent installed VFP9 state/guard recovery; no Copperfin oracle.
LOCAL lnClass, lnShape, lnBinding, lnCase, lnCalls, lnRow, lnArg, lnBase
LOCAL loList, loError, lcClass, lcOut, lcBefore, lcAfter, lcPhase, lcStatus
LOCAL ARRAY aExpr[20]
aExpr[1] = [1E-300]
aExpr[2] = [-1E-300]
aExpr[3] = [2.4]
aExpr[4] = [2.5]
aExpr[5] = [4.9]
aExpr[6] = [32769.9]
aExpr[7] = [65537]
aExpr[8] = [2147483647.9]
aExpr[9] = [4294967295.9]
aExpr[10] = [4294967298.9]
aExpr[11] = [4294967299]
aExpr[12] = [-4294967294.5]
aExpr[13] = [-4294967293]
aExpr[14] = [4295032831]
aExpr[15] = [-4294901761]
aExpr[16] = [-4294967297]
aExpr[17] = [-8589934590]
aExpr[18] = [32767.9]
aExpr[19] = [4295000063.9]
aExpr[20] = [11.9]
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    FOR lnShape = 1 TO IIF(lnClass=1, 5, 4)
        FOR lnBinding = 1 TO 2
            FOR lnCase = 1 TO ALEN(aExpr)
                loList = CREATEOBJECT(lcClass)
                * All clearing/configuration is BEFORE population.
                loList.RowSourceType = 0
                loList.ControlSource = IIF(lnBinding=1, '', '   ')
                loList.ColumnCount = 2
                IF lnShape = 5
                    loList.MultiSelect = .T.
                ENDIF
                IF lnShape = 3
                    loList.AddListItem('high', 32767)
                    loList.List(1,2) = 'highcell'
                    loList.ItemData(1) = 404
                    loList.ListIndex = 1
                ELSE
                    IF lnShape <> 4
                        lnBase = IIF(lnShape=1, 1, 11)
                        loList.AddListItem('first', lnBase)
                        loList.AddListItem('second', lnBase*2)
                        loList.AddListItem('third', lnBase*3)
                        FOR lnRow = 1 TO 3
                            loList.List(lnRow,2) = 'cell'+TRANSFORM(lnRow)
                            loList.ItemData(lnRow) = lnRow*101
                        ENDFOR
                        loList.ListIndex = 2
                        IF lnClass = 1
                            loList.TopIndex = 2
                        ENDIF
                        IF lnShape = 5
                            loList.Selected(1) = .T.
                            loList.Selected(2) = .T.
                        ENDIF
                    ENDIF
                ENDIF
                lcBefore = state_snapshot(loList, lnClass)
                lcPhase = 'evaluate'
                lcStatus = 'OK'
                TRY
                    lnArg = EVALUATE(aExpr[lnCase])
                    lcPhase = 'invoke'
                    loList.RemoveListItem(lnArg)
                    lcPhase = 'returned'
                CATCH TO loError
                    lcStatus = 'ERR:'+TRANSFORM(loError.ErrorNo)
                ENDTRY
                lcAfter = state_snapshot(loList, lnClass)
                lcOut = lcOut + lcClass + '|shape='+TRANSFORM(lnShape) + ;
                    '|binding='+TRANSFORM(lnBinding) + '|'+aExpr[lnCase] + ;
                    '|'+lcPhase+'|'+lcStatus+'|before='+lcBefore+'|after='+lcAfter + ;
                    CHR(13)+CHR(10)
                lnCalls = lnCalls + 1
            ENDFOR
        ENDFOR
    ENDFOR
ENDFOR
lcOut = lcOut+'DONE|'+TRANSFORM(lnCalls)+CHR(13)+CHR(10)
=STRTOFILE(lcOut,'state-results.txt')

FUNCTION state_snapshot(toList, tnClass)
    LOCAL lcState, lnSlot
    lcState = 'count='+TRANSFORM(toList.ListCount) + ;
        ',focus='+TRANSFORM(toList.ListIndex)+',id='+TRANSFORM(toList.ListItemID) + ;
        ',value='+TRANSFORM(toList.Value)+',display='+TRANSFORM(toList.DisplayValue)
    IF tnClass = 1
        lcState = lcState+',top='+TRANSFORM(toList.TopIndex)+',topid='+TRANSFORM(toList.TopItemID)
    ENDIF
    FOR lnSlot = 1 TO toList.ListCount
        lcState = lcState+';row='+TRANSFORM(toList.IndexToItemID(lnSlot))+',' + ;
            toList.List(lnSlot,1)+','+toList.List(lnSlot,2)+',' + ;
            TRANSFORM(toList.ItemData(lnSlot))+','+TRANSFORM(toList.Selected(lnSlot))
    ENDFOR
    RETURN lcState
ENDFUNC
