LOCAL lcFile, lnHandle, lnCase, lnMode, luSize, luResult, lnError, lnPosition, loError, luEmpty
LOCAL ARRAY aCases[41, 3]
SET SAFETY OFF
lcFile = ADDBS(SYS(2023)) + SYS(2015) + '-fchsize-numeric.dat'
aCases[1, 1] = 'zero'
aCases[1, 2] = '0'
aCases[2, 1] = 'exact'
aCases[2, 2] = '3'
aCases[3, 1] = 'extend'
aCases[3, 2] = '8'
aCases[4, 1] = 'fraction'
aCases[4, 2] = '3.9'
aCases[5, 1] = 'subunit'
aCases[5, 2] = '0.9'
aCases[6, 1] = 'negative'
aCases[6, 2] = '-1'
aCases[7, 1] = 'negative_fraction'
aCases[7, 2] = '-3.9'
aCases[8, 1] = 'negative_subunit'
aCases[8, 2] = '-0.9'
aCases[9, 1] = 'int32max'
aCases[9, 2] = '2147483647'
aCases[10, 1] = 'int32overflow'
aCases[10, 2] = '2147483648'
aCases[11, 1] = 'int32min'
aCases[11, 2] = '-2147483648'
aCases[12, 1] = 'int32underflow'
aCases[12, 2] = '-2147483649'
aCases[13, 1] = 'uint32max'
aCases[13, 2] = '4294967295'
aCases[14, 1] = 'uint32wrap'
aCases[14, 2] = '4294967299'
aCases[15, 1] = 'negative_uint32wrap'
aCases[15, 2] = '-4294967293'
aCases[16, 1] = 'double_wrap'
aCases[16, 2] = '8589934595'
aCases[17, 1] = 'negative_double_wrap'
aCases[17, 2] = '-8589934589'
aCases[18, 1] = 'huge'
aCases[18, 2] = '1E300'
aCases[19, 1] = 'negative_huge'
aCases[19, 2] = '-1E300'
aCases[20, 1] = 'infinity'
aCases[20, 2] = 'EXP(1000)'
aCases[21, 1] = 'negative_infinity'
aCases[21, 2] = '-EXP(1000)'
aCases[22, 1] = 'currency_exact'
aCases[22, 2] = '$3'
aCases[23, 1] = 'currency_fraction'
aCases[23, 2] = '$3.9000'
aCases[24, 1] = 'currency_subunit'
aCases[24, 2] = '$0.9000'
aCases[25, 1] = 'currency_negative'
aCases[25, 2] = '$-1'
aCases[26, 1] = 'currency_negative_fraction'
aCases[26, 2] = '$-3.9000'
aCases[27, 1] = 'currency_negative_subunit'
aCases[27, 2] = '$-0.9000'
aCases[28, 1] = 'currency_int32max'
aCases[28, 2] = '$2147483647.9000'
aCases[29, 1] = 'currency_int32overflow'
aCases[29, 2] = '$2147483648'
aCases[30, 1] = 'currency_int32min'
aCases[30, 2] = '$-2147483648'
aCases[31, 1] = 'currency_uint32max'
aCases[31, 2] = '$4294967295.9000'
aCases[32, 1] = 'currency_uint32wrap'
aCases[32, 2] = '$4294967299'
aCases[33, 1] = 'currency_negative_uint32wrap'
aCases[33, 2] = '$-4294967293'
aCases[34, 1] = 'currency_negative_limit'
aCases[34, 2] = '$-4294967296.9000'
aCases[35, 1] = 'currency_negative_overflow'
aCases[35, 2] = '$-4294967297'
aCases[36, 1] = 'currency_huge'
aCases[36, 2] = '$922337203685477.0000'
aCases[37, 1] = 'logical'
aCases[37, 2] = '.T.'
aCases[38, 1] = 'character'
aCases[38, 2] = "'3'"
aCases[39, 1] = 'null'
aCases[39, 2] = '.NULL.'
aCases[40, 1] = 'empty'
aCases[40, 2] = 'luEmpty'
aCases[41, 1] = 'logical_false'
aCases[41, 2] = '.F.'
* Only small values and known negative/no-resize inputs get writable handles.
* Boundary and wrap/huge probes stay read-only to prevent giant file creation.
FOR lnCase = 1 TO ALEN(aCases, 1)
    aCases[lnCase, 3] = lnCase <= 8 OR BETWEEN(lnCase, 22, 27) OR lnCase >= 37
ENDFOR
FOR lnMode = 0 TO 2 STEP 2
    FOR lnCase = 1 TO ALEN(aCases, 1)
        IF lnMode = 2 AND NOT aCases[lnCase, 3]
            LOOP
        ENDIF
        =STRTOFILE('abcdef', lcFile)
        lnHandle = FOPEN(lcFile, lnMode)
        =FSEEK(lnHandle, 3, 0)
        luSize = EVALUATE(aCases[lnCase, 2])
        TRY
            luResult = FCHSIZE(lnHandle, luSize)
            lnError = FERROR()
            ? 'mode' + TRANSFORM(lnMode) + '_' + aCases[lnCase, 1] + '=' + TRANSFORM(luResult) + ',FERR' + TRANSFORM(lnError)
        CATCH TO loError
            ? 'mode' + TRANSFORM(lnMode) + '_' + aCases[lnCase, 1] + '=ERR' + TRANSFORM(loError.ErrorNo)
        ENDTRY
        lnPosition = FSEEK(lnHandle, 0, 1)
        =FCLOSE(lnHandle)
        ? 'state=' + TRANSFORM(lnPosition) + ',length' + TRANSFORM(LEN(FILETOSTR(lcFile)))
        ERASE (lcFile)
    ENDFOR
ENDFOR
* Invalid handle and size combinations expose the validation order safely.
FOR lnCase = 1 TO ALEN(aCases, 1)
    luSize = EVALUATE(aCases[lnCase, 2])
    TRY
        luResult = FCHSIZE(-999, luSize)
        lnError = FERROR()
        ? 'invalid_' + aCases[lnCase, 1] + '=' + TRANSFORM(luResult) + ',FERR' + TRANSFORM(lnError)
    CATCH TO loError
        ? 'invalid_' + aCases[lnCase, 1] + '=ERR' + TRANSFORM(loError.ErrorNo)
    ENDTRY
ENDFOR
