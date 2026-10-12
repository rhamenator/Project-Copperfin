* Independent installedVFP9 evidence only; no expected result or Copperfin code.
LOCAL aExpr[12], nShape, nHeight, nInput, oList, cExpr, cError, oError
LOCAL cShape, cBefore, cAfter, cBits, cRows, cData, cIds, nRow
aExpr[1]='0'
aExpr[2]='0.9'
aExpr[3]='1'
aExpr[4]='1.9'
aExpr[5]='2.9'
aExpr[6]='3.9'
aExpr[7]='4'
aExpr[8]='22.9'
aExpr[9]='2147483648'
aExpr[10]='4294967295.5'
aExpr[11]='EXP(1000)'
aExpr[12]='-EXP(1000)'
? 'VERSION|' + VERSION()
FOR nShape=1 TO 4
 cShape=IIF(nShape=1,'dense',IIF(nShape=2,'sparse',IIF(nShape=3,'empty','sparse-multi')))
 FOR nHeight=1 TO 3
  FOR nInput=1 TO ALEN(aExpr)
   oList=CREATEOBJECT('ListBox')
   IF nHeight=2
    oList.Height=18
   ENDIF
   IF nHeight=3
    oList.Height=50
   ENDIF
   IF nShape<>3
    IF nShape=1
     oList.AddItem('first')
     oList.AddItem('second')
     oList.AddItem('third')
    ELSE
     oList.AddListItem('first',11)
     oList.AddListItem('second',22)
     oList.AddListItem('third',33)
    ENDIF
    oList.ItemData(1)=101
    oList.ItemData(2)=202
    oList.ItemData(3)=303
    IF nShape=4
     oList.MultiSelect=.T.
     oList.Selected(1)=.T.
     oList.Selected(2)=.T.
    ENDIF
    oList.ListIndex=2
    oList.TopIndex=1
   ENDIF
   cBefore=TRANSFORM(oList.TopIndex)+','+TRANSFORM(oList.TopItemID)+','+TRANSFORM(oList.ListIndex)+','+TRANSFORM(oList.ListItemID)
   cError='OK'
   cExpr=aExpr[nInput]
   TRY
    oList.TopIndex=EVALUATE(cExpr)
   CATCH TO oError
    cError='ERR'+TRANSFORM(oError.ErrorNo)
   ENDTRY
   cAfter=TRANSFORM(oList.TopIndex)+','+TRANSFORM(oList.TopItemID)+','+TRANSFORM(oList.ListIndex)+','+TRANSFORM(oList.ListItemID)
   cBits=''
   cRows=''
   cData=''
   cIds=''
   FOR nRow=1 TO oList.ListCount
    cBits=cBits+IIF(nRow>1,',','')+TRANSFORM(oList.Selected(nRow))
    cRows=cRows+IIF(nRow>1,',','')+oList.List(nRow)
    cData=cData+IIF(nRow>1,',','')+TRANSFORM(oList.ItemData(nRow))
    cIds=cIds+IIF(nRow>1,',','')+TRANSFORM(oList.IndexToItemID(nRow))
   ENDFOR
   ? cShape+'|'+TRANSFORM(oList.Height)+'|'+cExpr+'|'+cError+'|'+cBefore+'|'+cAfter+'|'+VARTYPE(oList.Value)+':'+TRANSFORM(oList.Value)+'|'+VARTYPE(oList.DisplayValue)+':'+TRANSFORM(oList.DisplayValue)+'|'+TRANSFORM(oList.ListCount)+'|'+cBits+'|'+cRows+'|'+cData+'|'+cIds
  ENDFOR
 ENDFOR
ENDFOR
