* Independent AddListItem existing-ID high-column/deferred readback capture.
* Native-only, configured2 first; bounded test expansion65538 (NOT product limit).
LOCAL ARRAY aValues[16],aRead[14]
aValues[1]='3.9'
aValues[2]='9'
aValues[3]='32767'
aValues[4]='32768'
aValues[5]='65534'
aValues[6]='65535'
aValues[7]='65535.9'
aValues[8]='65536'
aValues[9]='65536.9'
aValues[10]='65537'
aValues[11]='65537.9'
aValues[12]='65538.9'
aValues[13]='2147483647'
aValues[14]='2147483648'
aValues[15]='-2147483649'
aValues[16]='4294967297.9'
aRead[1]=1
aRead[2]=2
aRead[3]=3
aRead[4]=8
aRead[5]=9
aRead[6]=255
aRead[7]=256
aRead[8]=32767
aRead[9]=32768
aRead[10]=65534
aRead[11]=65535
aRead[12]=65536
aRead[13]=65537
aRead[14]=65538
LOCAL nClass,nArg,nRow,o,cClass,cSource,cPhase,v,nError,cReturn,cBefore,cAfter,cExpanded,nExpandError,cBits,nCalls,cOutput
cOutput='Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\high-storage-results.txt'
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),cOutput,0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nArg=1 TO ALEN(aValues)
  cSource=aValues[nArg]
  nError=0
  nExpandError=0
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
   cPhase='evaluate'
   v=EVALUATE(cSource)
   cBits=PackedHex(v)
   cPhase='invoke'
   cReturn=TRANSFORM(o.AddListItem('candidate',22,v))
  CATCH TO problem
   nError=problem.ErrorNo
  ENDTRY
  IF VARTYPE(o)='O'
   cAfter=Meta(o)
   TRY
    o.ColumnCount=65538
    cExpanded=Meta(o)+';reads='+ReadCells(o,@aRead)
   CATCH TO expandProblem
    nExpandError=expandProblem.ErrorNo
   ENDTRY
  ENDIF
  =STRTOFILE(cClass+'|target=22|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|preBRS='+cBits+'|expandError='+ALLTRIM(STR(nExpandError))+'|before='+cBefore+'|after='+cAfter+'|expanded='+cExpanded+CHR(13)+CHR(10),cOutput,1)
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

FUNCTION ReadCells
 LPARAMETERS o,aRead
 LOCAL c,n,cIndex,cItem,nIndexError,nItemError,vIndex,vItem
 c=''
 FOR n=1 TO ALEN(aRead)
  nIndexError=0
  nItemError=0
  cIndex='<not-read>'
  cItem='<not-read>'
  TRY
   vIndex=o.List(2,aRead[n])
   cIndex=TRANSFORM(vIndex)
  CATCH TO indexProblem
   nIndexError=indexProblem.ErrorNo
  ENDTRY
  TRY
   vItem=o.ListItem(22,aRead[n])
   cItem=TRANSFORM(vItem)
  CATCH TO itemProblem
   nItemError=itemProblem.ErrorNo
  ENDTRY
  c=c+'[column='+ALLTRIM(STR(aRead[n]))+',indexError='+ALLTRIM(STR(nIndexError))+',index='+cIndex+',itemError='+ALLTRIM(STR(nItemError))+',item='+cItem+']'
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
