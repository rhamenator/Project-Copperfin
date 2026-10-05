LOCAL lcFile, lnHandle, lnCase, lnWrite, lnFileError, loError
LOCAL ARRAY aCases[24, 2]
lcFile = ADDBS(SYS(2023)) + "copperfin-fopen-mode-probe.dat"
SET SAFETY OFF

aCases[1, 1] = "numeric_0"
aCases[1, 2] = 0
aCases[2, 1] = "numeric_0_9"
aCases[2, 2] = 0.9
aCases[3, 1] = "numeric_1"
aCases[3, 2] = 1
aCases[4, 1] = "numeric_1_9"
aCases[4, 2] = 1.9
aCases[5, 1] = "numeric_2"
aCases[5, 2] = 2
aCases[6, 1] = "numeric_2_9"
aCases[6, 2] = 2.9
aCases[7, 1] = "numeric_10"
aCases[7, 2] = 10
aCases[8, 1] = "numeric_11"
aCases[8, 2] = 11
aCases[9, 1] = "numeric_12"
aCases[9, 2] = 12
aCases[10, 1] = "numeric_13"
aCases[10, 2] = 13
aCases[11, 1] = "numeric_neg0_9"
aCases[11, 2] = -0.9
aCases[12, 1] = "numeric_neg1"
aCases[12, 2] = -1
aCases[13, 1] = "numeric_neg_uint32wrap"
aCases[13, 2] = -4294967296
aCases[14, 1] = "numeric_neg_uint32wrap_plus1"
aCases[14, 2] = -4294967295
aCases[15, 1] = "numeric_uint32wrap"
aCases[15, 2] = 4294967296
aCases[16, 1] = "numeric_huge"
aCases[16, 2] = 1E300
aCases[17, 1] = "numeric_neg_huge"
aCases[17, 2] = -1E300
aCases[18, 1] = "numeric_inf"
aCases[18, 2] = EXP(1000)
aCases[19, 1] = "numeric_neg_inf"
aCases[19, 2] = -EXP(1000)
aCases[20, 1] = "currency_0_9"
aCases[20, 2] = $0.9000
aCases[21, 1] = "currency_1_9"
aCases[21, 2] = $1.9000
aCases[22, 1] = "currency_neg0_9"
aCases[22, 2] = $-0.9000
aCases[23, 1] = "currency_uint32wrap"
aCases[23, 2] = $4294967296.0000
aCases[24, 1] = "currency_neg_uint32wrap"
aCases[24, 2] = $-4294967296.0000

FOR lnCase = 1 TO ALEN(aCases, 1)
    = STRTOFILE("seed", lcFile, 0)
    TRY
        lnHandle = FOPEN(lcFile, aCases[lnCase, 2])
        IF lnHandle >= 0
            lnWrite = FWRITE(lnHandle, "x")
            lnFileError = FERROR()
            = FCLOSE(lnHandle)
            ? "fopen_" + aCases[lnCase, 1] + "=OPEN,WRITE" + TRANSFORM(lnWrite) + ",FERR" + TRANSFORM(lnFileError)
        ELSE
            ? "fopen_" + aCases[lnCase, 1] + "=NEG1,FERR" + TRANSFORM(FERROR())
        ENDIF
    CATCH TO loError
        ? "fopen_" + aCases[lnCase, 1] + "=ERR" + TRANSFORM(loError.ErrorNo)
    ENDTRY
ENDFOR

ERASE (lcFile)
