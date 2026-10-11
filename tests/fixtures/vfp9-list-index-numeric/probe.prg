* Independent installed-VFP9 List one-argument display-row READ observation.
LOCAL lcOut, lcClass, lcShape, lcExpr, lnClass, lnShape, lnCase, loList
LOCAL luID, luResult, loError, lcStatus, lcKind, lcValue, lnCalls
LOCAL ARRAY aExpr[75]
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
lnCalls = 0
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
aExpr[19] = '10.9'
aExpr[20] = '11'
aExpr[21] = '11.49'
aExpr[22] = '11.5'
aExpr[23] = '11.9'
aExpr[24] = '21.9'
aExpr[25] = '22'
aExpr[26] = '22.49'
aExpr[27] = '22.5'
aExpr[28] = '22.9'
aExpr[29] = '33'
aExpr[30] = '33.9'
aExpr[31] = '34'
aExpr[32] = '-0.49'
aExpr[33] = '-0.5'
aExpr[34] = '-0.9'
aExpr[35] = '-1'
aExpr[36] = '-1.9'
aExpr[37] = '2147483647'
aExpr[38] = '2147483647.1'
aExpr[39] = '2147483647.5'
aExpr[40] = '2147483647.9'
aExpr[41] = '2147483648'
aExpr[42] = '-2147483648'
aExpr[43] = '-2147483648.9'
aExpr[44] = '-2147483649'
aExpr[45] = '-2147483649.9'
aExpr[46] = '4294967295'
aExpr[47] = '4294967296'
aExpr[48] = '4294967297'
aExpr[49] = '4294967297.9'
aExpr[50] = '4294967307'
aExpr[51] = '4294967318.9'
aExpr[52] = '-4294967295'
aExpr[53] = '-4294967295.9'
aExpr[54] = '-4294967294.5'
aExpr[55] = '-4294967293'
aExpr[56] = '-4294967285'
aExpr[57] = '-4294967274.9'
aExpr[58] = '-4294967263'
aExpr[59] = '-4294967296'
aExpr[60] = '-4294967296.5'
aExpr[61] = '-4294967297'
aExpr[62] = '9007199254740992'
aExpr[63] = '-9007199254740992'
aExpr[64] = '1e20'
aExpr[65] = '-1e20'
aExpr[66] = '1e300'
aExpr[67] = '-1e300'
aExpr[68] = 'EXP(1000)'
aExpr[69] = '-EXP(1000)'
aExpr[70] = '1e-300'
aExpr[71] = '-1e-300'
aExpr[72] = '.T.'
aExpr[73] = '.NULL.'
aExpr[74] = "'2.9'"
aExpr[75] = '$2.9'
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    FOR lnShape = 1 TO 2
        lcShape = IIF(lnShape = 1, 'sparse', 'dense')
        FOR lnCase = 1 TO ALEN(aExpr)
            loList = CREATEOBJECT(lcClass)
            IF lnShape = 1
                loList.AddListItem('first', 11)
                loList.AddListItem('second', 22)
                loList.AddListItem('third', 33)
            ELSE
                loList.AddItem('first')
                loList.AddItem('second')
                loList.AddItem('third')
            ENDIF
            loList.ItemData(1) = 101
            loList.ItemData(2) = 202
            loList.ItemData(3) = 303
            loList.ListIndex = 2
            lcExpr = aExpr[lnCase]
            lcStatus = 'OK'
            lcKind = '-'
            lcValue = '-'
            TRY
                luID = EVALUATE(lcExpr)
                luResult = loList.List(luID)
                lcKind = VARTYPE(luResult)
                lcValue = TRANSFORM(luResult)
            CATCH TO loError
                lcStatus = 'ERR' + TRANSFORM(loError.ErrorNo)
            ENDTRY
            lnCalls = lnCalls + 1
            lcOut = lcOut + lcClass + '|' + lcShape + '|' + lcExpr + '|' + lcStatus + '|' + lcKind + '|' + lcValue + '|'
            lcOut = lcOut + TRANSFORM(loList.ListCount) + '|' + TRANSFORM(loList.ListIndex) + '|' + TRANSFORM(loList.ListItemID) + '|'
            lcOut = lcOut + TRANSFORM(loList.ItemData(1)) + ',' + TRANSFORM(loList.ItemData(2)) + ',' + TRANSFORM(loList.ItemData(3)) + '|'
            lcOut = lcOut + TRANSFORM(loList.Selected(1)) + ',' + TRANSFORM(loList.Selected(2)) + ',' + TRANSFORM(loList.Selected(3)) + '|'
            lcOut = lcOut + LTRIM(STR(loList.IndexToItemID(1), 20, 0)) + ',' + LTRIM(STR(loList.IndexToItemID(2), 20, 0)) + ',' + LTRIM(STR(loList.IndexToItemID(3), 20, 0)) + CHR(13) + CHR(10)
            loList = .NULL.
        ENDFOR
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE|' + TRANSFORM(lnCalls) + CHR(13) + CHR(10)
=STRTOFILE(lcOut, 'Z:\home\rich\.codex\worktrees\list-index-numeric-5611\Project-Copperfin\build\list-index-native\native.out', 0)
