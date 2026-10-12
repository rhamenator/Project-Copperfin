* Exact old-smoke input corroboration only; no expected results or SETPEM claim.
LOCAL loList, loError, lcStatus
? 'VERSION|' + VERSION()
loList = CREATEOBJECT('ListBox')
loList.AddListItem('Zulu', 30)
loList.AddListItem('Alpha', 10)
loList.AddListItem('Echo', 20)
loList.TopIndex = 3
? 'BEFORE|top=' + TRANSFORM(loList.TopIndex) + '|id=' + TRANSFORM(loList.TopItemID) + ;
    '|focus=' + TRANSFORM(loList.ListIndex) + '|count=' + TRANSFORM(loList.ListCount)
lcStatus = 'OK'
TRY
    loList.TopIndex = 99
CATCH TO loError
    lcStatus = 'ERROR:' + TRANSFORM(loError.ErrorNo)
ENDTRY
? 'AFTER|input=99|status=' + lcStatus + '|top=' + TRANSFORM(loList.TopIndex) + ;
    '|id=' + TRANSFORM(loList.TopItemID) + '|focus=' + TRANSFORM(loList.ListIndex) + ;
    '|count=' + TRANSFORM(loList.ListCount)
? 'DONE|calls=1'
