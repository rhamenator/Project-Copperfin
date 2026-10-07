* Independent native absence and equivalent Gregorian February controls.
LOCAL lnYear, lcExpr, luValue, loError, lnGuard
LOCAL ARRAY aYears[10], aExpressions[10]
aYears[1]=1800
aYears[2]=1900
aYears[3]=1996
aYears[4]=2000
aYears[5]=2024
aYears[6]=2026
aYears[7]=2100
aYears[8]=2400
aYears[9]=9996
aYears[10]=9999
aExpressions[1]="ISLEAPYEAR(2024)"
aExpressions[2]="ISLEAPYEAR(1900)"
aExpressions[3]="ISLEAPYEAR(2024.9)"
aExpressions[4]="ISLEAPYEAR(0)"
aExpressions[5]="ISLEAPYEAR(-400)"
aExpressions[6]="ISLEAPYEAR(4294967300)"
aExpressions[7]="ISLEAPYEAR(1E20)"
aExpressions[8]="ISLEAPYEAR(.NULL.)"
aExpressions[9]="ISLEAPYEAR(.T.)"
aExpressions[10]="ISLEAPYEAR('2024')"
lnGuard=73
? "VERSION|"+VERSION()
? "BEFORE|"+TRANSFORM(lnGuard)
FOR lnYear=1 TO ALEN(aYears)
    lcExpr="DAY(GOMONTH(DATE("+TRANSFORM(aYears[lnYear])+",1,31),1))"
    TRY
        luValue=EVALUATE(lcExpr)
        ? lcExpr+"|"+VARTYPE(luValue)+"|"+TRANSFORM(luValue)
    CATCH TO loError
        ? lcExpr+"|ERR"+TRANSFORM(loError.ErrorNo)
    ENDTRY
ENDFOR
FOR lnYear=1 TO ALEN(aExpressions)
    lcExpr=aExpressions[lnYear]
    TRY
        luValue=EVALUATE(lcExpr)
        ? lcExpr+"|"+VARTYPE(luValue)+"|"+TRANSFORM(luValue)
    CATCH TO loError
        ? lcExpr+"|ERR"+TRANSFORM(loError.ErrorNo)
    ENDTRY
ENDFOR
? "AFTER|"+TRANSFORM(lnGuard)
