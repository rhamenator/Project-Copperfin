* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DIVISION-CONTEXT-001: constant payload/width/promotion recovery.
LOCAL ARRAY expressions[320], lefts[320], rights[320], settings[5]
expressions[1]='15/200'
lefts[1]='15'
rights[1]='200'
expressions[2]='-(15/200)'
lefts[2]='15'
rights[2]='200'
expressions[3]='-15/200'
lefts[3]='-15'
rights[3]='200'
expressions[4]='-(-15/200)'
lefts[4]='-15'
rights[4]='200'
expressions[5]='15/-200'
lefts[5]='15'
rights[5]='-200'
expressions[6]='-(15/-200)'
lefts[6]='15'
rights[6]='-200'
expressions[7]='-15/-200'
lefts[7]='-15'
rights[7]='-200'
expressions[8]='-(-15/-200)'
lefts[8]='-15'
rights[8]='-200'
expressions[9]='015/200'
lefts[9]='015'
rights[9]='200'
expressions[10]='-(015/200)'
lefts[10]='015'
rights[10]='200'
expressions[11]='-015/200'
lefts[11]='-015'
rights[11]='200'
expressions[12]='-(-015/200)'
lefts[12]='-015'
rights[12]='200'
expressions[13]='015/-200'
lefts[13]='015'
rights[13]='-200'
expressions[14]='-(015/-200)'
lefts[14]='015'
rights[14]='-200'
expressions[15]='-015/-200'
lefts[15]='-015'
rights[15]='-200'
expressions[16]='-(-015/-200)'
lefts[16]='-015'
rights[16]='-200'
expressions[17]='15/0200'
lefts[17]='15'
rights[17]='0200'
expressions[18]='-(15/0200)'
lefts[18]='15'
rights[18]='0200'
expressions[19]='-15/0200'
lefts[19]='-15'
rights[19]='0200'
expressions[20]='-(-15/0200)'
lefts[20]='-15'
rights[20]='0200'
expressions[21]='15/-0200'
lefts[21]='15'
rights[21]='-0200'
expressions[22]='-(15/-0200)'
lefts[22]='15'
rights[22]='-0200'
expressions[23]='-15/-0200'
lefts[23]='-15'
rights[23]='-0200'
expressions[24]='-(-15/-0200)'
lefts[24]='-15'
rights[24]='-0200'
expressions[25]='15.00/200'
lefts[25]='15.00'
rights[25]='200'
expressions[26]='-(15.00/200)'
lefts[26]='15.00'
rights[26]='200'
expressions[27]='-15.00/200'
lefts[27]='-15.00'
rights[27]='200'
expressions[28]='-(-15.00/200)'
lefts[28]='-15.00'
rights[28]='200'
expressions[29]='15.00/-200'
lefts[29]='15.00'
rights[29]='-200'
expressions[30]='-(15.00/-200)'
lefts[30]='15.00'
rights[30]='-200'
expressions[31]='-15.00/-200'
lefts[31]='-15.00'
rights[31]='-200'
expressions[32]='-(-15.00/-200)'
lefts[32]='-15.00'
rights[32]='-200'
expressions[33]='15/200.00'
lefts[33]='15'
rights[33]='200.00'
expressions[34]='-(15/200.00)'
lefts[34]='15'
rights[34]='200.00'
expressions[35]='-15/200.00'
lefts[35]='-15'
rights[35]='200.00'
expressions[36]='-(-15/200.00)'
lefts[36]='-15'
rights[36]='200.00'
expressions[37]='15/-200.00'
lefts[37]='15'
rights[37]='-200.00'
expressions[38]='-(15/-200.00)'
lefts[38]='15'
rights[38]='-200.00'
expressions[39]='-15/-200.00'
lefts[39]='-15'
rights[39]='-200.00'
expressions[40]='-(-15/-200.00)'
lefts[40]='-15'
rights[40]='-200.00'
expressions[41]='15.00/200.00'
lefts[41]='15.00'
rights[41]='200.00'
expressions[42]='-(15.00/200.00)'
lefts[42]='15.00'
rights[42]='200.00'
expressions[43]='-15.00/200.00'
lefts[43]='-15.00'
rights[43]='200.00'
expressions[44]='-(-15.00/200.00)'
lefts[44]='-15.00'
rights[44]='200.00'
expressions[45]='15.00/-200.00'
lefts[45]='15.00'
rights[45]='-200.00'
expressions[46]='-(15.00/-200.00)'
lefts[46]='15.00'
rights[46]='-200.00'
expressions[47]='-15.00/-200.00'
lefts[47]='-15.00'
rights[47]='-200.00'
expressions[48]='-(-15.00/-200.00)'
lefts[48]='-15.00'
rights[48]='-200.00'
expressions[49]='15.000000/200.00'
lefts[49]='15.000000'
rights[49]='200.00'
expressions[50]='-(15.000000/200.00)'
lefts[50]='15.000000'
rights[50]='200.00'
expressions[51]='-15.000000/200.00'
lefts[51]='-15.000000'
rights[51]='200.00'
expressions[52]='-(-15.000000/200.00)'
lefts[52]='-15.000000'
rights[52]='200.00'
expressions[53]='15.000000/-200.00'
lefts[53]='15.000000'
rights[53]='-200.00'
expressions[54]='-(15.000000/-200.00)'
lefts[54]='15.000000'
rights[54]='-200.00'
expressions[55]='-15.000000/-200.00'
lefts[55]='-15.000000'
rights[55]='-200.00'
expressions[56]='-(-15.000000/-200.00)'
lefts[56]='-15.000000'
rights[56]='-200.00'
expressions[57]='15.000000000000000000/200.000000000000000000'
lefts[57]='15.000000000000000000'
rights[57]='200.000000000000000000'
expressions[58]='-(15.000000000000000000/200.000000000000000000)'
lefts[58]='15.000000000000000000'
rights[58]='200.000000000000000000'
expressions[59]='-15.000000000000000000/200.000000000000000000'
lefts[59]='-15.000000000000000000'
rights[59]='200.000000000000000000'
expressions[60]='-(-15.000000000000000000/200.000000000000000000)'
lefts[60]='-15.000000000000000000'
rights[60]='200.000000000000000000'
expressions[61]='15.000000000000000000/-200.000000000000000000'
lefts[61]='15.000000000000000000'
rights[61]='-200.000000000000000000'
expressions[62]='-(15.000000000000000000/-200.000000000000000000)'
lefts[62]='15.000000000000000000'
rights[62]='-200.000000000000000000'
expressions[63]='-15.000000000000000000/-200.000000000000000000'
lefts[63]='-15.000000000000000000'
rights[63]='-200.000000000000000000'
expressions[64]='-(-15.000000000000000000/-200.000000000000000000)'
lefts[64]='-15.000000000000000000'
rights[64]='-200.000000000000000000'
expressions[65]='2147483646/2'
lefts[65]='2147483646'
rights[65]='2'
expressions[66]='-(2147483646/2)'
lefts[66]='2147483646'
rights[66]='2'
expressions[67]='-2147483646/2'
lefts[67]='-2147483646'
rights[67]='2'
expressions[68]='-(-2147483646/2)'
lefts[68]='-2147483646'
rights[68]='2'
expressions[69]='2147483646/-2'
lefts[69]='2147483646'
rights[69]='-2'
expressions[70]='-(2147483646/-2)'
lefts[70]='2147483646'
rights[70]='-2'
expressions[71]='-2147483646/-2'
lefts[71]='-2147483646'
rights[71]='-2'
expressions[72]='-(-2147483646/-2)'
lefts[72]='-2147483646'
rights[72]='-2'
expressions[73]='02147483646/2'
lefts[73]='02147483646'
rights[73]='2'
expressions[74]='-(02147483646/2)'
lefts[74]='02147483646'
rights[74]='2'
expressions[75]='-02147483646/2'
lefts[75]='-02147483646'
rights[75]='2'
expressions[76]='-(-02147483646/2)'
lefts[76]='-02147483646'
rights[76]='2'
expressions[77]='02147483646/-2'
lefts[77]='02147483646'
rights[77]='-2'
expressions[78]='-(02147483646/-2)'
lefts[78]='02147483646'
rights[78]='-2'
expressions[79]='-02147483646/-2'
lefts[79]='-02147483646'
rights[79]='-2'
expressions[80]='-(-02147483646/-2)'
lefts[80]='-02147483646'
rights[80]='-2'
expressions[81]='2147483646/02'
lefts[81]='2147483646'
rights[81]='02'
expressions[82]='-(2147483646/02)'
lefts[82]='2147483646'
rights[82]='02'
expressions[83]='-2147483646/02'
lefts[83]='-2147483646'
rights[83]='02'
expressions[84]='-(-2147483646/02)'
lefts[84]='-2147483646'
rights[84]='02'
expressions[85]='2147483646/-02'
lefts[85]='2147483646'
rights[85]='-02'
expressions[86]='-(2147483646/-02)'
lefts[86]='2147483646'
rights[86]='-02'
expressions[87]='-2147483646/-02'
lefts[87]='-2147483646'
rights[87]='-02'
expressions[88]='-(-2147483646/-02)'
lefts[88]='-2147483646'
rights[88]='-02'
expressions[89]='2147483646.00/2'
lefts[89]='2147483646.00'
rights[89]='2'
expressions[90]='-(2147483646.00/2)'
lefts[90]='2147483646.00'
rights[90]='2'
expressions[91]='-2147483646.00/2'
lefts[91]='-2147483646.00'
rights[91]='2'
expressions[92]='-(-2147483646.00/2)'
lefts[92]='-2147483646.00'
rights[92]='2'
expressions[93]='2147483646.00/-2'
lefts[93]='2147483646.00'
rights[93]='-2'
expressions[94]='-(2147483646.00/-2)'
lefts[94]='2147483646.00'
rights[94]='-2'
expressions[95]='-2147483646.00/-2'
lefts[95]='-2147483646.00'
rights[95]='-2'
expressions[96]='-(-2147483646.00/-2)'
lefts[96]='-2147483646.00'
rights[96]='-2'
expressions[97]='2147483646/2.00'
lefts[97]='2147483646'
rights[97]='2.00'
expressions[98]='-(2147483646/2.00)'
lefts[98]='2147483646'
rights[98]='2.00'
expressions[99]='-2147483646/2.00'
lefts[99]='-2147483646'
rights[99]='2.00'
expressions[100]='-(-2147483646/2.00)'
lefts[100]='-2147483646'
rights[100]='2.00'
expressions[101]='2147483646/-2.00'
lefts[101]='2147483646'
rights[101]='-2.00'
expressions[102]='-(2147483646/-2.00)'
lefts[102]='2147483646'
rights[102]='-2.00'
expressions[103]='-2147483646/-2.00'
lefts[103]='-2147483646'
rights[103]='-2.00'
expressions[104]='-(-2147483646/-2.00)'
lefts[104]='-2147483646'
rights[104]='-2.00'
expressions[105]='2147483646.00/2.00'
lefts[105]='2147483646.00'
rights[105]='2.00'
expressions[106]='-(2147483646.00/2.00)'
lefts[106]='2147483646.00'
rights[106]='2.00'
expressions[107]='-2147483646.00/2.00'
lefts[107]='-2147483646.00'
rights[107]='2.00'
expressions[108]='-(-2147483646.00/2.00)'
lefts[108]='-2147483646.00'
rights[108]='2.00'
expressions[109]='2147483646.00/-2.00'
lefts[109]='2147483646.00'
rights[109]='-2.00'
expressions[110]='-(2147483646.00/-2.00)'
lefts[110]='2147483646.00'
rights[110]='-2.00'
expressions[111]='-2147483646.00/-2.00'
lefts[111]='-2147483646.00'
rights[111]='-2.00'
expressions[112]='-(-2147483646.00/-2.00)'
lefts[112]='-2147483646.00'
rights[112]='-2.00'
expressions[113]='2147483646.000000/2.00'
lefts[113]='2147483646.000000'
rights[113]='2.00'
expressions[114]='-(2147483646.000000/2.00)'
lefts[114]='2147483646.000000'
rights[114]='2.00'
expressions[115]='-2147483646.000000/2.00'
lefts[115]='-2147483646.000000'
rights[115]='2.00'
expressions[116]='-(-2147483646.000000/2.00)'
lefts[116]='-2147483646.000000'
rights[116]='2.00'
expressions[117]='2147483646.000000/-2.00'
lefts[117]='2147483646.000000'
rights[117]='-2.00'
expressions[118]='-(2147483646.000000/-2.00)'
lefts[118]='2147483646.000000'
rights[118]='-2.00'
expressions[119]='-2147483646.000000/-2.00'
lefts[119]='-2147483646.000000'
rights[119]='-2.00'
expressions[120]='-(-2147483646.000000/-2.00)'
lefts[120]='-2147483646.000000'
rights[120]='-2.00'
expressions[121]='2147483646.000000000000000000/2.000000000000000000'
lefts[121]='2147483646.000000000000000000'
rights[121]='2.000000000000000000'
expressions[122]='-(2147483646.000000000000000000/2.000000000000000000)'
lefts[122]='2147483646.000000000000000000'
rights[122]='2.000000000000000000'
expressions[123]='-2147483646.000000000000000000/2.000000000000000000'
lefts[123]='-2147483646.000000000000000000'
rights[123]='2.000000000000000000'
expressions[124]='-(-2147483646.000000000000000000/2.000000000000000000)'
lefts[124]='-2147483646.000000000000000000'
rights[124]='2.000000000000000000'
expressions[125]='2147483646.000000000000000000/-2.000000000000000000'
lefts[125]='2147483646.000000000000000000'
rights[125]='-2.000000000000000000'
expressions[126]='-(2147483646.000000000000000000/-2.000000000000000000)'
lefts[126]='2147483646.000000000000000000'
rights[126]='-2.000000000000000000'
expressions[127]='-2147483646.000000000000000000/-2.000000000000000000'
lefts[127]='-2147483646.000000000000000000'
rights[127]='-2.000000000000000000'
expressions[128]='-(-2147483646.000000000000000000/-2.000000000000000000)'
lefts[128]='-2147483646.000000000000000000'
rights[128]='-2.000000000000000000'
expressions[129]='2147483647/3'
lefts[129]='2147483647'
rights[129]='3'
expressions[130]='-(2147483647/3)'
lefts[130]='2147483647'
rights[130]='3'
expressions[131]='-2147483647/3'
lefts[131]='-2147483647'
rights[131]='3'
expressions[132]='-(-2147483647/3)'
lefts[132]='-2147483647'
rights[132]='3'
expressions[133]='2147483647/-3'
lefts[133]='2147483647'
rights[133]='-3'
expressions[134]='-(2147483647/-3)'
lefts[134]='2147483647'
rights[134]='-3'
expressions[135]='-2147483647/-3'
lefts[135]='-2147483647'
rights[135]='-3'
expressions[136]='-(-2147483647/-3)'
lefts[136]='-2147483647'
rights[136]='-3'
expressions[137]='02147483647/3'
lefts[137]='02147483647'
rights[137]='3'
expressions[138]='-(02147483647/3)'
lefts[138]='02147483647'
rights[138]='3'
expressions[139]='-02147483647/3'
lefts[139]='-02147483647'
rights[139]='3'
expressions[140]='-(-02147483647/3)'
lefts[140]='-02147483647'
rights[140]='3'
expressions[141]='02147483647/-3'
lefts[141]='02147483647'
rights[141]='-3'
expressions[142]='-(02147483647/-3)'
lefts[142]='02147483647'
rights[142]='-3'
expressions[143]='-02147483647/-3'
lefts[143]='-02147483647'
rights[143]='-3'
expressions[144]='-(-02147483647/-3)'
lefts[144]='-02147483647'
rights[144]='-3'
expressions[145]='2147483647/03'
lefts[145]='2147483647'
rights[145]='03'
expressions[146]='-(2147483647/03)'
lefts[146]='2147483647'
rights[146]='03'
expressions[147]='-2147483647/03'
lefts[147]='-2147483647'
rights[147]='03'
expressions[148]='-(-2147483647/03)'
lefts[148]='-2147483647'
rights[148]='03'
expressions[149]='2147483647/-03'
lefts[149]='2147483647'
rights[149]='-03'
expressions[150]='-(2147483647/-03)'
lefts[150]='2147483647'
rights[150]='-03'
expressions[151]='-2147483647/-03'
lefts[151]='-2147483647'
rights[151]='-03'
expressions[152]='-(-2147483647/-03)'
lefts[152]='-2147483647'
rights[152]='-03'
expressions[153]='2147483647.00/3'
lefts[153]='2147483647.00'
rights[153]='3'
expressions[154]='-(2147483647.00/3)'
lefts[154]='2147483647.00'
rights[154]='3'
expressions[155]='-2147483647.00/3'
lefts[155]='-2147483647.00'
rights[155]='3'
expressions[156]='-(-2147483647.00/3)'
lefts[156]='-2147483647.00'
rights[156]='3'
expressions[157]='2147483647.00/-3'
lefts[157]='2147483647.00'
rights[157]='-3'
expressions[158]='-(2147483647.00/-3)'
lefts[158]='2147483647.00'
rights[158]='-3'
expressions[159]='-2147483647.00/-3'
lefts[159]='-2147483647.00'
rights[159]='-3'
expressions[160]='-(-2147483647.00/-3)'
lefts[160]='-2147483647.00'
rights[160]='-3'
expressions[161]='2147483647/3.00'
lefts[161]='2147483647'
rights[161]='3.00'
expressions[162]='-(2147483647/3.00)'
lefts[162]='2147483647'
rights[162]='3.00'
expressions[163]='-2147483647/3.00'
lefts[163]='-2147483647'
rights[163]='3.00'
expressions[164]='-(-2147483647/3.00)'
lefts[164]='-2147483647'
rights[164]='3.00'
expressions[165]='2147483647/-3.00'
lefts[165]='2147483647'
rights[165]='-3.00'
expressions[166]='-(2147483647/-3.00)'
lefts[166]='2147483647'
rights[166]='-3.00'
expressions[167]='-2147483647/-3.00'
lefts[167]='-2147483647'
rights[167]='-3.00'
expressions[168]='-(-2147483647/-3.00)'
lefts[168]='-2147483647'
rights[168]='-3.00'
expressions[169]='2147483647.00/3.00'
lefts[169]='2147483647.00'
rights[169]='3.00'
expressions[170]='-(2147483647.00/3.00)'
lefts[170]='2147483647.00'
rights[170]='3.00'
expressions[171]='-2147483647.00/3.00'
lefts[171]='-2147483647.00'
rights[171]='3.00'
expressions[172]='-(-2147483647.00/3.00)'
lefts[172]='-2147483647.00'
rights[172]='3.00'
expressions[173]='2147483647.00/-3.00'
lefts[173]='2147483647.00'
rights[173]='-3.00'
expressions[174]='-(2147483647.00/-3.00)'
lefts[174]='2147483647.00'
rights[174]='-3.00'
expressions[175]='-2147483647.00/-3.00'
lefts[175]='-2147483647.00'
rights[175]='-3.00'
expressions[176]='-(-2147483647.00/-3.00)'
lefts[176]='-2147483647.00'
rights[176]='-3.00'
expressions[177]='2147483647.000000/3.00'
lefts[177]='2147483647.000000'
rights[177]='3.00'
expressions[178]='-(2147483647.000000/3.00)'
lefts[178]='2147483647.000000'
rights[178]='3.00'
expressions[179]='-2147483647.000000/3.00'
lefts[179]='-2147483647.000000'
rights[179]='3.00'
expressions[180]='-(-2147483647.000000/3.00)'
lefts[180]='-2147483647.000000'
rights[180]='3.00'
expressions[181]='2147483647.000000/-3.00'
lefts[181]='2147483647.000000'
rights[181]='-3.00'
expressions[182]='-(2147483647.000000/-3.00)'
lefts[182]='2147483647.000000'
rights[182]='-3.00'
expressions[183]='-2147483647.000000/-3.00'
lefts[183]='-2147483647.000000'
rights[183]='-3.00'
expressions[184]='-(-2147483647.000000/-3.00)'
lefts[184]='-2147483647.000000'
rights[184]='-3.00'
expressions[185]='2147483647.000000000000000000/3.000000000000000000'
lefts[185]='2147483647.000000000000000000'
rights[185]='3.000000000000000000'
expressions[186]='-(2147483647.000000000000000000/3.000000000000000000)'
lefts[186]='2147483647.000000000000000000'
rights[186]='3.000000000000000000'
expressions[187]='-2147483647.000000000000000000/3.000000000000000000'
lefts[187]='-2147483647.000000000000000000'
rights[187]='3.000000000000000000'
expressions[188]='-(-2147483647.000000000000000000/3.000000000000000000)'
lefts[188]='-2147483647.000000000000000000'
rights[188]='3.000000000000000000'
expressions[189]='2147483647.000000000000000000/-3.000000000000000000'
lefts[189]='2147483647.000000000000000000'
rights[189]='-3.000000000000000000'
expressions[190]='-(2147483647.000000000000000000/-3.000000000000000000)'
lefts[190]='2147483647.000000000000000000'
rights[190]='-3.000000000000000000'
expressions[191]='-2147483647.000000000000000000/-3.000000000000000000'
lefts[191]='-2147483647.000000000000000000'
rights[191]='-3.000000000000000000'
expressions[192]='-(-2147483647.000000000000000000/-3.000000000000000000)'
lefts[192]='-2147483647.000000000000000000'
rights[192]='-3.000000000000000000'
expressions[193]='9999999999/1'
lefts[193]='9999999999'
rights[193]='1'
expressions[194]='-(9999999999/1)'
lefts[194]='9999999999'
rights[194]='1'
expressions[195]='-9999999999/1'
lefts[195]='-9999999999'
rights[195]='1'
expressions[196]='-(-9999999999/1)'
lefts[196]='-9999999999'
rights[196]='1'
expressions[197]='9999999999/-1'
lefts[197]='9999999999'
rights[197]='-1'
expressions[198]='-(9999999999/-1)'
lefts[198]='9999999999'
rights[198]='-1'
expressions[199]='-9999999999/-1'
lefts[199]='-9999999999'
rights[199]='-1'
expressions[200]='-(-9999999999/-1)'
lefts[200]='-9999999999'
rights[200]='-1'
expressions[201]='09999999999/1'
lefts[201]='09999999999'
rights[201]='1'
expressions[202]='-(09999999999/1)'
lefts[202]='09999999999'
rights[202]='1'
expressions[203]='-09999999999/1'
lefts[203]='-09999999999'
rights[203]='1'
expressions[204]='-(-09999999999/1)'
lefts[204]='-09999999999'
rights[204]='1'
expressions[205]='09999999999/-1'
lefts[205]='09999999999'
rights[205]='-1'
expressions[206]='-(09999999999/-1)'
lefts[206]='09999999999'
rights[206]='-1'
expressions[207]='-09999999999/-1'
lefts[207]='-09999999999'
rights[207]='-1'
expressions[208]='-(-09999999999/-1)'
lefts[208]='-09999999999'
rights[208]='-1'
expressions[209]='9999999999/01'
lefts[209]='9999999999'
rights[209]='01'
expressions[210]='-(9999999999/01)'
lefts[210]='9999999999'
rights[210]='01'
expressions[211]='-9999999999/01'
lefts[211]='-9999999999'
rights[211]='01'
expressions[212]='-(-9999999999/01)'
lefts[212]='-9999999999'
rights[212]='01'
expressions[213]='9999999999/-01'
lefts[213]='9999999999'
rights[213]='-01'
expressions[214]='-(9999999999/-01)'
lefts[214]='9999999999'
rights[214]='-01'
expressions[215]='-9999999999/-01'
lefts[215]='-9999999999'
rights[215]='-01'
expressions[216]='-(-9999999999/-01)'
lefts[216]='-9999999999'
rights[216]='-01'
expressions[217]='9999999999.00/1'
lefts[217]='9999999999.00'
rights[217]='1'
expressions[218]='-(9999999999.00/1)'
lefts[218]='9999999999.00'
rights[218]='1'
expressions[219]='-9999999999.00/1'
lefts[219]='-9999999999.00'
rights[219]='1'
expressions[220]='-(-9999999999.00/1)'
lefts[220]='-9999999999.00'
rights[220]='1'
expressions[221]='9999999999.00/-1'
lefts[221]='9999999999.00'
rights[221]='-1'
expressions[222]='-(9999999999.00/-1)'
lefts[222]='9999999999.00'
rights[222]='-1'
expressions[223]='-9999999999.00/-1'
lefts[223]='-9999999999.00'
rights[223]='-1'
expressions[224]='-(-9999999999.00/-1)'
lefts[224]='-9999999999.00'
rights[224]='-1'
expressions[225]='9999999999/1.00'
lefts[225]='9999999999'
rights[225]='1.00'
expressions[226]='-(9999999999/1.00)'
lefts[226]='9999999999'
rights[226]='1.00'
expressions[227]='-9999999999/1.00'
lefts[227]='-9999999999'
rights[227]='1.00'
expressions[228]='-(-9999999999/1.00)'
lefts[228]='-9999999999'
rights[228]='1.00'
expressions[229]='9999999999/-1.00'
lefts[229]='9999999999'
rights[229]='-1.00'
expressions[230]='-(9999999999/-1.00)'
lefts[230]='9999999999'
rights[230]='-1.00'
expressions[231]='-9999999999/-1.00'
lefts[231]='-9999999999'
rights[231]='-1.00'
expressions[232]='-(-9999999999/-1.00)'
lefts[232]='-9999999999'
rights[232]='-1.00'
expressions[233]='9999999999.00/1.00'
lefts[233]='9999999999.00'
rights[233]='1.00'
expressions[234]='-(9999999999.00/1.00)'
lefts[234]='9999999999.00'
rights[234]='1.00'
expressions[235]='-9999999999.00/1.00'
lefts[235]='-9999999999.00'
rights[235]='1.00'
expressions[236]='-(-9999999999.00/1.00)'
lefts[236]='-9999999999.00'
rights[236]='1.00'
expressions[237]='9999999999.00/-1.00'
lefts[237]='9999999999.00'
rights[237]='-1.00'
expressions[238]='-(9999999999.00/-1.00)'
lefts[238]='9999999999.00'
rights[238]='-1.00'
expressions[239]='-9999999999.00/-1.00'
lefts[239]='-9999999999.00'
rights[239]='-1.00'
expressions[240]='-(-9999999999.00/-1.00)'
lefts[240]='-9999999999.00'
rights[240]='-1.00'
expressions[241]='9999999999.000000/1.00'
lefts[241]='9999999999.000000'
rights[241]='1.00'
expressions[242]='-(9999999999.000000/1.00)'
lefts[242]='9999999999.000000'
rights[242]='1.00'
expressions[243]='-9999999999.000000/1.00'
lefts[243]='-9999999999.000000'
rights[243]='1.00'
expressions[244]='-(-9999999999.000000/1.00)'
lefts[244]='-9999999999.000000'
rights[244]='1.00'
expressions[245]='9999999999.000000/-1.00'
lefts[245]='9999999999.000000'
rights[245]='-1.00'
expressions[246]='-(9999999999.000000/-1.00)'
lefts[246]='9999999999.000000'
rights[246]='-1.00'
expressions[247]='-9999999999.000000/-1.00'
lefts[247]='-9999999999.000000'
rights[247]='-1.00'
expressions[248]='-(-9999999999.000000/-1.00)'
lefts[248]='-9999999999.000000'
rights[248]='-1.00'
expressions[249]='9999999999.000000000000000000/1.000000000000000000'
lefts[249]='9999999999.000000000000000000'
rights[249]='1.000000000000000000'
expressions[250]='-(9999999999.000000000000000000/1.000000000000000000)'
lefts[250]='9999999999.000000000000000000'
rights[250]='1.000000000000000000'
expressions[251]='-9999999999.000000000000000000/1.000000000000000000'
lefts[251]='-9999999999.000000000000000000'
rights[251]='1.000000000000000000'
expressions[252]='-(-9999999999.000000000000000000/1.000000000000000000)'
lefts[252]='-9999999999.000000000000000000'
rights[252]='1.000000000000000000'
expressions[253]='9999999999.000000000000000000/-1.000000000000000000'
lefts[253]='9999999999.000000000000000000'
rights[253]='-1.000000000000000000'
expressions[254]='-(9999999999.000000000000000000/-1.000000000000000000)'
lefts[254]='9999999999.000000000000000000'
rights[254]='-1.000000000000000000'
expressions[255]='-9999999999.000000000000000000/-1.000000000000000000'
lefts[255]='-9999999999.000000000000000000'
rights[255]='-1.000000000000000000'
expressions[256]='-(-9999999999.000000000000000000/-1.000000000000000000)'
lefts[256]='-9999999999.000000000000000000'
rights[256]='-1.000000000000000000'
expressions[257]='1/100'
lefts[257]='1'
rights[257]='100'
expressions[258]='-(1/100)'
lefts[258]='1'
rights[258]='100'
expressions[259]='-1/100'
lefts[259]='-1'
rights[259]='100'
expressions[260]='-(-1/100)'
lefts[260]='-1'
rights[260]='100'
expressions[261]='1/-100'
lefts[261]='1'
rights[261]='-100'
expressions[262]='-(1/-100)'
lefts[262]='1'
rights[262]='-100'
expressions[263]='-1/-100'
lefts[263]='-1'
rights[263]='-100'
expressions[264]='-(-1/-100)'
lefts[264]='-1'
rights[264]='-100'
expressions[265]='01/100'
lefts[265]='01'
rights[265]='100'
expressions[266]='-(01/100)'
lefts[266]='01'
rights[266]='100'
expressions[267]='-01/100'
lefts[267]='-01'
rights[267]='100'
expressions[268]='-(-01/100)'
lefts[268]='-01'
rights[268]='100'
expressions[269]='01/-100'
lefts[269]='01'
rights[269]='-100'
expressions[270]='-(01/-100)'
lefts[270]='01'
rights[270]='-100'
expressions[271]='-01/-100'
lefts[271]='-01'
rights[271]='-100'
expressions[272]='-(-01/-100)'
lefts[272]='-01'
rights[272]='-100'
expressions[273]='1/0100'
lefts[273]='1'
rights[273]='0100'
expressions[274]='-(1/0100)'
lefts[274]='1'
rights[274]='0100'
expressions[275]='-1/0100'
lefts[275]='-1'
rights[275]='0100'
expressions[276]='-(-1/0100)'
lefts[276]='-1'
rights[276]='0100'
expressions[277]='1/-0100'
lefts[277]='1'
rights[277]='-0100'
expressions[278]='-(1/-0100)'
lefts[278]='1'
rights[278]='-0100'
expressions[279]='-1/-0100'
lefts[279]='-1'
rights[279]='-0100'
expressions[280]='-(-1/-0100)'
lefts[280]='-1'
rights[280]='-0100'
expressions[281]='1.00/100'
lefts[281]='1.00'
rights[281]='100'
expressions[282]='-(1.00/100)'
lefts[282]='1.00'
rights[282]='100'
expressions[283]='-1.00/100'
lefts[283]='-1.00'
rights[283]='100'
expressions[284]='-(-1.00/100)'
lefts[284]='-1.00'
rights[284]='100'
expressions[285]='1.00/-100'
lefts[285]='1.00'
rights[285]='-100'
expressions[286]='-(1.00/-100)'
lefts[286]='1.00'
rights[286]='-100'
expressions[287]='-1.00/-100'
lefts[287]='-1.00'
rights[287]='-100'
expressions[288]='-(-1.00/-100)'
lefts[288]='-1.00'
rights[288]='-100'
expressions[289]='1/100.00'
lefts[289]='1'
rights[289]='100.00'
expressions[290]='-(1/100.00)'
lefts[290]='1'
rights[290]='100.00'
expressions[291]='-1/100.00'
lefts[291]='-1'
rights[291]='100.00'
expressions[292]='-(-1/100.00)'
lefts[292]='-1'
rights[292]='100.00'
expressions[293]='1/-100.00'
lefts[293]='1'
rights[293]='-100.00'
expressions[294]='-(1/-100.00)'
lefts[294]='1'
rights[294]='-100.00'
expressions[295]='-1/-100.00'
lefts[295]='-1'
rights[295]='-100.00'
expressions[296]='-(-1/-100.00)'
lefts[296]='-1'
rights[296]='-100.00'
expressions[297]='1.00/100.00'
lefts[297]='1.00'
rights[297]='100.00'
expressions[298]='-(1.00/100.00)'
lefts[298]='1.00'
rights[298]='100.00'
expressions[299]='-1.00/100.00'
lefts[299]='-1.00'
rights[299]='100.00'
expressions[300]='-(-1.00/100.00)'
lefts[300]='-1.00'
rights[300]='100.00'
expressions[301]='1.00/-100.00'
lefts[301]='1.00'
rights[301]='-100.00'
expressions[302]='-(1.00/-100.00)'
lefts[302]='1.00'
rights[302]='-100.00'
expressions[303]='-1.00/-100.00'
lefts[303]='-1.00'
rights[303]='-100.00'
expressions[304]='-(-1.00/-100.00)'
lefts[304]='-1.00'
rights[304]='-100.00'
expressions[305]='1.000000/100.00'
lefts[305]='1.000000'
rights[305]='100.00'
expressions[306]='-(1.000000/100.00)'
lefts[306]='1.000000'
rights[306]='100.00'
expressions[307]='-1.000000/100.00'
lefts[307]='-1.000000'
rights[307]='100.00'
expressions[308]='-(-1.000000/100.00)'
lefts[308]='-1.000000'
rights[308]='100.00'
expressions[309]='1.000000/-100.00'
lefts[309]='1.000000'
rights[309]='-100.00'
expressions[310]='-(1.000000/-100.00)'
lefts[310]='1.000000'
rights[310]='-100.00'
expressions[311]='-1.000000/-100.00'
lefts[311]='-1.000000'
rights[311]='-100.00'
expressions[312]='-(-1.000000/-100.00)'
lefts[312]='-1.000000'
rights[312]='-100.00'
expressions[313]='1.000000000000000000/100.000000000000000000'
lefts[313]='1.000000000000000000'
rights[313]='100.000000000000000000'
expressions[314]='-(1.000000000000000000/100.000000000000000000)'
lefts[314]='1.000000000000000000'
rights[314]='100.000000000000000000'
expressions[315]='-1.000000000000000000/100.000000000000000000'
lefts[315]='-1.000000000000000000'
rights[315]='100.000000000000000000'
expressions[316]='-(-1.000000000000000000/100.000000000000000000)'
lefts[316]='-1.000000000000000000'
rights[316]='100.000000000000000000'
expressions[317]='1.000000000000000000/-100.000000000000000000'
lefts[317]='1.000000000000000000'
rights[317]='-100.000000000000000000'
expressions[318]='-(1.000000000000000000/-100.000000000000000000)'
lefts[318]='1.000000000000000000'
rights[318]='-100.000000000000000000'
expressions[319]='-1.000000000000000000/-100.000000000000000000'
lefts[319]='-1.000000000000000000'
rights[319]='-100.000000000000000000'
expressions[320]='-(-1.000000000000000000/-100.000000000000000000)'
lefts[320]='-1.000000000000000000'
rights[320]='-100.000000000000000000'
settings[1]=0
settings[2]=2
settings[3]=6
settings[4]=14
settings[5]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught
outputPath='Z:\home\rich\.codex\worktrees\numeric-constant-division-5611\Project-Copperfin\build\constant-division-native\constant-payload-b-results.txt'
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
  RETURN CFNMETA(15/200)+'|'+HexBytes(BINTOC(15/200,'BRS'))+'|'+HexBytes(TRANSFORM(15/200))+'|'+CFNMETA(15)+'|'+CFNMETA(200)
 CASE index=2
  RETURN CFNMETA(-(15/200))+'|'+HexBytes(BINTOC(-(15/200),'BRS'))+'|'+HexBytes(TRANSFORM(-(15/200)))+'|'+CFNMETA(15)+'|'+CFNMETA(200)
 CASE index=3
  RETURN CFNMETA(-15/200)+'|'+HexBytes(BINTOC(-15/200,'BRS'))+'|'+HexBytes(TRANSFORM(-15/200))+'|'+CFNMETA(-15)+'|'+CFNMETA(200)
 CASE index=4
  RETURN CFNMETA(-(-15/200))+'|'+HexBytes(BINTOC(-(-15/200),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15/200)))+'|'+CFNMETA(-15)+'|'+CFNMETA(200)
 CASE index=5
  RETURN CFNMETA(15/-200)+'|'+HexBytes(BINTOC(15/-200,'BRS'))+'|'+HexBytes(TRANSFORM(15/-200))+'|'+CFNMETA(15)+'|'+CFNMETA(-200)
 CASE index=6
  RETURN CFNMETA(-(15/-200))+'|'+HexBytes(BINTOC(-(15/-200),'BRS'))+'|'+HexBytes(TRANSFORM(-(15/-200)))+'|'+CFNMETA(15)+'|'+CFNMETA(-200)
 CASE index=7
  RETURN CFNMETA(-15/-200)+'|'+HexBytes(BINTOC(-15/-200,'BRS'))+'|'+HexBytes(TRANSFORM(-15/-200))+'|'+CFNMETA(-15)+'|'+CFNMETA(-200)
 CASE index=8
  RETURN CFNMETA(-(-15/-200))+'|'+HexBytes(BINTOC(-(-15/-200),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15/-200)))+'|'+CFNMETA(-15)+'|'+CFNMETA(-200)
 CASE index=9
  RETURN CFNMETA(015/200)+'|'+HexBytes(BINTOC(015/200,'BRS'))+'|'+HexBytes(TRANSFORM(015/200))+'|'+CFNMETA(015)+'|'+CFNMETA(200)
 CASE index=10
  RETURN CFNMETA(-(015/200))+'|'+HexBytes(BINTOC(-(015/200),'BRS'))+'|'+HexBytes(TRANSFORM(-(015/200)))+'|'+CFNMETA(015)+'|'+CFNMETA(200)
 CASE index=11
  RETURN CFNMETA(-015/200)+'|'+HexBytes(BINTOC(-015/200,'BRS'))+'|'+HexBytes(TRANSFORM(-015/200))+'|'+CFNMETA(-015)+'|'+CFNMETA(200)
 CASE index=12
  RETURN CFNMETA(-(-015/200))+'|'+HexBytes(BINTOC(-(-015/200),'BRS'))+'|'+HexBytes(TRANSFORM(-(-015/200)))+'|'+CFNMETA(-015)+'|'+CFNMETA(200)
 CASE index=13
  RETURN CFNMETA(015/-200)+'|'+HexBytes(BINTOC(015/-200,'BRS'))+'|'+HexBytes(TRANSFORM(015/-200))+'|'+CFNMETA(015)+'|'+CFNMETA(-200)
 CASE index=14
  RETURN CFNMETA(-(015/-200))+'|'+HexBytes(BINTOC(-(015/-200),'BRS'))+'|'+HexBytes(TRANSFORM(-(015/-200)))+'|'+CFNMETA(015)+'|'+CFNMETA(-200)
 CASE index=15
  RETURN CFNMETA(-015/-200)+'|'+HexBytes(BINTOC(-015/-200,'BRS'))+'|'+HexBytes(TRANSFORM(-015/-200))+'|'+CFNMETA(-015)+'|'+CFNMETA(-200)
 CASE index=16
  RETURN CFNMETA(-(-015/-200))+'|'+HexBytes(BINTOC(-(-015/-200),'BRS'))+'|'+HexBytes(TRANSFORM(-(-015/-200)))+'|'+CFNMETA(-015)+'|'+CFNMETA(-200)
 CASE index=17
  RETURN CFNMETA(15/0200)+'|'+HexBytes(BINTOC(15/0200,'BRS'))+'|'+HexBytes(TRANSFORM(15/0200))+'|'+CFNMETA(15)+'|'+CFNMETA(0200)
 CASE index=18
  RETURN CFNMETA(-(15/0200))+'|'+HexBytes(BINTOC(-(15/0200),'BRS'))+'|'+HexBytes(TRANSFORM(-(15/0200)))+'|'+CFNMETA(15)+'|'+CFNMETA(0200)
 CASE index=19
  RETURN CFNMETA(-15/0200)+'|'+HexBytes(BINTOC(-15/0200,'BRS'))+'|'+HexBytes(TRANSFORM(-15/0200))+'|'+CFNMETA(-15)+'|'+CFNMETA(0200)
 CASE index=20
  RETURN CFNMETA(-(-15/0200))+'|'+HexBytes(BINTOC(-(-15/0200),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15/0200)))+'|'+CFNMETA(-15)+'|'+CFNMETA(0200)
 CASE index=21
  RETURN CFNMETA(15/-0200)+'|'+HexBytes(BINTOC(15/-0200,'BRS'))+'|'+HexBytes(TRANSFORM(15/-0200))+'|'+CFNMETA(15)+'|'+CFNMETA(-0200)
 CASE index=22
  RETURN CFNMETA(-(15/-0200))+'|'+HexBytes(BINTOC(-(15/-0200),'BRS'))+'|'+HexBytes(TRANSFORM(-(15/-0200)))+'|'+CFNMETA(15)+'|'+CFNMETA(-0200)
 CASE index=23
  RETURN CFNMETA(-15/-0200)+'|'+HexBytes(BINTOC(-15/-0200,'BRS'))+'|'+HexBytes(TRANSFORM(-15/-0200))+'|'+CFNMETA(-15)+'|'+CFNMETA(-0200)
 CASE index=24
  RETURN CFNMETA(-(-15/-0200))+'|'+HexBytes(BINTOC(-(-15/-0200),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15/-0200)))+'|'+CFNMETA(-15)+'|'+CFNMETA(-0200)
 CASE index=25
  RETURN CFNMETA(15.00/200)+'|'+HexBytes(BINTOC(15.00/200,'BRS'))+'|'+HexBytes(TRANSFORM(15.00/200))+'|'+CFNMETA(15.00)+'|'+CFNMETA(200)
 CASE index=26
  RETURN CFNMETA(-(15.00/200))+'|'+HexBytes(BINTOC(-(15.00/200),'BRS'))+'|'+HexBytes(TRANSFORM(-(15.00/200)))+'|'+CFNMETA(15.00)+'|'+CFNMETA(200)
 CASE index=27
  RETURN CFNMETA(-15.00/200)+'|'+HexBytes(BINTOC(-15.00/200,'BRS'))+'|'+HexBytes(TRANSFORM(-15.00/200))+'|'+CFNMETA(-15.00)+'|'+CFNMETA(200)
 CASE index=28
  RETURN CFNMETA(-(-15.00/200))+'|'+HexBytes(BINTOC(-(-15.00/200),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15.00/200)))+'|'+CFNMETA(-15.00)+'|'+CFNMETA(200)
 CASE index=29
  RETURN CFNMETA(15.00/-200)+'|'+HexBytes(BINTOC(15.00/-200,'BRS'))+'|'+HexBytes(TRANSFORM(15.00/-200))+'|'+CFNMETA(15.00)+'|'+CFNMETA(-200)
 CASE index=30
  RETURN CFNMETA(-(15.00/-200))+'|'+HexBytes(BINTOC(-(15.00/-200),'BRS'))+'|'+HexBytes(TRANSFORM(-(15.00/-200)))+'|'+CFNMETA(15.00)+'|'+CFNMETA(-200)
 CASE index=31
  RETURN CFNMETA(-15.00/-200)+'|'+HexBytes(BINTOC(-15.00/-200,'BRS'))+'|'+HexBytes(TRANSFORM(-15.00/-200))+'|'+CFNMETA(-15.00)+'|'+CFNMETA(-200)
 CASE index=32
  RETURN CFNMETA(-(-15.00/-200))+'|'+HexBytes(BINTOC(-(-15.00/-200),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15.00/-200)))+'|'+CFNMETA(-15.00)+'|'+CFNMETA(-200)
 CASE index=33
  RETURN CFNMETA(15/200.00)+'|'+HexBytes(BINTOC(15/200.00,'BRS'))+'|'+HexBytes(TRANSFORM(15/200.00))+'|'+CFNMETA(15)+'|'+CFNMETA(200.00)
 CASE index=34
  RETURN CFNMETA(-(15/200.00))+'|'+HexBytes(BINTOC(-(15/200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(15/200.00)))+'|'+CFNMETA(15)+'|'+CFNMETA(200.00)
 CASE index=35
  RETURN CFNMETA(-15/200.00)+'|'+HexBytes(BINTOC(-15/200.00,'BRS'))+'|'+HexBytes(TRANSFORM(-15/200.00))+'|'+CFNMETA(-15)+'|'+CFNMETA(200.00)
 CASE index=36
  RETURN CFNMETA(-(-15/200.00))+'|'+HexBytes(BINTOC(-(-15/200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15/200.00)))+'|'+CFNMETA(-15)+'|'+CFNMETA(200.00)
 CASE index=37
  RETURN CFNMETA(15/-200.00)+'|'+HexBytes(BINTOC(15/-200.00,'BRS'))+'|'+HexBytes(TRANSFORM(15/-200.00))+'|'+CFNMETA(15)+'|'+CFNMETA(-200.00)
 CASE index=38
  RETURN CFNMETA(-(15/-200.00))+'|'+HexBytes(BINTOC(-(15/-200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(15/-200.00)))+'|'+CFNMETA(15)+'|'+CFNMETA(-200.00)
 CASE index=39
  RETURN CFNMETA(-15/-200.00)+'|'+HexBytes(BINTOC(-15/-200.00,'BRS'))+'|'+HexBytes(TRANSFORM(-15/-200.00))+'|'+CFNMETA(-15)+'|'+CFNMETA(-200.00)
 CASE index=40
  RETURN CFNMETA(-(-15/-200.00))+'|'+HexBytes(BINTOC(-(-15/-200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15/-200.00)))+'|'+CFNMETA(-15)+'|'+CFNMETA(-200.00)
 CASE index=41
  RETURN CFNMETA(15.00/200.00)+'|'+HexBytes(BINTOC(15.00/200.00,'BRS'))+'|'+HexBytes(TRANSFORM(15.00/200.00))+'|'+CFNMETA(15.00)+'|'+CFNMETA(200.00)
 CASE index=42
  RETURN CFNMETA(-(15.00/200.00))+'|'+HexBytes(BINTOC(-(15.00/200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(15.00/200.00)))+'|'+CFNMETA(15.00)+'|'+CFNMETA(200.00)
 CASE index=43
  RETURN CFNMETA(-15.00/200.00)+'|'+HexBytes(BINTOC(-15.00/200.00,'BRS'))+'|'+HexBytes(TRANSFORM(-15.00/200.00))+'|'+CFNMETA(-15.00)+'|'+CFNMETA(200.00)
 CASE index=44
  RETURN CFNMETA(-(-15.00/200.00))+'|'+HexBytes(BINTOC(-(-15.00/200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15.00/200.00)))+'|'+CFNMETA(-15.00)+'|'+CFNMETA(200.00)
 CASE index=45
  RETURN CFNMETA(15.00/-200.00)+'|'+HexBytes(BINTOC(15.00/-200.00,'BRS'))+'|'+HexBytes(TRANSFORM(15.00/-200.00))+'|'+CFNMETA(15.00)+'|'+CFNMETA(-200.00)
 CASE index=46
  RETURN CFNMETA(-(15.00/-200.00))+'|'+HexBytes(BINTOC(-(15.00/-200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(15.00/-200.00)))+'|'+CFNMETA(15.00)+'|'+CFNMETA(-200.00)
 CASE index=47
  RETURN CFNMETA(-15.00/-200.00)+'|'+HexBytes(BINTOC(-15.00/-200.00,'BRS'))+'|'+HexBytes(TRANSFORM(-15.00/-200.00))+'|'+CFNMETA(-15.00)+'|'+CFNMETA(-200.00)
 CASE index=48
  RETURN CFNMETA(-(-15.00/-200.00))+'|'+HexBytes(BINTOC(-(-15.00/-200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15.00/-200.00)))+'|'+CFNMETA(-15.00)+'|'+CFNMETA(-200.00)
 CASE index=49
  RETURN CFNMETA(15.000000/200.00)+'|'+HexBytes(BINTOC(15.000000/200.00,'BRS'))+'|'+HexBytes(TRANSFORM(15.000000/200.00))+'|'+CFNMETA(15.000000)+'|'+CFNMETA(200.00)
 CASE index=50
  RETURN CFNMETA(-(15.000000/200.00))+'|'+HexBytes(BINTOC(-(15.000000/200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(15.000000/200.00)))+'|'+CFNMETA(15.000000)+'|'+CFNMETA(200.00)
 CASE index=51
  RETURN CFNMETA(-15.000000/200.00)+'|'+HexBytes(BINTOC(-15.000000/200.00,'BRS'))+'|'+HexBytes(TRANSFORM(-15.000000/200.00))+'|'+CFNMETA(-15.000000)+'|'+CFNMETA(200.00)
 CASE index=52
  RETURN CFNMETA(-(-15.000000/200.00))+'|'+HexBytes(BINTOC(-(-15.000000/200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15.000000/200.00)))+'|'+CFNMETA(-15.000000)+'|'+CFNMETA(200.00)
 CASE index=53
  RETURN CFNMETA(15.000000/-200.00)+'|'+HexBytes(BINTOC(15.000000/-200.00,'BRS'))+'|'+HexBytes(TRANSFORM(15.000000/-200.00))+'|'+CFNMETA(15.000000)+'|'+CFNMETA(-200.00)
 CASE index=54
  RETURN CFNMETA(-(15.000000/-200.00))+'|'+HexBytes(BINTOC(-(15.000000/-200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(15.000000/-200.00)))+'|'+CFNMETA(15.000000)+'|'+CFNMETA(-200.00)
 CASE index=55
  RETURN CFNMETA(-15.000000/-200.00)+'|'+HexBytes(BINTOC(-15.000000/-200.00,'BRS'))+'|'+HexBytes(TRANSFORM(-15.000000/-200.00))+'|'+CFNMETA(-15.000000)+'|'+CFNMETA(-200.00)
 CASE index=56
  RETURN CFNMETA(-(-15.000000/-200.00))+'|'+HexBytes(BINTOC(-(-15.000000/-200.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15.000000/-200.00)))+'|'+CFNMETA(-15.000000)+'|'+CFNMETA(-200.00)
 CASE index=57
  RETURN CFNMETA(15.000000000000000000/200.000000000000000000)+'|'+HexBytes(BINTOC(15.000000000000000000/200.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(15.000000000000000000/200.000000000000000000))+'|'+CFNMETA(15.000000000000000000)+'|'+CFNMETA(200.000000000000000000)
 CASE index=58
  RETURN CFNMETA(-(15.000000000000000000/200.000000000000000000))+'|'+HexBytes(BINTOC(-(15.000000000000000000/200.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(15.000000000000000000/200.000000000000000000)))+'|'+CFNMETA(15.000000000000000000)+'|'+CFNMETA(200.000000000000000000)
 CASE index=59
  RETURN CFNMETA(-15.000000000000000000/200.000000000000000000)+'|'+HexBytes(BINTOC(-15.000000000000000000/200.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-15.000000000000000000/200.000000000000000000))+'|'+CFNMETA(-15.000000000000000000)+'|'+CFNMETA(200.000000000000000000)
 CASE index=60
  RETURN CFNMETA(-(-15.000000000000000000/200.000000000000000000))+'|'+HexBytes(BINTOC(-(-15.000000000000000000/200.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15.000000000000000000/200.000000000000000000)))+'|'+CFNMETA(-15.000000000000000000)+'|'+CFNMETA(200.000000000000000000)
 CASE index=61
  RETURN CFNMETA(15.000000000000000000/-200.000000000000000000)+'|'+HexBytes(BINTOC(15.000000000000000000/-200.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(15.000000000000000000/-200.000000000000000000))+'|'+CFNMETA(15.000000000000000000)+'|'+CFNMETA(-200.000000000000000000)
 CASE index=62
  RETURN CFNMETA(-(15.000000000000000000/-200.000000000000000000))+'|'+HexBytes(BINTOC(-(15.000000000000000000/-200.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(15.000000000000000000/-200.000000000000000000)))+'|'+CFNMETA(15.000000000000000000)+'|'+CFNMETA(-200.000000000000000000)
 CASE index=63
  RETURN CFNMETA(-15.000000000000000000/-200.000000000000000000)+'|'+HexBytes(BINTOC(-15.000000000000000000/-200.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-15.000000000000000000/-200.000000000000000000))+'|'+CFNMETA(-15.000000000000000000)+'|'+CFNMETA(-200.000000000000000000)
 CASE index=64
  RETURN CFNMETA(-(-15.000000000000000000/-200.000000000000000000))+'|'+HexBytes(BINTOC(-(-15.000000000000000000/-200.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-15.000000000000000000/-200.000000000000000000)))+'|'+CFNMETA(-15.000000000000000000)+'|'+CFNMETA(-200.000000000000000000)
 CASE index=65
  RETURN CFNMETA(2147483646/2)+'|'+HexBytes(BINTOC(2147483646/2,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646/2))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(2)
 CASE index=66
  RETURN CFNMETA(-(2147483646/2))+'|'+HexBytes(BINTOC(-(2147483646/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646/2)))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(2)
 CASE index=67
  RETURN CFNMETA(-2147483646/2)+'|'+HexBytes(BINTOC(-2147483646/2,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646/2))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(2)
 CASE index=68
  RETURN CFNMETA(-(-2147483646/2))+'|'+HexBytes(BINTOC(-(-2147483646/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646/2)))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(2)
 CASE index=69
  RETURN CFNMETA(2147483646/-2)+'|'+HexBytes(BINTOC(2147483646/-2,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646/-2))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(-2)
 CASE index=70
  RETURN CFNMETA(-(2147483646/-2))+'|'+HexBytes(BINTOC(-(2147483646/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646/-2)))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(-2)
 CASE index=71
  RETURN CFNMETA(-2147483646/-2)+'|'+HexBytes(BINTOC(-2147483646/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646/-2))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(-2)
 CASE index=72
  RETURN CFNMETA(-(-2147483646/-2))+'|'+HexBytes(BINTOC(-(-2147483646/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646/-2)))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(-2)
 CASE index=73
  RETURN CFNMETA(02147483646/2)+'|'+HexBytes(BINTOC(02147483646/2,'BRS'))+'|'+HexBytes(TRANSFORM(02147483646/2))+'|'+CFNMETA(02147483646)+'|'+CFNMETA(2)
 CASE index=74
  RETURN CFNMETA(-(02147483646/2))+'|'+HexBytes(BINTOC(-(02147483646/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(02147483646/2)))+'|'+CFNMETA(02147483646)+'|'+CFNMETA(2)
 CASE index=75
  RETURN CFNMETA(-02147483646/2)+'|'+HexBytes(BINTOC(-02147483646/2,'BRS'))+'|'+HexBytes(TRANSFORM(-02147483646/2))+'|'+CFNMETA(-02147483646)+'|'+CFNMETA(2)
 CASE index=76
  RETURN CFNMETA(-(-02147483646/2))+'|'+HexBytes(BINTOC(-(-02147483646/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-02147483646/2)))+'|'+CFNMETA(-02147483646)+'|'+CFNMETA(2)
 CASE index=77
  RETURN CFNMETA(02147483646/-2)+'|'+HexBytes(BINTOC(02147483646/-2,'BRS'))+'|'+HexBytes(TRANSFORM(02147483646/-2))+'|'+CFNMETA(02147483646)+'|'+CFNMETA(-2)
 CASE index=78
  RETURN CFNMETA(-(02147483646/-2))+'|'+HexBytes(BINTOC(-(02147483646/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(02147483646/-2)))+'|'+CFNMETA(02147483646)+'|'+CFNMETA(-2)
 CASE index=79
  RETURN CFNMETA(-02147483646/-2)+'|'+HexBytes(BINTOC(-02147483646/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-02147483646/-2))+'|'+CFNMETA(-02147483646)+'|'+CFNMETA(-2)
 CASE index=80
  RETURN CFNMETA(-(-02147483646/-2))+'|'+HexBytes(BINTOC(-(-02147483646/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-02147483646/-2)))+'|'+CFNMETA(-02147483646)+'|'+CFNMETA(-2)
 CASE index=81
  RETURN CFNMETA(2147483646/02)+'|'+HexBytes(BINTOC(2147483646/02,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646/02))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(02)
 CASE index=82
  RETURN CFNMETA(-(2147483646/02))+'|'+HexBytes(BINTOC(-(2147483646/02),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646/02)))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(02)
 CASE index=83
  RETURN CFNMETA(-2147483646/02)+'|'+HexBytes(BINTOC(-2147483646/02,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646/02))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(02)
 CASE index=84
  RETURN CFNMETA(-(-2147483646/02))+'|'+HexBytes(BINTOC(-(-2147483646/02),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646/02)))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(02)
 CASE index=85
  RETURN CFNMETA(2147483646/-02)+'|'+HexBytes(BINTOC(2147483646/-02,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646/-02))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(-02)
 CASE index=86
  RETURN CFNMETA(-(2147483646/-02))+'|'+HexBytes(BINTOC(-(2147483646/-02),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646/-02)))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(-02)
 CASE index=87
  RETURN CFNMETA(-2147483646/-02)+'|'+HexBytes(BINTOC(-2147483646/-02,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646/-02))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(-02)
 CASE index=88
  RETURN CFNMETA(-(-2147483646/-02))+'|'+HexBytes(BINTOC(-(-2147483646/-02),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646/-02)))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(-02)
 CASE index=89
  RETURN CFNMETA(2147483646.00/2)+'|'+HexBytes(BINTOC(2147483646.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646.00/2))+'|'+CFNMETA(2147483646.00)+'|'+CFNMETA(2)
 CASE index=90
  RETURN CFNMETA(-(2147483646.00/2))+'|'+HexBytes(BINTOC(-(2147483646.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646.00/2)))+'|'+CFNMETA(2147483646.00)+'|'+CFNMETA(2)
 CASE index=91
  RETURN CFNMETA(-2147483646.00/2)+'|'+HexBytes(BINTOC(-2147483646.00/2,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646.00/2))+'|'+CFNMETA(-2147483646.00)+'|'+CFNMETA(2)
 CASE index=92
  RETURN CFNMETA(-(-2147483646.00/2))+'|'+HexBytes(BINTOC(-(-2147483646.00/2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646.00/2)))+'|'+CFNMETA(-2147483646.00)+'|'+CFNMETA(2)
 CASE index=93
  RETURN CFNMETA(2147483646.00/-2)+'|'+HexBytes(BINTOC(2147483646.00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646.00/-2))+'|'+CFNMETA(2147483646.00)+'|'+CFNMETA(-2)
 CASE index=94
  RETURN CFNMETA(-(2147483646.00/-2))+'|'+HexBytes(BINTOC(-(2147483646.00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646.00/-2)))+'|'+CFNMETA(2147483646.00)+'|'+CFNMETA(-2)
 CASE index=95
  RETURN CFNMETA(-2147483646.00/-2)+'|'+HexBytes(BINTOC(-2147483646.00/-2,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646.00/-2))+'|'+CFNMETA(-2147483646.00)+'|'+CFNMETA(-2)
 CASE index=96
  RETURN CFNMETA(-(-2147483646.00/-2))+'|'+HexBytes(BINTOC(-(-2147483646.00/-2),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646.00/-2)))+'|'+CFNMETA(-2147483646.00)+'|'+CFNMETA(-2)
 CASE index=97
  RETURN CFNMETA(2147483646/2.00)+'|'+HexBytes(BINTOC(2147483646/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646/2.00))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(2.00)
 CASE index=98
  RETURN CFNMETA(-(2147483646/2.00))+'|'+HexBytes(BINTOC(-(2147483646/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646/2.00)))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(2.00)
 CASE index=99
  RETURN CFNMETA(-2147483646/2.00)+'|'+HexBytes(BINTOC(-2147483646/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646/2.00))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(2.00)
 CASE index=100
  RETURN CFNMETA(-(-2147483646/2.00))+'|'+HexBytes(BINTOC(-(-2147483646/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646/2.00)))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(2.00)
 CASE index=101
  RETURN CFNMETA(2147483646/-2.00)+'|'+HexBytes(BINTOC(2147483646/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646/-2.00))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(-2.00)
 CASE index=102
  RETURN CFNMETA(-(2147483646/-2.00))+'|'+HexBytes(BINTOC(-(2147483646/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646/-2.00)))+'|'+CFNMETA(2147483646)+'|'+CFNMETA(-2.00)
 CASE index=103
  RETURN CFNMETA(-2147483646/-2.00)+'|'+HexBytes(BINTOC(-2147483646/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646/-2.00))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(-2.00)
 CASE index=104
  RETURN CFNMETA(-(-2147483646/-2.00))+'|'+HexBytes(BINTOC(-(-2147483646/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646/-2.00)))+'|'+CFNMETA(-2147483646)+'|'+CFNMETA(-2.00)
 CASE index=105
  RETURN CFNMETA(2147483646.00/2.00)+'|'+HexBytes(BINTOC(2147483646.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646.00/2.00))+'|'+CFNMETA(2147483646.00)+'|'+CFNMETA(2.00)
 CASE index=106
  RETURN CFNMETA(-(2147483646.00/2.00))+'|'+HexBytes(BINTOC(-(2147483646.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646.00/2.00)))+'|'+CFNMETA(2147483646.00)+'|'+CFNMETA(2.00)
 CASE index=107
  RETURN CFNMETA(-2147483646.00/2.00)+'|'+HexBytes(BINTOC(-2147483646.00/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646.00/2.00))+'|'+CFNMETA(-2147483646.00)+'|'+CFNMETA(2.00)
 CASE index=108
  RETURN CFNMETA(-(-2147483646.00/2.00))+'|'+HexBytes(BINTOC(-(-2147483646.00/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646.00/2.00)))+'|'+CFNMETA(-2147483646.00)+'|'+CFNMETA(2.00)
 CASE index=109
  RETURN CFNMETA(2147483646.00/-2.00)+'|'+HexBytes(BINTOC(2147483646.00/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646.00/-2.00))+'|'+CFNMETA(2147483646.00)+'|'+CFNMETA(-2.00)
 CASE index=110
  RETURN CFNMETA(-(2147483646.00/-2.00))+'|'+HexBytes(BINTOC(-(2147483646.00/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646.00/-2.00)))+'|'+CFNMETA(2147483646.00)+'|'+CFNMETA(-2.00)
 CASE index=111
  RETURN CFNMETA(-2147483646.00/-2.00)+'|'+HexBytes(BINTOC(-2147483646.00/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646.00/-2.00))+'|'+CFNMETA(-2147483646.00)+'|'+CFNMETA(-2.00)
 CASE index=112
  RETURN CFNMETA(-(-2147483646.00/-2.00))+'|'+HexBytes(BINTOC(-(-2147483646.00/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646.00/-2.00)))+'|'+CFNMETA(-2147483646.00)+'|'+CFNMETA(-2.00)
 CASE index=113
  RETURN CFNMETA(2147483646.000000/2.00)+'|'+HexBytes(BINTOC(2147483646.000000/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646.000000/2.00))+'|'+CFNMETA(2147483646.000000)+'|'+CFNMETA(2.00)
 CASE index=114
  RETURN CFNMETA(-(2147483646.000000/2.00))+'|'+HexBytes(BINTOC(-(2147483646.000000/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646.000000/2.00)))+'|'+CFNMETA(2147483646.000000)+'|'+CFNMETA(2.00)
 CASE index=115
  RETURN CFNMETA(-2147483646.000000/2.00)+'|'+HexBytes(BINTOC(-2147483646.000000/2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646.000000/2.00))+'|'+CFNMETA(-2147483646.000000)+'|'+CFNMETA(2.00)
 CASE index=116
  RETURN CFNMETA(-(-2147483646.000000/2.00))+'|'+HexBytes(BINTOC(-(-2147483646.000000/2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646.000000/2.00)))+'|'+CFNMETA(-2147483646.000000)+'|'+CFNMETA(2.00)
 CASE index=117
  RETURN CFNMETA(2147483646.000000/-2.00)+'|'+HexBytes(BINTOC(2147483646.000000/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646.000000/-2.00))+'|'+CFNMETA(2147483646.000000)+'|'+CFNMETA(-2.00)
 CASE index=118
  RETURN CFNMETA(-(2147483646.000000/-2.00))+'|'+HexBytes(BINTOC(-(2147483646.000000/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646.000000/-2.00)))+'|'+CFNMETA(2147483646.000000)+'|'+CFNMETA(-2.00)
 CASE index=119
  RETURN CFNMETA(-2147483646.000000/-2.00)+'|'+HexBytes(BINTOC(-2147483646.000000/-2.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646.000000/-2.00))+'|'+CFNMETA(-2147483646.000000)+'|'+CFNMETA(-2.00)
 CASE index=120
  RETURN CFNMETA(-(-2147483646.000000/-2.00))+'|'+HexBytes(BINTOC(-(-2147483646.000000/-2.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646.000000/-2.00)))+'|'+CFNMETA(-2147483646.000000)+'|'+CFNMETA(-2.00)
 CASE index=121
  RETURN CFNMETA(2147483646.000000000000000000/2.000000000000000000)+'|'+HexBytes(BINTOC(2147483646.000000000000000000/2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646.000000000000000000/2.000000000000000000))+'|'+CFNMETA(2147483646.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=122
  RETURN CFNMETA(-(2147483646.000000000000000000/2.000000000000000000))+'|'+HexBytes(BINTOC(-(2147483646.000000000000000000/2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646.000000000000000000/2.000000000000000000)))+'|'+CFNMETA(2147483646.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=123
  RETURN CFNMETA(-2147483646.000000000000000000/2.000000000000000000)+'|'+HexBytes(BINTOC(-2147483646.000000000000000000/2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646.000000000000000000/2.000000000000000000))+'|'+CFNMETA(-2147483646.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=124
  RETURN CFNMETA(-(-2147483646.000000000000000000/2.000000000000000000))+'|'+HexBytes(BINTOC(-(-2147483646.000000000000000000/2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646.000000000000000000/2.000000000000000000)))+'|'+CFNMETA(-2147483646.000000000000000000)+'|'+CFNMETA(2.000000000000000000)
 CASE index=125
  RETURN CFNMETA(2147483646.000000000000000000/-2.000000000000000000)+'|'+HexBytes(BINTOC(2147483646.000000000000000000/-2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(2147483646.000000000000000000/-2.000000000000000000))+'|'+CFNMETA(2147483646.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=126
  RETURN CFNMETA(-(2147483646.000000000000000000/-2.000000000000000000))+'|'+HexBytes(BINTOC(-(2147483646.000000000000000000/-2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483646.000000000000000000/-2.000000000000000000)))+'|'+CFNMETA(2147483646.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=127
  RETURN CFNMETA(-2147483646.000000000000000000/-2.000000000000000000)+'|'+HexBytes(BINTOC(-2147483646.000000000000000000/-2.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483646.000000000000000000/-2.000000000000000000))+'|'+CFNMETA(-2147483646.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=128
  RETURN CFNMETA(-(-2147483646.000000000000000000/-2.000000000000000000))+'|'+HexBytes(BINTOC(-(-2147483646.000000000000000000/-2.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483646.000000000000000000/-2.000000000000000000)))+'|'+CFNMETA(-2147483646.000000000000000000)+'|'+CFNMETA(-2.000000000000000000)
 CASE index=129
  RETURN CFNMETA(2147483647/3)+'|'+HexBytes(BINTOC(2147483647/3,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647/3))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(3)
 CASE index=130
  RETURN CFNMETA(-(2147483647/3))+'|'+HexBytes(BINTOC(-(2147483647/3),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647/3)))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(3)
 CASE index=131
  RETURN CFNMETA(-2147483647/3)+'|'+HexBytes(BINTOC(-2147483647/3,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647/3))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(3)
 CASE index=132
  RETURN CFNMETA(-(-2147483647/3))+'|'+HexBytes(BINTOC(-(-2147483647/3),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647/3)))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(3)
 CASE index=133
  RETURN CFNMETA(2147483647/-3)+'|'+HexBytes(BINTOC(2147483647/-3,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647/-3))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(-3)
 CASE index=134
  RETURN CFNMETA(-(2147483647/-3))+'|'+HexBytes(BINTOC(-(2147483647/-3),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647/-3)))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(-3)
 CASE index=135
  RETURN CFNMETA(-2147483647/-3)+'|'+HexBytes(BINTOC(-2147483647/-3,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647/-3))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(-3)
 CASE index=136
  RETURN CFNMETA(-(-2147483647/-3))+'|'+HexBytes(BINTOC(-(-2147483647/-3),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647/-3)))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(-3)
 CASE index=137
  RETURN CFNMETA(02147483647/3)+'|'+HexBytes(BINTOC(02147483647/3,'BRS'))+'|'+HexBytes(TRANSFORM(02147483647/3))+'|'+CFNMETA(02147483647)+'|'+CFNMETA(3)
 CASE index=138
  RETURN CFNMETA(-(02147483647/3))+'|'+HexBytes(BINTOC(-(02147483647/3),'BRS'))+'|'+HexBytes(TRANSFORM(-(02147483647/3)))+'|'+CFNMETA(02147483647)+'|'+CFNMETA(3)
 CASE index=139
  RETURN CFNMETA(-02147483647/3)+'|'+HexBytes(BINTOC(-02147483647/3,'BRS'))+'|'+HexBytes(TRANSFORM(-02147483647/3))+'|'+CFNMETA(-02147483647)+'|'+CFNMETA(3)
 CASE index=140
  RETURN CFNMETA(-(-02147483647/3))+'|'+HexBytes(BINTOC(-(-02147483647/3),'BRS'))+'|'+HexBytes(TRANSFORM(-(-02147483647/3)))+'|'+CFNMETA(-02147483647)+'|'+CFNMETA(3)
 CASE index=141
  RETURN CFNMETA(02147483647/-3)+'|'+HexBytes(BINTOC(02147483647/-3,'BRS'))+'|'+HexBytes(TRANSFORM(02147483647/-3))+'|'+CFNMETA(02147483647)+'|'+CFNMETA(-3)
 CASE index=142
  RETURN CFNMETA(-(02147483647/-3))+'|'+HexBytes(BINTOC(-(02147483647/-3),'BRS'))+'|'+HexBytes(TRANSFORM(-(02147483647/-3)))+'|'+CFNMETA(02147483647)+'|'+CFNMETA(-3)
 CASE index=143
  RETURN CFNMETA(-02147483647/-3)+'|'+HexBytes(BINTOC(-02147483647/-3,'BRS'))+'|'+HexBytes(TRANSFORM(-02147483647/-3))+'|'+CFNMETA(-02147483647)+'|'+CFNMETA(-3)
 CASE index=144
  RETURN CFNMETA(-(-02147483647/-3))+'|'+HexBytes(BINTOC(-(-02147483647/-3),'BRS'))+'|'+HexBytes(TRANSFORM(-(-02147483647/-3)))+'|'+CFNMETA(-02147483647)+'|'+CFNMETA(-3)
 CASE index=145
  RETURN CFNMETA(2147483647/03)+'|'+HexBytes(BINTOC(2147483647/03,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647/03))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(03)
 CASE index=146
  RETURN CFNMETA(-(2147483647/03))+'|'+HexBytes(BINTOC(-(2147483647/03),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647/03)))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(03)
 CASE index=147
  RETURN CFNMETA(-2147483647/03)+'|'+HexBytes(BINTOC(-2147483647/03,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647/03))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(03)
 CASE index=148
  RETURN CFNMETA(-(-2147483647/03))+'|'+HexBytes(BINTOC(-(-2147483647/03),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647/03)))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(03)
 CASE index=149
  RETURN CFNMETA(2147483647/-03)+'|'+HexBytes(BINTOC(2147483647/-03,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647/-03))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(-03)
 CASE index=150
  RETURN CFNMETA(-(2147483647/-03))+'|'+HexBytes(BINTOC(-(2147483647/-03),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647/-03)))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(-03)
 CASE index=151
  RETURN CFNMETA(-2147483647/-03)+'|'+HexBytes(BINTOC(-2147483647/-03,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647/-03))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(-03)
 CASE index=152
  RETURN CFNMETA(-(-2147483647/-03))+'|'+HexBytes(BINTOC(-(-2147483647/-03),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647/-03)))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(-03)
 CASE index=153
  RETURN CFNMETA(2147483647.00/3)+'|'+HexBytes(BINTOC(2147483647.00/3,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647.00/3))+'|'+CFNMETA(2147483647.00)+'|'+CFNMETA(3)
 CASE index=154
  RETURN CFNMETA(-(2147483647.00/3))+'|'+HexBytes(BINTOC(-(2147483647.00/3),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647.00/3)))+'|'+CFNMETA(2147483647.00)+'|'+CFNMETA(3)
 CASE index=155
  RETURN CFNMETA(-2147483647.00/3)+'|'+HexBytes(BINTOC(-2147483647.00/3,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647.00/3))+'|'+CFNMETA(-2147483647.00)+'|'+CFNMETA(3)
 CASE index=156
  RETURN CFNMETA(-(-2147483647.00/3))+'|'+HexBytes(BINTOC(-(-2147483647.00/3),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647.00/3)))+'|'+CFNMETA(-2147483647.00)+'|'+CFNMETA(3)
 CASE index=157
  RETURN CFNMETA(2147483647.00/-3)+'|'+HexBytes(BINTOC(2147483647.00/-3,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647.00/-3))+'|'+CFNMETA(2147483647.00)+'|'+CFNMETA(-3)
 CASE index=158
  RETURN CFNMETA(-(2147483647.00/-3))+'|'+HexBytes(BINTOC(-(2147483647.00/-3),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647.00/-3)))+'|'+CFNMETA(2147483647.00)+'|'+CFNMETA(-3)
 CASE index=159
  RETURN CFNMETA(-2147483647.00/-3)+'|'+HexBytes(BINTOC(-2147483647.00/-3,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647.00/-3))+'|'+CFNMETA(-2147483647.00)+'|'+CFNMETA(-3)
 CASE index=160
  RETURN CFNMETA(-(-2147483647.00/-3))+'|'+HexBytes(BINTOC(-(-2147483647.00/-3),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647.00/-3)))+'|'+CFNMETA(-2147483647.00)+'|'+CFNMETA(-3)
 CASE index=161
  RETURN CFNMETA(2147483647/3.00)+'|'+HexBytes(BINTOC(2147483647/3.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647/3.00))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(3.00)
 CASE index=162
  RETURN CFNMETA(-(2147483647/3.00))+'|'+HexBytes(BINTOC(-(2147483647/3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647/3.00)))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(3.00)
 CASE index=163
  RETURN CFNMETA(-2147483647/3.00)+'|'+HexBytes(BINTOC(-2147483647/3.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647/3.00))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(3.00)
 CASE index=164
  RETURN CFNMETA(-(-2147483647/3.00))+'|'+HexBytes(BINTOC(-(-2147483647/3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647/3.00)))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(3.00)
 CASE index=165
  RETURN CFNMETA(2147483647/-3.00)+'|'+HexBytes(BINTOC(2147483647/-3.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647/-3.00))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(-3.00)
 CASE index=166
  RETURN CFNMETA(-(2147483647/-3.00))+'|'+HexBytes(BINTOC(-(2147483647/-3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647/-3.00)))+'|'+CFNMETA(2147483647)+'|'+CFNMETA(-3.00)
 CASE index=167
  RETURN CFNMETA(-2147483647/-3.00)+'|'+HexBytes(BINTOC(-2147483647/-3.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647/-3.00))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(-3.00)
 CASE index=168
  RETURN CFNMETA(-(-2147483647/-3.00))+'|'+HexBytes(BINTOC(-(-2147483647/-3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647/-3.00)))+'|'+CFNMETA(-2147483647)+'|'+CFNMETA(-3.00)
 CASE index=169
  RETURN CFNMETA(2147483647.00/3.00)+'|'+HexBytes(BINTOC(2147483647.00/3.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647.00/3.00))+'|'+CFNMETA(2147483647.00)+'|'+CFNMETA(3.00)
 CASE index=170
  RETURN CFNMETA(-(2147483647.00/3.00))+'|'+HexBytes(BINTOC(-(2147483647.00/3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647.00/3.00)))+'|'+CFNMETA(2147483647.00)+'|'+CFNMETA(3.00)
 CASE index=171
  RETURN CFNMETA(-2147483647.00/3.00)+'|'+HexBytes(BINTOC(-2147483647.00/3.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647.00/3.00))+'|'+CFNMETA(-2147483647.00)+'|'+CFNMETA(3.00)
 CASE index=172
  RETURN CFNMETA(-(-2147483647.00/3.00))+'|'+HexBytes(BINTOC(-(-2147483647.00/3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647.00/3.00)))+'|'+CFNMETA(-2147483647.00)+'|'+CFNMETA(3.00)
 CASE index=173
  RETURN CFNMETA(2147483647.00/-3.00)+'|'+HexBytes(BINTOC(2147483647.00/-3.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647.00/-3.00))+'|'+CFNMETA(2147483647.00)+'|'+CFNMETA(-3.00)
 CASE index=174
  RETURN CFNMETA(-(2147483647.00/-3.00))+'|'+HexBytes(BINTOC(-(2147483647.00/-3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647.00/-3.00)))+'|'+CFNMETA(2147483647.00)+'|'+CFNMETA(-3.00)
 CASE index=175
  RETURN CFNMETA(-2147483647.00/-3.00)+'|'+HexBytes(BINTOC(-2147483647.00/-3.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647.00/-3.00))+'|'+CFNMETA(-2147483647.00)+'|'+CFNMETA(-3.00)
 CASE index=176
  RETURN CFNMETA(-(-2147483647.00/-3.00))+'|'+HexBytes(BINTOC(-(-2147483647.00/-3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647.00/-3.00)))+'|'+CFNMETA(-2147483647.00)+'|'+CFNMETA(-3.00)
 CASE index=177
  RETURN CFNMETA(2147483647.000000/3.00)+'|'+HexBytes(BINTOC(2147483647.000000/3.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647.000000/3.00))+'|'+CFNMETA(2147483647.000000)+'|'+CFNMETA(3.00)
 CASE index=178
  RETURN CFNMETA(-(2147483647.000000/3.00))+'|'+HexBytes(BINTOC(-(2147483647.000000/3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647.000000/3.00)))+'|'+CFNMETA(2147483647.000000)+'|'+CFNMETA(3.00)
 CASE index=179
  RETURN CFNMETA(-2147483647.000000/3.00)+'|'+HexBytes(BINTOC(-2147483647.000000/3.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647.000000/3.00))+'|'+CFNMETA(-2147483647.000000)+'|'+CFNMETA(3.00)
 CASE index=180
  RETURN CFNMETA(-(-2147483647.000000/3.00))+'|'+HexBytes(BINTOC(-(-2147483647.000000/3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647.000000/3.00)))+'|'+CFNMETA(-2147483647.000000)+'|'+CFNMETA(3.00)
 CASE index=181
  RETURN CFNMETA(2147483647.000000/-3.00)+'|'+HexBytes(BINTOC(2147483647.000000/-3.00,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647.000000/-3.00))+'|'+CFNMETA(2147483647.000000)+'|'+CFNMETA(-3.00)
 CASE index=182
  RETURN CFNMETA(-(2147483647.000000/-3.00))+'|'+HexBytes(BINTOC(-(2147483647.000000/-3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647.000000/-3.00)))+'|'+CFNMETA(2147483647.000000)+'|'+CFNMETA(-3.00)
 CASE index=183
  RETURN CFNMETA(-2147483647.000000/-3.00)+'|'+HexBytes(BINTOC(-2147483647.000000/-3.00,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647.000000/-3.00))+'|'+CFNMETA(-2147483647.000000)+'|'+CFNMETA(-3.00)
 CASE index=184
  RETURN CFNMETA(-(-2147483647.000000/-3.00))+'|'+HexBytes(BINTOC(-(-2147483647.000000/-3.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647.000000/-3.00)))+'|'+CFNMETA(-2147483647.000000)+'|'+CFNMETA(-3.00)
 CASE index=185
  RETURN CFNMETA(2147483647.000000000000000000/3.000000000000000000)+'|'+HexBytes(BINTOC(2147483647.000000000000000000/3.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647.000000000000000000/3.000000000000000000))+'|'+CFNMETA(2147483647.000000000000000000)+'|'+CFNMETA(3.000000000000000000)
 CASE index=186
  RETURN CFNMETA(-(2147483647.000000000000000000/3.000000000000000000))+'|'+HexBytes(BINTOC(-(2147483647.000000000000000000/3.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647.000000000000000000/3.000000000000000000)))+'|'+CFNMETA(2147483647.000000000000000000)+'|'+CFNMETA(3.000000000000000000)
 CASE index=187
  RETURN CFNMETA(-2147483647.000000000000000000/3.000000000000000000)+'|'+HexBytes(BINTOC(-2147483647.000000000000000000/3.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647.000000000000000000/3.000000000000000000))+'|'+CFNMETA(-2147483647.000000000000000000)+'|'+CFNMETA(3.000000000000000000)
 CASE index=188
  RETURN CFNMETA(-(-2147483647.000000000000000000/3.000000000000000000))+'|'+HexBytes(BINTOC(-(-2147483647.000000000000000000/3.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647.000000000000000000/3.000000000000000000)))+'|'+CFNMETA(-2147483647.000000000000000000)+'|'+CFNMETA(3.000000000000000000)
 CASE index=189
  RETURN CFNMETA(2147483647.000000000000000000/-3.000000000000000000)+'|'+HexBytes(BINTOC(2147483647.000000000000000000/-3.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(2147483647.000000000000000000/-3.000000000000000000))+'|'+CFNMETA(2147483647.000000000000000000)+'|'+CFNMETA(-3.000000000000000000)
 CASE index=190
  RETURN CFNMETA(-(2147483647.000000000000000000/-3.000000000000000000))+'|'+HexBytes(BINTOC(-(2147483647.000000000000000000/-3.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(2147483647.000000000000000000/-3.000000000000000000)))+'|'+CFNMETA(2147483647.000000000000000000)+'|'+CFNMETA(-3.000000000000000000)
 CASE index=191
  RETURN CFNMETA(-2147483647.000000000000000000/-3.000000000000000000)+'|'+HexBytes(BINTOC(-2147483647.000000000000000000/-3.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-2147483647.000000000000000000/-3.000000000000000000))+'|'+CFNMETA(-2147483647.000000000000000000)+'|'+CFNMETA(-3.000000000000000000)
 CASE index=192
  RETURN CFNMETA(-(-2147483647.000000000000000000/-3.000000000000000000))+'|'+HexBytes(BINTOC(-(-2147483647.000000000000000000/-3.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-2147483647.000000000000000000/-3.000000000000000000)))+'|'+CFNMETA(-2147483647.000000000000000000)+'|'+CFNMETA(-3.000000000000000000)
 CASE index=193
  RETURN CFNMETA(9999999999/1)+'|'+HexBytes(BINTOC(9999999999/1,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999/1))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(1)
 CASE index=194
  RETURN CFNMETA(-(9999999999/1))+'|'+HexBytes(BINTOC(-(9999999999/1),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999/1)))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(1)
 CASE index=195
  RETURN CFNMETA(-9999999999/1)+'|'+HexBytes(BINTOC(-9999999999/1,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999/1))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(1)
 CASE index=196
  RETURN CFNMETA(-(-9999999999/1))+'|'+HexBytes(BINTOC(-(-9999999999/1),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999/1)))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(1)
 CASE index=197
  RETURN CFNMETA(9999999999/-1)+'|'+HexBytes(BINTOC(9999999999/-1,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999/-1))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(-1)
 CASE index=198
  RETURN CFNMETA(-(9999999999/-1))+'|'+HexBytes(BINTOC(-(9999999999/-1),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999/-1)))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(-1)
 CASE index=199
  RETURN CFNMETA(-9999999999/-1)+'|'+HexBytes(BINTOC(-9999999999/-1,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999/-1))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(-1)
 CASE index=200
  RETURN CFNMETA(-(-9999999999/-1))+'|'+HexBytes(BINTOC(-(-9999999999/-1),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999/-1)))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(-1)
 CASE index=201
  RETURN CFNMETA(09999999999/1)+'|'+HexBytes(BINTOC(09999999999/1,'BRS'))+'|'+HexBytes(TRANSFORM(09999999999/1))+'|'+CFNMETA(09999999999)+'|'+CFNMETA(1)
 CASE index=202
  RETURN CFNMETA(-(09999999999/1))+'|'+HexBytes(BINTOC(-(09999999999/1),'BRS'))+'|'+HexBytes(TRANSFORM(-(09999999999/1)))+'|'+CFNMETA(09999999999)+'|'+CFNMETA(1)
 CASE index=203
  RETURN CFNMETA(-09999999999/1)+'|'+HexBytes(BINTOC(-09999999999/1,'BRS'))+'|'+HexBytes(TRANSFORM(-09999999999/1))+'|'+CFNMETA(-09999999999)+'|'+CFNMETA(1)
 CASE index=204
  RETURN CFNMETA(-(-09999999999/1))+'|'+HexBytes(BINTOC(-(-09999999999/1),'BRS'))+'|'+HexBytes(TRANSFORM(-(-09999999999/1)))+'|'+CFNMETA(-09999999999)+'|'+CFNMETA(1)
 CASE index=205
  RETURN CFNMETA(09999999999/-1)+'|'+HexBytes(BINTOC(09999999999/-1,'BRS'))+'|'+HexBytes(TRANSFORM(09999999999/-1))+'|'+CFNMETA(09999999999)+'|'+CFNMETA(-1)
 CASE index=206
  RETURN CFNMETA(-(09999999999/-1))+'|'+HexBytes(BINTOC(-(09999999999/-1),'BRS'))+'|'+HexBytes(TRANSFORM(-(09999999999/-1)))+'|'+CFNMETA(09999999999)+'|'+CFNMETA(-1)
 CASE index=207
  RETURN CFNMETA(-09999999999/-1)+'|'+HexBytes(BINTOC(-09999999999/-1,'BRS'))+'|'+HexBytes(TRANSFORM(-09999999999/-1))+'|'+CFNMETA(-09999999999)+'|'+CFNMETA(-1)
 CASE index=208
  RETURN CFNMETA(-(-09999999999/-1))+'|'+HexBytes(BINTOC(-(-09999999999/-1),'BRS'))+'|'+HexBytes(TRANSFORM(-(-09999999999/-1)))+'|'+CFNMETA(-09999999999)+'|'+CFNMETA(-1)
 CASE index=209
  RETURN CFNMETA(9999999999/01)+'|'+HexBytes(BINTOC(9999999999/01,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999/01))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(01)
 CASE index=210
  RETURN CFNMETA(-(9999999999/01))+'|'+HexBytes(BINTOC(-(9999999999/01),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999/01)))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(01)
 CASE index=211
  RETURN CFNMETA(-9999999999/01)+'|'+HexBytes(BINTOC(-9999999999/01,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999/01))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(01)
 CASE index=212
  RETURN CFNMETA(-(-9999999999/01))+'|'+HexBytes(BINTOC(-(-9999999999/01),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999/01)))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(01)
 CASE index=213
  RETURN CFNMETA(9999999999/-01)+'|'+HexBytes(BINTOC(9999999999/-01,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999/-01))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(-01)
 CASE index=214
  RETURN CFNMETA(-(9999999999/-01))+'|'+HexBytes(BINTOC(-(9999999999/-01),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999/-01)))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(-01)
 CASE index=215
  RETURN CFNMETA(-9999999999/-01)+'|'+HexBytes(BINTOC(-9999999999/-01,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999/-01))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(-01)
 CASE index=216
  RETURN CFNMETA(-(-9999999999/-01))+'|'+HexBytes(BINTOC(-(-9999999999/-01),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999/-01)))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(-01)
 CASE index=217
  RETURN CFNMETA(9999999999.00/1)+'|'+HexBytes(BINTOC(9999999999.00/1,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.00/1))+'|'+CFNMETA(9999999999.00)+'|'+CFNMETA(1)
 CASE index=218
  RETURN CFNMETA(-(9999999999.00/1))+'|'+HexBytes(BINTOC(-(9999999999.00/1),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999.00/1)))+'|'+CFNMETA(9999999999.00)+'|'+CFNMETA(1)
 CASE index=219
  RETURN CFNMETA(-9999999999.00/1)+'|'+HexBytes(BINTOC(-9999999999.00/1,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.00/1))+'|'+CFNMETA(-9999999999.00)+'|'+CFNMETA(1)
 CASE index=220
  RETURN CFNMETA(-(-9999999999.00/1))+'|'+HexBytes(BINTOC(-(-9999999999.00/1),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999.00/1)))+'|'+CFNMETA(-9999999999.00)+'|'+CFNMETA(1)
 CASE index=221
  RETURN CFNMETA(9999999999.00/-1)+'|'+HexBytes(BINTOC(9999999999.00/-1,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.00/-1))+'|'+CFNMETA(9999999999.00)+'|'+CFNMETA(-1)
 CASE index=222
  RETURN CFNMETA(-(9999999999.00/-1))+'|'+HexBytes(BINTOC(-(9999999999.00/-1),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999.00/-1)))+'|'+CFNMETA(9999999999.00)+'|'+CFNMETA(-1)
 CASE index=223
  RETURN CFNMETA(-9999999999.00/-1)+'|'+HexBytes(BINTOC(-9999999999.00/-1,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.00/-1))+'|'+CFNMETA(-9999999999.00)+'|'+CFNMETA(-1)
 CASE index=224
  RETURN CFNMETA(-(-9999999999.00/-1))+'|'+HexBytes(BINTOC(-(-9999999999.00/-1),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999.00/-1)))+'|'+CFNMETA(-9999999999.00)+'|'+CFNMETA(-1)
 CASE index=225
  RETURN CFNMETA(9999999999/1.00)+'|'+HexBytes(BINTOC(9999999999/1.00,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999/1.00))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(1.00)
 CASE index=226
  RETURN CFNMETA(-(9999999999/1.00))+'|'+HexBytes(BINTOC(-(9999999999/1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999/1.00)))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(1.00)
 CASE index=227
  RETURN CFNMETA(-9999999999/1.00)+'|'+HexBytes(BINTOC(-9999999999/1.00,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999/1.00))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(1.00)
 CASE index=228
  RETURN CFNMETA(-(-9999999999/1.00))+'|'+HexBytes(BINTOC(-(-9999999999/1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999/1.00)))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(1.00)
 CASE index=229
  RETURN CFNMETA(9999999999/-1.00)+'|'+HexBytes(BINTOC(9999999999/-1.00,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999/-1.00))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(-1.00)
 CASE index=230
  RETURN CFNMETA(-(9999999999/-1.00))+'|'+HexBytes(BINTOC(-(9999999999/-1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999/-1.00)))+'|'+CFNMETA(9999999999)+'|'+CFNMETA(-1.00)
 CASE index=231
  RETURN CFNMETA(-9999999999/-1.00)+'|'+HexBytes(BINTOC(-9999999999/-1.00,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999/-1.00))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(-1.00)
 CASE index=232
  RETURN CFNMETA(-(-9999999999/-1.00))+'|'+HexBytes(BINTOC(-(-9999999999/-1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999/-1.00)))+'|'+CFNMETA(-9999999999)+'|'+CFNMETA(-1.00)
 CASE index=233
  RETURN CFNMETA(9999999999.00/1.00)+'|'+HexBytes(BINTOC(9999999999.00/1.00,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.00/1.00))+'|'+CFNMETA(9999999999.00)+'|'+CFNMETA(1.00)
 CASE index=234
  RETURN CFNMETA(-(9999999999.00/1.00))+'|'+HexBytes(BINTOC(-(9999999999.00/1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999.00/1.00)))+'|'+CFNMETA(9999999999.00)+'|'+CFNMETA(1.00)
 CASE index=235
  RETURN CFNMETA(-9999999999.00/1.00)+'|'+HexBytes(BINTOC(-9999999999.00/1.00,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.00/1.00))+'|'+CFNMETA(-9999999999.00)+'|'+CFNMETA(1.00)
 CASE index=236
  RETURN CFNMETA(-(-9999999999.00/1.00))+'|'+HexBytes(BINTOC(-(-9999999999.00/1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999.00/1.00)))+'|'+CFNMETA(-9999999999.00)+'|'+CFNMETA(1.00)
 CASE index=237
  RETURN CFNMETA(9999999999.00/-1.00)+'|'+HexBytes(BINTOC(9999999999.00/-1.00,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.00/-1.00))+'|'+CFNMETA(9999999999.00)+'|'+CFNMETA(-1.00)
 CASE index=238
  RETURN CFNMETA(-(9999999999.00/-1.00))+'|'+HexBytes(BINTOC(-(9999999999.00/-1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999.00/-1.00)))+'|'+CFNMETA(9999999999.00)+'|'+CFNMETA(-1.00)
 CASE index=239
  RETURN CFNMETA(-9999999999.00/-1.00)+'|'+HexBytes(BINTOC(-9999999999.00/-1.00,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.00/-1.00))+'|'+CFNMETA(-9999999999.00)+'|'+CFNMETA(-1.00)
 CASE index=240
  RETURN CFNMETA(-(-9999999999.00/-1.00))+'|'+HexBytes(BINTOC(-(-9999999999.00/-1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999.00/-1.00)))+'|'+CFNMETA(-9999999999.00)+'|'+CFNMETA(-1.00)
 CASE index=241
  RETURN CFNMETA(9999999999.000000/1.00)+'|'+HexBytes(BINTOC(9999999999.000000/1.00,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.000000/1.00))+'|'+CFNMETA(9999999999.000000)+'|'+CFNMETA(1.00)
 CASE index=242
  RETURN CFNMETA(-(9999999999.000000/1.00))+'|'+HexBytes(BINTOC(-(9999999999.000000/1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999.000000/1.00)))+'|'+CFNMETA(9999999999.000000)+'|'+CFNMETA(1.00)
 CASE index=243
  RETURN CFNMETA(-9999999999.000000/1.00)+'|'+HexBytes(BINTOC(-9999999999.000000/1.00,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.000000/1.00))+'|'+CFNMETA(-9999999999.000000)+'|'+CFNMETA(1.00)
 CASE index=244
  RETURN CFNMETA(-(-9999999999.000000/1.00))+'|'+HexBytes(BINTOC(-(-9999999999.000000/1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999.000000/1.00)))+'|'+CFNMETA(-9999999999.000000)+'|'+CFNMETA(1.00)
 CASE index=245
  RETURN CFNMETA(9999999999.000000/-1.00)+'|'+HexBytes(BINTOC(9999999999.000000/-1.00,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.000000/-1.00))+'|'+CFNMETA(9999999999.000000)+'|'+CFNMETA(-1.00)
 CASE index=246
  RETURN CFNMETA(-(9999999999.000000/-1.00))+'|'+HexBytes(BINTOC(-(9999999999.000000/-1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999.000000/-1.00)))+'|'+CFNMETA(9999999999.000000)+'|'+CFNMETA(-1.00)
 CASE index=247
  RETURN CFNMETA(-9999999999.000000/-1.00)+'|'+HexBytes(BINTOC(-9999999999.000000/-1.00,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.000000/-1.00))+'|'+CFNMETA(-9999999999.000000)+'|'+CFNMETA(-1.00)
 CASE index=248
  RETURN CFNMETA(-(-9999999999.000000/-1.00))+'|'+HexBytes(BINTOC(-(-9999999999.000000/-1.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999.000000/-1.00)))+'|'+CFNMETA(-9999999999.000000)+'|'+CFNMETA(-1.00)
 CASE index=249
  RETURN CFNMETA(9999999999.000000000000000000/1.000000000000000000)+'|'+HexBytes(BINTOC(9999999999.000000000000000000/1.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.000000000000000000/1.000000000000000000))+'|'+CFNMETA(9999999999.000000000000000000)+'|'+CFNMETA(1.000000000000000000)
 CASE index=250
  RETURN CFNMETA(-(9999999999.000000000000000000/1.000000000000000000))+'|'+HexBytes(BINTOC(-(9999999999.000000000000000000/1.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999.000000000000000000/1.000000000000000000)))+'|'+CFNMETA(9999999999.000000000000000000)+'|'+CFNMETA(1.000000000000000000)
 CASE index=251
  RETURN CFNMETA(-9999999999.000000000000000000/1.000000000000000000)+'|'+HexBytes(BINTOC(-9999999999.000000000000000000/1.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.000000000000000000/1.000000000000000000))+'|'+CFNMETA(-9999999999.000000000000000000)+'|'+CFNMETA(1.000000000000000000)
 CASE index=252
  RETURN CFNMETA(-(-9999999999.000000000000000000/1.000000000000000000))+'|'+HexBytes(BINTOC(-(-9999999999.000000000000000000/1.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999.000000000000000000/1.000000000000000000)))+'|'+CFNMETA(-9999999999.000000000000000000)+'|'+CFNMETA(1.000000000000000000)
 CASE index=253
  RETURN CFNMETA(9999999999.000000000000000000/-1.000000000000000000)+'|'+HexBytes(BINTOC(9999999999.000000000000000000/-1.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999.000000000000000000/-1.000000000000000000))+'|'+CFNMETA(9999999999.000000000000000000)+'|'+CFNMETA(-1.000000000000000000)
 CASE index=254
  RETURN CFNMETA(-(9999999999.000000000000000000/-1.000000000000000000))+'|'+HexBytes(BINTOC(-(9999999999.000000000000000000/-1.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(9999999999.000000000000000000/-1.000000000000000000)))+'|'+CFNMETA(9999999999.000000000000000000)+'|'+CFNMETA(-1.000000000000000000)
 CASE index=255
  RETURN CFNMETA(-9999999999.000000000000000000/-1.000000000000000000)+'|'+HexBytes(BINTOC(-9999999999.000000000000000000/-1.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-9999999999.000000000000000000/-1.000000000000000000))+'|'+CFNMETA(-9999999999.000000000000000000)+'|'+CFNMETA(-1.000000000000000000)
 CASE index=256
  RETURN CFNMETA(-(-9999999999.000000000000000000/-1.000000000000000000))+'|'+HexBytes(BINTOC(-(-9999999999.000000000000000000/-1.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-9999999999.000000000000000000/-1.000000000000000000)))+'|'+CFNMETA(-9999999999.000000000000000000)+'|'+CFNMETA(-1.000000000000000000)
 CASE index=257
  RETURN CFNMETA(1/100)+'|'+HexBytes(BINTOC(1/100,'BRS'))+'|'+HexBytes(TRANSFORM(1/100))+'|'+CFNMETA(1)+'|'+CFNMETA(100)
 CASE index=258
  RETURN CFNMETA(-(1/100))+'|'+HexBytes(BINTOC(-(1/100),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/100)))+'|'+CFNMETA(1)+'|'+CFNMETA(100)
 CASE index=259
  RETURN CFNMETA(-1/100)+'|'+HexBytes(BINTOC(-1/100,'BRS'))+'|'+HexBytes(TRANSFORM(-1/100))+'|'+CFNMETA(-1)+'|'+CFNMETA(100)
 CASE index=260
  RETURN CFNMETA(-(-1/100))+'|'+HexBytes(BINTOC(-(-1/100),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/100)))+'|'+CFNMETA(-1)+'|'+CFNMETA(100)
 CASE index=261
  RETURN CFNMETA(1/-100)+'|'+HexBytes(BINTOC(1/-100,'BRS'))+'|'+HexBytes(TRANSFORM(1/-100))+'|'+CFNMETA(1)+'|'+CFNMETA(-100)
 CASE index=262
  RETURN CFNMETA(-(1/-100))+'|'+HexBytes(BINTOC(-(1/-100),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/-100)))+'|'+CFNMETA(1)+'|'+CFNMETA(-100)
 CASE index=263
  RETURN CFNMETA(-1/-100)+'|'+HexBytes(BINTOC(-1/-100,'BRS'))+'|'+HexBytes(TRANSFORM(-1/-100))+'|'+CFNMETA(-1)+'|'+CFNMETA(-100)
 CASE index=264
  RETURN CFNMETA(-(-1/-100))+'|'+HexBytes(BINTOC(-(-1/-100),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/-100)))+'|'+CFNMETA(-1)+'|'+CFNMETA(-100)
 CASE index=265
  RETURN CFNMETA(01/100)+'|'+HexBytes(BINTOC(01/100,'BRS'))+'|'+HexBytes(TRANSFORM(01/100))+'|'+CFNMETA(01)+'|'+CFNMETA(100)
 CASE index=266
  RETURN CFNMETA(-(01/100))+'|'+HexBytes(BINTOC(-(01/100),'BRS'))+'|'+HexBytes(TRANSFORM(-(01/100)))+'|'+CFNMETA(01)+'|'+CFNMETA(100)
 CASE index=267
  RETURN CFNMETA(-01/100)+'|'+HexBytes(BINTOC(-01/100,'BRS'))+'|'+HexBytes(TRANSFORM(-01/100))+'|'+CFNMETA(-01)+'|'+CFNMETA(100)
 CASE index=268
  RETURN CFNMETA(-(-01/100))+'|'+HexBytes(BINTOC(-(-01/100),'BRS'))+'|'+HexBytes(TRANSFORM(-(-01/100)))+'|'+CFNMETA(-01)+'|'+CFNMETA(100)
 CASE index=269
  RETURN CFNMETA(01/-100)+'|'+HexBytes(BINTOC(01/-100,'BRS'))+'|'+HexBytes(TRANSFORM(01/-100))+'|'+CFNMETA(01)+'|'+CFNMETA(-100)
 CASE index=270
  RETURN CFNMETA(-(01/-100))+'|'+HexBytes(BINTOC(-(01/-100),'BRS'))+'|'+HexBytes(TRANSFORM(-(01/-100)))+'|'+CFNMETA(01)+'|'+CFNMETA(-100)
 CASE index=271
  RETURN CFNMETA(-01/-100)+'|'+HexBytes(BINTOC(-01/-100,'BRS'))+'|'+HexBytes(TRANSFORM(-01/-100))+'|'+CFNMETA(-01)+'|'+CFNMETA(-100)
 CASE index=272
  RETURN CFNMETA(-(-01/-100))+'|'+HexBytes(BINTOC(-(-01/-100),'BRS'))+'|'+HexBytes(TRANSFORM(-(-01/-100)))+'|'+CFNMETA(-01)+'|'+CFNMETA(-100)
 CASE index=273
  RETURN CFNMETA(1/0100)+'|'+HexBytes(BINTOC(1/0100,'BRS'))+'|'+HexBytes(TRANSFORM(1/0100))+'|'+CFNMETA(1)+'|'+CFNMETA(0100)
 CASE index=274
  RETURN CFNMETA(-(1/0100))+'|'+HexBytes(BINTOC(-(1/0100),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/0100)))+'|'+CFNMETA(1)+'|'+CFNMETA(0100)
 CASE index=275
  RETURN CFNMETA(-1/0100)+'|'+HexBytes(BINTOC(-1/0100,'BRS'))+'|'+HexBytes(TRANSFORM(-1/0100))+'|'+CFNMETA(-1)+'|'+CFNMETA(0100)
 CASE index=276
  RETURN CFNMETA(-(-1/0100))+'|'+HexBytes(BINTOC(-(-1/0100),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/0100)))+'|'+CFNMETA(-1)+'|'+CFNMETA(0100)
 CASE index=277
  RETURN CFNMETA(1/-0100)+'|'+HexBytes(BINTOC(1/-0100,'BRS'))+'|'+HexBytes(TRANSFORM(1/-0100))+'|'+CFNMETA(1)+'|'+CFNMETA(-0100)
 CASE index=278
  RETURN CFNMETA(-(1/-0100))+'|'+HexBytes(BINTOC(-(1/-0100),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/-0100)))+'|'+CFNMETA(1)+'|'+CFNMETA(-0100)
 CASE index=279
  RETURN CFNMETA(-1/-0100)+'|'+HexBytes(BINTOC(-1/-0100,'BRS'))+'|'+HexBytes(TRANSFORM(-1/-0100))+'|'+CFNMETA(-1)+'|'+CFNMETA(-0100)
 CASE index=280
  RETURN CFNMETA(-(-1/-0100))+'|'+HexBytes(BINTOC(-(-1/-0100),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/-0100)))+'|'+CFNMETA(-1)+'|'+CFNMETA(-0100)
 CASE index=281
  RETURN CFNMETA(1.00/100)+'|'+HexBytes(BINTOC(1.00/100,'BRS'))+'|'+HexBytes(TRANSFORM(1.00/100))+'|'+CFNMETA(1.00)+'|'+CFNMETA(100)
 CASE index=282
  RETURN CFNMETA(-(1.00/100))+'|'+HexBytes(BINTOC(-(1.00/100),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00/100)))+'|'+CFNMETA(1.00)+'|'+CFNMETA(100)
 CASE index=283
  RETURN CFNMETA(-1.00/100)+'|'+HexBytes(BINTOC(-1.00/100,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00/100))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(100)
 CASE index=284
  RETURN CFNMETA(-(-1.00/100))+'|'+HexBytes(BINTOC(-(-1.00/100),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.00/100)))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(100)
 CASE index=285
  RETURN CFNMETA(1.00/-100)+'|'+HexBytes(BINTOC(1.00/-100,'BRS'))+'|'+HexBytes(TRANSFORM(1.00/-100))+'|'+CFNMETA(1.00)+'|'+CFNMETA(-100)
 CASE index=286
  RETURN CFNMETA(-(1.00/-100))+'|'+HexBytes(BINTOC(-(1.00/-100),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00/-100)))+'|'+CFNMETA(1.00)+'|'+CFNMETA(-100)
 CASE index=287
  RETURN CFNMETA(-1.00/-100)+'|'+HexBytes(BINTOC(-1.00/-100,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00/-100))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(-100)
 CASE index=288
  RETURN CFNMETA(-(-1.00/-100))+'|'+HexBytes(BINTOC(-(-1.00/-100),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.00/-100)))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(-100)
 CASE index=289
  RETURN CFNMETA(1/100.00)+'|'+HexBytes(BINTOC(1/100.00,'BRS'))+'|'+HexBytes(TRANSFORM(1/100.00))+'|'+CFNMETA(1)+'|'+CFNMETA(100.00)
 CASE index=290
  RETURN CFNMETA(-(1/100.00))+'|'+HexBytes(BINTOC(-(1/100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/100.00)))+'|'+CFNMETA(1)+'|'+CFNMETA(100.00)
 CASE index=291
  RETURN CFNMETA(-1/100.00)+'|'+HexBytes(BINTOC(-1/100.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1/100.00))+'|'+CFNMETA(-1)+'|'+CFNMETA(100.00)
 CASE index=292
  RETURN CFNMETA(-(-1/100.00))+'|'+HexBytes(BINTOC(-(-1/100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/100.00)))+'|'+CFNMETA(-1)+'|'+CFNMETA(100.00)
 CASE index=293
  RETURN CFNMETA(1/-100.00)+'|'+HexBytes(BINTOC(1/-100.00,'BRS'))+'|'+HexBytes(TRANSFORM(1/-100.00))+'|'+CFNMETA(1)+'|'+CFNMETA(-100.00)
 CASE index=294
  RETURN CFNMETA(-(1/-100.00))+'|'+HexBytes(BINTOC(-(1/-100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1/-100.00)))+'|'+CFNMETA(1)+'|'+CFNMETA(-100.00)
 CASE index=295
  RETURN CFNMETA(-1/-100.00)+'|'+HexBytes(BINTOC(-1/-100.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1/-100.00))+'|'+CFNMETA(-1)+'|'+CFNMETA(-100.00)
 CASE index=296
  RETURN CFNMETA(-(-1/-100.00))+'|'+HexBytes(BINTOC(-(-1/-100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1/-100.00)))+'|'+CFNMETA(-1)+'|'+CFNMETA(-100.00)
 CASE index=297
  RETURN CFNMETA(1.00/100.00)+'|'+HexBytes(BINTOC(1.00/100.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.00/100.00))+'|'+CFNMETA(1.00)+'|'+CFNMETA(100.00)
 CASE index=298
  RETURN CFNMETA(-(1.00/100.00))+'|'+HexBytes(BINTOC(-(1.00/100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00/100.00)))+'|'+CFNMETA(1.00)+'|'+CFNMETA(100.00)
 CASE index=299
  RETURN CFNMETA(-1.00/100.00)+'|'+HexBytes(BINTOC(-1.00/100.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00/100.00))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(100.00)
 CASE index=300
  RETURN CFNMETA(-(-1.00/100.00))+'|'+HexBytes(BINTOC(-(-1.00/100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.00/100.00)))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(100.00)
 CASE index=301
  RETURN CFNMETA(1.00/-100.00)+'|'+HexBytes(BINTOC(1.00/-100.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.00/-100.00))+'|'+CFNMETA(1.00)+'|'+CFNMETA(-100.00)
 CASE index=302
  RETURN CFNMETA(-(1.00/-100.00))+'|'+HexBytes(BINTOC(-(1.00/-100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.00/-100.00)))+'|'+CFNMETA(1.00)+'|'+CFNMETA(-100.00)
 CASE index=303
  RETURN CFNMETA(-1.00/-100.00)+'|'+HexBytes(BINTOC(-1.00/-100.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1.00/-100.00))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(-100.00)
 CASE index=304
  RETURN CFNMETA(-(-1.00/-100.00))+'|'+HexBytes(BINTOC(-(-1.00/-100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.00/-100.00)))+'|'+CFNMETA(-1.00)+'|'+CFNMETA(-100.00)
 CASE index=305
  RETURN CFNMETA(1.000000/100.00)+'|'+HexBytes(BINTOC(1.000000/100.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000/100.00))+'|'+CFNMETA(1.000000)+'|'+CFNMETA(100.00)
 CASE index=306
  RETURN CFNMETA(-(1.000000/100.00))+'|'+HexBytes(BINTOC(-(1.000000/100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.000000/100.00)))+'|'+CFNMETA(1.000000)+'|'+CFNMETA(100.00)
 CASE index=307
  RETURN CFNMETA(-1.000000/100.00)+'|'+HexBytes(BINTOC(-1.000000/100.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000/100.00))+'|'+CFNMETA(-1.000000)+'|'+CFNMETA(100.00)
 CASE index=308
  RETURN CFNMETA(-(-1.000000/100.00))+'|'+HexBytes(BINTOC(-(-1.000000/100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.000000/100.00)))+'|'+CFNMETA(-1.000000)+'|'+CFNMETA(100.00)
 CASE index=309
  RETURN CFNMETA(1.000000/-100.00)+'|'+HexBytes(BINTOC(1.000000/-100.00,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000/-100.00))+'|'+CFNMETA(1.000000)+'|'+CFNMETA(-100.00)
 CASE index=310
  RETURN CFNMETA(-(1.000000/-100.00))+'|'+HexBytes(BINTOC(-(1.000000/-100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.000000/-100.00)))+'|'+CFNMETA(1.000000)+'|'+CFNMETA(-100.00)
 CASE index=311
  RETURN CFNMETA(-1.000000/-100.00)+'|'+HexBytes(BINTOC(-1.000000/-100.00,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000/-100.00))+'|'+CFNMETA(-1.000000)+'|'+CFNMETA(-100.00)
 CASE index=312
  RETURN CFNMETA(-(-1.000000/-100.00))+'|'+HexBytes(BINTOC(-(-1.000000/-100.00),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.000000/-100.00)))+'|'+CFNMETA(-1.000000)+'|'+CFNMETA(-100.00)
 CASE index=313
  RETURN CFNMETA(1.000000000000000000/100.000000000000000000)+'|'+HexBytes(BINTOC(1.000000000000000000/100.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000000000000000/100.000000000000000000))+'|'+CFNMETA(1.000000000000000000)+'|'+CFNMETA(100.000000000000000000)
 CASE index=314
  RETURN CFNMETA(-(1.000000000000000000/100.000000000000000000))+'|'+HexBytes(BINTOC(-(1.000000000000000000/100.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.000000000000000000/100.000000000000000000)))+'|'+CFNMETA(1.000000000000000000)+'|'+CFNMETA(100.000000000000000000)
 CASE index=315
  RETURN CFNMETA(-1.000000000000000000/100.000000000000000000)+'|'+HexBytes(BINTOC(-1.000000000000000000/100.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000000000000000/100.000000000000000000))+'|'+CFNMETA(-1.000000000000000000)+'|'+CFNMETA(100.000000000000000000)
 CASE index=316
  RETURN CFNMETA(-(-1.000000000000000000/100.000000000000000000))+'|'+HexBytes(BINTOC(-(-1.000000000000000000/100.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.000000000000000000/100.000000000000000000)))+'|'+CFNMETA(-1.000000000000000000)+'|'+CFNMETA(100.000000000000000000)
 CASE index=317
  RETURN CFNMETA(1.000000000000000000/-100.000000000000000000)+'|'+HexBytes(BINTOC(1.000000000000000000/-100.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(1.000000000000000000/-100.000000000000000000))+'|'+CFNMETA(1.000000000000000000)+'|'+CFNMETA(-100.000000000000000000)
 CASE index=318
  RETURN CFNMETA(-(1.000000000000000000/-100.000000000000000000))+'|'+HexBytes(BINTOC(-(1.000000000000000000/-100.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(1.000000000000000000/-100.000000000000000000)))+'|'+CFNMETA(1.000000000000000000)+'|'+CFNMETA(-100.000000000000000000)
 CASE index=319
  RETURN CFNMETA(-1.000000000000000000/-100.000000000000000000)+'|'+HexBytes(BINTOC(-1.000000000000000000/-100.000000000000000000,'BRS'))+'|'+HexBytes(TRANSFORM(-1.000000000000000000/-100.000000000000000000))+'|'+CFNMETA(-1.000000000000000000)+'|'+CFNMETA(-100.000000000000000000)
 CASE index=320
  RETURN CFNMETA(-(-1.000000000000000000/-100.000000000000000000))+'|'+HexBytes(BINTOC(-(-1.000000000000000000/-100.000000000000000000),'BRS'))+'|'+HexBytes(TRANSFORM(-(-1.000000000000000000/-100.000000000000000000)))+'|'+CFNMETA(-1.000000000000000000)+'|'+CFNMETA(-100.000000000000000000)
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
