* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: native intervention/source freeze.
LOCAL ARRAY expressions[384],settings[3]
expressions[1]='0.50'
expressions[2]='CFNCLONE(0.50)'
expressions[3]='CFNDEC(CFNWIDTH(0.50,1),0)'
expressions[4]='CFNDEC(CFNWIDTH(0.50,1),1)'
expressions[5]='CFNDEC(CFNWIDTH(0.50,1),2)'
expressions[6]='CFNDEC(CFNWIDTH(0.50,1),14)'
expressions[7]='CFNDEC(CFNWIDTH(0.50,1),15)'
expressions[8]='CFNDEC(CFNWIDTH(0.50,1),18)'
expressions[9]='CFNDEC(CFNWIDTH(0.50,2),0)'
expressions[10]='CFNDEC(CFNWIDTH(0.50,2),1)'
expressions[11]='CFNDEC(CFNWIDTH(0.50,2),2)'
expressions[12]='CFNDEC(CFNWIDTH(0.50,2),14)'
expressions[13]='CFNDEC(CFNWIDTH(0.50,2),15)'
expressions[14]='CFNDEC(CFNWIDTH(0.50,2),18)'
expressions[15]='CFNDEC(CFNWIDTH(0.50,3),0)'
expressions[16]='CFNDEC(CFNWIDTH(0.50,3),1)'
expressions[17]='CFNDEC(CFNWIDTH(0.50,3),2)'
expressions[18]='CFNDEC(CFNWIDTH(0.50,3),14)'
expressions[19]='CFNDEC(CFNWIDTH(0.50,3),15)'
expressions[20]='CFNDEC(CFNWIDTH(0.50,3),18)'
expressions[21]='CFNDEC(CFNWIDTH(0.50,4),0)'
expressions[22]='CFNDEC(CFNWIDTH(0.50,4),1)'
expressions[23]='CFNDEC(CFNWIDTH(0.50,4),2)'
expressions[24]='CFNDEC(CFNWIDTH(0.50,4),14)'
expressions[25]='CFNDEC(CFNWIDTH(0.50,4),15)'
expressions[26]='CFNDEC(CFNWIDTH(0.50,4),18)'
expressions[27]='CFNDEC(CFNWIDTH(0.50,12),0)'
expressions[28]='CFNDEC(CFNWIDTH(0.50,12),1)'
expressions[29]='CFNDEC(CFNWIDTH(0.50,12),2)'
expressions[30]='CFNDEC(CFNWIDTH(0.50,12),14)'
expressions[31]='CFNDEC(CFNWIDTH(0.50,12),15)'
expressions[32]='CFNDEC(CFNWIDTH(0.50,12),18)'
expressions[33]='CFNDEC(CFNWIDTH(0.50,20),0)'
expressions[34]='CFNDEC(CFNWIDTH(0.50,20),1)'
expressions[35]='CFNDEC(CFNWIDTH(0.50,20),2)'
expressions[36]='CFNDEC(CFNWIDTH(0.50,20),14)'
expressions[37]='CFNDEC(CFNWIDTH(0.50,20),15)'
expressions[38]='CFNDEC(CFNWIDTH(0.50,20),18)'
expressions[39]='CFNDEC(CFNWIDTH(0.50,40),0)'
expressions[40]='CFNDEC(CFNWIDTH(0.50,40),1)'
expressions[41]='CFNDEC(CFNWIDTH(0.50,40),2)'
expressions[42]='CFNDEC(CFNWIDTH(0.50,40),14)'
expressions[43]='CFNDEC(CFNWIDTH(0.50,40),15)'
expressions[44]='CFNDEC(CFNWIDTH(0.50,40),18)'
expressions[45]='-0.50'
expressions[46]='CFNCLONE(-0.50)'
expressions[47]='CFNDEC(CFNWIDTH(-0.50,1),0)'
expressions[48]='CFNDEC(CFNWIDTH(-0.50,1),1)'
expressions[49]='CFNDEC(CFNWIDTH(-0.50,1),2)'
expressions[50]='CFNDEC(CFNWIDTH(-0.50,1),14)'
expressions[51]='CFNDEC(CFNWIDTH(-0.50,1),15)'
expressions[52]='CFNDEC(CFNWIDTH(-0.50,1),18)'
expressions[53]='CFNDEC(CFNWIDTH(-0.50,2),0)'
expressions[54]='CFNDEC(CFNWIDTH(-0.50,2),1)'
expressions[55]='CFNDEC(CFNWIDTH(-0.50,2),2)'
expressions[56]='CFNDEC(CFNWIDTH(-0.50,2),14)'
expressions[57]='CFNDEC(CFNWIDTH(-0.50,2),15)'
expressions[58]='CFNDEC(CFNWIDTH(-0.50,2),18)'
expressions[59]='CFNDEC(CFNWIDTH(-0.50,3),0)'
expressions[60]='CFNDEC(CFNWIDTH(-0.50,3),1)'
expressions[61]='CFNDEC(CFNWIDTH(-0.50,3),2)'
expressions[62]='CFNDEC(CFNWIDTH(-0.50,3),14)'
expressions[63]='CFNDEC(CFNWIDTH(-0.50,3),15)'
expressions[64]='CFNDEC(CFNWIDTH(-0.50,3),18)'
expressions[65]='CFNDEC(CFNWIDTH(-0.50,4),0)'
expressions[66]='CFNDEC(CFNWIDTH(-0.50,4),1)'
expressions[67]='CFNDEC(CFNWIDTH(-0.50,4),2)'
expressions[68]='CFNDEC(CFNWIDTH(-0.50,4),14)'
expressions[69]='CFNDEC(CFNWIDTH(-0.50,4),15)'
expressions[70]='CFNDEC(CFNWIDTH(-0.50,4),18)'
expressions[71]='CFNDEC(CFNWIDTH(-0.50,12),0)'
expressions[72]='CFNDEC(CFNWIDTH(-0.50,12),1)'
expressions[73]='CFNDEC(CFNWIDTH(-0.50,12),2)'
expressions[74]='CFNDEC(CFNWIDTH(-0.50,12),14)'
expressions[75]='CFNDEC(CFNWIDTH(-0.50,12),15)'
expressions[76]='CFNDEC(CFNWIDTH(-0.50,12),18)'
expressions[77]='CFNDEC(CFNWIDTH(-0.50,20),0)'
expressions[78]='CFNDEC(CFNWIDTH(-0.50,20),1)'
expressions[79]='CFNDEC(CFNWIDTH(-0.50,20),2)'
expressions[80]='CFNDEC(CFNWIDTH(-0.50,20),14)'
expressions[81]='CFNDEC(CFNWIDTH(-0.50,20),15)'
expressions[82]='CFNDEC(CFNWIDTH(-0.50,20),18)'
expressions[83]='CFNDEC(CFNWIDTH(-0.50,40),0)'
expressions[84]='CFNDEC(CFNWIDTH(-0.50,40),1)'
expressions[85]='CFNDEC(CFNWIDTH(-0.50,40),2)'
expressions[86]='CFNDEC(CFNWIDTH(-0.50,40),14)'
expressions[87]='CFNDEC(CFNWIDTH(-0.50,40),15)'
expressions[88]='CFNDEC(CFNWIDTH(-0.50,40),18)'
expressions[89]='0.00'
expressions[90]='CFNCLONE(0.00)'
expressions[91]='CFNDEC(CFNWIDTH(0.00,1),0)'
expressions[92]='CFNDEC(CFNWIDTH(0.00,1),1)'
expressions[93]='CFNDEC(CFNWIDTH(0.00,1),2)'
expressions[94]='CFNDEC(CFNWIDTH(0.00,1),14)'
expressions[95]='CFNDEC(CFNWIDTH(0.00,1),15)'
expressions[96]='CFNDEC(CFNWIDTH(0.00,1),18)'
expressions[97]='CFNDEC(CFNWIDTH(0.00,2),0)'
expressions[98]='CFNDEC(CFNWIDTH(0.00,2),1)'
expressions[99]='CFNDEC(CFNWIDTH(0.00,2),2)'
expressions[100]='CFNDEC(CFNWIDTH(0.00,2),14)'
expressions[101]='CFNDEC(CFNWIDTH(0.00,2),15)'
expressions[102]='CFNDEC(CFNWIDTH(0.00,2),18)'
expressions[103]='CFNDEC(CFNWIDTH(0.00,3),0)'
expressions[104]='CFNDEC(CFNWIDTH(0.00,3),1)'
expressions[105]='CFNDEC(CFNWIDTH(0.00,3),2)'
expressions[106]='CFNDEC(CFNWIDTH(0.00,3),14)'
expressions[107]='CFNDEC(CFNWIDTH(0.00,3),15)'
expressions[108]='CFNDEC(CFNWIDTH(0.00,3),18)'
expressions[109]='CFNDEC(CFNWIDTH(0.00,4),0)'
expressions[110]='CFNDEC(CFNWIDTH(0.00,4),1)'
expressions[111]='CFNDEC(CFNWIDTH(0.00,4),2)'
expressions[112]='CFNDEC(CFNWIDTH(0.00,4),14)'
expressions[113]='CFNDEC(CFNWIDTH(0.00,4),15)'
expressions[114]='CFNDEC(CFNWIDTH(0.00,4),18)'
expressions[115]='CFNDEC(CFNWIDTH(0.00,12),0)'
expressions[116]='CFNDEC(CFNWIDTH(0.00,12),1)'
expressions[117]='CFNDEC(CFNWIDTH(0.00,12),2)'
expressions[118]='CFNDEC(CFNWIDTH(0.00,12),14)'
expressions[119]='CFNDEC(CFNWIDTH(0.00,12),15)'
expressions[120]='CFNDEC(CFNWIDTH(0.00,12),18)'
expressions[121]='CFNDEC(CFNWIDTH(0.00,20),0)'
expressions[122]='CFNDEC(CFNWIDTH(0.00,20),1)'
expressions[123]='CFNDEC(CFNWIDTH(0.00,20),2)'
expressions[124]='CFNDEC(CFNWIDTH(0.00,20),14)'
expressions[125]='CFNDEC(CFNWIDTH(0.00,20),15)'
expressions[126]='CFNDEC(CFNWIDTH(0.00,20),18)'
expressions[127]='CFNDEC(CFNWIDTH(0.00,40),0)'
expressions[128]='CFNDEC(CFNWIDTH(0.00,40),1)'
expressions[129]='CFNDEC(CFNWIDTH(0.00,40),2)'
expressions[130]='CFNDEC(CFNWIDTH(0.00,40),14)'
expressions[131]='CFNDEC(CFNWIDTH(0.00,40),15)'
expressions[132]='CFNDEC(CFNWIDTH(0.00,40),18)'
expressions[133]='-0.00'
expressions[134]='CFNCLONE(-0.00)'
expressions[135]='CFNDEC(CFNWIDTH(-0.00,1),0)'
expressions[136]='CFNDEC(CFNWIDTH(-0.00,1),1)'
expressions[137]='CFNDEC(CFNWIDTH(-0.00,1),2)'
expressions[138]='CFNDEC(CFNWIDTH(-0.00,1),14)'
expressions[139]='CFNDEC(CFNWIDTH(-0.00,1),15)'
expressions[140]='CFNDEC(CFNWIDTH(-0.00,1),18)'
expressions[141]='CFNDEC(CFNWIDTH(-0.00,2),0)'
expressions[142]='CFNDEC(CFNWIDTH(-0.00,2),1)'
expressions[143]='CFNDEC(CFNWIDTH(-0.00,2),2)'
expressions[144]='CFNDEC(CFNWIDTH(-0.00,2),14)'
expressions[145]='CFNDEC(CFNWIDTH(-0.00,2),15)'
expressions[146]='CFNDEC(CFNWIDTH(-0.00,2),18)'
expressions[147]='CFNDEC(CFNWIDTH(-0.00,3),0)'
expressions[148]='CFNDEC(CFNWIDTH(-0.00,3),1)'
expressions[149]='CFNDEC(CFNWIDTH(-0.00,3),2)'
expressions[150]='CFNDEC(CFNWIDTH(-0.00,3),14)'
expressions[151]='CFNDEC(CFNWIDTH(-0.00,3),15)'
expressions[152]='CFNDEC(CFNWIDTH(-0.00,3),18)'
expressions[153]='CFNDEC(CFNWIDTH(-0.00,4),0)'
expressions[154]='CFNDEC(CFNWIDTH(-0.00,4),1)'
expressions[155]='CFNDEC(CFNWIDTH(-0.00,4),2)'
expressions[156]='CFNDEC(CFNWIDTH(-0.00,4),14)'
expressions[157]='CFNDEC(CFNWIDTH(-0.00,4),15)'
expressions[158]='CFNDEC(CFNWIDTH(-0.00,4),18)'
expressions[159]='CFNDEC(CFNWIDTH(-0.00,12),0)'
expressions[160]='CFNDEC(CFNWIDTH(-0.00,12),1)'
expressions[161]='CFNDEC(CFNWIDTH(-0.00,12),2)'
expressions[162]='CFNDEC(CFNWIDTH(-0.00,12),14)'
expressions[163]='CFNDEC(CFNWIDTH(-0.00,12),15)'
expressions[164]='CFNDEC(CFNWIDTH(-0.00,12),18)'
expressions[165]='CFNDEC(CFNWIDTH(-0.00,20),0)'
expressions[166]='CFNDEC(CFNWIDTH(-0.00,20),1)'
expressions[167]='CFNDEC(CFNWIDTH(-0.00,20),2)'
expressions[168]='CFNDEC(CFNWIDTH(-0.00,20),14)'
expressions[169]='CFNDEC(CFNWIDTH(-0.00,20),15)'
expressions[170]='CFNDEC(CFNWIDTH(-0.00,20),18)'
expressions[171]='CFNDEC(CFNWIDTH(-0.00,40),0)'
expressions[172]='CFNDEC(CFNWIDTH(-0.00,40),1)'
expressions[173]='CFNDEC(CFNWIDTH(-0.00,40),2)'
expressions[174]='CFNDEC(CFNWIDTH(-0.00,40),14)'
expressions[175]='CFNDEC(CFNWIDTH(-0.00,40),15)'
expressions[176]='CFNDEC(CFNWIDTH(-0.00,40),18)'
expressions[177]='1E-14'
expressions[178]='CFNCLONE(1E-14)'
expressions[179]='CFNDEC(CFNWIDTH(1E-14,1),0)'
expressions[180]='CFNDEC(CFNWIDTH(1E-14,1),1)'
expressions[181]='CFNDEC(CFNWIDTH(1E-14,1),2)'
expressions[182]='CFNDEC(CFNWIDTH(1E-14,1),14)'
expressions[183]='CFNDEC(CFNWIDTH(1E-14,1),15)'
expressions[184]='CFNDEC(CFNWIDTH(1E-14,1),18)'
expressions[185]='CFNDEC(CFNWIDTH(1E-14,2),0)'
expressions[186]='CFNDEC(CFNWIDTH(1E-14,2),1)'
expressions[187]='CFNDEC(CFNWIDTH(1E-14,2),2)'
expressions[188]='CFNDEC(CFNWIDTH(1E-14,2),14)'
expressions[189]='CFNDEC(CFNWIDTH(1E-14,2),15)'
expressions[190]='CFNDEC(CFNWIDTH(1E-14,2),18)'
expressions[191]='CFNDEC(CFNWIDTH(1E-14,3),0)'
expressions[192]='CFNDEC(CFNWIDTH(1E-14,3),1)'
expressions[193]='CFNDEC(CFNWIDTH(1E-14,3),2)'
expressions[194]='CFNDEC(CFNWIDTH(1E-14,3),14)'
expressions[195]='CFNDEC(CFNWIDTH(1E-14,3),15)'
expressions[196]='CFNDEC(CFNWIDTH(1E-14,3),18)'
expressions[197]='CFNDEC(CFNWIDTH(1E-14,4),0)'
expressions[198]='CFNDEC(CFNWIDTH(1E-14,4),1)'
expressions[199]='CFNDEC(CFNWIDTH(1E-14,4),2)'
expressions[200]='CFNDEC(CFNWIDTH(1E-14,4),14)'
expressions[201]='CFNDEC(CFNWIDTH(1E-14,4),15)'
expressions[202]='CFNDEC(CFNWIDTH(1E-14,4),18)'
expressions[203]='CFNDEC(CFNWIDTH(1E-14,12),0)'
expressions[204]='CFNDEC(CFNWIDTH(1E-14,12),1)'
expressions[205]='CFNDEC(CFNWIDTH(1E-14,12),2)'
expressions[206]='CFNDEC(CFNWIDTH(1E-14,12),14)'
expressions[207]='CFNDEC(CFNWIDTH(1E-14,12),15)'
expressions[208]='CFNDEC(CFNWIDTH(1E-14,12),18)'
expressions[209]='CFNDEC(CFNWIDTH(1E-14,20),0)'
expressions[210]='CFNDEC(CFNWIDTH(1E-14,20),1)'
expressions[211]='CFNDEC(CFNWIDTH(1E-14,20),2)'
expressions[212]='CFNDEC(CFNWIDTH(1E-14,20),14)'
expressions[213]='CFNDEC(CFNWIDTH(1E-14,20),15)'
expressions[214]='CFNDEC(CFNWIDTH(1E-14,20),18)'
expressions[215]='CFNDEC(CFNWIDTH(1E-14,40),0)'
expressions[216]='CFNDEC(CFNWIDTH(1E-14,40),1)'
expressions[217]='CFNDEC(CFNWIDTH(1E-14,40),2)'
expressions[218]='CFNDEC(CFNWIDTH(1E-14,40),14)'
expressions[219]='CFNDEC(CFNWIDTH(1E-14,40),15)'
expressions[220]='CFNDEC(CFNWIDTH(1E-14,40),18)'
expressions[221]='1E-18'
expressions[222]='CFNCLONE(1E-18)'
expressions[223]='CFNDEC(CFNWIDTH(1E-18,1),0)'
expressions[224]='CFNDEC(CFNWIDTH(1E-18,1),1)'
expressions[225]='CFNDEC(CFNWIDTH(1E-18,1),2)'
expressions[226]='CFNDEC(CFNWIDTH(1E-18,1),14)'
expressions[227]='CFNDEC(CFNWIDTH(1E-18,1),15)'
expressions[228]='CFNDEC(CFNWIDTH(1E-18,1),18)'
expressions[229]='CFNDEC(CFNWIDTH(1E-18,2),0)'
expressions[230]='CFNDEC(CFNWIDTH(1E-18,2),1)'
expressions[231]='CFNDEC(CFNWIDTH(1E-18,2),2)'
expressions[232]='CFNDEC(CFNWIDTH(1E-18,2),14)'
expressions[233]='CFNDEC(CFNWIDTH(1E-18,2),15)'
expressions[234]='CFNDEC(CFNWIDTH(1E-18,2),18)'
expressions[235]='CFNDEC(CFNWIDTH(1E-18,3),0)'
expressions[236]='CFNDEC(CFNWIDTH(1E-18,3),1)'
expressions[237]='CFNDEC(CFNWIDTH(1E-18,3),2)'
expressions[238]='CFNDEC(CFNWIDTH(1E-18,3),14)'
expressions[239]='CFNDEC(CFNWIDTH(1E-18,3),15)'
expressions[240]='CFNDEC(CFNWIDTH(1E-18,3),18)'
expressions[241]='CFNDEC(CFNWIDTH(1E-18,4),0)'
expressions[242]='CFNDEC(CFNWIDTH(1E-18,4),1)'
expressions[243]='CFNDEC(CFNWIDTH(1E-18,4),2)'
expressions[244]='CFNDEC(CFNWIDTH(1E-18,4),14)'
expressions[245]='CFNDEC(CFNWIDTH(1E-18,4),15)'
expressions[246]='CFNDEC(CFNWIDTH(1E-18,4),18)'
expressions[247]='CFNDEC(CFNWIDTH(1E-18,12),0)'
expressions[248]='CFNDEC(CFNWIDTH(1E-18,12),1)'
expressions[249]='CFNDEC(CFNWIDTH(1E-18,12),2)'
expressions[250]='CFNDEC(CFNWIDTH(1E-18,12),14)'
expressions[251]='CFNDEC(CFNWIDTH(1E-18,12),15)'
expressions[252]='CFNDEC(CFNWIDTH(1E-18,12),18)'
expressions[253]='CFNDEC(CFNWIDTH(1E-18,20),0)'
expressions[254]='CFNDEC(CFNWIDTH(1E-18,20),1)'
expressions[255]='CFNDEC(CFNWIDTH(1E-18,20),2)'
expressions[256]='CFNDEC(CFNWIDTH(1E-18,20),14)'
expressions[257]='CFNDEC(CFNWIDTH(1E-18,20),15)'
expressions[258]='CFNDEC(CFNWIDTH(1E-18,20),18)'
expressions[259]='CFNDEC(CFNWIDTH(1E-18,40),0)'
expressions[260]='CFNDEC(CFNWIDTH(1E-18,40),1)'
expressions[261]='CFNDEC(CFNWIDTH(1E-18,40),2)'
expressions[262]='CFNDEC(CFNWIDTH(1E-18,40),14)'
expressions[263]='CFNDEC(CFNWIDTH(1E-18,40),15)'
expressions[264]='CFNDEC(CFNWIDTH(1E-18,40),18)'
expressions[265]='-1E-18'
expressions[266]='CFNCLONE(-1E-18)'
expressions[267]='CFNDEC(CFNWIDTH(-1E-18,1),0)'
expressions[268]='CFNDEC(CFNWIDTH(-1E-18,1),1)'
expressions[269]='CFNDEC(CFNWIDTH(-1E-18,1),2)'
expressions[270]='CFNDEC(CFNWIDTH(-1E-18,1),14)'
expressions[271]='CFNDEC(CFNWIDTH(-1E-18,1),15)'
expressions[272]='CFNDEC(CFNWIDTH(-1E-18,1),18)'
expressions[273]='CFNDEC(CFNWIDTH(-1E-18,2),0)'
expressions[274]='CFNDEC(CFNWIDTH(-1E-18,2),1)'
expressions[275]='CFNDEC(CFNWIDTH(-1E-18,2),2)'
expressions[276]='CFNDEC(CFNWIDTH(-1E-18,2),14)'
expressions[277]='CFNDEC(CFNWIDTH(-1E-18,2),15)'
expressions[278]='CFNDEC(CFNWIDTH(-1E-18,2),18)'
expressions[279]='CFNDEC(CFNWIDTH(-1E-18,3),0)'
expressions[280]='CFNDEC(CFNWIDTH(-1E-18,3),1)'
expressions[281]='CFNDEC(CFNWIDTH(-1E-18,3),2)'
expressions[282]='CFNDEC(CFNWIDTH(-1E-18,3),14)'
expressions[283]='CFNDEC(CFNWIDTH(-1E-18,3),15)'
expressions[284]='CFNDEC(CFNWIDTH(-1E-18,3),18)'
expressions[285]='CFNDEC(CFNWIDTH(-1E-18,4),0)'
expressions[286]='CFNDEC(CFNWIDTH(-1E-18,4),1)'
expressions[287]='CFNDEC(CFNWIDTH(-1E-18,4),2)'
expressions[288]='CFNDEC(CFNWIDTH(-1E-18,4),14)'
expressions[289]='CFNDEC(CFNWIDTH(-1E-18,4),15)'
expressions[290]='CFNDEC(CFNWIDTH(-1E-18,4),18)'
expressions[291]='CFNDEC(CFNWIDTH(-1E-18,12),0)'
expressions[292]='CFNDEC(CFNWIDTH(-1E-18,12),1)'
expressions[293]='CFNDEC(CFNWIDTH(-1E-18,12),2)'
expressions[294]='CFNDEC(CFNWIDTH(-1E-18,12),14)'
expressions[295]='CFNDEC(CFNWIDTH(-1E-18,12),15)'
expressions[296]='CFNDEC(CFNWIDTH(-1E-18,12),18)'
expressions[297]='CFNDEC(CFNWIDTH(-1E-18,20),0)'
expressions[298]='CFNDEC(CFNWIDTH(-1E-18,20),1)'
expressions[299]='CFNDEC(CFNWIDTH(-1E-18,20),2)'
expressions[300]='CFNDEC(CFNWIDTH(-1E-18,20),14)'
expressions[301]='CFNDEC(CFNWIDTH(-1E-18,20),15)'
expressions[302]='CFNDEC(CFNWIDTH(-1E-18,20),18)'
expressions[303]='CFNDEC(CFNWIDTH(-1E-18,40),0)'
expressions[304]='CFNDEC(CFNWIDTH(-1E-18,40),1)'
expressions[305]='CFNDEC(CFNWIDTH(-1E-18,40),2)'
expressions[306]='CFNDEC(CFNWIDTH(-1E-18,40),14)'
expressions[307]='CFNDEC(CFNWIDTH(-1E-18,40),15)'
expressions[308]='CFNDEC(CFNWIDTH(-1E-18,40),18)'
expressions[309]='1.234567890123456789'
expressions[310]='CFNCLONE(1.234567890123456789)'
expressions[311]='CFNDEC(CFNWIDTH(1.234567890123456789,1),0)'
expressions[312]='CFNDEC(CFNWIDTH(1.234567890123456789,1),1)'
expressions[313]='CFNDEC(CFNWIDTH(1.234567890123456789,1),2)'
expressions[314]='CFNDEC(CFNWIDTH(1.234567890123456789,1),14)'
expressions[315]='CFNDEC(CFNWIDTH(1.234567890123456789,1),15)'
expressions[316]='CFNDEC(CFNWIDTH(1.234567890123456789,1),18)'
expressions[317]='CFNDEC(CFNWIDTH(1.234567890123456789,2),0)'
expressions[318]='CFNDEC(CFNWIDTH(1.234567890123456789,2),1)'
expressions[319]='CFNDEC(CFNWIDTH(1.234567890123456789,2),2)'
expressions[320]='CFNDEC(CFNWIDTH(1.234567890123456789,2),14)'
expressions[321]='CFNDEC(CFNWIDTH(1.234567890123456789,2),15)'
expressions[322]='CFNDEC(CFNWIDTH(1.234567890123456789,2),18)'
expressions[323]='CFNDEC(CFNWIDTH(1.234567890123456789,3),0)'
expressions[324]='CFNDEC(CFNWIDTH(1.234567890123456789,3),1)'
expressions[325]='CFNDEC(CFNWIDTH(1.234567890123456789,3),2)'
expressions[326]='CFNDEC(CFNWIDTH(1.234567890123456789,3),14)'
expressions[327]='CFNDEC(CFNWIDTH(1.234567890123456789,3),15)'
expressions[328]='CFNDEC(CFNWIDTH(1.234567890123456789,3),18)'
expressions[329]='CFNDEC(CFNWIDTH(1.234567890123456789,4),0)'
expressions[330]='CFNDEC(CFNWIDTH(1.234567890123456789,4),1)'
expressions[331]='CFNDEC(CFNWIDTH(1.234567890123456789,4),2)'
expressions[332]='CFNDEC(CFNWIDTH(1.234567890123456789,4),14)'
expressions[333]='CFNDEC(CFNWIDTH(1.234567890123456789,4),15)'
expressions[334]='CFNDEC(CFNWIDTH(1.234567890123456789,4),18)'
expressions[335]='CFNDEC(CFNWIDTH(1.234567890123456789,12),0)'
expressions[336]='CFNDEC(CFNWIDTH(1.234567890123456789,12),1)'
expressions[337]='CFNDEC(CFNWIDTH(1.234567890123456789,12),2)'
expressions[338]='CFNDEC(CFNWIDTH(1.234567890123456789,12),14)'
expressions[339]='CFNDEC(CFNWIDTH(1.234567890123456789,12),15)'
expressions[340]='CFNDEC(CFNWIDTH(1.234567890123456789,12),18)'
expressions[341]='CFNDEC(CFNWIDTH(1.234567890123456789,20),0)'
expressions[342]='CFNDEC(CFNWIDTH(1.234567890123456789,20),1)'
expressions[343]='CFNDEC(CFNWIDTH(1.234567890123456789,20),2)'
expressions[344]='CFNDEC(CFNWIDTH(1.234567890123456789,20),14)'
expressions[345]='CFNDEC(CFNWIDTH(1.234567890123456789,20),15)'
expressions[346]='CFNDEC(CFNWIDTH(1.234567890123456789,20),18)'
expressions[347]='CFNDEC(CFNWIDTH(1.234567890123456789,40),0)'
expressions[348]='CFNDEC(CFNWIDTH(1.234567890123456789,40),1)'
expressions[349]='CFNDEC(CFNWIDTH(1.234567890123456789,40),2)'
expressions[350]='CFNDEC(CFNWIDTH(1.234567890123456789,40),14)'
expressions[351]='CFNDEC(CFNWIDTH(1.234567890123456789,40),15)'
expressions[352]='CFNDEC(CFNWIDTH(1.234567890123456789,40),18)'
expressions[353]='0.5'
expressions[354]='-0.5'
expressions[355]='1.5'
expressions[356]='-1.5'
expressions[357]='2.5'
expressions[358]='-2.5'
expressions[359]='2.675'
expressions[360]='-2.675'
expressions[361]='1.005'
expressions[362]='-1.005'
expressions[363]='9.995'
expressions[364]='-9.995'
expressions[365]='0.499999999999999'
expressions[366]='0.500000000000001'
expressions[367]='1.2345678901234545'
expressions[368]='1.2345678901234555'
expressions[369]='1.2345678901234565'
expressions[370]='1.2345678901234575'
expressions[371]='1.2345678901234585'
expressions[372]='9.999999999999999'
expressions[373]='99.999999999999999'
expressions[374]='999999999.123456789'
expressions[375]='-999999999.123456789'
expressions[376]='12345678901234.5625'
expressions[377]='-12345678901234.5625'
expressions[378]='999999999999999.5'
expressions[379]='999999999999999.9'
expressions[380]='1000000000000000.5'
expressions[381]='0.00000000000000049'
expressions[382]='0.00000000000000050'
expressions[383]='0.00000000000000051'
expressions[384]='-0.00000000000000049'
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
