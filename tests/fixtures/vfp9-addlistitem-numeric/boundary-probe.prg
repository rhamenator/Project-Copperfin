* Independent installed VFP9 AddListItem observations; NO acceptance assertions.
* Selected second argument, Character item, omitted third column; generic capture only.
LOCAL ARRAY aValues[32]
aValues[1] = '0.9999999999999999'
aValues[2] = '1-2^(-52)'
aValues[3] = '1-2^(-53)'
aValues[4] = '32766.5'
aValues[5] = '32767.5'
aValues[6] = '32767.999'
aValues[7] = '32768-2^(-38)'
aValues[8] = '32768-.00000000001'
aValues[9] = '32768.5'
aValues[10] = '32769.9'
aValues[11] = '4295000063.1'
aValues[12] = '4295000063.5'
aValues[13] = '4295000063.9'
aValues[14] = '4295000064.9'
aValues[15] = '-4294934529.1'
aValues[16] = '-4294934529.5'
aValues[17] = '-4294934529.9'
aValues[18] = '-4294934528.9'
aValues[19] = '8589934593'
aValues[20] = '-8589934591.9'
aValues[21] = '-8589934590'
aValues[22] = '-4294967296'
aValues[23] = '-4294967297'
aValues[24] = '4295032831'
aValues[25] = '-4294901761'
aValues[26] = '9007199254740993'
aValues[27] = '-9007199254740993'
aValues[28] = '-9007199254740991'
aValues[29] = '1E20'
aValues[30] = '-1E20'
aValues[31] = '(1E300*1E300)'
aValues[32] = '(-1E300*1E300)'
LOCAL nClass,nShape,nArg,o,cClass,cSource,cPhase,cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10), 'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\boundary-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nShape=0 TO 3
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
    o.RowSourceType=0
    o.ControlSource=''
    o.ColumnCount=2
    DO CASE
    CASE nShape=1
     o.AddListItem('first',1)
     o.AddListItem('second',2)
     o.AddListItem('third',3)
    CASE nShape=2
     o.AddListItem('first',11)
     o.AddListItem('second',22)
     o.AddListItem('third',33)
    CASE nShape=3
     o.AddListItem('first',32767)
    ENDCASE
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
    cMessage='one='+TRANSFORM(v=1)+',belowone='+TRANSFORM(v<1)+',upper='+TRANSFORM(v=32768)+',belowupper='+TRANSFORM(v<32768)
    cReturn=TRANSFORM(o.AddListItem('candidate',v))
   CATCH TO problem
    nError=problem.ErrorNo
    cMessage=STRTRAN(STRTRAN(problem.Message,CHR(13),' '),CHR(10),' ')
   ENDTRY
   IF VARTYPE(o)='O'
    cAfter=Snapshot(o,cClass)
   ENDIF
   cLine=cClass+'|shape='+ALLTRIM(STR(nShape))+'|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|message='+cMessage+'|before='+cBefore+'|after='+cAfter
   =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\boundary-results.txt',1)
   nCalls=nCalls+1
   o=.NULL.
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\boundary-results.txt',1)
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
