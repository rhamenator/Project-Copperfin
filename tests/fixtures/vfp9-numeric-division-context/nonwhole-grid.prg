* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001: complete-expression native observations.
LOCAL ARRAY expressions[578],settings[5]
expressions[1]='(0.5/2)'
expressions[2]='+((0.5/2))'
expressions[3]='-((0.5/2))'
expressions[4]='-(-((0.5/2)))'
expressions[5]='IIF(.T.,(0.5/2),0)'
expressions[6]='-(IIF(.T.,(0.5/2),0))'
expressions[7]='EVALUATE("(0.5/2)")'
expressions[8]='-(EVALUATE("(0.5/2)"))'
expressions[9]='(-(0.5)/2)'
expressions[10]='+((-(0.5)/2))'
expressions[11]='-((-(0.5)/2))'
expressions[12]='-(-((-(0.5)/2)))'
expressions[13]='IIF(.T.,(-(0.5)/2),0)'
expressions[14]='-(IIF(.T.,(-(0.5)/2),0))'
expressions[15]='EVALUATE("(-(0.5)/2)")'
expressions[16]='-(EVALUATE("(-(0.5)/2)"))'
expressions[17]='(0.5/-(2))'
expressions[18]='+((0.5/-(2)))'
expressions[19]='-((0.5/-(2)))'
expressions[20]='-(-((0.5/-(2))))'
expressions[21]='IIF(.T.,(0.5/-(2)),0)'
expressions[22]='-(IIF(.T.,(0.5/-(2)),0))'
expressions[23]='EVALUATE("(0.5/-(2))")'
expressions[24]='-(EVALUATE("(0.5/-(2))"))'
expressions[25]='(-(0.5)/-(2))'
expressions[26]='+((-(0.5)/-(2)))'
expressions[27]='-((-(0.5)/-(2)))'
expressions[28]='-(-((-(0.5)/-(2))))'
expressions[29]='IIF(.T.,(-(0.5)/-(2)),0)'
expressions[30]='-(IIF(.T.,(-(0.5)/-(2)),0))'
expressions[31]='EVALUATE("(-(0.5)/-(2))")'
expressions[32]='-(EVALUATE("(-(0.5)/-(2))"))'
expressions[33]='(0.5/2.5)'
expressions[34]='+((0.5/2.5))'
expressions[35]='-((0.5/2.5))'
expressions[36]='-(-((0.5/2.5)))'
expressions[37]='IIF(.T.,(0.5/2.5),0)'
expressions[38]='-(IIF(.T.,(0.5/2.5),0))'
expressions[39]='EVALUATE("(0.5/2.5)")'
expressions[40]='-(EVALUATE("(0.5/2.5)"))'
expressions[41]='(-(0.5)/2.5)'
expressions[42]='+((-(0.5)/2.5))'
expressions[43]='-((-(0.5)/2.5))'
expressions[44]='-(-((-(0.5)/2.5)))'
expressions[45]='IIF(.T.,(-(0.5)/2.5),0)'
expressions[46]='-(IIF(.T.,(-(0.5)/2.5),0))'
expressions[47]='EVALUATE("(-(0.5)/2.5)")'
expressions[48]='-(EVALUATE("(-(0.5)/2.5)"))'
expressions[49]='(0.5/-(2.5))'
expressions[50]='+((0.5/-(2.5)))'
expressions[51]='-((0.5/-(2.5)))'
expressions[52]='-(-((0.5/-(2.5))))'
expressions[53]='IIF(.T.,(0.5/-(2.5)),0)'
expressions[54]='-(IIF(.T.,(0.5/-(2.5)),0))'
expressions[55]='EVALUATE("(0.5/-(2.5))")'
expressions[56]='-(EVALUATE("(0.5/-(2.5))"))'
expressions[57]='(-(0.5)/-(2.5))'
expressions[58]='+((-(0.5)/-(2.5)))'
expressions[59]='-((-(0.5)/-(2.5)))'
expressions[60]='-(-((-(0.5)/-(2.5))))'
expressions[61]='IIF(.T.,(-(0.5)/-(2.5)),0)'
expressions[62]='-(IIF(.T.,(-(0.5)/-(2.5)),0))'
expressions[63]='EVALUATE("(-(0.5)/-(2.5))")'
expressions[64]='-(EVALUATE("(-(0.5)/-(2.5))"))'
expressions[65]='(0.5/02.50)'
expressions[66]='+((0.5/02.50))'
expressions[67]='-((0.5/02.50))'
expressions[68]='-(-((0.5/02.50)))'
expressions[69]='IIF(.T.,(0.5/02.50),0)'
expressions[70]='-(IIF(.T.,(0.5/02.50),0))'
expressions[71]='EVALUATE("(0.5/02.50)")'
expressions[72]='-(EVALUATE("(0.5/02.50)"))'
expressions[73]='(-(0.5)/02.50)'
expressions[74]='+((-(0.5)/02.50))'
expressions[75]='-((-(0.5)/02.50))'
expressions[76]='-(-((-(0.5)/02.50)))'
expressions[77]='IIF(.T.,(-(0.5)/02.50),0)'
expressions[78]='-(IIF(.T.,(-(0.5)/02.50),0))'
expressions[79]='EVALUATE("(-(0.5)/02.50)")'
expressions[80]='-(EVALUATE("(-(0.5)/02.50)"))'
expressions[81]='(0.5/-(02.50))'
expressions[82]='+((0.5/-(02.50)))'
expressions[83]='-((0.5/-(02.50)))'
expressions[84]='-(-((0.5/-(02.50))))'
expressions[85]='IIF(.T.,(0.5/-(02.50)),0)'
expressions[86]='-(IIF(.T.,(0.5/-(02.50)),0))'
expressions[87]='EVALUATE("(0.5/-(02.50))")'
expressions[88]='-(EVALUATE("(0.5/-(02.50))"))'
expressions[89]='(-(0.5)/-(02.50))'
expressions[90]='+((-(0.5)/-(02.50)))'
expressions[91]='-((-(0.5)/-(02.50)))'
expressions[92]='-(-((-(0.5)/-(02.50))))'
expressions[93]='IIF(.T.,(-(0.5)/-(02.50)),0)'
expressions[94]='-(IIF(.T.,(-(0.5)/-(02.50)),0))'
expressions[95]='EVALUATE("(-(0.5)/-(02.50))")'
expressions[96]='-(EVALUATE("(-(0.5)/-(02.50))"))'
expressions[97]='(2.50/2)'
expressions[98]='+((2.50/2))'
expressions[99]='-((2.50/2))'
expressions[100]='-(-((2.50/2)))'
expressions[101]='IIF(.T.,(2.50/2),0)'
expressions[102]='-(IIF(.T.,(2.50/2),0))'
expressions[103]='EVALUATE("(2.50/2)")'
expressions[104]='-(EVALUATE("(2.50/2)"))'
expressions[105]='(-(2.50)/2)'
expressions[106]='+((-(2.50)/2))'
expressions[107]='-((-(2.50)/2))'
expressions[108]='-(-((-(2.50)/2)))'
expressions[109]='IIF(.T.,(-(2.50)/2),0)'
expressions[110]='-(IIF(.T.,(-(2.50)/2),0))'
expressions[111]='EVALUATE("(-(2.50)/2)")'
expressions[112]='-(EVALUATE("(-(2.50)/2)"))'
expressions[113]='(2.50/-(2))'
expressions[114]='+((2.50/-(2)))'
expressions[115]='-((2.50/-(2)))'
expressions[116]='-(-((2.50/-(2))))'
expressions[117]='IIF(.T.,(2.50/-(2)),0)'
expressions[118]='-(IIF(.T.,(2.50/-(2)),0))'
expressions[119]='EVALUATE("(2.50/-(2))")'
expressions[120]='-(EVALUATE("(2.50/-(2))"))'
expressions[121]='(-(2.50)/-(2))'
expressions[122]='+((-(2.50)/-(2)))'
expressions[123]='-((-(2.50)/-(2)))'
expressions[124]='-(-((-(2.50)/-(2))))'
expressions[125]='IIF(.T.,(-(2.50)/-(2)),0)'
expressions[126]='-(IIF(.T.,(-(2.50)/-(2)),0))'
expressions[127]='EVALUATE("(-(2.50)/-(2))")'
expressions[128]='-(EVALUATE("(-(2.50)/-(2))"))'
expressions[129]='(2.50/2.5)'
expressions[130]='+((2.50/2.5))'
expressions[131]='-((2.50/2.5))'
expressions[132]='-(-((2.50/2.5)))'
expressions[133]='IIF(.T.,(2.50/2.5),0)'
expressions[134]='-(IIF(.T.,(2.50/2.5),0))'
expressions[135]='EVALUATE("(2.50/2.5)")'
expressions[136]='-(EVALUATE("(2.50/2.5)"))'
expressions[137]='(-(2.50)/2.5)'
expressions[138]='+((-(2.50)/2.5))'
expressions[139]='-((-(2.50)/2.5))'
expressions[140]='-(-((-(2.50)/2.5)))'
expressions[141]='IIF(.T.,(-(2.50)/2.5),0)'
expressions[142]='-(IIF(.T.,(-(2.50)/2.5),0))'
expressions[143]='EVALUATE("(-(2.50)/2.5)")'
expressions[144]='-(EVALUATE("(-(2.50)/2.5)"))'
expressions[145]='(2.50/-(2.5))'
expressions[146]='+((2.50/-(2.5)))'
expressions[147]='-((2.50/-(2.5)))'
expressions[148]='-(-((2.50/-(2.5))))'
expressions[149]='IIF(.T.,(2.50/-(2.5)),0)'
expressions[150]='-(IIF(.T.,(2.50/-(2.5)),0))'
expressions[151]='EVALUATE("(2.50/-(2.5))")'
expressions[152]='-(EVALUATE("(2.50/-(2.5))"))'
expressions[153]='(-(2.50)/-(2.5))'
expressions[154]='+((-(2.50)/-(2.5)))'
expressions[155]='-((-(2.50)/-(2.5)))'
expressions[156]='-(-((-(2.50)/-(2.5))))'
expressions[157]='IIF(.T.,(-(2.50)/-(2.5)),0)'
expressions[158]='-(IIF(.T.,(-(2.50)/-(2.5)),0))'
expressions[159]='EVALUATE("(-(2.50)/-(2.5))")'
expressions[160]='-(EVALUATE("(-(2.50)/-(2.5))"))'
expressions[161]='(2.50/02.50)'
expressions[162]='+((2.50/02.50))'
expressions[163]='-((2.50/02.50))'
expressions[164]='-(-((2.50/02.50)))'
expressions[165]='IIF(.T.,(2.50/02.50),0)'
expressions[166]='-(IIF(.T.,(2.50/02.50),0))'
expressions[167]='EVALUATE("(2.50/02.50)")'
expressions[168]='-(EVALUATE("(2.50/02.50)"))'
expressions[169]='(-(2.50)/02.50)'
expressions[170]='+((-(2.50)/02.50))'
expressions[171]='-((-(2.50)/02.50))'
expressions[172]='-(-((-(2.50)/02.50)))'
expressions[173]='IIF(.T.,(-(2.50)/02.50),0)'
expressions[174]='-(IIF(.T.,(-(2.50)/02.50),0))'
expressions[175]='EVALUATE("(-(2.50)/02.50)")'
expressions[176]='-(EVALUATE("(-(2.50)/02.50)"))'
expressions[177]='(2.50/-(02.50))'
expressions[178]='+((2.50/-(02.50)))'
expressions[179]='-((2.50/-(02.50)))'
expressions[180]='-(-((2.50/-(02.50))))'
expressions[181]='IIF(.T.,(2.50/-(02.50)),0)'
expressions[182]='-(IIF(.T.,(2.50/-(02.50)),0))'
expressions[183]='EVALUATE("(2.50/-(02.50))")'
expressions[184]='-(EVALUATE("(2.50/-(02.50))"))'
expressions[185]='(-(2.50)/-(02.50))'
expressions[186]='+((-(2.50)/-(02.50)))'
expressions[187]='-((-(2.50)/-(02.50)))'
expressions[188]='-(-((-(2.50)/-(02.50))))'
expressions[189]='IIF(.T.,(-(2.50)/-(02.50)),0)'
expressions[190]='-(IIF(.T.,(-(2.50)/-(02.50)),0))'
expressions[191]='EVALUATE("(-(2.50)/-(02.50))")'
expressions[192]='-(EVALUATE("(-(2.50)/-(02.50))"))'
expressions[193]='(0.000000000000000001/2)'
expressions[194]='+((0.000000000000000001/2))'
expressions[195]='-((0.000000000000000001/2))'
expressions[196]='-(-((0.000000000000000001/2)))'
expressions[197]='IIF(.T.,(0.000000000000000001/2),0)'
expressions[198]='-(IIF(.T.,(0.000000000000000001/2),0))'
expressions[199]='EVALUATE("(0.000000000000000001/2)")'
expressions[200]='-(EVALUATE("(0.000000000000000001/2)"))'
expressions[201]='(-(0.000000000000000001)/2)'
expressions[202]='+((-(0.000000000000000001)/2))'
expressions[203]='-((-(0.000000000000000001)/2))'
expressions[204]='-(-((-(0.000000000000000001)/2)))'
expressions[205]='IIF(.T.,(-(0.000000000000000001)/2),0)'
expressions[206]='-(IIF(.T.,(-(0.000000000000000001)/2),0))'
expressions[207]='EVALUATE("(-(0.000000000000000001)/2)")'
expressions[208]='-(EVALUATE("(-(0.000000000000000001)/2)"))'
expressions[209]='(0.000000000000000001/-(2))'
expressions[210]='+((0.000000000000000001/-(2)))'
expressions[211]='-((0.000000000000000001/-(2)))'
expressions[212]='-(-((0.000000000000000001/-(2))))'
expressions[213]='IIF(.T.,(0.000000000000000001/-(2)),0)'
expressions[214]='-(IIF(.T.,(0.000000000000000001/-(2)),0))'
expressions[215]='EVALUATE("(0.000000000000000001/-(2))")'
expressions[216]='-(EVALUATE("(0.000000000000000001/-(2))"))'
expressions[217]='(-(0.000000000000000001)/-(2))'
expressions[218]='+((-(0.000000000000000001)/-(2)))'
expressions[219]='-((-(0.000000000000000001)/-(2)))'
expressions[220]='-(-((-(0.000000000000000001)/-(2))))'
expressions[221]='IIF(.T.,(-(0.000000000000000001)/-(2)),0)'
expressions[222]='-(IIF(.T.,(-(0.000000000000000001)/-(2)),0))'
expressions[223]='EVALUATE("(-(0.000000000000000001)/-(2))")'
expressions[224]='-(EVALUATE("(-(0.000000000000000001)/-(2))"))'
expressions[225]='(0.000000000000000001/2.5)'
expressions[226]='+((0.000000000000000001/2.5))'
expressions[227]='-((0.000000000000000001/2.5))'
expressions[228]='-(-((0.000000000000000001/2.5)))'
expressions[229]='IIF(.T.,(0.000000000000000001/2.5),0)'
expressions[230]='-(IIF(.T.,(0.000000000000000001/2.5),0))'
expressions[231]='EVALUATE("(0.000000000000000001/2.5)")'
expressions[232]='-(EVALUATE("(0.000000000000000001/2.5)"))'
expressions[233]='(-(0.000000000000000001)/2.5)'
expressions[234]='+((-(0.000000000000000001)/2.5))'
expressions[235]='-((-(0.000000000000000001)/2.5))'
expressions[236]='-(-((-(0.000000000000000001)/2.5)))'
expressions[237]='IIF(.T.,(-(0.000000000000000001)/2.5),0)'
expressions[238]='-(IIF(.T.,(-(0.000000000000000001)/2.5),0))'
expressions[239]='EVALUATE("(-(0.000000000000000001)/2.5)")'
expressions[240]='-(EVALUATE("(-(0.000000000000000001)/2.5)"))'
expressions[241]='(0.000000000000000001/-(2.5))'
expressions[242]='+((0.000000000000000001/-(2.5)))'
expressions[243]='-((0.000000000000000001/-(2.5)))'
expressions[244]='-(-((0.000000000000000001/-(2.5))))'
expressions[245]='IIF(.T.,(0.000000000000000001/-(2.5)),0)'
expressions[246]='-(IIF(.T.,(0.000000000000000001/-(2.5)),0))'
expressions[247]='EVALUATE("(0.000000000000000001/-(2.5))")'
expressions[248]='-(EVALUATE("(0.000000000000000001/-(2.5))"))'
expressions[249]='(-(0.000000000000000001)/-(2.5))'
expressions[250]='+((-(0.000000000000000001)/-(2.5)))'
expressions[251]='-((-(0.000000000000000001)/-(2.5)))'
expressions[252]='-(-((-(0.000000000000000001)/-(2.5))))'
expressions[253]='IIF(.T.,(-(0.000000000000000001)/-(2.5)),0)'
expressions[254]='-(IIF(.T.,(-(0.000000000000000001)/-(2.5)),0))'
expressions[255]='EVALUATE("(-(0.000000000000000001)/-(2.5))")'
expressions[256]='-(EVALUATE("(-(0.000000000000000001)/-(2.5))"))'
expressions[257]='(0.000000000000000001/02.50)'
expressions[258]='+((0.000000000000000001/02.50))'
expressions[259]='-((0.000000000000000001/02.50))'
expressions[260]='-(-((0.000000000000000001/02.50)))'
expressions[261]='IIF(.T.,(0.000000000000000001/02.50),0)'
expressions[262]='-(IIF(.T.,(0.000000000000000001/02.50),0))'
expressions[263]='EVALUATE("(0.000000000000000001/02.50)")'
expressions[264]='-(EVALUATE("(0.000000000000000001/02.50)"))'
expressions[265]='(-(0.000000000000000001)/02.50)'
expressions[266]='+((-(0.000000000000000001)/02.50))'
expressions[267]='-((-(0.000000000000000001)/02.50))'
expressions[268]='-(-((-(0.000000000000000001)/02.50)))'
expressions[269]='IIF(.T.,(-(0.000000000000000001)/02.50),0)'
expressions[270]='-(IIF(.T.,(-(0.000000000000000001)/02.50),0))'
expressions[271]='EVALUATE("(-(0.000000000000000001)/02.50)")'
expressions[272]='-(EVALUATE("(-(0.000000000000000001)/02.50)"))'
expressions[273]='(0.000000000000000001/-(02.50))'
expressions[274]='+((0.000000000000000001/-(02.50)))'
expressions[275]='-((0.000000000000000001/-(02.50)))'
expressions[276]='-(-((0.000000000000000001/-(02.50))))'
expressions[277]='IIF(.T.,(0.000000000000000001/-(02.50)),0)'
expressions[278]='-(IIF(.T.,(0.000000000000000001/-(02.50)),0))'
expressions[279]='EVALUATE("(0.000000000000000001/-(02.50))")'
expressions[280]='-(EVALUATE("(0.000000000000000001/-(02.50))"))'
expressions[281]='(-(0.000000000000000001)/-(02.50))'
expressions[282]='+((-(0.000000000000000001)/-(02.50)))'
expressions[283]='-((-(0.000000000000000001)/-(02.50)))'
expressions[284]='-(-((-(0.000000000000000001)/-(02.50))))'
expressions[285]='IIF(.T.,(-(0.000000000000000001)/-(02.50)),0)'
expressions[286]='-(IIF(.T.,(-(0.000000000000000001)/-(02.50)),0))'
expressions[287]='EVALUATE("(-(0.000000000000000001)/-(02.50))")'
expressions[288]='-(EVALUATE("(-(0.000000000000000001)/-(02.50))"))'
expressions[289]='(1.234567890123456789/2)'
expressions[290]='+((1.234567890123456789/2))'
expressions[291]='-((1.234567890123456789/2))'
expressions[292]='-(-((1.234567890123456789/2)))'
expressions[293]='IIF(.T.,(1.234567890123456789/2),0)'
expressions[294]='-(IIF(.T.,(1.234567890123456789/2),0))'
expressions[295]='EVALUATE("(1.234567890123456789/2)")'
expressions[296]='-(EVALUATE("(1.234567890123456789/2)"))'
expressions[297]='(-(1.234567890123456789)/2)'
expressions[298]='+((-(1.234567890123456789)/2))'
expressions[299]='-((-(1.234567890123456789)/2))'
expressions[300]='-(-((-(1.234567890123456789)/2)))'
expressions[301]='IIF(.T.,(-(1.234567890123456789)/2),0)'
expressions[302]='-(IIF(.T.,(-(1.234567890123456789)/2),0))'
expressions[303]='EVALUATE("(-(1.234567890123456789)/2)")'
expressions[304]='-(EVALUATE("(-(1.234567890123456789)/2)"))'
expressions[305]='(1.234567890123456789/-(2))'
expressions[306]='+((1.234567890123456789/-(2)))'
expressions[307]='-((1.234567890123456789/-(2)))'
expressions[308]='-(-((1.234567890123456789/-(2))))'
expressions[309]='IIF(.T.,(1.234567890123456789/-(2)),0)'
expressions[310]='-(IIF(.T.,(1.234567890123456789/-(2)),0))'
expressions[311]='EVALUATE("(1.234567890123456789/-(2))")'
expressions[312]='-(EVALUATE("(1.234567890123456789/-(2))"))'
expressions[313]='(-(1.234567890123456789)/-(2))'
expressions[314]='+((-(1.234567890123456789)/-(2)))'
expressions[315]='-((-(1.234567890123456789)/-(2)))'
expressions[316]='-(-((-(1.234567890123456789)/-(2))))'
expressions[317]='IIF(.T.,(-(1.234567890123456789)/-(2)),0)'
expressions[318]='-(IIF(.T.,(-(1.234567890123456789)/-(2)),0))'
expressions[319]='EVALUATE("(-(1.234567890123456789)/-(2))")'
expressions[320]='-(EVALUATE("(-(1.234567890123456789)/-(2))"))'
expressions[321]='(1.234567890123456789/2.5)'
expressions[322]='+((1.234567890123456789/2.5))'
expressions[323]='-((1.234567890123456789/2.5))'
expressions[324]='-(-((1.234567890123456789/2.5)))'
expressions[325]='IIF(.T.,(1.234567890123456789/2.5),0)'
expressions[326]='-(IIF(.T.,(1.234567890123456789/2.5),0))'
expressions[327]='EVALUATE("(1.234567890123456789/2.5)")'
expressions[328]='-(EVALUATE("(1.234567890123456789/2.5)"))'
expressions[329]='(-(1.234567890123456789)/2.5)'
expressions[330]='+((-(1.234567890123456789)/2.5))'
expressions[331]='-((-(1.234567890123456789)/2.5))'
expressions[332]='-(-((-(1.234567890123456789)/2.5)))'
expressions[333]='IIF(.T.,(-(1.234567890123456789)/2.5),0)'
expressions[334]='-(IIF(.T.,(-(1.234567890123456789)/2.5),0))'
expressions[335]='EVALUATE("(-(1.234567890123456789)/2.5)")'
expressions[336]='-(EVALUATE("(-(1.234567890123456789)/2.5)"))'
expressions[337]='(1.234567890123456789/-(2.5))'
expressions[338]='+((1.234567890123456789/-(2.5)))'
expressions[339]='-((1.234567890123456789/-(2.5)))'
expressions[340]='-(-((1.234567890123456789/-(2.5))))'
expressions[341]='IIF(.T.,(1.234567890123456789/-(2.5)),0)'
expressions[342]='-(IIF(.T.,(1.234567890123456789/-(2.5)),0))'
expressions[343]='EVALUATE("(1.234567890123456789/-(2.5))")'
expressions[344]='-(EVALUATE("(1.234567890123456789/-(2.5))"))'
expressions[345]='(-(1.234567890123456789)/-(2.5))'
expressions[346]='+((-(1.234567890123456789)/-(2.5)))'
expressions[347]='-((-(1.234567890123456789)/-(2.5)))'
expressions[348]='-(-((-(1.234567890123456789)/-(2.5))))'
expressions[349]='IIF(.T.,(-(1.234567890123456789)/-(2.5)),0)'
expressions[350]='-(IIF(.T.,(-(1.234567890123456789)/-(2.5)),0))'
expressions[351]='EVALUATE("(-(1.234567890123456789)/-(2.5))")'
expressions[352]='-(EVALUATE("(-(1.234567890123456789)/-(2.5))"))'
expressions[353]='(1.234567890123456789/02.50)'
expressions[354]='+((1.234567890123456789/02.50))'
expressions[355]='-((1.234567890123456789/02.50))'
expressions[356]='-(-((1.234567890123456789/02.50)))'
expressions[357]='IIF(.T.,(1.234567890123456789/02.50),0)'
expressions[358]='-(IIF(.T.,(1.234567890123456789/02.50),0))'
expressions[359]='EVALUATE("(1.234567890123456789/02.50)")'
expressions[360]='-(EVALUATE("(1.234567890123456789/02.50)"))'
expressions[361]='(-(1.234567890123456789)/02.50)'
expressions[362]='+((-(1.234567890123456789)/02.50))'
expressions[363]='-((-(1.234567890123456789)/02.50))'
expressions[364]='-(-((-(1.234567890123456789)/02.50)))'
expressions[365]='IIF(.T.,(-(1.234567890123456789)/02.50),0)'
expressions[366]='-(IIF(.T.,(-(1.234567890123456789)/02.50),0))'
expressions[367]='EVALUATE("(-(1.234567890123456789)/02.50)")'
expressions[368]='-(EVALUATE("(-(1.234567890123456789)/02.50)"))'
expressions[369]='(1.234567890123456789/-(02.50))'
expressions[370]='+((1.234567890123456789/-(02.50)))'
expressions[371]='-((1.234567890123456789/-(02.50)))'
expressions[372]='-(-((1.234567890123456789/-(02.50))))'
expressions[373]='IIF(.T.,(1.234567890123456789/-(02.50)),0)'
expressions[374]='-(IIF(.T.,(1.234567890123456789/-(02.50)),0))'
expressions[375]='EVALUATE("(1.234567890123456789/-(02.50))")'
expressions[376]='-(EVALUATE("(1.234567890123456789/-(02.50))"))'
expressions[377]='(-(1.234567890123456789)/-(02.50))'
expressions[378]='+((-(1.234567890123456789)/-(02.50)))'
expressions[379]='-((-(1.234567890123456789)/-(02.50)))'
expressions[380]='-(-((-(1.234567890123456789)/-(02.50))))'
expressions[381]='IIF(.T.,(-(1.234567890123456789)/-(02.50)),0)'
expressions[382]='-(IIF(.T.,(-(1.234567890123456789)/-(02.50)),0))'
expressions[383]='EVALUATE("(-(1.234567890123456789)/-(02.50))")'
expressions[384]='-(EVALUATE("(-(1.234567890123456789)/-(02.50))"))'
expressions[385]='((5/2)/(5/2))'
expressions[386]='+(((5/2)/(5/2)))'
expressions[387]='-(((5/2)/(5/2)))'
expressions[388]='-(-(((5/2)/(5/2))))'
expressions[389]='IIF(.T.,((5/2)/(5/2)),0)'
expressions[390]='-(IIF(.T.,((5/2)/(5/2)),0))'
expressions[391]='EVALUATE("((5/2)/(5/2))")'
expressions[392]='-(EVALUATE("((5/2)/(5/2))"))'
expressions[393]='((1/7)/3)'
expressions[394]='+(((1/7)/3))'
expressions[395]='-(((1/7)/3))'
expressions[396]='-(-(((1/7)/3)))'
expressions[397]='IIF(.T.,((1/7)/3),0)'
expressions[398]='-(IIF(.T.,((1/7)/3),0))'
expressions[399]='EVALUATE("((1/7)/3)")'
expressions[400]='-(EVALUATE("((1/7)/3)"))'
expressions[401]='(5/(5/2))'
expressions[402]='+((5/(5/2)))'
expressions[403]='-((5/(5/2)))'
expressions[404]='-(-((5/(5/2))))'
expressions[405]='IIF(.T.,(5/(5/2)),0)'
expressions[406]='-(IIF(.T.,(5/(5/2)),0))'
expressions[407]='EVALUATE("(5/(5/2))")'
expressions[408]='-(EVALUATE("(5/(5/2))"))'
expressions[409]='((2/3)/7)'
expressions[410]='+(((2/3)/7))'
expressions[411]='-(((2/3)/7))'
expressions[412]='-(-(((2/3)/7)))'
expressions[413]='IIF(.T.,((2/3)/7),0)'
expressions[414]='-(IIF(.T.,((2/3)/7),0))'
expressions[415]='EVALUATE("((2/3)/7)")'
expressions[416]='-(EVALUATE("((2/3)/7)"))'
expressions[417]='((1/3)/7)'
expressions[418]='+(((1/3)/7))'
expressions[419]='-(((1/3)/7))'
expressions[420]='-(-(((1/3)/7)))'
expressions[421]='IIF(.T.,((1/3)/7),0)'
expressions[422]='-(IIF(.T.,((1/3)/7),0))'
expressions[423]='EVALUATE("((1/3)/7)")'
expressions[424]='-(EVALUATE("((1/3)/7)"))'
expressions[425]='((1/3)/(7/3))'
expressions[426]='+(((1/3)/(7/3)))'
expressions[427]='-(((1/3)/(7/3)))'
expressions[428]='-(-(((1/3)/(7/3))))'
expressions[429]='IIF(.T.,((1/3)/(7/3)),0)'
expressions[430]='-(IIF(.T.,((1/3)/(7/3)),0))'
expressions[431]='EVALUATE("((1/3)/(7/3))")'
expressions[432]='-(EVALUATE("((1/3)/(7/3))"))'
expressions[433]='(((1/7)/3)/3)'
expressions[434]='+((((1/7)/3)/3))'
expressions[435]='-((((1/7)/3)/3))'
expressions[436]='-(-((((1/7)/3)/3)))'
expressions[437]='IIF(.T.,(((1/7)/3)/3),0)'
expressions[438]='-(IIF(.T.,(((1/7)/3)/3),0))'
expressions[439]='EVALUATE("(((1/7)/3)/3)")'
expressions[440]='-(EVALUATE("(((1/7)/3)/3)"))'
expressions[441]='((5.00/2.00)/(7.0/3.0))'
expressions[442]='+(((5.00/2.00)/(7.0/3.0)))'
expressions[443]='-(((5.00/2.00)/(7.0/3.0)))'
expressions[444]='-(-(((5.00/2.00)/(7.0/3.0))))'
expressions[445]='IIF(.T.,((5.00/2.00)/(7.0/3.0)),0)'
expressions[446]='-(IIF(.T.,((5.00/2.00)/(7.0/3.0)),0))'
expressions[447]='EVALUATE("((5.00/2.00)/(7.0/3.0))")'
expressions[448]='-(EVALUATE("((5.00/2.00)/(7.0/3.0))"))'
expressions[449]='((0.5/2)/2)'
expressions[450]='+(((0.5/2)/2))'
expressions[451]='-(((0.5/2)/2))'
expressions[452]='-(-(((0.5/2)/2)))'
expressions[453]='IIF(.T.,((0.5/2)/2),0)'
expressions[454]='-(IIF(.T.,((0.5/2)/2),0))'
expressions[455]='EVALUATE("((0.5/2)/2)")'
expressions[456]='-(EVALUATE("((0.5/2)/2)"))'
expressions[457]='(((5/2)/(5/2))/3)'
expressions[458]='+((((5/2)/(5/2))/3))'
expressions[459]='-((((5/2)/(5/2))/3))'
expressions[460]='-(-((((5/2)/(5/2))/3)))'
expressions[461]='IIF(.T.,(((5/2)/(5/2))/3),0)'
expressions[462]='-(IIF(.T.,(((5/2)/(5/2))/3),0))'
expressions[463]='EVALUATE("(((5/2)/(5/2))/3)")'
expressions[464]='-(EVALUATE("(((5/2)/(5/2))/3)"))'
expressions[465]='(((5/2)/2)/(7/3))'
expressions[466]='+((((5/2)/2)/(7/3)))'
expressions[467]='-((((5/2)/2)/(7/3)))'
expressions[468]='-(-((((5/2)/2)/(7/3))))'
expressions[469]='IIF(.T.,(((5/2)/2)/(7/3)),0)'
expressions[470]='-(IIF(.T.,(((5/2)/2)/(7/3)),0))'
expressions[471]='EVALUATE("(((5/2)/2)/(7/3))")'
expressions[472]='-(EVALUATE("(((5/2)/2)/(7/3))"))'
expressions[473]='((2.50/2.5)/(5/2))'
expressions[474]='+(((2.50/2.5)/(5/2)))'
expressions[475]='-(((2.50/2.5)/(5/2)))'
expressions[476]='-(-(((2.50/2.5)/(5/2))))'
expressions[477]='IIF(.T.,((2.50/2.5)/(5/2)),0)'
expressions[478]='-(IIF(.T.,((2.50/2.5)/(5/2)),0))'
expressions[479]='EVALUATE("((2.50/2.5)/(5/2))")'
expressions[480]='-(EVALUATE("((2.50/2.5)/(5/2))"))'
expressions[481]='((-(5)/2)/(7/3))'
expressions[482]='+(((-(5)/2)/(7/3)))'
expressions[483]='-(((-(5)/2)/(7/3)))'
expressions[484]='-(-(((-(5)/2)/(7/3))))'
expressions[485]='IIF(.T.,((-(5)/2)/(7/3)),0)'
expressions[486]='-(IIF(.T.,((-(5)/2)/(7/3)),0))'
expressions[487]='EVALUATE("((-(5)/2)/(7/3))")'
expressions[488]='-(EVALUATE("((-(5)/2)/(7/3))"))'
expressions[489]='((5/-(2))/(7/3))'
expressions[490]='+(((5/-(2))/(7/3)))'
expressions[491]='-(((5/-(2))/(7/3)))'
expressions[492]='-(-(((5/-(2))/(7/3))))'
expressions[493]='IIF(.T.,((5/-(2))/(7/3)),0)'
expressions[494]='-(IIF(.T.,((5/-(2))/(7/3)),0))'
expressions[495]='EVALUATE("((5/-(2))/(7/3))")'
expressions[496]='-(EVALUATE("((5/-(2))/(7/3))"))'
expressions[497]='((5/2)/(-(7)/3))'
expressions[498]='+(((5/2)/(-(7)/3)))'
expressions[499]='-(((5/2)/(-(7)/3)))'
expressions[500]='-(-(((5/2)/(-(7)/3))))'
expressions[501]='IIF(.T.,((5/2)/(-(7)/3)),0)'
expressions[502]='-(IIF(.T.,((5/2)/(-(7)/3)),0))'
expressions[503]='EVALUATE("((5/2)/(-(7)/3))")'
expressions[504]='-(EVALUATE("((5/2)/(-(7)/3))"))'
expressions[505]='((5/2)/(7/-(3)))'
expressions[506]='+(((5/2)/(7/-(3))))'
expressions[507]='-(((5/2)/(7/-(3))))'
expressions[508]='-(-(((5/2)/(7/-(3)))))'
expressions[509]='IIF(.T.,((5/2)/(7/-(3))),0)'
expressions[510]='-(IIF(.T.,((5/2)/(7/-(3))),0))'
expressions[511]='EVALUATE("((5/2)/(7/-(3)))")'
expressions[512]='-(EVALUATE("((5/2)/(7/-(3)))"))'
expressions[513]='(1.0000000000000002/2.5)'
expressions[514]='+((1.0000000000000002/2.5))'
expressions[515]='-((1.0000000000000002/2.5))'
expressions[516]='-(-((1.0000000000000002/2.5)))'
expressions[517]='IIF(.T.,(1.0000000000000002/2.5),0)'
expressions[518]='-(IIF(.T.,(1.0000000000000002/2.5),0))'
expressions[519]='EVALUATE("(1.0000000000000002/2.5)")'
expressions[520]='-(EVALUATE("(1.0000000000000002/2.5)"))'
expressions[521]='(0.9999999999999999/2.5)'
expressions[522]='+((0.9999999999999999/2.5))'
expressions[523]='-((0.9999999999999999/2.5))'
expressions[524]='-(-((0.9999999999999999/2.5)))'
expressions[525]='IIF(.T.,(0.9999999999999999/2.5),0)'
expressions[526]='-(IIF(.T.,(0.9999999999999999/2.5),0))'
expressions[527]='EVALUATE("(0.9999999999999999/2.5)")'
expressions[528]='-(EVALUATE("(0.9999999999999999/2.5)"))'
expressions[529]='(3.9999999999999996/2.5)'
expressions[530]='+((3.9999999999999996/2.5))'
expressions[531]='-((3.9999999999999996/2.5))'
expressions[532]='-(-((3.9999999999999996/2.5)))'
expressions[533]='IIF(.T.,(3.9999999999999996/2.5),0)'
expressions[534]='-(IIF(.T.,(3.9999999999999996/2.5),0))'
expressions[535]='EVALUATE("(3.9999999999999996/2.5)")'
expressions[536]='-(EVALUATE("(3.9999999999999996/2.5)"))'
expressions[537]='(1.234567890123456789/3)'
expressions[538]='+((1.234567890123456789/3))'
expressions[539]='-((1.234567890123456789/3))'
expressions[540]='-(-((1.234567890123456789/3)))'
expressions[541]='IIF(.T.,(1.234567890123456789/3),0)'
expressions[542]='-(IIF(.T.,(1.234567890123456789/3),0))'
expressions[543]='EVALUATE("(1.234567890123456789/3)")'
expressions[544]='-(EVALUATE("(1.234567890123456789/3)"))'
expressions[545]='0.5'
expressions[546]='2'
expressions[547]='2.5'
expressions[548]='02.50'
expressions[549]='2.50'
expressions[550]='0.000000000000000001'
expressions[551]='1.234567890123456789'
expressions[552]='5'
expressions[553]='1'
expressions[554]='7'
expressions[555]='3'
expressions[556]='5.00'
expressions[557]='2.00'
expressions[558]='7.0'
expressions[559]='3.0'
expressions[560]='1.0000000000000002'
expressions[561]='0.9999999999999999'
expressions[562]='3.9999999999999996'
expressions[563]='(1/7)/iTwo'
expressions[564]='iTwo/(5/2)'
expressions[565]='((5/2)/(5/2))/iTwo'
expressions[566]='iTwo/((5/2)/(5/2))'
expressions[567]='(2.50/2)/nTwo'
expressions[568]='nTwo/(2.50/2)'
expressions[569]='-((1/7)/iTwo)'
expressions[570]='-(iTwo/(5/2))'
expressions[571]='((1/7)/3)/iTwo'
expressions[572]='((5/2)/(7/3))/iTwo'
expressions[573]='-(((5/2)/(7/3))/iTwo)'
expressions[574]='iTwo/((7/3)/(5/2))'
expressions[575]='5/0.5'
expressions[576]='(1/3)/(1/3)'
expressions[577]='1E-14/2'
expressions[578]='9999999999/2.5'
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
outputPath='Z:\home\rich\.codex\worktrees\numeric-nonwhole-division-5611\Project-Copperfin\build\nonwhole-native\grid-results.txt'
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
  RETURN CFNMETA((0.5/2))+'|'+HexBytes(BINTOC((0.5/2),'BRS'))+'|'+HexBytes(TRANSFORM((0.5/2)))
 CASE index=2
  RETURN CFNMETA(+((0.5/2)))+'|'+HexBytes(BINTOC(+((0.5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.5/2))))
 CASE index=3
  RETURN CFNMETA(-((0.5/2)))+'|'+HexBytes(BINTOC(-((0.5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.5/2))))
 CASE index=4
  RETURN CFNMETA(-(-((0.5/2))))+'|'+HexBytes(BINTOC(-(-((0.5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.5/2)))))
 CASE index=5
  RETURN CFNMETA(IIF(.T.,(0.5/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.5/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.5/2),0)))
 CASE index=6
  RETURN CFNMETA(-(IIF(.T.,(0.5/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.5/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.5/2),0))))
 CASE index=7
  RETURN CFNMETA(EVALUATE("(0.5/2)"))+'|'+HexBytes(BINTOC(EVALUATE("(0.5/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.5/2)")))
 CASE index=8
  RETURN CFNMETA(-(EVALUATE("(0.5/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.5/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.5/2)"))))
 CASE index=9
  RETURN CFNMETA((-(0.5)/2))+'|'+HexBytes(BINTOC((-(0.5)/2),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.5)/2)))
 CASE index=10
  RETURN CFNMETA(+((-(0.5)/2)))+'|'+HexBytes(BINTOC(+((-(0.5)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.5)/2))))
 CASE index=11
  RETURN CFNMETA(-((-(0.5)/2)))+'|'+HexBytes(BINTOC(-((-(0.5)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.5)/2))))
 CASE index=12
  RETURN CFNMETA(-(-((-(0.5)/2))))+'|'+HexBytes(BINTOC(-(-((-(0.5)/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.5)/2)))))
 CASE index=13
  RETURN CFNMETA(IIF(.T.,(-(0.5)/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.5)/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.5)/2),0)))
 CASE index=14
  RETURN CFNMETA(-(IIF(.T.,(-(0.5)/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.5)/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.5)/2),0))))
 CASE index=15
  RETURN CFNMETA(EVALUATE("(-(0.5)/2)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.5)/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.5)/2)")))
 CASE index=16
  RETURN CFNMETA(-(EVALUATE("(-(0.5)/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.5)/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.5)/2)"))))
 CASE index=17
  RETURN CFNMETA((0.5/-(2)))+'|'+HexBytes(BINTOC((0.5/-(2)),'BRS'))+'|'+HexBytes(TRANSFORM((0.5/-(2))))
 CASE index=18
  RETURN CFNMETA(+((0.5/-(2))))+'|'+HexBytes(BINTOC(+((0.5/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.5/-(2)))))
 CASE index=19
  RETURN CFNMETA(-((0.5/-(2))))+'|'+HexBytes(BINTOC(-((0.5/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.5/-(2)))))
 CASE index=20
  RETURN CFNMETA(-(-((0.5/-(2)))))+'|'+HexBytes(BINTOC(-(-((0.5/-(2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.5/-(2))))))
 CASE index=21
  RETURN CFNMETA(IIF(.T.,(0.5/-(2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.5/-(2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.5/-(2)),0)))
 CASE index=22
  RETURN CFNMETA(-(IIF(.T.,(0.5/-(2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.5/-(2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.5/-(2)),0))))
 CASE index=23
  RETURN CFNMETA(EVALUATE("(0.5/-(2))"))+'|'+HexBytes(BINTOC(EVALUATE("(0.5/-(2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.5/-(2))")))
 CASE index=24
  RETURN CFNMETA(-(EVALUATE("(0.5/-(2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.5/-(2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.5/-(2))"))))
 CASE index=25
  RETURN CFNMETA((-(0.5)/-(2)))+'|'+HexBytes(BINTOC((-(0.5)/-(2)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.5)/-(2))))
 CASE index=26
  RETURN CFNMETA(+((-(0.5)/-(2))))+'|'+HexBytes(BINTOC(+((-(0.5)/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.5)/-(2)))))
 CASE index=27
  RETURN CFNMETA(-((-(0.5)/-(2))))+'|'+HexBytes(BINTOC(-((-(0.5)/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.5)/-(2)))))
 CASE index=28
  RETURN CFNMETA(-(-((-(0.5)/-(2)))))+'|'+HexBytes(BINTOC(-(-((-(0.5)/-(2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.5)/-(2))))))
 CASE index=29
  RETURN CFNMETA(IIF(.T.,(-(0.5)/-(2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.5)/-(2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.5)/-(2)),0)))
 CASE index=30
  RETURN CFNMETA(-(IIF(.T.,(-(0.5)/-(2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.5)/-(2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.5)/-(2)),0))))
 CASE index=31
  RETURN CFNMETA(EVALUATE("(-(0.5)/-(2))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.5)/-(2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.5)/-(2))")))
 CASE index=32
  RETURN CFNMETA(-(EVALUATE("(-(0.5)/-(2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.5)/-(2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.5)/-(2))"))))
 CASE index=33
  RETURN CFNMETA((0.5/2.5))+'|'+HexBytes(BINTOC((0.5/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((0.5/2.5)))
 CASE index=34
  RETURN CFNMETA(+((0.5/2.5)))+'|'+HexBytes(BINTOC(+((0.5/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.5/2.5))))
 CASE index=35
  RETURN CFNMETA(-((0.5/2.5)))+'|'+HexBytes(BINTOC(-((0.5/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.5/2.5))))
 CASE index=36
  RETURN CFNMETA(-(-((0.5/2.5))))+'|'+HexBytes(BINTOC(-(-((0.5/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.5/2.5)))))
 CASE index=37
  RETURN CFNMETA(IIF(.T.,(0.5/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.5/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.5/2.5),0)))
 CASE index=38
  RETURN CFNMETA(-(IIF(.T.,(0.5/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.5/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.5/2.5),0))))
 CASE index=39
  RETURN CFNMETA(EVALUATE("(0.5/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(0.5/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.5/2.5)")))
 CASE index=40
  RETURN CFNMETA(-(EVALUATE("(0.5/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.5/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.5/2.5)"))))
 CASE index=41
  RETURN CFNMETA((-(0.5)/2.5))+'|'+HexBytes(BINTOC((-(0.5)/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.5)/2.5)))
 CASE index=42
  RETURN CFNMETA(+((-(0.5)/2.5)))+'|'+HexBytes(BINTOC(+((-(0.5)/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.5)/2.5))))
 CASE index=43
  RETURN CFNMETA(-((-(0.5)/2.5)))+'|'+HexBytes(BINTOC(-((-(0.5)/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.5)/2.5))))
 CASE index=44
  RETURN CFNMETA(-(-((-(0.5)/2.5))))+'|'+HexBytes(BINTOC(-(-((-(0.5)/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.5)/2.5)))))
 CASE index=45
  RETURN CFNMETA(IIF(.T.,(-(0.5)/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.5)/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.5)/2.5),0)))
 CASE index=46
  RETURN CFNMETA(-(IIF(.T.,(-(0.5)/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.5)/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.5)/2.5),0))))
 CASE index=47
  RETURN CFNMETA(EVALUATE("(-(0.5)/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.5)/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.5)/2.5)")))
 CASE index=48
  RETURN CFNMETA(-(EVALUATE("(-(0.5)/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.5)/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.5)/2.5)"))))
 CASE index=49
  RETURN CFNMETA((0.5/-(2.5)))+'|'+HexBytes(BINTOC((0.5/-(2.5)),'BRS'))+'|'+HexBytes(TRANSFORM((0.5/-(2.5))))
 CASE index=50
  RETURN CFNMETA(+((0.5/-(2.5))))+'|'+HexBytes(BINTOC(+((0.5/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.5/-(2.5)))))
 CASE index=51
  RETURN CFNMETA(-((0.5/-(2.5))))+'|'+HexBytes(BINTOC(-((0.5/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.5/-(2.5)))))
 CASE index=52
  RETURN CFNMETA(-(-((0.5/-(2.5)))))+'|'+HexBytes(BINTOC(-(-((0.5/-(2.5)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.5/-(2.5))))))
 CASE index=53
  RETURN CFNMETA(IIF(.T.,(0.5/-(2.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.5/-(2.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.5/-(2.5)),0)))
 CASE index=54
  RETURN CFNMETA(-(IIF(.T.,(0.5/-(2.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.5/-(2.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.5/-(2.5)),0))))
 CASE index=55
  RETURN CFNMETA(EVALUATE("(0.5/-(2.5))"))+'|'+HexBytes(BINTOC(EVALUATE("(0.5/-(2.5))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.5/-(2.5))")))
 CASE index=56
  RETURN CFNMETA(-(EVALUATE("(0.5/-(2.5))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.5/-(2.5))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.5/-(2.5))"))))
 CASE index=57
  RETURN CFNMETA((-(0.5)/-(2.5)))+'|'+HexBytes(BINTOC((-(0.5)/-(2.5)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.5)/-(2.5))))
 CASE index=58
  RETURN CFNMETA(+((-(0.5)/-(2.5))))+'|'+HexBytes(BINTOC(+((-(0.5)/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.5)/-(2.5)))))
 CASE index=59
  RETURN CFNMETA(-((-(0.5)/-(2.5))))+'|'+HexBytes(BINTOC(-((-(0.5)/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.5)/-(2.5)))))
 CASE index=60
  RETURN CFNMETA(-(-((-(0.5)/-(2.5)))))+'|'+HexBytes(BINTOC(-(-((-(0.5)/-(2.5)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.5)/-(2.5))))))
 CASE index=61
  RETURN CFNMETA(IIF(.T.,(-(0.5)/-(2.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.5)/-(2.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.5)/-(2.5)),0)))
 CASE index=62
  RETURN CFNMETA(-(IIF(.T.,(-(0.5)/-(2.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.5)/-(2.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.5)/-(2.5)),0))))
 CASE index=63
  RETURN CFNMETA(EVALUATE("(-(0.5)/-(2.5))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.5)/-(2.5))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.5)/-(2.5))")))
 CASE index=64
  RETURN CFNMETA(-(EVALUATE("(-(0.5)/-(2.5))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.5)/-(2.5))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.5)/-(2.5))"))))
 CASE index=65
  RETURN CFNMETA((0.5/02.50))+'|'+HexBytes(BINTOC((0.5/02.50),'BRS'))+'|'+HexBytes(TRANSFORM((0.5/02.50)))
 CASE index=66
  RETURN CFNMETA(+((0.5/02.50)))+'|'+HexBytes(BINTOC(+((0.5/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.5/02.50))))
 CASE index=67
  RETURN CFNMETA(-((0.5/02.50)))+'|'+HexBytes(BINTOC(-((0.5/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.5/02.50))))
 CASE index=68
  RETURN CFNMETA(-(-((0.5/02.50))))+'|'+HexBytes(BINTOC(-(-((0.5/02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.5/02.50)))))
 CASE index=69
  RETURN CFNMETA(IIF(.T.,(0.5/02.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.5/02.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.5/02.50),0)))
 CASE index=70
  RETURN CFNMETA(-(IIF(.T.,(0.5/02.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.5/02.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.5/02.50),0))))
 CASE index=71
  RETURN CFNMETA(EVALUATE("(0.5/02.50)"))+'|'+HexBytes(BINTOC(EVALUATE("(0.5/02.50)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.5/02.50)")))
 CASE index=72
  RETURN CFNMETA(-(EVALUATE("(0.5/02.50)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.5/02.50)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.5/02.50)"))))
 CASE index=73
  RETURN CFNMETA((-(0.5)/02.50))+'|'+HexBytes(BINTOC((-(0.5)/02.50),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.5)/02.50)))
 CASE index=74
  RETURN CFNMETA(+((-(0.5)/02.50)))+'|'+HexBytes(BINTOC(+((-(0.5)/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.5)/02.50))))
 CASE index=75
  RETURN CFNMETA(-((-(0.5)/02.50)))+'|'+HexBytes(BINTOC(-((-(0.5)/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.5)/02.50))))
 CASE index=76
  RETURN CFNMETA(-(-((-(0.5)/02.50))))+'|'+HexBytes(BINTOC(-(-((-(0.5)/02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.5)/02.50)))))
 CASE index=77
  RETURN CFNMETA(IIF(.T.,(-(0.5)/02.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.5)/02.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.5)/02.50),0)))
 CASE index=78
  RETURN CFNMETA(-(IIF(.T.,(-(0.5)/02.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.5)/02.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.5)/02.50),0))))
 CASE index=79
  RETURN CFNMETA(EVALUATE("(-(0.5)/02.50)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.5)/02.50)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.5)/02.50)")))
 CASE index=80
  RETURN CFNMETA(-(EVALUATE("(-(0.5)/02.50)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.5)/02.50)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.5)/02.50)"))))
 CASE index=81
  RETURN CFNMETA((0.5/-(02.50)))+'|'+HexBytes(BINTOC((0.5/-(02.50)),'BRS'))+'|'+HexBytes(TRANSFORM((0.5/-(02.50))))
 CASE index=82
  RETURN CFNMETA(+((0.5/-(02.50))))+'|'+HexBytes(BINTOC(+((0.5/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.5/-(02.50)))))
 CASE index=83
  RETURN CFNMETA(-((0.5/-(02.50))))+'|'+HexBytes(BINTOC(-((0.5/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.5/-(02.50)))))
 CASE index=84
  RETURN CFNMETA(-(-((0.5/-(02.50)))))+'|'+HexBytes(BINTOC(-(-((0.5/-(02.50)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.5/-(02.50))))))
 CASE index=85
  RETURN CFNMETA(IIF(.T.,(0.5/-(02.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.5/-(02.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.5/-(02.50)),0)))
 CASE index=86
  RETURN CFNMETA(-(IIF(.T.,(0.5/-(02.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.5/-(02.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.5/-(02.50)),0))))
 CASE index=87
  RETURN CFNMETA(EVALUATE("(0.5/-(02.50))"))+'|'+HexBytes(BINTOC(EVALUATE("(0.5/-(02.50))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.5/-(02.50))")))
 CASE index=88
  RETURN CFNMETA(-(EVALUATE("(0.5/-(02.50))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.5/-(02.50))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.5/-(02.50))"))))
 CASE index=89
  RETURN CFNMETA((-(0.5)/-(02.50)))+'|'+HexBytes(BINTOC((-(0.5)/-(02.50)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.5)/-(02.50))))
 CASE index=90
  RETURN CFNMETA(+((-(0.5)/-(02.50))))+'|'+HexBytes(BINTOC(+((-(0.5)/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.5)/-(02.50)))))
 CASE index=91
  RETURN CFNMETA(-((-(0.5)/-(02.50))))+'|'+HexBytes(BINTOC(-((-(0.5)/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.5)/-(02.50)))))
 CASE index=92
  RETURN CFNMETA(-(-((-(0.5)/-(02.50)))))+'|'+HexBytes(BINTOC(-(-((-(0.5)/-(02.50)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.5)/-(02.50))))))
 CASE index=93
  RETURN CFNMETA(IIF(.T.,(-(0.5)/-(02.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.5)/-(02.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.5)/-(02.50)),0)))
 CASE index=94
  RETURN CFNMETA(-(IIF(.T.,(-(0.5)/-(02.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.5)/-(02.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.5)/-(02.50)),0))))
 CASE index=95
  RETURN CFNMETA(EVALUATE("(-(0.5)/-(02.50))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.5)/-(02.50))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.5)/-(02.50))")))
 CASE index=96
  RETURN CFNMETA(-(EVALUATE("(-(0.5)/-(02.50))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.5)/-(02.50))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.5)/-(02.50))"))))
 CASE index=97
  RETURN CFNMETA((2.50/2))+'|'+HexBytes(BINTOC((2.50/2),'BRS'))+'|'+HexBytes(TRANSFORM((2.50/2)))
 CASE index=98
  RETURN CFNMETA(+((2.50/2)))+'|'+HexBytes(BINTOC(+((2.50/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+((2.50/2))))
 CASE index=99
  RETURN CFNMETA(-((2.50/2)))+'|'+HexBytes(BINTOC(-((2.50/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((2.50/2))))
 CASE index=100
  RETURN CFNMETA(-(-((2.50/2))))+'|'+HexBytes(BINTOC(-(-((2.50/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((2.50/2)))))
 CASE index=101
  RETURN CFNMETA(IIF(.T.,(2.50/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(2.50/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(2.50/2),0)))
 CASE index=102
  RETURN CFNMETA(-(IIF(.T.,(2.50/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(2.50/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(2.50/2),0))))
 CASE index=103
  RETURN CFNMETA(EVALUATE("(2.50/2)"))+'|'+HexBytes(BINTOC(EVALUATE("(2.50/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(2.50/2)")))
 CASE index=104
  RETURN CFNMETA(-(EVALUATE("(2.50/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(2.50/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(2.50/2)"))))
 CASE index=105
  RETURN CFNMETA((-(2.50)/2))+'|'+HexBytes(BINTOC((-(2.50)/2),'BRS'))+'|'+HexBytes(TRANSFORM((-(2.50)/2)))
 CASE index=106
  RETURN CFNMETA(+((-(2.50)/2)))+'|'+HexBytes(BINTOC(+((-(2.50)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(2.50)/2))))
 CASE index=107
  RETURN CFNMETA(-((-(2.50)/2)))+'|'+HexBytes(BINTOC(-((-(2.50)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(2.50)/2))))
 CASE index=108
  RETURN CFNMETA(-(-((-(2.50)/2))))+'|'+HexBytes(BINTOC(-(-((-(2.50)/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(2.50)/2)))))
 CASE index=109
  RETURN CFNMETA(IIF(.T.,(-(2.50)/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(2.50)/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(2.50)/2),0)))
 CASE index=110
  RETURN CFNMETA(-(IIF(.T.,(-(2.50)/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(2.50)/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(2.50)/2),0))))
 CASE index=111
  RETURN CFNMETA(EVALUATE("(-(2.50)/2)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(2.50)/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(2.50)/2)")))
 CASE index=112
  RETURN CFNMETA(-(EVALUATE("(-(2.50)/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(2.50)/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(2.50)/2)"))))
 CASE index=113
  RETURN CFNMETA((2.50/-(2)))+'|'+HexBytes(BINTOC((2.50/-(2)),'BRS'))+'|'+HexBytes(TRANSFORM((2.50/-(2))))
 CASE index=114
  RETURN CFNMETA(+((2.50/-(2))))+'|'+HexBytes(BINTOC(+((2.50/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(+((2.50/-(2)))))
 CASE index=115
  RETURN CFNMETA(-((2.50/-(2))))+'|'+HexBytes(BINTOC(-((2.50/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(-((2.50/-(2)))))
 CASE index=116
  RETURN CFNMETA(-(-((2.50/-(2)))))+'|'+HexBytes(BINTOC(-(-((2.50/-(2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((2.50/-(2))))))
 CASE index=117
  RETURN CFNMETA(IIF(.T.,(2.50/-(2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(2.50/-(2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(2.50/-(2)),0)))
 CASE index=118
  RETURN CFNMETA(-(IIF(.T.,(2.50/-(2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(2.50/-(2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(2.50/-(2)),0))))
 CASE index=119
  RETURN CFNMETA(EVALUATE("(2.50/-(2))"))+'|'+HexBytes(BINTOC(EVALUATE("(2.50/-(2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(2.50/-(2))")))
 CASE index=120
  RETURN CFNMETA(-(EVALUATE("(2.50/-(2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(2.50/-(2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(2.50/-(2))"))))
 CASE index=121
  RETURN CFNMETA((-(2.50)/-(2)))+'|'+HexBytes(BINTOC((-(2.50)/-(2)),'BRS'))+'|'+HexBytes(TRANSFORM((-(2.50)/-(2))))
 CASE index=122
  RETURN CFNMETA(+((-(2.50)/-(2))))+'|'+HexBytes(BINTOC(+((-(2.50)/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(2.50)/-(2)))))
 CASE index=123
  RETURN CFNMETA(-((-(2.50)/-(2))))+'|'+HexBytes(BINTOC(-((-(2.50)/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(2.50)/-(2)))))
 CASE index=124
  RETURN CFNMETA(-(-((-(2.50)/-(2)))))+'|'+HexBytes(BINTOC(-(-((-(2.50)/-(2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(2.50)/-(2))))))
 CASE index=125
  RETURN CFNMETA(IIF(.T.,(-(2.50)/-(2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(2.50)/-(2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(2.50)/-(2)),0)))
 CASE index=126
  RETURN CFNMETA(-(IIF(.T.,(-(2.50)/-(2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(2.50)/-(2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(2.50)/-(2)),0))))
 CASE index=127
  RETURN CFNMETA(EVALUATE("(-(2.50)/-(2))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(2.50)/-(2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(2.50)/-(2))")))
 CASE index=128
  RETURN CFNMETA(-(EVALUATE("(-(2.50)/-(2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(2.50)/-(2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(2.50)/-(2))"))))
 CASE index=129
  RETURN CFNMETA((2.50/2.5))+'|'+HexBytes(BINTOC((2.50/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((2.50/2.5)))
 CASE index=130
  RETURN CFNMETA(+((2.50/2.5)))+'|'+HexBytes(BINTOC(+((2.50/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((2.50/2.5))))
 CASE index=131
  RETURN CFNMETA(-((2.50/2.5)))+'|'+HexBytes(BINTOC(-((2.50/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((2.50/2.5))))
 CASE index=132
  RETURN CFNMETA(-(-((2.50/2.5))))+'|'+HexBytes(BINTOC(-(-((2.50/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((2.50/2.5)))))
 CASE index=133
  RETURN CFNMETA(IIF(.T.,(2.50/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(2.50/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(2.50/2.5),0)))
 CASE index=134
  RETURN CFNMETA(-(IIF(.T.,(2.50/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(2.50/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(2.50/2.5),0))))
 CASE index=135
  RETURN CFNMETA(EVALUATE("(2.50/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(2.50/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(2.50/2.5)")))
 CASE index=136
  RETURN CFNMETA(-(EVALUATE("(2.50/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(2.50/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(2.50/2.5)"))))
 CASE index=137
  RETURN CFNMETA((-(2.50)/2.5))+'|'+HexBytes(BINTOC((-(2.50)/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((-(2.50)/2.5)))
 CASE index=138
  RETURN CFNMETA(+((-(2.50)/2.5)))+'|'+HexBytes(BINTOC(+((-(2.50)/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(2.50)/2.5))))
 CASE index=139
  RETURN CFNMETA(-((-(2.50)/2.5)))+'|'+HexBytes(BINTOC(-((-(2.50)/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(2.50)/2.5))))
 CASE index=140
  RETURN CFNMETA(-(-((-(2.50)/2.5))))+'|'+HexBytes(BINTOC(-(-((-(2.50)/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(2.50)/2.5)))))
 CASE index=141
  RETURN CFNMETA(IIF(.T.,(-(2.50)/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(2.50)/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(2.50)/2.5),0)))
 CASE index=142
  RETURN CFNMETA(-(IIF(.T.,(-(2.50)/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(2.50)/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(2.50)/2.5),0))))
 CASE index=143
  RETURN CFNMETA(EVALUATE("(-(2.50)/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(2.50)/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(2.50)/2.5)")))
 CASE index=144
  RETURN CFNMETA(-(EVALUATE("(-(2.50)/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(2.50)/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(2.50)/2.5)"))))
 CASE index=145
  RETURN CFNMETA((2.50/-(2.5)))+'|'+HexBytes(BINTOC((2.50/-(2.5)),'BRS'))+'|'+HexBytes(TRANSFORM((2.50/-(2.5))))
 CASE index=146
  RETURN CFNMETA(+((2.50/-(2.5))))+'|'+HexBytes(BINTOC(+((2.50/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(+((2.50/-(2.5)))))
 CASE index=147
  RETURN CFNMETA(-((2.50/-(2.5))))+'|'+HexBytes(BINTOC(-((2.50/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((2.50/-(2.5)))))
 CASE index=148
  RETURN CFNMETA(-(-((2.50/-(2.5)))))+'|'+HexBytes(BINTOC(-(-((2.50/-(2.5)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((2.50/-(2.5))))))
 CASE index=149
  RETURN CFNMETA(IIF(.T.,(2.50/-(2.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(2.50/-(2.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(2.50/-(2.5)),0)))
 CASE index=150
  RETURN CFNMETA(-(IIF(.T.,(2.50/-(2.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(2.50/-(2.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(2.50/-(2.5)),0))))
 CASE index=151
  RETURN CFNMETA(EVALUATE("(2.50/-(2.5))"))+'|'+HexBytes(BINTOC(EVALUATE("(2.50/-(2.5))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(2.50/-(2.5))")))
 CASE index=152
  RETURN CFNMETA(-(EVALUATE("(2.50/-(2.5))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(2.50/-(2.5))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(2.50/-(2.5))"))))
 CASE index=153
  RETURN CFNMETA((-(2.50)/-(2.5)))+'|'+HexBytes(BINTOC((-(2.50)/-(2.5)),'BRS'))+'|'+HexBytes(TRANSFORM((-(2.50)/-(2.5))))
 CASE index=154
  RETURN CFNMETA(+((-(2.50)/-(2.5))))+'|'+HexBytes(BINTOC(+((-(2.50)/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(2.50)/-(2.5)))))
 CASE index=155
  RETURN CFNMETA(-((-(2.50)/-(2.5))))+'|'+HexBytes(BINTOC(-((-(2.50)/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(2.50)/-(2.5)))))
 CASE index=156
  RETURN CFNMETA(-(-((-(2.50)/-(2.5)))))+'|'+HexBytes(BINTOC(-(-((-(2.50)/-(2.5)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(2.50)/-(2.5))))))
 CASE index=157
  RETURN CFNMETA(IIF(.T.,(-(2.50)/-(2.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(2.50)/-(2.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(2.50)/-(2.5)),0)))
 CASE index=158
  RETURN CFNMETA(-(IIF(.T.,(-(2.50)/-(2.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(2.50)/-(2.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(2.50)/-(2.5)),0))))
 CASE index=159
  RETURN CFNMETA(EVALUATE("(-(2.50)/-(2.5))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(2.50)/-(2.5))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(2.50)/-(2.5))")))
 CASE index=160
  RETURN CFNMETA(-(EVALUATE("(-(2.50)/-(2.5))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(2.50)/-(2.5))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(2.50)/-(2.5))"))))
 CASE index=161
  RETURN CFNMETA((2.50/02.50))+'|'+HexBytes(BINTOC((2.50/02.50),'BRS'))+'|'+HexBytes(TRANSFORM((2.50/02.50)))
 CASE index=162
  RETURN CFNMETA(+((2.50/02.50)))+'|'+HexBytes(BINTOC(+((2.50/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(+((2.50/02.50))))
 CASE index=163
  RETURN CFNMETA(-((2.50/02.50)))+'|'+HexBytes(BINTOC(-((2.50/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((2.50/02.50))))
 CASE index=164
  RETURN CFNMETA(-(-((2.50/02.50))))+'|'+HexBytes(BINTOC(-(-((2.50/02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((2.50/02.50)))))
 CASE index=165
  RETURN CFNMETA(IIF(.T.,(2.50/02.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(2.50/02.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(2.50/02.50),0)))
 CASE index=166
  RETURN CFNMETA(-(IIF(.T.,(2.50/02.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(2.50/02.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(2.50/02.50),0))))
 CASE index=167
  RETURN CFNMETA(EVALUATE("(2.50/02.50)"))+'|'+HexBytes(BINTOC(EVALUATE("(2.50/02.50)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(2.50/02.50)")))
 CASE index=168
  RETURN CFNMETA(-(EVALUATE("(2.50/02.50)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(2.50/02.50)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(2.50/02.50)"))))
 CASE index=169
  RETURN CFNMETA((-(2.50)/02.50))+'|'+HexBytes(BINTOC((-(2.50)/02.50),'BRS'))+'|'+HexBytes(TRANSFORM((-(2.50)/02.50)))
 CASE index=170
  RETURN CFNMETA(+((-(2.50)/02.50)))+'|'+HexBytes(BINTOC(+((-(2.50)/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(2.50)/02.50))))
 CASE index=171
  RETURN CFNMETA(-((-(2.50)/02.50)))+'|'+HexBytes(BINTOC(-((-(2.50)/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(2.50)/02.50))))
 CASE index=172
  RETURN CFNMETA(-(-((-(2.50)/02.50))))+'|'+HexBytes(BINTOC(-(-((-(2.50)/02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(2.50)/02.50)))))
 CASE index=173
  RETURN CFNMETA(IIF(.T.,(-(2.50)/02.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(2.50)/02.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(2.50)/02.50),0)))
 CASE index=174
  RETURN CFNMETA(-(IIF(.T.,(-(2.50)/02.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(2.50)/02.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(2.50)/02.50),0))))
 CASE index=175
  RETURN CFNMETA(EVALUATE("(-(2.50)/02.50)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(2.50)/02.50)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(2.50)/02.50)")))
 CASE index=176
  RETURN CFNMETA(-(EVALUATE("(-(2.50)/02.50)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(2.50)/02.50)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(2.50)/02.50)"))))
 CASE index=177
  RETURN CFNMETA((2.50/-(02.50)))+'|'+HexBytes(BINTOC((2.50/-(02.50)),'BRS'))+'|'+HexBytes(TRANSFORM((2.50/-(02.50))))
 CASE index=178
  RETURN CFNMETA(+((2.50/-(02.50))))+'|'+HexBytes(BINTOC(+((2.50/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(+((2.50/-(02.50)))))
 CASE index=179
  RETURN CFNMETA(-((2.50/-(02.50))))+'|'+HexBytes(BINTOC(-((2.50/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((2.50/-(02.50)))))
 CASE index=180
  RETURN CFNMETA(-(-((2.50/-(02.50)))))+'|'+HexBytes(BINTOC(-(-((2.50/-(02.50)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((2.50/-(02.50))))))
 CASE index=181
  RETURN CFNMETA(IIF(.T.,(2.50/-(02.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(2.50/-(02.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(2.50/-(02.50)),0)))
 CASE index=182
  RETURN CFNMETA(-(IIF(.T.,(2.50/-(02.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(2.50/-(02.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(2.50/-(02.50)),0))))
 CASE index=183
  RETURN CFNMETA(EVALUATE("(2.50/-(02.50))"))+'|'+HexBytes(BINTOC(EVALUATE("(2.50/-(02.50))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(2.50/-(02.50))")))
 CASE index=184
  RETURN CFNMETA(-(EVALUATE("(2.50/-(02.50))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(2.50/-(02.50))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(2.50/-(02.50))"))))
 CASE index=185
  RETURN CFNMETA((-(2.50)/-(02.50)))+'|'+HexBytes(BINTOC((-(2.50)/-(02.50)),'BRS'))+'|'+HexBytes(TRANSFORM((-(2.50)/-(02.50))))
 CASE index=186
  RETURN CFNMETA(+((-(2.50)/-(02.50))))+'|'+HexBytes(BINTOC(+((-(2.50)/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(2.50)/-(02.50)))))
 CASE index=187
  RETURN CFNMETA(-((-(2.50)/-(02.50))))+'|'+HexBytes(BINTOC(-((-(2.50)/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(2.50)/-(02.50)))))
 CASE index=188
  RETURN CFNMETA(-(-((-(2.50)/-(02.50)))))+'|'+HexBytes(BINTOC(-(-((-(2.50)/-(02.50)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(2.50)/-(02.50))))))
 CASE index=189
  RETURN CFNMETA(IIF(.T.,(-(2.50)/-(02.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(2.50)/-(02.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(2.50)/-(02.50)),0)))
 CASE index=190
  RETURN CFNMETA(-(IIF(.T.,(-(2.50)/-(02.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(2.50)/-(02.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(2.50)/-(02.50)),0))))
 CASE index=191
  RETURN CFNMETA(EVALUATE("(-(2.50)/-(02.50))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(2.50)/-(02.50))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(2.50)/-(02.50))")))
 CASE index=192
  RETURN CFNMETA(-(EVALUATE("(-(2.50)/-(02.50))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(2.50)/-(02.50))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(2.50)/-(02.50))"))))
 CASE index=193
  RETURN CFNMETA((0.000000000000000001/2))+'|'+HexBytes(BINTOC((0.000000000000000001/2),'BRS'))+'|'+HexBytes(TRANSFORM((0.000000000000000001/2)))
 CASE index=194
  RETURN CFNMETA(+((0.000000000000000001/2)))+'|'+HexBytes(BINTOC(+((0.000000000000000001/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.000000000000000001/2))))
 CASE index=195
  RETURN CFNMETA(-((0.000000000000000001/2)))+'|'+HexBytes(BINTOC(-((0.000000000000000001/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.000000000000000001/2))))
 CASE index=196
  RETURN CFNMETA(-(-((0.000000000000000001/2))))+'|'+HexBytes(BINTOC(-(-((0.000000000000000001/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.000000000000000001/2)))))
 CASE index=197
  RETURN CFNMETA(IIF(.T.,(0.000000000000000001/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.000000000000000001/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.000000000000000001/2),0)))
 CASE index=198
  RETURN CFNMETA(-(IIF(.T.,(0.000000000000000001/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.000000000000000001/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.000000000000000001/2),0))))
 CASE index=199
  RETURN CFNMETA(EVALUATE("(0.000000000000000001/2)"))+'|'+HexBytes(BINTOC(EVALUATE("(0.000000000000000001/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.000000000000000001/2)")))
 CASE index=200
  RETURN CFNMETA(-(EVALUATE("(0.000000000000000001/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.000000000000000001/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.000000000000000001/2)"))))
 CASE index=201
  RETURN CFNMETA((-(0.000000000000000001)/2))+'|'+HexBytes(BINTOC((-(0.000000000000000001)/2),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.000000000000000001)/2)))
 CASE index=202
  RETURN CFNMETA(+((-(0.000000000000000001)/2)))+'|'+HexBytes(BINTOC(+((-(0.000000000000000001)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.000000000000000001)/2))))
 CASE index=203
  RETURN CFNMETA(-((-(0.000000000000000001)/2)))+'|'+HexBytes(BINTOC(-((-(0.000000000000000001)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.000000000000000001)/2))))
 CASE index=204
  RETURN CFNMETA(-(-((-(0.000000000000000001)/2))))+'|'+HexBytes(BINTOC(-(-((-(0.000000000000000001)/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.000000000000000001)/2)))))
 CASE index=205
  RETURN CFNMETA(IIF(.T.,(-(0.000000000000000001)/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.000000000000000001)/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.000000000000000001)/2),0)))
 CASE index=206
  RETURN CFNMETA(-(IIF(.T.,(-(0.000000000000000001)/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.000000000000000001)/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.000000000000000001)/2),0))))
 CASE index=207
  RETURN CFNMETA(EVALUATE("(-(0.000000000000000001)/2)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.000000000000000001)/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.000000000000000001)/2)")))
 CASE index=208
  RETURN CFNMETA(-(EVALUATE("(-(0.000000000000000001)/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.000000000000000001)/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.000000000000000001)/2)"))))
 CASE index=209
  RETURN CFNMETA((0.000000000000000001/-(2)))+'|'+HexBytes(BINTOC((0.000000000000000001/-(2)),'BRS'))+'|'+HexBytes(TRANSFORM((0.000000000000000001/-(2))))
 CASE index=210
  RETURN CFNMETA(+((0.000000000000000001/-(2))))+'|'+HexBytes(BINTOC(+((0.000000000000000001/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.000000000000000001/-(2)))))
 CASE index=211
  RETURN CFNMETA(-((0.000000000000000001/-(2))))+'|'+HexBytes(BINTOC(-((0.000000000000000001/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.000000000000000001/-(2)))))
 CASE index=212
  RETURN CFNMETA(-(-((0.000000000000000001/-(2)))))+'|'+HexBytes(BINTOC(-(-((0.000000000000000001/-(2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.000000000000000001/-(2))))))
 CASE index=213
  RETURN CFNMETA(IIF(.T.,(0.000000000000000001/-(2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.000000000000000001/-(2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.000000000000000001/-(2)),0)))
 CASE index=214
  RETURN CFNMETA(-(IIF(.T.,(0.000000000000000001/-(2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.000000000000000001/-(2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.000000000000000001/-(2)),0))))
 CASE index=215
  RETURN CFNMETA(EVALUATE("(0.000000000000000001/-(2))"))+'|'+HexBytes(BINTOC(EVALUATE("(0.000000000000000001/-(2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.000000000000000001/-(2))")))
 CASE index=216
  RETURN CFNMETA(-(EVALUATE("(0.000000000000000001/-(2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.000000000000000001/-(2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.000000000000000001/-(2))"))))
 CASE index=217
  RETURN CFNMETA((-(0.000000000000000001)/-(2)))+'|'+HexBytes(BINTOC((-(0.000000000000000001)/-(2)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.000000000000000001)/-(2))))
 CASE index=218
  RETURN CFNMETA(+((-(0.000000000000000001)/-(2))))+'|'+HexBytes(BINTOC(+((-(0.000000000000000001)/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.000000000000000001)/-(2)))))
 CASE index=219
  RETURN CFNMETA(-((-(0.000000000000000001)/-(2))))+'|'+HexBytes(BINTOC(-((-(0.000000000000000001)/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.000000000000000001)/-(2)))))
 CASE index=220
  RETURN CFNMETA(-(-((-(0.000000000000000001)/-(2)))))+'|'+HexBytes(BINTOC(-(-((-(0.000000000000000001)/-(2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.000000000000000001)/-(2))))))
 CASE index=221
  RETURN CFNMETA(IIF(.T.,(-(0.000000000000000001)/-(2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.000000000000000001)/-(2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.000000000000000001)/-(2)),0)))
 CASE index=222
  RETURN CFNMETA(-(IIF(.T.,(-(0.000000000000000001)/-(2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.000000000000000001)/-(2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.000000000000000001)/-(2)),0))))
 CASE index=223
  RETURN CFNMETA(EVALUATE("(-(0.000000000000000001)/-(2))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.000000000000000001)/-(2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.000000000000000001)/-(2))")))
 CASE index=224
  RETURN CFNMETA(-(EVALUATE("(-(0.000000000000000001)/-(2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.000000000000000001)/-(2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.000000000000000001)/-(2))"))))
 CASE index=225
  RETURN CFNMETA((0.000000000000000001/2.5))+'|'+HexBytes(BINTOC((0.000000000000000001/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((0.000000000000000001/2.5)))
 CASE index=226
  RETURN CFNMETA(+((0.000000000000000001/2.5)))+'|'+HexBytes(BINTOC(+((0.000000000000000001/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.000000000000000001/2.5))))
 CASE index=227
  RETURN CFNMETA(-((0.000000000000000001/2.5)))+'|'+HexBytes(BINTOC(-((0.000000000000000001/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.000000000000000001/2.5))))
 CASE index=228
  RETURN CFNMETA(-(-((0.000000000000000001/2.5))))+'|'+HexBytes(BINTOC(-(-((0.000000000000000001/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.000000000000000001/2.5)))))
 CASE index=229
  RETURN CFNMETA(IIF(.T.,(0.000000000000000001/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.000000000000000001/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.000000000000000001/2.5),0)))
 CASE index=230
  RETURN CFNMETA(-(IIF(.T.,(0.000000000000000001/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.000000000000000001/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.000000000000000001/2.5),0))))
 CASE index=231
  RETURN CFNMETA(EVALUATE("(0.000000000000000001/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(0.000000000000000001/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.000000000000000001/2.5)")))
 CASE index=232
  RETURN CFNMETA(-(EVALUATE("(0.000000000000000001/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.000000000000000001/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.000000000000000001/2.5)"))))
 CASE index=233
  RETURN CFNMETA((-(0.000000000000000001)/2.5))+'|'+HexBytes(BINTOC((-(0.000000000000000001)/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.000000000000000001)/2.5)))
 CASE index=234
  RETURN CFNMETA(+((-(0.000000000000000001)/2.5)))+'|'+HexBytes(BINTOC(+((-(0.000000000000000001)/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.000000000000000001)/2.5))))
 CASE index=235
  RETURN CFNMETA(-((-(0.000000000000000001)/2.5)))+'|'+HexBytes(BINTOC(-((-(0.000000000000000001)/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.000000000000000001)/2.5))))
 CASE index=236
  RETURN CFNMETA(-(-((-(0.000000000000000001)/2.5))))+'|'+HexBytes(BINTOC(-(-((-(0.000000000000000001)/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.000000000000000001)/2.5)))))
 CASE index=237
  RETURN CFNMETA(IIF(.T.,(-(0.000000000000000001)/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.000000000000000001)/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.000000000000000001)/2.5),0)))
 CASE index=238
  RETURN CFNMETA(-(IIF(.T.,(-(0.000000000000000001)/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.000000000000000001)/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.000000000000000001)/2.5),0))))
 CASE index=239
  RETURN CFNMETA(EVALUATE("(-(0.000000000000000001)/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.000000000000000001)/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.000000000000000001)/2.5)")))
 CASE index=240
  RETURN CFNMETA(-(EVALUATE("(-(0.000000000000000001)/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.000000000000000001)/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.000000000000000001)/2.5)"))))
 CASE index=241
  RETURN CFNMETA((0.000000000000000001/-(2.5)))+'|'+HexBytes(BINTOC((0.000000000000000001/-(2.5)),'BRS'))+'|'+HexBytes(TRANSFORM((0.000000000000000001/-(2.5))))
 CASE index=242
  RETURN CFNMETA(+((0.000000000000000001/-(2.5))))+'|'+HexBytes(BINTOC(+((0.000000000000000001/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.000000000000000001/-(2.5)))))
 CASE index=243
  RETURN CFNMETA(-((0.000000000000000001/-(2.5))))+'|'+HexBytes(BINTOC(-((0.000000000000000001/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.000000000000000001/-(2.5)))))
 CASE index=244
  RETURN CFNMETA(-(-((0.000000000000000001/-(2.5)))))+'|'+HexBytes(BINTOC(-(-((0.000000000000000001/-(2.5)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.000000000000000001/-(2.5))))))
 CASE index=245
  RETURN CFNMETA(IIF(.T.,(0.000000000000000001/-(2.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.000000000000000001/-(2.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.000000000000000001/-(2.5)),0)))
 CASE index=246
  RETURN CFNMETA(-(IIF(.T.,(0.000000000000000001/-(2.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.000000000000000001/-(2.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.000000000000000001/-(2.5)),0))))
 CASE index=247
  RETURN CFNMETA(EVALUATE("(0.000000000000000001/-(2.5))"))+'|'+HexBytes(BINTOC(EVALUATE("(0.000000000000000001/-(2.5))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.000000000000000001/-(2.5))")))
 CASE index=248
  RETURN CFNMETA(-(EVALUATE("(0.000000000000000001/-(2.5))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.000000000000000001/-(2.5))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.000000000000000001/-(2.5))"))))
 CASE index=249
  RETURN CFNMETA((-(0.000000000000000001)/-(2.5)))+'|'+HexBytes(BINTOC((-(0.000000000000000001)/-(2.5)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.000000000000000001)/-(2.5))))
 CASE index=250
  RETURN CFNMETA(+((-(0.000000000000000001)/-(2.5))))+'|'+HexBytes(BINTOC(+((-(0.000000000000000001)/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.000000000000000001)/-(2.5)))))
 CASE index=251
  RETURN CFNMETA(-((-(0.000000000000000001)/-(2.5))))+'|'+HexBytes(BINTOC(-((-(0.000000000000000001)/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.000000000000000001)/-(2.5)))))
 CASE index=252
  RETURN CFNMETA(-(-((-(0.000000000000000001)/-(2.5)))))+'|'+HexBytes(BINTOC(-(-((-(0.000000000000000001)/-(2.5)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.000000000000000001)/-(2.5))))))
 CASE index=253
  RETURN CFNMETA(IIF(.T.,(-(0.000000000000000001)/-(2.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.000000000000000001)/-(2.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.000000000000000001)/-(2.5)),0)))
 CASE index=254
  RETURN CFNMETA(-(IIF(.T.,(-(0.000000000000000001)/-(2.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.000000000000000001)/-(2.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.000000000000000001)/-(2.5)),0))))
 CASE index=255
  RETURN CFNMETA(EVALUATE("(-(0.000000000000000001)/-(2.5))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.000000000000000001)/-(2.5))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.000000000000000001)/-(2.5))")))
 CASE index=256
  RETURN CFNMETA(-(EVALUATE("(-(0.000000000000000001)/-(2.5))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.000000000000000001)/-(2.5))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.000000000000000001)/-(2.5))"))))
 CASE index=257
  RETURN CFNMETA((0.000000000000000001/02.50))+'|'+HexBytes(BINTOC((0.000000000000000001/02.50),'BRS'))+'|'+HexBytes(TRANSFORM((0.000000000000000001/02.50)))
 CASE index=258
  RETURN CFNMETA(+((0.000000000000000001/02.50)))+'|'+HexBytes(BINTOC(+((0.000000000000000001/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.000000000000000001/02.50))))
 CASE index=259
  RETURN CFNMETA(-((0.000000000000000001/02.50)))+'|'+HexBytes(BINTOC(-((0.000000000000000001/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.000000000000000001/02.50))))
 CASE index=260
  RETURN CFNMETA(-(-((0.000000000000000001/02.50))))+'|'+HexBytes(BINTOC(-(-((0.000000000000000001/02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.000000000000000001/02.50)))))
 CASE index=261
  RETURN CFNMETA(IIF(.T.,(0.000000000000000001/02.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.000000000000000001/02.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.000000000000000001/02.50),0)))
 CASE index=262
  RETURN CFNMETA(-(IIF(.T.,(0.000000000000000001/02.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.000000000000000001/02.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.000000000000000001/02.50),0))))
 CASE index=263
  RETURN CFNMETA(EVALUATE("(0.000000000000000001/02.50)"))+'|'+HexBytes(BINTOC(EVALUATE("(0.000000000000000001/02.50)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.000000000000000001/02.50)")))
 CASE index=264
  RETURN CFNMETA(-(EVALUATE("(0.000000000000000001/02.50)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.000000000000000001/02.50)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.000000000000000001/02.50)"))))
 CASE index=265
  RETURN CFNMETA((-(0.000000000000000001)/02.50))+'|'+HexBytes(BINTOC((-(0.000000000000000001)/02.50),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.000000000000000001)/02.50)))
 CASE index=266
  RETURN CFNMETA(+((-(0.000000000000000001)/02.50)))+'|'+HexBytes(BINTOC(+((-(0.000000000000000001)/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.000000000000000001)/02.50))))
 CASE index=267
  RETURN CFNMETA(-((-(0.000000000000000001)/02.50)))+'|'+HexBytes(BINTOC(-((-(0.000000000000000001)/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.000000000000000001)/02.50))))
 CASE index=268
  RETURN CFNMETA(-(-((-(0.000000000000000001)/02.50))))+'|'+HexBytes(BINTOC(-(-((-(0.000000000000000001)/02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.000000000000000001)/02.50)))))
 CASE index=269
  RETURN CFNMETA(IIF(.T.,(-(0.000000000000000001)/02.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.000000000000000001)/02.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.000000000000000001)/02.50),0)))
 CASE index=270
  RETURN CFNMETA(-(IIF(.T.,(-(0.000000000000000001)/02.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.000000000000000001)/02.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.000000000000000001)/02.50),0))))
 CASE index=271
  RETURN CFNMETA(EVALUATE("(-(0.000000000000000001)/02.50)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.000000000000000001)/02.50)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.000000000000000001)/02.50)")))
 CASE index=272
  RETURN CFNMETA(-(EVALUATE("(-(0.000000000000000001)/02.50)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.000000000000000001)/02.50)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.000000000000000001)/02.50)"))))
 CASE index=273
  RETURN CFNMETA((0.000000000000000001/-(02.50)))+'|'+HexBytes(BINTOC((0.000000000000000001/-(02.50)),'BRS'))+'|'+HexBytes(TRANSFORM((0.000000000000000001/-(02.50))))
 CASE index=274
  RETURN CFNMETA(+((0.000000000000000001/-(02.50))))+'|'+HexBytes(BINTOC(+((0.000000000000000001/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.000000000000000001/-(02.50)))))
 CASE index=275
  RETURN CFNMETA(-((0.000000000000000001/-(02.50))))+'|'+HexBytes(BINTOC(-((0.000000000000000001/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.000000000000000001/-(02.50)))))
 CASE index=276
  RETURN CFNMETA(-(-((0.000000000000000001/-(02.50)))))+'|'+HexBytes(BINTOC(-(-((0.000000000000000001/-(02.50)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.000000000000000001/-(02.50))))))
 CASE index=277
  RETURN CFNMETA(IIF(.T.,(0.000000000000000001/-(02.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.000000000000000001/-(02.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.000000000000000001/-(02.50)),0)))
 CASE index=278
  RETURN CFNMETA(-(IIF(.T.,(0.000000000000000001/-(02.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.000000000000000001/-(02.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.000000000000000001/-(02.50)),0))))
 CASE index=279
  RETURN CFNMETA(EVALUATE("(0.000000000000000001/-(02.50))"))+'|'+HexBytes(BINTOC(EVALUATE("(0.000000000000000001/-(02.50))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.000000000000000001/-(02.50))")))
 CASE index=280
  RETURN CFNMETA(-(EVALUATE("(0.000000000000000001/-(02.50))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.000000000000000001/-(02.50))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.000000000000000001/-(02.50))"))))
 CASE index=281
  RETURN CFNMETA((-(0.000000000000000001)/-(02.50)))+'|'+HexBytes(BINTOC((-(0.000000000000000001)/-(02.50)),'BRS'))+'|'+HexBytes(TRANSFORM((-(0.000000000000000001)/-(02.50))))
 CASE index=282
  RETURN CFNMETA(+((-(0.000000000000000001)/-(02.50))))+'|'+HexBytes(BINTOC(+((-(0.000000000000000001)/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(0.000000000000000001)/-(02.50)))))
 CASE index=283
  RETURN CFNMETA(-((-(0.000000000000000001)/-(02.50))))+'|'+HexBytes(BINTOC(-((-(0.000000000000000001)/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(0.000000000000000001)/-(02.50)))))
 CASE index=284
  RETURN CFNMETA(-(-((-(0.000000000000000001)/-(02.50)))))+'|'+HexBytes(BINTOC(-(-((-(0.000000000000000001)/-(02.50)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(0.000000000000000001)/-(02.50))))))
 CASE index=285
  RETURN CFNMETA(IIF(.T.,(-(0.000000000000000001)/-(02.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(0.000000000000000001)/-(02.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(0.000000000000000001)/-(02.50)),0)))
 CASE index=286
  RETURN CFNMETA(-(IIF(.T.,(-(0.000000000000000001)/-(02.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(0.000000000000000001)/-(02.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(0.000000000000000001)/-(02.50)),0))))
 CASE index=287
  RETURN CFNMETA(EVALUATE("(-(0.000000000000000001)/-(02.50))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(0.000000000000000001)/-(02.50))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(0.000000000000000001)/-(02.50))")))
 CASE index=288
  RETURN CFNMETA(-(EVALUATE("(-(0.000000000000000001)/-(02.50))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(0.000000000000000001)/-(02.50))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(0.000000000000000001)/-(02.50))"))))
 CASE index=289
  RETURN CFNMETA((1.234567890123456789/2))+'|'+HexBytes(BINTOC((1.234567890123456789/2),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789/2)))
 CASE index=290
  RETURN CFNMETA(+((1.234567890123456789/2)))+'|'+HexBytes(BINTOC(+((1.234567890123456789/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+((1.234567890123456789/2))))
 CASE index=291
  RETURN CFNMETA(-((1.234567890123456789/2)))+'|'+HexBytes(BINTOC(-((1.234567890123456789/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.234567890123456789/2))))
 CASE index=292
  RETURN CFNMETA(-(-((1.234567890123456789/2))))+'|'+HexBytes(BINTOC(-(-((1.234567890123456789/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((1.234567890123456789/2)))))
 CASE index=293
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.234567890123456789/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.234567890123456789/2),0)))
 CASE index=294
  RETURN CFNMETA(-(IIF(.T.,(1.234567890123456789/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.234567890123456789/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.234567890123456789/2),0))))
 CASE index=295
  RETURN CFNMETA(EVALUATE("(1.234567890123456789/2)"))+'|'+HexBytes(BINTOC(EVALUATE("(1.234567890123456789/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(1.234567890123456789/2)")))
 CASE index=296
  RETURN CFNMETA(-(EVALUATE("(1.234567890123456789/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(1.234567890123456789/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(1.234567890123456789/2)"))))
 CASE index=297
  RETURN CFNMETA((-(1.234567890123456789)/2))+'|'+HexBytes(BINTOC((-(1.234567890123456789)/2),'BRS'))+'|'+HexBytes(TRANSFORM((-(1.234567890123456789)/2)))
 CASE index=298
  RETURN CFNMETA(+((-(1.234567890123456789)/2)))+'|'+HexBytes(BINTOC(+((-(1.234567890123456789)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(1.234567890123456789)/2))))
 CASE index=299
  RETURN CFNMETA(-((-(1.234567890123456789)/2)))+'|'+HexBytes(BINTOC(-((-(1.234567890123456789)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1.234567890123456789)/2))))
 CASE index=300
  RETURN CFNMETA(-(-((-(1.234567890123456789)/2))))+'|'+HexBytes(BINTOC(-(-((-(1.234567890123456789)/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(1.234567890123456789)/2)))))
 CASE index=301
  RETURN CFNMETA(IIF(.T.,(-(1.234567890123456789)/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1.234567890123456789)/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1.234567890123456789)/2),0)))
 CASE index=302
  RETURN CFNMETA(-(IIF(.T.,(-(1.234567890123456789)/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1.234567890123456789)/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1.234567890123456789)/2),0))))
 CASE index=303
  RETURN CFNMETA(EVALUATE("(-(1.234567890123456789)/2)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(1.234567890123456789)/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(1.234567890123456789)/2)")))
 CASE index=304
  RETURN CFNMETA(-(EVALUATE("(-(1.234567890123456789)/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(1.234567890123456789)/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(1.234567890123456789)/2)"))))
 CASE index=305
  RETURN CFNMETA((1.234567890123456789/-(2)))+'|'+HexBytes(BINTOC((1.234567890123456789/-(2)),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789/-(2))))
 CASE index=306
  RETURN CFNMETA(+((1.234567890123456789/-(2))))+'|'+HexBytes(BINTOC(+((1.234567890123456789/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(+((1.234567890123456789/-(2)))))
 CASE index=307
  RETURN CFNMETA(-((1.234567890123456789/-(2))))+'|'+HexBytes(BINTOC(-((1.234567890123456789/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.234567890123456789/-(2)))))
 CASE index=308
  RETURN CFNMETA(-(-((1.234567890123456789/-(2)))))+'|'+HexBytes(BINTOC(-(-((1.234567890123456789/-(2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((1.234567890123456789/-(2))))))
 CASE index=309
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/-(2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.234567890123456789/-(2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.234567890123456789/-(2)),0)))
 CASE index=310
  RETURN CFNMETA(-(IIF(.T.,(1.234567890123456789/-(2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.234567890123456789/-(2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.234567890123456789/-(2)),0))))
 CASE index=311
  RETURN CFNMETA(EVALUATE("(1.234567890123456789/-(2))"))+'|'+HexBytes(BINTOC(EVALUATE("(1.234567890123456789/-(2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(1.234567890123456789/-(2))")))
 CASE index=312
  RETURN CFNMETA(-(EVALUATE("(1.234567890123456789/-(2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(1.234567890123456789/-(2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(1.234567890123456789/-(2))"))))
 CASE index=313
  RETURN CFNMETA((-(1.234567890123456789)/-(2)))+'|'+HexBytes(BINTOC((-(1.234567890123456789)/-(2)),'BRS'))+'|'+HexBytes(TRANSFORM((-(1.234567890123456789)/-(2))))
 CASE index=314
  RETURN CFNMETA(+((-(1.234567890123456789)/-(2))))+'|'+HexBytes(BINTOC(+((-(1.234567890123456789)/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(1.234567890123456789)/-(2)))))
 CASE index=315
  RETURN CFNMETA(-((-(1.234567890123456789)/-(2))))+'|'+HexBytes(BINTOC(-((-(1.234567890123456789)/-(2))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1.234567890123456789)/-(2)))))
 CASE index=316
  RETURN CFNMETA(-(-((-(1.234567890123456789)/-(2)))))+'|'+HexBytes(BINTOC(-(-((-(1.234567890123456789)/-(2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(1.234567890123456789)/-(2))))))
 CASE index=317
  RETURN CFNMETA(IIF(.T.,(-(1.234567890123456789)/-(2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1.234567890123456789)/-(2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1.234567890123456789)/-(2)),0)))
 CASE index=318
  RETURN CFNMETA(-(IIF(.T.,(-(1.234567890123456789)/-(2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1.234567890123456789)/-(2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1.234567890123456789)/-(2)),0))))
 CASE index=319
  RETURN CFNMETA(EVALUATE("(-(1.234567890123456789)/-(2))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(1.234567890123456789)/-(2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(1.234567890123456789)/-(2))")))
 CASE index=320
  RETURN CFNMETA(-(EVALUATE("(-(1.234567890123456789)/-(2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(1.234567890123456789)/-(2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(1.234567890123456789)/-(2))"))))
 CASE index=321
  RETURN CFNMETA((1.234567890123456789/2.5))+'|'+HexBytes(BINTOC((1.234567890123456789/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789/2.5)))
 CASE index=322
  RETURN CFNMETA(+((1.234567890123456789/2.5)))+'|'+HexBytes(BINTOC(+((1.234567890123456789/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((1.234567890123456789/2.5))))
 CASE index=323
  RETURN CFNMETA(-((1.234567890123456789/2.5)))+'|'+HexBytes(BINTOC(-((1.234567890123456789/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.234567890123456789/2.5))))
 CASE index=324
  RETURN CFNMETA(-(-((1.234567890123456789/2.5))))+'|'+HexBytes(BINTOC(-(-((1.234567890123456789/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((1.234567890123456789/2.5)))))
 CASE index=325
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.234567890123456789/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.234567890123456789/2.5),0)))
 CASE index=326
  RETURN CFNMETA(-(IIF(.T.,(1.234567890123456789/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.234567890123456789/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.234567890123456789/2.5),0))))
 CASE index=327
  RETURN CFNMETA(EVALUATE("(1.234567890123456789/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(1.234567890123456789/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(1.234567890123456789/2.5)")))
 CASE index=328
  RETURN CFNMETA(-(EVALUATE("(1.234567890123456789/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(1.234567890123456789/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(1.234567890123456789/2.5)"))))
 CASE index=329
  RETURN CFNMETA((-(1.234567890123456789)/2.5))+'|'+HexBytes(BINTOC((-(1.234567890123456789)/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((-(1.234567890123456789)/2.5)))
 CASE index=330
  RETURN CFNMETA(+((-(1.234567890123456789)/2.5)))+'|'+HexBytes(BINTOC(+((-(1.234567890123456789)/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(1.234567890123456789)/2.5))))
 CASE index=331
  RETURN CFNMETA(-((-(1.234567890123456789)/2.5)))+'|'+HexBytes(BINTOC(-((-(1.234567890123456789)/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1.234567890123456789)/2.5))))
 CASE index=332
  RETURN CFNMETA(-(-((-(1.234567890123456789)/2.5))))+'|'+HexBytes(BINTOC(-(-((-(1.234567890123456789)/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(1.234567890123456789)/2.5)))))
 CASE index=333
  RETURN CFNMETA(IIF(.T.,(-(1.234567890123456789)/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1.234567890123456789)/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1.234567890123456789)/2.5),0)))
 CASE index=334
  RETURN CFNMETA(-(IIF(.T.,(-(1.234567890123456789)/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1.234567890123456789)/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1.234567890123456789)/2.5),0))))
 CASE index=335
  RETURN CFNMETA(EVALUATE("(-(1.234567890123456789)/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(1.234567890123456789)/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(1.234567890123456789)/2.5)")))
 CASE index=336
  RETURN CFNMETA(-(EVALUATE("(-(1.234567890123456789)/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(1.234567890123456789)/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(1.234567890123456789)/2.5)"))))
 CASE index=337
  RETURN CFNMETA((1.234567890123456789/-(2.5)))+'|'+HexBytes(BINTOC((1.234567890123456789/-(2.5)),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789/-(2.5))))
 CASE index=338
  RETURN CFNMETA(+((1.234567890123456789/-(2.5))))+'|'+HexBytes(BINTOC(+((1.234567890123456789/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(+((1.234567890123456789/-(2.5)))))
 CASE index=339
  RETURN CFNMETA(-((1.234567890123456789/-(2.5))))+'|'+HexBytes(BINTOC(-((1.234567890123456789/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.234567890123456789/-(2.5)))))
 CASE index=340
  RETURN CFNMETA(-(-((1.234567890123456789/-(2.5)))))+'|'+HexBytes(BINTOC(-(-((1.234567890123456789/-(2.5)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((1.234567890123456789/-(2.5))))))
 CASE index=341
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/-(2.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.234567890123456789/-(2.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.234567890123456789/-(2.5)),0)))
 CASE index=342
  RETURN CFNMETA(-(IIF(.T.,(1.234567890123456789/-(2.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.234567890123456789/-(2.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.234567890123456789/-(2.5)),0))))
 CASE index=343
  RETURN CFNMETA(EVALUATE("(1.234567890123456789/-(2.5))"))+'|'+HexBytes(BINTOC(EVALUATE("(1.234567890123456789/-(2.5))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(1.234567890123456789/-(2.5))")))
 CASE index=344
  RETURN CFNMETA(-(EVALUATE("(1.234567890123456789/-(2.5))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(1.234567890123456789/-(2.5))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(1.234567890123456789/-(2.5))"))))
 CASE index=345
  RETURN CFNMETA((-(1.234567890123456789)/-(2.5)))+'|'+HexBytes(BINTOC((-(1.234567890123456789)/-(2.5)),'BRS'))+'|'+HexBytes(TRANSFORM((-(1.234567890123456789)/-(2.5))))
 CASE index=346
  RETURN CFNMETA(+((-(1.234567890123456789)/-(2.5))))+'|'+HexBytes(BINTOC(+((-(1.234567890123456789)/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(1.234567890123456789)/-(2.5)))))
 CASE index=347
  RETURN CFNMETA(-((-(1.234567890123456789)/-(2.5))))+'|'+HexBytes(BINTOC(-((-(1.234567890123456789)/-(2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1.234567890123456789)/-(2.5)))))
 CASE index=348
  RETURN CFNMETA(-(-((-(1.234567890123456789)/-(2.5)))))+'|'+HexBytes(BINTOC(-(-((-(1.234567890123456789)/-(2.5)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(1.234567890123456789)/-(2.5))))))
 CASE index=349
  RETURN CFNMETA(IIF(.T.,(-(1.234567890123456789)/-(2.5)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1.234567890123456789)/-(2.5)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1.234567890123456789)/-(2.5)),0)))
 CASE index=350
  RETURN CFNMETA(-(IIF(.T.,(-(1.234567890123456789)/-(2.5)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1.234567890123456789)/-(2.5)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1.234567890123456789)/-(2.5)),0))))
 CASE index=351
  RETURN CFNMETA(EVALUATE("(-(1.234567890123456789)/-(2.5))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(1.234567890123456789)/-(2.5))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(1.234567890123456789)/-(2.5))")))
 CASE index=352
  RETURN CFNMETA(-(EVALUATE("(-(1.234567890123456789)/-(2.5))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(1.234567890123456789)/-(2.5))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(1.234567890123456789)/-(2.5))"))))
 CASE index=353
  RETURN CFNMETA((1.234567890123456789/02.50))+'|'+HexBytes(BINTOC((1.234567890123456789/02.50),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789/02.50)))
 CASE index=354
  RETURN CFNMETA(+((1.234567890123456789/02.50)))+'|'+HexBytes(BINTOC(+((1.234567890123456789/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(+((1.234567890123456789/02.50))))
 CASE index=355
  RETURN CFNMETA(-((1.234567890123456789/02.50)))+'|'+HexBytes(BINTOC(-((1.234567890123456789/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.234567890123456789/02.50))))
 CASE index=356
  RETURN CFNMETA(-(-((1.234567890123456789/02.50))))+'|'+HexBytes(BINTOC(-(-((1.234567890123456789/02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((1.234567890123456789/02.50)))))
 CASE index=357
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/02.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.234567890123456789/02.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.234567890123456789/02.50),0)))
 CASE index=358
  RETURN CFNMETA(-(IIF(.T.,(1.234567890123456789/02.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.234567890123456789/02.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.234567890123456789/02.50),0))))
 CASE index=359
  RETURN CFNMETA(EVALUATE("(1.234567890123456789/02.50)"))+'|'+HexBytes(BINTOC(EVALUATE("(1.234567890123456789/02.50)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(1.234567890123456789/02.50)")))
 CASE index=360
  RETURN CFNMETA(-(EVALUATE("(1.234567890123456789/02.50)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(1.234567890123456789/02.50)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(1.234567890123456789/02.50)"))))
 CASE index=361
  RETURN CFNMETA((-(1.234567890123456789)/02.50))+'|'+HexBytes(BINTOC((-(1.234567890123456789)/02.50),'BRS'))+'|'+HexBytes(TRANSFORM((-(1.234567890123456789)/02.50)))
 CASE index=362
  RETURN CFNMETA(+((-(1.234567890123456789)/02.50)))+'|'+HexBytes(BINTOC(+((-(1.234567890123456789)/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(1.234567890123456789)/02.50))))
 CASE index=363
  RETURN CFNMETA(-((-(1.234567890123456789)/02.50)))+'|'+HexBytes(BINTOC(-((-(1.234567890123456789)/02.50)),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1.234567890123456789)/02.50))))
 CASE index=364
  RETURN CFNMETA(-(-((-(1.234567890123456789)/02.50))))+'|'+HexBytes(BINTOC(-(-((-(1.234567890123456789)/02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(1.234567890123456789)/02.50)))))
 CASE index=365
  RETURN CFNMETA(IIF(.T.,(-(1.234567890123456789)/02.50),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1.234567890123456789)/02.50),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1.234567890123456789)/02.50),0)))
 CASE index=366
  RETURN CFNMETA(-(IIF(.T.,(-(1.234567890123456789)/02.50),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1.234567890123456789)/02.50),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1.234567890123456789)/02.50),0))))
 CASE index=367
  RETURN CFNMETA(EVALUATE("(-(1.234567890123456789)/02.50)"))+'|'+HexBytes(BINTOC(EVALUATE("(-(1.234567890123456789)/02.50)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(1.234567890123456789)/02.50)")))
 CASE index=368
  RETURN CFNMETA(-(EVALUATE("(-(1.234567890123456789)/02.50)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(1.234567890123456789)/02.50)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(1.234567890123456789)/02.50)"))))
 CASE index=369
  RETURN CFNMETA((1.234567890123456789/-(02.50)))+'|'+HexBytes(BINTOC((1.234567890123456789/-(02.50)),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789/-(02.50))))
 CASE index=370
  RETURN CFNMETA(+((1.234567890123456789/-(02.50))))+'|'+HexBytes(BINTOC(+((1.234567890123456789/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(+((1.234567890123456789/-(02.50)))))
 CASE index=371
  RETURN CFNMETA(-((1.234567890123456789/-(02.50))))+'|'+HexBytes(BINTOC(-((1.234567890123456789/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.234567890123456789/-(02.50)))))
 CASE index=372
  RETURN CFNMETA(-(-((1.234567890123456789/-(02.50)))))+'|'+HexBytes(BINTOC(-(-((1.234567890123456789/-(02.50)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((1.234567890123456789/-(02.50))))))
 CASE index=373
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/-(02.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.234567890123456789/-(02.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.234567890123456789/-(02.50)),0)))
 CASE index=374
  RETURN CFNMETA(-(IIF(.T.,(1.234567890123456789/-(02.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.234567890123456789/-(02.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.234567890123456789/-(02.50)),0))))
 CASE index=375
  RETURN CFNMETA(EVALUATE("(1.234567890123456789/-(02.50))"))+'|'+HexBytes(BINTOC(EVALUATE("(1.234567890123456789/-(02.50))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(1.234567890123456789/-(02.50))")))
 CASE index=376
  RETURN CFNMETA(-(EVALUATE("(1.234567890123456789/-(02.50))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(1.234567890123456789/-(02.50))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(1.234567890123456789/-(02.50))"))))
 CASE index=377
  RETURN CFNMETA((-(1.234567890123456789)/-(02.50)))+'|'+HexBytes(BINTOC((-(1.234567890123456789)/-(02.50)),'BRS'))+'|'+HexBytes(TRANSFORM((-(1.234567890123456789)/-(02.50))))
 CASE index=378
  RETURN CFNMETA(+((-(1.234567890123456789)/-(02.50))))+'|'+HexBytes(BINTOC(+((-(1.234567890123456789)/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(+((-(1.234567890123456789)/-(02.50)))))
 CASE index=379
  RETURN CFNMETA(-((-(1.234567890123456789)/-(02.50))))+'|'+HexBytes(BINTOC(-((-(1.234567890123456789)/-(02.50))),'BRS'))+'|'+HexBytes(TRANSFORM(-((-(1.234567890123456789)/-(02.50)))))
 CASE index=380
  RETURN CFNMETA(-(-((-(1.234567890123456789)/-(02.50)))))+'|'+HexBytes(BINTOC(-(-((-(1.234567890123456789)/-(02.50)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((-(1.234567890123456789)/-(02.50))))))
 CASE index=381
  RETURN CFNMETA(IIF(.T.,(-(1.234567890123456789)/-(02.50)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(-(1.234567890123456789)/-(02.50)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(-(1.234567890123456789)/-(02.50)),0)))
 CASE index=382
  RETURN CFNMETA(-(IIF(.T.,(-(1.234567890123456789)/-(02.50)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(-(1.234567890123456789)/-(02.50)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(-(1.234567890123456789)/-(02.50)),0))))
 CASE index=383
  RETURN CFNMETA(EVALUATE("(-(1.234567890123456789)/-(02.50))"))+'|'+HexBytes(BINTOC(EVALUATE("(-(1.234567890123456789)/-(02.50))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(-(1.234567890123456789)/-(02.50))")))
 CASE index=384
  RETURN CFNMETA(-(EVALUATE("(-(1.234567890123456789)/-(02.50))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(-(1.234567890123456789)/-(02.50))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(-(1.234567890123456789)/-(02.50))"))))
 CASE index=385
  RETURN CFNMETA(((5/2)/(5/2)))+'|'+HexBytes(BINTOC(((5/2)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(((5/2)/(5/2))))
 CASE index=386
  RETURN CFNMETA(+(((5/2)/(5/2))))+'|'+HexBytes(BINTOC(+(((5/2)/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(+(((5/2)/(5/2)))))
 CASE index=387
  RETURN CFNMETA(-(((5/2)/(5/2))))+'|'+HexBytes(BINTOC(-(((5/2)/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(((5/2)/(5/2)))))
 CASE index=388
  RETURN CFNMETA(-(-(((5/2)/(5/2)))))+'|'+HexBytes(BINTOC(-(-(((5/2)/(5/2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((5/2)/(5/2))))))
 CASE index=389
  RETURN CFNMETA(IIF(.T.,((5/2)/(5/2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,((5/2)/(5/2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((5/2)/(5/2)),0)))
 CASE index=390
  RETURN CFNMETA(-(IIF(.T.,((5/2)/(5/2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((5/2)/(5/2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((5/2)/(5/2)),0))))
 CASE index=391
  RETURN CFNMETA(EVALUATE("((5/2)/(5/2))"))+'|'+HexBytes(BINTOC(EVALUATE("((5/2)/(5/2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((5/2)/(5/2))")))
 CASE index=392
  RETURN CFNMETA(-(EVALUATE("((5/2)/(5/2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("((5/2)/(5/2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((5/2)/(5/2))"))))
 CASE index=393
  RETURN CFNMETA(((1/7)/3))+'|'+HexBytes(BINTOC(((1/7)/3),'BRS'))+'|'+HexBytes(TRANSFORM(((1/7)/3)))
 CASE index=394
  RETURN CFNMETA(+(((1/7)/3)))+'|'+HexBytes(BINTOC(+(((1/7)/3)),'BRS'))+'|'+HexBytes(TRANSFORM(+(((1/7)/3))))
 CASE index=395
  RETURN CFNMETA(-(((1/7)/3)))+'|'+HexBytes(BINTOC(-(((1/7)/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-(((1/7)/3))))
 CASE index=396
  RETURN CFNMETA(-(-(((1/7)/3))))+'|'+HexBytes(BINTOC(-(-(((1/7)/3))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((1/7)/3)))))
 CASE index=397
  RETURN CFNMETA(IIF(.T.,((1/7)/3),0))+'|'+HexBytes(BINTOC(IIF(.T.,((1/7)/3),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((1/7)/3),0)))
 CASE index=398
  RETURN CFNMETA(-(IIF(.T.,((1/7)/3),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((1/7)/3),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((1/7)/3),0))))
 CASE index=399
  RETURN CFNMETA(EVALUATE("((1/7)/3)"))+'|'+HexBytes(BINTOC(EVALUATE("((1/7)/3)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((1/7)/3)")))
 CASE index=400
  RETURN CFNMETA(-(EVALUATE("((1/7)/3)")))+'|'+HexBytes(BINTOC(-(EVALUATE("((1/7)/3)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((1/7)/3)"))))
 CASE index=401
  RETURN CFNMETA((5/(5/2)))+'|'+HexBytes(BINTOC((5/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM((5/(5/2))))
 CASE index=402
  RETURN CFNMETA(+((5/(5/2))))+'|'+HexBytes(BINTOC(+((5/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(+((5/(5/2)))))
 CASE index=403
  RETURN CFNMETA(-((5/(5/2))))+'|'+HexBytes(BINTOC(-((5/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-((5/(5/2)))))
 CASE index=404
  RETURN CFNMETA(-(-((5/(5/2)))))+'|'+HexBytes(BINTOC(-(-((5/(5/2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((5/(5/2))))))
 CASE index=405
  RETURN CFNMETA(IIF(.T.,(5/(5/2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(5/(5/2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(5/(5/2)),0)))
 CASE index=406
  RETURN CFNMETA(-(IIF(.T.,(5/(5/2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(5/(5/2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(5/(5/2)),0))))
 CASE index=407
  RETURN CFNMETA(EVALUATE("(5/(5/2))"))+'|'+HexBytes(BINTOC(EVALUATE("(5/(5/2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(5/(5/2))")))
 CASE index=408
  RETURN CFNMETA(-(EVALUATE("(5/(5/2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(5/(5/2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(5/(5/2))"))))
 CASE index=409
  RETURN CFNMETA(((2/3)/7))+'|'+HexBytes(BINTOC(((2/3)/7),'BRS'))+'|'+HexBytes(TRANSFORM(((2/3)/7)))
 CASE index=410
  RETURN CFNMETA(+(((2/3)/7)))+'|'+HexBytes(BINTOC(+(((2/3)/7)),'BRS'))+'|'+HexBytes(TRANSFORM(+(((2/3)/7))))
 CASE index=411
  RETURN CFNMETA(-(((2/3)/7)))+'|'+HexBytes(BINTOC(-(((2/3)/7)),'BRS'))+'|'+HexBytes(TRANSFORM(-(((2/3)/7))))
 CASE index=412
  RETURN CFNMETA(-(-(((2/3)/7))))+'|'+HexBytes(BINTOC(-(-(((2/3)/7))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((2/3)/7)))))
 CASE index=413
  RETURN CFNMETA(IIF(.T.,((2/3)/7),0))+'|'+HexBytes(BINTOC(IIF(.T.,((2/3)/7),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((2/3)/7),0)))
 CASE index=414
  RETURN CFNMETA(-(IIF(.T.,((2/3)/7),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((2/3)/7),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((2/3)/7),0))))
 CASE index=415
  RETURN CFNMETA(EVALUATE("((2/3)/7)"))+'|'+HexBytes(BINTOC(EVALUATE("((2/3)/7)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((2/3)/7)")))
 CASE index=416
  RETURN CFNMETA(-(EVALUATE("((2/3)/7)")))+'|'+HexBytes(BINTOC(-(EVALUATE("((2/3)/7)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((2/3)/7)"))))
 CASE index=417
  RETURN CFNMETA(((1/3)/7))+'|'+HexBytes(BINTOC(((1/3)/7),'BRS'))+'|'+HexBytes(TRANSFORM(((1/3)/7)))
 CASE index=418
  RETURN CFNMETA(+(((1/3)/7)))+'|'+HexBytes(BINTOC(+(((1/3)/7)),'BRS'))+'|'+HexBytes(TRANSFORM(+(((1/3)/7))))
 CASE index=419
  RETURN CFNMETA(-(((1/3)/7)))+'|'+HexBytes(BINTOC(-(((1/3)/7)),'BRS'))+'|'+HexBytes(TRANSFORM(-(((1/3)/7))))
 CASE index=420
  RETURN CFNMETA(-(-(((1/3)/7))))+'|'+HexBytes(BINTOC(-(-(((1/3)/7))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((1/3)/7)))))
 CASE index=421
  RETURN CFNMETA(IIF(.T.,((1/3)/7),0))+'|'+HexBytes(BINTOC(IIF(.T.,((1/3)/7),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((1/3)/7),0)))
 CASE index=422
  RETURN CFNMETA(-(IIF(.T.,((1/3)/7),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((1/3)/7),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((1/3)/7),0))))
 CASE index=423
  RETURN CFNMETA(EVALUATE("((1/3)/7)"))+'|'+HexBytes(BINTOC(EVALUATE("((1/3)/7)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((1/3)/7)")))
 CASE index=424
  RETURN CFNMETA(-(EVALUATE("((1/3)/7)")))+'|'+HexBytes(BINTOC(-(EVALUATE("((1/3)/7)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((1/3)/7)"))))
 CASE index=425
  RETURN CFNMETA(((1/3)/(7/3)))+'|'+HexBytes(BINTOC(((1/3)/(7/3)),'BRS'))+'|'+HexBytes(TRANSFORM(((1/3)/(7/3))))
 CASE index=426
  RETURN CFNMETA(+(((1/3)/(7/3))))+'|'+HexBytes(BINTOC(+(((1/3)/(7/3))),'BRS'))+'|'+HexBytes(TRANSFORM(+(((1/3)/(7/3)))))
 CASE index=427
  RETURN CFNMETA(-(((1/3)/(7/3))))+'|'+HexBytes(BINTOC(-(((1/3)/(7/3))),'BRS'))+'|'+HexBytes(TRANSFORM(-(((1/3)/(7/3)))))
 CASE index=428
  RETURN CFNMETA(-(-(((1/3)/(7/3)))))+'|'+HexBytes(BINTOC(-(-(((1/3)/(7/3)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((1/3)/(7/3))))))
 CASE index=429
  RETURN CFNMETA(IIF(.T.,((1/3)/(7/3)),0))+'|'+HexBytes(BINTOC(IIF(.T.,((1/3)/(7/3)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((1/3)/(7/3)),0)))
 CASE index=430
  RETURN CFNMETA(-(IIF(.T.,((1/3)/(7/3)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((1/3)/(7/3)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((1/3)/(7/3)),0))))
 CASE index=431
  RETURN CFNMETA(EVALUATE("((1/3)/(7/3))"))+'|'+HexBytes(BINTOC(EVALUATE("((1/3)/(7/3))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((1/3)/(7/3))")))
 CASE index=432
  RETURN CFNMETA(-(EVALUATE("((1/3)/(7/3))")))+'|'+HexBytes(BINTOC(-(EVALUATE("((1/3)/(7/3))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((1/3)/(7/3))"))))
 CASE index=433
  RETURN CFNMETA((((1/7)/3)/3))+'|'+HexBytes(BINTOC((((1/7)/3)/3),'BRS'))+'|'+HexBytes(TRANSFORM((((1/7)/3)/3)))
 CASE index=434
  RETURN CFNMETA(+((((1/7)/3)/3)))+'|'+HexBytes(BINTOC(+((((1/7)/3)/3)),'BRS'))+'|'+HexBytes(TRANSFORM(+((((1/7)/3)/3))))
 CASE index=435
  RETURN CFNMETA(-((((1/7)/3)/3)))+'|'+HexBytes(BINTOC(-((((1/7)/3)/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-((((1/7)/3)/3))))
 CASE index=436
  RETURN CFNMETA(-(-((((1/7)/3)/3))))+'|'+HexBytes(BINTOC(-(-((((1/7)/3)/3))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((((1/7)/3)/3)))))
 CASE index=437
  RETURN CFNMETA(IIF(.T.,(((1/7)/3)/3),0))+'|'+HexBytes(BINTOC(IIF(.T.,(((1/7)/3)/3),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(((1/7)/3)/3),0)))
 CASE index=438
  RETURN CFNMETA(-(IIF(.T.,(((1/7)/3)/3),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(((1/7)/3)/3),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(((1/7)/3)/3),0))))
 CASE index=439
  RETURN CFNMETA(EVALUATE("(((1/7)/3)/3)"))+'|'+HexBytes(BINTOC(EVALUATE("(((1/7)/3)/3)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(((1/7)/3)/3)")))
 CASE index=440
  RETURN CFNMETA(-(EVALUATE("(((1/7)/3)/3)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(((1/7)/3)/3)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(((1/7)/3)/3)"))))
 CASE index=441
  RETURN CFNMETA(((5.00/2.00)/(7.0/3.0)))+'|'+HexBytes(BINTOC(((5.00/2.00)/(7.0/3.0)),'BRS'))+'|'+HexBytes(TRANSFORM(((5.00/2.00)/(7.0/3.0))))
 CASE index=442
  RETURN CFNMETA(+(((5.00/2.00)/(7.0/3.0))))+'|'+HexBytes(BINTOC(+(((5.00/2.00)/(7.0/3.0))),'BRS'))+'|'+HexBytes(TRANSFORM(+(((5.00/2.00)/(7.0/3.0)))))
 CASE index=443
  RETURN CFNMETA(-(((5.00/2.00)/(7.0/3.0))))+'|'+HexBytes(BINTOC(-(((5.00/2.00)/(7.0/3.0))),'BRS'))+'|'+HexBytes(TRANSFORM(-(((5.00/2.00)/(7.0/3.0)))))
 CASE index=444
  RETURN CFNMETA(-(-(((5.00/2.00)/(7.0/3.0)))))+'|'+HexBytes(BINTOC(-(-(((5.00/2.00)/(7.0/3.0)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((5.00/2.00)/(7.0/3.0))))))
 CASE index=445
  RETURN CFNMETA(IIF(.T.,((5.00/2.00)/(7.0/3.0)),0))+'|'+HexBytes(BINTOC(IIF(.T.,((5.00/2.00)/(7.0/3.0)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((5.00/2.00)/(7.0/3.0)),0)))
 CASE index=446
  RETURN CFNMETA(-(IIF(.T.,((5.00/2.00)/(7.0/3.0)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((5.00/2.00)/(7.0/3.0)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((5.00/2.00)/(7.0/3.0)),0))))
 CASE index=447
  RETURN CFNMETA(EVALUATE("((5.00/2.00)/(7.0/3.0))"))+'|'+HexBytes(BINTOC(EVALUATE("((5.00/2.00)/(7.0/3.0))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((5.00/2.00)/(7.0/3.0))")))
 CASE index=448
  RETURN CFNMETA(-(EVALUATE("((5.00/2.00)/(7.0/3.0))")))+'|'+HexBytes(BINTOC(-(EVALUATE("((5.00/2.00)/(7.0/3.0))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((5.00/2.00)/(7.0/3.0))"))))
 CASE index=449
  RETURN CFNMETA(((0.5/2)/2))+'|'+HexBytes(BINTOC(((0.5/2)/2),'BRS'))+'|'+HexBytes(TRANSFORM(((0.5/2)/2)))
 CASE index=450
  RETURN CFNMETA(+(((0.5/2)/2)))+'|'+HexBytes(BINTOC(+(((0.5/2)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(+(((0.5/2)/2))))
 CASE index=451
  RETURN CFNMETA(-(((0.5/2)/2)))+'|'+HexBytes(BINTOC(-(((0.5/2)/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(((0.5/2)/2))))
 CASE index=452
  RETURN CFNMETA(-(-(((0.5/2)/2))))+'|'+HexBytes(BINTOC(-(-(((0.5/2)/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((0.5/2)/2)))))
 CASE index=453
  RETURN CFNMETA(IIF(.T.,((0.5/2)/2),0))+'|'+HexBytes(BINTOC(IIF(.T.,((0.5/2)/2),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((0.5/2)/2),0)))
 CASE index=454
  RETURN CFNMETA(-(IIF(.T.,((0.5/2)/2),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((0.5/2)/2),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((0.5/2)/2),0))))
 CASE index=455
  RETURN CFNMETA(EVALUATE("((0.5/2)/2)"))+'|'+HexBytes(BINTOC(EVALUATE("((0.5/2)/2)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((0.5/2)/2)")))
 CASE index=456
  RETURN CFNMETA(-(EVALUATE("((0.5/2)/2)")))+'|'+HexBytes(BINTOC(-(EVALUATE("((0.5/2)/2)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((0.5/2)/2)"))))
 CASE index=457
  RETURN CFNMETA((((5/2)/(5/2))/3))+'|'+HexBytes(BINTOC((((5/2)/(5/2))/3),'BRS'))+'|'+HexBytes(TRANSFORM((((5/2)/(5/2))/3)))
 CASE index=458
  RETURN CFNMETA(+((((5/2)/(5/2))/3)))+'|'+HexBytes(BINTOC(+((((5/2)/(5/2))/3)),'BRS'))+'|'+HexBytes(TRANSFORM(+((((5/2)/(5/2))/3))))
 CASE index=459
  RETURN CFNMETA(-((((5/2)/(5/2))/3)))+'|'+HexBytes(BINTOC(-((((5/2)/(5/2))/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-((((5/2)/(5/2))/3))))
 CASE index=460
  RETURN CFNMETA(-(-((((5/2)/(5/2))/3))))+'|'+HexBytes(BINTOC(-(-((((5/2)/(5/2))/3))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((((5/2)/(5/2))/3)))))
 CASE index=461
  RETURN CFNMETA(IIF(.T.,(((5/2)/(5/2))/3),0))+'|'+HexBytes(BINTOC(IIF(.T.,(((5/2)/(5/2))/3),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(((5/2)/(5/2))/3),0)))
 CASE index=462
  RETURN CFNMETA(-(IIF(.T.,(((5/2)/(5/2))/3),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(((5/2)/(5/2))/3),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(((5/2)/(5/2))/3),0))))
 CASE index=463
  RETURN CFNMETA(EVALUATE("(((5/2)/(5/2))/3)"))+'|'+HexBytes(BINTOC(EVALUATE("(((5/2)/(5/2))/3)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(((5/2)/(5/2))/3)")))
 CASE index=464
  RETURN CFNMETA(-(EVALUATE("(((5/2)/(5/2))/3)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(((5/2)/(5/2))/3)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(((5/2)/(5/2))/3)"))))
 CASE index=465
  RETURN CFNMETA((((5/2)/2)/(7/3)))+'|'+HexBytes(BINTOC((((5/2)/2)/(7/3)),'BRS'))+'|'+HexBytes(TRANSFORM((((5/2)/2)/(7/3))))
 CASE index=466
  RETURN CFNMETA(+((((5/2)/2)/(7/3))))+'|'+HexBytes(BINTOC(+((((5/2)/2)/(7/3))),'BRS'))+'|'+HexBytes(TRANSFORM(+((((5/2)/2)/(7/3)))))
 CASE index=467
  RETURN CFNMETA(-((((5/2)/2)/(7/3))))+'|'+HexBytes(BINTOC(-((((5/2)/2)/(7/3))),'BRS'))+'|'+HexBytes(TRANSFORM(-((((5/2)/2)/(7/3)))))
 CASE index=468
  RETURN CFNMETA(-(-((((5/2)/2)/(7/3)))))+'|'+HexBytes(BINTOC(-(-((((5/2)/2)/(7/3)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((((5/2)/2)/(7/3))))))
 CASE index=469
  RETURN CFNMETA(IIF(.T.,(((5/2)/2)/(7/3)),0))+'|'+HexBytes(BINTOC(IIF(.T.,(((5/2)/2)/(7/3)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(((5/2)/2)/(7/3)),0)))
 CASE index=470
  RETURN CFNMETA(-(IIF(.T.,(((5/2)/2)/(7/3)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(((5/2)/2)/(7/3)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(((5/2)/2)/(7/3)),0))))
 CASE index=471
  RETURN CFNMETA(EVALUATE("(((5/2)/2)/(7/3))"))+'|'+HexBytes(BINTOC(EVALUATE("(((5/2)/2)/(7/3))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(((5/2)/2)/(7/3))")))
 CASE index=472
  RETURN CFNMETA(-(EVALUATE("(((5/2)/2)/(7/3))")))+'|'+HexBytes(BINTOC(-(EVALUATE("(((5/2)/2)/(7/3))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(((5/2)/2)/(7/3))"))))
 CASE index=473
  RETURN CFNMETA(((2.50/2.5)/(5/2)))+'|'+HexBytes(BINTOC(((2.50/2.5)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(((2.50/2.5)/(5/2))))
 CASE index=474
  RETURN CFNMETA(+(((2.50/2.5)/(5/2))))+'|'+HexBytes(BINTOC(+(((2.50/2.5)/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(+(((2.50/2.5)/(5/2)))))
 CASE index=475
  RETURN CFNMETA(-(((2.50/2.5)/(5/2))))+'|'+HexBytes(BINTOC(-(((2.50/2.5)/(5/2))),'BRS'))+'|'+HexBytes(TRANSFORM(-(((2.50/2.5)/(5/2)))))
 CASE index=476
  RETURN CFNMETA(-(-(((2.50/2.5)/(5/2)))))+'|'+HexBytes(BINTOC(-(-(((2.50/2.5)/(5/2)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((2.50/2.5)/(5/2))))))
 CASE index=477
  RETURN CFNMETA(IIF(.T.,((2.50/2.5)/(5/2)),0))+'|'+HexBytes(BINTOC(IIF(.T.,((2.50/2.5)/(5/2)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((2.50/2.5)/(5/2)),0)))
 CASE index=478
  RETURN CFNMETA(-(IIF(.T.,((2.50/2.5)/(5/2)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((2.50/2.5)/(5/2)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((2.50/2.5)/(5/2)),0))))
 CASE index=479
  RETURN CFNMETA(EVALUATE("((2.50/2.5)/(5/2))"))+'|'+HexBytes(BINTOC(EVALUATE("((2.50/2.5)/(5/2))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((2.50/2.5)/(5/2))")))
 CASE index=480
  RETURN CFNMETA(-(EVALUATE("((2.50/2.5)/(5/2))")))+'|'+HexBytes(BINTOC(-(EVALUATE("((2.50/2.5)/(5/2))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((2.50/2.5)/(5/2))"))))
 CASE index=481
  RETURN CFNMETA(((-(5)/2)/(7/3)))+'|'+HexBytes(BINTOC(((-(5)/2)/(7/3)),'BRS'))+'|'+HexBytes(TRANSFORM(((-(5)/2)/(7/3))))
 CASE index=482
  RETURN CFNMETA(+(((-(5)/2)/(7/3))))+'|'+HexBytes(BINTOC(+(((-(5)/2)/(7/3))),'BRS'))+'|'+HexBytes(TRANSFORM(+(((-(5)/2)/(7/3)))))
 CASE index=483
  RETURN CFNMETA(-(((-(5)/2)/(7/3))))+'|'+HexBytes(BINTOC(-(((-(5)/2)/(7/3))),'BRS'))+'|'+HexBytes(TRANSFORM(-(((-(5)/2)/(7/3)))))
 CASE index=484
  RETURN CFNMETA(-(-(((-(5)/2)/(7/3)))))+'|'+HexBytes(BINTOC(-(-(((-(5)/2)/(7/3)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((-(5)/2)/(7/3))))))
 CASE index=485
  RETURN CFNMETA(IIF(.T.,((-(5)/2)/(7/3)),0))+'|'+HexBytes(BINTOC(IIF(.T.,((-(5)/2)/(7/3)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((-(5)/2)/(7/3)),0)))
 CASE index=486
  RETURN CFNMETA(-(IIF(.T.,((-(5)/2)/(7/3)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((-(5)/2)/(7/3)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((-(5)/2)/(7/3)),0))))
 CASE index=487
  RETURN CFNMETA(EVALUATE("((-(5)/2)/(7/3))"))+'|'+HexBytes(BINTOC(EVALUATE("((-(5)/2)/(7/3))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((-(5)/2)/(7/3))")))
 CASE index=488
  RETURN CFNMETA(-(EVALUATE("((-(5)/2)/(7/3))")))+'|'+HexBytes(BINTOC(-(EVALUATE("((-(5)/2)/(7/3))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((-(5)/2)/(7/3))"))))
 CASE index=489
  RETURN CFNMETA(((5/-(2))/(7/3)))+'|'+HexBytes(BINTOC(((5/-(2))/(7/3)),'BRS'))+'|'+HexBytes(TRANSFORM(((5/-(2))/(7/3))))
 CASE index=490
  RETURN CFNMETA(+(((5/-(2))/(7/3))))+'|'+HexBytes(BINTOC(+(((5/-(2))/(7/3))),'BRS'))+'|'+HexBytes(TRANSFORM(+(((5/-(2))/(7/3)))))
 CASE index=491
  RETURN CFNMETA(-(((5/-(2))/(7/3))))+'|'+HexBytes(BINTOC(-(((5/-(2))/(7/3))),'BRS'))+'|'+HexBytes(TRANSFORM(-(((5/-(2))/(7/3)))))
 CASE index=492
  RETURN CFNMETA(-(-(((5/-(2))/(7/3)))))+'|'+HexBytes(BINTOC(-(-(((5/-(2))/(7/3)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((5/-(2))/(7/3))))))
 CASE index=493
  RETURN CFNMETA(IIF(.T.,((5/-(2))/(7/3)),0))+'|'+HexBytes(BINTOC(IIF(.T.,((5/-(2))/(7/3)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((5/-(2))/(7/3)),0)))
 CASE index=494
  RETURN CFNMETA(-(IIF(.T.,((5/-(2))/(7/3)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((5/-(2))/(7/3)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((5/-(2))/(7/3)),0))))
 CASE index=495
  RETURN CFNMETA(EVALUATE("((5/-(2))/(7/3))"))+'|'+HexBytes(BINTOC(EVALUATE("((5/-(2))/(7/3))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((5/-(2))/(7/3))")))
 CASE index=496
  RETURN CFNMETA(-(EVALUATE("((5/-(2))/(7/3))")))+'|'+HexBytes(BINTOC(-(EVALUATE("((5/-(2))/(7/3))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((5/-(2))/(7/3))"))))
 CASE index=497
  RETURN CFNMETA(((5/2)/(-(7)/3)))+'|'+HexBytes(BINTOC(((5/2)/(-(7)/3)),'BRS'))+'|'+HexBytes(TRANSFORM(((5/2)/(-(7)/3))))
 CASE index=498
  RETURN CFNMETA(+(((5/2)/(-(7)/3))))+'|'+HexBytes(BINTOC(+(((5/2)/(-(7)/3))),'BRS'))+'|'+HexBytes(TRANSFORM(+(((5/2)/(-(7)/3)))))
 CASE index=499
  RETURN CFNMETA(-(((5/2)/(-(7)/3))))+'|'+HexBytes(BINTOC(-(((5/2)/(-(7)/3))),'BRS'))+'|'+HexBytes(TRANSFORM(-(((5/2)/(-(7)/3)))))
 CASE index=500
  RETURN CFNMETA(-(-(((5/2)/(-(7)/3)))))+'|'+HexBytes(BINTOC(-(-(((5/2)/(-(7)/3)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((5/2)/(-(7)/3))))))
 CASE index=501
  RETURN CFNMETA(IIF(.T.,((5/2)/(-(7)/3)),0))+'|'+HexBytes(BINTOC(IIF(.T.,((5/2)/(-(7)/3)),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((5/2)/(-(7)/3)),0)))
 CASE index=502
  RETURN CFNMETA(-(IIF(.T.,((5/2)/(-(7)/3)),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((5/2)/(-(7)/3)),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((5/2)/(-(7)/3)),0))))
 CASE index=503
  RETURN CFNMETA(EVALUATE("((5/2)/(-(7)/3))"))+'|'+HexBytes(BINTOC(EVALUATE("((5/2)/(-(7)/3))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((5/2)/(-(7)/3))")))
 CASE index=504
  RETURN CFNMETA(-(EVALUATE("((5/2)/(-(7)/3))")))+'|'+HexBytes(BINTOC(-(EVALUATE("((5/2)/(-(7)/3))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((5/2)/(-(7)/3))"))))
 CASE index=505
  RETURN CFNMETA(((5/2)/(7/-(3))))+'|'+HexBytes(BINTOC(((5/2)/(7/-(3))),'BRS'))+'|'+HexBytes(TRANSFORM(((5/2)/(7/-(3)))))
 CASE index=506
  RETURN CFNMETA(+(((5/2)/(7/-(3)))))+'|'+HexBytes(BINTOC(+(((5/2)/(7/-(3)))),'BRS'))+'|'+HexBytes(TRANSFORM(+(((5/2)/(7/-(3))))))
 CASE index=507
  RETURN CFNMETA(-(((5/2)/(7/-(3)))))+'|'+HexBytes(BINTOC(-(((5/2)/(7/-(3)))),'BRS'))+'|'+HexBytes(TRANSFORM(-(((5/2)/(7/-(3))))))
 CASE index=508
  RETURN CFNMETA(-(-(((5/2)/(7/-(3))))))+'|'+HexBytes(BINTOC(-(-(((5/2)/(7/-(3))))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-(((5/2)/(7/-(3)))))))
 CASE index=509
  RETURN CFNMETA(IIF(.T.,((5/2)/(7/-(3))),0))+'|'+HexBytes(BINTOC(IIF(.T.,((5/2)/(7/-(3))),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,((5/2)/(7/-(3))),0)))
 CASE index=510
  RETURN CFNMETA(-(IIF(.T.,((5/2)/(7/-(3))),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,((5/2)/(7/-(3))),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,((5/2)/(7/-(3))),0))))
 CASE index=511
  RETURN CFNMETA(EVALUATE("((5/2)/(7/-(3)))"))+'|'+HexBytes(BINTOC(EVALUATE("((5/2)/(7/-(3)))"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("((5/2)/(7/-(3)))")))
 CASE index=512
  RETURN CFNMETA(-(EVALUATE("((5/2)/(7/-(3)))")))+'|'+HexBytes(BINTOC(-(EVALUATE("((5/2)/(7/-(3)))")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("((5/2)/(7/-(3)))"))))
 CASE index=513
  RETURN CFNMETA((1.0000000000000002/2.5))+'|'+HexBytes(BINTOC((1.0000000000000002/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((1.0000000000000002/2.5)))
 CASE index=514
  RETURN CFNMETA(+((1.0000000000000002/2.5)))+'|'+HexBytes(BINTOC(+((1.0000000000000002/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((1.0000000000000002/2.5))))
 CASE index=515
  RETURN CFNMETA(-((1.0000000000000002/2.5)))+'|'+HexBytes(BINTOC(-((1.0000000000000002/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.0000000000000002/2.5))))
 CASE index=516
  RETURN CFNMETA(-(-((1.0000000000000002/2.5))))+'|'+HexBytes(BINTOC(-(-((1.0000000000000002/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((1.0000000000000002/2.5)))))
 CASE index=517
  RETURN CFNMETA(IIF(.T.,(1.0000000000000002/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.0000000000000002/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.0000000000000002/2.5),0)))
 CASE index=518
  RETURN CFNMETA(-(IIF(.T.,(1.0000000000000002/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.0000000000000002/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.0000000000000002/2.5),0))))
 CASE index=519
  RETURN CFNMETA(EVALUATE("(1.0000000000000002/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(1.0000000000000002/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(1.0000000000000002/2.5)")))
 CASE index=520
  RETURN CFNMETA(-(EVALUATE("(1.0000000000000002/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(1.0000000000000002/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(1.0000000000000002/2.5)"))))
 CASE index=521
  RETURN CFNMETA((0.9999999999999999/2.5))+'|'+HexBytes(BINTOC((0.9999999999999999/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((0.9999999999999999/2.5)))
 CASE index=522
  RETURN CFNMETA(+((0.9999999999999999/2.5)))+'|'+HexBytes(BINTOC(+((0.9999999999999999/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((0.9999999999999999/2.5))))
 CASE index=523
  RETURN CFNMETA(-((0.9999999999999999/2.5)))+'|'+HexBytes(BINTOC(-((0.9999999999999999/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((0.9999999999999999/2.5))))
 CASE index=524
  RETURN CFNMETA(-(-((0.9999999999999999/2.5))))+'|'+HexBytes(BINTOC(-(-((0.9999999999999999/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((0.9999999999999999/2.5)))))
 CASE index=525
  RETURN CFNMETA(IIF(.T.,(0.9999999999999999/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(0.9999999999999999/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(0.9999999999999999/2.5),0)))
 CASE index=526
  RETURN CFNMETA(-(IIF(.T.,(0.9999999999999999/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(0.9999999999999999/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(0.9999999999999999/2.5),0))))
 CASE index=527
  RETURN CFNMETA(EVALUATE("(0.9999999999999999/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(0.9999999999999999/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(0.9999999999999999/2.5)")))
 CASE index=528
  RETURN CFNMETA(-(EVALUATE("(0.9999999999999999/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(0.9999999999999999/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(0.9999999999999999/2.5)"))))
 CASE index=529
  RETURN CFNMETA((3.9999999999999996/2.5))+'|'+HexBytes(BINTOC((3.9999999999999996/2.5),'BRS'))+'|'+HexBytes(TRANSFORM((3.9999999999999996/2.5)))
 CASE index=530
  RETURN CFNMETA(+((3.9999999999999996/2.5)))+'|'+HexBytes(BINTOC(+((3.9999999999999996/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(+((3.9999999999999996/2.5))))
 CASE index=531
  RETURN CFNMETA(-((3.9999999999999996/2.5)))+'|'+HexBytes(BINTOC(-((3.9999999999999996/2.5)),'BRS'))+'|'+HexBytes(TRANSFORM(-((3.9999999999999996/2.5))))
 CASE index=532
  RETURN CFNMETA(-(-((3.9999999999999996/2.5))))+'|'+HexBytes(BINTOC(-(-((3.9999999999999996/2.5))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((3.9999999999999996/2.5)))))
 CASE index=533
  RETURN CFNMETA(IIF(.T.,(3.9999999999999996/2.5),0))+'|'+HexBytes(BINTOC(IIF(.T.,(3.9999999999999996/2.5),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(3.9999999999999996/2.5),0)))
 CASE index=534
  RETURN CFNMETA(-(IIF(.T.,(3.9999999999999996/2.5),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(3.9999999999999996/2.5),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(3.9999999999999996/2.5),0))))
 CASE index=535
  RETURN CFNMETA(EVALUATE("(3.9999999999999996/2.5)"))+'|'+HexBytes(BINTOC(EVALUATE("(3.9999999999999996/2.5)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(3.9999999999999996/2.5)")))
 CASE index=536
  RETURN CFNMETA(-(EVALUATE("(3.9999999999999996/2.5)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(3.9999999999999996/2.5)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(3.9999999999999996/2.5)"))))
 CASE index=537
  RETURN CFNMETA((1.234567890123456789/3))+'|'+HexBytes(BINTOC((1.234567890123456789/3),'BRS'))+'|'+HexBytes(TRANSFORM((1.234567890123456789/3)))
 CASE index=538
  RETURN CFNMETA(+((1.234567890123456789/3)))+'|'+HexBytes(BINTOC(+((1.234567890123456789/3)),'BRS'))+'|'+HexBytes(TRANSFORM(+((1.234567890123456789/3))))
 CASE index=539
  RETURN CFNMETA(-((1.234567890123456789/3)))+'|'+HexBytes(BINTOC(-((1.234567890123456789/3)),'BRS'))+'|'+HexBytes(TRANSFORM(-((1.234567890123456789/3))))
 CASE index=540
  RETURN CFNMETA(-(-((1.234567890123456789/3))))+'|'+HexBytes(BINTOC(-(-((1.234567890123456789/3))),'BRS'))+'|'+HexBytes(TRANSFORM(-(-((1.234567890123456789/3)))))
 CASE index=541
  RETURN CFNMETA(IIF(.T.,(1.234567890123456789/3),0))+'|'+HexBytes(BINTOC(IIF(.T.,(1.234567890123456789/3),0),'BRS'))+'|'+HexBytes(TRANSFORM(IIF(.T.,(1.234567890123456789/3),0)))
 CASE index=542
  RETURN CFNMETA(-(IIF(.T.,(1.234567890123456789/3),0)))+'|'+HexBytes(BINTOC(-(IIF(.T.,(1.234567890123456789/3),0)),'BRS'))+'|'+HexBytes(TRANSFORM(-(IIF(.T.,(1.234567890123456789/3),0))))
 CASE index=543
  RETURN CFNMETA(EVALUATE("(1.234567890123456789/3)"))+'|'+HexBytes(BINTOC(EVALUATE("(1.234567890123456789/3)"),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE("(1.234567890123456789/3)")))
 CASE index=544
  RETURN CFNMETA(-(EVALUATE("(1.234567890123456789/3)")))+'|'+HexBytes(BINTOC(-(EVALUATE("(1.234567890123456789/3)")),'BRS'))+'|'+HexBytes(TRANSFORM(-(EVALUATE("(1.234567890123456789/3)"))))
 CASE index=545
  RETURN CFNMETA(0.5)+'|'+HexBytes(BINTOC(0.5,'BRS'))+'|'+HexBytes(TRANSFORM(0.5))
 CASE index=546
  RETURN CFNMETA(2)+'|'+HexBytes(BINTOC(2,'BRS'))+'|'+HexBytes(TRANSFORM(2))
 CASE index=547
  RETURN CFNMETA(2.5)+'|'+HexBytes(BINTOC(2.5,'BRS'))+'|'+HexBytes(TRANSFORM(2.5))
 CASE index=548
  RETURN CFNMETA(02.50)+'|'+HexBytes(BINTOC(02.50,'BRS'))+'|'+HexBytes(TRANSFORM(02.50))
 CASE index=549
  RETURN CFNMETA(2.50)+'|'+HexBytes(BINTOC(2.50,'BRS'))+'|'+HexBytes(TRANSFORM(2.50))
 CASE index=550
  RETURN CFNMETA(0.000000000000000001)+'|'+HexBytes(BINTOC(0.000000000000000001,'BRS'))+'|'+HexBytes(TRANSFORM(0.000000000000000001))
 CASE index=551
  RETURN CFNMETA(1.234567890123456789)+'|'+HexBytes(BINTOC(1.234567890123456789,'BRS'))+'|'+HexBytes(TRANSFORM(1.234567890123456789))
 CASE index=552
  RETURN CFNMETA(5)+'|'+HexBytes(BINTOC(5,'BRS'))+'|'+HexBytes(TRANSFORM(5))
 CASE index=553
  RETURN CFNMETA(1)+'|'+HexBytes(BINTOC(1,'BRS'))+'|'+HexBytes(TRANSFORM(1))
 CASE index=554
  RETURN CFNMETA(7)+'|'+HexBytes(BINTOC(7,'BRS'))+'|'+HexBytes(TRANSFORM(7))
 CASE index=555
  RETURN CFNMETA(3)+'|'+HexBytes(BINTOC(3,'BRS'))+'|'+HexBytes(TRANSFORM(3))
 CASE index=556
  RETURN CFNMETA(5.00)+'|'+HexBytes(BINTOC(5.00,'BRS'))+'|'+HexBytes(TRANSFORM(5.00))
 CASE index=557
  RETURN CFNMETA(2.00)+'|'+HexBytes(BINTOC(2.00,'BRS'))+'|'+HexBytes(TRANSFORM(2.00))
 CASE index=558
  RETURN CFNMETA(7.0)+'|'+HexBytes(BINTOC(7.0,'BRS'))+'|'+HexBytes(TRANSFORM(7.0))
 CASE index=559
  RETURN CFNMETA(3.0)+'|'+HexBytes(BINTOC(3.0,'BRS'))+'|'+HexBytes(TRANSFORM(3.0))
 CASE index=560
  RETURN CFNMETA(1.0000000000000002)+'|'+HexBytes(BINTOC(1.0000000000000002,'BRS'))+'|'+HexBytes(TRANSFORM(1.0000000000000002))
 CASE index=561
  RETURN CFNMETA(0.9999999999999999)+'|'+HexBytes(BINTOC(0.9999999999999999,'BRS'))+'|'+HexBytes(TRANSFORM(0.9999999999999999))
 CASE index=562
  RETURN CFNMETA(3.9999999999999996)+'|'+HexBytes(BINTOC(3.9999999999999996,'BRS'))+'|'+HexBytes(TRANSFORM(3.9999999999999996))
 CASE index=563
  RETURN CFNMETA((1/7)/iTwo)+'|'+HexBytes(BINTOC((1/7)/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM((1/7)/iTwo))
 CASE index=564
  RETURN CFNMETA(iTwo/(5/2))+'|'+HexBytes(BINTOC(iTwo/(5/2),'BRS'))+'|'+HexBytes(TRANSFORM(iTwo/(5/2)))
 CASE index=565
  RETURN CFNMETA(((5/2)/(5/2))/iTwo)+'|'+HexBytes(BINTOC(((5/2)/(5/2))/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(((5/2)/(5/2))/iTwo))
 CASE index=566
  RETURN CFNMETA(iTwo/((5/2)/(5/2)))+'|'+HexBytes(BINTOC(iTwo/((5/2)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(iTwo/((5/2)/(5/2))))
 CASE index=567
  RETURN CFNMETA((2.50/2)/nTwo)+'|'+HexBytes(BINTOC((2.50/2)/nTwo,'BRS'))+'|'+HexBytes(TRANSFORM((2.50/2)/nTwo))
 CASE index=568
  RETURN CFNMETA(nTwo/(2.50/2))+'|'+HexBytes(BINTOC(nTwo/(2.50/2),'BRS'))+'|'+HexBytes(TRANSFORM(nTwo/(2.50/2)))
 CASE index=569
  RETURN CFNMETA(-((1/7)/iTwo))+'|'+HexBytes(BINTOC(-((1/7)/iTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-((1/7)/iTwo)))
 CASE index=570
  RETURN CFNMETA(-(iTwo/(5/2)))+'|'+HexBytes(BINTOC(-(iTwo/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(-(iTwo/(5/2))))
 CASE index=571
  RETURN CFNMETA(((1/7)/3)/iTwo)+'|'+HexBytes(BINTOC(((1/7)/3)/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(((1/7)/3)/iTwo))
 CASE index=572
  RETURN CFNMETA(((5/2)/(7/3))/iTwo)+'|'+HexBytes(BINTOC(((5/2)/(7/3))/iTwo,'BRS'))+'|'+HexBytes(TRANSFORM(((5/2)/(7/3))/iTwo))
 CASE index=573
  RETURN CFNMETA(-(((5/2)/(7/3))/iTwo))+'|'+HexBytes(BINTOC(-(((5/2)/(7/3))/iTwo),'BRS'))+'|'+HexBytes(TRANSFORM(-(((5/2)/(7/3))/iTwo)))
 CASE index=574
  RETURN CFNMETA(iTwo/((7/3)/(5/2)))+'|'+HexBytes(BINTOC(iTwo/((7/3)/(5/2)),'BRS'))+'|'+HexBytes(TRANSFORM(iTwo/((7/3)/(5/2))))
 CASE index=575
  RETURN CFNMETA(5/0.5)+'|'+HexBytes(BINTOC(5/0.5,'BRS'))+'|'+HexBytes(TRANSFORM(5/0.5))
 CASE index=576
  RETURN CFNMETA((1/3)/(1/3))+'|'+HexBytes(BINTOC((1/3)/(1/3),'BRS'))+'|'+HexBytes(TRANSFORM((1/3)/(1/3)))
 CASE index=577
  RETURN CFNMETA(1E-14/2)+'|'+HexBytes(BINTOC(1E-14/2,'BRS'))+'|'+HexBytes(TRANSFORM(1E-14/2))
 CASE index=578
  RETURN CFNMETA(9999999999/2.5)+'|'+HexBytes(BINTOC(9999999999/2.5,'BRS'))+'|'+HexBytes(TRANSFORM(9999999999/2.5))
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
