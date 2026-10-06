LOCAL lcExpression, luResult, loError
? 'VERSION|' + VERSION()
SET COMPATIBLE OFF
CREATE CURSOR cfprobe (alpha C(5))
CREATE CURSOR cfother (bravo N(4))
SELECT cfprobe
lcExpression = "SELECT(0)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(2)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(3)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(4)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(1.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(1.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(1.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(2.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(2.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(2.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(3.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-0.49)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-0.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-1.1)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(2147483647)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-2147483648)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-2147483649)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(4294967295)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(4294967296)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(4294967297)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(4294967298)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(4294967299)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(4294967300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-4294967296)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-4294967295)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-4294967294)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-4294967293)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(4294967298.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-4294967294.9)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(1E20)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-1E20)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-1E300)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-EXP(1000))"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(9007199254740992)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(32767)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(32768)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(65535)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(65536)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(65537)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-65535)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(-65536)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT()"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT('cfprobe')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT('cfother')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT('missing')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT('0')"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT($0.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT($1.5)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(.T.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(.F.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
lcExpression = "SELECT(.NULL.)"
TRY
    luResult = EVALUATE(lcExpression)
    ? lcExpression + '|' + VARTYPE(luResult) + ':[' + TRANSFORM(luResult) + ']|ALIAS:' + ALIAS()
CATCH TO loError
    ? lcExpression + '|ERR' + TRANSFORM(loError.ErrorNo) + '|ALIAS:' + ALIAS()
ENDTRY
USE IN cfother
USE IN cfprobe
