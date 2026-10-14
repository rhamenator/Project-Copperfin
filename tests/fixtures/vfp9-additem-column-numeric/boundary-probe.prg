* Independent installed VFP9 AddItem THIRD column capture; no expectations/helper.
* Generic observation scaffold only; no second-position or sibling policy table.
LOCAL ARRAY aValues[36], aColumns[3]
aValues[1]='0.9999999999999999'
aValues[2]='1-2^(-52)'
aValues[3]='1-2^(-53)'
aValues[4]='1+2^(-52)'
aValues[5]='2-2^(-52)'
aValues[6]='3.9999999999999996'
aValues[7]='4-2^(-51)'
aValues[8]='8.999999999999998'
aValues[9]='65535.9'
aValues[10]='65536.9'
aValues[11]='65537.9'
aValues[12]='65538.9'
aValues[13]='-65535.9'
aValues[14]='-65534.9'
aValues[15]='2147483647.9'
aValues[16]='2147483648'
aValues[17]='2147483648.9'
aValues[18]='2147483649'
aValues[19]='3221225472'
aValues[20]='4294967293'
aValues[21]='4294967294'
aValues[22]='4294967294.9'
aValues[23]='4294967295.9'
aValues[24]='-2147483648'
aValues[25]='-2147483649'
aValues[26]='-3221225472'
aValues[27]='-4294967293'
aValues[28]='-4294967294'
aValues[29]='-4294967295'
aValues[30]='-4294967296'
aValues[31]='-4294967297'
aValues[32]='8589934593'
aValues[33]='-8589934591'
aValues[34]='9007199254740991'
aValues[35]='-9007199254740991'
aValues[36]='9007199254740992'
aColumns[1]=0
aColumns[2]=2
aColumns[3]=8
LOCAL nClass,nColShape,nShape,nArg,nColumns,o,cClass,cSource,cPhase,cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow,nCol,cBits
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-column-numeric-5611\Project-Copperfin\build\additem-column-native\boundary-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nColShape=1 TO ALEN(aColumns)
  nColumns=aColumns[nColShape]
  FOR nShape=0 TO 1
   FOR nArg=1 TO ALEN(aValues)
    cSource=aValues[nArg]
    nError=0
    cMessage=''
    cReturn='<not-called>'
    cBefore='<setup-incomplete>'
    cAfter='<setup-incomplete>'
    cBits='<not-evaluated>'
    cPhase='setup'
    o=.NULL.
    TRY
     o=CREATEOBJECT(cClass)
     o.RowSourceType=0
     o.ControlSource=''
     o.ColumnCount=nColumns
     IF nShape=1
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
     cPhase='invoke'
     cReturn=TRANSFORM(o.AddItem('candidate',1,v))
    CATCH TO problem
     nError=problem.ErrorNo
     cMessage=STRTRAN(STRTRAN(problem.Message,CHR(13),' '),CHR(10),' ')
    ENDTRY
    IF VARTYPE(o)='O'
     cAfter=Snapshot(o,cClass,nColumns)
    ENDIF
    IF cPhase='invoke'
     cBits=PackedHex(v)
    ENDIF
    cLine=cClass+'|columns='+ALLTRIM(STR(nColumns))+'|shape='+ALLTRIM(STR(nShape))+'|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|message='+cMessage+'|vartype='+VARTYPE(v)+'|postcallBRS='+cBits+'|before='+cBefore+'|after='+cAfter
    =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-column-numeric-5611\Project-Copperfin\build\additem-column-native\boundary-results.txt',1)
    nCalls=nCalls+1
    o=.NULL.
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-column-numeric-5611\Project-Copperfin\build\additem-column-native\boundary-results.txt',1)
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
