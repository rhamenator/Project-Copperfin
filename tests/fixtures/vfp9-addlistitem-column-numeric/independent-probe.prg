* Independent installed VFP9 AddListItem THIRD-column observations.
* No Copperfin call/helper/expectation table, default or neighboring-column model.
LOCAL ARRAY aValues[54], aColumns[3]
aValues[1]='-3'
aValues[2]='-1.9'
aValues[3]='-0.9'
aValues[4]='0'
aValues[5]='0.5'
aValues[6]='0.9'
aValues[7]='1'
aValues[8]='1.4'
aValues[9]='1.5'
aValues[10]='1.9'
aValues[11]='2'
aValues[12]='2.5'
aValues[13]='2.9'
aValues[14]='3.9'
aValues[15]='8.9'
aValues[16]='127'
aValues[17]='255'
aValues[18]='256'
aValues[19]='32767'
aValues[20]='32768'
aValues[21]='65534'
aValues[22]='65535'
aValues[23]='65535.9'
aValues[24]='65536'
aValues[25]='65536.9'
aValues[26]='65537'
aValues[27]='65537.9'
aValues[28]='65538.9'
aValues[29]='2147483647'
aValues[30]='2147483647.9'
aValues[31]='2147483648'
aValues[32]='2147483648.9'
aValues[33]='2147483649'
aValues[34]='4294967295'
aValues[35]='4294967296'
aValues[36]='4294967297.9'
aValues[37]='4294967298.9'
aValues[38]='-2147483648'
aValues[39]='-2147483648.9'
aValues[40]='-2147483649'
aValues[41]='-4294967295.9'
aValues[42]='-4294967294.9'
aValues[43]='-4294967296'
aValues[44]='1E300'
aValues[45]='-1E300'
aValues[46]='1E20'
aValues[47]='-1E20'
aValues[48]='1E-300'
aValues[49]='(1E300*1E300)'
aValues[50]='(-1E300*1E300)'
aValues[51]='1-2^(-52)'
aValues[52]='1-2^(-53)'
aValues[53]='1+2^(-52)'
aValues[54]='2147483648-2^(-22)'
aColumns[1]=0
aColumns[2]=2
aColumns[3]=8
LOCAL nClass,nConfig,nShape,nArg,nColumns,nTarget,o,cClass,cSource,cPhase
LOCAL cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow,nCol,cPreBits,cPostBits,cPreType,cPostType
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\native-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nConfig=1 TO ALEN(aColumns)
  nColumns=aColumns[nConfig]
  FOR nShape=0 TO 2
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
    =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\native-results.txt',1)
    nCalls=nCalls+1
    o=.NULL.
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\native-results.txt',1)
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
