LOCAL lcFile, lcPrefix, lnHandle, lnCase, lnWrite, lnFileError, luEmpty, loError
LOCAL ARRAY aCases[25, 2]
SET SAFETY OFF
lcPrefix = SYS(2015)

aCases[1, 1] = "numeric_0"
aCases[1, 2] = 0
aCases[2, 1] = "numeric_0_9"
aCases[2, 2] = 0.9
aCases[3, 1] = "numeric_1"
aCases[3, 2] = 1
aCases[4, 1] = "numeric_1_9"
aCases[4, 2] = 1.9
aCases[5, 1] = "numeric_7"
aCases[5, 2] = 7
aCases[6, 1] = "numeric_7_9"
aCases[6, 2] = 7.9
aCases[7, 1] = "numeric_8"
aCases[7, 2] = 8
aCases[8, 1] = "numeric_neg0_9"
aCases[8, 2] = -0.9
aCases[9, 1] = "numeric_neg1"
aCases[9, 2] = -1
aCases[10, 1] = "numeric_neg_uint32wrap"
aCases[10, 2] = -4294967296
aCases[11, 1] = "numeric_neg_uint32wrap_plus1"
aCases[11, 2] = -4294967295
aCases[12, 1] = "numeric_uint32wrap"
aCases[12, 2] = 4294967296
aCases[13, 1] = "numeric_huge"
aCases[13, 2] = 1E300
aCases[14, 1] = "numeric_neg_huge"
aCases[14, 2] = -1E300
aCases[15, 1] = "numeric_inf"
aCases[15, 2] = EXP(1000)
aCases[16, 1] = "numeric_neg_inf"
aCases[16, 2] = -EXP(1000)
aCases[17, 1] = "currency_0_9"
aCases[17, 2] = $0.9000
aCases[18, 1] = "currency_1_9"
aCases[18, 2] = $1.9000
aCases[19, 1] = "currency_neg0_9"
aCases[19, 2] = $-0.9000
aCases[20, 1] = "currency_uint32wrap"
aCases[20, 2] = $4294967296.0000
aCases[21, 1] = "currency_neg_uint32wrap"
aCases[21, 2] = $-4294967296.0000
aCases[22, 1] = "logical_true"
aCases[22, 2] = .T.
aCases[23, 1] = "character_one"
aCases[23, 2] = "1"
aCases[24, 1] = "null"
aCases[24, 2] = .NULL.
aCases[25, 1] = "empty"
aCases[25, 2] = luEmpty

FOR lnCase = 1 TO ALEN(aCases, 1)
    lcFile = ADDBS(SYS(2023)) + lcPrefix + "-fcreate-attribute-" + TRANSFORM(lnCase) + ".dat"
    TRY
        lnHandle = FCREATE(lcFile, aCases[lnCase, 2])
        IF lnHandle >= 0
            lnWrite = FWRITE(lnHandle, "x")
            lnFileError = FERROR()
            = FCLOSE(lnHandle)
            ? "fcreate_" + aCases[lnCase, 1] + "=OPEN,WRITE" + TRANSFORM(lnWrite) + ",FERR" + TRANSFORM(lnFileError)
        ELSE
            ? "fcreate_" + aCases[lnCase, 1] + "=NEG1,FERR" + TRANSFORM(FERROR())
        ENDIF
    CATCH TO loError
        ? "fcreate_" + aCases[lnCase, 1] + "=ERR" + TRANSFORM(loError.ErrorNo)
    ENDTRY
ENDFOR
