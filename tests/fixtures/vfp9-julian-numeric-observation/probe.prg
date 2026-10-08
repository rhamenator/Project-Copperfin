* Clean-room native admission and equivalent Gregorian calendar controls.
LOCAL lnI, lcExpr, luValue, loError, lcKind, lcRender, lnGuard
LOCAL ARRAY aExpressions[44]
aExpressions[1]="JTOD(2461149)"
aExpressions[2]="JTOD(2461149.9)"
aExpressions[3]="JTOD(0)"
aExpressions[4]="JTOD(-1)"
aExpressions[5]="JTOD(2147483647)"
aExpressions[6]="JTOD(1E20)"
aExpressions[7]="JTOD(1E300)"
aExpressions[8]="JTOD(.NULL.)"
aExpressions[9]="JTOD('2461149')"
aExpressions[10]="JTOD($2461149.9)"
aExpressions[11]="JTOT(2461149)"
aExpressions[12]="JTOT(2461149.9)"
aExpressions[13]="JTOT(0)"
aExpressions[14]="JTOT(-1)"
aExpressions[15]="JTOT(2147483647)"
aExpressions[16]="JTOT(1E20)"
aExpressions[17]="JTOT(1E300)"
aExpressions[18]="JTOT(.NULL.)"
aExpressions[19]="JTOT('2461149')"
aExpressions[20]="JTOT($2461149.9)"
aExpressions[21]="DTOJ(DATE(2026,4,18))"
aExpressions[22]="TTOJ(DATETIME(2026,4,18,12,13,14))"
aExpressions[23]="{^0001-01-01}"
aExpressions[24]="DATE(100,1,1)"
aExpressions[25]="DATE(1601,1,1)"
aExpressions[26]="DATE(1900,2,28)"
aExpressions[27]="DATE(1900,3,1)"
aExpressions[28]="DATE(1970,1,1)"
aExpressions[29]="DATE(2000,1,1)"
aExpressions[30]="DATE(2024,2,29)"
aExpressions[31]="DATE(2026,4,18)"
aExpressions[32]="DATE(9999,12,31)"
aExpressions[33]="CTOD('not a date')"
aExpressions[34]="{^0001-01-01 00:00:00}"
aExpressions[35]="DATETIME(100,1,1,0,0,0)"
aExpressions[36]="DATETIME(1601,1,1,0,0,0)"
aExpressions[37]="DATETIME(1900,2,28,0,0,0)"
aExpressions[38]="DATETIME(1900,3,1,0,0,0)"
aExpressions[39]="DATETIME(1970,1,1,0,0,0)"
aExpressions[40]="DATETIME(2000,1,1,0,0,0)"
aExpressions[41]="DATETIME(2024,2,29,0,0,0)"
aExpressions[42]="DATETIME(2026,4,18,0,0,0)"
aExpressions[43]="DATETIME(9999,12,31,0,0,0)"
aExpressions[44]="CTOT('not a date')"
SET STRICTDATE TO 0
SET DATE TO YMD
SET CENTURY ON
SET MARK TO '-'
lnGuard=31
? "VERSION|"+VERSION()
? "BEFORE|"+TRANSFORM(lnGuard)
FOR lnI=1 TO ALEN(aExpressions)
    lcExpr=aExpressions[lnI]
    TRY
        luValue=EVALUATE(lcExpr)
        lcKind=VARTYPE(luValue)
        DO CASE
        CASE lcKind='D'
            IF EMPTY(luValue)
                lcRender='EMPTY'
            ELSE
                lcRender=DTOS(luValue)
            ENDIF
        CASE lcKind='T'
            IF EMPTY(luValue)
                lcRender='EMPTY'
            ELSE
                lcRender=TTOC(luValue,3)
            ENDIF
        OTHERWISE
            lcRender=TRANSFORM(luValue)
        ENDCASE
        ? lcExpr+'|'+lcKind+'|'+lcRender
    CATCH TO loError
        ? lcExpr+'|ERR'+TRANSFORM(loError.ErrorNo)
    ENDTRY
ENDFOR
? "AFTER|"+TRANSFORM(lnGuard)
