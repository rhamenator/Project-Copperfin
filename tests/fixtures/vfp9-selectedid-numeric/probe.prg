* Independent installed-VFP9 observation, not Copperfin expectations.
LOCAL lcOut, lcClass, lcExpr, lnClass, lnCase, loList, luId, luResult, loError
LOCAL lcStatus, lcKind, lcValue, lcPath
LOCAL ARRAY aExpr[47]
lcPath = 'Z:\home\rich\.codex\worktrees\selectedid-numeric-5611\Project-Copperfin\build\selectedid-native\native.out'
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
aExpr[1] = '0'
aExpr[2] = '0.49'
aExpr[3] = '0.5'
aExpr[4] = '0.9'
aExpr[5] = '1'
aExpr[6] = '11'
aExpr[7] = '11.49'
aExpr[8] = '11.5'
aExpr[9] = '11.9'
aExpr[10] = '22'
aExpr[11] = '22.49'
aExpr[12] = '22.5'
aExpr[13] = '22.9'
aExpr[14] = '33'
aExpr[15] = '33.9'
aExpr[16] = '23'
aExpr[17] = '34'
aExpr[18] = '-0.49'
aExpr[19] = '-0.5'
aExpr[20] = '-0.9'
aExpr[21] = '-1'
aExpr[22] = '-1.9'
aExpr[23] = '2147483647'
aExpr[24] = '2147483648'
aExpr[25] = '-2147483648'
aExpr[26] = '-2147483649'
aExpr[27] = '4294967295'
aExpr[28] = '4294967296'
aExpr[29] = '4294967318'
aExpr[30] = '4294967318.9'
aExpr[31] = '-4294967274'
aExpr[32] = '-4294967274.9'
aExpr[33] = '-4294967285'
aExpr[34] = '-4294967263'
aExpr[35] = '1e20'
aExpr[36] = '-1e20'
aExpr[37] = '1e300'
aExpr[38] = '-1e300'
aExpr[39] = 'EXP(1000)'
aExpr[40] = '-EXP(1000)'
aExpr[41] = '.T.'
aExpr[42] = '.F.'
aExpr[43] = '.NULL.'
aExpr[44] = "'22.9'"
aExpr[45] = '$22.9'
aExpr[46] = '1e-300'
aExpr[47] = '-1e-300'
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
lcOut = lcOut + 'DONE|94' + CHR(13) + CHR(10)
=STRTOFILE(lcOut, lcPath, 0)
