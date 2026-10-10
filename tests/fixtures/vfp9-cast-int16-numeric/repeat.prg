LOCAL lcOut, lcTypes, lcValues, lcType, lcValue, lcCommand, lcResult
LOCAL lnType, lnValue, lnError, loError, vResult
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcTypes = 'INTEGER|INT16|SHORT'
lcValues = '0|-0|0.9|-0.9|1.9|-1.9|32766|32767|32767.9|32768|32768.9|'
lcValues = lcValues + '-32767|-32768|-32768.9|-32769|-32769.9|65535|65536|-65536|'
lcValues = lcValues + '2147483647|2147483648|-2147483649|9007199254740992|1E20|-1E20|1E300|-1E300|EXP(1000)|-EXP(1000)'
FOR lnType = 1 TO GETWORDCOUNT(lcTypes, '|')
  lcType = GETWORDNUM(lcTypes, lnType, '|')
  FOR lnValue = 1 TO GETWORDCOUNT(lcValues, '|')
    lcValue = GETWORDNUM(lcValues, lnValue, '|')
    lcCommand = 'vResult = CAST(' + lcValue + ' AS ' + lcType + ')'
    lnError = 0
    lcResult = '<not-returned>'
    TRY
      &lcCommand
      lcResult = ALLTRIM(STR(vResult, 30, 0))
    CATCH TO loError
      lnError = loError.ErrorNo
    ENDTRY
    lcOut = lcOut + lcValue + ' AS ' + lcType + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
  ENDFOR
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'native-2.out')
QUIT
