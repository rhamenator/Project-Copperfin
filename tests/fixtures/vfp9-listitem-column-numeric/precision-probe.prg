* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Independent adjacent-binary-value explicit COLUMN READ observations, not a conversion expected table.
LOCAL ARRAY operands[180], ids[2]
operands[1]='1-1*2^(-53)'
operands[2]='1-2*2^(-53)'
operands[3]='1-3*2^(-53)'
operands[4]='1-4*2^(-53)'
operands[5]='1-5*2^(-53)'
operands[6]='1-6*2^(-53)'
operands[7]='1-7*2^(-53)'
operands[8]='1-8*2^(-53)'
operands[9]='1-9*2^(-53)'
operands[10]='1-10*2^(-53)'
operands[11]='1-11*2^(-53)'
operands[12]='1-12*2^(-53)'
operands[13]='1-13*2^(-53)'
operands[14]='1-14*2^(-53)'
operands[15]='1-15*2^(-53)'
operands[16]='1-16*2^(-53)'
operands[17]='1-17*2^(-53)'
operands[18]='1-18*2^(-53)'
operands[19]='1-19*2^(-53)'
operands[20]='1-20*2^(-53)'
operands[21]='2-1*2^(-52)'
operands[22]='2-2*2^(-52)'
operands[23]='2-3*2^(-52)'
operands[24]='2-4*2^(-52)'
operands[25]='2-5*2^(-52)'
operands[26]='2-6*2^(-52)'
operands[27]='2-7*2^(-52)'
operands[28]='2-8*2^(-52)'
operands[29]='2-9*2^(-52)'
operands[30]='2-10*2^(-52)'
operands[31]='2-11*2^(-52)'
operands[32]='2-12*2^(-52)'
operands[33]='2-13*2^(-52)'
operands[34]='2-14*2^(-52)'
operands[35]='2-15*2^(-52)'
operands[36]='2-16*2^(-52)'
operands[37]='2-17*2^(-52)'
operands[38]='2-18*2^(-52)'
operands[39]='2-19*2^(-52)'
operands[40]='2-20*2^(-52)'
operands[41]='3-1*2^(-51)'
operands[42]='3-2*2^(-51)'
operands[43]='3-3*2^(-51)'
operands[44]='3-4*2^(-51)'
operands[45]='3-5*2^(-51)'
operands[46]='3-6*2^(-51)'
operands[47]='3-7*2^(-51)'
operands[48]='3-8*2^(-51)'
operands[49]='3-9*2^(-51)'
operands[50]='3-10*2^(-51)'
operands[51]='3-11*2^(-51)'
operands[52]='3-12*2^(-51)'
operands[53]='3-13*2^(-51)'
operands[54]='3-14*2^(-51)'
operands[55]='3-15*2^(-51)'
operands[56]='3-16*2^(-51)'
operands[57]='3-17*2^(-51)'
operands[58]='3-18*2^(-51)'
operands[59]='3-19*2^(-51)'
operands[60]='3-20*2^(-51)'
operands[61]='4-1*2^(-51)'
operands[62]='4-2*2^(-51)'
operands[63]='4-3*2^(-51)'
operands[64]='4-4*2^(-51)'
operands[65]='4-5*2^(-51)'
operands[66]='4-6*2^(-51)'
operands[67]='4-7*2^(-51)'
operands[68]='4-8*2^(-51)'
operands[69]='4-9*2^(-51)'
operands[70]='4-10*2^(-51)'
operands[71]='4-11*2^(-51)'
operands[72]='4-12*2^(-51)'
operands[73]='4-13*2^(-51)'
operands[74]='4-14*2^(-51)'
operands[75]='4-15*2^(-51)'
operands[76]='4-16*2^(-51)'
operands[77]='4-17*2^(-51)'
operands[78]='4-18*2^(-51)'
operands[79]='4-19*2^(-51)'
operands[80]='4-20*2^(-51)'
operands[81]='5-1*2^(-50)'
operands[82]='5-2*2^(-50)'
operands[83]='5-3*2^(-50)'
operands[84]='5-4*2^(-50)'
operands[85]='5-5*2^(-50)'
operands[86]='5-6*2^(-50)'
operands[87]='5-7*2^(-50)'
operands[88]='5-8*2^(-50)'
operands[89]='5-9*2^(-50)'
operands[90]='5-10*2^(-50)'
operands[91]='5-11*2^(-50)'
operands[92]='5-12*2^(-50)'
operands[93]='5-13*2^(-50)'
operands[94]='5-14*2^(-50)'
operands[95]='5-15*2^(-50)'
operands[96]='5-16*2^(-50)'
operands[97]='5-17*2^(-50)'
operands[98]='5-18*2^(-50)'
operands[99]='5-19*2^(-50)'
operands[100]='5-20*2^(-50)'
operands[101]='6-1*2^(-50)'
operands[102]='6-2*2^(-50)'
operands[103]='6-3*2^(-50)'
operands[104]='6-4*2^(-50)'
operands[105]='6-5*2^(-50)'
operands[106]='6-6*2^(-50)'
operands[107]='6-7*2^(-50)'
operands[108]='6-8*2^(-50)'
operands[109]='6-9*2^(-50)'
operands[110]='6-10*2^(-50)'
operands[111]='6-11*2^(-50)'
operands[112]='6-12*2^(-50)'
operands[113]='6-13*2^(-50)'
operands[114]='6-14*2^(-50)'
operands[115]='6-15*2^(-50)'
operands[116]='6-16*2^(-50)'
operands[117]='6-17*2^(-50)'
operands[118]='6-18*2^(-50)'
operands[119]='6-19*2^(-50)'
operands[120]='6-20*2^(-50)'
operands[121]='7-1*2^(-50)'
operands[122]='7-2*2^(-50)'
operands[123]='7-3*2^(-50)'
operands[124]='7-4*2^(-50)'
operands[125]='7-5*2^(-50)'
operands[126]='7-6*2^(-50)'
operands[127]='7-7*2^(-50)'
operands[128]='7-8*2^(-50)'
operands[129]='7-9*2^(-50)'
operands[130]='7-10*2^(-50)'
operands[131]='7-11*2^(-50)'
operands[132]='7-12*2^(-50)'
operands[133]='7-13*2^(-50)'
operands[134]='7-14*2^(-50)'
operands[135]='7-15*2^(-50)'
operands[136]='7-16*2^(-50)'
operands[137]='7-17*2^(-50)'
operands[138]='7-18*2^(-50)'
operands[139]='7-19*2^(-50)'
operands[140]='7-20*2^(-50)'
operands[141]='8-1*2^(-50)'
operands[142]='8-2*2^(-50)'
operands[143]='8-3*2^(-50)'
operands[144]='8-4*2^(-50)'
operands[145]='8-5*2^(-50)'
operands[146]='8-6*2^(-50)'
operands[147]='8-7*2^(-50)'
operands[148]='8-8*2^(-50)'
operands[149]='8-9*2^(-50)'
operands[150]='8-10*2^(-50)'
operands[151]='8-11*2^(-50)'
operands[152]='8-12*2^(-50)'
operands[153]='8-13*2^(-50)'
operands[154]='8-14*2^(-50)'
operands[155]='8-15*2^(-50)'
operands[156]='8-16*2^(-50)'
operands[157]='8-17*2^(-50)'
operands[158]='8-18*2^(-50)'
operands[159]='8-19*2^(-50)'
operands[160]='8-20*2^(-50)'
operands[161]='9-1*2^(-49)'
operands[162]='9-2*2^(-49)'
operands[163]='9-3*2^(-49)'
operands[164]='9-4*2^(-49)'
operands[165]='9-5*2^(-49)'
operands[166]='9-6*2^(-49)'
operands[167]='9-7*2^(-49)'
operands[168]='9-8*2^(-49)'
operands[169]='9-9*2^(-49)'
operands[170]='9-10*2^(-49)'
operands[171]='9-11*2^(-49)'
operands[172]='9-12*2^(-49)'
operands[173]='9-13*2^(-49)'
operands[174]='9-14*2^(-49)'
operands[175]='9-15*2^(-49)'
operands[176]='9-16*2^(-49)'
operands[177]='9-17*2^(-49)'
operands[178]='9-18*2^(-49)'
operands[179]='9-19*2^(-49)'
operands[180]='9-20*2^(-49)'
ids[1]=11
ids[2]=22
LOCAL classNo,idNo,argNo,rowNo,colNo,ctrl,kind,target,operand,value,result
LOCAL stage,errorNo,errorMessage,typeBefore,typeAfter,bitsBefore,bitsAfter
LOCAL before,after,returned,contextBefore,contextAfter,foundRow,total,outputPath
outputPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\precision\native-results.txt'
total=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
FOR classNo=1 TO 2
 kind=IIF(classNo=1,'ListBox','ComboBox')
 FOR idNo=1 TO ALEN(ids)
  target=ids[idNo]
  FOR argNo=1 TO ALEN(operands)
   operand=operands[argNo]
   stage='setup'
   errorNo=0
   errorMessage=''
   returned='<not-called>'
   before='<not-captured>'
   after='<not-captured>'
   contextBefore='<not-captured>'
   contextAfter='<not-captured>'
   typeBefore='<not-evaluated>'
   typeAfter='<not-evaluated>'
   bitsBefore='<not-evaluated>'
   bitsAfter='<not-evaluated>'
   foundRow=0
   ctrl=.NULL.
   TRY
    ctrl=CREATEOBJECT(kind)
    ctrl.RowSourceType=0
    ctrl.ControlSource=''
    ctrl.ColumnCount=9
    FOR rowNo=1 TO 3
     ctrl.AddListItem('id'+TRANSFORM(rowNo*11)+'c1',rowNo*11)
     FOR colNo=2 TO 9
      ctrl.List(rowNo,colNo)='id'+TRANSFORM(rowNo*11)+'c'+TRANSFORM(colNo)
     ENDFOR
     ctrl.ItemData(rowNo)=rowNo*101
    ENDFOR
    ctrl.ListIndex=2
    IF kind='ListBox'
     ctrl.TopIndex=2
    ENDIF
    foundRow=ctrl.ItemIDToIndex(target)
    before=Capture(ctrl,kind)
    contextBefore=TRANSFORM(SELECT())+';'+ALIAS()+';'+TRANSFORM(SET('DATASESSION'))
    stage='evaluate'
    value=EVALUATE(operand)
    typeBefore=VARTYPE(value)
    bitsBefore=NumericBits(value)
    stage='invoke'
    result=ctrl.ListItem(target,value)
    returned=VARTYPE(result)+':'+TRANSFORM(result)
   CATCH TO probeError
    errorNo=probeError.ErrorNo
    errorMessage=STRTRAN(probeError.Message,'|','/')
   ENDTRY
   IF VARTYPE(ctrl)='O'
    IF stage='invoke'
     typeAfter=VARTYPE(value)
     bitsAfter=NumericBits(value)
    ENDIF
    after=Capture(ctrl,kind)
    contextAfter=TRANSFORM(SELECT())+';'+ALIAS()+';'+TRANSFORM(SET('DATASESSION'))
   ENDIF
   =STRTOFILE('READ|'+kind+'|'+TRANSFORM(target)+'|'+TRANSFORM(foundRow)+'|'+operand+'|'+typeBefore+'|'+bitsBefore+'|'+typeAfter+'|'+bitsAfter+'|'+stage+'|'+TRANSFORM(errorNo)+'|'+errorMessage+'|'+returned+'|'+contextBefore+'|'+contextAfter+'|'+before+'|'+after+CHR(13)+CHR(10),outputPath,1)
   total=total+1
   ctrl=.NULL.
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+TRANSFORM(total)+CHR(13)+CHR(10),outputPath,1)
RETURN

