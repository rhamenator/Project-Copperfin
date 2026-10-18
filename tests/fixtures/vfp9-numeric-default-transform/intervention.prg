* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: native intervention/source freeze.
LOCAL ARRAY expressions[240],settings[3]
expressions[1]='0.50'
expressions[2]='CFNCLONE(0.50)'
expressions[3]='CFNDEC(CFNWIDTH(0.50,1),0)'
expressions[4]='CFNDEC(CFNWIDTH(0.50,2),0)'
expressions[5]='CFNDEC(CFNWIDTH(0.50,2),1)'
expressions[6]='CFNDEC(CFNWIDTH(0.50,3),0)'
expressions[7]='CFNDEC(CFNWIDTH(0.50,3),1)'
expressions[8]='CFNDEC(CFNWIDTH(0.50,3),2)'
expressions[9]='CFNDEC(CFNWIDTH(0.50,4),0)'
expressions[10]='CFNDEC(CFNWIDTH(0.50,4),1)'
expressions[11]='CFNDEC(CFNWIDTH(0.50,4),2)'
expressions[12]='CFNDEC(CFNWIDTH(0.50,12),0)'
expressions[13]='CFNDEC(CFNWIDTH(0.50,12),1)'
expressions[14]='CFNDEC(CFNWIDTH(0.50,12),2)'
expressions[15]='CFNDEC(CFNWIDTH(0.50,20),0)'
expressions[16]='CFNDEC(CFNWIDTH(0.50,20),1)'
expressions[17]='CFNDEC(CFNWIDTH(0.50,20),2)'
expressions[18]='CFNDEC(CFNWIDTH(0.50,20),14)'
expressions[19]='CFNDEC(CFNWIDTH(0.50,20),15)'
expressions[20]='CFNDEC(CFNWIDTH(0.50,20),18)'
expressions[21]='CFNDEC(CFNWIDTH(0.50,40),0)'
expressions[22]='CFNDEC(CFNWIDTH(0.50,40),1)'
expressions[23]='CFNDEC(CFNWIDTH(0.50,40),2)'
expressions[24]='CFNDEC(CFNWIDTH(0.50,40),14)'
expressions[25]='CFNDEC(CFNWIDTH(0.50,40),15)'
expressions[26]='CFNDEC(CFNWIDTH(0.50,40),18)'
expressions[27]='-0.50'
expressions[28]='CFNCLONE(-0.50)'
expressions[29]='CFNDEC(CFNWIDTH(-0.50,1),0)'
expressions[30]='CFNDEC(CFNWIDTH(-0.50,2),0)'
expressions[31]='CFNDEC(CFNWIDTH(-0.50,2),1)'
expressions[32]='CFNDEC(CFNWIDTH(-0.50,3),0)'
expressions[33]='CFNDEC(CFNWIDTH(-0.50,3),1)'
expressions[34]='CFNDEC(CFNWIDTH(-0.50,3),2)'
expressions[35]='CFNDEC(CFNWIDTH(-0.50,4),0)'
expressions[36]='CFNDEC(CFNWIDTH(-0.50,4),1)'
expressions[37]='CFNDEC(CFNWIDTH(-0.50,4),2)'
expressions[38]='CFNDEC(CFNWIDTH(-0.50,12),0)'
expressions[39]='CFNDEC(CFNWIDTH(-0.50,12),1)'
expressions[40]='CFNDEC(CFNWIDTH(-0.50,12),2)'
expressions[41]='CFNDEC(CFNWIDTH(-0.50,20),0)'
expressions[42]='CFNDEC(CFNWIDTH(-0.50,20),1)'
expressions[43]='CFNDEC(CFNWIDTH(-0.50,20),2)'
expressions[44]='CFNDEC(CFNWIDTH(-0.50,20),14)'
expressions[45]='CFNDEC(CFNWIDTH(-0.50,20),15)'
expressions[46]='CFNDEC(CFNWIDTH(-0.50,20),18)'
expressions[47]='CFNDEC(CFNWIDTH(-0.50,40),0)'
expressions[48]='CFNDEC(CFNWIDTH(-0.50,40),1)'
expressions[49]='CFNDEC(CFNWIDTH(-0.50,40),2)'
expressions[50]='CFNDEC(CFNWIDTH(-0.50,40),14)'
expressions[51]='CFNDEC(CFNWIDTH(-0.50,40),15)'
expressions[52]='CFNDEC(CFNWIDTH(-0.50,40),18)'
expressions[53]='0.00'
expressions[54]='CFNCLONE(0.00)'
expressions[55]='CFNDEC(CFNWIDTH(0.00,1),0)'
expressions[56]='CFNDEC(CFNWIDTH(0.00,2),0)'
expressions[57]='CFNDEC(CFNWIDTH(0.00,2),1)'
expressions[58]='CFNDEC(CFNWIDTH(0.00,3),0)'
expressions[59]='CFNDEC(CFNWIDTH(0.00,3),1)'
expressions[60]='CFNDEC(CFNWIDTH(0.00,3),2)'
expressions[61]='CFNDEC(CFNWIDTH(0.00,4),0)'
expressions[62]='CFNDEC(CFNWIDTH(0.00,4),1)'
expressions[63]='CFNDEC(CFNWIDTH(0.00,4),2)'
expressions[64]='CFNDEC(CFNWIDTH(0.00,12),0)'
expressions[65]='CFNDEC(CFNWIDTH(0.00,12),1)'
expressions[66]='CFNDEC(CFNWIDTH(0.00,12),2)'
expressions[67]='CFNDEC(CFNWIDTH(0.00,20),0)'
expressions[68]='CFNDEC(CFNWIDTH(0.00,20),1)'
expressions[69]='CFNDEC(CFNWIDTH(0.00,20),2)'
expressions[70]='CFNDEC(CFNWIDTH(0.00,20),14)'
expressions[71]='CFNDEC(CFNWIDTH(0.00,20),15)'
expressions[72]='CFNDEC(CFNWIDTH(0.00,20),18)'
expressions[73]='CFNDEC(CFNWIDTH(0.00,40),0)'
expressions[74]='CFNDEC(CFNWIDTH(0.00,40),1)'
expressions[75]='CFNDEC(CFNWIDTH(0.00,40),2)'
expressions[76]='CFNDEC(CFNWIDTH(0.00,40),14)'
expressions[77]='CFNDEC(CFNWIDTH(0.00,40),15)'
expressions[78]='CFNDEC(CFNWIDTH(0.00,40),18)'
expressions[79]='-0.00'
expressions[80]='CFNCLONE(-0.00)'
expressions[81]='CFNDEC(CFNWIDTH(-0.00,1),0)'
expressions[82]='CFNDEC(CFNWIDTH(-0.00,2),0)'
expressions[83]='CFNDEC(CFNWIDTH(-0.00,2),1)'
expressions[84]='CFNDEC(CFNWIDTH(-0.00,3),0)'
expressions[85]='CFNDEC(CFNWIDTH(-0.00,3),1)'
expressions[86]='CFNDEC(CFNWIDTH(-0.00,3),2)'
expressions[87]='CFNDEC(CFNWIDTH(-0.00,4),0)'
expressions[88]='CFNDEC(CFNWIDTH(-0.00,4),1)'
expressions[89]='CFNDEC(CFNWIDTH(-0.00,4),2)'
expressions[90]='CFNDEC(CFNWIDTH(-0.00,12),0)'
expressions[91]='CFNDEC(CFNWIDTH(-0.00,12),1)'
expressions[92]='CFNDEC(CFNWIDTH(-0.00,12),2)'
expressions[93]='CFNDEC(CFNWIDTH(-0.00,20),0)'
expressions[94]='CFNDEC(CFNWIDTH(-0.00,20),1)'
expressions[95]='CFNDEC(CFNWIDTH(-0.00,20),2)'
expressions[96]='CFNDEC(CFNWIDTH(-0.00,20),14)'
expressions[97]='CFNDEC(CFNWIDTH(-0.00,20),15)'
expressions[98]='CFNDEC(CFNWIDTH(-0.00,20),18)'
expressions[99]='CFNDEC(CFNWIDTH(-0.00,40),0)'
expressions[100]='CFNDEC(CFNWIDTH(-0.00,40),1)'
expressions[101]='CFNDEC(CFNWIDTH(-0.00,40),2)'
expressions[102]='CFNDEC(CFNWIDTH(-0.00,40),14)'
expressions[103]='CFNDEC(CFNWIDTH(-0.00,40),15)'
expressions[104]='CFNDEC(CFNWIDTH(-0.00,40),18)'
expressions[105]='1E-14'
expressions[106]='CFNCLONE(1E-14)'
expressions[107]='CFNDEC(CFNWIDTH(1E-14,1),0)'
expressions[108]='CFNDEC(CFNWIDTH(1E-14,2),0)'
expressions[109]='CFNDEC(CFNWIDTH(1E-14,2),1)'
expressions[110]='CFNDEC(CFNWIDTH(1E-14,3),0)'
expressions[111]='CFNDEC(CFNWIDTH(1E-14,3),1)'
expressions[112]='CFNDEC(CFNWIDTH(1E-14,3),2)'
expressions[113]='CFNDEC(CFNWIDTH(1E-14,4),0)'
expressions[114]='CFNDEC(CFNWIDTH(1E-14,4),1)'
expressions[115]='CFNDEC(CFNWIDTH(1E-14,4),2)'
expressions[116]='CFNDEC(CFNWIDTH(1E-14,12),0)'
expressions[117]='CFNDEC(CFNWIDTH(1E-14,12),1)'
expressions[118]='CFNDEC(CFNWIDTH(1E-14,12),2)'
expressions[119]='CFNDEC(CFNWIDTH(1E-14,20),0)'
expressions[120]='CFNDEC(CFNWIDTH(1E-14,20),1)'
expressions[121]='CFNDEC(CFNWIDTH(1E-14,20),2)'
expressions[122]='CFNDEC(CFNWIDTH(1E-14,20),14)'
expressions[123]='CFNDEC(CFNWIDTH(1E-14,20),15)'
expressions[124]='CFNDEC(CFNWIDTH(1E-14,20),18)'
expressions[125]='CFNDEC(CFNWIDTH(1E-14,40),0)'
expressions[126]='CFNDEC(CFNWIDTH(1E-14,40),1)'
expressions[127]='CFNDEC(CFNWIDTH(1E-14,40),2)'
expressions[128]='CFNDEC(CFNWIDTH(1E-14,40),14)'
expressions[129]='CFNDEC(CFNWIDTH(1E-14,40),15)'
expressions[130]='CFNDEC(CFNWIDTH(1E-14,40),18)'
expressions[131]='1E-18'
expressions[132]='CFNCLONE(1E-18)'
expressions[133]='CFNDEC(CFNWIDTH(1E-18,1),0)'
expressions[134]='CFNDEC(CFNWIDTH(1E-18,2),0)'
expressions[135]='CFNDEC(CFNWIDTH(1E-18,2),1)'
expressions[136]='CFNDEC(CFNWIDTH(1E-18,3),0)'
expressions[137]='CFNDEC(CFNWIDTH(1E-18,3),1)'
expressions[138]='CFNDEC(CFNWIDTH(1E-18,3),2)'
expressions[139]='CFNDEC(CFNWIDTH(1E-18,4),0)'
expressions[140]='CFNDEC(CFNWIDTH(1E-18,4),1)'
expressions[141]='CFNDEC(CFNWIDTH(1E-18,4),2)'
expressions[142]='CFNDEC(CFNWIDTH(1E-18,12),0)'
expressions[143]='CFNDEC(CFNWIDTH(1E-18,12),1)'
expressions[144]='CFNDEC(CFNWIDTH(1E-18,12),2)'
expressions[145]='CFNDEC(CFNWIDTH(1E-18,20),0)'
expressions[146]='CFNDEC(CFNWIDTH(1E-18,20),1)'
expressions[147]='CFNDEC(CFNWIDTH(1E-18,20),2)'
expressions[148]='CFNDEC(CFNWIDTH(1E-18,20),14)'
expressions[149]='CFNDEC(CFNWIDTH(1E-18,20),15)'
expressions[150]='CFNDEC(CFNWIDTH(1E-18,20),18)'
expressions[151]='CFNDEC(CFNWIDTH(1E-18,40),0)'
expressions[152]='CFNDEC(CFNWIDTH(1E-18,40),1)'
expressions[153]='CFNDEC(CFNWIDTH(1E-18,40),2)'
expressions[154]='CFNDEC(CFNWIDTH(1E-18,40),14)'
expressions[155]='CFNDEC(CFNWIDTH(1E-18,40),15)'
expressions[156]='CFNDEC(CFNWIDTH(1E-18,40),18)'
expressions[157]='-1E-18'
expressions[158]='CFNCLONE(-1E-18)'
expressions[159]='CFNDEC(CFNWIDTH(-1E-18,1),0)'
expressions[160]='CFNDEC(CFNWIDTH(-1E-18,2),0)'
expressions[161]='CFNDEC(CFNWIDTH(-1E-18,2),1)'
expressions[162]='CFNDEC(CFNWIDTH(-1E-18,3),0)'
expressions[163]='CFNDEC(CFNWIDTH(-1E-18,3),1)'
expressions[164]='CFNDEC(CFNWIDTH(-1E-18,3),2)'
expressions[165]='CFNDEC(CFNWIDTH(-1E-18,4),0)'
expressions[166]='CFNDEC(CFNWIDTH(-1E-18,4),1)'
expressions[167]='CFNDEC(CFNWIDTH(-1E-18,4),2)'
expressions[168]='CFNDEC(CFNWIDTH(-1E-18,12),0)'
expressions[169]='CFNDEC(CFNWIDTH(-1E-18,12),1)'
expressions[170]='CFNDEC(CFNWIDTH(-1E-18,12),2)'
expressions[171]='CFNDEC(CFNWIDTH(-1E-18,20),0)'
expressions[172]='CFNDEC(CFNWIDTH(-1E-18,20),1)'
expressions[173]='CFNDEC(CFNWIDTH(-1E-18,20),2)'
expressions[174]='CFNDEC(CFNWIDTH(-1E-18,20),14)'
expressions[175]='CFNDEC(CFNWIDTH(-1E-18,20),15)'
expressions[176]='CFNDEC(CFNWIDTH(-1E-18,20),18)'
expressions[177]='CFNDEC(CFNWIDTH(-1E-18,40),0)'
expressions[178]='CFNDEC(CFNWIDTH(-1E-18,40),1)'
expressions[179]='CFNDEC(CFNWIDTH(-1E-18,40),2)'
expressions[180]='CFNDEC(CFNWIDTH(-1E-18,40),14)'
expressions[181]='CFNDEC(CFNWIDTH(-1E-18,40),15)'
expressions[182]='CFNDEC(CFNWIDTH(-1E-18,40),18)'
expressions[183]='1.234567890123456789'
expressions[184]='CFNCLONE(1.234567890123456789)'
expressions[185]='CFNDEC(CFNWIDTH(1.234567890123456789,1),0)'
expressions[186]='CFNDEC(CFNWIDTH(1.234567890123456789,2),0)'
expressions[187]='CFNDEC(CFNWIDTH(1.234567890123456789,2),1)'
expressions[188]='CFNDEC(CFNWIDTH(1.234567890123456789,3),0)'
expressions[189]='CFNDEC(CFNWIDTH(1.234567890123456789,3),1)'
expressions[190]='CFNDEC(CFNWIDTH(1.234567890123456789,3),2)'
expressions[191]='CFNDEC(CFNWIDTH(1.234567890123456789,4),0)'
expressions[192]='CFNDEC(CFNWIDTH(1.234567890123456789,4),1)'
expressions[193]='CFNDEC(CFNWIDTH(1.234567890123456789,4),2)'
expressions[194]='CFNDEC(CFNWIDTH(1.234567890123456789,12),0)'
expressions[195]='CFNDEC(CFNWIDTH(1.234567890123456789,12),1)'
expressions[196]='CFNDEC(CFNWIDTH(1.234567890123456789,12),2)'
expressions[197]='CFNDEC(CFNWIDTH(1.234567890123456789,20),0)'
expressions[198]='CFNDEC(CFNWIDTH(1.234567890123456789,20),1)'
expressions[199]='CFNDEC(CFNWIDTH(1.234567890123456789,20),2)'
expressions[200]='CFNDEC(CFNWIDTH(1.234567890123456789,20),14)'
expressions[201]='CFNDEC(CFNWIDTH(1.234567890123456789,20),15)'
expressions[202]='CFNDEC(CFNWIDTH(1.234567890123456789,20),18)'
expressions[203]='CFNDEC(CFNWIDTH(1.234567890123456789,40),0)'
expressions[204]='CFNDEC(CFNWIDTH(1.234567890123456789,40),1)'
expressions[205]='CFNDEC(CFNWIDTH(1.234567890123456789,40),2)'
expressions[206]='CFNDEC(CFNWIDTH(1.234567890123456789,40),14)'
expressions[207]='CFNDEC(CFNWIDTH(1.234567890123456789,40),15)'
expressions[208]='CFNDEC(CFNWIDTH(1.234567890123456789,40),18)'
expressions[209]='0.5'
expressions[210]='-0.5'
expressions[211]='1.5'
expressions[212]='-1.5'
expressions[213]='2.5'
expressions[214]='-2.5'
expressions[215]='2.675'
expressions[216]='-2.675'
expressions[217]='1.005'
expressions[218]='-1.005'
expressions[219]='9.995'
expressions[220]='-9.995'
expressions[221]='0.499999999999999'
expressions[222]='0.500000000000001'
expressions[223]='1.2345678901234545'
expressions[224]='1.2345678901234555'
expressions[225]='1.2345678901234565'
expressions[226]='1.2345678901234575'
expressions[227]='1.2345678901234585'
expressions[228]='9.999999999999999'
expressions[229]='99.999999999999999'
expressions[230]='999999999.123456789'
expressions[231]='-999999999.123456789'
expressions[232]='12345678901234.5625'
expressions[233]='-12345678901234.5625'
expressions[234]='999999999999999.5'
expressions[235]='999999999999999.9'
expressions[236]='1000000000000000.5'
expressions[237]='0.00000000000000049'
expressions[238]='0.00000000000000050'
expressions[239]='0.00000000000000051'
expressions[240]='-0.00000000000000049'
settings[1]=0
settings[2]=2
settings[3]=18
PRIVATE frozen1,frozen2,frozen3,frozen4,frozen5,frozen6,frozen7,frozen8,frozen9,frozen10,frozen11,frozen12
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,birthNo,birthSetting,birthFixedNo,birthFixed
outputPath='Z:\home\rich\.codex\worktrees\numeric-literal-display-5611\Project-Copperfin\build\default-transform-intervention\results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR settingNo=1 TO ALEN(settings)
 decimalCount=settings[settingNo]
 SET DECIMALS TO decimalCount
 FOR fixedNo=1 TO 2
  fixedMode=SetFixedMode(fixedNo)
  FOR index=1 TO ALEN(expressions)
   expression=expressions[index]
   TRY
    observation=CFNMETA(EVALUATE(expression))+'|'+HexBytes(BINTOC(EVALUATE(expression),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE(expression)))+'|'+HexBytes(STR(EVALUATE(expression),40,decimalCount))+'|'+VARTYPE(TRANSFORM(EVALUATE(expression)))
    line='GRID|-|-|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
   CATCH TO caught
    line='ERROR-GRID|-|-|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
   ENDTRY
   =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   total=total+1
  ENDFOR
 ENDFOR
