* Independent refinement after observed 32000 stored / 32760 empty.
* Generic observation scaffold only; no acceptance expectations.
LOCAL ARRAY aValues[16]
aValues[1]=32700
aValues[2]=32720
aValues[3]=32730
aValues[4]=32740
aValues[5]=32748
aValues[6]=32749
aValues[7]=32750
aValues[8]=32751
aValues[9]=32752
aValues[10]=32753
aValues[11]=32754
aValues[12]=32755
aValues[13]=32756
aValues[14]=32757
aValues[15]=32758
aValues[16]=32759
LOCAL nClass,nRows,nTarget,nArg,nColumn,o,cClass,cPhase,nError,nExpandError,cReturn,cBefore,cAfter,cExpanded,nCalls,nRow,cOutput,nID
cOutput='Z:\home\rich\.codex\worktrees\addlistitem-column-numeric-5611\Project-Copperfin\build\addlistitem-column-native\storage-refinement-results.txt'
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),cOutput,0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nRows=1 TO 5 STEP 2
  FOR nTarget=1 TO nRows
   FOR nArg=1 TO ALEN(aValues)
    nColumn=aValues[nArg]
    nID=nTarget*11
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
     FOR nRow=1 TO nRows
      o.AddListItem('row'+ALLTRIM(STR(nRow)),nRow*11)
      o.List(nRow,2)='cell'+ALLTRIM(STR(nRow))+':2'
      o.ItemData(nRow)=nRow*101
     ENDFOR
     o.ListIndex=nTarget
     cBefore=Meta(o)
     cPhase='invoke'
     cReturn=TRANSFORM(o.AddListItem('candidate',nID,nColumn))
    CATCH TO problem
     nError=problem.ErrorNo
    ENDTRY
    IF VARTYPE(o)='O'
     cAfter=Meta(o)+';request='+ReadCell(o,nTarget,nID,nColumn)
     TRY
      o.ColumnCount=nColumn
      cExpanded=Meta(o)+';request='+ReadCell(o,nTarget,nID,nColumn)+';first='+ReadCell(o,nTarget,nID,1)+';second='+ReadCell(o,nTarget,nID,2)
     CATCH TO expandProblem
      nExpandError=expandProblem.ErrorNo
     ENDTRY
    ENDIF
    =STRTOFILE(cClass+'|rows='+ALLTRIM(STR(nRows))+'|targetRow='+ALLTRIM(STR(nTarget))+'|targetID='+ALLTRIM(STR(nID))+'|column='+ALLTRIM(STR(nColumn))+'|type='+VARTYPE(nColumn)+'|BRS='+HexBits(nColumn)+'|'+cPhase+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|expandError='+ALLTRIM(STR(nExpandError))+'|before='+cBefore+'|after='+cAfter+'|expanded='+cExpanded+CHR(13)+CHR(10),cOutput,1)
    nCalls=nCalls+1
    o=.NULL.
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),cOutput,1)
RETURN

FUNCTION Meta
 LPARAMETERS o
 RETURN 'count='+ALLTRIM(STR(o.ListCount))+',columns='+ALLTRIM(STR(o.ColumnCount))+',focus='+ALLTRIM(STR(o.ListIndex))+',id='+ALLTRIM(STR(o.ListItemID))+',value='+TRANSFORM(o.Value)+',display='+TRANSFORM(o.DisplayValue)+',newindex='+ALLTRIM(STR(o.NewIndex))+',newid='+ALLTRIM(STR(o.NewItemID))
ENDFUNC

FUNCTION ReadCell
 LPARAMETERS o,nTarget,nID,nColumn
 LOCAL cIndex,cItem,nIndexError,nItemError,vIndex,vItem
 nIndexError=0
 nItemError=0
 cIndex='<not-read>'
 cItem='<not-read>'
 TRY
  vIndex=o.List(nTarget,nColumn)
  cIndex=TRANSFORM(vIndex)
 CATCH TO indexProblem
  nIndexError=indexProblem.ErrorNo
 ENDTRY
 TRY
  vItem=o.ListItem(nID,nColumn)
  cItem=TRANSFORM(vItem)
 CATCH TO itemProblem
  nItemError=itemProblem.ErrorNo
 ENDTRY
 RETURN '[indexError='+ALLTRIM(STR(nIndexError))+',index='+cIndex+',itemError='+ALLTRIM(STR(nItemError))+',item='+cItem+']'
ENDFUNC

FUNCTION HexBits
 LPARAMETERS v
 LOCAL cBytes,cHex,nByte
 cBytes=BINTOC(v,'BRS')
 cHex=''
 FOR nByte=1 TO LEN(cBytes)
  cHex=cHex+RIGHT(TRANSFORM(ASC(SUBSTR(cBytes,nByte,1)),'@0'),2)
 ENDFOR
 RETURN cHex
ENDFUNC
