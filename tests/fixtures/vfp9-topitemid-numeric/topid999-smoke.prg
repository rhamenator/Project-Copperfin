* Fresh exact old-smoke input corroboration; no expected result or SETPEM claim.
LOCAL loList, loError, lcStatus, lcOut
lcOut='VERSION|'+VERSION()+CHR(13)+CHR(10)
loList=CREATEOBJECT('ListBox')
loList.AddListItem('Zulu',30)
loList.AddListItem('Alpha',10)
loList.AddListItem('Echo',20)
loList.TopItemID=20
lcOut=lcOut+'BEFORE|top='+TRANSFORM(loList.TopIndex)+'|id='+TRANSFORM(loList.TopItemID)+'|focus='+TRANSFORM(loList.ListIndex)+'|count='+TRANSFORM(loList.ListCount)+CHR(13)+CHR(10)
lcStatus='OK'
TRY
    loList.TopItemID=999
CATCH TO loError
    lcStatus='ERROR:'+TRANSFORM(loError.ErrorNo)
ENDTRY
lcOut=lcOut+'AFTER|input=999|status='+lcStatus+'|top='+TRANSFORM(loList.TopIndex)+'|id='+TRANSFORM(loList.TopItemID)+'|focus='+TRANSFORM(loList.ListIndex)+'|count='+TRANSFORM(loList.ListCount)+CHR(13)+CHR(10)
lcOut=lcOut+'DONE|calls=1'+CHR(13)+CHR(10)
=STRTOFILE(lcOut,'Z:\home\rich\.codex\worktrees\topitemid-numeric-5611\Project-Copperfin\build\topitemid-native\topid999.out',0)
