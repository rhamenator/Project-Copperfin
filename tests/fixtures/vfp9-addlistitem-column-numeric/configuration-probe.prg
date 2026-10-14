* Independent default/explicit RST0, trim-empty binding and Sorted corroboration.
* Generic native observation scaffold only; no helper/expected-conversion table.
LOCAL ARRAY aValues[12], aColumns[3]
aValues[1]='-0.9'
aValues[2]='1.9'
aValues[3]='2.9'
aValues[4]='3.9'
aValues[5]='32745.9'
aValues[6]='32746.9'
aValues[7]='65537.9'
aValues[8]='2147483648.9'
aValues[9]='-2147483648.9'
aValues[10]='4294967298.9'
aValues[11]='-4294967294.9'
aValues[12]='1E300'
aColumns[1]=0
aColumns[2]=2
aColumns[3]=8
LOCAL nClass,nConfig,nShape,nArg,nColumns,nTarget,o,cClass,cSource,cPhase
LOCAL cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow,nCol,cPreBits,cPostBits,cPreType,cPostType
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\configuration-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nConfig=1 TO ALEN(aColumns)
  nColumns=aColumns[nConfig]
  FOR nShape=1 TO 4
   nTarget=22
   FOR nArg=1 TO ALEN(aValues)
    cSource=aValues[nArg]
    nError=0
    cMessage=''
    cReturn='<not-called>'
    cBefore='<setup-incomplete>'
    cAfter='<setup-incomplete>'
    cPreBits='<not-evaluated>'
    cPostBits='<not-evaluated>'
    cPreType='<not-evaluated>'
    cPostType='<not-evaluated>'
    cPhase='setup'
    o=.NULL.
    TRY
     o=CREATEOBJECT(cClass)
     IF nShape>1
      o.RowSourceType=0
     ENDIF
     o.ControlSource=IIF(nShape>=3,'   ','')
     o.Sorted=(nShape=4)
     o.ColumnCount=nColumns
     IF nShape>0
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
     ENDIF
     cBefore=Snapshot(o,cClass,nColumns)
     cPhase='evaluate'
     v=EVALUATE(cSource)
     cPreType=VARTYPE(v)
     cPreBits=PackedHex(v)
     cPhase='invoke'
     cReturn=TRANSFORM(o.AddListItem('candidate',nTarget,v))
    CATCH TO problem
     nError=problem.ErrorNo
     cMessage=STRTRAN(STRTRAN(problem.Message,CHR(13),' '),CHR(10),' ')
    ENDTRY
    IF VARTYPE(o)='O'
     cAfter=Snapshot(o,cClass,nColumns)
    ENDIF
    IF cPhase='invoke'
     cPostType=VARTYPE(v)
     cPostBits=PackedHex(v)
    ENDIF
    cLine=cClass+'|columns='+ALLTRIM(STR(nColumns))+'|shape='+ALLTRIM(STR(nShape))+'|target='+ALLTRIM(STR(nTarget))+'|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|message='+cMessage+'|preType='+cPreType+'|preBRS='+cPreBits+'|postType='+cPostType+'|postBRS='+cPostBits+'|rst='+TRANSFORM(o.RowSourceType)+'|binding='+TRANSFORM(o.ControlSource)+'|sorted='+TRANSFORM(o.Sorted)+'|before='+cBefore+'|after='+cAfter
    =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\configuration-results.txt',1)
    nCalls=nCalls+1
    o=.NULL.
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\configuration-results.txt',1)
RETURN

FUNCTION Snapshot
 LPARAMETERS o,cClass,nColumns
 LOCAL ARRAY aReads[7]
 aReads[1]=1
 aReads[2]=2
 aReads[3]=3
 aReads[4]=32744
 aReads[5]=32745
 aReads[6]=32746
 aReads[7]=65537
 LOCAL c,nRow,nCol,nRead,nIndexError,nItemError,vIndex,vItem
 c='count='+ALLTRIM(STR(o.ListCount))+',columns='+ALLTRIM(STR(o.ColumnCount))+',focus='+ALLTRIM(STR(o.ListIndex))+',id='+ALLTRIM(STR(o.ListItemID))+',value='+TRANSFORM(o.Value)+',display='+TRANSFORM(o.DisplayValue)+',newindex='+ALLTRIM(STR(o.NewIndex))+',newid='+ALLTRIM(STR(o.NewItemID))
 IF cClass='ListBox'
  c=c+',top='+ALLTRIM(STR(o.TopIndex))+',topid='+ALLTRIM(STR(o.TopItemID))
 ENDIF
 FOR nRow=1 TO o.ListCount
  c=c+';row='+ALLTRIM(STR(o.IndexToItemID(nRow)))+',data='+TRANSFORM(o.ItemData(nRow))+',selected='+TRANSFORM(o.Selected(nRow))
  FOR nRead=1 TO ALEN(aReads)
   nCol=aReads[nRead]
   nIndexError=0
   nItemError=0
   vIndex='<not-read>'
   vItem='<not-read>'
   TRY
    vIndex=o.List(nRow,nCol)
   CATCH TO indexProblem
    nIndexError=indexProblem.ErrorNo
   ENDTRY
   TRY
    vItem=o.ListItem(o.IndexToItemID(nRow),nCol)
   CATCH TO itemProblem
    nItemError=itemProblem.ErrorNo
   ENDTRY
   c=c+'[column='+ALLTRIM(STR(nCol))+',indexError='+ALLTRIM(STR(nIndexError))+',index='+TRANSFORM(vIndex)+',itemError='+ALLTRIM(STR(nItemError))+',item='+TRANSFORM(vItem)+']'
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
