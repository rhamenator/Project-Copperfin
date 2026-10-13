SET NUMERICBEHAVIOR TO VFP9
SET DATASESSION TO 1
SET NUMERICBEHAVIOR TO COPPERFIN
PUBLIC o,nBefore,nCount,nFocus,nID,cValue,cDisplay,nTop,nTopID,nNewIndex,nNewID,nCode,vResult,cContext,cPhase,cOther
nCode=0
vResult='not-returned'
cPhase='setup'
TRY
o=CREATEOBJECT('ListBox')
o.ColumnCount=2
o.RowSourceType=1
o.RowSource='first,second,third'
o.Requery()
o.ListIndex=2
o.TopIndex=2
nBefore=o.ListCount
cPhase='invoke'
vResult=o.AddItem('candidate',1.9)
CATCH TO problem
nCode=problem.ErrorNo
ENDTRY
nCount=o.ListCount
nFocus=o.ListIndex
nID=o.ListItemID
cValue=o.Value
cDisplay=o.DisplayValue
nNewIndex=o.NewIndex
nNewID=o.NewItemID
nTop=o.TopIndex
nTopID=o.TopItemID
cContext=SET('DATASESSION')+'|'+SET('NUMERICBEHAVIOR')+'|'+UPPER(ALIAS())
SET DATASESSION TO 1
cOther=SET('NUMERICBEHAVIOR')
RETURN
