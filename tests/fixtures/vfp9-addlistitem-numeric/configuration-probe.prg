* Independent installed VFP9 AddListItem observations; NO acceptance assertions.
* Selected second argument, Character item, omitted third column; generic capture only.
LOCAL ARRAY aValues[12]
aValues[1] = '0'
aValues[2] = '0.9'
aValues[3] = '1.9'
aValues[4] = '2.5'
aValues[5] = '4.9'
aValues[6] = '11.9'
aValues[7] = '32767.9'
aValues[8] = '32768'
aValues[9] = '4294967307.9'
aValues[10] = '-4294967285.5'
aValues[11] = '(1E300*1E300)'
aValues[12] = '1E-300'
LOCAL nClass,nShape,nArg,o,cClass,cSource,cPhase,cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10), 'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\configuration-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nShape=1 TO 4
  FOR nArg=1 TO ALEN(aValues)
   cSource=aValues[nArg]
   nError=0
   cMessage=''
   cReturn='<not-called>'
   cBefore='<setup-incomplete>'
   cAfter='<setup-incomplete>'
   cPhase='setup'
   TRY
    o=CREATEOBJECT(cClass)
    IF nShape>1
     o.RowSourceType=0
    ENDIF
    o.ControlSource=IIF(nShape=3,'   ',IIF(nShape=4,CHR(9)+'  ',''))
    o.ColumnCount=2
    o.AddListItem('first',11)
    o.AddListItem('second',22)
    o.AddListItem('third',33)
    FOR nRow=1 TO o.ListCount
     o.List(nRow,2)='cell'+ALLTRIM(STR(nRow))
     o.ItemData(nRow)=nRow*101
    ENDFOR
    IF o.ListCount>0
     o.ListIndex=IIF(o.ListCount=1,1,2)
     IF cClass='ListBox'
      o.TopIndex=IIF(o.ListCount=1,1,2)
     ENDIF
    ENDIF
    cBefore=Snapshot(o,cClass)
    cPhase='evaluate'
    v=EVALUATE(cSource)
    cPhase='invoke'
    cReturn=TRANSFORM(o.AddListItem('candidate',v))
   CATCH TO problem
    nError=problem.ErrorNo
    cMessage=STRTRAN(STRTRAN(problem.Message,CHR(13),' '),CHR(10),' ')
   ENDTRY
   IF VARTYPE(o)='O'
    cAfter=Snapshot(o,cClass)
   ENDIF
   cLine=cClass+'|shape='+ALLTRIM(STR(nShape))+'|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|message='+cMessage+'|before='+cBefore+'|after='+cAfter
   =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\configuration-results.txt',1)
   nCalls=nCalls+1
   o=.NULL.
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\configuration-results.txt',1)
RETURN

FUNCTION Snapshot
 LPARAMETERS o,cClass
 LOCAL c,nRow
 c='count='+ALLTRIM(STR(o.ListCount))+',focus='+ALLTRIM(STR(o.ListIndex))+',id='+ALLTRIM(STR(o.ListItemID))+',value='+TRANSFORM(o.Value)+',display='+TRANSFORM(o.DisplayValue)+',newindex='+ALLTRIM(STR(o.NewIndex))+',newid='+ALLTRIM(STR(o.NewItemID))
 IF cClass='ListBox'
  c=c+',top='+ALLTRIM(STR(o.TopIndex))+',topid='+ALLTRIM(STR(o.TopItemID))
 ENDIF
 FOR nRow=1 TO o.ListCount
  c=c+';row='+ALLTRIM(STR(o.IndexToItemID(nRow)))+','+TRANSFORM(o.List(nRow,1))+','+TRANSFORM(o.List(nRow,2))+','+TRANSFORM(o.ItemData(nRow))+','+TRANSFORM(o.Selected(nRow))
 ENDFOR
 RETURN c
ENDFUNC