ENDFOR
FOR birthNo=1 TO ALEN(settings)
 birthSetting=settings[birthNo]
 FOR birthFixedNo=1 TO 2
  SET DECIMALS TO birthSetting
  birthFixed=SetFixedMode(birthFixedNo)
  frozen1=0.5
  frozen2=2.50
  frozen3=1.00
  frozen4=-0.00
  frozen5=1E-14
  frozen6=-1E-18
  frozen7=1.234567890123456789
  frozen8=(0.5)
  frozen9=(2.50)
  frozen10=(1E-14)
  frozen11=(1E-18)
  frozen12=(1E+20)
  FOR settingNo=1 TO ALEN(settings)
   decimalCount=settings[settingNo]
   SET DECIMALS TO decimalCount
   FOR fixedNo=1 TO 2
    fixedMode=SetFixedMode(fixedNo)
    FOR index=1 TO 12
     TRY
      observation=FrozenObservation(index,decimalCount)
      line='STORED|'+ALLTRIM(STR(birthSetting,4,0))+'|'+birthFixed+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(FrozenSource(index))+'|'+observation
     CATCH TO caught
      line='ERROR-STORED|'+ALLTRIM(STR(birthSetting,4,0))+'|'+birthFixed+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
     ENDTRY
     =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
     total=total+1
    ENDFOR
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION SetFixedMode
 LPARAMETERS fixedNo
 IF fixedNo=1
  SET FIXED OFF
  RETURN 'OFF'
 ENDIF
 SET FIXED ON
 RETURN 'ON'

