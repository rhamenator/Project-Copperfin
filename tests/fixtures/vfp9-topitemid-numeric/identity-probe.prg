* Independent installed VFP9 TopItemID identity/boundary observation.
* No Copperfin code, helper, prior output or expected-result oracle.
LOCAL lcOut, loList, loError, lcStatus, lcIDs, lcExpr, lnID, lnCase, lnCalls
LOCAL ARRAY aIDs[8], aExpr[12]
aIDs[1]=2147483646
aIDs[2]=2147483647
aIDs[3]=2147483648
aIDs[4]=2147483649
aIDs[5]=4294967293
aIDs[6]=4294967294
aIDs[7]=4294967295
aIDs[8]=4294967296
aExpr[1]='2147483646.9'
aExpr[2]='2147483647'
aExpr[3]='2147483647.9'
aExpr[4]='2147483648'
aExpr[5]='2147483649.9'
aExpr[6]='4294967293'
aExpr[7]='4294967294'
aExpr[8]='4294967295'
aExpr[9]='-2147483649'
aExpr[10]='-2147483648'
aExpr[11]='-1'
aExpr[12]='-6442450945'
lcOut='VERSION|'+VERSION()+CHR(13)+CHR(10)
lnCalls=0
FOR lnID=1 TO ALEN(aIDs)
    FOR lnCase=1 TO ALEN(aExpr)
        loList=CREATEOBJECT('ListBox')
        loList.AddListItem('first',11)
        lcStatus='OK'
        TRY
            loList.AddListItem('boundary',aIDs[lnID])
        CATCH TO loError
            lcStatus='ERR'+TRANSFORM(loError.ErrorNo)
        ENDTRY
        lcIDs=''
        FOR lnRow=1 TO loList.ListCount
            lcIDs=lcIDs+LTRIM(STR(loList.IndexToItemID(lnRow),20,0))+','
        ENDFOR
        lcOut=lcOut+'SETUP|'+LTRIM(STR(aIDs[lnID],20,0))+'|'+lcStatus+'|'+TRANSFORM(loList.ListCount)+'|'+lcIDs+CHR(13)+CHR(10)
        lcExpr=aExpr[lnCase]
        lcStatus='OK'
        TRY
            loList.TopItemID=EVALUATE(lcExpr)
        CATCH TO loError
            lcStatus='ERR'+TRANSFORM(loError.ErrorNo)
        ENDTRY
        lnCalls=lnCalls+1
        lcOut=lcOut+'ASSIGN|'+LTRIM(STR(aIDs[lnID],20,0))+'|'+lcExpr+'|'+lcStatus+'|'+TRANSFORM(loList.TopIndex)+'|'+LTRIM(STR(loList.TopItemID,20,0))+'|'+TRANSFORM(loList.ListIndex)+'|'+lcIDs+CHR(13)+CHR(10)
        loList=.NULL.
    ENDFOR
ENDFOR
lcOut=lcOut+'DONE|'+TRANSFORM(lnCalls)+CHR(13)+CHR(10)
=STRTOFILE(lcOut,'Z:\home\rich\.codex\worktrees\topitemid-numeric-5611\Project-Copperfin\build\topitemid-native\identity.out',0)
