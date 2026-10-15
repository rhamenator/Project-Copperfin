* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Plain decimal boundary recovery; generated literals are retained in full.
LOCAL ARRAY expressions[27]
expressions[1]='1.'
expressions[2]='0.'
expressions[3]='0001.'
expressions[4]='1.0'
expressions[5]='1.'+REPLICATE('0',17)
expressions[6]='1.'+REPLICATE('0',18)
expressions[7]='1.'+REPLICATE('0',19)
expressions[8]='1.'+REPLICATE('0',38)
expressions[9]='1.'+REPLICATE('0',39)
expressions[10]='1.'+REPLICATE('0',40)
expressions[11]='1.'+REPLICATE('0',41)
expressions[12]='1.'+REPLICATE('0',100)
expressions[13]='1.'+REPLICATE('0',300)
expressions[14]=REPLICATE('0',10)+'.1'
expressions[15]=REPLICATE('0',38)+'.1'
expressions[16]=REPLICATE('0',39)+'.1'
expressions[17]=REPLICATE('0',40)+'.1'
expressions[18]=REPLICATE('0',41)+'.1'
expressions[19]=REPLICATE('0',100)+'.1'
expressions[20]=REPLICATE('0',300)+'.1'
expressions[21]='1'+REPLICATE('0',38)+'.0'
expressions[22]='1'+REPLICATE('0',39)+'.0'
expressions[23]='1'+REPLICATE('0',40)+'.0'
expressions[24]='1'+REPLICATE('0',100)+'.0'
expressions[25]='1'+REPLICATE('0',300)+'.0'
expressions[26]='(1.00)'
expressions[27]='  1.00  '
LOCAL outputPath,libraryPath,index,expression,value,line,directMeta,directBits,total
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-metadata-5611\Project-Copperfin\build\numeric-literal-native\plain-boundary-results.txt'
libraryPath='Z:\home\rich\.local\share\Trash\files\listitem-column-numeric-5611-build\listitem-column-native\cause\cause-probe.fll'
SET FIXED OFF
SET DECIMALS TO 2
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR index=1 TO ALEN(expressions)
 expression=expressions[index]
 directMeta=CFNMETA(EVALUATE(expression))
 directBits=HexBytes(BINTOC(EVALUATE(expression),'BRS'))
 value=EVALUATE(expression)
 line='PLAIN|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+directMeta+'|'+CFNMETA(value)+'|'+;
  directBits+'|'+HexBytes(BINTOC(value,'BRS'))
 =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
 total=total+1
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION HexBytes
 LPARAMETERS source
 LOCAL result,index,byte,digits
 result=''
 digits='0123456789ABCDEF'
 FOR index=1 TO LEN(source)
  byte=ASC(SUBSTR(source,index,1))
  result=result+SUBSTR(digits,INT(byte/16)+1,1)+SUBSTR(digits,MOD(byte,16)+1,1)
 ENDFOR
 RETURN result
ENDFUNC
