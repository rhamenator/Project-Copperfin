* Independent boundary diagnostic; lifecycle is outside RemoveItem admission.
LOCAL lcClass, loList, lcOut, lnClass, lnBefore, lnAfter
lcOut = 'VERSION|' + VERSION() + CHR(13) + CHR(10)
FOR lnClass = 1 TO 2
    lcClass = IIF(lnClass = 1, 'ListBox', 'ComboBox')
    loList = CREATEOBJECT(lcClass)
    loList.AddListItem('first', 11)
    loList.AddListItem('second', 22)
    loList.AddListItem('third', 33)
    lnBefore = loList.ListCount
    loList.RowSourceType = 0
    lnAfter = loList.ListCount
    lcOut = lcOut + lcClass + '|before=' + TRANSFORM(lnBefore) + ;
        '|afterRST0=' + TRANSFORM(lnAfter) + CHR(13) + CHR(10)
ENDFOR
lcOut = lcOut + 'DONE|2' + CHR(13) + CHR(10)
=STRTOFILE(lcOut, 'review2-rst0-assignment-results.txt')
