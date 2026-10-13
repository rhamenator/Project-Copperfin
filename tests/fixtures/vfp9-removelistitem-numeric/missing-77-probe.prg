* Independent exact old-smoke shape, not Copperfin expected output.
LOCAL lnClass, lnTrial, lnStage, lnCalls, lnBefore, loList, loError
LOCAL lcClass, lcOut, lcStatus, lcPhase
LOCAL ARRAY aID[3]
aID[1] = 10
aID[2] = 20
aID[3] = 77
lcOut = 'VERSION|'+VERSION()+CHR(13)+CHR(10)
lnCalls = 0
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass=1,'ListBox','ComboBox')
    FOR lnTrial = 1 TO 2
        loList = CREATEOBJECT(lcClass)
        loList.RowSourceType = 0
        loList.ColumnCount = 2
        loList.AddListItem('Alpha',10)
        loList.AddListItem('A',10,2)
        loList.AddListItem('Beta',20)
        loList.AddListItem('B',20,2)
        loList.AddListItem('Gamma',30)
        loList.AddListItem('C',30,2)
        loList.ListItemID = 20
        FOR lnStage = 1 TO 3
            lnBefore = loList.ListCount
            lcStatus = 'OK'
            lcPhase = 'invoke'
            TRY
                loList.RemoveListItem(aID[lnStage])
                lcPhase = 'returned'
            CATCH TO loError
                lcStatus = 'ERR:'+TRANSFORM(loError.ErrorNo)
            ENDTRY
            lcOut = lcOut+lcClass+'|trial='+TRANSFORM(lnTrial)+'|id='+TRANSFORM(aID[lnStage]) + ;
                '|before='+TRANSFORM(lnBefore)+'|'+lcPhase+'|'+lcStatus + ;
                '|after='+TRANSFORM(loList.ListCount)+CHR(13)+CHR(10)
            lnCalls = lnCalls + 1
        ENDFOR
    ENDFOR
ENDFOR
lcOut = lcOut+'DONE|'+TRANSFORM(lnCalls)+CHR(13)+CHR(10)
=STRTOFILE(lcOut,'missing-77-results.txt')
