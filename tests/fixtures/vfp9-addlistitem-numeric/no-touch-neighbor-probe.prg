* Independent installed VFP9 AddListItem observations; NO acceptance assertions.
* Selected second argument, Character item, omitted third column; generic capture only.
LOCAL ARRAY aValues[35]
aValues[1] = '1-2^(-50)'
aValues[2] = '1-2^(-51)'
aValues[3] = '1-2^(-52)'
aValues[4] = '1-2^(-53)'
aValues[5] = '1-2^(-54)'
aValues[6] = '2-2^(-50)'
aValues[7] = '2-2^(-51)'
aValues[8] = '2-2^(-52)'
aValues[9] = '3-2^(-49)'
aValues[10] = '3-2^(-50)'
aValues[11] = '3-2^(-51)'
aValues[12] = '4-2^(-49)'
aValues[13] = '4-2^(-50)'
aValues[14] = '4-2^(-51)'
aValues[15] = '10-2^(-47)'
aValues[16] = '10-2^(-48)'
aValues[17] = '10-2^(-49)'
aValues[18] = '11-2^(-47)'
aValues[19] = '11-2^(-48)'
aValues[20] = '11-2^(-49)'
aValues[21] = '16-2^(-47)'
aValues[22] = '16-2^(-48)'
aValues[23] = '16-2^(-49)'
aValues[24] = '100-2^(-44)'
aValues[25] = '100-2^(-45)'
aValues[26] = '100-2^(-46)'
aValues[27] = '1000-2^(-41)'
aValues[28] = '1000-2^(-42)'
aValues[29] = '1000-2^(-43)'
aValues[30] = '32767-2^(-36)'
aValues[31] = '32767-2^(-37)'
aValues[32] = '32767-2^(-38)'
aValues[33] = '32768-2^(-36)'
aValues[34] = '32768-2^(-37)'
aValues[35] = '32768-2^(-38)'
LOCAL nClass,nShape,nArg,o,cClass,cSource,cPhase,cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow
LOCAL cBytes,cHex,cDigits,nByte,nValue
cDigits='0123456789ABCDEF'
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10), 'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\no-touch-neighbor-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nShape=0 TO 0
  FOR nArg=1 TO ALEN(aValues)
   cSource=aValues[nArg]
   nError=0
   cMessage=''
   cReturn='<not-called>'
   cHex='<not-evaluated>'
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
    cBytes=BINTOC(v,'BRS')
    cHex=''
    FOR nByte=1 TO LEN(cBytes)
     nValue=ASC(SUBSTR(cBytes,nByte,1))
     cHex=cHex+SUBSTR(cDigits,INT(nValue/16)+1,1)+SUBSTR(cDigits,MOD(nValue,16)+1,1)
    ENDFOR
   IF VARTYPE(o)='O'
    cAfter=Snapshot(o,cClass)
   ENDIF
   cLine=cClass+'|shape='+ALLTRIM(STR(nShape))+'|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|BRShex='+cHex+'|return='+cReturn+'|message='+cMessage+'|before='+cBefore+'|after='+cAfter
   =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\no-touch-neighbor-results.txt',1)
   nCalls=nCalls+1
   o=.NULL.
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\no-touch-neighbor-results.txt',1)
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
