* Independent installed VFP9 AddItem observations; NO acceptance assertions.
* Selected second argument, Character item, omitted third column; generic capture only. Input values/scaffold reused, NO sibling expectation or policy.
LOCAL ARRAY aValues[28]
aValues[1] = '0.9999999999999999'
aValues[2] = '1-2^(-52)'
aValues[3] = '1-2^(-53)'
aValues[4] = '1+2^(-52)'
aValues[5] = '2-2^(-52)'
aValues[6] = '2.0000000000000004'
aValues[7] = '3.9999999999999996'
aValues[8] = '4-2^(-51)'
aValues[9] = '4.999999999999999'
aValues[10] = '5-2^(-50)'
aValues[11] = '5'
aValues[12] = '5.000000000000001'
aValues[13] = '4294967297.9'
aValues[14] = '4294967298.9'
aValues[15] = '4294967299.9'
aValues[16] = '4294967300.9'
aValues[17] = '4294967301'
aValues[18] = '-4294967295.9'
aValues[19] = '-4294967294.9'
aValues[20] = '-4294967293.9'
aValues[21] = '-4294967292.9'
aValues[22] = '-4294967291.9'
aValues[23] = '8589934593'
aValues[24] = '-8589934591.9'
aValues[25] = '1E20'
aValues[26] = '-1E20'
aValues[27] = '(1E300*1E300)'
aValues[28] = '(-1E300*1E300)'
LOCAL nClass,nShape,nArg,o,cClass,cSource,cPhase,cBefore,cAfter,nError,cMessage,v,cReturn,cLine,nCalls,nRow
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10), 'Z:\home\rich\.codex\worktrees\additem-numeric-5611\Project-Copperfin\build\additem-native\boundary-postbits-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nShape=0 TO 3
  FOR nArg=1 TO ALEN(aValues)
   cSource=aValues[nArg]
   nError=0
   cMessage=''
   cReturn='<not-called>'
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
    cReturn=TRANSFORM(o.AddItem('candidate',v))
   CATCH TO problem
    nError=problem.ErrorNo
    cMessage=STRTRAN(STRTRAN(problem.Message,CHR(13),' '),CHR(10),' ')
   ENDTRY
   IF VARTYPE(o)='O'
    cAfter=Snapshot(o,cClass)
   ENDIF
   cLine=cClass+'|shape='+ALLTRIM(STR(nShape))+'|'+cSource+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|message='+cMessage+'|postcallBRS='+PackedHex(v)+'|before='+cBefore+'|after='+cAfter
   =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-numeric-5611\Project-Copperfin\build\additem-native\boundary-postbits-results.txt',1)
   nCalls=nCalls+1
   o=.NULL.
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\additem-numeric-5611\Project-Copperfin\build\additem-native\boundary-postbits-results.txt',1)
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
