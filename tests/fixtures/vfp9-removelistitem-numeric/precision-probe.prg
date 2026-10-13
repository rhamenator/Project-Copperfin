* Evaluation provenance only: decimal source is not a binary nextafter oracle.
LOCAL lnCase, lnValue, lcOut
LOCAL ARRAY aExpr[6]
aExpr[1] = [0.9999999999999999]
aExpr[2] = [1-2^(-52)]
aExpr[3] = [1-2^(-53)]
aExpr[4] = [32768-2^(-38)]
aExpr[5] = [32768-0.00000000001]
aExpr[6] = [32767.9]
lcOut = 'VERSION|'+VERSION()+CHR(13)+CHR(10)
FOR lnCase = 1 TO ALEN(aExpr)
    lnValue = EVALUATE(aExpr[lnCase])
    lcOut = lcOut+aExpr[lnCase]+'|value='+STR(lnValue,30,18) + ;
        '|equalsOne='+TRANSFORM(lnValue=1)+'|lessOne='+TRANSFORM(lnValue<1) + ;
        '|equalsUpper='+TRANSFORM(lnValue=32768)+CHR(13)+CHR(10)
ENDFOR
lcOut = lcOut+'DONE|6'+CHR(13)+CHR(10)
=STRTOFILE(lcOut,'precision-results.txt')
