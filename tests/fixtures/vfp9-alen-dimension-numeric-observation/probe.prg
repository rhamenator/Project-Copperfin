LOCAL lnIndex, lnShape, lcArray, lcExpression, luResult, loError
LOCAL ARRAY laArgs[57], laTwo[3,4], laOne[5]
laTwo = 101
laTwo[2,3] = 789
laOne = 31
laOne[5] = 73
? 'VERSION|' + VERSION()
? 'BEFORE|' + TRANSFORM(ALEN(laTwo)) + ':' + TRANSFORM(ALEN(laTwo,1)) + ':' + TRANSFORM(ALEN(laTwo,2)) + ':' + TRANSFORM(ALEN(laOne)) + ':' + TRANSFORM(ALEN(laOne,1)) + ':' + TRANSFORM(ALEN(laOne,2))
laArgs[1] = "0"
laArgs[2] = "1"
laArgs[3] = "2"
laArgs[4] = "-1"
laArgs[5] = "3"
laArgs[6] = "0.49"
laArgs[7] = "0.5"
laArgs[8] = "0.9"
laArgs[9] = "1.49"
laArgs[10] = "1.5"
laArgs[11] = "1.9"
laArgs[12] = "2.49"
laArgs[13] = "2.5"
laArgs[14] = "2.9"
laArgs[15] = "3.1"
laArgs[16] = "-0.49"
laArgs[17] = "-0.5"
laArgs[18] = "-0.9"
laArgs[19] = "-1.1"
laArgs[20] = "2147483647"
laArgs[21] = "2147483648"
laArgs[22] = "-2147483648"
laArgs[23] = "-2147483649"
laArgs[24] = "4294967295"
laArgs[25] = "4294967296"
laArgs[26] = "4294967297"
laArgs[27] = "4294967298"
laArgs[28] = "4294967299"
laArgs[29] = "-4294967296"
laArgs[30] = "-4294967295"
laArgs[31] = "-4294967294"
laArgs[32] = "1E20"
laArgs[33] = "-1E20"
laArgs[34] = "1E300"
laArgs[35] = "-1E300"
laArgs[36] = "EXP(1000)"
laArgs[37] = "-EXP(1000)"
laArgs[38] = "9007199254740992"
laArgs[39] = "65536"
laArgs[40] = "65537"
laArgs[41] = "65538"
laArgs[42] = "-65535"
laArgs[43] = "-65534"
laArgs[44] = "32768"
laArgs[45] = "32769"
laArgs[46] = "-32767"
laArgs[47] = "4294967296.9"
laArgs[48] = "-4294967295.9"
laArgs[49] = "$0.5"
laArgs[50] = "$1.5"
laArgs[51] = "$2.5"
laArgs[52] = ".T."
laArgs[53] = ".F."
laArgs[54] = ".NULL."
laArgs[55] = "'0'"
laArgs[56] = "'1'"
laArgs[57] = "'2'"
FOR lnShape = 1 TO 2
    lcArray = IIF(lnShape = 1, 'laTwo', 'laOne')
    FOR lnIndex = 1 TO ALEN(laArgs,1)
        lcExpression = 'ALEN(' + lcArray + ',' + laArgs[lnIndex] + ')'
        TRY
            luResult = EVALUATE(lcExpression)
            ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
        CATCH TO loError
            ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo)
        ENDTRY
    ENDFOR
ENDFOR
? 'AFTER|' + TRANSFORM(ALEN(laTwo)) + ':' + TRANSFORM(ALEN(laTwo,1)) + ':' + TRANSFORM(ALEN(laTwo,2)) + ':' + TRANSFORM(ALEN(laOne)) + ':' + TRANSFORM(ALEN(laOne,1)) + ':' + TRANSFORM(ALEN(laOne,2))
? 'MARKERS|' + TRANSFORM(laTwo[2,3]) + ':' + TRANSFORM(laOne[5])
? 'SESSION|' + TRANSFORM(SET('DATASESSION'))
