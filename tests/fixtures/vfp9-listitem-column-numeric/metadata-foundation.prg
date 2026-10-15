* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Native observation only: no Copperfin expectations or inferred propagation rules.
LOCAL ARRAY expressions[16], decimalSettings[3], values[1]
expressions[1]='1'
expressions[2]='1.0'
expressions[3]='1.00'
expressions[4]='001.2300'
expressions[5]='0'
expressions[6]='0.0000'
expressions[7]='-0.00'
expressions[8]='-12.3400'
expressions[9]='1E-14'
expressions[10]='1.00E2'
expressions[11]='1.23E-2'
expressions[12]='1-1E-14'
expressions[13]='1-90*2^(-53)'
expressions[14]='2.5'
expressions[15]='5/2'
expressions[16]='123456789.123456'
decimalSettings[1]=2
decimalSettings[2]=6
decimalSettings[3]=14
LOCAL outputPath,libraryPath,decimalNo,fixedNo,expressionNo,decimals,expression
LOCAL value,box,directMeta,assignedMeta,arrayMeta,parameterMeta,returnedMeta,propertyMeta
LOCAL directBits,assignedBits,arrayBits,returnedBits,propertyBits,directDisplay,assignedDisplay
LOCAL explicitDisplay,line,total,errorNo,errorMessage,stage
outputPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\cause\metadata-foundation-results.txt'
libraryPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\cause\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
SET UDFPARMS TO VALUE
total=0
FOR decimalNo=1 TO ALEN(decimalSettings)
 decimals=decimalSettings[decimalNo]
 SET DECIMALS TO decimals
 FOR fixedNo=0 TO 1
  IF fixedNo=0
   SET FIXED OFF
  ELSE
   SET FIXED ON
  ENDIF
  FOR expressionNo=1 TO ALEN(expressions)
   expression=expressions[expressionNo]
   errorNo=0
   errorMessage=''
   stage='evaluate'
   TRY
    directMeta=CFNMETA(EVALUATE(expression))
    directBits=BINTOC(EVALUATE(expression),'BRS')
    directDisplay=TRANSFORM(EVALUATE(expression))
    value=EVALUATE(expression)
    assignedMeta=CFNMETA(value)
    assignedBits=BINTOC(value,'BRS')
    assignedDisplay=TRANSFORM(value)
    stage='copy'
    values[1]=value
    arrayMeta=CFNMETA(values[1])
    arrayBits=BINTOC(values[1],'BRS')
    parameterMeta=ParameterMetadata(value)
    returnedMeta=CFNMETA(ReturnValue(value))
    returnedBits=BINTOC(ReturnValue(value),'BRS')
    box=CREATEOBJECT('Empty')
    =ADDPROPERTY(box,'NumericValue',value)
    propertyMeta=CFNMETA(box.NumericValue)
    propertyBits=BINTOC(box.NumericValue,'BRS')
    explicitDisplay=STR(value,30,6)
    stage='done'
   CATCH TO caught
    errorNo=caught.ErrorNo
    errorMessage=caught.Message
   ENDTRY
   IF errorNo<>0
    =STRTOFILE('ERROR|'+expression+'|'+stage+'|'+TRANSFORM(errorNo)+'|'+errorMessage+CHR(13)+CHR(10),outputPath,1)
    ERROR errorNo
   ENDIF
   line='ROW|'+TRANSFORM(decimals)+'|'+TRANSFORM(fixedNo)+'|'+expression+'|'+;
    directMeta+'|'+assignedMeta+'|'+arrayMeta+'|'+parameterMeta+'|'+returnedMeta+'|'+propertyMeta+'|'+;
    HexBytes(directBits)+'|'+HexBytes(assignedBits)+'|'+HexBytes(arrayBits)+'|'+HexBytes(returnedBits)+'|'+HexBytes(propertyBits)+'|'+;
    HexBytes(directDisplay)+'|'+HexBytes(assignedDisplay)+'|'+HexBytes(explicitDisplay)
   =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   total=total+1
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+TRANSFORM(total)+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION ParameterMetadata
 LPARAMETERS argument
 RETURN CFNMETA(argument)
ENDFUNC

FUNCTION ReturnValue
 LPARAMETERS argument
 RETURN argument
ENDFUNC

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
