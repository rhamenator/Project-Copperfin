* Independent installed-VFP9 boundary observation before helper/assertion design.
LOCAL lcOut, lcClass, lcExpr, lnClass, lnCase, loList, luIndex, luResult, loError
LOCAL lcStatus, lcKind, lcValue, lcPath
LOCAL ARRAY aExpr[20]
lcPath = 'Z:\home\rich\.codex\worktrees\selectedid-numeric-5611\Project-Copperfin\build\selectedid-native\boundary.out'
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
aExpr[1] = '2147483647.1'
aExpr[2] = '2147483647.5'
aExpr[3] = '2147483647.9'
aExpr[4] = '2147483648.1'
aExpr[5] = '-2147483648.1'
aExpr[6] = '-2147483648.9'
aExpr[7] = '-2147483649.1'
aExpr[8] = '-2147483649.5'
aExpr[9] = '-2147483649.9'
aExpr[10] = '-4294967295.9'
aExpr[11] = '-4294967294.5'
aExpr[12] = '-4294967296'
aExpr[13] = '-4294967296.5'
aExpr[14] = '-4294967297'
aExpr[15] = '9007199254740992'
aExpr[16] = '-9007199254740992'
aExpr[17] = '-0'
aExpr[18] = '0.9999999999999999'
aExpr[19] = '3.999999999999999'
aExpr[20] = '-4294967297.9'
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    FOR lnCase = 1 TO ALEN(aExpr)
        loList = CREATEOBJECT(lcClass)
        loList.AddItem('first')
        loList.AddItem('second')
        loList.AddItem('third')
        loList.ItemData(1) = 101
        loList.ItemData(2) = 202
        loList.ItemData(3) = 303
        loList.ListIndex = 2
        lcExpr = aExpr[lnCase]
        lcStatus = 'OK'
        lcKind = '-'
        lcValue = '-'
        TRY
            luIndex = EVALUATE(lcExpr)
            luResult = loList.SelectedID(luIndex)
            lcKind = VARTYPE(luResult)
            lcValue = TRANSFORM(luResult)
        CATCH TO loError
            lcStatus = 'ERR' + TRANSFORM(loError.ErrorNo)
        ENDTRY
        lcOut = lcOut + lcClass + '|' + lcExpr + '|' + lcStatus + '|' + lcKind + '|' + lcValue + '|'
        lcOut = lcOut + TRANSFORM(loList.ListCount) + '|' + TRANSFORM(loList.ListIndex) + '|'
        lcOut = lcOut + TRANSFORM(loList.ItemData(1)) + ',' + TRANSFORM(loList.ItemData(2)) + ',' + TRANSFORM(loList.ItemData(3)) + '|'
        lcOut = lcOut + TRANSFORM(loList.Selected(1)) + ',' + TRANSFORM(loList.Selected(2)) + ',' + TRANSFORM(loList.Selected(3)) + CHR(13) + CHR(10)
        loList = .NULL.
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE|40' + CHR(13) + CHR(10)
=STRTOFILE(lcOut, lcPath, 0)
