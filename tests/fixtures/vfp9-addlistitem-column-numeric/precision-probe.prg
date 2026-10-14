* Independent existing-ID fractional/cycle/BRS corroboration after integer storage pairs.
* No helper/default/expected-conversion table; generic observation scaffold only.
LOCAL ARRAY aValues[63], aColumns[3]
aValues[1]='32744.1'
aValues[2]='32744.9'
aValues[3]='32745'
aValues[4]='32745.1'
aValues[5]='32745.5'
aValues[6]='32745.9'
aValues[7]='32746'
aValues[8]='32746.1'
aValues[9]='32746.9'
aValues[10]='32745-2^(-38)'
aValues[11]='32745+2^(-38)'
aValues[12]='32746-2^(-38)'
aValues[13]='32746+2^(-38)'
aValues[14]='2147483647.1'
aValues[15]='2147483647.5'
aValues[16]='2147483647.9'
aValues[17]='2147483648-2^(-22)'
aValues[18]='2147483648'
aValues[19]='2147483648+2^(-21)'
aValues[20]='2147483648.1'
aValues[21]='2147483648.5'
aValues[22]='2147483648.9'
aValues[23]='2147483649'
aValues[24]='-2147483647.9'
aValues[25]='-2147483648+2^(-22)'
aValues[26]='-2147483648'
aValues[27]='-2147483648-2^(-21)'
aValues[28]='-2147483648.1'
aValues[29]='-2147483648.5'
aValues[30]='-2147483648.9'
aValues[31]='-2147483649'
aValues[32]='4294967296+32744.9'
aValues[33]='4294967296+32745.9'
aValues[34]='4294967296+32746.9'
aValues[35]='-4294967296+32744.9'
aValues[36]='-4294967296+32745.9'
aValues[37]='-4294967296+32746.9'
aValues[38]='6442450944-1'
aValues[39]='6442450944'
aValues[40]='6442450944+1'
aValues[41]='-6442450944-1'
aValues[42]='-6442450944'
aValues[43]='-6442450944+1'
aValues[44]='8589934592+32745.9'
aValues[45]='-8589934592+32745.9'
aValues[46]='1-2^(-52)'
aValues[47]='1-2^(-53)'
aValues[48]='1+2^(-52)'
aValues[49]='1+2^(-53)'
aValues[50]='4294967297.9'
aValues[51]='-4294967294.9'
aValues[52]='4294967296'
aValues[53]='-4294967296'
aValues[54]='9007199254740991'
aValues[55]='9007199254740992'
aValues[56]='9007199254740993'
aValues[57]='1E300'
aValues[58]='-1E300'
aValues[59]='1E20'
aValues[60]='-1E20'
aValues[61]='1E-300'
aValues[62]='1E300*1E300'
aValues[63]='-1E300*1E300'
aColumns[1]=0
aColumns[2]=2
aColumns[3]=8
LOCAL nClass,nConfig,nShape,nArg,nColumns,nTarget,o,cClass,cSource,cPhase
LOCAL cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow,nCol,cPreBits,cPostBits,cPreType,cPostType
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\precision-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nConfig=1 TO ALEN(aColumns)
  nColumns=aColumns[nConfig]
  FOR nShape=1 TO 1
   nTarget=IIF(nShape=0,11,IIF(nShape=1,22,44))
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
     o.RowSourceType=0
     o.ControlSource=''
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
    cLine=cClass+'|columns='+ALLTRIM(STR(nColumns))+'|shape='+ALLTRIM(STR(nShape))+'|target='+ALLTRIM(STR(nTarget))+'|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|message='+cMessage+'|preType='+cPreType+'|preBRS='+cPreBits+'|postType='+cPostType+'|postBRS='+cPostBits+'|before='+cBefore+'|after='+cAfter
    =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\precision-results.txt',1)
    nCalls=nCalls+1
    o=.NULL.
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\precision-results.txt',1)
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