FUNCTION NumericBits(v)
 LOCAL bytes,hex,indexNo
 bytes=BINTOC(v,'BRS')
 hex=''
 FOR indexNo=1 TO LEN(bytes)
  hex=hex+RIGHT(TRANSFORM(ASC(SUBSTR(bytes,indexNo,1)),'@0'),2)
 ENDFOR
 RETURN hex
ENDFUNC

FUNCTION Capture(o,kind)
 LOCAL captured,row,col,top
 top='<not-applicable>'
 IF kind='ListBox'
  top=TRANSFORM(o.TopIndex)+':'+TRANSFORM(o.TopItemID)
 ENDIF
 captured='count='+TRANSFORM(o.ListCount)+';columns='+TRANSFORM(o.ColumnCount)+';rst='+VARTYPE(o.RowSourceType)+':'+TRANSFORM(o.RowSourceType)+';binding='+VARTYPE(o.ControlSource)+':['+o.ControlSource+'];newid='+TRANSFORM(o.NewItemID)+';newindex='+TRANSFORM(o.NewIndex)+';index='+TRANSFORM(o.ListIndex)+';itemid='+TRANSFORM(o.ListItemID)+';display='+TRANSFORM(o.DisplayValue)+';value='+TRANSFORM(o.Value)+';top='+top
 FOR row=1 TO o.ListCount
  captured=captured+';row='+TRANSFORM(row)+',id='+TRANSFORM(o.IndexToItemID(row))+',data='+TRANSFORM(o.ItemData(row))+',selected='+TRANSFORM(o.Selected(row))
  FOR col=1 TO 9
   captured=captured+',c'+TRANSFORM(col)+'=['+TRANSFORM(o.List(row,col))+']'
  ENDFOR
 ENDFOR
 RETURN captured
ENDFUNC
