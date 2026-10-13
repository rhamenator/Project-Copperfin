* Evaluation-only native precision observations; not consumer acceptance.
LOCAL ARRAY aValues[6]
aValues[1] = '0.9999999999999999'
aValues[2] = '1-2^(-52)'
aValues[3] = '1-2^(-53)'
aValues[4] = '32768-2^(-38)'
aValues[5] = '32768-.00000000001'
aValues[6] = '32767.999'
LOCAL n,cSource,v,cLine,cBytes,cHex,nByte,nValue,cDigits
cDigits='0123456789ABCDEF'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\precision-bits-results.txt',0)
FOR n=1 TO ALEN(aValues)
 cSource=aValues[n]
 v=EVALUATE(cSource)
 cBytes=BINTOC(v,'BRS')
 cHex=''
 FOR nByte=1 TO LEN(cBytes)
  nValue=ASC(SUBSTR(cBytes,nByte,1))
  cHex=cHex+SUBSTR(cDigits,INT(nValue/16)+1,1)+SUBSTR(cDigits,MOD(nValue,16)+1,1)
 ENDFOR
 cLine=cSource+'|bytes='+ALLTRIM(STR(LEN(cBytes)))+'|BRShex='+cHex+'|one='+TRANSFORM(v=1)+'|belowone='+TRANSFORM(v<1)+'|upper='+TRANSFORM(v=32768)+'|belowupper='+TRANSFORM(v<32768)
 =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\precision-bits-results.txt',1)
ENDFOR
=STRTOFILE('DONE|6'+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\precision-bits-results.txt',1)
RETURN
