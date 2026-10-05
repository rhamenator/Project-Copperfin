LOCAL lcFile, lnHandle, lnCase, lnFunction, lcFunction, lcExpression, luArgument, luResult, lnError, loError, luEmpty
LOCAL ARRAY aCases[38, 2], aFunctions[10]
SET SAFETY OFF
lcFile = ADDBS(SYS(2023)) + SYS(2015) + '-file-handle-numeric.dat'
aCases[1, 1] = 'exact'
aCases[1, 2] = "lnHandle"
aCases[2, 1] = 'fraction'
aCases[2, 2] = "lnHandle + 0.9"
aCases[3, 1] = 'fraction_below'
aCases[3, 2] = "lnHandle - 0.1"
aCases[4, 1] = 'negative_fraction'
aCases[4, 2] = "-0.9"
aCases[5, 1] = 'negative'
aCases[5, 2] = "-999"
aCases[6, 1] = 'zero'
aCases[6, 2] = "0"
aCases[7, 1] = 'int32max'
aCases[7, 2] = "2147483647"
aCases[8, 1] = 'int32overflow'
aCases[8, 2] = "2147483648"
aCases[9, 1] = 'int32min'
aCases[9, 2] = "-2147483648"
aCases[10, 1] = 'int32underflow'
aCases[10, 2] = "-2147483649"
aCases[11, 1] = 'uint16_alias'
aCases[11, 2] = "lnHandle + 65536"
aCases[12, 1] = 'uint32_alias'
aCases[12, 2] = "lnHandle + 4294967296"
aCases[13, 1] = 'negative_uint32_alias'
aCases[13, 2] = "lnHandle - 4294967296"
aCases[14, 1] = 'huge'
aCases[14, 2] = "1E300"
aCases[15, 1] = 'negative_huge'
aCases[15, 2] = "-1E300"
aCases[16, 1] = 'infinity'
aCases[16, 2] = "EXP(1000)"
aCases[17, 1] = 'negative_infinity'
aCases[17, 2] = "-EXP(1000)"
aCases[18, 1] = 'currency_exact'
aCases[18, 2] = "NTOM(lnHandle)"
aCases[19, 1] = 'currency_fraction'
aCases[19, 2] = "NTOM(lnHandle + 0.9)"
aCases[20, 1] = 'currency_subunit'
aCases[20, 2] = "$0.9000"
aCases[21, 1] = 'currency_negative_subunit'
aCases[21, 2] = "$-0.9000"
aCases[22, 1] = 'currency_uint16_alias'
aCases[22, 2] = "NTOM(lnHandle + 65536)"
aCases[23, 1] = 'currency_uint32_alias'
aCases[23, 2] = "NTOM(lnHandle + 4294967296)"
aCases[24, 1] = 'currency_negative_uint32_alias'
aCases[24, 2] = "NTOM(lnHandle - 4294967296)"
aCases[25, 1] = 'currency_int32max'
aCases[25, 2] = "$2147483647.9000"
aCases[26, 1] = 'currency_int32overflow'
aCases[26, 2] = "$2147483648"
aCases[27, 1] = 'currency_int32min'
aCases[27, 2] = "$-2147483648"
aCases[28, 1] = 'currency_uint32max'
aCases[28, 2] = "$4294967295.9000"
aCases[29, 1] = 'currency_negative_limit'
aCases[29, 2] = "$-4294967296.9000"
aCases[30, 1] = 'currency_negative_overflow'
aCases[30, 2] = "$-4294967297"
aCases[31, 1] = 'currency_huge'
aCases[31, 2] = "$922337203685477.0000"
aCases[32, 1] = 'logical'
aCases[32, 2] = ".T."
aCases[33, 1] = 'character'
aCases[33, 2] = "TRANSFORM(lnHandle)"
aCases[34, 1] = 'null'
aCases[34, 2] = ".NULL."
aCases[35, 1] = 'empty'
aCases[35, 2] = "luEmpty"
aCases[36, 1] = 'currency_negative_double_wrap_alias'
aCases[36, 2] = "NTOM(lnHandle - 8589934592)"
aCases[37, 1] = 'numeric_negative_double_wrap_alias'
aCases[37, 2] = "lnHandle - 8589934592"
aCases[38, 1] = 'currency_positive_double_wrap_alias'
aCases[38, 2] = "NTOM(lnHandle + 8589934592)"
aFunctions[1] = 'FCLOSE'
aFunctions[2] = 'FREAD'
aFunctions[3] = 'FWRITE'
aFunctions[4] = 'FGETS'
aFunctions[5] = 'FPUTS'
aFunctions[6] = 'FSEEK'
aFunctions[7] = 'FEOF'
aFunctions[8] = 'FFLUSH'
aFunctions[9] = 'FTELL'
aFunctions[10] = 'FCHSIZE'
FOR lnFunction = 1 TO ALEN(aFunctions, 1)
    lcFunction = aFunctions[lnFunction]
    FOR lnCase = 1 TO ALEN(aCases, 1)
        =STRTOFILE('abcdef', lcFile)
        lnHandle = FOPEN(lcFile, 2)
        =FSEEK(lnHandle, 3, 0)
        luArgument = EVALUATE(aCases[lnCase, 2])
        DO CASE
        CASE lcFunction = 'FREAD' OR lcFunction = 'FGETS'
            lcExpression = lcFunction + '(luArgument, 1)'
        CASE lcFunction = 'FWRITE' OR lcFunction = 'FPUTS'
            lcExpression = lcFunction + "(luArgument, 'Z')"
        CASE lcFunction = 'FSEEK'
            lcExpression = 'FSEEK(luArgument, 1, 0)'
        CASE lcFunction = 'FCHSIZE'
            lcExpression = 'FCHSIZE(luArgument, 3)'
        OTHERWISE
            lcExpression = lcFunction + '(luArgument)'
        ENDCASE
        TRY
            luResult = EVALUATE(lcExpression)
            lnError = FERROR()
            ? lcFunction + '_' + aCases[lnCase, 1] + '=' + TRANSFORM(luResult) + ',FERR' + TRANSFORM(lnError)
        CATCH TO loError
            ? lcFunction + '_' + aCases[lnCase, 1] + '=ERR' + TRANSFORM(loError.ErrorNo)
        ENDTRY
        =FCLOSE(lnHandle)
        ERASE (lcFile)
    ENDFOR
ENDFOR
