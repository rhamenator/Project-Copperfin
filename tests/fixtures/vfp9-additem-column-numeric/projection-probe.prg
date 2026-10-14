* Independent THIRD-column precise signed32/cycle/large-range observation.
* No helper, expected behavior or acceptance constants.
LOCAL ARRAY aValues[36], aColumns[3]
aValues[1]='2147483647.1'
aValues[2]='2147483647.5'
aValues[3]='2147483647.9999998'
aValues[4]='2147483648-2^(-22)'
aValues[5]='2147483648'
aValues[6]='2147483648.1'
aValues[7]='2147483648.5'
aValues[8]='2147483648.9'
aValues[9]='2147483649'
aValues[10]='-2147483648+0.5'
aValues[11]='-2147483648'
aValues[12]='-2147483648.1'
aValues[13]='-2147483648.5'
aValues[14]='-2147483648.9'
aValues[15]='-2147483649'
aValues[16]='6442450944'
aValues[17]='6442450944.9'
aValues[18]='6442450945'
aValues[19]='-6442450944'
aValues[20]='-6442450944.9'
aValues[21]='-6442450945'
aValues[22]='4294967295.9999995'
aValues[23]='4294967296'
aValues[24]='4294967296.9'
aValues[25]='4294967297'
aValues[26]='-4294967295.9999995'
aValues[27]='-4294967296'
aValues[28]='-4294967296.9'
aValues[29]='-4294967297'
aValues[30]='2^63-1024'
aValues[31]='2^63'
aValues[32]='2^63+2048'
aValues[33]='-2^63'
aValues[34]='-2^63-2048'
aValues[35]='2^64'
aValues[36]='-2^64'
aColumns[1]=0
aColumns[2]=2
aColumns[3]=8
LOCAL nClass,nColShape,nShape,nArg,nColumns,o,cClass,cSource,cPhase,cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow,nCol,cBits
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-column-numeric-5611\Project-Copperfin\build\additem-column-native\projection-results.txt',0)
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
    =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-column-numeric-5611\Project-Copperfin\build\additem-column-native\projection-results.txt',1)
    nCalls=nCalls+1
    o=.NULL.
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-column-numeric-5611\Project-Copperfin\build\additem-column-native\projection-results.txt',1)
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
