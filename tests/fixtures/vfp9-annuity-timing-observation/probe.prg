* Installed VFP controls and explicit optional-timing syntax boundary.
LOCAL lcExpr, luValue, loError, lnIndex, lnGuard
LOCAL ARRAY aExpressions[34]
aExpressions[1]="FV(100,0.05,2)"
aExpressions[2]="PV(100,0.05,2)"
aExpressions[3]="FV(-100,0.05,2)"
aExpressions[4]="PV(-100,0.05,2)"
aExpressions[5]="FV(100,0.05,1)"
aExpressions[6]="PV(100,0.05,1)"
aExpressions[7]="FV(100,0,2)"
aExpressions[8]="PV(100,0,2)"
aExpressions[9]="FV(100,0.05,2,0)"
aExpressions[10]="PV(100,0.05,2,0)"
aExpressions[11]="FV(0.05,2,-100,0,0)"
aExpressions[12]="PV(0.05,2,-100,0,0)"
aExpressions[13]="FV(0.05,2,-100,0,1)"
aExpressions[14]="PV(0.05,2,-100,0,1)"
aExpressions[15]="FV(0.05,2,-100,0,0.5)"
aExpressions[16]="PV(0.05,2,-100,0,0.5)"
aExpressions[17]="FV(0.05,2,-100,0,-1)"
aExpressions[18]="PV(0.05,2,-100,0,-1)"
aExpressions[19]="FV(0.05,2,-100,0,2)"
aExpressions[20]="PV(0.05,2,-100,0,2)"
aExpressions[21]="FV(0.05,2,-100,0,2147483648)"
aExpressions[22]="PV(0.05,2,-100,0,2147483648)"
aExpressions[23]="FV(0.05,2,-100,0,4294967297)"
aExpressions[24]="PV(0.05,2,-100,0,4294967297)"
aExpressions[25]="FV(0.05,2,-100,0,1E20)"
aExpressions[26]="PV(0.05,2,-100,0,1E20)"
aExpressions[27]="FV(0.05,2,-100,0,1E300)"
aExpressions[28]="PV(0.05,2,-100,0,1E300)"
aExpressions[29]="FV(0.05,2,-100,0,.NULL.)"
aExpressions[30]="PV(0.05,2,-100,0,.NULL.)"
aExpressions[31]="FV(0.05,2,-100,0,.T.)"
aExpressions[32]="PV(0.05,2,-100,0,.T.)"
aExpressions[33]="FV(0.05,2,-100,0,'1')"
aExpressions[34]="PV(0.05,2,-100,0,'1')"
lnGuard=41
? "VERSION|"+VERSION()
? "BEFORE|"+TRANSFORM(lnGuard)
FOR lnIndex=1 TO ALEN(aExpressions)
    lcExpr=aExpressions[lnIndex]
    TRY
        luValue=EVALUATE(lcExpr)
        ? lcExpr+"|"+VARTYPE(luValue)+"|"+TRANSFORM(luValue)
    CATCH TO loError
        ? lcExpr+"|ERR"+TRANSFORM(loError.ErrorNo)
    ENDTRY
ENDFOR
? "AFTER|"+TRANSFORM(lnGuard)

