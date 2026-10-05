LOCAL lcFile, lnHandle, lnCase, luEmpty, luResult, loError
LOCAL ARRAY aCases[24, 2]
lcFile = ADDBS(SYS(2023)) + "copperfin-fdate-type-probe.dat"
lnHandle = FCREATE(lcFile)
= FPUTS(lnHandle, "probe")
= FCLOSE(lnHandle)

aCases[1, 1] = "numeric_0"
aCases[1, 2] = 0
aCases[2, 1] = "numeric_0_9"
aCases[2, 2] = 0.9
aCases[3, 1] = "numeric_1"
aCases[3, 2] = 1
aCases[4, 1] = "numeric_1_0001"
aCases[4, 2] = 1.0001
aCases[5, 1] = "numeric_neg0_9"
aCases[5, 2] = -0.9
aCases[6, 1] = "numeric_neg1"
aCases[6, 2] = -1
aCases[7, 1] = "numeric_neg_uint32wrap"
aCases[7, 2] = -4294967296
aCases[8, 1] = "numeric_huge"
aCases[8, 2] = 1E300
aCases[9, 1] = "numeric_neg_huge"
aCases[9, 2] = -1E300
aCases[10, 1] = "numeric_inf"
aCases[10, 2] = EXP(1000)
aCases[11, 1] = "numeric_neg_inf"
aCases[11, 2] = -EXP(1000)
aCases[12, 1] = "currency_0"
aCases[12, 2] = $0.0000
aCases[13, 1] = "currency_0_9"
aCases[13, 2] = $0.9000
aCases[14, 1] = "currency_1"
aCases[14, 2] = $1.0000
aCases[15, 1] = "currency_1_9"
aCases[15, 2] = $1.9000
aCases[16, 1] = "currency_2"
aCases[16, 2] = $2.0000
aCases[17, 1] = "currency_neg0_9"
aCases[17, 2] = $-0.9000
aCases[18, 1] = "currency_neg_uint32wrap"
aCases[18, 2] = $-4294967296.0000
aCases[19, 1] = "currency_uint32wrap"
aCases[19, 2] = $4294967296.0000
aCases[20, 1] = "currency_uint32wrap_plus1"
aCases[20, 2] = $4294967297.0000
aCases[21, 1] = "logical_true"
aCases[21, 2] = .T.
aCases[22, 1] = "character_one"
aCases[22, 2] = "1"
aCases[23, 1] = "empty"
aCases[23, 2] = luEmpty
aCases[24, 1] = "null"
aCases[24, 2] = .NULL.

FOR lnCase = 1 TO ALEN(aCases, 1)
    TRY
        luResult = FDATE(lcFile, aCases[lnCase, 2])
        ? "fdate_" + aCases[lnCase, 1] + "=" + VARTYPE(luResult)
    CATCH TO loError
        ? "fdate_" + aCases[lnCase, 1] + "=ERR" + TRANSFORM(loError.ErrorNo)
    ENDTRY
ENDFOR

ERASE (lcFile)