FUNCTION FrozenObservation
 LPARAMETERS index,decimalCount
 DO CASE
 CASE index=1
  RETURN CFNMETA(frozen1)+'|'+HexBytes(BINTOC(frozen1,'BRS'))+'|'+HexBytes(TRANSFORM(frozen1))+'|'+HexBytes(STR(frozen1,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen1))
 CASE index=2
  RETURN CFNMETA(frozen2)+'|'+HexBytes(BINTOC(frozen2,'BRS'))+'|'+HexBytes(TRANSFORM(frozen2))+'|'+HexBytes(STR(frozen2,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen2))
 CASE index=3
  RETURN CFNMETA(frozen3)+'|'+HexBytes(BINTOC(frozen3,'BRS'))+'|'+HexBytes(TRANSFORM(frozen3))+'|'+HexBytes(STR(frozen3,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen3))
 CASE index=4
  RETURN CFNMETA(frozen4)+'|'+HexBytes(BINTOC(frozen4,'BRS'))+'|'+HexBytes(TRANSFORM(frozen4))+'|'+HexBytes(STR(frozen4,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen4))
 CASE index=5
  RETURN CFNMETA(frozen5)+'|'+HexBytes(BINTOC(frozen5,'BRS'))+'|'+HexBytes(TRANSFORM(frozen5))+'|'+HexBytes(STR(frozen5,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen5))
 CASE index=6
  RETURN CFNMETA(frozen6)+'|'+HexBytes(BINTOC(frozen6,'BRS'))+'|'+HexBytes(TRANSFORM(frozen6))+'|'+HexBytes(STR(frozen6,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen6))
 CASE index=7
  RETURN CFNMETA(frozen7)+'|'+HexBytes(BINTOC(frozen7,'BRS'))+'|'+HexBytes(TRANSFORM(frozen7))+'|'+HexBytes(STR(frozen7,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen7))
 CASE index=8
  RETURN CFNMETA(frozen8)+'|'+HexBytes(BINTOC(frozen8,'BRS'))+'|'+HexBytes(TRANSFORM(frozen8))+'|'+HexBytes(STR(frozen8,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen8))
 CASE index=9
  RETURN CFNMETA(frozen9)+'|'+HexBytes(BINTOC(frozen9,'BRS'))+'|'+HexBytes(TRANSFORM(frozen9))+'|'+HexBytes(STR(frozen9,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen9))
 CASE index=10
  RETURN CFNMETA(frozen10)+'|'+HexBytes(BINTOC(frozen10,'BRS'))+'|'+HexBytes(TRANSFORM(frozen10))+'|'+HexBytes(STR(frozen10,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen10))
 CASE index=11
  RETURN CFNMETA(frozen11)+'|'+HexBytes(BINTOC(frozen11,'BRS'))+'|'+HexBytes(TRANSFORM(frozen11))+'|'+HexBytes(STR(frozen11,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen11))
 CASE index=12
  RETURN CFNMETA(frozen12)+'|'+HexBytes(BINTOC(frozen12,'BRS'))+'|'+HexBytes(TRANSFORM(frozen12))+'|'+HexBytes(STR(frozen12,40,decimalCount))+'|'+VARTYPE(TRANSFORM(frozen12))
 ENDCASE
 RETURN ''

FUNCTION FrozenSource
 LPARAMETERS index
 DO CASE
 CASE index=1
  RETURN '0.5'
 CASE index=2
  RETURN '2.50'
 CASE index=3
  RETURN '1.00'
 CASE index=4
  RETURN '-0.00'
 CASE index=5
  RETURN '1E-14'
 CASE index=6
  RETURN '-1E-18'
 CASE index=7
  RETURN '1.234567890123456789'
 CASE index=8
  RETURN '(0.5)'
 CASE index=9
  RETURN '(2.50)'
 CASE index=10
  RETURN '(1E-14)'
 CASE index=11
  RETURN '(1E-18)'
 CASE index=12
  RETURN '(1E+20)'
 ENDCASE
 RETURN ''

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,index,byteValue
 result=''
 FOR index=1 TO LEN(value)
  byteValue=ASC(SUBSTR(value,index,1))
  result=result+SUBSTR('0123456789ABCDEF',INT(byteValue/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(byteValue,16)+1,1)
 ENDFOR
 RETURN result
