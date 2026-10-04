DIMENSION aCases[16,2]
aCases[1,1] = "negative-point-one"
aCases[1,2] = "RGB(-0.1,0,0)"
aCases[2,1] = "negative-point-nine"
aCases[2,2] = "RGB(-0.9,0,0)"
aCases[3,1] = "negative-one-point-one"
aCases[3,2] = "RGB(-1.1,0,0)"
aCases[4,1] = "wrap-negative-one"
aCases[4,2] = "RGB(-4294967295,0,0)"
aCases[5,1] = "wrap-zero"
aCases[5,2] = "RGB(-4294967296,0,0)"
aCases[6,1] = "wrap-two-five-five"
aCases[6,2] = "RGB(-4294967041,0,0)"
aCases[7,1] = "wrap-two-five-six"
aCases[7,2] = "RGB(-4294967040,0,0)"
aCases[8,1] = "twice-wrap-one"
aCases[8,2] = "RGB(-8589934591,0,0)"
aCases[9,1] = "green-wrap-one"
aCases[9,2] = "RGB(0,-4294967295,0)"
aCases[10,1] = "blue-wrap-one"
aCases[10,2] = "RGB(0,0,-4294967295)"
aCases[11,1] = "all-wrap"
aCases[11,2] = "RGB(-4294967295,-4294967294,-4294967293)"
aCases[12,1] = "upper-exact"
aCases[12,2] = "RGB(255.0,0,0)"
aCases[13,1] = "upper-just-over"
aCases[13,2] = "RGB(255.0001,0,0)"
aCases[14,1] = "positive-two-five-four-nine"
aCases[14,2] = "RGB(254.9999,0,0)"
aCases[15,1] = "negative-wrap-fraction"
aCases[15,2] = "RGB(-4294967295.9,0,0)"
aCases[16,1] = "negative-wrap-fraction-zero"
aCases[16,2] = "RGB(-4294967296.9,0,0)"

FOR nCase = 1 TO ALEN(aCases, 1)
    TRY
        uResult = EVALUATE(aCases[nCase,2])
        ? aCases[nCase,1] + " => N:[" + TRANSFORM(uResult) + "]"
    CATCH TO oError
        ? aCases[nCase,1] + " => ERR " + TRANSFORM(oError.ErrorNo) + " :" + oError.Message
    ENDTRY
ENDFOR
