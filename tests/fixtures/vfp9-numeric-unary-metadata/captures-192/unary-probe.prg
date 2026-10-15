* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Native unary recovery only; no Copperfin-derived expected descriptors.
LOCAL ARRAY expressions[48], settings[2]
expressions[1]='1.00'
expressions[2]='-1.00'
expressions[3]='+1.00'
expressions[4]='- 1.00'
expressions[5]='+ 1.00'
expressions[6]='-(1.00)'
expressions[7]='+ (1.00)'
expressions[8]='(-1.00)'
expressions[9]='(+1.00)'
expressions[10]='-(-1.00)'
expressions[11]='+(-1.00)'
expressions[12]='-(+1.00)'
expressions[13]='--1.00'
expressions[14]='++1.00'
expressions[15]='- -1.00'
expressions[16]='0.00'
expressions[17]='-0.00'
expressions[18]='+0.00'
expressions[19]='-(-0.00)'
expressions[20]='-12.3400'
expressions[21]='-0012.3400'
expressions[22]='-0.0000000000000000001'
expressions[23]='-1.'+REPLICATE('0',18)
expressions[24]='-1.'+REPLICATE('0',38)
expressions[25]='-(1.'+REPLICATE('0',38)+')'
expressions[26]='-1.'+REPLICATE('0',254)
expressions[27]='-nSource'
expressions[28]='+nSource'
expressions[29]='-(-nSource)'
expressions[30]='-(nSource)'
expressions[31]='-nFraction'
expressions[32]='-2.5'
expressions[33]='-1'
expressions[34]='-1.'
expressions[35]='-1E-14'
expressions[36]='-(-1E-14)'
expressions[37]='-5/2'
expressions[38]='-(5/2)'
expressions[39]='-(-12.3400)'
expressions[40]='-000.000'
expressions[41]='-nZero'
expressions[42]='-(-nZero)'
expressions[43]='-nMinusZero'
expressions[44]='-(-nMinusZero)'
expressions[45]='-(0.00-0.00)'
expressions[46]='-(-(-0.00))'
expressions[47]='-(-(-1.00))'
expressions[48]='-(-1.'+REPLICATE('0',38)+')'
settings[1]=2
settings[2]=14
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL directMeta,directBits,directText,value,line,total,caught,nSource,nFraction,nZero,nMinusZero
outputPath='Z:\home\rich\.codex\worktrees\numeric-unary-metadata-5611\Project-Copperfin\build\numeric-unary-native\results.txt'
libraryPath='Z:\home\rich\.local\share\Trash\files\listitem-column-numeric-5611-build\listitem-column-native\cause\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR settingNo=1 TO ALEN(settings)
 decimalCount=settings[settingNo]
 SET DECIMALS TO decimalCount
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  nSource=1.00
  nFraction=2.5
  nZero=0.00
  nMinusZero=-0.00
  FOR index=1 TO ALEN(expressions)
   expression=expressions[index]
   TRY
    directMeta=CFNMETA(EVALUATE(expression))
    directBits=HexBytes(BINTOC(EVALUATE(expression),'BRS'))
    directText=HexBytes(TRANSFORM(EVALUATE(expression)))
    value=EVALUATE(expression)
    line='UNARY|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+;
     HexBytes(expression)+'|'+directMeta+'|'+CFNMETA(value)+'|'+directBits+'|'+;
     HexBytes(BINTOC(value,'BRS'))+'|'+directText+'|'+HexBytes(TRANSFORM(value))
   CATCH TO caught
    line='ERROR|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+;
     HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
   ENDTRY
   =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   total=total+1
  ENDFOR
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
