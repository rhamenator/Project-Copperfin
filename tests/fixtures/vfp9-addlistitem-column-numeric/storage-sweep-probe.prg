* Independent native existing-ID storage/readback sweep, no expectations.
* Read request both before and after bounded expansion MAX(9,column)<=32766.
LOCAL ARRAY aValues[25]
aValues[1]=1
aValues[2]=2
aValues[3]=3
aValues[4]=8
aValues[5]=9
aValues[6]=127
aValues[7]=128
aValues[8]=255
aValues[9]=256
aValues[10]=257
aValues[11]=511
aValues[12]=512
aValues[13]=1023
aValues[14]=1024
aValues[15]=2047
aValues[16]=2048
aValues[17]=4095
aValues[18]=4096
aValues[19]=8191
aValues[20]=8192
aValues[21]=16383
aValues[22]=16384
aValues[23]=21845
aValues[24]=21846
aValues[25]=32766
LOCAL nClass,nArg,nColumn,o,cClass,cPhase,nError,nExpandError,cReturn,cBefore,cAfter,cExpanded,nCalls,nRow,cOutput
cOutput='Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\storage-sweep-results.txt'
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),cOutput,0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nArg=1 TO ALEN(aValues)
  nColumn=aValues[nArg]
  nError=0
  nExpandError=0
  cReturn='<not-called>'
  cBefore='<setup-incomplete>'
  cAfter='<setup-incomplete>'
  cExpanded='<not-expanded>'
  cPhase='setup'
  o=.NULL.
  TRY
   o=CREATEOBJECT(cClass)
   o.RowSourceType=0
   o.ControlSource=''
   o.ColumnCount=2
   o.AddListItem('first',11)
   o.AddListItem('second',22)
   o.AddListItem('third',33)
   FOR nRow=1 TO o.ListCount
    o.List(nRow,2)='cell'+ALLTRIM(STR(nRow))+':2'
    o.ItemData(nRow)=nRow*101
   ENDFOR
   o.ListIndex=2
   cBefore=Meta(o)
   cPhase='invoke'
   cReturn=TRANSFORM(o.AddListItem('candidate',22,nColumn))
  CATCH TO problem
   nError=problem.ErrorNo
  ENDTRY
  IF VARTYPE(o)='O'
   cAfter=Meta(o)+';request='+ReadCell(o,nColumn)
   TRY
    o.ColumnCount=MAX(9,nColumn)
    cExpanded=Meta(o)+';request='+ReadCell(o,nColumn)+';first='+ReadCell(o,1)+';second='+ReadCell(o,2)
   CATCH TO expandProblem
    nExpandError=expandProblem.ErrorNo
   ENDTRY
  ENDIF
  =STRTOFILE(cClass+'|target=22|column='+ALLTRIM(STR(nColumn))+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|expandError='+ALLTRIM(STR(nExpandError))+'|before='+cBefore+'|after='+cAfter+'|expanded='+cExpanded+CHR(13)+CHR(10),cOutput,1)
  nCalls=nCalls+1
  o=.NULL.
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),cOutput,1)
RETURN

FUNCTION Meta
 LPARAMETERS o
 RETURN 'count='+ALLTRIM(STR(o.ListCount))+',columns='+ALLTRIM(STR(o.ColumnCount))+',focus='+ALLTRIM(STR(o.ListIndex))+',id='+ALLTRIM(STR(o.ListItemID))+',value='+TRANSFORM(o.Value)+',display='+TRANSFORM(o.DisplayValue)+',newindex='+ALLTRIM(STR(o.NewIndex))+',newid='+ALLTRIM(STR(o.NewItemID))
ENDFUNC

FUNCTION ReadCell
 LPARAMETERS o,nColumn
 LOCAL cIndex,cItem,nIndexError,nItemError,vIndex,vItem
 nIndexError=0
 nItemError=0
 cIndex='<not-read>'
 cItem='<not-read>'
 TRY
  vIndex=o.List(2,nColumn)
  cIndex=TRANSFORM(vIndex)
 CATCH TO indexProblem
  nIndexError=indexProblem.ErrorNo
 ENDTRY
 TRY
  vItem=o.ListItem(22,nColumn)
  cItem=TRANSFORM(vItem)
 CATCH TO itemProblem
  nItemError=itemProblem.ErrorNo
 ENDTRY
 RETURN '[indexError='+ALLTRIM(STR(nIndexError))+',index='+cIndex+',itemError='+ALLTRIM(STR(nItemError))+',item='+cItem+']'
ENDFUNC
