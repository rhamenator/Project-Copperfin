* Evaluation-only native precision observations; not consumer acceptance.
LOCAL ARRAY aValues[6]
aValues[1] = '0.9999999999999999'
aValues[2] = '1-2^(-52)'
aValues[3] = '1-2^(-53)'
aValues[4] = '32768-2^(-38)'
aValues[5] = '32768-.00000000001'
aValues[6] = '32767.999'
LOCAL n,cSource,v,cLine
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\precision-results.txt',0)
FOR n=1 TO ALEN(aValues)
 cSource=aValues[n]
 v=EVALUATE(cSource)
 cLine=cSource+'|one='+TRANSFORM(v=1)+'|belowone='+TRANSFORM(v<1)+'|upper='+TRANSFORM(v=32768)+'|belowupper='+TRANSFORM(v<32768)
 =STRTOFILE(cLine+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\precision-results.txt',1)
ENDFOR
=STRTOFILE('DONE|6'+CHR(13)+CHR(10),'Z:\home\rich\.codex\worktrees\addlistitem-numeric-5611\Project-Copperfin\build\addlistitem-native\precision-results.txt',1)
RETURN
