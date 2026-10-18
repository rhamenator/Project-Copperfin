* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: native-feedback local boundary recovery.
LOCAL ARRAY baseHex[10],negative[10],setting[10]
baseHex[1]='3FE0000000000000'
negative[1]=.F.
setting[1]=0
baseHex[2]='3FE0000000000000'
negative[2]=.T.
setting[2]=0
baseHex[3]='3FF0147AE147AE14'
negative[3]=.F.
setting[3]=2
baseHex[4]='3FF0147AE147AE14'
negative[4]=.T.
setting[4]=2
baseHex[5]='4005666666666666'
negative[5]=.F.
setting[5]=2
baseHex[6]='4005666666666666'
negative[6]=.T.
setting[6]=2
baseHex[7]='4023FD70A3D70A3D'
negative[7]=.F.
setting[7]=2
baseHex[8]='4023FD70A3D70A3D'
negative[8]=.T.
setting[8]=2
baseHex[9]='3CC203AF9EE75616'
negative[9]=.F.
setting[9]=15
baseHex[10]='3CC203AF9EE75616'
negative[10]=.T.
setting[10]=15
LOCAL outputPath,libraryPath,groupNo,queryNo,lowOffset,highOffset,midOffset,lowText,highText,midText,lowHex,highHex,midHex,total,decimalCount
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-rounding-edge\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR groupNo=1 TO 10
 decimalCount=setting[groupNo]
 SET DECIMALS TO decimalCount
 lowOffset=-4503599627370496
 highOffset=0
 queryNo=1
 highHex=OffsetHex(baseHex[groupNo],negative[groupNo],highOffset)
 highText=Observe(highHex,groupNo,queryNo,setting[groupNo],outputPath)
 queryNo=2
 lowHex=OffsetHex(baseHex[groupNo],negative[groupNo],lowOffset)
 lowText=Observe(lowHex,groupNo,queryNo,setting[groupNo],outputPath)
 IF lowText==highText
  =STRTOFILE('UNBRACKETED|'+ALLTRIM(STR(groupNo,4,0))+CHR(13)+CHR(10),outputPath,1)
 ELSE
  DO WHILE highOffset-lowOffset>1 AND queryNo<54
   midOffset=INT((lowOffset+highOffset)/2)
   midHex=OffsetHex(baseHex[groupNo],negative[groupNo],midOffset)
   queryNo=queryNo+1
   midText=Observe(midHex,groupNo,queryNo,setting[groupNo],outputPath)
   IF midText==highText
    highOffset=midOffset
    highHex=midHex
   ELSE
    lowOffset=midOffset
    lowHex=midHex
    lowText=midText
   ENDIF
  ENDDO
  =STRTOFILE('EDGE|'+ALLTRIM(STR(groupNo,4,0))+'|'+lowHex+'|'+highHex+'|'+lowText+'|'+highText+CHR(13)+CHR(10),outputPath,1)
 ENDIF
 total=total+queryNo*2
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION Observe
 LPARAMETERS inputHex,groupNo,queryNo,decimalCount,outputPath
 LOCAL payload,fixedNo,fixedMode,line,result,caught
 payload=CTOBIN(Unhex(inputHex),'BS')
 result=''
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  TRY
   result=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
   line=ALLTRIM(STR(groupNo,4,0))+'|'+ALLTRIM(STR(queryNo,4,0))+'|'+fixedMode+'|'+inputHex+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))+'|'+HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))+'|'+result+'|'+HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,decimalCount))+'|'+VARTYPE(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
  CATCH TO caught
   line='ERROR|'+ALLTRIM(STR(groupNo,4,0))+'|'+ALLTRIM(STR(queryNo,4,0))+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
  ENDTRY
  =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
 ENDFOR
 RETURN result

FUNCTION OffsetHex
 LPARAMETERS base,negative,offset
 LOCAL high,low,carry
 high=HexNumber(LEFT(base,8))
 low=HexNumber(RIGHT(base,8))+offset
 carry=FLOOR(low/4294967296)
 high=high+carry
 low=low-carry*4294967296
 IF negative
  high=high+2147483648
 ENDIF
 RETURN Hex8(high)+Hex8(low)

FUNCTION HexNumber
 LPARAMETERS value
 LOCAL result,position
 result=0
 FOR position=1 TO LEN(value)
  result=result*16+AT(SUBSTR(value,position,1),'0123456789ABCDEF')-1
 ENDFOR
 RETURN result

FUNCTION Hex8
 LPARAMETERS value
 LOCAL result,position,digit
 result=''
 FOR position=1 TO 8
  digit=MOD(value,16)
  result=SUBSTR('0123456789ABCDEF',digit+1,1)+result
  value=INT(value/16)
 ENDFOR
 RETURN result

FUNCTION Unhex
 LPARAMETERS value
 LOCAL result,position,high,low
 result=''
 FOR position=1 TO LEN(value) STEP 2
  high=AT(SUBSTR(value,position,1),'0123456789ABCDEF')-1
  low=AT(SUBSTR(value,position+1,1),'0123456789ABCDEF')-1
  result=result+CHR(high*16+low)
 ENDFOR
 RETURN result

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,position,byteValue
 result=''
 FOR position=1 TO LEN(value)
  byteValue=ASC(SUBSTR(value,position,1))
  result=result+SUBSTR('0123456789ABCDEF',INT(byteValue/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(byteValue,16)+1,1)
 ENDFOR
 RETURN result
