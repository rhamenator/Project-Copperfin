* Independent installed-VFP9 observation; no Copperfin helper expectations.
LOCAL lcOut, lcClass, lcExpr, lnClass, lnCase, loList, luIndex, luResult, loError
LOCAL lcStatus, lcKind, lcValue, lcPath
LOCAL ARRAY aExpr[48]
lcPath = 'Z:\home\rich\.codex\worktrees\itemdata-index-numeric-5611\Project-Copperfin\build\itemdata-native\native.out'
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
aExpr[1] = '0'
aExpr[2] = '0.49'
aExpr[3] = '0.5'
aExpr[4] = '0.9'
aExpr[5] = '1'
aExpr[6] = '1.49'
aExpr[7] = '1.5'
aExpr[8] = '1.9'
aExpr[9] = '2'
aExpr[10] = '2.49'
aExpr[11] = '2.5'
aExpr[12] = '2.9'
aExpr[13] = '3'
aExpr[14] = '3.49'
aExpr[15] = '3.5'
aExpr[16] = '3.9'
aExpr[17] = '4'
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
aExpr[29] = '4294967297'
aExpr[30] = '4294967297.9'
aExpr[31] = '4294967298'
aExpr[32] = '4294967299'
aExpr[33] = '-4294967295'
aExpr[34] = '-4294967294'
aExpr[35] = '-4294967293'
aExpr[36] = '1e20'
aExpr[37] = '-1e20'
aExpr[38] = '1e300'
aExpr[39] = '-1e300'
aExpr[40] = 'EXP(1000)'
aExpr[41] = '-EXP(1000)'
aExpr[42] = '.T.'
aExpr[43] = '.F.'
aExpr[44] = '.NULL.'
aExpr[45] = "'1.9'"
aExpr[46] = '$1.9'
aExpr[47] = '1e-300'
aExpr[48] = '-1e-300'
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
            luResult = loList.ItemData(luIndex)
            lcKind = VARTYPE(luResult)
            lcValue = TRANSFORM(luResult)
        CATCH TO loError
            lcStatus = 'ERR' + TRANSFORM(loError.ErrorNo)
        ENDTRY
        lcOut = lcOut + lcClass + '|' + lcExpr + '|' + lcStatus + '|' + lcKind + '|' + lcValue + '|'
        lcOut = lcOut + TRANSFORM(loList.ListCount) + '|' + TRANSFORM(loList.ListIndex) + '|'
        lcOut = lcOut + TRANSFORM(loList.ItemData(1)) + ',' + TRANSFORM(loList.ItemData(2)) + ',' + TRANSFORM(loList.ItemData(3)) + CHR(13) + CHR(10)
        loList = .NULL.
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE|96' + CHR(13) + CHR(10)
=STRTOFILE(lcOut, lcPath, 0)
