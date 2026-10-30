* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001: native-only source freeze.
LOCAL ARRAY expressions[253],settings[5]
expressions[1]='(5/0.5)'
expressions[2]='-((5/0.5))'
expressions[3]='IIF(.T.,(5/0.5),0)'
expressions[4]='-(IIF(.T.,(5/0.5),0))'
expressions[5]='(-(5)/0.5)'
expressions[6]='-((-(5)/0.5))'
expressions[7]='IIF(.T.,(-(5)/0.5),0)'
expressions[8]='-(IIF(.T.,(-(5)/0.5),0))'
expressions[9]='(5/-(0.5))'
expressions[10]='-((5/-(0.5)))'
expressions[11]='IIF(.T.,(5/-(0.5)),0)'
expressions[12]='-(IIF(.T.,(5/-(0.5)),0))'
expressions[13]='(-(5)/-(0.5))'
expressions[14]='-((-(5)/-(0.5)))'
expressions[15]='IIF(.T.,(-(5)/-(0.5)),0)'
expressions[16]='-(IIF(.T.,(-(5)/-(0.5)),0))'
expressions[17]='(1/0.5)'
expressions[18]='-((1/0.5))'
expressions[19]='IIF(.T.,(1/0.5),0)'
expressions[20]='-(IIF(.T.,(1/0.5),0))'
expressions[21]='(-(1)/0.5)'
expressions[22]='-((-(1)/0.5))'
expressions[23]='IIF(.T.,(-(1)/0.5),0)'
expressions[24]='-(IIF(.T.,(-(1)/0.5),0))'
expressions[25]='(1/-(0.5))'
expressions[26]='-((1/-(0.5)))'
expressions[27]='IIF(.T.,(1/-(0.5)),0)'
expressions[28]='-(IIF(.T.,(1/-(0.5)),0))'
expressions[29]='(-(1)/-(0.5))'
expressions[30]='-((-(1)/-(0.5)))'
expressions[31]='IIF(.T.,(-(1)/-(0.5)),0)'
expressions[32]='-(IIF(.T.,(-(1)/-(0.5)),0))'
expressions[33]='(0/0.5)'
expressions[34]='-((0/0.5))'
expressions[35]='IIF(.T.,(0/0.5),0)'
expressions[36]='-(IIF(.T.,(0/0.5),0))'
expressions[37]='(-(0)/0.5)'
expressions[38]='-((-(0)/0.5))'
expressions[39]='IIF(.T.,(-(0)/0.5),0)'
expressions[40]='-(IIF(.T.,(-(0)/0.5),0))'
expressions[41]='(0/-(0.5))'
expressions[42]='-((0/-(0.5)))'
expressions[43]='IIF(.T.,(0/-(0.5)),0)'
expressions[44]='-(IIF(.T.,(0/-(0.5)),0))'
expressions[45]='(-(0)/-(0.5))'
expressions[46]='-((-(0)/-(0.5)))'
expressions[47]='IIF(.T.,(-(0)/-(0.5)),0)'
expressions[48]='-(IIF(.T.,(-(0)/-(0.5)),0))'
expressions[49]='(0.5/0.25)'
expressions[50]='-((0.5/0.25))'
expressions[51]='IIF(.T.,(0.5/0.25),0)'
expressions[52]='-(IIF(.T.,(0.5/0.25),0))'
expressions[53]='(-(0.5)/0.25)'
expressions[54]='-((-(0.5)/0.25))'
expressions[55]='IIF(.T.,(-(0.5)/0.25),0)'
expressions[56]='-(IIF(.T.,(-(0.5)/0.25),0))'
expressions[57]='(0.5/-(0.25))'
expressions[58]='-((0.5/-(0.25)))'
expressions[59]='IIF(.T.,(0.5/-(0.25)),0)'
expressions[60]='-(IIF(.T.,(0.5/-(0.25)),0))'
expressions[61]='(-(0.5)/-(0.25))'
expressions[62]='-((-(0.5)/-(0.25)))'
expressions[63]='IIF(.T.,(-(0.5)/-(0.25)),0)'
expressions[64]='-(IIF(.T.,(-(0.5)/-(0.25)),0))'
expressions[65]='(2.50/0.50)'
expressions[66]='-((2.50/0.50))'
expressions[67]='IIF(.T.,(2.50/0.50),0)'
expressions[68]='-(IIF(.T.,(2.50/0.50),0))'
expressions[69]='(-(2.50)/0.50)'
expressions[70]='-((-(2.50)/0.50))'
expressions[71]='IIF(.T.,(-(2.50)/0.50),0)'
expressions[72]='-(IIF(.T.,(-(2.50)/0.50),0))'
expressions[73]='(2.50/-(0.50))'
expressions[74]='-((2.50/-(0.50)))'
expressions[75]='IIF(.T.,(2.50/-(0.50)),0)'
expressions[76]='-(IIF(.T.,(2.50/-(0.50)),0))'
expressions[77]='(-(2.50)/-(0.50))'
expressions[78]='-((-(2.50)/-(0.50)))'
expressions[79]='IIF(.T.,(-(2.50)/-(0.50)),0)'
expressions[80]='-(IIF(.T.,(-(2.50)/-(0.50)),0))'
expressions[81]='(1/0.3)'
expressions[82]='-((1/0.3))'
expressions[83]='IIF(.T.,(1/0.3),0)'
expressions[84]='-(IIF(.T.,(1/0.3),0))'
expressions[85]='(-(1)/0.3)'
expressions[86]='-((-(1)/0.3))'
expressions[87]='IIF(.T.,(-(1)/0.3),0)'
expressions[88]='-(IIF(.T.,(-(1)/0.3),0))'
expressions[89]='(1/-(0.3))'
expressions[90]='-((1/-(0.3)))'
expressions[91]='IIF(.T.,(1/-(0.3)),0)'
expressions[92]='-(IIF(.T.,(1/-(0.3)),0))'
expressions[93]='(-(1)/-(0.3))'
expressions[94]='-((-(1)/-(0.3)))'
expressions[95]='IIF(.T.,(-(1)/-(0.3)),0)'
expressions[96]='-(IIF(.T.,(-(1)/-(0.3)),0))'
expressions[97]='(1/0.25)'
expressions[98]='-((1/0.25))'
expressions[99]='IIF(.T.,(1/0.25),0)'
expressions[100]='-(IIF(.T.,(1/0.25),0))'
expressions[101]='(-(1)/0.25)'
expressions[102]='-((-(1)/0.25))'
expressions[103]='IIF(.T.,(-(1)/0.25),0)'
expressions[104]='-(IIF(.T.,(-(1)/0.25),0))'
expressions[105]='(1/-(0.25))'
expressions[106]='-((1/-(0.25)))'
expressions[107]='IIF(.T.,(1/-(0.25)),0)'
expressions[108]='-(IIF(.T.,(1/-(0.25)),0))'
expressions[109]='(-(1)/-(0.25))'
expressions[110]='-((-(1)/-(0.25)))'
expressions[111]='IIF(.T.,(-(1)/-(0.25)),0)'
expressions[112]='-(IIF(.T.,(-(1)/-(0.25)),0))'
expressions[113]='(0.000000000000000001/0.5)'
expressions[114]='-((0.000000000000000001/0.5))'
expressions[115]='IIF(.T.,(0.000000000000000001/0.5),0)'
expressions[116]='-(IIF(.T.,(0.000000000000000001/0.5),0))'
expressions[117]='(-(0.000000000000000001)/0.5)'
expressions[118]='-((-(0.000000000000000001)/0.5))'
expressions[119]='IIF(.T.,(-(0.000000000000000001)/0.5),0)'
expressions[120]='-(IIF(.T.,(-(0.000000000000000001)/0.5),0))'
expressions[121]='(0.000000000000000001/-(0.5))'
expressions[122]='-((0.000000000000000001/-(0.5)))'
expressions[123]='IIF(.T.,(0.000000000000000001/-(0.5)),0)'
expressions[124]='-(IIF(.T.,(0.000000000000000001/-(0.5)),0))'
expressions[125]='(-(0.000000000000000001)/-(0.5))'
expressions[126]='-((-(0.000000000000000001)/-(0.5)))'
expressions[127]='IIF(.T.,(-(0.000000000000000001)/-(0.5)),0)'
expressions[128]='-(IIF(.T.,(-(0.000000000000000001)/-(0.5)),0))'
expressions[129]='(1.234567890123456789/0.5)'
expressions[130]='-((1.234567890123456789/0.5))'
expressions[131]='IIF(.T.,(1.234567890123456789/0.5),0)'
expressions[132]='-(IIF(.T.,(1.234567890123456789/0.5),0))'
expressions[133]='(-(1.234567890123456789)/0.5)'
expressions[134]='-((-(1.234567890123456789)/0.5))'
expressions[135]='IIF(.T.,(-(1.234567890123456789)/0.5),0)'
expressions[136]='-(IIF(.T.,(-(1.234567890123456789)/0.5),0))'
expressions[137]='(1.234567890123456789/-(0.5))'
expressions[138]='-((1.234567890123456789/-(0.5)))'
expressions[139]='IIF(.T.,(1.234567890123456789/-(0.5)),0)'
expressions[140]='-(IIF(.T.,(1.234567890123456789/-(0.5)),0))'
expressions[141]='(-(1.234567890123456789)/-(0.5))'
expressions[142]='-((-(1.234567890123456789)/-(0.5)))'
expressions[143]='IIF(.T.,(-(1.234567890123456789)/-(0.5)),0)'
expressions[144]='-(IIF(.T.,(-(1.234567890123456789)/-(0.5)),0))'
expressions[145]='(5/00.50)'
expressions[146]='-((5/00.50))'
expressions[147]='IIF(.T.,(5/00.50),0)'
expressions[148]='-(IIF(.T.,(5/00.50),0))'
expressions[149]='(-(5)/00.50)'
expressions[150]='-((-(5)/00.50))'
expressions[151]='IIF(.T.,(-(5)/00.50),0)'
expressions[152]='-(IIF(.T.,(-(5)/00.50),0))'
expressions[153]='(5/-(00.50))'
expressions[154]='-((5/-(00.50)))'
expressions[155]='IIF(.T.,(5/-(00.50)),0)'
expressions[156]='-(IIF(.T.,(5/-(00.50)),0))'
expressions[157]='(-(5)/-(00.50))'
expressions[158]='-((-(5)/-(00.50)))'
expressions[159]='IIF(.T.,(-(5)/-(00.50)),0)'
expressions[160]='-(IIF(.T.,(-(5)/-(00.50)),0))'
expressions[161]='(1/0.125)'
expressions[162]='-((1/0.125))'
expressions[163]='IIF(.T.,(1/0.125),0)'
expressions[164]='-(IIF(.T.,(1/0.125),0))'
expressions[165]='(-(1)/0.125)'
expressions[166]='-((-(1)/0.125))'
expressions[167]='IIF(.T.,(-(1)/0.125),0)'
expressions[168]='-(IIF(.T.,(-(1)/0.125),0))'
expressions[169]='(1/-(0.125))'
expressions[170]='-((1/-(0.125)))'
expressions[171]='IIF(.T.,(1/-(0.125)),0)'
expressions[172]='-(IIF(.T.,(1/-(0.125)),0))'
expressions[173]='(-(1)/-(0.125))'
expressions[174]='-((-(1)/-(0.125)))'
expressions[175]='IIF(.T.,(-(1)/-(0.125)),0)'
expressions[176]='-(IIF(.T.,(-(1)/-(0.125)),0))'
expressions[177]='(0/0.000000000000000001)'
expressions[178]='-((0/0.000000000000000001))'
expressions[179]='IIF(.T.,(0/0.000000000000000001),0)'
expressions[180]='-(IIF(.T.,(0/0.000000000000000001),0))'
expressions[181]='(-(0)/0.000000000000000001)'
expressions[182]='-((-(0)/0.000000000000000001))'
expressions[183]='IIF(.T.,(-(0)/0.000000000000000001),0)'
expressions[184]='-(IIF(.T.,(-(0)/0.000000000000000001),0))'
expressions[185]='(0/-(0.000000000000000001))'
expressions[186]='-((0/-(0.000000000000000001)))'
expressions[187]='IIF(.T.,(0/-(0.000000000000000001)),0)'
expressions[188]='-(IIF(.T.,(0/-(0.000000000000000001)),0))'
expressions[189]='(-(0)/-(0.000000000000000001))'
expressions[190]='-((-(0)/-(0.000000000000000001)))'
expressions[191]='IIF(.T.,(-(0)/-(0.000000000000000001)),0)'
expressions[192]='-(IIF(.T.,(-(0)/-(0.000000000000000001)),0))'
expressions[193]='(1/3)/(1/3)'
expressions[194]='-((1/3)/(1/3))'
expressions[195]='IIF(.T.,(1/3)/(1/3),0)'
expressions[196]='-(IIF(.T.,(1/3)/(1/3),0))'
expressions[197]='5/(1/3)'
expressions[198]='-(5/(1/3))'
expressions[199]='IIF(.T.,5/(1/3),0)'
expressions[200]='-(IIF(.T.,5/(1/3),0))'
expressions[201]='(1/7)/(1/7)'
expressions[202]='-((1/7)/(1/7))'
expressions[203]='IIF(.T.,(1/7)/(1/7),0)'
expressions[204]='-(IIF(.T.,(1/7)/(1/7),0))'
expressions[205]='(0.5/2)/(0.5/2)'
expressions[206]='-((0.5/2)/(0.5/2))'
expressions[207]='IIF(.T.,(0.5/2)/(0.5/2),0)'
expressions[208]='-(IIF(.T.,(0.5/2)/(0.5/2),0))'
expressions[209]='((1/3)/(1/3))/2'
expressions[210]='-(((1/3)/(1/3))/2)'
expressions[211]='IIF(.T.,((1/3)/(1/3))/2,0)'
expressions[212]='-(IIF(.T.,((1/3)/(1/3))/2,0))'
expressions[213]='(1/3)/((1/3)/2)'
expressions[214]='-((1/3)/((1/3)/2))'
expressions[215]='IIF(.T.,(1/3)/((1/3)/2),0)'
expressions[216]='-(IIF(.T.,(1/3)/((1/3)/2),0))'
expressions[217]='(5/2)/(1/3)'
expressions[218]='-((5/2)/(1/3))'
expressions[219]='IIF(.T.,(5/2)/(1/3),0)'
expressions[220]='-(IIF(.T.,(5/2)/(1/3),0))'
expressions[221]='((5/2)/(1/3))/2'
expressions[222]='-(((5/2)/(1/3))/2)'
expressions[223]='IIF(.T.,((5/2)/(1/3))/2,0)'
expressions[224]='-(IIF(.T.,((5/2)/(1/3))/2,0))'
expressions[225]='2'
expressions[226]='3'
expressions[227]='7'
expressions[228]='5'
expressions[229]='0.5'
expressions[230]='1'
expressions[231]='0'
expressions[232]='0.25'
expressions[233]='2.50'
expressions[234]='0.50'
expressions[235]='0.3'
expressions[236]='0.000000000000000001'
expressions[237]='1.234567890123456789'
expressions[238]='00.50'
expressions[239]='0.125'
expressions[240]='(5/0.5)/iTwo'
expressions[241]='iTwo/(1/3)'
expressions[242]='(1/3)/nTwo'
expressions[243]='nTwo/(1/3)'
expressions[244]='-((5/0.5)/iTwo)'
expressions[245]='-(iTwo/(1/3))'
expressions[246]='1/0'
expressions[247]='0/0'
expressions[248]='5/-0.00'
expressions[249]='1/0.0000000001'
expressions[250]='1/0.000000000000000001'
expressions[251]='1E-14/0.5'
expressions[252]='9999999999/0.5'
expressions[253]='5/1E-2'
settings[1]=0
settings[2]=2
settings[3]=6
settings[4]=14
settings[5]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,modeNo,mode
PRIVATE iTwo,nTwo
iTwo=2
nTwo=2.00
outputPath='Z:\home\rich\.codex\worktrees\numeric-small-divisor-5611\Project-Copperfin\build\small-divisor-native\grid-results.txt'
libraryPath='Z:\home\rich\dev\Project-Copperfin\build\pr7203-completed\native-evidence\diagnostic\cause-probe.fll'
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
  FOR index=1 TO ALEN(expressions)
   expression=expressions[index]
   FOR modeNo=1 TO 2
    IF modeNo=1
     mode='EVAL'
    ELSE
     mode='COMPILED'
    ENDIF
    TRY
     IF modeNo=1
      observation=CFNMETA(EVALUATE(expression))+'|'+HexBytes(BINTOC(EVALUATE(expression),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE(expression)))
     ELSE
      observation=CompiledObservation(index)
     ENDIF
     line=mode+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
    CATCH TO caught
     line='ERROR-'+mode+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
    ENDTRY
    =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
    total=total+1
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION CompiledObservation
 LPARAMETERS index
 DO CASE
 CASE index=1
  RETURN CFNMETA((5/0.5))+'|'+HexBytes(BINTOC((5/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((5/0.5)))
 CASE index=2
  RETURN CFNMETA(-((5/0.5)))+'|'+HexBytes(BINTOC(-((5/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/0.5))))
 CASE index=3
  RETURN CFNMETA(IIF(.T.,(5/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(5/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(5/0.5),0)))
 CASE index=4
  RETURN CFNMETA(-(IIF(.T.,(5/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(5/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(5/0.5),0))))
 CASE index=5
  RETURN CFNMETA((-(5)/0.5))+'|'+HexBytes(BINTOC((-(5)/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((-(5)/0.5)))
 CASE index=6
  RETURN CFNMETA(-((-(5)/0.5)))+'|'+HexBytes(BINTOC(-((-(5)/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(5)/0.5))))
 CASE index=7
  RETURN CFNMETA(IIF(.T.,(-(5)/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(5)/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(5)/0.5),0)))
 CASE index=8
  RETURN CFNMETA(-(IIF(.T.,(-(5)/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(5)/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(5)/0.5),0))))
 CASE index=9
  RETURN CFNMETA((5/-(0.5)))+'|'+HexBytes(BINTOC((5/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((5/-(0.5))))
 CASE index=10
  RETURN CFNMETA(-((5/-(0.5))))+'|'+HexBytes(BINTOC(-((5/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/-(0.5)))))
 CASE index=11
  RETURN CFNMETA(IIF(.T.,(5/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(5/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(5/-(0.5)),0)))
 CASE index=12
  RETURN CFNMETA(-(IIF(.T.,(5/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(5/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(5/-(0.5)),0))))
 CASE index=13
  RETURN CFNMETA((-(5)/-(0.5)))+'|'+HexBytes(BINTOC((-(5)/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((-(5)/-(0.5))))
 CASE index=14
  RETURN CFNMETA(-((-(5)/-(0.5))))+'|'+HexBytes(BINTOC(-((-(5)/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(5)/-(0.5)))))
 CASE index=15
  RETURN CFNMETA(IIF(.T.,(-(5)/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(5)/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(5)/-(0.5)),0)))
 CASE index=16
  RETURN CFNMETA(-(IIF(.T.,(-(5)/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(5)/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(5)/-(0.5)),0))))
 CASE index=17
  RETURN CFNMETA((1/0.5))+'|'+HexBytes(BINTOC((1/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((1/0.5)))
 CASE index=18
  RETURN CFNMETA(-((1/0.5)))+'|'+HexBytes(BINTOC(-((1/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/0.5))))
 CASE index=19
  RETURN CFNMETA(IIF(.T.,(1/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/0.5),0)))
 CASE index=20
  RETURN CFNMETA(-(IIF(.T.,(1/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/0.5),0))))
 CASE index=21
  RETURN CFNMETA((-(1)/0.5))+'|'+HexBytes(BINTOC((-(1)/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((-(1)/0.5)))
 CASE index=22
  RETURN CFNMETA(-((-(1)/0.5)))+'|'+HexBytes(BINTOC(-((-(1)/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1)/0.5))))
 CASE index=23
  RETURN CFNMETA(IIF(.T.,(-(1)/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1)/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1)/0.5),0)))
 CASE index=24
  RETURN CFNMETA(-(IIF(.T.,(-(1)/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1)/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1)/0.5),0))))
 CASE index=25
  RETURN CFNMETA((1/-(0.5)))+'|'+HexBytes(BINTOC((1/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((1/-(0.5))))
 CASE index=26
  RETURN CFNMETA(-((1/-(0.5))))+'|'+HexBytes(BINTOC(-((1/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/-(0.5)))))
 CASE index=27
  RETURN CFNMETA(IIF(.T.,(1/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/-(0.5)),0)))
 CASE index=28
  RETURN CFNMETA(-(IIF(.T.,(1/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/-(0.5)),0))))
 CASE index=29
  RETURN CFNMETA((-(1)/-(0.5)))+'|'+HexBytes(BINTOC((-(1)/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((-(1)/-(0.5))))
 CASE index=30
  RETURN CFNMETA(-((-(1)/-(0.5))))+'|'+HexBytes(BINTOC(-((-(1)/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1)/-(0.5)))))
 CASE index=31
  RETURN CFNMETA(IIF(.T.,(-(1)/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1)/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1)/-(0.5)),0)))
 CASE index=32
  RETURN CFNMETA(-(IIF(.T.,(-(1)/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1)/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1)/-(0.5)),0))))
 CASE index=33
  RETURN CFNMETA((0/0.5))+'|'+HexBytes(BINTOC((0/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((0/0.5)))
 CASE index=34
  RETURN CFNMETA(-((0/0.5)))+'|'+HexBytes(BINTOC(-((0/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0/0.5))))
 CASE index=35
  RETURN CFNMETA(IIF(.T.,(0/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0/0.5),0)))
 CASE index=36
  RETURN CFNMETA(-(IIF(.T.,(0/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0/0.5),0))))
 CASE index=37
  RETURN CFNMETA((-(0)/0.5))+'|'+HexBytes(BINTOC((-(0)/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((-(0)/0.5)))
 CASE index=38
  RETURN CFNMETA(-((-(0)/0.5)))+'|'+HexBytes(BINTOC(-((-(0)/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0)/0.5))))
 CASE index=39
  RETURN CFNMETA(IIF(.T.,(-(0)/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0)/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0)/0.5),0)))
 CASE index=40
  RETURN CFNMETA(-(IIF(.T.,(-(0)/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0)/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0)/0.5),0))))
 CASE index=41
  RETURN CFNMETA((0/-(0.5)))+'|'+HexBytes(BINTOC((0/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((0/-(0.5))))
 CASE index=42
  RETURN CFNMETA(-((0/-(0.5))))+'|'+HexBytes(BINTOC(-((0/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0/-(0.5)))))
 CASE index=43
  RETURN CFNMETA(IIF(.T.,(0/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0/-(0.5)),0)))
 CASE index=44
  RETURN CFNMETA(-(IIF(.T.,(0/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0/-(0.5)),0))))
 CASE index=45
  RETURN CFNMETA((-(0)/-(0.5)))+'|'+HexBytes(BINTOC((-(0)/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0)/-(0.5))))
 CASE index=46
  RETURN CFNMETA(-((-(0)/-(0.5))))+'|'+HexBytes(BINTOC(-((-(0)/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0)/-(0.5)))))
 CASE index=47
  RETURN CFNMETA(IIF(.T.,(-(0)/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0)/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0)/-(0.5)),0)))
 CASE index=48
  RETURN CFNMETA(-(IIF(.T.,(-(0)/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0)/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0)/-(0.5)),0))))
 CASE index=49
  RETURN CFNMETA((0.5/0.25))+'|'+HexBytes(BINTOC((0.5/0.25),'BRS'))+'|'+HexBytes(TRANSFORM((0.5/0.25)))
 CASE index=50
  RETURN CFNMETA(-((0.5/0.25)))+'|'+HexBytes(BINTOC(-((0.5/0.25)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.5/0.25))))
 CASE index=51
  RETURN CFNMETA(IIF(.T.,(0.5/0.25),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.5/0.25),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.5/0.25),0)))
 CASE index=52
  RETURN CFNMETA(-(IIF(.T.,(0.5/0.25),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.5/0.25),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.5/0.25),0))))
 CASE index=53
  RETURN CFNMETA((-(0.5)/0.25))+'|'+HexBytes(BINTOC((-(0.5)/0.25),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.5)/0.25)))
 CASE index=54
  RETURN CFNMETA(-((-(0.5)/0.25)))+'|'+HexBytes(BINTOC(-((-(0.5)/0.25)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.5)/0.25))))
 CASE index=55
  RETURN CFNMETA(IIF(.T.,(-(0.5)/0.25),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.5)/0.25),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.5)/0.25),0)))
 CASE index=56
  RETURN CFNMETA(-(IIF(.T.,(-(0.5)/0.25),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.5)/0.25),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.5)/0.25),0))))
 CASE index=57
  RETURN CFNMETA((0.5/-(0.25)))+'|'+HexBytes(BINTOC((0.5/-(0.25)),'BRS'))+'|'+HexBytes(TRANSFORM((0.5/-(0.25))))
 CASE index=58
  RETURN CFNMETA(-((0.5/-(0.25))))+'|'+HexBytes(BINTOC(-((0.5/-(0.25))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.5/-(0.25)))))
 CASE index=59
  RETURN CFNMETA(IIF(.T.,(0.5/-(0.25)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.5/-(0.25)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.5/-(0.25)),0)))
 CASE index=60
  RETURN CFNMETA(-(IIF(.T.,(0.5/-(0.25)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.5/-(0.25)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.5/-(0.25)),0))))
 CASE index=61
  RETURN CFNMETA((-(0.5)/-(0.25)))+'|'+HexBytes(BINTOC((-(0.5)/-(0.25)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.5)/-(0.25))))
 CASE index=62
  RETURN CFNMETA(-((-(0.5)/-(0.25))))+'|'+HexBytes(BINTOC(-((-(0.5)/-(0.25))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.5)/-(0.25)))))
 CASE index=63
  RETURN CFNMETA(IIF(.T.,(-(0.5)/-(0.25)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.5)/-(0.25)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.5)/-(0.25)),0)))
 CASE index=64
  RETURN CFNMETA(-(IIF(.T.,(-(0.5)/-(0.25)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.5)/-(0.25)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.5)/-(0.25)),0))))
 CASE index=65
  RETURN CFNMETA((2.50/0.50))+'|'+HexBytes(BINTOC((2.50/0.50),'BRS'))+'|'+HexBytes(TRANSFORM((2.50/0.50)))
 CASE index=66
  RETURN CFNMETA(-((2.50/0.50)))+'|'+HexBytes(BINTOC(-((2.50/0.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((2.50/0.50))))
 CASE index=67
  RETURN CFNMETA(IIF(.T.,(2.50/0.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(2.50/0.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(2.50/0.50),0)))
 CASE index=68
  RETURN CFNMETA(-(IIF(.T.,(2.50/0.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(2.50/0.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(2.50/0.50),0))))
 CASE index=69
  RETURN CFNMETA((-(2.50)/0.50))+'|'+HexBytes(BINTOC((-(2.50)/0.50),'BRS'))+'|'+HexBytes(TRANSFORM((-(2.50)/0.50)))
 CASE index=70
  RETURN CFNMETA(-((-(2.50)/0.50)))+'|'+HexBytes(BINTOC(-((-(2.50)/0.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(2.50)/0.50))))
 CASE index=71
  RETURN CFNMETA(IIF(.T.,(-(2.50)/0.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(2.50)/0.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(2.50)/0.50),0)))
 CASE index=72
  RETURN CFNMETA(-(IIF(.T.,(-(2.50)/0.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(2.50)/0.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(2.50)/0.50),0))))
 CASE index=73
  RETURN CFNMETA((2.50/-(0.50)))+'|'+HexBytes(BINTOC((2.50/-(0.50)),'BRS'))+'|'+HexBytes(TRANSFORM((2.50/-(0.50))))
 CASE index=74
  RETURN CFNMETA(-((2.50/-(0.50))))+'|'+HexBytes(BINTOC(-((2.50/-(0.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((2.50/-(0.50)))))
 CASE index=75
  RETURN CFNMETA(IIF(.T.,(2.50/-(0.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(2.50/-(0.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(2.50/-(0.50)),0)))
 CASE index=76
  RETURN CFNMETA(-(IIF(.T.,(2.50/-(0.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(2.50/-(0.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(2.50/-(0.50)),0))))
 CASE index=77
  RETURN CFNMETA((-(2.50)/-(0.50)))+'|'+HexBytes(BINTOC((-(2.50)/-(0.50)),'BRS'))+'|'+HexBytes(TRANSFORM((-(2.50)/-(0.50))))
 CASE index=78
  RETURN CFNMETA(-((-(2.50)/-(0.50))))+'|'+HexBytes(BINTOC(-((-(2.50)/-(0.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(2.50)/-(0.50)))))
 CASE index=79
  RETURN CFNMETA(IIF(.T.,(-(2.50)/-(0.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(2.50)/-(0.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(2.50)/-(0.50)),0)))
 CASE index=80
  RETURN CFNMETA(-(IIF(.T.,(-(2.50)/-(0.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(2.50)/-(0.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(2.50)/-(0.50)),0))))
 CASE index=81
  RETURN CFNMETA((1/0.3))+'|'+HexBytes(BINTOC((1/0.3),'BRS'))+'|'+HexBytes(TRANSFORM((1/0.3)))
 CASE index=82
  RETURN CFNMETA(-((1/0.3)))+'|'+HexBytes(BINTOC(-((1/0.3)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/0.3))))
 CASE index=83
  RETURN CFNMETA(IIF(.T.,(1/0.3),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/0.3),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/0.3),0)))
 CASE index=84
  RETURN CFNMETA(-(IIF(.T.,(1/0.3),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/0.3),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/0.3),0))))
 CASE index=85
  RETURN CFNMETA((-(1)/0.3))+'|'+HexBytes(BINTOC((-(1)/0.3),'BRS'))+'|'+HexBytes(TRANSFORM((-(1)/0.3)))
 CASE index=86
  RETURN CFNMETA(-((-(1)/0.3)))+'|'+HexBytes(BINTOC(-((-(1)/0.3)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1)/0.3))))
 CASE index=87
  RETURN CFNMETA(IIF(.T.,(-(1)/0.3),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1)/0.3),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1)/0.3),0)))
 CASE index=88
  RETURN CFNMETA(-(IIF(.T.,(-(1)/0.3),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1)/0.3),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1)/0.3),0))))
 CASE index=89
  RETURN CFNMETA((1/-(0.3)))+'|'+HexBytes(BINTOC((1/-(0.3)),'BRS'))+'|'+HexBytes(TRANSFORM((1/-(0.3))))
 CASE index=90
  RETURN CFNMETA(-((1/-(0.3))))+'|'+HexBytes(BINTOC(-((1/-(0.3))),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/-(0.3)))))
 CASE index=91
  RETURN CFNMETA(IIF(.T.,(1/-(0.3)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/-(0.3)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/-(0.3)),0)))
 CASE index=92
  RETURN CFNMETA(-(IIF(.T.,(1/-(0.3)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/-(0.3)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/-(0.3)),0))))
 CASE index=93
  RETURN CFNMETA((-(1)/-(0.3)))+'|'+HexBytes(BINTOC((-(1)/-(0.3)),'BRS'))+'|'+HexBytes(TRANSFORM((-(1)/-(0.3))))
 CASE index=94
  RETURN CFNMETA(-((-(1)/-(0.3))))+'|'+HexBytes(BINTOC(-((-(1)/-(0.3))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1)/-(0.3)))))
 CASE index=95
  RETURN CFNMETA(IIF(.T.,(-(1)/-(0.3)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1)/-(0.3)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1)/-(0.3)),0)))
 CASE index=96
  RETURN CFNMETA(-(IIF(.T.,(-(1)/-(0.3)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1)/-(0.3)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1)/-(0.3)),0))))
 CASE index=97
  RETURN CFNMETA((1/0.25))+'|'+HexBytes(BINTOC((1/0.25),'BRS'))+'|'+HexBytes(TRANSFORM((1/0.25)))
 CASE index=98
  RETURN CFNMETA(-((1/0.25)))+'|'+HexBytes(BINTOC(-((1/0.25)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/0.25))))
 CASE index=99
  RETURN CFNMETA(IIF(.T.,(1/0.25),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/0.25),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/0.25),0)))
 CASE index=100
  RETURN CFNMETA(-(IIF(.T.,(1/0.25),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/0.25),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/0.25),0))))
 CASE index=101
  RETURN CFNMETA((-(1)/0.25))+'|'+HexBytes(BINTOC((-(1)/0.25),'BRS'))+'|'+HexBytes(TRANSFORM((-(1)/0.25)))
 CASE index=102
  RETURN CFNMETA(-((-(1)/0.25)))+'|'+HexBytes(BINTOC(-((-(1)/0.25)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1)/0.25))))
 CASE index=103
  RETURN CFNMETA(IIF(.T.,(-(1)/0.25),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1)/0.25),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1)/0.25),0)))
 CASE index=104
  RETURN CFNMETA(-(IIF(.T.,(-(1)/0.25),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1)/0.25),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1)/0.25),0))))
 CASE index=105
  RETURN CFNMETA((1/-(0.25)))+'|'+HexBytes(BINTOC((1/-(0.25)),'BRS'))+'|'+HexBytes(TRANSFORM((1/-(0.25))))
 CASE index=106
  RETURN CFNMETA(-((1/-(0.25))))+'|'+HexBytes(BINTOC(-((1/-(0.25))),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/-(0.25)))))
 CASE index=107
  RETURN CFNMETA(IIF(.T.,(1/-(0.25)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/-(0.25)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/-(0.25)),0)))
 CASE index=108
  RETURN CFNMETA(-(IIF(.T.,(1/-(0.25)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/-(0.25)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/-(0.25)),0))))
 CASE index=109
  RETURN CFNMETA((-(1)/-(0.25)))+'|'+HexBytes(BINTOC((-(1)/-(0.25)),'BRS'))+'|'+HexBytes(TRANSFORM((-(1)/-(0.25))))
 CASE index=110
  RETURN CFNMETA(-((-(1)/-(0.25))))+'|'+HexBytes(BINTOC(-((-(1)/-(0.25))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1)/-(0.25)))))
 CASE index=111
  RETURN CFNMETA(IIF(.T.,(-(1)/-(0.25)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1)/-(0.25)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1)/-(0.25)),0)))
 CASE index=112
  RETURN CFNMETA(-(IIF(.T.,(-(1)/-(0.25)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1)/-(0.25)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1)/-(0.25)),0))))
 CASE index=113
  RETURN CFNMETA((0.000000000000000001/0.5))+'|'+HexBytes(BINTOC((0.000000000000000001/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((0.000000000000000001/0.5)))
 CASE index=114
  RETURN CFNMETA(-((0.000000000000000001/0.5)))+'|'+HexBytes(BINTOC(-((0.000000000000000001/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.000000000000000001/0.5))))
 CASE index=115
  RETURN CFNMETA(IIF(.T.,(0.000000000000000001/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.000000000000000001/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.000000000000000001/0.5),0)))
 CASE index=116
  RETURN CFNMETA(-(IIF(.T.,(0.000000000000000001/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.000000000000000001/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.000000000000000001/0.5),0))))
 CASE index=117
  RETURN CFNMETA((-(0.000000000000000001)/0.5))+'|'+HexBytes(BINTOC((-(0.000000000000000001)/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.000000000000000001)/0.5)))
 CASE index=118
  RETURN CFNMETA(-((-(0.000000000000000001)/0.5)))+'|'+HexBytes(BINTOC(-((-(0.000000000000000001)/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.000000000000000001)/0.5))))
 CASE index=119
  RETURN CFNMETA(IIF(.T.,(-(0.000000000000000001)/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.000000000000000001)/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.000000000000000001)/0.5),0)))
 CASE index=120
  RETURN CFNMETA(-(IIF(.T.,(-(0.000000000000000001)/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.000000000000000001)/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.000000000000000001)/0.5),0))))
 CASE index=121
  RETURN CFNMETA((0.000000000000000001/-(0.5)))+'|'+HexBytes(BINTOC((0.000000000000000001/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((0.000000000000000001/-(0.5))))
 CASE index=122
  RETURN CFNMETA(-((0.000000000000000001/-(0.5))))+'|'+HexBytes(BINTOC(-((0.000000000000000001/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.000000000000000001/-(0.5)))))
 CASE index=123
  RETURN CFNMETA(IIF(.T.,(0.000000000000000001/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.000000000000000001/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.000000000000000001/-(0.5)),0)))
 CASE index=124
  RETURN CFNMETA(-(IIF(.T.,(0.000000000000000001/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.000000000000000001/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.000000000000000001/-(0.5)),0))))
 CASE index=125
  RETURN CFNMETA((-(0.000000000000000001)/-(0.5)))+'|'+HexBytes(BINTOC((-(0.000000000000000001)/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.000000000000000001)/-(0.5))))
 CASE index=126
  RETURN CFNMETA(-((-(0.000000000000000001)/-(0.5))))+'|'+HexBytes(BINTOC(-((-(0.000000000000000001)/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.000000000000000001)/-(0.5)))))
 CASE index=127
  RETURN CFNMETA(IIF(.T.,(-(0.000000000000000001)/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.000000000000000001)/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.000000000000000001)/-(0.5)),0)))
 CASE index=128
  RETURN CFNMETA(-(IIF(.T.,(-(0.000000000000000001)/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.000000000000000001)/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.000000000000000001)/-(0.5)),0))))
 CASE index=129
  RETURN CFNMETA((1.234567890123456789/0.5))+'|'+HexBytes(BINTOC((1.234567890123456789/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789/0.5)))
 CASE index=130
  RETURN CFNMETA(-((1.234567890123456789/0.5)))+'|'+HexBytes(BINTOC(-((1.234567890123456789/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.234567890123456789/0.5))))
 CASE index=131
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.234567890123456789/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.234567890123456789/0.5),0)))
 CASE index=132
  RETURN CFNMETA(-(IIF(.T.,(1.234567890123456789/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.234567890123456789/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.234567890123456789/0.5),0))))
 CASE index=133
  RETURN CFNMETA((-(1.234567890123456789)/0.5))+'|'+HexBytes(BINTOC((-(1.234567890123456789)/0.5),'BRS'))+'|'+HexBytes(TRANSFORM((-(1.234567890123456789)/0.5)))
 CASE index=134
  RETURN CFNMETA(-((-(1.234567890123456789)/0.5)))+'|'+HexBytes(BINTOC(-((-(1.234567890123456789)/0.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1.234567890123456789)/0.5))))
 CASE index=135
  RETURN CFNMETA(IIF(.T.,(-(1.234567890123456789)/0.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1.234567890123456789)/0.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1.234567890123456789)/0.5),0)))
 CASE index=136
  RETURN CFNMETA(-(IIF(.T.,(-(1.234567890123456789)/0.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1.234567890123456789)/0.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1.234567890123456789)/0.5),0))))
 CASE index=137
  RETURN CFNMETA((1.234567890123456789/-(0.5)))+'|'+HexBytes(BINTOC((1.234567890123456789/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789/-(0.5))))
 CASE index=138
  RETURN CFNMETA(-((1.234567890123456789/-(0.5))))+'|'+HexBytes(BINTOC(-((1.234567890123456789/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.234567890123456789/-(0.5)))))
 CASE index=139
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.234567890123456789/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.234567890123456789/-(0.5)),0)))
 CASE index=140
  RETURN CFNMETA(-(IIF(.T.,(1.234567890123456789/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.234567890123456789/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.234567890123456789/-(0.5)),0))))
 CASE index=141
  RETURN CFNMETA((-(1.234567890123456789)/-(0.5)))+'|'+HexBytes(BINTOC((-(1.234567890123456789)/-(0.5)),'BRS'))+'|'+HexBytes(TRANSFORM((-(1.234567890123456789)/-(0.5))))
 CASE index=142
  RETURN CFNMETA(-((-(1.234567890123456789)/-(0.5))))+'|'+HexBytes(BINTOC(-((-(1.234567890123456789)/-(0.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1.234567890123456789)/-(0.5)))))
 CASE index=143
  RETURN CFNMETA(IIF(.T.,(-(1.234567890123456789)/-(0.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1.234567890123456789)/-(0.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1.234567890123456789)/-(0.5)),0)))
 CASE index=144
  RETURN CFNMETA(-(IIF(.T.,(-(1.234567890123456789)/-(0.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1.234567890123456789)/-(0.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1.234567890123456789)/-(0.5)),0))))
 CASE index=145
  RETURN CFNMETA((5/00.50))+'|'+HexBytes(BINTOC((5/00.50),'BRS'))+'|'+HexBytes(TRANSFORM((5/00.50)))
 CASE index=146
  RETURN CFNMETA(-((5/00.50)))+'|'+HexBytes(BINTOC(-((5/00.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/00.50))))
 CASE index=147
  RETURN CFNMETA(IIF(.T.,(5/00.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(5/00.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(5/00.50),0)))
 CASE index=148
  RETURN CFNMETA(-(IIF(.T.,(5/00.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(5/00.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(5/00.50),0))))
 CASE index=149
  RETURN CFNMETA((-(5)/00.50))+'|'+HexBytes(BINTOC((-(5)/00.50),'BRS'))+'|'+HexBytes(TRANSFORM((-(5)/00.50)))
 CASE index=150
  RETURN CFNMETA(-((-(5)/00.50)))+'|'+HexBytes(BINTOC(-((-(5)/00.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(5)/00.50))))
 CASE index=151
  RETURN CFNMETA(IIF(.T.,(-(5)/00.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(5)/00.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(5)/00.50),0)))
 CASE index=152
  RETURN CFNMETA(-(IIF(.T.,(-(5)/00.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(5)/00.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(5)/00.50),0))))
 CASE index=153
  RETURN CFNMETA((5/-(00.50)))+'|'+HexBytes(BINTOC((5/-(00.50)),'BRS'))+'|'+HexBytes(TRANSFORM((5/-(00.50))))
 CASE index=154
  RETURN CFNMETA(-((5/-(00.50))))+'|'+HexBytes(BINTOC(-((5/-(00.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/-(00.50)))))
 CASE index=155
  RETURN CFNMETA(IIF(.T.,(5/-(00.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(5/-(00.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(5/-(00.50)),0)))
 CASE index=156
  RETURN CFNMETA(-(IIF(.T.,(5/-(00.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(5/-(00.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(5/-(00.50)),0))))
 CASE index=157
  RETURN CFNMETA((-(5)/-(00.50)))+'|'+HexBytes(BINTOC((-(5)/-(00.50)),'BRS'))+'|'+HexBytes(TRANSFORM((-(5)/-(00.50))))
 CASE index=158
  RETURN CFNMETA(-((-(5)/-(00.50))))+'|'+HexBytes(BINTOC(-((-(5)/-(00.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(5)/-(00.50)))))
 CASE index=159
  RETURN CFNMETA(IIF(.T.,(-(5)/-(00.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(5)/-(00.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(5)/-(00.50)),0)))
 CASE index=160
  RETURN CFNMETA(-(IIF(.T.,(-(5)/-(00.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(5)/-(00.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(5)/-(00.50)),0))))
 CASE index=161
  RETURN CFNMETA((1/0.125))+'|'+HexBytes(BINTOC((1/0.125),'BRS'))+'|'+HexBytes(TRANSFORM((1/0.125)))
 CASE index=162
  RETURN CFNMETA(-((1/0.125)))+'|'+HexBytes(BINTOC(-((1/0.125)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/0.125))))
 CASE index=163
  RETURN CFNMETA(IIF(.T.,(1/0.125),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/0.125),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/0.125),0)))
 CASE index=164
  RETURN CFNMETA(-(IIF(.T.,(1/0.125),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/0.125),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/0.125),0))))
 CASE index=165
  RETURN CFNMETA((-(1)/0.125))+'|'+HexBytes(BINTOC((-(1)/0.125),'BRS'))+'|'+HexBytes(TRANSFORM((-(1)/0.125)))
 CASE index=166
  RETURN CFNMETA(-((-(1)/0.125)))+'|'+HexBytes(BINTOC(-((-(1)/0.125)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1)/0.125))))
 CASE index=167
  RETURN CFNMETA(IIF(.T.,(-(1)/0.125),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1)/0.125),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1)/0.125),0)))
 CASE index=168
  RETURN CFNMETA(-(IIF(.T.,(-(1)/0.125),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1)/0.125),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1)/0.125),0))))
 CASE index=169
  RETURN CFNMETA((1/-(0.125)))+'|'+HexBytes(BINTOC((1/-(0.125)),'BRS'))+'|'+HexBytes(TRANSFORM((1/-(0.125))))
 CASE index=170
  RETURN CFNMETA(-((1/-(0.125))))+'|'+HexBytes(BINTOC(-((1/-(0.125))),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/-(0.125)))))
 CASE index=171
  RETURN CFNMETA(IIF(.T.,(1/-(0.125)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/-(0.125)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/-(0.125)),0)))
 CASE index=172
  RETURN CFNMETA(-(IIF(.T.,(1/-(0.125)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/-(0.125)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/-(0.125)),0))))
 CASE index=173
  RETURN CFNMETA((-(1)/-(0.125)))+'|'+HexBytes(BINTOC((-(1)/-(0.125)),'BRS'))+'|'+HexBytes(TRANSFORM((-(1)/-(0.125))))
 CASE index=174
  RETURN CFNMETA(-((-(1)/-(0.125))))+'|'+HexBytes(BINTOC(-((-(1)/-(0.125))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1)/-(0.125)))))
 CASE index=175
  RETURN CFNMETA(IIF(.T.,(-(1)/-(0.125)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1)/-(0.125)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1)/-(0.125)),0)))
 CASE index=176
  RETURN CFNMETA(-(IIF(.T.,(-(1)/-(0.125)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1)/-(0.125)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1)/-(0.125)),0))))
 CASE index=177
  RETURN CFNMETA((0/0.000000000000000001))+'|'+HexBytes(BINTOC((0/0.000000000000000001),'BRS'))+'|'+HexBytes(TRANSFORM((0/0.000000000000000001)))
 CASE index=178
  RETURN CFNMETA(-((0/0.000000000000000001)))+'|'+HexBytes(BINTOC(-((0/0.000000000000000001)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0/0.000000000000000001))))
 CASE index=179
  RETURN CFNMETA(IIF(.T.,(0/0.000000000000000001),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0/0.000000000000000001),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0/0.000000000000000001),0)))
 CASE index=180
  RETURN CFNMETA(-(IIF(.T.,(0/0.000000000000000001),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0/0.000000000000000001),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0/0.000000000000000001),0))))
 CASE index=181
  RETURN CFNMETA((-(0)/0.000000000000000001))+'|'+HexBytes(BINTOC((-(0)/0.000000000000000001),'BRS'))+'|'+HexBytes(TRANSFORM((-(0)/0.000000000000000001)))
 CASE index=182
  RETURN CFNMETA(-((-(0)/0.000000000000000001)))+'|'+HexBytes(BINTOC(-((-(0)/0.000000000000000001)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0)/0.000000000000000001))))
 CASE index=183
  RETURN CFNMETA(IIF(.T.,(-(0)/0.000000000000000001),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0)/0.000000000000000001),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0)/0.000000000000000001),0)))
 CASE index=184
  RETURN CFNMETA(-(IIF(.T.,(-(0)/0.000000000000000001),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0)/0.000000000000000001),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0)/0.000000000000000001),0))))
 CASE index=185
  RETURN CFNMETA((0/-(0.000000000000000001)))+'|'+HexBytes(BINTOC((0/-(0.000000000000000001)),'BRS'))+'|'+HexBytes(TRANSFORM((0/-(0.000000000000000001))))
 CASE index=186
  RETURN CFNMETA(-((0/-(0.000000000000000001))))+'|'+HexBytes(BINTOC(-((0/-(0.000000000000000001))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0/-(0.000000000000000001)))))
 CASE index=187
  RETURN CFNMETA(IIF(.T.,(0/-(0.000000000000000001)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0/-(0.000000000000000001)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0/-(0.000000000000000001)),0)))
 CASE index=188
  RETURN CFNMETA(-(IIF(.T.,(0/-(0.000000000000000001)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0/-(0.000000000000000001)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0/-(0.000000000000000001)),0))))
 CASE index=189
  RETURN CFNMETA((-(0)/-(0.000000000000000001)))+'|'+HexBytes(BINTOC((-(0)/-(0.000000000000000001)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0)/-(0.000000000000000001))))
 CASE index=190
  RETURN CFNMETA(-((-(0)/-(0.000000000000000001))))+'|'+HexBytes(BINTOC(-((-(0)/-(0.000000000000000001))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0)/-(0.000000000000000001)))))
 CASE index=191
  RETURN CFNMETA(IIF(.T.,(-(0)/-(0.000000000000000001)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0)/-(0.000000000000000001)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0)/-(0.000000000000000001)),0)))
 CASE index=192
  RETURN CFNMETA(-(IIF(.T.,(-(0)/-(0.000000000000000001)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0)/-(0.000000000000000001)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0)/-(0.000000000000000001)),0))))
 CASE index=193
  RETURN CFNMETA((1/3)/(1/3))+'|'+HexBytes(BINTOC((1/3)/(1/3),'BRS'))+'|'+HexBytes(TRANSFORM((1/3)/(1/3)))
 CASE index=194
  RETURN CFNMETA(-((1/3)/(1/3)))+'|'+HexBytes(BINTOC(-((1/3)/(1/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/3)/(1/3))))
 CASE index=195
  RETURN CFNMETA(IIF(.T.,(1/3)/(1/3),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/3)/(1/3),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/3)/(1/3),0)))
 CASE index=196
  RETURN CFNMETA(-(IIF(.T.,(1/3)/(1/3),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/3)/(1/3),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/3)/(1/3),0))))
 CASE index=197
  RETURN CFNMETA(5/(1/3))+'|'+HexBytes(BINTOC(5/(1/3),'BRS'))+'|'+HexBytes(TRANSFORM(5/(1/3)))
 CASE index=198
  RETURN CFNMETA(-(5/(1/3)))+'|'+HexBytes(BINTOC(-(5/(1/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-(5/(1/3))))
 CASE index=199
  RETURN CFNMETA(IIF(.T.,5/(1/3),0))+'|'+HexBytes(BINTOC(IIF(.T.,5/(1/3),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,5/(1/3),0)))
 CASE index=200
  RETURN CFNMETA(-(IIF(.T.,5/(1/3),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,5/(1/3),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,5/(1/3),0))))
 CASE index=201
  RETURN CFNMETA((1/7)/(1/7))+'|'+HexBytes(BINTOC((1/7)/(1/7),'BRS'))+'|'+HexBytes(TRANSFORM((1/7)/(1/7)))
 CASE index=202
  RETURN CFNMETA(-((1/7)/(1/7)))+'|'+HexBytes(BINTOC(-((1/7)/(1/7)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/7)/(1/7))))
 CASE index=203
  RETURN CFNMETA(IIF(.T.,(1/7)/(1/7),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/7)/(1/7),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/7)/(1/7),0)))
 CASE index=204
  RETURN CFNMETA(-(IIF(.T.,(1/7)/(1/7),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/7)/(1/7),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/7)/(1/7),0))))
 CASE index=205
  RETURN CFNMETA((0.5/2)/(0.5/2))+'|'+HexBytes(BINTOC((0.5/2)/(0.5/2),'BRS'))+'|'+HexBytes(TRANSFORM((0.5/2)/(0.5/2)))
 CASE index=206
  RETURN CFNMETA(-((0.5/2)/(0.5/2)))+'|'+HexBytes(BINTOC(-((0.5/2)/(0.5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.5/2)/(0.5/2))))
 CASE index=207
  RETURN CFNMETA(IIF(.T.,(0.5/2)/(0.5/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.5/2)/(0.5/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.5/2)/(0.5/2),0)))
 CASE index=208
  RETURN CFNMETA(-(IIF(.T.,(0.5/2)/(0.5/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.5/2)/(0.5/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.5/2)/(0.5/2),0))))
 CASE index=209
  RETURN CFNMETA(((1/3)/(1/3))/2)+'|'+HexBytes(BINTOC(((1/3)/(1/3))/2,'BRS'))+'|'+HexBytes(TRANSFORM(((1/3)/(1/3))/2))
 CASE index=210
  RETURN CFNMETA(-(((1/3)/(1/3))/2))+'|'+HexBytes(BINTOC(-(((1/3)/(1/3))/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(((1/3)/(1/3))/2)))
 CASE index=211
  RETURN CFNMETA(IIF(.T.,((1/3)/(1/3))/2,0))+'|'+HexBytes(BINTOC(IIF(.T.,((1/3)/(1/3))/2,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((1/3)/(1/3))/2,0)))
 CASE index=212
  RETURN CFNMETA(-(IIF(.T.,((1/3)/(1/3))/2,0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((1/3)/(1/3))/2,0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((1/3)/(1/3))/2,0))))
 CASE index=213
  RETURN CFNMETA((1/3)/((1/3)/2))+'|'+HexBytes(BINTOC((1/3)/((1/3)/2),'BRS'))+'|'+HexBytes(TRANSFORM((1/3)/((1/3)/2)))
 CASE index=214
  RETURN CFNMETA(-((1/3)/((1/3)/2)))+'|'+HexBytes(BINTOC(-((1/3)/((1/3)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/3)/((1/3)/2))))
 CASE index=215
  RETURN CFNMETA(IIF(.T.,(1/3)/((1/3)/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1/3)/((1/3)/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1/3)/((1/3)/2),0)))
 CASE index=216
  RETURN CFNMETA(-(IIF(.T.,(1/3)/((1/3)/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1/3)/((1/3)/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1/3)/((1/3)/2),0))))
 CASE index=217
  RETURN CFNMETA((5/2)/(1/3))+'|'+HexBytes(BINTOC((5/2)/(1/3),'BRS'))+'|'+HexBytes(TRANSFORM((5/2)/(1/3)))
 CASE index=218
  RETURN CFNMETA(-((5/2)/(1/3)))+'|'+HexBytes(BINTOC(-((5/2)/(1/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/2)/(1/3))))
 CASE index=219
  RETURN CFNMETA(IIF(.T.,(5/2)/(1/3),0))+'|'+HexBytes(BINTOC(IIF(.T.,(5/2)/(1/3),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(5/2)/(1/3),0)))
 CASE index=220
  RETURN CFNMETA(-(IIF(.T.,(5/2)/(1/3),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(5/2)/(1/3),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(5/2)/(1/3),0))))
 CASE index=221
  RETURN CFNMETA(((5/2)/(1/3))/2)+'|'+HexBytes(BINTOC(((5/2)/(1/3))/2,'BRS'))+'|'+HexBytes(TRANSFORM(((5/2)/(1/3))/2))
 CASE index=222
  RETURN CFNMETA(-(((5/2)/(1/3))/2))+'|'+HexBytes(BINTOC(-(((5/2)/(1/3))/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(((5/2)/(1/3))/2)))
 CASE index=223
  RETURN CFNMETA(IIF(.T.,((5/2)/(1/3))/2,0))+'|'+HexBytes(BINTOC(IIF(.T.,((5/2)/(1/3))/2,0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((5/2)/(1/3))/2,0)))
 CASE index=224
  RETURN CFNMETA(-(IIF(.T.,((5/2)/(1/3))/2,0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((5/2)/(1/3))/2,0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((5/2)/(1/3))/2,0))))
 CASE index=225
  RETURN CFNMETA(2)+'|'+HexBytes(BINTOC(2,'BRS'))+'|'+HexBytes(TRANSFORM(2))
 CASE index=226
  RETURN CFNMETA(3)+'|'+HexBytes(BINTOC(3,'BRS'))+'|'+HexBytes(TRANSFORM(3))
 CASE index=227
  RETURN CFNMETA(7)+'|'+HexBytes(BINTOC(7,'BRS'))+'|'+HexBytes(TRANSFORM(7))
 CASE index=228
  RETURN CFNMETA(5)+'|'+HexBytes(BINTOC(5,'BRS'))+'|'+HexBytes(TRANSFORM(5))
 CASE index=229
  RETURN CFNMETA(0.5)+'|'+HexBytes(BINTOC(0.5,'BRS'))+'|'+HexBytes(TRANSFORM(0.5))
 CASE index=230
  RETURN CFNMETA(1)+'|'+HexBytes(BINTOC(1,'BRS'))+'|'+HexBytes(TRANSFORM(1))
 CASE index=231
  RETURN CFNMETA(0)+'|'+HexBytes(BINTOC(0,'BRS'))+'|'+HexBytes(TRANSFORM(0))
 CASE index=232
  RETURN CFNMETA(0.25)+'|'+HexBytes(BINTOC(0.25,'BRS'))+'|'+HexBytes(TRANSFORM(0.25))
 CASE index=233
  RETURN CFNMETA(2.50)+'|'+HexBytes(BINTOC(2.50,'BRS'))+'|'+HexBytes(TRANSFORM(2.50))
 CASE index=234
  RETURN CFNMETA(0.50)+'|'+HexBytes(BINTOC(0.50,'BRS'))+'|'+HexBytes(TRANSFORM(0.50))
 CASE index=235
  RETURN CFNMETA(0.3)+'|'+HexBytes(BINTOC(0.3,'BRS'))+'|'+HexBytes(TRANSFORM(0.3))
 CASE index=236
  RETURN CFNMETA(0.000000000000000001)+'|'+HexBytes(BINTOC(0.000000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000000000000001))
 CASE index=237
  RETURN CFNMETA(1.234567890123456789)+'|'+HexBytes(BINTOC(1.234567890123456789,'BRS'))+'|'+HexBytes(TRANSFORM(1.234567890123456789))
 CASE index=238
  RETURN CFNMETA(00.50)+'|'+HexBytes(BINTOC(00.50,'BRS'))+'|'+HexBytes(TRANSFORM(00.50))
 CASE index=239
  RETURN CFNMETA(0.125)+'|'+HexBytes(BINTOC(0.125,'BRS'))+'|'+HexBytes(TRANSFORM(0.125))
 CASE index=240
  RETURN CFNMETA((5/0.5)/iTwo)+'|'+HexBytes(BINTOC((5/0.5)/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM((5/0.5)/iTwo))
 CASE index=241
  RETURN CFNMETA(iTwo/(1/3))+'|'+HexBytes(BINTOC(iTwo/(1/3),'BRS'))+'|'+HexBytes(TRANSFORM(iTwo/(1/3)))
 CASE index=242
  RETURN CFNMETA((1/3)/nTwo)+'|'+HexBytes(BINTOC((1/3)/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM((1/3)/nTwo))
 CASE index=243
  RETURN CFNMETA(nTwo/(1/3))+'|'+HexBytes(BINTOC(nTwo/(1/3),'BRS'))+'|'+HexBytes(TRANSFORM(nTwo/(1/3)))
 CASE index=244
  RETURN CFNMETA(-((5/0.5)/iTwo))+'|'+HexBytes(BINTOC(-((5/0.5)/iTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/0.5)/iTwo)))
 CASE index=245
  RETURN CFNMETA(-(iTwo/(1/3)))+'|'+HexBytes(BINTOC(-(iTwo/(1/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-(iTwo/(1/3))))
 CASE index=246
  RETURN CFNMETA(1/0)+'|'+HexBytes(BINTOC(1/0,'BRS'))+'|'+HexBytes(TRANSFORM(1/0))
 CASE index=247
  RETURN CFNMETA(0/0)+'|'+HexBytes(BINTOC(0/0,'BRS'))+'|'+HexBytes(TRANSFORM(0/0))
 CASE index=248
  RETURN CFNMETA(5/-0.00)+'|'+HexBytes(BINTOC(5/-0.00,'BRS'))+'|'+HexBytes(TRANSFORM(5/-0.00))
 CASE index=249
  RETURN CFNMETA(1/0.0000000001)+'|'+HexBytes(BINTOC(1/0.0000000001,'BRS'))+'|'+HexBytes(TRANSFORM(1/0.0000000001))
 CASE index=250
  RETURN CFNMETA(1/0.000000000000000001)+'|'+HexBytes(BINTOC(1/0.000000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(1/0.000000000000000001))
 CASE index=251
  RETURN CFNMETA(1E-14/0.5)+'|'+HexBytes(BINTOC(1E-14/0.5,'BRS'))+'|'+HexBytes(TRANSFORM(1E-14/0.5))
 CASE index=252
  RETURN CFNMETA(9999999999/0.5)+'|'+HexBytes(BINTOC(9999999999/0.5,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999/0.5))
 CASE index=253
  RETURN CFNMETA(5/1E-2)+'|'+HexBytes(BINTOC(5/1E-2,'BRS'))+'|'+HexBytes(TRANSFORM(5/1E-2))
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
