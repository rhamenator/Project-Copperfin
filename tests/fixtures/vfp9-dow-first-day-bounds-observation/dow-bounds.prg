SET CENTURY ON
SET DATE TO YMD
SET FDOW TO 4

DIMENSION aValues[23,2]
aValues[1,1] = "omitted"
aValues[1,2] = ""
aValues[2,1] = "zero-uses-fdow"
aValues[2,2] = "0"
aValues[3,1] = "one"
aValues[3,2] = "1"
aValues[4,1] = "seven"
aValues[4,2] = "7"
aValues[5,1] = "eight"
aValues[5,2] = "8"
aValues[6,1] = "negative-one"
aValues[6,2] = "-1"
aValues[7,1] = "fraction-1.4"
aValues[7,2] = "1.4"
aValues[8,1] = "fraction-1.5"
aValues[8,2] = "1.5"
aValues[9,1] = "fraction-1.9"
aValues[9,2] = "1.9"
aValues[10,1] = "negative-fraction"
aValues[10,2] = "-0.9"
aValues[11,1] = "huge-positive"
aValues[11,2] = "1E20"
aValues[12,1] = "huge-negative"
aValues[12,2] = "-1E20"
aValues[13,1] = "int-max"
aValues[13,2] = "2147483647"
aValues[14,1] = "int-max-plus-one"
aValues[14,2] = "2147483648"
aValues[15,1] = "int-min"
aValues[15,2] = "-2147483648"
aValues[16,1] = "int-min-minus-one"
aValues[16,2] = "-2147483649"
aValues[17,1] = "uint32-max"
aValues[17,2] = "4294967295"
aValues[18,1] = "uint32-wrap-zero"
aValues[18,2] = "4294967296"
aValues[19,1] = "negative-wrap-one"
aValues[19,2] = "-4294967295"
aValues[20,1] = "negative-wrap-zero"
aValues[20,2] = "-4294967296"
aValues[21,1] = "positive-infinity"
aValues[21,2] = "EXP(1000)"
aValues[22,1] = "negative-infinity"
aValues[22,2] = "-EXP(1000)"
aValues[23,1] = "large-exact-double"
aValues[23,2] = "9007199254740992"

FOR nCase = 1 TO ALEN(aValues, 1)
    IF EMPTY(aValues[nCase,2])
        cExpression = "DOW(DATE(2026,1,7))"
    ELSE
        cExpression = "DOW(DATE(2026,1,7)," + aValues[nCase,2] + ")"
    ENDIF
    TRY
        uResult = EVALUATE(cExpression)
        ? aValues[nCase,1] + " => N:[" + TRANSFORM(uResult) + "]"
    CATCH TO oError
        ? aValues[nCase,1] + " => ERR " + TRANSFORM(oError.ErrorNo) + " :" + oError.Message
    ENDTRY
ENDFOR
