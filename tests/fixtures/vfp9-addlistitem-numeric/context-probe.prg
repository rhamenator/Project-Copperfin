* Native diagnostic of capture placement; NO helper or acceptance assertions.
LOCAL ARRAY aSources[3]
aSources[1]='1-2^(-52)'
aSources[2]='2-2^(-52)'
aSources[3]='32768-2^(-38)'
LOCAL nClass,nSource,nVariant,cClass,cSource,o,v,lFlag,cBeforeHex,cAfterHex,nError,cReturn,cLine,nCalls
nCalls=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\context-results.txt',0)
FOR nClass=1 TO 2
 cClass=IIF(nClass=1,'ListBox','ComboBox')
 FOR nSource=1 TO 3
  cSource=aSources[nSource]
  FOR nVariant=1 TO 4
   o=CREATEOBJECT(cClass)
   o.RowSourceType=0
   o.ControlSource=''
   v=EVALUATE(cSource)
   cBeforeHex='<not-captured>'
   cAfterHex='<not-captured>'
   IF nVariant=3 OR nVariant=4
    cBeforeHex=PackedHex(v)
   ENDIF
   lFlag=.F.
   IF nVariant=2 OR nVariant=4
    lFlag=(v=1)
   ENDIF
   IF nVariant=4
    cAfterHex=PackedHex(v)
   ENDIF
   nError=0
   cReturn='<not-called>'
   TRY
    cReturn=TRANSFORM(o.AddListItem('candidate',v))
   CATCH TO problem
    nError=problem.ErrorNo
   ENDTRY
   cLine=cClass+'|'+cSource+'|variant='+ALLTRIM(STR(nVariant))+'|flag='+TRANSFORM(lFlag)+'|beforehex='+cBeforeHex+'|aftercomparisonhex='+cAfterHex+'|postcallhex='+PackedHex(v)+'|error='+ALLTRIM(STR(nError))+'|return='+cReturn+'|count='+ALLTRIM(STR(o.ListCount))+'|newid='+ALLTRIM(STR(o.NewItemID))
   =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\context-results.txt',1)
   nCalls=nCalls+1
   o=.NULL.
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(nCalls))+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\context-results.txt',1)
RETURN
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
