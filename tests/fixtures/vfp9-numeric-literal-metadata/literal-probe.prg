* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Native observation only; no Copperfin-derived literal expectations.
LOCAL ARRAY expressions[48], settings[3]
expressions[1]='1'
expressions[2]='1.0'
expressions[3]='1.00'
expressions[4]='001.2300'
expressions[5]='0'
expressions[6]='0.0'
expressions[7]='0.0000'
expressions[8]='0.1'
expressions[9]='0.10'
expressions[10]='0.01'
expressions[11]='0.001'
expressions[12]='1.23456789012345678'
expressions[13]='0.123456789012345678'
expressions[14]='0.000000000000000001'
expressions[15]='0.0000000000000000001'
expressions[16]='12345678901234567890.0'
expressions[17]='123456789012345678901.0'
expressions[18]='1.2345678901234567890'
expressions[19]='123456789.123456'
expressions[20]='12.3400'
expressions[21]='0012.3400'
expressions[22]='000.000'
expressions[23]='100.00'
expressions[24]='1000.000'
expressions[25]='999999999999999.0'
expressions[26]='-0.00'
expressions[27]='-12.3400'
expressions[28]='+12.3400'
expressions[29]='1E0'
expressions[30]='1E1'
expressions[31]='1E2'
expressions[32]='1.0E2'
expressions[33]='1.00E2'
expressions[34]='1.23E2'
expressions[35]='1.23E3'
expressions[36]='1.23E-2'
expressions[37]='1E-1'
expressions[38]='1E-2'
expressions[39]='1E-14'
expressions[40]='1.0E-14'
expressions[41]='1.00E-14'
expressions[42]='1E18'
expressions[43]='1E19'
expressions[44]='1E20'
expressions[45]='1E-18'
expressions[46]='1E-19'
expressions[47]='1E300'
expressions[48]='1E-300'
settings[1]=2
settings[2]=6
settings[3]=14
LOCAL outputPath,libraryPath,index,settingNo,decimalCount,expression,value,total
LOCAL directMeta,assignedMeta,directBits,assignedBits,directText,assignedText,line
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-metadata-5611\Project-Copperfin\build\numeric-literal-native\results.txt'
libraryPath='Z:\home\rich\.local\share\Trash\files\listitem-column-numeric-5611-build\listitem-column-native\cause\cause-probe.fll'
SET FIXED OFF
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR settingNo=1 TO ALEN(settings)
 decimalCount=settings[settingNo]
 SET DECIMALS TO decimalCount
 FOR index=1 TO ALEN(expressions)
  expression=expressions[index]
  directMeta=CFNMETA(EVALUATE(expression))
  directBits=HexBytes(BINTOC(EVALUATE(expression),'BRS'))
  directText=HexBytes(TRANSFORM(EVALUATE(expression)))
  value=EVALUATE(expression)
  assignedMeta=CFNMETA(value)
  assignedBits=HexBytes(BINTOC(value,'BRS'))
  assignedText=HexBytes(TRANSFORM(value))
  line='LITERAL|'+ALLTRIM(STR(decimalCount,4,0))+'|'+expression+'|'+directMeta+'|'+assignedMeta+'|'+;
   directBits+'|'+assignedBits+'|'+directText+'|'+assignedText
  =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
  total=total+1
 ENDFOR
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
