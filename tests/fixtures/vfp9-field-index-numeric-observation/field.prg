LOCAL lcExpression, luResult, loError
? 'VERSION|' + VERSION()
CREATE CURSOR cfprobe (alpha C(5), bravo N(4), charlie L)
CREATE CURSOR cfother (delta C(5), echo N(4), foxtrot L)
SELECT cfprobe
lcExpression = "FIELD(0)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(2)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(3)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(4)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(1.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(1.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(1.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(2.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(2.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(2.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(3.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-1.1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(2147483647)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-2147483649)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(4294967295)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(4294967296)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(4294967297)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(4294967298)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(4294967299)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(4294967300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-4294967296)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-4294967295)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-4294967294)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-4294967293)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(4294967298.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-4294967294.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(1E20)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-1E20)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(9007199254740992)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(32767)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(32768)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(65535)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(65536)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(65537)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-65535)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(-65536)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(1.9, 'cfother')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(2.9, 'cfprobe')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(1.9, SELECT('cfother'))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD()"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD('2.5')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD($2.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(.T.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "FIELD(.NULL.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
USE IN cfother
USE IN cfprobe
