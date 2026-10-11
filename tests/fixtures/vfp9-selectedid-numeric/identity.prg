* Independent installed-VFP9 observation, not Copperfin expectations.
LOCAL lcOut, lcClass, lcExpr, lnClass, lnCase, loList, luId, luResult, loError
LOCAL lcStatus, lcKind, lcValue, lcPath
LOCAL ARRAY aExpr[10]
lcPath = 'Z:\home\rich\.codex\worktrees\selectedid-numeric-5611\Project-Copperfin\build\selectedid-native\identity.out'
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
aExpr[1] = '2'
aExpr[2] = '2.9'
aExpr[3] = '21.9'
aExpr[4] = '22.9'
aExpr[5] = '23.9'
aExpr[6] = '-4294967274.5'
aExpr[7] = '-4294967274.9'
aExpr[8] = '-4294967285.9'
aExpr[9] = '-4294967263.9'
aExpr[10] = '2147483647.5'
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    FOR lnCase = 1 TO ALEN(aExpr)
        loList = CREATEOBJECT(lcClass)
        loList.AddListItem('first', 11)
        loList.AddListItem('second', 22)
        loList.AddListItem('third', 33)
        loList.ItemData(1) = 101
        loList.ItemData(2) = 202
        loList.ItemData(3) = 303
        loList.ListIndex = 2
        lcExpr = aExpr[lnCase]
        lcStatus = 'OK'
        lcKind = '-'
        lcValue = '-'
        TRY
            luId = EVALUATE(lcExpr)
            luResult = loList.SelectedID(luId)
            lcKind = VARTYPE(luResult)
            lcValue = TRANSFORM(luResult)
        CATCH TO loError
            lcStatus = 'ERR' + TRANSFORM(loError.ErrorNo)
        ENDTRY
        lcOut = lcOut + lcClass + '|' + lcExpr + '|' + lcStatus + '|' + lcKind + '|' + lcValue + '|'
        lcOut = lcOut + TRANSFORM(loList.ListCount) + '|' + TRANSFORM(loList.ListIndex) + '|' + TRANSFORM(loList.ListItemID) + '|'
        lcOut = lcOut + TRANSFORM(loList.ItemData(1)) + ',' + TRANSFORM(loList.ItemData(2)) + ',' + TRANSFORM(loList.ItemData(3)) + '|'
        lcOut = lcOut + TRANSFORM(loList.Selected(1)) + ',' + TRANSFORM(loList.Selected(2)) + ',' + TRANSFORM(loList.Selected(3)) + CHR(13) + CHR(10)
        loList = .NULL.
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE|20' + CHR(13) + CHR(10)
=STRTOFILE(lcOut, lcPath, 0)
