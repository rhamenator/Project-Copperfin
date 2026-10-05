LOCAL lcOut
lcOut = "VFP9 RAND seed conversion observation" + CHR(13) + CHR(10)
lcOut = lcOut + ProbeSeed("zero", "0")
lcOut = lcOut + ProbeSeed("negative-zero", "-0.0")
lcOut = lcOut + ProbeSeed("positive-one", "1")
lcOut = lcOut + ProbeSeed("negative-one", "-1")
lcOut = lcOut + ProbeSeed("positive-123", "123")
lcOut = lcOut + ProbeSeed("negative-123", "-123")
lcOut = lcOut + ProbeSeed("positive-123.4", "123.4")
lcOut = lcOut + ProbeSeed("positive-123.5", "123.5")
lcOut = lcOut + ProbeSeed("positive-123.6", "123.6")
lcOut = lcOut + ProbeSeed("negative-123.4", "-123.4")
lcOut = lcOut + ProbeSeed("negative-123.5", "-123.5")
lcOut = lcOut + ProbeSeed("negative-123.6", "-123.6")
lcOut = lcOut + ProbeSeed("positive-int32-max", "2147483647")
lcOut = lcOut + ProbeSeed("positive-int32-max-plus-one", "2147483648")
lcOut = lcOut + ProbeSeed("positive-int32-max-plus-two", "2147483649")
lcOut = lcOut + ProbeSeed("negative-int32-min", "-2147483648")
lcOut = lcOut + ProbeSeed("negative-int32-min-minus-one", "-2147483649")
lcOut = lcOut + ProbeSeed("positive-uint32-max", "4294967295")
lcOut = lcOut + ProbeSeed("positive-uint32-wrap", "4294967296")
lcOut = lcOut + ProbeSeed("negative-uint32-max", "-4294967295")
lcOut = lcOut + ProbeSeed("negative-uint32-wrap", "-4294967296")
lcOut = lcOut + ProbeSeed("positive-2pow53", "9007199254740992")
lcOut = lcOut + ProbeSeed("negative-2pow53", "-9007199254740992")
lcOut = lcOut + ProbeSeed("positive-1e20", "1E20")
lcOut = lcOut + ProbeSeed("negative-1e20", "-1E20")
lcOut = lcOut + ProbeSeed("positive-overflow", "EXP(1000)")
lcOut = lcOut + ProbeSeed("negative-overflow", "-EXP(1000)")
lcOut = lcOut + ProbeSeed("currency-int32-max", "$2147483647.0000")
lcOut = lcOut + ProbeSeed("currency-int32-max-plus-one", "$2147483648.0000")
lcOut = lcOut + ProbeSeed("currency-int32-min", "$-2147483648.0000")

= STRTOFILE(lcOut, "Z:\home\rich\temp\vfp9-probes\rand-seed-5611\result.txt", 0)
QUIT

FUNCTION ProbeSeed
    LPARAMETERS tcLabel, tcExpression
    LOCAL loError, lnFirst, lnNext, lnRepeat, lnRepeatNext, lcLine
    TRY
        lnFirst = RAND(EVALUATE(tcExpression))
        lnNext = RAND()
        lnRepeat = RAND(EVALUATE(tcExpression))
        lnRepeatNext = RAND()
        lcLine = tcLabel + "|expr=" + tcExpression + ;
            "|first=" + STR(lnFirst, 20, 15) + ;
            "|next=" + STR(lnNext, 20, 15) + ;
            "|repeat=" + STR(lnRepeat, 20, 15) + ;
            "|repeat-next=" + STR(lnRepeatNext, 20, 15) + ;
            "|deterministic=" + IIF(lnFirst == lnRepeat AND lnNext == lnRepeatNext, ".T.", ".F.")
    CATCH TO loError
        lcLine = tcLabel + "|expr=" + tcExpression + ;
            "|error=" + TRANSFORM(loError.ErrorNo) + ;
            "|message=" + loError.Message
    ENDTRY
    RETURN lcLine + CHR(13) + CHR(10)
ENDFUNC
