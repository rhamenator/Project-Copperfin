LOCAL lcFile, lnHandle, lnCase, lnResult, lnError, lnPosition, luEmpty, loError
LOCAL ARRAY aCases[40, 2]
SET SAFETY OFF
lcFile = ADDBS(SYS(2023)) + SYS(2015) + '-fseek-numeric.dat'
=STRTOFILE('abcdef', lcFile)
lnHandle = FOPEN(lcFile, 2)
aCases[1, 1] = 'zero'
aCases[1, 2] = 0
aCases[2, 1] = 'fraction'
aCases[2, 2] = 1.9
aCases[3, 1] = 'negative_fraction'
aCases[3, 2] = -0.9
aCases[4, 1] = 'negative'
aCases[4, 2] = -1
aCases[5, 1] = 'int32max'
aCases[5, 2] = 2147483647
aCases[6, 1] = 'int32max_fraction'
aCases[6, 2] = 2147483647.9
aCases[7, 1] = 'int32overflow'
aCases[7, 2] = 2147483648
aCases[8, 1] = 'int32min'
aCases[8, 2] = -2147483648
aCases[9, 1] = 'int32underflow'
aCases[9, 2] = -2147483649
aCases[10, 1] = 'uint32wrap'
aCases[10, 2] = 4294967296
aCases[11, 1] = 'uint32wrap_plus1'
aCases[11, 2] = 4294967297
aCases[12, 1] = 'negative_uint32wrap'
aCases[12, 2] = -4294967296
aCases[13, 1] = 'huge'
aCases[13, 2] = 1E300
aCases[14, 1] = 'negative_huge'
aCases[14, 2] = -1E300
aCases[15, 1] = 'infinity'
aCases[15, 2] = EXP(1000)
aCases[16, 1] = 'negative_infinity'
aCases[16, 2] = -EXP(1000)
aCases[17, 1] = 'currency_subunit'
aCases[17, 2] = $0.9000
aCases[18, 1] = 'currency_fraction'
aCases[18, 2] = $1.9000
aCases[19, 1] = 'currency_negative_subunit'
aCases[19, 2] = $-0.9000
aCases[20, 1] = 'currency_wrap'
aCases[20, 2] = $4294967297.0000
aCases[21, 1] = 'logical'
aCases[21, 2] = .T.
aCases[22, 1] = 'character'
aCases[22, 2] = '1'
aCases[23, 1] = 'null'
aCases[23, 2] = .NULL.
aCases[24, 1] = 'empty'
aCases[24, 2] = luEmpty
aCases[25, 1] = 'currency_max'
aCases[25, 2] = $2147483647.9000
aCases[26, 1] = 'currency_overflow'
aCases[26, 2] = $2147483648.0000
aCases[27, 1] = 'currency_min'
aCases[27, 2] = $-2147483648.0000
aCases[28, 1] = 'currency_underflow'
aCases[28, 2] = $-2147483649.0000
aCases[29, 1] = 'currency_zero'
aCases[29, 2] = $0.0000
aCases[30, 1] = 'numeric_two'
aCases[30, 2] = 2
aCases[31, 1] = 'numeric_two_fraction'
aCases[31, 2] = 2.9
aCases[32, 1] = 'currency_two_fraction'
aCases[32, 2] = $2.9000
aCases[33, 1] = 'numeric_negative_wrap_plus1'
aCases[33, 2] = -4294967295
aCases[34, 1] = 'currency_negative_wrap'
aCases[34, 2] = $-4294967296.0000
aCases[35, 1] = 'currency_uint32max'
aCases[35, 2] = $4294967295.9000
aCases[36, 1] = 'currency_uint32limit'
aCases[36, 2] = $4294967296.0000
aCases[37, 1] = 'currency_negative_uint32limit_minus1'
aCases[37, 2] = $-4294967297.0000
aCases[38, 1] = 'currency_large'
aCases[38, 2] = $922337203685477.0000
aCases[39, 1] = 'currency_negative_large'
aCases[39, 2] = $-922337203685477.0000
aCases[40, 1] = 'currency_negative_limit_fraction'
aCases[40, 2] = $-4294967296.9000
FOR lnCase = 1 TO ALEN(aCases, 1)
    =FSEEK(lnHandle, 3, 0)
    TRY
        lnResult = FSEEK(lnHandle, aCases[lnCase, 2], 0)
        lnError = FERROR()
        lnPosition = FSEEK(lnHandle, 0, 1)
        ? 'offset_' + aCases[lnCase, 1] + '=' + TRANSFORM(lnResult) + ',FERR' + TRANSFORM(lnError) + ',POS' + TRANSFORM(lnPosition)
    CATCH TO loError
        ? 'offset_' + aCases[lnCase, 1] + '=ERR' + TRANSFORM(loError.ErrorNo) + ',POS' + TRANSFORM(FSEEK(lnHandle, 0, 1))
    ENDTRY
ENDFOR
FOR lnCase = 1 TO ALEN(aCases, 1)
    =FSEEK(lnHandle, 3, 0)
    TRY
        lnResult = FSEEK(lnHandle, 1, aCases[lnCase, 2])
        lnError = FERROR()
        lnPosition = FSEEK(lnHandle, 0, 1)
        ? 'origin_' + aCases[lnCase, 1] + '=' + TRANSFORM(lnResult) + ',FERR' + TRANSFORM(lnError) + ',POS' + TRANSFORM(lnPosition)
    CATCH TO loError
        ? 'origin_' + aCases[lnCase, 1] + '=ERR' + TRANSFORM(loError.ErrorNo) + ',POS' + TRANSFORM(FSEEK(lnHandle, 0, 1))
    ENDTRY
ENDFOR
TRY
    lnResult = FSEEK(-999, .T., 0)
    ? 'bad_handle_bad_offset=' + TRANSFORM(lnResult) + ',FERR' + TRANSFORM(FERROR())
CATCH TO loError
    ? 'bad_handle_bad_offset=ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
TRY
    lnResult = FSEEK(-999, 0, .T.)
    ? 'bad_handle_bad_origin=' + TRANSFORM(lnResult) + ',FERR' + TRANSFORM(FERROR())
CATCH TO loError
    ? 'bad_handle_bad_origin=ERR' + TRANSFORM(loError.ErrorNo)
ENDTRY
=FCLOSE(lnHandle)
ERASE (lcFile)
