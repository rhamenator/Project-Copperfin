LOCAL lcOut, lcValues, lcValue, lcCommand, lcResult, lcRaw
LOCAL lnValue, lnError, loError, vSource, vResult
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcValues = '(2^63-1024)|(2^63)|(2^63+2048)|(-2^63)|(-2^63-2048)|9223372036854774784|'
lcValues = lcValues + '2147483647.9999998|2147483648.0000005|-2147483648.9999995|-2147483649.0000005|EXP(1000)|-EXP(1000)'
FOR lnValue = 1 TO GETWORDCOUNT(lcValues, '|')
  lcValue = GETWORDNUM(lcValues, lnValue, '|')
  lnError = 0
  lcResult = '<not-returned>'
  lcRaw = '<not-returned>'
  TRY
    lcCommand = 'vSource = ' + lcValue
    &lcCommand
    lcRaw = ALLTRIM(STR(vSource, 40, 0))
    vResult = CAST(vSource AS INTEGER)
    lcResult = ALLTRIM(STR(vResult, 30, 0))
  CATCH TO loError
    lnError = loError.ErrorNo
  ENDTRY
  lcOut = lcOut + lcValue + '|SOURCE=' + lcRaw + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'supplement.out')
QUIT
