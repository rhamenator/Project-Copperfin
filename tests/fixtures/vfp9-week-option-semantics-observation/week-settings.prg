SET CENTURY ON
SET DATE TO YMD

DIMENSION aDates[5]
aDates[1] = "DATE(2021,1,1)"
aDates[2] = "DATE(2021,1,3)"
aDates[3] = "DATE(2021,1,4)"
aDates[4] = "DATE(2021,1,7)"
aDates[5] = "DATE(2021,12,31)"

FOR nSettings = 1 TO 2
    IF nSettings = 1
        SET FWEEK TO 1
        SET FDOW TO 1
        cSettings = "set-1-1"
    ELSE
        SET FWEEK TO 3
        SET FDOW TO 4
        cSettings = "set-3-4"
    ENDIF

    FOR nDate = 1 TO ALEN(aDates)
        cDate = aDates[nDate]
        ? cSettings + " date-" + TRANSFORM(nDate) + " omitted => " + TRANSFORM(EVALUATE("WEEK(" + cDate + ")"))
        ? cSettings + " date-" + TRANSFORM(nDate) + " arg2-zero => " + TRANSFORM(EVALUATE("WEEK(" + cDate + ",0)"))
        ? cSettings + " date-" + TRANSFORM(nDate) + " zeros => " + TRANSFORM(EVALUATE("WEEK(" + cDate + ",0,0)"))
        ? cSettings + " date-" + TRANSFORM(nDate) + " firstweek-one => " + TRANSFORM(EVALUATE("WEEK(" + cDate + ",1)"))
        ? cSettings + " date-" + TRANSFORM(nDate) + " firstweek-one-day-zero => " + TRANSFORM(EVALUATE("WEEK(" + cDate + ",1,0)"))
        ? cSettings + " date-" + TRANSFORM(nDate) + " firstweek-one-day-four => " + TRANSFORM(EVALUATE("WEEK(" + cDate + ",1,4)"))
    ENDFOR
ENDFOR
