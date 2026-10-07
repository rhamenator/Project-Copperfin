* RQ-CF-PRG-DESCENDING-ORDINAL-NUMERIC-001: clean-room installed observations.
LOCAL lcRoot, lcTable, lcIndex, lnIndex, lnForm, lcExpression, luResult, loError
LOCAL ARRAY laArgs[68]
lcRoot = ADDBS(SYS(2023)) + SYS(2015)
MD (lcRoot)
lcTable = ADDBS(lcRoot) + 'descprobe.dbf'
CREATE TABLE (lcTable) FREE (id I, marker C(8))
INSERT INTO descprobe VALUES (31,'guard')
INDEX ON id TAG FIRST
INDEX ON marker TAG SECOND DESCENDING
INDEX ON -id TAG THIRD
SET ORDER TO TAG FIRST ASCENDING
lcIndex = CDX(1)
? 'VERSION|' + VERSION()
? 'BEFORE|' + ORDER() + ':' + TRANSFORM(RECCOUNT()) + ':' + TRANSFORM(RECNO()) + ':' + TRANSFORM(id) + ':' + marker + ':' + TRANSFORM(SET('DATASESSION')) + ':' + TRANSFORM(DESCENDING())
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
laArgs[58] = "-2.9"
laArgs[59] = "3.9"
laArgs[60] = "4"
laArgs[61] = "65539"
laArgs[62] = "32770"
laArgs[63] = "32766"
laArgs[64] = "32767"
laArgs[65] = "32767.9"
laArgs[66] = "-4294934530"
laArgs[67] = "-4294934529"
laArgs[68] = "-4294934528"
FOR lnForm = 1 TO 2
    FOR lnIndex = 1 TO ALEN(laArgs,1)
        IF lnForm = 1
            lcExpression = 'DESCENDING(lcIndex,' + laArgs[lnIndex] + ')'
        ELSE
            lcExpression = "DESCENDING(lcIndex," + laArgs[lnIndex] + ",'descprobe')"
        ENDIF
        TRY
            luResult = EVALUATE(lcExpression)
            ? TRANSFORM(lnForm) + '|' + laArgs[lnIndex] + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
        CATCH TO loError
            ? TRANSFORM(lnForm) + '|' + laArgs[lnIndex] + '|ERR' + TRANSFORM(loError.ErrorNo)
        ENDTRY
    ENDFOR
ENDFOR
FOR lnForm = 1 TO 2
    lcExpression = IIF(lnForm = 1, 'DESCENDING()', "DESCENDING('descprobe')")
    TRY
        luResult = EVALUATE(lcExpression)
        ? 'OMITTED|' + TRANSFORM(lnForm) + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']'
    CATCH TO loError
        ? 'OMITTED|' + TRANSFORM(lnForm) + '|ERR' + TRANSFORM(loError.ErrorNo)
    ENDTRY
ENDFOR
? 'AFTER|' + ORDER() + ':' + TRANSFORM(RECCOUNT()) + ':' + TRANSFORM(RECNO()) + ':' + TRANSFORM(id) + ':' + marker + ':' + TRANSFORM(SET('DATASESSION')) + ':' + TRANSFORM(DESCENDING())
USE IN descprobe
ERASE (lcTable)
ERASE (FORCEEXT(lcTable,'cdx'))
RD (lcRoot)
? 'CLEANUP|' + TRANSFORM(DIRECTORY(lcRoot))
