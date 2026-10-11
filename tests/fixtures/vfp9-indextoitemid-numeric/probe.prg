* Independent installed-VFP9 IndexToItemID observation.
LOCAL lcOut, lcClass, lcExpr, lnClass, lnCase, loList, luIndex, luResult, loError
LOCAL lcStatus, lcKind, lcValue, lcPath
LOCAL ARRAY aExpr[59]
lcPath = 'Z:\home\rich\.codex\worktrees\indextoitemid-numeric-5611\Project-Copperfin\build\indextoitemid-native\native.out'
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
aExpr[1] = '0'
aExpr[2] = '-0'
aExpr[3] = '0.49'
aExpr[4] = '0.5'
aExpr[5] = '0.9'
aExpr[6] = '1'
aExpr[7] = '1.49'
aExpr[8] = '1.5'
aExpr[9] = '1.9'
aExpr[10] = '2'
aExpr[11] = '2.49'
aExpr[12] = '2.5'
aExpr[13] = '2.9'
aExpr[14] = '3'
aExpr[15] = '3.49'
aExpr[16] = '3.5'
aExpr[17] = '3.9'
aExpr[18] = '4'
aExpr[19] = '22'
aExpr[20] = '-0.49'
aExpr[21] = '-0.5'
aExpr[22] = '-0.9'
aExpr[23] = '-1'
aExpr[24] = '-1.9'
aExpr[25] = '2147483647'
aExpr[26] = '2147483647.1'
aExpr[27] = '2147483647.5'
aExpr[28] = '2147483647.9'
aExpr[29] = '2147483648'
aExpr[30] = '-2147483648'
aExpr[31] = '-2147483648.9'
aExpr[32] = '-2147483649'
aExpr[33] = '-2147483649.9'
aExpr[34] = '4294967295'
aExpr[35] = '4294967296'
aExpr[36] = '4294967297'
aExpr[37] = '4294967297.9'
aExpr[38] = '-4294967295'
aExpr[39] = '-4294967295.9'
aExpr[40] = '-4294967294.5'
aExpr[41] = '-4294967293'
aExpr[42] = '-4294967296'
aExpr[43] = '-4294967296.5'
aExpr[44] = '-4294967297'
aExpr[45] = '9007199254740992'
aExpr[46] = '-9007199254740992'
aExpr[47] = '1e20'
aExpr[48] = '-1e20'
aExpr[49] = '1e300'
aExpr[50] = '-1e300'
aExpr[51] = 'EXP(1000)'
aExpr[52] = '-EXP(1000)'
aExpr[53] = '1e-300'
aExpr[54] = '-1e-300'
aExpr[55] = '.T.'
aExpr[56] = '.F.'
aExpr[57] = '.NULL.'
aExpr[58] = "'2.9'"
aExpr[59] = '$2.9'
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
            luIndex = EVALUATE(lcExpr)
            luResult = loList.IndexToItemID(luIndex)
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
lcOut = lcOut + 'DONE|118' + CHR(13) + CHR(10)
=STRTOFILE(lcOut, lcPath, 0)
