* Independent installed VFP9 AddListItem observations; NO acceptance assertions.
* Selected second argument, Character item, omitted third column; generic capture only.
LOCAL ARRAY aValues[55]
aValues[1] = '0'
aValues[2] = '-0.9'
aValues[3] = '0.9'
aValues[4] = '1'
aValues[5] = '1.4'
aValues[6] = '1.5'
aValues[7] = '1.9'
aValues[8] = '2'
aValues[9] = '2.4'
aValues[10] = '2.5'
aValues[11] = '2.9'
aValues[12] = '3'
aValues[13] = '3.9'
aValues[14] = '4'
aValues[15] = '4.9'
aValues[16] = '11'
aValues[17] = '11.9'
aValues[18] = '22.9'
aValues[19] = '33.9'
aValues[20] = '34'
aValues[21] = '-1'
aValues[22] = '-1.9'
aValues[23] = '32766'
aValues[24] = '32766.9'
aValues[25] = '32767'
aValues[26] = '32767.1'
aValues[27] = '32767.9'
aValues[28] = '32768'
aValues[29] = '32768.9'
aValues[30] = '32769'
aValues[31] = '65535'
aValues[32] = '65537'
aValues[33] = '2147483647'
aValues[34] = '2147483648'
aValues[35] = '4294967295'
aValues[36] = '4294967296'
aValues[37] = '4294967297'
aValues[38] = '4294967298.9'
aValues[39] = '4294967299'
aValues[40] = '4294967307.9'
aValues[41] = '-4294967295.9'
aValues[42] = '-4294967294.5'
aValues[43] = '-4294967293'
aValues[44] = '-4294967285.5'
aValues[45] = '4295000063.9'
aValues[46] = '-4294934529.9'
aValues[47] = '4295000064'
aValues[48] = '-4294934528'
aValues[49] = '9007199254740991'
aValues[50] = '-9007199254740991'
aValues[51] = '9007199254740992'
aValues[52] = '1E300'
aValues[53] = '-1E300'
aValues[54] = '1E-300'
aValues[55] = '-1E-300'
LOCAL nClass,nShape,nArg,o,cClass,cSource,cPhase,cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10), 'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\independent-results.txt',0)
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
    cReturn=TRANSFORM(o.AddListItem('candidate',v))
   CATCH TO problem
    nError=problem.ErrorNo
    cMessage=STRTRAN(STRTRAN(problem.Message,CHR(13),' '),CHR(10),' ')
   ENDTRY
   IF VARTYPE(o)='O'
    cAfter=Snapshot(o,cClass)
   ENDIF
   cLine=cClass+'|shape='+ALLTRIM(STR(nShape))+'|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|message='+cMessage+'|before='+cBefore+'|after='+cAfter
   =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\independent-results.txt',1)
   nCalls=nCalls+1
   o=.NULL.
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\independent-results.txt',1)
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
