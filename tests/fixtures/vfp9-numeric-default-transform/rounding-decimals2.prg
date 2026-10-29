* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: refined native-feedback DECIMALS0..18 with settings readback.
LOCAL ARRAY baseHex[38],negative[38],setting[38]
baseHex[1]='3FE0000000000000'
negative[1]=.F.
setting[1]=0
baseHex[2]='3FE0000000000000'
negative[2]=.T.
setting[2]=0
baseHex[3]='3FA999999999999A'
negative[3]=.F.
setting[3]=1
baseHex[4]='3FA999999999999A'
negative[4]=.T.
setting[4]=1
baseHex[5]='3F747AE147AE147B'
negative[5]=.F.
setting[5]=2
baseHex[6]='3F747AE147AE147B'
negative[6]=.T.
setting[6]=2
baseHex[7]='3F40624DD2F1A9FC'
negative[7]=.F.
setting[7]=3
baseHex[8]='3F40624DD2F1A9FC'
negative[8]=.T.
setting[8]=3
baseHex[9]='3F0A36E2EB1C432D'
negative[9]=.F.
setting[9]=4
baseHex[10]='3F0A36E2EB1C432D'
negative[10]=.T.
setting[10]=4
baseHex[11]='3ED4F8B588E368F1'
negative[11]=.F.
setting[11]=5
baseHex[12]='3ED4F8B588E368F1'
negative[12]=.T.
setting[12]=5
baseHex[13]='3EA0C6F7A0B5ED8D'
negative[13]=.F.
setting[13]=6
baseHex[14]='3EA0C6F7A0B5ED8D'
negative[14]=.T.
setting[14]=6
baseHex[15]='3E6AD7F29ABCAF48'
negative[15]=.F.
setting[15]=7
baseHex[16]='3E6AD7F29ABCAF48'
negative[16]=.T.
setting[16]=7
baseHex[17]='3E35798EE2308C3A'
negative[17]=.F.
setting[17]=8
baseHex[18]='3E35798EE2308C3A'
negative[18]=.T.
setting[18]=8
baseHex[19]='3E012E0BE826D695'
negative[19]=.F.
setting[19]=9
baseHex[20]='3E012E0BE826D695'
negative[20]=.T.
setting[20]=9
baseHex[21]='3DCB7CDFD9D7BDBB'
negative[21]=.F.
setting[21]=10
baseHex[22]='3DCB7CDFD9D7BDBB'
negative[22]=.T.
setting[22]=10
baseHex[23]='3D95FD7FE1796495'
negative[23]=.F.
setting[23]=11
baseHex[24]='3D95FD7FE1796495'
negative[24]=.T.
setting[24]=11
baseHex[25]='3D619799812DEA11'
negative[25]=.F.
setting[25]=12
baseHex[26]='3D619799812DEA11'
negative[26]=.T.
setting[26]=12
baseHex[27]='3D2C25C268497682'
negative[27]=.F.
setting[27]=13
baseHex[28]='3D2C25C268497682'
negative[28]=.T.
setting[28]=13
baseHex[29]='3CF6849B86A12B9B'
negative[29]=.F.
setting[29]=14
baseHex[30]='3CF6849B86A12B9B'
negative[30]=.T.
setting[30]=14
baseHex[31]='3CC203AF9EE75616'
negative[31]=.F.
setting[31]=15
baseHex[32]='3CC203AF9EE75616'
negative[32]=.T.
setting[32]=15
baseHex[33]='3CC203AF9EE75616'
negative[33]=.F.
setting[33]=16
baseHex[34]='3CC203AF9EE75616'
negative[34]=.T.
setting[34]=16
baseHex[35]='3CC203AF9EE75616'
negative[35]=.F.
setting[35]=17
baseHex[36]='3CC203AF9EE75616'
negative[36]=.T.
setting[36]=17
baseHex[37]='3CC203AF9EE75616'
negative[37]=.F.
setting[37]=18
baseHex[38]='3CC203AF9EE75616'
negative[38]=.T.
setting[38]=18
LOCAL outputPath,libraryPath,groupNo,queryNo,lowOffset,highOffset,midOffset,lowText,highText,midText,lowHex,highHex,midHex,total,decimalCount
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-rounding-decimals2\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR groupNo=1 TO 38
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
   line=ALLTRIM(STR(groupNo,4,0))+'|'+ALLTRIM(STR(queryNo,4,0))+'|'+fixedMode+'|'+inputHex+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))+'|'+HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))+'|'+result+'|'+HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,decimalCount))+'|'+VARTYPE(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))+'|'+ALLTRIM(SET('DECIMALS'))+'|'+SET('FIXED')
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
