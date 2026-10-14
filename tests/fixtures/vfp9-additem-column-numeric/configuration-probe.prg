* Independent THIRD-column configuration/position corroboration, no expectations.
* Configure before population. Ordinary positions1/2/4 are independently supplied.
LOCAL ARRAY aValues[12], aColumns[3], aPositions[3]
aValues[1]='1.9'
aValues[2]='2.9'
aValues[3]='8.9'
aValues[4]='65536.9'
aValues[5]='65537.9'
aValues[6]='2147483648'
aValues[7]='2147483649'
aValues[8]='-2147483648'
aValues[9]='-2147483649'
aValues[10]='4294967297.9'
aValues[11]='-4294967295.9'
aValues[12]='-1.9'
aColumns[1]=0
aColumns[2]=2
aColumns[3]=8
aPositions[1]=1
aPositions[2]=2
aPositions[3]=4
LOCAL nClass,nColShape,nConfig,nPos,nArg,nColumns,nPosition,o,cClass,cSource,cPhase,cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow,nCol,cBits
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-column-numeric-5611\Project-Copperfin\build\additem-column-native\configuration-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nColShape=1 TO ALEN(aColumns)
  nColumns=aColumns[nColShape]
  FOR nConfig=0 TO 3
   FOR nPos=1 TO ALEN(aPositions)
    nPosition=aPositions[nPos]
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
      IF nConfig>0
       o.RowSourceType=0
       o.ControlSource=''
      ENDIF
      IF nConfig=2
       o.ControlSource='   '
      ENDIF
      IF nConfig=3
       o.Sorted=.T.
      ENDIF
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
      cPhase='invoke'
      cReturn=TRANSFORM(o.AddItem('candidate',nPosition,v))
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
     cLine=cClass+'|columns='+ALLTRIM(STR(nColumns))+'|config='+ALLTRIM(STR(nConfig))+',position='+ALLTRIM(STR(nPosition))+'|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|message='+cMessage+'|vartype='+VARTYPE(v)+'|postcallBRS='+cBits+'|before='+cBefore+'|after='+cAfter
     =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-column-numeric-5611\Project-Copperfin\build\additem-column-native\configuration-results.txt',1)
     nCalls=nCalls+1
     o=.NULL.
    ENDFOR
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-column-numeric-5611\Project-Copperfin\build\additem-column-native\configuration-results.txt',1)
RETURN

FUNCTION Snapshot
 LPARAMETERS o,cClass,nColumns
 LOCAL c,nRow,nCol
 c='count='+ALLTRIM(STR(o.ListCount))+',columns='+ALLTRIM(STR(o.ColumnCount))+',rst='+TRANSFORM(o.RowSourceType)+',binding=['+o.ControlSource+'],sorted='+TRANSFORM(o.Sorted)+',focus='+ALLTRIM(STR(o.ListIndex))+',id='+ALLTRIM(STR(o.ListItemID))+',value='+TRANSFORM(o.Value)+',display='+TRANSFORM(o.DisplayValue)+',newindex='+ALLTRIM(STR(o.NewIndex))+',newid='+ALLTRIM(STR(o.NewItemID))
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
