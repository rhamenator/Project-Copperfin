* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DIVISION-CONTEXT-001: constant payload/width/promotion recovery.
LOCAL ARRAY expressions[320], lefts[320], rights[320], settings[5]
expressions[1]='0/2'
lefts[1]='0'
rights[1]='2'
expressions[2]='-(0/2)'
lefts[2]='0'
rights[2]='2'
expressions[3]='-0/2'
lefts[3]='-0'
rights[3]='2'
expressions[4]='-(-0/2)'
lefts[4]='-0'
rights[4]='2'
expressions[5]='0/-2'
lefts[5]='0'
rights[5]='-2'
expressions[6]='-(0/-2)'
lefts[6]='0'
rights[6]='-2'
expressions[7]='-0/-2'
lefts[7]='-0'
rights[7]='-2'
expressions[8]='-(-0/-2)'
lefts[8]='-0'
rights[8]='-2'
expressions[9]='00/2'
lefts[9]='00'
rights[9]='2'
expressions[10]='-(00/2)'
lefts[10]='00'
rights[10]='2'
expressions[11]='-00/2'
lefts[11]='-00'
rights[11]='2'
expressions[12]='-(-00/2)'
lefts[12]='-00'
rights[12]='2'
expressions[13]='00/-2'
lefts[13]='00'
rights[13]='-2'
expressions[14]='-(00/-2)'
lefts[14]='00'
rights[14]='-2'
expressions[15]='-00/-2'
lefts[15]='-00'
rights[15]='-2'
expressions[16]='-(-00/-2)'
lefts[16]='-00'
rights[16]='-2'
expressions[17]='0/02'
lefts[17]='0'
rights[17]='02'
expressions[18]='-(0/02)'
lefts[18]='0'
rights[18]='02'
expressions[19]='-0/02'
lefts[19]='-0'
rights[19]='02'
expressions[20]='-(-0/02)'
lefts[20]='-0'
rights[20]='02'
expressions[21]='0/-02'
lefts[21]='0'
rights[21]='-02'
expressions[22]='-(0/-02)'
lefts[22]='0'
rights[22]='-02'
expressions[23]='-0/-02'
lefts[23]='-0'
rights[23]='-02'
expressions[24]='-(-0/-02)'
lefts[24]='-0'
rights[24]='-02'
expressions[25]='0.00/2'
lefts[25]='0.00'
rights[25]='2'
expressions[26]='-(0.00/2)'
lefts[26]='0.00'
rights[26]='2'
expressions[27]='-0.00/2'
lefts[27]='-0.00'
rights[27]='2'
expressions[28]='-(-0.00/2)'
lefts[28]='-0.00'
rights[28]='2'
expressions[29]='0.00/-2'
lefts[29]='0.00'
rights[29]='-2'
expressions[30]='-(0.00/-2)'
lefts[30]='0.00'
rights[30]='-2'
expressions[31]='-0.00/-2'
lefts[31]='-0.00'
rights[31]='-2'
expressions[32]='-(-0.00/-2)'
lefts[32]='-0.00'
rights[32]='-2'
expressions[33]='0/2.00'
lefts[33]='0'
rights[33]='2.00'
expressions[34]='-(0/2.00)'
lefts[34]='0'
rights[34]='2.00'
expressions[35]='-0/2.00'
lefts[35]='-0'
rights[35]='2.00'
expressions[36]='-(-0/2.00)'
lefts[36]='-0'
rights[36]='2.00'
expressions[37]='0/-2.00'
lefts[37]='0'
rights[37]='-2.00'
expressions[38]='-(0/-2.00)'
lefts[38]='0'
rights[38]='-2.00'
expressions[39]='-0/-2.00'
lefts[39]='-0'
rights[39]='-2.00'
expressions[40]='-(-0/-2.00)'
lefts[40]='-0'
rights[40]='-2.00'
expressions[41]='0.00/2.00'
lefts[41]='0.00'
rights[41]='2.00'
expressions[42]='-(0.00/2.00)'
lefts[42]='0.00'
rights[42]='2.00'
expressions[43]='-0.00/2.00'
lefts[43]='-0.00'
rights[43]='2.00'
expressions[44]='-(-0.00/2.00)'
lefts[44]='-0.00'
rights[44]='2.00'
expressions[45]='0.00/-2.00'
lefts[45]='0.00'
rights[45]='-2.00'
expressions[46]='-(0.00/-2.00)'
lefts[46]='0.00'
rights[46]='-2.00'
expressions[47]='-0.00/-2.00'
lefts[47]='-0.00'
rights[47]='-2.00'
expressions[48]='-(-0.00/-2.00)'
lefts[48]='-0.00'
rights[48]='-2.00'
expressions[49]='0.000000/2.00'
lefts[49]='0.000000'
rights[49]='2.00'
expressions[50]='-(0.000000/2.00)'
lefts[50]='0.000000'
rights[50]='2.00'
expressions[51]='-0.000000/2.00'
lefts[51]='-0.000000'
rights[51]='2.00'
expressions[52]='-(-0.000000/2.00)'
lefts[52]='-0.000000'
rights[52]='2.00'
expressions[53]='0.000000/-2.00'
lefts[53]='0.000000'
rights[53]='-2.00'
expressions[54]='-(0.000000/-2.00)'
lefts[54]='0.000000'
rights[54]='-2.00'
expressions[55]='-0.000000/-2.00'
lefts[55]='-0.000000'
rights[55]='-2.00'
expressions[56]='-(-0.000000/-2.00)'
lefts[56]='-0.000000'
rights[56]='-2.00'
expressions[57]='0.000000000000000000/2.000000000000000000'
lefts[57]='0.000000000000000000'
rights[57]='2.000000000000000000'
expressions[58]='-(0.000000000000000000/2.000000000000000000)'
lefts[58]='0.000000000000000000'
rights[58]='2.000000000000000000'
expressions[59]='-0.000000000000000000/2.000000000000000000'
lefts[59]='-0.000000000000000000'
rights[59]='2.000000000000000000'
expressions[60]='-(-0.000000000000000000/2.000000000000000000)'
lefts[60]='-0.000000000000000000'
rights[60]='2.000000000000000000'
expressions[61]='0.000000000000000000/-2.000000000000000000'
lefts[61]='0.000000000000000000'
rights[61]='-2.000000000000000000'
expressions[62]='-(0.000000000000000000/-2.000000000000000000)'
lefts[62]='0.000000000000000000'
rights[62]='-2.000000000000000000'
expressions[63]='-0.000000000000000000/-2.000000000000000000'
lefts[63]='-0.000000000000000000'
rights[63]='-2.000000000000000000'
expressions[64]='-(-0.000000000000000000/-2.000000000000000000)'
lefts[64]='-0.000000000000000000'
rights[64]='-2.000000000000000000'
expressions[65]='1/7'
lefts[65]='1'
rights[65]='7'
expressions[66]='-(1/7)'
lefts[66]='1'
rights[66]='7'
expressions[67]='-1/7'
lefts[67]='-1'
rights[67]='7'
expressions[68]='-(-1/7)'
lefts[68]='-1'
rights[68]='7'
expressions[69]='1/-7'
lefts[69]='1'
rights[69]='-7'
expressions[70]='-(1/-7)'
lefts[70]='1'
rights[70]='-7'
expressions[71]='-1/-7'
lefts[71]='-1'
rights[71]='-7'
expressions[72]='-(-1/-7)'
lefts[72]='-1'
rights[72]='-7'
expressions[73]='01/7'
lefts[73]='01'
rights[73]='7'
expressions[74]='-(01/7)'
lefts[74]='01'
rights[74]='7'
expressions[75]='-01/7'
lefts[75]='-01'
rights[75]='7'
expressions[76]='-(-01/7)'
lefts[76]='-01'
rights[76]='7'
expressions[77]='01/-7'
lefts[77]='01'
rights[77]='-7'
expressions[78]='-(01/-7)'
lefts[78]='01'
rights[78]='-7'
expressions[79]='-01/-7'
lefts[79]='-01'
rights[79]='-7'
expressions[80]='-(-01/-7)'
lefts[80]='-01'
rights[80]='-7'
expressions[81]='1/07'
lefts[81]='1'
rights[81]='07'
expressions[82]='-(1/07)'
lefts[82]='1'
rights[82]='07'
expressions[83]='-1/07'
lefts[83]='-1'
rights[83]='07'
expressions[84]='-(-1/07)'
lefts[84]='-1'
rights[84]='07'
expressions[85]='1/-07'
lefts[85]='1'
rights[85]='-07'
expressions[86]='-(1/-07)'
lefts[86]='1'
rights[86]='-07'
expressions[87]='-1/-07'
lefts[87]='-1'
rights[87]='-07'
expressions[88]='-(-1/-07)'
lefts[88]='-1'
rights[88]='-07'
expressions[89]='1.00/7'
lefts[89]='1.00'
rights[89]='7'
expressions[90]='-(1.00/7)'
lefts[90]='1.00'
rights[90]='7'
expressions[91]='-1.00/7'
lefts[91]='-1.00'
rights[91]='7'
expressions[92]='-(-1.00/7)'
lefts[92]='-1.00'
rights[92]='7'
expressions[93]='1.00/-7'
lefts[93]='1.00'
rights[93]='-7'
expressions[94]='-(1.00/-7)'
lefts[94]='1.00'
rights[94]='-7'
expressions[95]='-1.00/-7'
lefts[95]='-1.00'
rights[95]='-7'
expressions[96]='-(-1.00/-7)'
lefts[96]='-1.00'
rights[96]='-7'
expressions[97]='1/7.00'
lefts[97]='1'
rights[97]='7.00'
expressions[98]='-(1/7.00)'
lefts[98]='1'
rights[98]='7.00'
expressions[99]='-1/7.00'
lefts[99]='-1'
rights[99]='7.00'
expressions[100]='-(-1/7.00)'
lefts[100]='-1'
rights[100]='7.00'
expressions[101]='1/-7.00'
lefts[101]='1'
rights[101]='-7.00'
expressions[102]='-(1/-7.00)'
lefts[102]='1'
rights[102]='-7.00'
expressions[103]='-1/-7.00'
lefts[103]='-1'
rights[103]='-7.00'
expressions[104]='-(-1/-7.00)'
lefts[104]='-1'
rights[104]='-7.00'
expressions[105]='1.00/7.00'
lefts[105]='1.00'
rights[105]='7.00'
expressions[106]='-(1.00/7.00)'
lefts[106]='1.00'
rights[106]='7.00'
expressions[107]='-1.00/7.00'
lefts[107]='-1.00'
rights[107]='7.00'
expressions[108]='-(-1.00/7.00)'
lefts[108]='-1.00'
rights[108]='7.00'
expressions[109]='1.00/-7.00'
lefts[109]='1.00'
rights[109]='-7.00'
expressions[110]='-(1.00/-7.00)'
lefts[110]='1.00'
rights[110]='-7.00'
expressions[111]='-1.00/-7.00'
lefts[111]='-1.00'
rights[111]='-7.00'
expressions[112]='-(-1.00/-7.00)'
lefts[112]='-1.00'
rights[112]='-7.00'
expressions[113]='1.000000/7.00'
lefts[113]='1.000000'
rights[113]='7.00'
expressions[114]='-(1.000000/7.00)'
lefts[114]='1.000000'
rights[114]='7.00'
expressions[115]='-1.000000/7.00'
lefts[115]='-1.000000'
rights[115]='7.00'
expressions[116]='-(-1.000000/7.00)'
lefts[116]='-1.000000'
rights[116]='7.00'
expressions[117]='1.000000/-7.00'
lefts[117]='1.000000'
rights[117]='-7.00'
expressions[118]='-(1.000000/-7.00)'
lefts[118]='1.000000'
rights[118]='-7.00'
expressions[119]='-1.000000/-7.00'
lefts[119]='-1.000000'
rights[119]='-7.00'
expressions[120]='-(-1.000000/-7.00)'
lefts[120]='-1.000000'
rights[120]='-7.00'
expressions[121]='1.000000000000000000/7.000000000000000000'
lefts[121]='1.000000000000000000'
rights[121]='7.000000000000000000'
expressions[122]='-(1.000000000000000000/7.000000000000000000)'
lefts[122]='1.000000000000000000'
rights[122]='7.000000000000000000'
expressions[123]='-1.000000000000000000/7.000000000000000000'
lefts[123]='-1.000000000000000000'
rights[123]='7.000000000000000000'
expressions[124]='-(-1.000000000000000000/7.000000000000000000)'
lefts[124]='-1.000000000000000000'
rights[124]='7.000000000000000000'
expressions[125]='1.000000000000000000/-7.000000000000000000'
lefts[125]='1.000000000000000000'
rights[125]='-7.000000000000000000'
expressions[126]='-(1.000000000000000000/-7.000000000000000000)'
lefts[126]='1.000000000000000000'
rights[126]='-7.000000000000000000'
expressions[127]='-1.000000000000000000/-7.000000000000000000'
lefts[127]='-1.000000000000000000'
rights[127]='-7.000000000000000000'
expressions[128]='-(-1.000000000000000000/-7.000000000000000000)'
lefts[128]='-1.000000000000000000'
rights[128]='-7.000000000000000000'
expressions[129]='10/2'
lefts[129]='10'
rights[129]='2'
expressions[130]='-(10/2)'
lefts[130]='10'
rights[130]='2'
expressions[131]='-10/2'
lefts[131]='-10'
rights[131]='2'
expressions[132]='-(-10/2)'
lefts[132]='-10'
rights[132]='2'
expressions[133]='10/-2'
lefts[133]='10'
rights[133]='-2'
expressions[134]='-(10/-2)'
lefts[134]='10'
rights[134]='-2'
expressions[135]='-10/-2'
lefts[135]='-10'
rights[135]='-2'
expressions[136]='-(-10/-2)'
lefts[136]='-10'
rights[136]='-2'
expressions[137]='010/2'
lefts[137]='010'
rights[137]='2'
expressions[138]='-(010/2)'
lefts[138]='010'
rights[138]='2'
expressions[139]='-010/2'
lefts[139]='-010'
rights[139]='2'
expressions[140]='-(-010/2)'
lefts[140]='-010'
rights[140]='2'
expressions[141]='010/-2'
lefts[141]='010'
rights[141]='-2'
expressions[142]='-(010/-2)'
lefts[142]='010'
rights[142]='-2'
expressions[143]='-010/-2'
lefts[143]='-010'
rights[143]='-2'
expressions[144]='-(-010/-2)'
lefts[144]='-010'
rights[144]='-2'
expressions[145]='10/02'
lefts[145]='10'
rights[145]='02'
expressions[146]='-(10/02)'
lefts[146]='10'
rights[146]='02'
expressions[147]='-10/02'
lefts[147]='-10'
rights[147]='02'
expressions[148]='-(-10/02)'
lefts[148]='-10'
rights[148]='02'
expressions[149]='10/-02'
lefts[149]='10'
rights[149]='-02'
expressions[150]='-(10/-02)'
lefts[150]='10'
rights[150]='-02'
expressions[151]='-10/-02'
lefts[151]='-10'
rights[151]='-02'
expressions[152]='-(-10/-02)'
lefts[152]='-10'
rights[152]='-02'
expressions[153]='10.00/2'
lefts[153]='10.00'
rights[153]='2'
expressions[154]='-(10.00/2)'
lefts[154]='10.00'
rights[154]='2'
expressions[155]='-10.00/2'
lefts[155]='-10.00'
rights[155]='2'
expressions[156]='-(-10.00/2)'
lefts[156]='-10.00'
rights[156]='2'
expressions[157]='10.00/-2'
lefts[157]='10.00'
rights[157]='-2'
expressions[158]='-(10.00/-2)'
lefts[158]='10.00'
rights[158]='-2'
expressions[159]='-10.00/-2'
lefts[159]='-10.00'
rights[159]='-2'
expressions[160]='-(-10.00/-2)'
lefts[160]='-10.00'
rights[160]='-2'
expressions[161]='10/2.00'
lefts[161]='10'
rights[161]='2.00'
expressions[162]='-(10/2.00)'
lefts[162]='10'
rights[162]='2.00'
expressions[163]='-10/2.00'
lefts[163]='-10'
rights[163]='2.00'
expressions[164]='-(-10/2.00)'
lefts[164]='-10'
rights[164]='2.00'
expressions[165]='10/-2.00'
lefts[165]='10'
rights[165]='-2.00'
expressions[166]='-(10/-2.00)'
lefts[166]='10'
rights[166]='-2.00'
expressions[167]='-10/-2.00'
lefts[167]='-10'
rights[167]='-2.00'
expressions[168]='-(-10/-2.00)'
lefts[168]='-10'
rights[168]='-2.00'
expressions[169]='10.00/2.00'
lefts[169]='10.00'
rights[169]='2.00'
expressions[170]='-(10.00/2.00)'
lefts[170]='10.00'
rights[170]='2.00'
expressions[171]='-10.00/2.00'
lefts[171]='-10.00'
rights[171]='2.00'
expressions[172]='-(-10.00/2.00)'
lefts[172]='-10.00'
rights[172]='2.00'
expressions[173]='10.00/-2.00'
lefts[173]='10.00'
rights[173]='-2.00'
expressions[174]='-(10.00/-2.00)'
lefts[174]='10.00'
rights[174]='-2.00'
expressions[175]='-10.00/-2.00'
lefts[175]='-10.00'
rights[175]='-2.00'
expressions[176]='-(-10.00/-2.00)'
lefts[176]='-10.00'
rights[176]='-2.00'
expressions[177]='10.000000/2.00'
lefts[177]='10.000000'
rights[177]='2.00'
expressions[178]='-(10.000000/2.00)'
lefts[178]='10.000000'
rights[178]='2.00'
expressions[179]='-10.000000/2.00'
lefts[179]='-10.000000'
rights[179]='2.00'
expressions[180]='-(-10.000000/2.00)'
lefts[180]='-10.000000'
rights[180]='2.00'
expressions[181]='10.000000/-2.00'
lefts[181]='10.000000'
rights[181]='-2.00'
expressions[182]='-(10.000000/-2.00)'
lefts[182]='10.000000'
rights[182]='-2.00'
expressions[183]='-10.000000/-2.00'
lefts[183]='-10.000000'
rights[183]='-2.00'
expressions[184]='-(-10.000000/-2.00)'
lefts[184]='-10.000000'
rights[184]='-2.00'
expressions[185]='10.000000000000000000/2.000000000000000000'
lefts[185]='10.000000000000000000'
rights[185]='2.000000000000000000'
expressions[186]='-(10.000000000000000000/2.000000000000000000)'
lefts[186]='10.000000000000000000'
rights[186]='2.000000000000000000'
expressions[187]='-10.000000000000000000/2.000000000000000000'
lefts[187]='-10.000000000000000000'
rights[187]='2.000000000000000000'
expressions[188]='-(-10.000000000000000000/2.000000000000000000)'
lefts[188]='-10.000000000000000000'
rights[188]='2.000000000000000000'
expressions[189]='10.000000000000000000/-2.000000000000000000'
lefts[189]='10.000000000000000000'
rights[189]='-2.000000000000000000'
expressions[190]='-(10.000000000000000000/-2.000000000000000000)'
lefts[190]='10.000000000000000000'
rights[190]='-2.000000000000000000'
expressions[191]='-10.000000000000000000/-2.000000000000000000'
lefts[191]='-10.000000000000000000'
rights[191]='-2.000000000000000000'
expressions[192]='-(-10.000000000000000000/-2.000000000000000000)'
lefts[192]='-10.000000000000000000'
rights[192]='-2.000000000000000000'
expressions[193]='100/2'
lefts[193]='100'
rights[193]='2'
expressions[194]='-(100/2)'
lefts[194]='100'
rights[194]='2'
expressions[195]='-100/2'
lefts[195]='-100'
rights[195]='2'
expressions[196]='-(-100/2)'
lefts[196]='-100'
rights[196]='2'
expressions[197]='100/-2'
lefts[197]='100'
rights[197]='-2'
expressions[198]='-(100/-2)'
lefts[198]='100'
rights[198]='-2'
expressions[199]='-100/-2'
lefts[199]='-100'
rights[199]='-2'
expressions[200]='-(-100/-2)'
lefts[200]='-100'
rights[200]='-2'
expressions[201]='0100/2'
lefts[201]='0100'
rights[201]='2'
expressions[202]='-(0100/2)'
lefts[202]='0100'
rights[202]='2'
expressions[203]='-0100/2'
lefts[203]='-0100'
rights[203]='2'
expressions[204]='-(-0100/2)'
lefts[204]='-0100'
rights[204]='2'
expressions[205]='0100/-2'
lefts[205]='0100'
rights[205]='-2'
expressions[206]='-(0100/-2)'
lefts[206]='0100'
rights[206]='-2'
expressions[207]='-0100/-2'
lefts[207]='-0100'
rights[207]='-2'
expressions[208]='-(-0100/-2)'
lefts[208]='-0100'
rights[208]='-2'
expressions[209]='100/02'
lefts[209]='100'
rights[209]='02'
expressions[210]='-(100/02)'
lefts[210]='100'
rights[210]='02'
expressions[211]='-100/02'
lefts[211]='-100'
rights[211]='02'
expressions[212]='-(-100/02)'
lefts[212]='-100'
rights[212]='02'
expressions[213]='100/-02'
lefts[213]='100'
rights[213]='-02'
expressions[214]='-(100/-02)'
lefts[214]='100'
rights[214]='-02'
expressions[215]='-100/-02'
lefts[215]='-100'
rights[215]='-02'
expressions[216]='-(-100/-02)'
lefts[216]='-100'
rights[216]='-02'
expressions[217]='100.00/2'
lefts[217]='100.00'
rights[217]='2'
expressions[218]='-(100.00/2)'
lefts[218]='100.00'
rights[218]='2'
expressions[219]='-100.00/2'
lefts[219]='-100.00'
rights[219]='2'
expressions[220]='-(-100.00/2)'
lefts[220]='-100.00'
rights[220]='2'
expressions[221]='100.00/-2'
lefts[221]='100.00'
rights[221]='-2'
expressions[222]='-(100.00/-2)'
lefts[222]='100.00'
rights[222]='-2'
expressions[223]='-100.00/-2'
lefts[223]='-100.00'
rights[223]='-2'
expressions[224]='-(-100.00/-2)'
lefts[224]='-100.00'
rights[224]='-2'
expressions[225]='100/2.00'
lefts[225]='100'
rights[225]='2.00'
expressions[226]='-(100/2.00)'
lefts[226]='100'
rights[226]='2.00'
expressions[227]='-100/2.00'
lefts[227]='-100'
rights[227]='2.00'
expressions[228]='-(-100/2.00)'
lefts[228]='-100'
rights[228]='2.00'
expressions[229]='100/-2.00'
lefts[229]='100'
rights[229]='-2.00'
expressions[230]='-(100/-2.00)'
lefts[230]='100'
rights[230]='-2.00'
expressions[231]='-100/-2.00'
lefts[231]='-100'
rights[231]='-2.00'
expressions[232]='-(-100/-2.00)'
lefts[232]='-100'
rights[232]='-2.00'
expressions[233]='100.00/2.00'
lefts[233]='100.00'
rights[233]='2.00'
expressions[234]='-(100.00/2.00)'
lefts[234]='100.00'
rights[234]='2.00'
expressions[235]='-100.00/2.00'
lefts[235]='-100.00'
rights[235]='2.00'
expressions[236]='-(-100.00/2.00)'
lefts[236]='-100.00'
rights[236]='2.00'
expressions[237]='100.00/-2.00'
lefts[237]='100.00'
rights[237]='-2.00'
expressions[238]='-(100.00/-2.00)'
lefts[238]='100.00'
rights[238]='-2.00'
expressions[239]='-100.00/-2.00'
lefts[239]='-100.00'
rights[239]='-2.00'
expressions[240]='-(-100.00/-2.00)'
lefts[240]='-100.00'
rights[240]='-2.00'
expressions[241]='100.000000/2.00'
lefts[241]='100.000000'
rights[241]='2.00'
expressions[242]='-(100.000000/2.00)'
lefts[242]='100.000000'
rights[242]='2.00'
expressions[243]='-100.000000/2.00'
lefts[243]='-100.000000'
rights[243]='2.00'
expressions[244]='-(-100.000000/2.00)'
lefts[244]='-100.000000'
rights[244]='2.00'
expressions[245]='100.000000/-2.00'
lefts[245]='100.000000'
rights[245]='-2.00'
expressions[246]='-(100.000000/-2.00)'
lefts[246]='100.000000'
rights[246]='-2.00'
expressions[247]='-100.000000/-2.00'
lefts[247]='-100.000000'
rights[247]='-2.00'
expressions[248]='-(-100.000000/-2.00)'
lefts[248]='-100.000000'
rights[248]='-2.00'
expressions[249]='100.000000000000000000/2.000000000000000000'
lefts[249]='100.000000000000000000'
rights[249]='2.000000000000000000'
expressions[250]='-(100.000000000000000000/2.000000000000000000)'
lefts[250]='100.000000000000000000'
rights[250]='2.000000000000000000'
expressions[251]='-100.000000000000000000/2.000000000000000000'
lefts[251]='-100.000000000000000000'
rights[251]='2.000000000000000000'
expressions[252]='-(-100.000000000000000000/2.000000000000000000)'
lefts[252]='-100.000000000000000000'
rights[252]='2.000000000000000000'
expressions[253]='100.000000000000000000/-2.000000000000000000'
lefts[253]='100.000000000000000000'
rights[253]='-2.000000000000000000'
expressions[254]='-(100.000000000000000000/-2.000000000000000000)'
lefts[254]='100.000000000000000000'
rights[254]='-2.000000000000000000'
expressions[255]='-100.000000000000000000/-2.000000000000000000'
lefts[255]='-100.000000000000000000'
rights[255]='-2.000000000000000000'
expressions[256]='-(-100.000000000000000000/-2.000000000000000000)'
lefts[256]='-100.000000000000000000'
rights[256]='-2.000000000000000000'
expressions[257]='200/15'
lefts[257]='200'
rights[257]='15'
expressions[258]='-(200/15)'
lefts[258]='200'
rights[258]='15'
expressions[259]='-200/15'
lefts[259]='-200'
rights[259]='15'
expressions[260]='-(-200/15)'
lefts[260]='-200'
rights[260]='15'
expressions[261]='200/-15'
lefts[261]='200'
rights[261]='-15'
expressions[262]='-(200/-15)'
lefts[262]='200'
rights[262]='-15'
expressions[263]='-200/-15'
lefts[263]='-200'
rights[263]='-15'
expressions[264]='-(-200/-15)'
lefts[264]='-200'
rights[264]='-15'
expressions[265]='0200/15'
lefts[265]='0200'
rights[265]='15'
expressions[266]='-(0200/15)'
lefts[266]='0200'
rights[266]='15'
expressions[267]='-0200/15'
lefts[267]='-0200'
rights[267]='15'
expressions[268]='-(-0200/15)'
lefts[268]='-0200'
rights[268]='15'
expressions[269]='0200/-15'
lefts[269]='0200'
rights[269]='-15'
expressions[270]='-(0200/-15)'
lefts[270]='0200'
rights[270]='-15'
expressions[271]='-0200/-15'
lefts[271]='-0200'
rights[271]='-15'
expressions[272]='-(-0200/-15)'
lefts[272]='-0200'
rights[272]='-15'
expressions[273]='200/015'
lefts[273]='200'
rights[273]='015'
expressions[274]='-(200/015)'
lefts[274]='200'
rights[274]='015'
expressions[275]='-200/015'
lefts[275]='-200'
rights[275]='015'
expressions[276]='-(-200/015)'
lefts[276]='-200'
rights[276]='015'
expressions[277]='200/-015'
lefts[277]='200'
rights[277]='-015'
expressions[278]='-(200/-015)'
lefts[278]='200'
rights[278]='-015'
expressions[279]='-200/-015'
lefts[279]='-200'
rights[279]='-015'
expressions[280]='-(-200/-015)'
lefts[280]='-200'
rights[280]='-015'
expressions[281]='200.00/15'
lefts[281]='200.00'
rights[281]='15'
expressions[282]='-(200.00/15)'
lefts[282]='200.00'
rights[282]='15'
expressions[283]='-200.00/15'
lefts[283]='-200.00'
rights[283]='15'
expressions[284]='-(-200.00/15)'
lefts[284]='-200.00'
rights[284]='15'
expressions[285]='200.00/-15'
lefts[285]='200.00'
rights[285]='-15'
expressions[286]='-(200.00/-15)'
lefts[286]='200.00'
rights[286]='-15'
expressions[287]='-200.00/-15'
lefts[287]='-200.00'
rights[287]='-15'
expressions[288]='-(-200.00/-15)'
lefts[288]='-200.00'
rights[288]='-15'
expressions[289]='200/15.00'
lefts[289]='200'
rights[289]='15.00'
expressions[290]='-(200/15.00)'
lefts[290]='200'
rights[290]='15.00'
expressions[291]='-200/15.00'
lefts[291]='-200'
rights[291]='15.00'
expressions[292]='-(-200/15.00)'
lefts[292]='-200'
rights[292]='15.00'
expressions[293]='200/-15.00'
lefts[293]='200'
rights[293]='-15.00'
expressions[294]='-(200/-15.00)'
lefts[294]='200'
rights[294]='-15.00'
expressions[295]='-200/-15.00'
lefts[295]='-200'
rights[295]='-15.00'
expressions[296]='-(-200/-15.00)'
lefts[296]='-200'
rights[296]='-15.00'
expressions[297]='200.00/15.00'
lefts[297]='200.00'
rights[297]='15.00'
expressions[298]='-(200.00/15.00)'
lefts[298]='200.00'
rights[298]='15.00'
expressions[299]='-200.00/15.00'
lefts[299]='-200.00'
rights[299]='15.00'
expressions[300]='-(-200.00/15.00)'
lefts[300]='-200.00'
rights[300]='15.00'
expressions[301]='200.00/-15.00'
lefts[301]='200.00'
rights[301]='-15.00'
expressions[302]='-(200.00/-15.00)'
lefts[302]='200.00'
rights[302]='-15.00'
expressions[303]='-200.00/-15.00'
lefts[303]='-200.00'
rights[303]='-15.00'
expressions[304]='-(-200.00/-15.00)'
lefts[304]='-200.00'
rights[304]='-15.00'
expressions[305]='200.000000/15.00'
lefts[305]='200.000000'
rights[305]='15.00'
expressions[306]='-(200.000000/15.00)'
lefts[306]='200.000000'
rights[306]='15.00'
expressions[307]='-200.000000/15.00'
lefts[307]='-200.000000'
rights[307]='15.00'
expressions[308]='-(-200.000000/15.00)'
lefts[308]='-200.000000'
rights[308]='15.00'
expressions[309]='200.000000/-15.00'
lefts[309]='200.000000'
rights[309]='-15.00'
expressions[310]='-(200.000000/-15.00)'
lefts[310]='200.000000'
rights[310]='-15.00'
expressions[311]='-200.000000/-15.00'
lefts[311]='-200.000000'
rights[311]='-15.00'
expressions[312]='-(-200.000000/-15.00)'
lefts[312]='-200.000000'
rights[312]='-15.00'
expressions[313]='200.000000000000000000/15.000000000000000000'
lefts[313]='200.000000000000000000'
rights[313]='15.000000000000000000'
expressions[314]='-(200.000000000000000000/15.000000000000000000)'
lefts[314]='200.000000000000000000'
rights[314]='15.000000000000000000'
expressions[315]='-200.000000000000000000/15.000000000000000000'
lefts[315]='-200.000000000000000000'
rights[315]='15.000000000000000000'
expressions[316]='-(-200.000000000000000000/15.000000000000000000)'
lefts[316]='-200.000000000000000000'
rights[316]='15.000000000000000000'
expressions[317]='200.000000000000000000/-15.000000000000000000'
lefts[317]='200.000000000000000000'
rights[317]='-15.000000000000000000'
expressions[318]='-(200.000000000000000000/-15.000000000000000000)'
lefts[318]='200.000000000000000000'
rights[318]='-15.000000000000000000'
expressions[319]='-200.000000000000000000/-15.000000000000000000'
lefts[319]='-200.000000000000000000'
rights[319]='-15.000000000000000000'
expressions[320]='-(-200.000000000000000000/-15.000000000000000000)'
lefts[320]='-200.000000000000000000'
rights[320]='-15.000000000000000000'
settings[1]=0
settings[2]=2
settings[3]=6
settings[4]=14
settings[5]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught
outputPath='Z:\home\rich\.codex\worktrees\numeric-constant-division-5611\Project-Copperfin\build\constant-division-native\constant-payload-a-results.txt'
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
   TRY
    observation=CFNMETA(EVALUATE(expression))+'|'+HexBytes(BINTOC(EVALUATE(expression),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE(expression)))+'|'+CFNMETA(EVALUATE(lefts[index]))+'|'+CFNMETA(EVALUATE(rights[index]))
    line='EVAL|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
   CATCH TO caught
    line='ERROR-EVAL|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
   ENDTRY
   =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   total=total+1
   TRY
    observation=CompiledObservation(index)
    line='COMPILED|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
   CATCH TO caught
    line='ERROR-COMPILED|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
   ENDTRY
   =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   total=total+1
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION CompiledObservation
 LPARAMETERS index
 DO CASE
 CASE index=1
  RETURN CFNMETA(0/2)+'|'+HexBytes(BINTOC(0/2,'BRS'))+'|'+HexBytes(TRANSFORM(0/2))+'|'+CFNMETA(0)+'|'+CFNMETA(2)
 CASE index=2
  RETURN CFNMETA(-(0/2))+'|'+HexBytes(BINTOC(-(0/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(0/2)))+'|'+CFNMETA(0)+'|'+CFNMETA(2)
 CASE index=3
  RETURN CFNMETA(-0/2)+'|'+HexBytes(BINTOC(-0/2,'BRS'))+'|'+HexBytes(TRANSFORM(-0/2))+'|'+CFNMETA(-0)+'|'+CFNMETA(2)
 CASE index=4
  RETURN CFNMETA(-(-0/2))+'|'+HexBytes(BINTOC(-(-0/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0/2)))+'|'+CFNMETA(-0)+'|'+CFNMETA(2)
 CASE index=5
  RETURN CFNMETA(0/-2)+'|'+HexBytes(BINTOC(0/-2,'BRS'))+'|'+HexBytes(TRANSFORM(0/-2))+'|'+CFNMETA(0)+'|'+CFNMETA(-2)
 CASE index=6
  RETURN CFNMETA(-(0/-2))+'|'+HexBytes(BINTOC(-(0/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(0/-2)))+'|'+CFNMETA(0)+'|'+CFNMETA(-2)
 CASE index=7
  RETURN CFNMETA(-0/-2)+'|'+HexBytes(BINTOC(-0/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-0/-2))+'|'+CFNMETA(-0)+'|'+CFNMETA(-2)
 CASE index=8
  RETURN CFNMETA(-(-0/-2))+'|'+HexBytes(BINTOC(-(-0/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0/-2)))+'|'+CFNMETA(-0)+'|'+CFNMETA(-2)
 CASE index=9
  RETURN CFNMETA(00/2)+'|'+HexBytes(BINTOC(00/2,'BRS'))+'|'+HexBytes(TRANSFORM(00/2))+'|'+CFNMETA(00)+'|'+CFNMETA(2)
 CASE index=10
  RETURN CFNMETA(-(00/2))+'|'+HexBytes(BINTOC(-(00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(00/2)))+'|'+CFNMETA(00)+'|'+CFNMETA(2)
 CASE index=11
  RETURN CFNMETA(-00/2)+'|'+HexBytes(BINTOC(-00/2,'BRS'))+'|'+HexBytes(TRANSFORM(-00/2))+'|'+CFNMETA(-00)+'|'+CFNMETA(2)
 CASE index=12
  RETURN CFNMETA(-(-00/2))+'|'+HexBytes(BINTOC(-(-00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-00/2)))+'|'+CFNMETA(-00)+'|'+CFNMETA(2)
 CASE index=13
  RETURN CFNMETA(00/-2)+'|'+HexBytes(BINTOC(00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(00/-2))+'|'+CFNMETA(00)+'|'+CFNMETA(-2)
 CASE index=14
  RETURN CFNMETA(-(00/-2))+'|'+HexBytes(BINTOC(-(00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(00/-2)))+'|'+CFNMETA(00)+'|'+CFNMETA(-2)
 CASE index=15
  RETURN CFNMETA(-00/-2)+'|'+HexBytes(BINTOC(-00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-00/-2))+'|'+CFNMETA(-00)+'|'+CFNMETA(-2)
 CASE index=16
  RETURN CFNMETA(-(-00/-2))+'|'+HexBytes(BINTOC(-(-00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-00/-2)))+'|'+CFNMETA(-00)+'|'+CFNMETA(-2)
 CASE index=17
  RETURN CFNMETA(0/02)+'|'+HexBytes(BINTOC(0/02,'BRS'))+'|'+HexBytes(TRANSFORM(0/02))+'|'+CFNMETA(0)+'|'+CFNMETA(02)
 CASE index=18
  RETURN CFNMETA(-(0/02))+'|'+HexBytes(BINTOC(-(0/02),'BRS'))+'|'+HexBytes(TRANSFORM(-(0/02)))+'|'+CFNMETA(0)+'|'+CFNMETA(02)
 CASE index=19
  RETURN CFNMETA(-0/02)+'|'+HexBytes(BINTOC(-0/02,'BRS'))+'|'+HexBytes(TRANSFORM(-0/02))+'|'+CFNMETA(-0)+'|'+CFNMETA(02)
 CASE index=20
  RETURN CFNMETA(-(-0/02))+'|'+HexBytes(BINTOC(-(-0/02),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0/02)))+'|'+CFNMETA(-0)+'|'+CFNMETA(02)
 CASE index=21
  RETURN CFNMETA(0/-02)+'|'+HexBytes(BINTOC(0/-02,'BRS'))+'|'+HexBytes(TRANSFORM(0/-02))+'|'+CFNMETA(0)+'|'+CFNMETA(-02)
 CASE index=22
  RETURN CFNMETA(-(0/-02))+'|'+HexBytes(BINTOC(-(0/-02),'BRS'))+'|'+HexBytes(TRANSFORM(-(0/-02)))+'|'+CFNMETA(0)+'|'+CFNMETA(-02)
 CASE index=23
  RETURN CFNMETA(-0/-02)+'|'+HexBytes(BINTOC(-0/-02,'BRS'))+'|'+HexBytes(TRANSFORM(-0/-02))+'|'+CFNMETA(-0)+'|'+CFNMETA(-02)
 CASE index=24
  RETURN CFNMETA(-(-0/-02))+'|'+HexBytes(BINTOC(-(-0/-02),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0/-02)))+'|'+CFNMETA(-0)+'|'+CFNMETA(-02)
 CASE index=25
  RETURN CFNMETA(0.00/2)+'|'+HexBytes(BINTOC(0.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(0.00/2))+'|'+CFNMETA(0.00)+'|'+CFNMETA(2)
 CASE index=26
  RETURN CFNMETA(-(0.00/2))+'|'+HexBytes(BINTOC(-(0.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00/2)))+'|'+CFNMETA(0.00)+'|'+CFNMETA(2)
 CASE index=27
  RETURN CFNMETA(-0.00/2)+'|'+HexBytes(BINTOC(-0.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(-0.00/2))+'|'+CFNMETA(-0.00)+'|'+CFNMETA(2)
 CASE index=28
  RETURN CFNMETA(-(-0.00/2))+'|'+HexBytes(BINTOC(-(-0.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.00/2)))+'|'+CFNMETA(-0.00)+'|'+CFNMETA(2)
 CASE index=29
  RETURN CFNMETA(0.00/-2)+'|'+HexBytes(BINTOC(0.00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(0.00/-2))+'|'+CFNMETA(0.00)+'|'+CFNMETA(-2)
 CASE index=30
  RETURN CFNMETA(-(0.00/-2))+'|'+HexBytes(BINTOC(-(0.00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00/-2)))+'|'+CFNMETA(0.00)+'|'+CFNMETA(-2)
 CASE index=31
  RETURN CFNMETA(-0.00/-2)+'|'+HexBytes(BINTOC(-0.00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-0.00/-2))+'|'+CFNMETA(-0.00)+'|'+CFNMETA(-2)
 CASE index=32
  RETURN CFNMETA(-(-0.00/-2))+'|'+HexBytes(BINTOC(-(-0.00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.00/-2)))+'|'+CFNMETA(-0.00)+'|'+CFNMETA(-2)
 CASE index=33
  RETURN CFNMETA(0/2.00)+'|'+HexBytes(BINTOC(0/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(0/2.00))+'|'+CFNMETA(0)+'|'+CFNMETA(2.00)
 CASE index=34
  RETURN CFNMETA(-(0/2.00))+'|'+HexBytes(BINTOC(-(0/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0/2.00)))+'|'+CFNMETA(0)+'|'+CFNMETA(2.00)
 CASE index=35
  RETURN CFNMETA(-0/2.00)+'|'+HexBytes(BINTOC(-0/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-0/2.00))+'|'+CFNMETA(-0)+'|'+CFNMETA(2.00)
 CASE index=36
  RETURN CFNMETA(-(-0/2.00))+'|'+HexBytes(BINTOC(-(-0/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0/2.00)))+'|'+CFNMETA(-0)+'|'+CFNMETA(2.00)
 CASE index=37
  RETURN CFNMETA(0/-2.00)+'|'+HexBytes(BINTOC(0/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(0/-2.00))+'|'+CFNMETA(0)+'|'+CFNMETA(-2.00)
 CASE index=38
  RETURN CFNMETA(-(0/-2.00))+'|'+HexBytes(BINTOC(-(0/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0/-2.00)))+'|'+CFNMETA(0)+'|'+CFNMETA(-2.00)
 CASE index=39
  RETURN CFNMETA(-0/-2.00)+'|'+HexBytes(BINTOC(-0/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-0/-2.00))+'|'+CFNMETA(-0)+'|'+CFNMETA(-2.00)
 CASE index=40
  RETURN CFNMETA(-(-0/-2.00))+'|'+HexBytes(BINTOC(-(-0/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0/-2.00)))+'|'+CFNMETA(-0)+'|'+CFNMETA(-2.00)
 CASE index=41
  RETURN CFNMETA(0.00/2.00)+'|'+HexBytes(BINTOC(0.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(0.00/2.00))+'|'+CFNMETA(0.00)+'|'+CFNMETA(2.00)
 CASE index=42
  RETURN CFNMETA(-(0.00/2.00))+'|'+HexBytes(BINTOC(-(0.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00/2.00)))+'|'+CFNMETA(0.00)+'|'+CFNMETA(2.00)
 CASE index=43
  RETURN CFNMETA(-0.00/2.00)+'|'+HexBytes(BINTOC(-0.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-0.00/2.00))+'|'+CFNMETA(-0.00)+'|'+CFNMETA(2.00)
 CASE index=44
  RETURN CFNMETA(-(-0.00/2.00))+'|'+HexBytes(BINTOC(-(-0.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.00/2.00)))+'|'+CFNMETA(-0.00)+'|'+CFNMETA(2.00)
 CASE index=45
  RETURN CFNMETA(0.00/-2.00)+'|'+HexBytes(BINTOC(0.00/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(0.00/-2.00))+'|'+CFNMETA(0.00)+'|'+CFNMETA(-2.00)
 CASE index=46
  RETURN CFNMETA(-(0.00/-2.00))+'|'+HexBytes(BINTOC(-(0.00/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.00/-2.00)))+'|'+CFNMETA(0.00)+'|'+CFNMETA(-2.00)
 CASE index=47
  RETURN CFNMETA(-0.00/-2.00)+'|'+HexBytes(BINTOC(-0.00/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-0.00/-2.00))+'|'+CFNMETA(-0.00)+'|'+CFNMETA(-2.00)
 CASE index=48
  RETURN CFNMETA(-(-0.00/-2.00))+'|'+HexBytes(BINTOC(-(-0.00/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.00/-2.00)))+'|'+CFNMETA(-0.00)+'|'+CFNMETA(-2.00)
 CASE index=49
  RETURN CFNMETA(0.000000/2.00)+'|'+HexBytes(BINTOC(0.000000/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000/2.00))+'|'+CFNMETA(0.000000)+'|'+CFNMETA(2.00)
 CASE index=50
  RETURN CFNMETA(-(0.000000/2.00))+'|'+HexBytes(BINTOC(-(0.000000/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.000000/2.00)))+'|'+CFNMETA(0.000000)+'|'+CFNMETA(2.00)
 CASE index=51
  RETURN CFNMETA(-0.000000/2.00)+'|'+HexBytes(BINTOC(-0.000000/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-0.000000/2.00))+'|'+CFNMETA(-0.000000)+'|'+CFNMETA(2.00)
 CASE index=52
  RETURN CFNMETA(-(-0.000000/2.00))+'|'+HexBytes(BINTOC(-(-0.000000/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.000000/2.00)))+'|'+CFNMETA(-0.000000)+'|'+CFNMETA(2.00)
 CASE index=53
  RETURN CFNMETA(0.000000/-2.00)+'|'+HexBytes(BINTOC(0.000000/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000/-2.00))+'|'+CFNMETA(0.000000)+'|'+CFNMETA(-2.00)
 CASE index=54
  RETURN CFNMETA(-(0.000000/-2.00))+'|'+HexBytes(BINTOC(-(0.000000/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.000000/-2.00)))+'|'+CFNMETA(0.000000)+'|'+CFNMETA(-2.00)
 CASE index=55
  RETURN CFNMETA(-0.000000/-2.00)+'|'+HexBytes(BINTOC(-0.000000/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-0.000000/-2.00))+'|'+CFNMETA(-0.000000)+'|'+CFNMETA(-2.00)
 CASE index=56
  RETURN CFNMETA(-(-0.000000/-2.00))+'|'+HexBytes(BINTOC(-(-0.000000/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.000000/-2.00)))+'|'+CFNMETA(-0.000000)+'|'+CFNMETA(-2.00)
 CASE index=57
  RETURN CFNMETA(0.000000000000000000/2.000000000000000000)+'|'+HexBytes(BINTOC(0.000000000000000000/2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000000000000000/2.000000000000000000))+'|'+CFNMETA(0.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=58
  RETURN CFNMETA(-(0.000000000000000000/2.000000000000000000))+'|'+HexBytes(BINTOC(-(0.000000000000000000/2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.000000000000000000/2.000000000000000000)))+'|'+CFNMETA(0.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=59
  RETURN CFNMETA(-0.000000000000000000/2.000000000000000000)+'|'+HexBytes(BINTOC(-0.000000000000000000/2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-0.000000000000000000/2.000000000000000000))+'|'+CFNMETA(-0.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=60
  RETURN CFNMETA(-(-0.000000000000000000/2.000000000000000000))+'|'+HexBytes(BINTOC(-(-0.000000000000000000/2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.000000000000000000/2.000000000000000000)))+'|'+CFNMETA(-0.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=61
  RETURN CFNMETA(0.000000000000000000/-2.000000000000000000)+'|'+HexBytes(BINTOC(0.000000000000000000/-2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000000000000000/-2.000000000000000000))+'|'+CFNMETA(0.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=62
  RETURN CFNMETA(-(0.000000000000000000/-2.000000000000000000))+'|'+HexBytes(BINTOC(-(0.000000000000000000/-2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(0.000000000000000000/-2.000000000000000000)))+'|'+CFNMETA(0.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=63
  RETURN CFNMETA(-0.000000000000000000/-2.000000000000000000)+'|'+HexBytes(BINTOC(-0.000000000000000000/-2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-0.000000000000000000/-2.000000000000000000))+'|'+CFNMETA(-0.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=64
  RETURN CFNMETA(-(-0.000000000000000000/-2.000000000000000000))+'|'+HexBytes(BINTOC(-(-0.000000000000000000/-2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0.000000000000000000/-2.000000000000000000)))+'|'+CFNMETA(-0.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=65
  RETURN CFNMETA(1/7)+'|'+HexBytes(BINTOC(1/7,'BRS'))+'|'+HexBytes(TRANSFORM(1/7))+'|'+CFNMETA(1)+'|'+CFNMETA(7)
 CASE index=66
  RETURN CFNMETA(-(1/7))+'|'+HexBytes(BINTOC(-(1/7),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/7)))+'|'+CFNMETA(1)+'|'+CFNMETA(7)
 CASE index=67
  RETURN CFNMETA(-1/7)+'|'+HexBytes(BINTOC(-1/7,'BRS'))+'|'+HexBytes(TRANSFORM(-1/7))+'|'+CFNMETA(-1)+'|'+CFNMETA(7)
 CASE index=68
  RETURN CFNMETA(-(-1/7))+'|'+HexBytes(BINTOC(-(-1/7),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/7)))+'|'+CFNMETA(-1)+'|'+CFNMETA(7)
 CASE index=69
  RETURN CFNMETA(1/-7)+'|'+HexBytes(BINTOC(1/-7,'BRS'))+'|'+HexBytes(TRANSFORM(1/-7))+'|'+CFNMETA(1)+'|'+CFNMETA(-7)
 CASE index=70
  RETURN CFNMETA(-(1/-7))+'|'+HexBytes(BINTOC(-(1/-7),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/-7)))+'|'+CFNMETA(1)+'|'+CFNMETA(-7)
 CASE index=71
  RETURN CFNMETA(-1/-7)+'|'+HexBytes(BINTOC(-1/-7,'BRS'))+'|'+HexBytes(TRANSFORM(-1/-7))+'|'+CFNMETA(-1)+'|'+CFNMETA(-7)
 CASE index=72
  RETURN CFNMETA(-(-1/-7))+'|'+HexBytes(BINTOC(-(-1/-7),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/-7)))+'|'+CFNMETA(-1)+'|'+CFNMETA(-7)
 CASE index=73
  RETURN CFNMETA(01/7)+'|'+HexBytes(BINTOC(01/7,'BRS'))+'|'+HexBytes(TRANSFORM(01/7))+'|'+CFNMETA(01)+'|'+CFNMETA(7)
 CASE index=74
  RETURN CFNMETA(-(01/7))+'|'+HexBytes(BINTOC(-(01/7),'BRS'))+'|'+HexBytes(TRANSFORM(-(01/7)))+'|'+CFNMETA(01)+'|'+CFNMETA(7)
 CASE index=75
  RETURN CFNMETA(-01/7)+'|'+HexBytes(BINTOC(-01/7,'BRS'))+'|'+HexBytes(TRANSFORM(-01/7))+'|'+CFNMETA(-01)+'|'+CFNMETA(7)
 CASE index=76
  RETURN CFNMETA(-(-01/7))+'|'+HexBytes(BINTOC(-(-01/7),'BRS'))+'|'+HexBytes(TRANSFORM(-(-01/7)))+'|'+CFNMETA(-01)+'|'+CFNMETA(7)
 CASE index=77
  RETURN CFNMETA(01/-7)+'|'+HexBytes(BINTOC(01/-7,'BRS'))+'|'+HexBytes(TRANSFORM(01/-7))+'|'+CFNMETA(01)+'|'+CFNMETA(-7)
 CASE index=78
  RETURN CFNMETA(-(01/-7))+'|'+HexBytes(BINTOC(-(01/-7),'BRS'))+'|'+HexBytes(TRANSFORM(-(01/-7)))+'|'+CFNMETA(01)+'|'+CFNMETA(-7)
 CASE index=79
  RETURN CFNMETA(-01/-7)+'|'+HexBytes(BINTOC(-01/-7,'BRS'))+'|'+HexBytes(TRANSFORM(-01/-7))+'|'+CFNMETA(-01)+'|'+CFNMETA(-7)
 CASE index=80
  RETURN CFNMETA(-(-01/-7))+'|'+HexBytes(BINTOC(-(-01/-7),'BRS'))+'|'+HexBytes(TRANSFORM(-(-01/-7)))+'|'+CFNMETA(-01)+'|'+CFNMETA(-7)
 CASE index=81
  RETURN CFNMETA(1/07)+'|'+HexBytes(BINTOC(1/07,'BRS'))+'|'+HexBytes(TRANSFORM(1/07))+'|'+CFNMETA(1)+'|'+CFNMETA(07)
 CASE index=82
  RETURN CFNMETA(-(1/07))+'|'+HexBytes(BINTOC(-(1/07),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/07)))+'|'+CFNMETA(1)+'|'+CFNMETA(07)
 CASE index=83
  RETURN CFNMETA(-1/07)+'|'+HexBytes(BINTOC(-1/07,'BRS'))+'|'+HexBytes(TRANSFORM(-1/07))+'|'+CFNMETA(-1)+'|'+CFNMETA(07)
 CASE index=84
  RETURN CFNMETA(-(-1/07))+'|'+HexBytes(BINTOC(-(-1/07),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/07)))+'|'+CFNMETA(-1)+'|'+CFNMETA(07)
 CASE index=85
  RETURN CFNMETA(1/-07)+'|'+HexBytes(BINTOC(1/-07,'BRS'))+'|'+HexBytes(TRANSFORM(1/-07))+'|'+CFNMETA(1)+'|'+CFNMETA(-07)
 CASE index=86
  RETURN CFNMETA(-(1/-07))+'|'+HexBytes(BINTOC(-(1/-07),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/-07)))+'|'+CFNMETA(1)+'|'+CFNMETA(-07)
 CASE index=87
  RETURN CFNMETA(-1/-07)+'|'+HexBytes(BINTOC(-1/-07,'BRS'))+'|'+HexBytes(TRANSFORM(-1/-07))+'|'+CFNMETA(-1)+'|'+CFNMETA(-07)
 CASE index=88
  RETURN CFNMETA(-(-1/-07))+'|'+HexBytes(BINTOC(-(-1/-07),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/-07)))+'|'+CFNMETA(-1)+'|'+CFNMETA(-07)
 CASE index=89
  RETURN CFNMETA(1.00/7)+'|'+HexBytes(BINTOC(1.00/7,'BRS'))+'|'+HexBytes(TRANSFORM(1.00/7))+'|'+CFNMETA(1.00)+'|'+CFNMETA(7)
 CASE index=90
  RETURN CFNMETA(-(1.00/7))+'|'+HexBytes(BINTOC(-(1.00/7),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00/7)))+'|'+CFNMETA(1.00)+'|'+CFNMETA(7)
 CASE index=91
  RETURN CFNMETA(-1.00/7)+'|'+HexBytes(BINTOC(-1.00/7,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00/7))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(7)
 CASE index=92
  RETURN CFNMETA(-(-1.00/7))+'|'+HexBytes(BINTOC(-(-1.00/7),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.00/7)))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(7)
 CASE index=93
  RETURN CFNMETA(1.00/-7)+'|'+HexBytes(BINTOC(1.00/-7,'BRS'))+'|'+HexBytes(TRANSFORM(1.00/-7))+'|'+CFNMETA(1.00)+'|'+CFNMETA(-7)
 CASE index=94
  RETURN CFNMETA(-(1.00/-7))+'|'+HexBytes(BINTOC(-(1.00/-7),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00/-7)))+'|'+CFNMETA(1.00)+'|'+CFNMETA(-7)
 CASE index=95
  RETURN CFNMETA(-1.00/-7)+'|'+HexBytes(BINTOC(-1.00/-7,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00/-7))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(-7)
 CASE index=96
  RETURN CFNMETA(-(-1.00/-7))+'|'+HexBytes(BINTOC(-(-1.00/-7),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.00/-7)))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(-7)
 CASE index=97
  RETURN CFNMETA(1/7.00)+'|'+HexBytes(BINTOC(1/7.00,'BRS'))+'|'+HexBytes(TRANSFORM(1/7.00))+'|'+CFNMETA(1)+'|'+CFNMETA(7.00)
 CASE index=98
  RETURN CFNMETA(-(1/7.00))+'|'+HexBytes(BINTOC(-(1/7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/7.00)))+'|'+CFNMETA(1)+'|'+CFNMETA(7.00)
 CASE index=99
  RETURN CFNMETA(-1/7.00)+'|'+HexBytes(BINTOC(-1/7.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1/7.00))+'|'+CFNMETA(-1)+'|'+CFNMETA(7.00)
 CASE index=100
  RETURN CFNMETA(-(-1/7.00))+'|'+HexBytes(BINTOC(-(-1/7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/7.00)))+'|'+CFNMETA(-1)+'|'+CFNMETA(7.00)
 CASE index=101
  RETURN CFNMETA(1/-7.00)+'|'+HexBytes(BINTOC(1/-7.00,'BRS'))+'|'+HexBytes(TRANSFORM(1/-7.00))+'|'+CFNMETA(1)+'|'+CFNMETA(-7.00)
 CASE index=102
  RETURN CFNMETA(-(1/-7.00))+'|'+HexBytes(BINTOC(-(1/-7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/-7.00)))+'|'+CFNMETA(1)+'|'+CFNMETA(-7.00)
 CASE index=103
  RETURN CFNMETA(-1/-7.00)+'|'+HexBytes(BINTOC(-1/-7.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1/-7.00))+'|'+CFNMETA(-1)+'|'+CFNMETA(-7.00)
 CASE index=104
  RETURN CFNMETA(-(-1/-7.00))+'|'+HexBytes(BINTOC(-(-1/-7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/-7.00)))+'|'+CFNMETA(-1)+'|'+CFNMETA(-7.00)
 CASE index=105
  RETURN CFNMETA(1.00/7.00)+'|'+HexBytes(BINTOC(1.00/7.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.00/7.00))+'|'+CFNMETA(1.00)+'|'+CFNMETA(7.00)
 CASE index=106
  RETURN CFNMETA(-(1.00/7.00))+'|'+HexBytes(BINTOC(-(1.00/7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00/7.00)))+'|'+CFNMETA(1.00)+'|'+CFNMETA(7.00)
 CASE index=107
  RETURN CFNMETA(-1.00/7.00)+'|'+HexBytes(BINTOC(-1.00/7.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00/7.00))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(7.00)
 CASE index=108
  RETURN CFNMETA(-(-1.00/7.00))+'|'+HexBytes(BINTOC(-(-1.00/7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.00/7.00)))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(7.00)
 CASE index=109
  RETURN CFNMETA(1.00/-7.00)+'|'+HexBytes(BINTOC(1.00/-7.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.00/-7.00))+'|'+CFNMETA(1.00)+'|'+CFNMETA(-7.00)
 CASE index=110
  RETURN CFNMETA(-(1.00/-7.00))+'|'+HexBytes(BINTOC(-(1.00/-7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00/-7.00)))+'|'+CFNMETA(1.00)+'|'+CFNMETA(-7.00)
 CASE index=111
  RETURN CFNMETA(-1.00/-7.00)+'|'+HexBytes(BINTOC(-1.00/-7.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00/-7.00))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(-7.00)
 CASE index=112
  RETURN CFNMETA(-(-1.00/-7.00))+'|'+HexBytes(BINTOC(-(-1.00/-7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.00/-7.00)))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(-7.00)
 CASE index=113
  RETURN CFNMETA(1.000000/7.00)+'|'+HexBytes(BINTOC(1.000000/7.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000/7.00))+'|'+CFNMETA(1.000000)+'|'+CFNMETA(7.00)
 CASE index=114
  RETURN CFNMETA(-(1.000000/7.00))+'|'+HexBytes(BINTOC(-(1.000000/7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.000000/7.00)))+'|'+CFNMETA(1.000000)+'|'+CFNMETA(7.00)
 CASE index=115
  RETURN CFNMETA(-1.000000/7.00)+'|'+HexBytes(BINTOC(-1.000000/7.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000/7.00))+'|'+CFNMETA(-1.000000)+'|'+CFNMETA(7.00)
 CASE index=116
  RETURN CFNMETA(-(-1.000000/7.00))+'|'+HexBytes(BINTOC(-(-1.000000/7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.000000/7.00)))+'|'+CFNMETA(-1.000000)+'|'+CFNMETA(7.00)
 CASE index=117
  RETURN CFNMETA(1.000000/-7.00)+'|'+HexBytes(BINTOC(1.000000/-7.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000/-7.00))+'|'+CFNMETA(1.000000)+'|'+CFNMETA(-7.00)
 CASE index=118
  RETURN CFNMETA(-(1.000000/-7.00))+'|'+HexBytes(BINTOC(-(1.000000/-7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.000000/-7.00)))+'|'+CFNMETA(1.000000)+'|'+CFNMETA(-7.00)
 CASE index=119
  RETURN CFNMETA(-1.000000/-7.00)+'|'+HexBytes(BINTOC(-1.000000/-7.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000/-7.00))+'|'+CFNMETA(-1.000000)+'|'+CFNMETA(-7.00)
 CASE index=120
  RETURN CFNMETA(-(-1.000000/-7.00))+'|'+HexBytes(BINTOC(-(-1.000000/-7.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.000000/-7.00)))+'|'+CFNMETA(-1.000000)+'|'+CFNMETA(-7.00)
 CASE index=121
  RETURN CFNMETA(1.000000000000000000/7.000000000000000000)+'|'+HexBytes(BINTOC(1.000000000000000000/7.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000000000000000/7.000000000000000000))+'|'+CFNMETA(1.000000000000000000)+'|'+CFNMETA(7.000000000000000000)
 CASE index=122
  RETURN CFNMETA(-(1.000000000000000000/7.000000000000000000))+'|'+HexBytes(BINTOC(-(1.000000000000000000/7.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.000000000000000000/7.000000000000000000)))+'|'+CFNMETA(1.000000000000000000)+'|'+CFNMETA(7.000000000000000000)
 CASE index=123
  RETURN CFNMETA(-1.000000000000000000/7.000000000000000000)+'|'+HexBytes(BINTOC(-1.000000000000000000/7.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000000000000000/7.000000000000000000))+'|'+CFNMETA(-1.000000000000000000)+'|'+CFNMETA(7.000000000000000000)
 CASE index=124
  RETURN CFNMETA(-(-1.000000000000000000/7.000000000000000000))+'|'+HexBytes(BINTOC(-(-1.000000000000000000/7.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.000000000000000000/7.000000000000000000)))+'|'+CFNMETA(-1.000000000000000000)+'|'+CFNMETA(7.000000000000000000)
 CASE index=125
  RETURN CFNMETA(1.000000000000000000/-7.000000000000000000)+'|'+HexBytes(BINTOC(1.000000000000000000/-7.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000000000000000/-7.000000000000000000))+'|'+CFNMETA(1.000000000000000000)+'|'+CFNMETA(-7.000000000000000000)
 CASE index=126
  RETURN CFNMETA(-(1.000000000000000000/-7.000000000000000000))+'|'+HexBytes(BINTOC(-(1.000000000000000000/-7.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.000000000000000000/-7.000000000000000000)))+'|'+CFNMETA(1.000000000000000000)+'|'+CFNMETA(-7.000000000000000000)
 CASE index=127
  RETURN CFNMETA(-1.000000000000000000/-7.000000000000000000)+'|'+HexBytes(BINTOC(-1.000000000000000000/-7.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000000000000000/-7.000000000000000000))+'|'+CFNMETA(-1.000000000000000000)+'|'+CFNMETA(-7.000000000000000000)
 CASE index=128
  RETURN CFNMETA(-(-1.000000000000000000/-7.000000000000000000))+'|'+HexBytes(BINTOC(-(-1.000000000000000000/-7.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.000000000000000000/-7.000000000000000000)))+'|'+CFNMETA(-1.000000000000000000)+'|'+CFNMETA(-7.000000000000000000)
 CASE index=129
  RETURN CFNMETA(10/2)+'|'+HexBytes(BINTOC(10/2,'BRS'))+'|'+HexBytes(TRANSFORM(10/2))+'|'+CFNMETA(10)+'|'+CFNMETA(2)
 CASE index=130
  RETURN CFNMETA(-(10/2))+'|'+HexBytes(BINTOC(-(10/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(10/2)))+'|'+CFNMETA(10)+'|'+CFNMETA(2)
 CASE index=131
  RETURN CFNMETA(-10/2)+'|'+HexBytes(BINTOC(-10/2,'BRS'))+'|'+HexBytes(TRANSFORM(-10/2))+'|'+CFNMETA(-10)+'|'+CFNMETA(2)
 CASE index=132
  RETURN CFNMETA(-(-10/2))+'|'+HexBytes(BINTOC(-(-10/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10/2)))+'|'+CFNMETA(-10)+'|'+CFNMETA(2)
 CASE index=133
  RETURN CFNMETA(10/-2)+'|'+HexBytes(BINTOC(10/-2,'BRS'))+'|'+HexBytes(TRANSFORM(10/-2))+'|'+CFNMETA(10)+'|'+CFNMETA(-2)
 CASE index=134
  RETURN CFNMETA(-(10/-2))+'|'+HexBytes(BINTOC(-(10/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(10/-2)))+'|'+CFNMETA(10)+'|'+CFNMETA(-2)
 CASE index=135
  RETURN CFNMETA(-10/-2)+'|'+HexBytes(BINTOC(-10/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-10/-2))+'|'+CFNMETA(-10)+'|'+CFNMETA(-2)
 CASE index=136
  RETURN CFNMETA(-(-10/-2))+'|'+HexBytes(BINTOC(-(-10/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10/-2)))+'|'+CFNMETA(-10)+'|'+CFNMETA(-2)
 CASE index=137
  RETURN CFNMETA(010/2)+'|'+HexBytes(BINTOC(010/2,'BRS'))+'|'+HexBytes(TRANSFORM(010/2))+'|'+CFNMETA(010)+'|'+CFNMETA(2)
 CASE index=138
  RETURN CFNMETA(-(010/2))+'|'+HexBytes(BINTOC(-(010/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(010/2)))+'|'+CFNMETA(010)+'|'+CFNMETA(2)
 CASE index=139
  RETURN CFNMETA(-010/2)+'|'+HexBytes(BINTOC(-010/2,'BRS'))+'|'+HexBytes(TRANSFORM(-010/2))+'|'+CFNMETA(-010)+'|'+CFNMETA(2)
 CASE index=140
  RETURN CFNMETA(-(-010/2))+'|'+HexBytes(BINTOC(-(-010/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-010/2)))+'|'+CFNMETA(-010)+'|'+CFNMETA(2)
 CASE index=141
  RETURN CFNMETA(010/-2)+'|'+HexBytes(BINTOC(010/-2,'BRS'))+'|'+HexBytes(TRANSFORM(010/-2))+'|'+CFNMETA(010)+'|'+CFNMETA(-2)
 CASE index=142
  RETURN CFNMETA(-(010/-2))+'|'+HexBytes(BINTOC(-(010/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(010/-2)))+'|'+CFNMETA(010)+'|'+CFNMETA(-2)
 CASE index=143
  RETURN CFNMETA(-010/-2)+'|'+HexBytes(BINTOC(-010/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-010/-2))+'|'+CFNMETA(-010)+'|'+CFNMETA(-2)
 CASE index=144
  RETURN CFNMETA(-(-010/-2))+'|'+HexBytes(BINTOC(-(-010/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-010/-2)))+'|'+CFNMETA(-010)+'|'+CFNMETA(-2)
 CASE index=145
  RETURN CFNMETA(10/02)+'|'+HexBytes(BINTOC(10/02,'BRS'))+'|'+HexBytes(TRANSFORM(10/02))+'|'+CFNMETA(10)+'|'+CFNMETA(02)
 CASE index=146
  RETURN CFNMETA(-(10/02))+'|'+HexBytes(BINTOC(-(10/02),'BRS'))+'|'+HexBytes(TRANSFORM(-(10/02)))+'|'+CFNMETA(10)+'|'+CFNMETA(02)
 CASE index=147
  RETURN CFNMETA(-10/02)+'|'+HexBytes(BINTOC(-10/02,'BRS'))+'|'+HexBytes(TRANSFORM(-10/02))+'|'+CFNMETA(-10)+'|'+CFNMETA(02)
 CASE index=148
  RETURN CFNMETA(-(-10/02))+'|'+HexBytes(BINTOC(-(-10/02),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10/02)))+'|'+CFNMETA(-10)+'|'+CFNMETA(02)
 CASE index=149
  RETURN CFNMETA(10/-02)+'|'+HexBytes(BINTOC(10/-02,'BRS'))+'|'+HexBytes(TRANSFORM(10/-02))+'|'+CFNMETA(10)+'|'+CFNMETA(-02)
 CASE index=150
  RETURN CFNMETA(-(10/-02))+'|'+HexBytes(BINTOC(-(10/-02),'BRS'))+'|'+HexBytes(TRANSFORM(-(10/-02)))+'|'+CFNMETA(10)+'|'+CFNMETA(-02)
 CASE index=151
  RETURN CFNMETA(-10/-02)+'|'+HexBytes(BINTOC(-10/-02,'BRS'))+'|'+HexBytes(TRANSFORM(-10/-02))+'|'+CFNMETA(-10)+'|'+CFNMETA(-02)
 CASE index=152
  RETURN CFNMETA(-(-10/-02))+'|'+HexBytes(BINTOC(-(-10/-02),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10/-02)))+'|'+CFNMETA(-10)+'|'+CFNMETA(-02)
 CASE index=153
  RETURN CFNMETA(10.00/2)+'|'+HexBytes(BINTOC(10.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(10.00/2))+'|'+CFNMETA(10.00)+'|'+CFNMETA(2)
 CASE index=154
  RETURN CFNMETA(-(10.00/2))+'|'+HexBytes(BINTOC(-(10.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(10.00/2)))+'|'+CFNMETA(10.00)+'|'+CFNMETA(2)
 CASE index=155
  RETURN CFNMETA(-10.00/2)+'|'+HexBytes(BINTOC(-10.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(-10.00/2))+'|'+CFNMETA(-10.00)+'|'+CFNMETA(2)
 CASE index=156
  RETURN CFNMETA(-(-10.00/2))+'|'+HexBytes(BINTOC(-(-10.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10.00/2)))+'|'+CFNMETA(-10.00)+'|'+CFNMETA(2)
 CASE index=157
  RETURN CFNMETA(10.00/-2)+'|'+HexBytes(BINTOC(10.00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(10.00/-2))+'|'+CFNMETA(10.00)+'|'+CFNMETA(-2)
 CASE index=158
  RETURN CFNMETA(-(10.00/-2))+'|'+HexBytes(BINTOC(-(10.00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(10.00/-2)))+'|'+CFNMETA(10.00)+'|'+CFNMETA(-2)
 CASE index=159
  RETURN CFNMETA(-10.00/-2)+'|'+HexBytes(BINTOC(-10.00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-10.00/-2))+'|'+CFNMETA(-10.00)+'|'+CFNMETA(-2)
 CASE index=160
  RETURN CFNMETA(-(-10.00/-2))+'|'+HexBytes(BINTOC(-(-10.00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10.00/-2)))+'|'+CFNMETA(-10.00)+'|'+CFNMETA(-2)
 CASE index=161
  RETURN CFNMETA(10/2.00)+'|'+HexBytes(BINTOC(10/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(10/2.00))+'|'+CFNMETA(10)+'|'+CFNMETA(2.00)
 CASE index=162
  RETURN CFNMETA(-(10/2.00))+'|'+HexBytes(BINTOC(-(10/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(10/2.00)))+'|'+CFNMETA(10)+'|'+CFNMETA(2.00)
 CASE index=163
  RETURN CFNMETA(-10/2.00)+'|'+HexBytes(BINTOC(-10/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-10/2.00))+'|'+CFNMETA(-10)+'|'+CFNMETA(2.00)
 CASE index=164
  RETURN CFNMETA(-(-10/2.00))+'|'+HexBytes(BINTOC(-(-10/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10/2.00)))+'|'+CFNMETA(-10)+'|'+CFNMETA(2.00)
 CASE index=165
  RETURN CFNMETA(10/-2.00)+'|'+HexBytes(BINTOC(10/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(10/-2.00))+'|'+CFNMETA(10)+'|'+CFNMETA(-2.00)
 CASE index=166
  RETURN CFNMETA(-(10/-2.00))+'|'+HexBytes(BINTOC(-(10/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(10/-2.00)))+'|'+CFNMETA(10)+'|'+CFNMETA(-2.00)
 CASE index=167
  RETURN CFNMETA(-10/-2.00)+'|'+HexBytes(BINTOC(-10/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-10/-2.00))+'|'+CFNMETA(-10)+'|'+CFNMETA(-2.00)
 CASE index=168
  RETURN CFNMETA(-(-10/-2.00))+'|'+HexBytes(BINTOC(-(-10/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10/-2.00)))+'|'+CFNMETA(-10)+'|'+CFNMETA(-2.00)
 CASE index=169
  RETURN CFNMETA(10.00/2.00)+'|'+HexBytes(BINTOC(10.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(10.00/2.00))+'|'+CFNMETA(10.00)+'|'+CFNMETA(2.00)
 CASE index=170
  RETURN CFNMETA(-(10.00/2.00))+'|'+HexBytes(BINTOC(-(10.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(10.00/2.00)))+'|'+CFNMETA(10.00)+'|'+CFNMETA(2.00)
 CASE index=171
  RETURN CFNMETA(-10.00/2.00)+'|'+HexBytes(BINTOC(-10.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-10.00/2.00))+'|'+CFNMETA(-10.00)+'|'+CFNMETA(2.00)
 CASE index=172
  RETURN CFNMETA(-(-10.00/2.00))+'|'+HexBytes(BINTOC(-(-10.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10.00/2.00)))+'|'+CFNMETA(-10.00)+'|'+CFNMETA(2.00)
 CASE index=173
  RETURN CFNMETA(10.00/-2.00)+'|'+HexBytes(BINTOC(10.00/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(10.00/-2.00))+'|'+CFNMETA(10.00)+'|'+CFNMETA(-2.00)
 CASE index=174
  RETURN CFNMETA(-(10.00/-2.00))+'|'+HexBytes(BINTOC(-(10.00/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(10.00/-2.00)))+'|'+CFNMETA(10.00)+'|'+CFNMETA(-2.00)
 CASE index=175
  RETURN CFNMETA(-10.00/-2.00)+'|'+HexBytes(BINTOC(-10.00/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-10.00/-2.00))+'|'+CFNMETA(-10.00)+'|'+CFNMETA(-2.00)
 CASE index=176
  RETURN CFNMETA(-(-10.00/-2.00))+'|'+HexBytes(BINTOC(-(-10.00/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10.00/-2.00)))+'|'+CFNMETA(-10.00)+'|'+CFNMETA(-2.00)
 CASE index=177
  RETURN CFNMETA(10.000000/2.00)+'|'+HexBytes(BINTOC(10.000000/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(10.000000/2.00))+'|'+CFNMETA(10.000000)+'|'+CFNMETA(2.00)
 CASE index=178
  RETURN CFNMETA(-(10.000000/2.00))+'|'+HexBytes(BINTOC(-(10.000000/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(10.000000/2.00)))+'|'+CFNMETA(10.000000)+'|'+CFNMETA(2.00)
 CASE index=179
  RETURN CFNMETA(-10.000000/2.00)+'|'+HexBytes(BINTOC(-10.000000/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-10.000000/2.00))+'|'+CFNMETA(-10.000000)+'|'+CFNMETA(2.00)
 CASE index=180
  RETURN CFNMETA(-(-10.000000/2.00))+'|'+HexBytes(BINTOC(-(-10.000000/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10.000000/2.00)))+'|'+CFNMETA(-10.000000)+'|'+CFNMETA(2.00)
 CASE index=181
  RETURN CFNMETA(10.000000/-2.00)+'|'+HexBytes(BINTOC(10.000000/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(10.000000/-2.00))+'|'+CFNMETA(10.000000)+'|'+CFNMETA(-2.00)
 CASE index=182
  RETURN CFNMETA(-(10.000000/-2.00))+'|'+HexBytes(BINTOC(-(10.000000/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(10.000000/-2.00)))+'|'+CFNMETA(10.000000)+'|'+CFNMETA(-2.00)
 CASE index=183
  RETURN CFNMETA(-10.000000/-2.00)+'|'+HexBytes(BINTOC(-10.000000/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-10.000000/-2.00))+'|'+CFNMETA(-10.000000)+'|'+CFNMETA(-2.00)
 CASE index=184
  RETURN CFNMETA(-(-10.000000/-2.00))+'|'+HexBytes(BINTOC(-(-10.000000/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10.000000/-2.00)))+'|'+CFNMETA(-10.000000)+'|'+CFNMETA(-2.00)
 CASE index=185
  RETURN CFNMETA(10.000000000000000000/2.000000000000000000)+'|'+HexBytes(BINTOC(10.000000000000000000/2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(10.000000000000000000/2.000000000000000000))+'|'+CFNMETA(10.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=186
  RETURN CFNMETA(-(10.000000000000000000/2.000000000000000000))+'|'+HexBytes(BINTOC(-(10.000000000000000000/2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(10.000000000000000000/2.000000000000000000)))+'|'+CFNMETA(10.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=187
  RETURN CFNMETA(-10.000000000000000000/2.000000000000000000)+'|'+HexBytes(BINTOC(-10.000000000000000000/2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-10.000000000000000000/2.000000000000000000))+'|'+CFNMETA(-10.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=188
  RETURN CFNMETA(-(-10.000000000000000000/2.000000000000000000))+'|'+HexBytes(BINTOC(-(-10.000000000000000000/2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10.000000000000000000/2.000000000000000000)))+'|'+CFNMETA(-10.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=189
  RETURN CFNMETA(10.000000000000000000/-2.000000000000000000)+'|'+HexBytes(BINTOC(10.000000000000000000/-2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(10.000000000000000000/-2.000000000000000000))+'|'+CFNMETA(10.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=190
  RETURN CFNMETA(-(10.000000000000000000/-2.000000000000000000))+'|'+HexBytes(BINTOC(-(10.000000000000000000/-2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(10.000000000000000000/-2.000000000000000000)))+'|'+CFNMETA(10.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=191
  RETURN CFNMETA(-10.000000000000000000/-2.000000000000000000)+'|'+HexBytes(BINTOC(-10.000000000000000000/-2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-10.000000000000000000/-2.000000000000000000))+'|'+CFNMETA(-10.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=192
  RETURN CFNMETA(-(-10.000000000000000000/-2.000000000000000000))+'|'+HexBytes(BINTOC(-(-10.000000000000000000/-2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-10.000000000000000000/-2.000000000000000000)))+'|'+CFNMETA(-10.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=193
  RETURN CFNMETA(100/2)+'|'+HexBytes(BINTOC(100/2,'BRS'))+'|'+HexBytes(TRANSFORM(100/2))+'|'+CFNMETA(100)+'|'+CFNMETA(2)
 CASE index=194
  RETURN CFNMETA(-(100/2))+'|'+HexBytes(BINTOC(-(100/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(100/2)))+'|'+CFNMETA(100)+'|'+CFNMETA(2)
 CASE index=195
  RETURN CFNMETA(-100/2)+'|'+HexBytes(BINTOC(-100/2,'BRS'))+'|'+HexBytes(TRANSFORM(-100/2))+'|'+CFNMETA(-100)+'|'+CFNMETA(2)
 CASE index=196
  RETURN CFNMETA(-(-100/2))+'|'+HexBytes(BINTOC(-(-100/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100/2)))+'|'+CFNMETA(-100)+'|'+CFNMETA(2)
 CASE index=197
  RETURN CFNMETA(100/-2)+'|'+HexBytes(BINTOC(100/-2,'BRS'))+'|'+HexBytes(TRANSFORM(100/-2))+'|'+CFNMETA(100)+'|'+CFNMETA(-2)
 CASE index=198
  RETURN CFNMETA(-(100/-2))+'|'+HexBytes(BINTOC(-(100/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(100/-2)))+'|'+CFNMETA(100)+'|'+CFNMETA(-2)
 CASE index=199
  RETURN CFNMETA(-100/-2)+'|'+HexBytes(BINTOC(-100/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-100/-2))+'|'+CFNMETA(-100)+'|'+CFNMETA(-2)
 CASE index=200
  RETURN CFNMETA(-(-100/-2))+'|'+HexBytes(BINTOC(-(-100/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100/-2)))+'|'+CFNMETA(-100)+'|'+CFNMETA(-2)
 CASE index=201
  RETURN CFNMETA(0100/2)+'|'+HexBytes(BINTOC(0100/2,'BRS'))+'|'+HexBytes(TRANSFORM(0100/2))+'|'+CFNMETA(0100)+'|'+CFNMETA(2)
 CASE index=202
  RETURN CFNMETA(-(0100/2))+'|'+HexBytes(BINTOC(-(0100/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(0100/2)))+'|'+CFNMETA(0100)+'|'+CFNMETA(2)
 CASE index=203
  RETURN CFNMETA(-0100/2)+'|'+HexBytes(BINTOC(-0100/2,'BRS'))+'|'+HexBytes(TRANSFORM(-0100/2))+'|'+CFNMETA(-0100)+'|'+CFNMETA(2)
 CASE index=204
  RETURN CFNMETA(-(-0100/2))+'|'+HexBytes(BINTOC(-(-0100/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0100/2)))+'|'+CFNMETA(-0100)+'|'+CFNMETA(2)
 CASE index=205
  RETURN CFNMETA(0100/-2)+'|'+HexBytes(BINTOC(0100/-2,'BRS'))+'|'+HexBytes(TRANSFORM(0100/-2))+'|'+CFNMETA(0100)+'|'+CFNMETA(-2)
 CASE index=206
  RETURN CFNMETA(-(0100/-2))+'|'+HexBytes(BINTOC(-(0100/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(0100/-2)))+'|'+CFNMETA(0100)+'|'+CFNMETA(-2)
 CASE index=207
  RETURN CFNMETA(-0100/-2)+'|'+HexBytes(BINTOC(-0100/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-0100/-2))+'|'+CFNMETA(-0100)+'|'+CFNMETA(-2)
 CASE index=208
  RETURN CFNMETA(-(-0100/-2))+'|'+HexBytes(BINTOC(-(-0100/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0100/-2)))+'|'+CFNMETA(-0100)+'|'+CFNMETA(-2)
 CASE index=209
  RETURN CFNMETA(100/02)+'|'+HexBytes(BINTOC(100/02,'BRS'))+'|'+HexBytes(TRANSFORM(100/02))+'|'+CFNMETA(100)+'|'+CFNMETA(02)
 CASE index=210
  RETURN CFNMETA(-(100/02))+'|'+HexBytes(BINTOC(-(100/02),'BRS'))+'|'+HexBytes(TRANSFORM(-(100/02)))+'|'+CFNMETA(100)+'|'+CFNMETA(02)
 CASE index=211
  RETURN CFNMETA(-100/02)+'|'+HexBytes(BINTOC(-100/02,'BRS'))+'|'+HexBytes(TRANSFORM(-100/02))+'|'+CFNMETA(-100)+'|'+CFNMETA(02)
 CASE index=212
  RETURN CFNMETA(-(-100/02))+'|'+HexBytes(BINTOC(-(-100/02),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100/02)))+'|'+CFNMETA(-100)+'|'+CFNMETA(02)
 CASE index=213
  RETURN CFNMETA(100/-02)+'|'+HexBytes(BINTOC(100/-02,'BRS'))+'|'+HexBytes(TRANSFORM(100/-02))+'|'+CFNMETA(100)+'|'+CFNMETA(-02)
 CASE index=214
  RETURN CFNMETA(-(100/-02))+'|'+HexBytes(BINTOC(-(100/-02),'BRS'))+'|'+HexBytes(TRANSFORM(-(100/-02)))+'|'+CFNMETA(100)+'|'+CFNMETA(-02)
 CASE index=215
  RETURN CFNMETA(-100/-02)+'|'+HexBytes(BINTOC(-100/-02,'BRS'))+'|'+HexBytes(TRANSFORM(-100/-02))+'|'+CFNMETA(-100)+'|'+CFNMETA(-02)
 CASE index=216
  RETURN CFNMETA(-(-100/-02))+'|'+HexBytes(BINTOC(-(-100/-02),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100/-02)))+'|'+CFNMETA(-100)+'|'+CFNMETA(-02)
 CASE index=217
  RETURN CFNMETA(100.00/2)+'|'+HexBytes(BINTOC(100.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(100.00/2))+'|'+CFNMETA(100.00)+'|'+CFNMETA(2)
 CASE index=218
  RETURN CFNMETA(-(100.00/2))+'|'+HexBytes(BINTOC(-(100.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(100.00/2)))+'|'+CFNMETA(100.00)+'|'+CFNMETA(2)
 CASE index=219
  RETURN CFNMETA(-100.00/2)+'|'+HexBytes(BINTOC(-100.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(-100.00/2))+'|'+CFNMETA(-100.00)+'|'+CFNMETA(2)
 CASE index=220
  RETURN CFNMETA(-(-100.00/2))+'|'+HexBytes(BINTOC(-(-100.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100.00/2)))+'|'+CFNMETA(-100.00)+'|'+CFNMETA(2)
 CASE index=221
  RETURN CFNMETA(100.00/-2)+'|'+HexBytes(BINTOC(100.00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(100.00/-2))+'|'+CFNMETA(100.00)+'|'+CFNMETA(-2)
 CASE index=222
  RETURN CFNMETA(-(100.00/-2))+'|'+HexBytes(BINTOC(-(100.00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(100.00/-2)))+'|'+CFNMETA(100.00)+'|'+CFNMETA(-2)
 CASE index=223
  RETURN CFNMETA(-100.00/-2)+'|'+HexBytes(BINTOC(-100.00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-100.00/-2))+'|'+CFNMETA(-100.00)+'|'+CFNMETA(-2)
 CASE index=224
  RETURN CFNMETA(-(-100.00/-2))+'|'+HexBytes(BINTOC(-(-100.00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100.00/-2)))+'|'+CFNMETA(-100.00)+'|'+CFNMETA(-2)
 CASE index=225
  RETURN CFNMETA(100/2.00)+'|'+HexBytes(BINTOC(100/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(100/2.00))+'|'+CFNMETA(100)+'|'+CFNMETA(2.00)
 CASE index=226
  RETURN CFNMETA(-(100/2.00))+'|'+HexBytes(BINTOC(-(100/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(100/2.00)))+'|'+CFNMETA(100)+'|'+CFNMETA(2.00)
 CASE index=227
  RETURN CFNMETA(-100/2.00)+'|'+HexBytes(BINTOC(-100/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-100/2.00))+'|'+CFNMETA(-100)+'|'+CFNMETA(2.00)
 CASE index=228
  RETURN CFNMETA(-(-100/2.00))+'|'+HexBytes(BINTOC(-(-100/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100/2.00)))+'|'+CFNMETA(-100)+'|'+CFNMETA(2.00)
 CASE index=229
  RETURN CFNMETA(100/-2.00)+'|'+HexBytes(BINTOC(100/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(100/-2.00))+'|'+CFNMETA(100)+'|'+CFNMETA(-2.00)
 CASE index=230
  RETURN CFNMETA(-(100/-2.00))+'|'+HexBytes(BINTOC(-(100/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(100/-2.00)))+'|'+CFNMETA(100)+'|'+CFNMETA(-2.00)
 CASE index=231
  RETURN CFNMETA(-100/-2.00)+'|'+HexBytes(BINTOC(-100/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-100/-2.00))+'|'+CFNMETA(-100)+'|'+CFNMETA(-2.00)
 CASE index=232
  RETURN CFNMETA(-(-100/-2.00))+'|'+HexBytes(BINTOC(-(-100/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100/-2.00)))+'|'+CFNMETA(-100)+'|'+CFNMETA(-2.00)
 CASE index=233
  RETURN CFNMETA(100.00/2.00)+'|'+HexBytes(BINTOC(100.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(100.00/2.00))+'|'+CFNMETA(100.00)+'|'+CFNMETA(2.00)
 CASE index=234
  RETURN CFNMETA(-(100.00/2.00))+'|'+HexBytes(BINTOC(-(100.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(100.00/2.00)))+'|'+CFNMETA(100.00)+'|'+CFNMETA(2.00)
 CASE index=235
  RETURN CFNMETA(-100.00/2.00)+'|'+HexBytes(BINTOC(-100.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-100.00/2.00))+'|'+CFNMETA(-100.00)+'|'+CFNMETA(2.00)
 CASE index=236
  RETURN CFNMETA(-(-100.00/2.00))+'|'+HexBytes(BINTOC(-(-100.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100.00/2.00)))+'|'+CFNMETA(-100.00)+'|'+CFNMETA(2.00)
 CASE index=237
  RETURN CFNMETA(100.00/-2.00)+'|'+HexBytes(BINTOC(100.00/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(100.00/-2.00))+'|'+CFNMETA(100.00)+'|'+CFNMETA(-2.00)
 CASE index=238
  RETURN CFNMETA(-(100.00/-2.00))+'|'+HexBytes(BINTOC(-(100.00/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(100.00/-2.00)))+'|'+CFNMETA(100.00)+'|'+CFNMETA(-2.00)
 CASE index=239
  RETURN CFNMETA(-100.00/-2.00)+'|'+HexBytes(BINTOC(-100.00/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-100.00/-2.00))+'|'+CFNMETA(-100.00)+'|'+CFNMETA(-2.00)
 CASE index=240
  RETURN CFNMETA(-(-100.00/-2.00))+'|'+HexBytes(BINTOC(-(-100.00/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100.00/-2.00)))+'|'+CFNMETA(-100.00)+'|'+CFNMETA(-2.00)
 CASE index=241
  RETURN CFNMETA(100.000000/2.00)+'|'+HexBytes(BINTOC(100.000000/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(100.000000/2.00))+'|'+CFNMETA(100.000000)+'|'+CFNMETA(2.00)
 CASE index=242
  RETURN CFNMETA(-(100.000000/2.00))+'|'+HexBytes(BINTOC(-(100.000000/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(100.000000/2.00)))+'|'+CFNMETA(100.000000)+'|'+CFNMETA(2.00)
 CASE index=243
  RETURN CFNMETA(-100.000000/2.00)+'|'+HexBytes(BINTOC(-100.000000/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-100.000000/2.00))+'|'+CFNMETA(-100.000000)+'|'+CFNMETA(2.00)
 CASE index=244
  RETURN CFNMETA(-(-100.000000/2.00))+'|'+HexBytes(BINTOC(-(-100.000000/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100.000000/2.00)))+'|'+CFNMETA(-100.000000)+'|'+CFNMETA(2.00)
 CASE index=245
  RETURN CFNMETA(100.000000/-2.00)+'|'+HexBytes(BINTOC(100.000000/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(100.000000/-2.00))+'|'+CFNMETA(100.000000)+'|'+CFNMETA(-2.00)
 CASE index=246
  RETURN CFNMETA(-(100.000000/-2.00))+'|'+HexBytes(BINTOC(-(100.000000/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(100.000000/-2.00)))+'|'+CFNMETA(100.000000)+'|'+CFNMETA(-2.00)
 CASE index=247
  RETURN CFNMETA(-100.000000/-2.00)+'|'+HexBytes(BINTOC(-100.000000/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-100.000000/-2.00))+'|'+CFNMETA(-100.000000)+'|'+CFNMETA(-2.00)
 CASE index=248
  RETURN CFNMETA(-(-100.000000/-2.00))+'|'+HexBytes(BINTOC(-(-100.000000/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100.000000/-2.00)))+'|'+CFNMETA(-100.000000)+'|'+CFNMETA(-2.00)
 CASE index=249
  RETURN CFNMETA(100.000000000000000000/2.000000000000000000)+'|'+HexBytes(BINTOC(100.000000000000000000/2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(100.000000000000000000/2.000000000000000000))+'|'+CFNMETA(100.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=250
  RETURN CFNMETA(-(100.000000000000000000/2.000000000000000000))+'|'+HexBytes(BINTOC(-(100.000000000000000000/2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(100.000000000000000000/2.000000000000000000)))+'|'+CFNMETA(100.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=251
  RETURN CFNMETA(-100.000000000000000000/2.000000000000000000)+'|'+HexBytes(BINTOC(-100.000000000000000000/2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-100.000000000000000000/2.000000000000000000))+'|'+CFNMETA(-100.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=252
  RETURN CFNMETA(-(-100.000000000000000000/2.000000000000000000))+'|'+HexBytes(BINTOC(-(-100.000000000000000000/2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100.000000000000000000/2.000000000000000000)))+'|'+CFNMETA(-100.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=253
  RETURN CFNMETA(100.000000000000000000/-2.000000000000000000)+'|'+HexBytes(BINTOC(100.000000000000000000/-2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(100.000000000000000000/-2.000000000000000000))+'|'+CFNMETA(100.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=254
  RETURN CFNMETA(-(100.000000000000000000/-2.000000000000000000))+'|'+HexBytes(BINTOC(-(100.000000000000000000/-2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(100.000000000000000000/-2.000000000000000000)))+'|'+CFNMETA(100.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=255
  RETURN CFNMETA(-100.000000000000000000/-2.000000000000000000)+'|'+HexBytes(BINTOC(-100.000000000000000000/-2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-100.000000000000000000/-2.000000000000000000))+'|'+CFNMETA(-100.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=256
  RETURN CFNMETA(-(-100.000000000000000000/-2.000000000000000000))+'|'+HexBytes(BINTOC(-(-100.000000000000000000/-2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-100.000000000000000000/-2.000000000000000000)))+'|'+CFNMETA(-100.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=257
  RETURN CFNMETA(200/15)+'|'+HexBytes(BINTOC(200/15,'BRS'))+'|'+HexBytes(TRANSFORM(200/15))+'|'+CFNMETA(200)+'|'+CFNMETA(15)
 CASE index=258
  RETURN CFNMETA(-(200/15))+'|'+HexBytes(BINTOC(-(200/15),'BRS'))+'|'+HexBytes(TRANSFORM(-(200/15)))+'|'+CFNMETA(200)+'|'+CFNMETA(15)
 CASE index=259
  RETURN CFNMETA(-200/15)+'|'+HexBytes(BINTOC(-200/15,'BRS'))+'|'+HexBytes(TRANSFORM(-200/15))+'|'+CFNMETA(-200)+'|'+CFNMETA(15)
 CASE index=260
  RETURN CFNMETA(-(-200/15))+'|'+HexBytes(BINTOC(-(-200/15),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200/15)))+'|'+CFNMETA(-200)+'|'+CFNMETA(15)
 CASE index=261
  RETURN CFNMETA(200/-15)+'|'+HexBytes(BINTOC(200/-15,'BRS'))+'|'+HexBytes(TRANSFORM(200/-15))+'|'+CFNMETA(200)+'|'+CFNMETA(-15)
 CASE index=262
  RETURN CFNMETA(-(200/-15))+'|'+HexBytes(BINTOC(-(200/-15),'BRS'))+'|'+HexBytes(TRANSFORM(-(200/-15)))+'|'+CFNMETA(200)+'|'+CFNMETA(-15)
 CASE index=263
  RETURN CFNMETA(-200/-15)+'|'+HexBytes(BINTOC(-200/-15,'BRS'))+'|'+HexBytes(TRANSFORM(-200/-15))+'|'+CFNMETA(-200)+'|'+CFNMETA(-15)
 CASE index=264
  RETURN CFNMETA(-(-200/-15))+'|'+HexBytes(BINTOC(-(-200/-15),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200/-15)))+'|'+CFNMETA(-200)+'|'+CFNMETA(-15)
 CASE index=265
  RETURN CFNMETA(0200/15)+'|'+HexBytes(BINTOC(0200/15,'BRS'))+'|'+HexBytes(TRANSFORM(0200/15))+'|'+CFNMETA(0200)+'|'+CFNMETA(15)
 CASE index=266
  RETURN CFNMETA(-(0200/15))+'|'+HexBytes(BINTOC(-(0200/15),'BRS'))+'|'+HexBytes(TRANSFORM(-(0200/15)))+'|'+CFNMETA(0200)+'|'+CFNMETA(15)
 CASE index=267
  RETURN CFNMETA(-0200/15)+'|'+HexBytes(BINTOC(-0200/15,'BRS'))+'|'+HexBytes(TRANSFORM(-0200/15))+'|'+CFNMETA(-0200)+'|'+CFNMETA(15)
 CASE index=268
  RETURN CFNMETA(-(-0200/15))+'|'+HexBytes(BINTOC(-(-0200/15),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0200/15)))+'|'+CFNMETA(-0200)+'|'+CFNMETA(15)
 CASE index=269
  RETURN CFNMETA(0200/-15)+'|'+HexBytes(BINTOC(0200/-15,'BRS'))+'|'+HexBytes(TRANSFORM(0200/-15))+'|'+CFNMETA(0200)+'|'+CFNMETA(-15)
 CASE index=270
  RETURN CFNMETA(-(0200/-15))+'|'+HexBytes(BINTOC(-(0200/-15),'BRS'))+'|'+HexBytes(TRANSFORM(-(0200/-15)))+'|'+CFNMETA(0200)+'|'+CFNMETA(-15)
 CASE index=271
  RETURN CFNMETA(-0200/-15)+'|'+HexBytes(BINTOC(-0200/-15,'BRS'))+'|'+HexBytes(TRANSFORM(-0200/-15))+'|'+CFNMETA(-0200)+'|'+CFNMETA(-15)
 CASE index=272
  RETURN CFNMETA(-(-0200/-15))+'|'+HexBytes(BINTOC(-(-0200/-15),'BRS'))+'|'+HexBytes(TRANSFORM(-(-0200/-15)))+'|'+CFNMETA(-0200)+'|'+CFNMETA(-15)
 CASE index=273
  RETURN CFNMETA(200/015)+'|'+HexBytes(BINTOC(200/015,'BRS'))+'|'+HexBytes(TRANSFORM(200/015))+'|'+CFNMETA(200)+'|'+CFNMETA(015)
 CASE index=274
  RETURN CFNMETA(-(200/015))+'|'+HexBytes(BINTOC(-(200/015),'BRS'))+'|'+HexBytes(TRANSFORM(-(200/015)))+'|'+CFNMETA(200)+'|'+CFNMETA(015)
 CASE index=275
  RETURN CFNMETA(-200/015)+'|'+HexBytes(BINTOC(-200/015,'BRS'))+'|'+HexBytes(TRANSFORM(-200/015))+'|'+CFNMETA(-200)+'|'+CFNMETA(015)
 CASE index=276
  RETURN CFNMETA(-(-200/015))+'|'+HexBytes(BINTOC(-(-200/015),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200/015)))+'|'+CFNMETA(-200)+'|'+CFNMETA(015)
 CASE index=277
  RETURN CFNMETA(200/-015)+'|'+HexBytes(BINTOC(200/-015,'BRS'))+'|'+HexBytes(TRANSFORM(200/-015))+'|'+CFNMETA(200)+'|'+CFNMETA(-015)
 CASE index=278
  RETURN CFNMETA(-(200/-015))+'|'+HexBytes(BINTOC(-(200/-015),'BRS'))+'|'+HexBytes(TRANSFORM(-(200/-015)))+'|'+CFNMETA(200)+'|'+CFNMETA(-015)
 CASE index=279
  RETURN CFNMETA(-200/-015)+'|'+HexBytes(BINTOC(-200/-015,'BRS'))+'|'+HexBytes(TRANSFORM(-200/-015))+'|'+CFNMETA(-200)+'|'+CFNMETA(-015)
 CASE index=280
  RETURN CFNMETA(-(-200/-015))+'|'+HexBytes(BINTOC(-(-200/-015),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200/-015)))+'|'+CFNMETA(-200)+'|'+CFNMETA(-015)
 CASE index=281
  RETURN CFNMETA(200.00/15)+'|'+HexBytes(BINTOC(200.00/15,'BRS'))+'|'+HexBytes(TRANSFORM(200.00/15))+'|'+CFNMETA(200.00)+'|'+CFNMETA(15)
 CASE index=282
  RETURN CFNMETA(-(200.00/15))+'|'+HexBytes(BINTOC(-(200.00/15),'BRS'))+'|'+HexBytes(TRANSFORM(-(200.00/15)))+'|'+CFNMETA(200.00)+'|'+CFNMETA(15)
 CASE index=283
  RETURN CFNMETA(-200.00/15)+'|'+HexBytes(BINTOC(-200.00/15,'BRS'))+'|'+HexBytes(TRANSFORM(-200.00/15))+'|'+CFNMETA(-200.00)+'|'+CFNMETA(15)
 CASE index=284
  RETURN CFNMETA(-(-200.00/15))+'|'+HexBytes(BINTOC(-(-200.00/15),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200.00/15)))+'|'+CFNMETA(-200.00)+'|'+CFNMETA(15)
 CASE index=285
  RETURN CFNMETA(200.00/-15)+'|'+HexBytes(BINTOC(200.00/-15,'BRS'))+'|'+HexBytes(TRANSFORM(200.00/-15))+'|'+CFNMETA(200.00)+'|'+CFNMETA(-15)
 CASE index=286
  RETURN CFNMETA(-(200.00/-15))+'|'+HexBytes(BINTOC(-(200.00/-15),'BRS'))+'|'+HexBytes(TRANSFORM(-(200.00/-15)))+'|'+CFNMETA(200.00)+'|'+CFNMETA(-15)
 CASE index=287
  RETURN CFNMETA(-200.00/-15)+'|'+HexBytes(BINTOC(-200.00/-15,'BRS'))+'|'+HexBytes(TRANSFORM(-200.00/-15))+'|'+CFNMETA(-200.00)+'|'+CFNMETA(-15)
 CASE index=288
  RETURN CFNMETA(-(-200.00/-15))+'|'+HexBytes(BINTOC(-(-200.00/-15),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200.00/-15)))+'|'+CFNMETA(-200.00)+'|'+CFNMETA(-15)
 CASE index=289
  RETURN CFNMETA(200/15.00)+'|'+HexBytes(BINTOC(200/15.00,'BRS'))+'|'+HexBytes(TRANSFORM(200/15.00))+'|'+CFNMETA(200)+'|'+CFNMETA(15.00)
 CASE index=290
  RETURN CFNMETA(-(200/15.00))+'|'+HexBytes(BINTOC(-(200/15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(200/15.00)))+'|'+CFNMETA(200)+'|'+CFNMETA(15.00)
 CASE index=291
  RETURN CFNMETA(-200/15.00)+'|'+HexBytes(BINTOC(-200/15.00,'BRS'))+'|'+HexBytes(TRANSFORM(-200/15.00))+'|'+CFNMETA(-200)+'|'+CFNMETA(15.00)
 CASE index=292
  RETURN CFNMETA(-(-200/15.00))+'|'+HexBytes(BINTOC(-(-200/15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200/15.00)))+'|'+CFNMETA(-200)+'|'+CFNMETA(15.00)
 CASE index=293
  RETURN CFNMETA(200/-15.00)+'|'+HexBytes(BINTOC(200/-15.00,'BRS'))+'|'+HexBytes(TRANSFORM(200/-15.00))+'|'+CFNMETA(200)+'|'+CFNMETA(-15.00)
 CASE index=294
  RETURN CFNMETA(-(200/-15.00))+'|'+HexBytes(BINTOC(-(200/-15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(200/-15.00)))+'|'+CFNMETA(200)+'|'+CFNMETA(-15.00)
 CASE index=295
  RETURN CFNMETA(-200/-15.00)+'|'+HexBytes(BINTOC(-200/-15.00,'BRS'))+'|'+HexBytes(TRANSFORM(-200/-15.00))+'|'+CFNMETA(-200)+'|'+CFNMETA(-15.00)
 CASE index=296
  RETURN CFNMETA(-(-200/-15.00))+'|'+HexBytes(BINTOC(-(-200/-15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200/-15.00)))+'|'+CFNMETA(-200)+'|'+CFNMETA(-15.00)
 CASE index=297
  RETURN CFNMETA(200.00/15.00)+'|'+HexBytes(BINTOC(200.00/15.00,'BRS'))+'|'+HexBytes(TRANSFORM(200.00/15.00))+'|'+CFNMETA(200.00)+'|'+CFNMETA(15.00)
 CASE index=298
  RETURN CFNMETA(-(200.00/15.00))+'|'+HexBytes(BINTOC(-(200.00/15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(200.00/15.00)))+'|'+CFNMETA(200.00)+'|'+CFNMETA(15.00)
 CASE index=299
  RETURN CFNMETA(-200.00/15.00)+'|'+HexBytes(BINTOC(-200.00/15.00,'BRS'))+'|'+HexBytes(TRANSFORM(-200.00/15.00))+'|'+CFNMETA(-200.00)+'|'+CFNMETA(15.00)
 CASE index=300
  RETURN CFNMETA(-(-200.00/15.00))+'|'+HexBytes(BINTOC(-(-200.00/15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200.00/15.00)))+'|'+CFNMETA(-200.00)+'|'+CFNMETA(15.00)
 CASE index=301
  RETURN CFNMETA(200.00/-15.00)+'|'+HexBytes(BINTOC(200.00/-15.00,'BRS'))+'|'+HexBytes(TRANSFORM(200.00/-15.00))+'|'+CFNMETA(200.00)+'|'+CFNMETA(-15.00)
 CASE index=302
  RETURN CFNMETA(-(200.00/-15.00))+'|'+HexBytes(BINTOC(-(200.00/-15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(200.00/-15.00)))+'|'+CFNMETA(200.00)+'|'+CFNMETA(-15.00)
 CASE index=303
  RETURN CFNMETA(-200.00/-15.00)+'|'+HexBytes(BINTOC(-200.00/-15.00,'BRS'))+'|'+HexBytes(TRANSFORM(-200.00/-15.00))+'|'+CFNMETA(-200.00)+'|'+CFNMETA(-15.00)
 CASE index=304
  RETURN CFNMETA(-(-200.00/-15.00))+'|'+HexBytes(BINTOC(-(-200.00/-15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200.00/-15.00)))+'|'+CFNMETA(-200.00)+'|'+CFNMETA(-15.00)
 CASE index=305
  RETURN CFNMETA(200.000000/15.00)+'|'+HexBytes(BINTOC(200.000000/15.00,'BRS'))+'|'+HexBytes(TRANSFORM(200.000000/15.00))+'|'+CFNMETA(200.000000)+'|'+CFNMETA(15.00)
 CASE index=306
  RETURN CFNMETA(-(200.000000/15.00))+'|'+HexBytes(BINTOC(-(200.000000/15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(200.000000/15.00)))+'|'+CFNMETA(200.000000)+'|'+CFNMETA(15.00)
 CASE index=307
  RETURN CFNMETA(-200.000000/15.00)+'|'+HexBytes(BINTOC(-200.000000/15.00,'BRS'))+'|'+HexBytes(TRANSFORM(-200.000000/15.00))+'|'+CFNMETA(-200.000000)+'|'+CFNMETA(15.00)
 CASE index=308
  RETURN CFNMETA(-(-200.000000/15.00))+'|'+HexBytes(BINTOC(-(-200.000000/15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200.000000/15.00)))+'|'+CFNMETA(-200.000000)+'|'+CFNMETA(15.00)
 CASE index=309
  RETURN CFNMETA(200.000000/-15.00)+'|'+HexBytes(BINTOC(200.000000/-15.00,'BRS'))+'|'+HexBytes(TRANSFORM(200.000000/-15.00))+'|'+CFNMETA(200.000000)+'|'+CFNMETA(-15.00)
 CASE index=310
  RETURN CFNMETA(-(200.000000/-15.00))+'|'+HexBytes(BINTOC(-(200.000000/-15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(200.000000/-15.00)))+'|'+CFNMETA(200.000000)+'|'+CFNMETA(-15.00)
 CASE index=311
  RETURN CFNMETA(-200.000000/-15.00)+'|'+HexBytes(BINTOC(-200.000000/-15.00,'BRS'))+'|'+HexBytes(TRANSFORM(-200.000000/-15.00))+'|'+CFNMETA(-200.000000)+'|'+CFNMETA(-15.00)
 CASE index=312
  RETURN CFNMETA(-(-200.000000/-15.00))+'|'+HexBytes(BINTOC(-(-200.000000/-15.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200.000000/-15.00)))+'|'+CFNMETA(-200.000000)+'|'+CFNMETA(-15.00)
 CASE index=313
  RETURN CFNMETA(200.000000000000000000/15.000000000000000000)+'|'+HexBytes(BINTOC(200.000000000000000000/15.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(200.000000000000000000/15.000000000000000000))+'|'+CFNMETA(200.000000000000000000)+'|'+CFNMETA(15.000000000000000000)
 CASE index=314
  RETURN CFNMETA(-(200.000000000000000000/15.000000000000000000))+'|'+HexBytes(BINTOC(-(200.000000000000000000/15.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(200.000000000000000000/15.000000000000000000)))+'|'+CFNMETA(200.000000000000000000)+'|'+CFNMETA(15.000000000000000000)
 CASE index=315
  RETURN CFNMETA(-200.000000000000000000/15.000000000000000000)+'|'+HexBytes(BINTOC(-200.000000000000000000/15.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-200.000000000000000000/15.000000000000000000))+'|'+CFNMETA(-200.000000000000000000)+'|'+CFNMETA(15.000000000000000000)
 CASE index=316
  RETURN CFNMETA(-(-200.000000000000000000/15.000000000000000000))+'|'+HexBytes(BINTOC(-(-200.000000000000000000/15.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200.000000000000000000/15.000000000000000000)))+'|'+CFNMETA(-200.000000000000000000)+'|'+CFNMETA(15.000000000000000000)
 CASE index=317
  RETURN CFNMETA(200.000000000000000000/-15.000000000000000000)+'|'+HexBytes(BINTOC(200.000000000000000000/-15.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(200.000000000000000000/-15.000000000000000000))+'|'+CFNMETA(200.000000000000000000)+'|'+CFNMETA(-15.000000000000000000)
 CASE index=318
  RETURN CFNMETA(-(200.000000000000000000/-15.000000000000000000))+'|'+HexBytes(BINTOC(-(200.000000000000000000/-15.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(200.000000000000000000/-15.000000000000000000)))+'|'+CFNMETA(200.000000000000000000)+'|'+CFNMETA(-15.000000000000000000)
 CASE index=319
  RETURN CFNMETA(-200.000000000000000000/-15.000000000000000000)+'|'+HexBytes(BINTOC(-200.000000000000000000/-15.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-200.000000000000000000/-15.000000000000000000))+'|'+CFNMETA(-200.000000000000000000)+'|'+CFNMETA(-15.000000000000000000)
 CASE index=320
  RETURN CFNMETA(-(-200.000000000000000000/-15.000000000000000000))+'|'+HexBytes(BINTOC(-(-200.000000000000000000/-15.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-200.000000000000000000/-15.000000000000000000)))+'|'+CFNMETA(-200.000000000000000000)+'|'+CFNMETA(-15.000000000000000000)
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
