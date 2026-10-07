LOCAL lcExpression, luResult, loError, lnIndex, hLen
LOCAL ARRAY laArgs[58]
? 'VERSION|' + VERSION()
SET LIBRARY TO 'C:\Program Files (x86)\Microsoft Visual FoxPro 9\foxtools.fll'
? 'FOXTOOLS|' + FOXTOOLVER()
hLen = REGFN32('lstrlenA', 'C', 'I', 'kernel32.dll')
? 'HANDLE|' + TRANSFORM(hLen)
? 'CONTROL|' + TRANSFORM(CALLFN(hLen, 'Copperfin'))
laArgs[1] = "0"
laArgs[2] = "1"
laArgs[3] = "2"
laArgs[4] = "-1"
laArgs[5] = "0.49"
laArgs[6] = "0.5"
laArgs[7] = "0.9"
laArgs[8] = "1.49"
laArgs[9] = "1.5"
laArgs[10] = "1.9"
laArgs[11] = "-0.49"
laArgs[12] = "-0.5"
laArgs[13] = "-0.9"
laArgs[14] = "-1.1"
laArgs[15] = "2147483647"
laArgs[16] = "2147483648"
laArgs[17] = "-2147483648"
laArgs[18] = "-2147483649"
laArgs[19] = "4294967295"
laArgs[20] = "4294967296"
laArgs[21] = "4294967297"
laArgs[22] = "4294967298"
laArgs[23] = "-4294967296"
laArgs[24] = "-4294967295"
laArgs[25] = "-4294967294"
laArgs[26] = "4294967296.9"
laArgs[27] = "-4294967295.9"
laArgs[28] = "1E20"
laArgs[29] = "-1E20"
laArgs[30] = "1E300"
laArgs[31] = "-1E300"
laArgs[32] = "EXP(1000)"
laArgs[33] = "-EXP(1000)"
laArgs[34] = "9007199254740992"
laArgs[35] = "32767"
laArgs[36] = "32768"
laArgs[37] = "65535"
laArgs[38] = "65536"
laArgs[39] = "65537"
laArgs[40] = "-65535"
laArgs[41] = "-65536"
laArgs[42] = "$0.5"
laArgs[43] = "$1.5"
laArgs[44] = ".T."
laArgs[45] = ".F."
laArgs[46] = ".NULL."
laArgs[47] = "'0'"
laArgs[48] = "'1'"
laArgs[49] = "65536.9"
laArgs[50] = "65537.9"
laArgs[51] = "-65535.9"
laArgs[52] = "-65536.9"
laArgs[53] = "2147483649"
laArgs[54] = "-2147483647"
laArgs[55] = "65538"
laArgs[56] = "2.9"
laArgs[57] = "32769"
laArgs[58] = "-32767"
FOR lnIndex = 1 TO ALEN(laArgs, 1)
    lcExpression = 'CALLFN(' + laArgs[lnIndex] + ", 'Copperfin')"
    TRY
        luResult = EVALUATE(lcExpression)
        ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
    CATCH TO loError
        ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
        IF lnIndex = 1
            ? 'FIRST_ERROR|' + loError.Message
        ENDIF
    ENDTRY
ENDFOR
? 'RETAINED|' + TRANSFORM(CALLFN(hLen, 'Copperfin'))
SET LIBRARY TO
? 'SESSION|' + TRANSFORM(SET('DATASESSION'))
