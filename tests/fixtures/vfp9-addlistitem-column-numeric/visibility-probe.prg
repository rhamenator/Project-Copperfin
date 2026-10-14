* Independent deferred-visible-column corroboration for supplied third column.
* Only present sparse stable ID22; no Copperfin helper/default/expectations.
LOCAL ARRAY aValues[10],aColumns[3]
aValues[1]='1.9'
aValues[2]='2.9'
aValues[3]='3.9'
aValues[4]='8.9'
aValues[5]='9'
aValues[6]='65535'
aValues[7]='65536'
aValues[8]='65537'
aValues[9]='4294967297.9'
aValues[10]='4294967298.9'
aColumns[1]=0
aColumns[2]=2
aColumns[3]=8
LOCAL nClass,nConfig,nArg,nColumns,o,cClass,v,cBefore,cAfter,cExpanded,cSource
LOCAL cPhase,nError,nExpandError,cMessage,cReturn,nCalls,nRow,nCol,cBits
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\visibility-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nConfig=1 TO ALEN(aColumns)
  nColumns=aColumns[nConfig]
  FOR nArg=1 TO ALEN(aValues)
   cSource=aValues[nArg]
   nError=0
   nExpandError=0
   cMessage=''
   cReturn='<not-called>'
   cBefore='<setup-incomplete>'
   cAfter='<setup-incomplete>'
   cExpanded='<not-expanded>'
   cBits='<not-evaluated>'
   cPhase='setup'
   o=.NULL.
   TRY
    o=CREATEOBJECT(cClass)
    o.RowSourceType=0
    o.ControlSource=''
    o.ColumnCount=nColumns
    o.AddListItem('first',11)
    o.AddListItem('second',22)
    o.AddListItem('third',33)
    FOR nRow=1 TO o.ListCount
     FOR nCol=2 TO MAX(1,nColumns)
      o.List(nRow,nCol)='cell'+ALLTRIM(STR(nRow))+':'+ALLTRIM(STR(nCol))
     ENDFOR
     o.ItemData(nRow)=nRow*101
    ENDFOR
    o.ListIndex=2
    IF cClass='ListBox'
     o.TopIndex=2
    ENDIF
    cBefore=Snapshot(o,cClass,nColumns)
    cPhase='evaluate'
    v=EVALUATE(cSource)
    cBits=PackedHex(v)
    cPhase='invoke'
    cReturn=TRANSFORM(o.AddListItem('candidate',22,v))
   CATCH TO problem
    nError=problem.ErrorNo
    cMessage=STRTRAN(STRTRAN(problem.Message,CHR(13),' '),CHR(10),' ')
   ENDTRY
   IF VARTYPE(o)='O'
    cAfter=Snapshot(o,cClass,nColumns)
    TRY
     o.ColumnCount=9
     cExpanded=Snapshot(o,cClass,9)
    CATCH TO expandProblem
     nExpandError=expandProblem.ErrorNo
    ENDTRY
   ENDIF
   =STRTOFILE(cClass+'|columns='+ALLTRIM(STR(nColumns))+'|target=22|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|message='+cMessage+'|preBRS='+cBits+'|expandError='+ALLTRIM(STR(nExpandError))+'|before='+cBefore+'|after='+cAfter+'|expanded='+cExpanded+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\visibility-results.txt',1)
   nCalls=nCalls+1
   o=.NULL.
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\visibility-results.txt',1)
RETURN

FUNCTION Snapshot
 LPARAMETERS o,cClass,nColumns
 LOCAL c,nRow,nCol
 c='count='+ALLTRIM(STR(o.ListCount))+',columns='+ALLTRIM(STR(o.ColumnCount))+',focus='+ALLTRIM(STR(o.ListIndex))+',id='+ALLTRIM(STR(o.ListItemID))+',value='+TRANSFORM(o.Value)+',display='+TRANSFORM(o.DisplayValue)+',newindex='+ALLTRIM(STR(o.NewIndex))+',newid='+ALLTRIM(STR(o.NewItemID))
 IF cClass='ListBox'
  c=c+',top='+ALLTRIM(STR(o.TopIndex))+',topid='+ALLTRIM(STR(o.TopItemID))
 ENDIF
 FOR nRow=1 TO o.ListCount
  c=c+';row='+ALLTRIM(STR(o.IndexToItemID(nRow)))+',data='+TRANSFORM(o.ItemData(nRow))+',selected='+TRANSFORM(o.Selected(nRow))
  FOR nCol=1 TO MAX(1,nColumns)
   c=c+',cell'+ALLTRIM(STR(nCol))+'='+TRANSFORM(o.List(nRow,nCol))
  ENDFOR
 ENDFOR
 RETURN c
ENDFUNC

FUNCTION PackedHex
 LPARAMETERS value
 LOCAL bytes,hex,digits,nByte,nValue
 digits='0123456789ABCDEF'
 bytes=BINTOC(value,'BRS')
 hex=''
 FOR nByte=1 TO LEN(bytes)
  nValue=ASC(SUBSTR(bytes,nByte,1))
  hex=hex+SUBSTR(digits,INT(nValue/16)+1,1)+SUBSTR(digits,MOD(nValue,16)+1,1)
 ENDFOR
 RETURN hex
ENDFUNC
