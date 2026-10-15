* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Independent adjacent-value threshold COLUMN READ observations, not a conversion expected table.
LOCAL ARRAY operands[153], ids[2]
operands[1]='1-21*2^(-53)'
operands[2]='1-24*2^(-53)'
operands[3]='1-28*2^(-53)'
operands[4]='1-30*2^(-53)'
operands[5]='1-31*2^(-53)'
operands[6]='1-32*2^(-53)'
operands[7]='1-33*2^(-53)'
operands[8]='1-34*2^(-53)'
operands[9]='1-35*2^(-53)'
operands[10]='1-36*2^(-53)'
operands[11]='1-40*2^(-53)'
operands[12]='1-48*2^(-53)'
operands[13]='1-56*2^(-53)'
operands[14]='1-64*2^(-53)'
operands[15]='1-80*2^(-53)'
operands[16]='1-96*2^(-53)'
operands[17]='1-128*2^(-53)'
operands[18]='2-21*2^(-52)'
operands[19]='2-24*2^(-52)'
operands[20]='2-28*2^(-52)'
operands[21]='2-30*2^(-52)'
operands[22]='2-31*2^(-52)'
operands[23]='2-32*2^(-52)'
operands[24]='2-33*2^(-52)'
operands[25]='2-34*2^(-52)'
operands[26]='2-35*2^(-52)'
operands[27]='2-36*2^(-52)'
operands[28]='2-40*2^(-52)'
operands[29]='2-48*2^(-52)'
operands[30]='2-56*2^(-52)'
operands[31]='2-64*2^(-52)'
operands[32]='2-80*2^(-52)'
operands[33]='2-96*2^(-52)'
operands[34]='2-128*2^(-52)'
operands[35]='3-21*2^(-51)'
operands[36]='3-24*2^(-51)'
operands[37]='3-28*2^(-51)'
operands[38]='3-30*2^(-51)'
operands[39]='3-31*2^(-51)'
operands[40]='3-32*2^(-51)'
operands[41]='3-33*2^(-51)'
operands[42]='3-34*2^(-51)'
operands[43]='3-35*2^(-51)'
operands[44]='3-36*2^(-51)'
operands[45]='3-40*2^(-51)'
operands[46]='3-48*2^(-51)'
operands[47]='3-56*2^(-51)'
operands[48]='3-64*2^(-51)'
operands[49]='3-80*2^(-51)'
operands[50]='3-96*2^(-51)'
operands[51]='3-128*2^(-51)'
operands[52]='4-21*2^(-51)'
operands[53]='4-24*2^(-51)'
operands[54]='4-28*2^(-51)'
operands[55]='4-30*2^(-51)'
operands[56]='4-31*2^(-51)'
operands[57]='4-32*2^(-51)'
operands[58]='4-33*2^(-51)'
operands[59]='4-34*2^(-51)'
operands[60]='4-35*2^(-51)'
operands[61]='4-36*2^(-51)'
operands[62]='4-40*2^(-51)'
operands[63]='4-48*2^(-51)'
operands[64]='4-56*2^(-51)'
operands[65]='4-64*2^(-51)'
operands[66]='4-80*2^(-51)'
operands[67]='4-96*2^(-51)'
operands[68]='4-128*2^(-51)'
operands[69]='5-21*2^(-50)'
operands[70]='5-24*2^(-50)'
operands[71]='5-28*2^(-50)'
operands[72]='5-30*2^(-50)'
operands[73]='5-31*2^(-50)'
operands[74]='5-32*2^(-50)'
operands[75]='5-33*2^(-50)'
operands[76]='5-34*2^(-50)'
operands[77]='5-35*2^(-50)'
operands[78]='5-36*2^(-50)'
operands[79]='5-40*2^(-50)'
operands[80]='5-48*2^(-50)'
operands[81]='5-56*2^(-50)'
operands[82]='5-64*2^(-50)'
operands[83]='5-80*2^(-50)'
operands[84]='5-96*2^(-50)'
operands[85]='5-128*2^(-50)'
operands[86]='6-21*2^(-50)'
operands[87]='6-24*2^(-50)'
operands[88]='6-28*2^(-50)'
operands[89]='6-30*2^(-50)'
operands[90]='6-31*2^(-50)'
operands[91]='6-32*2^(-50)'
operands[92]='6-33*2^(-50)'
operands[93]='6-34*2^(-50)'
operands[94]='6-35*2^(-50)'
operands[95]='6-36*2^(-50)'
operands[96]='6-40*2^(-50)'
operands[97]='6-48*2^(-50)'
operands[98]='6-56*2^(-50)'
operands[99]='6-64*2^(-50)'
operands[100]='6-80*2^(-50)'
operands[101]='6-96*2^(-50)'
operands[102]='6-128*2^(-50)'
operands[103]='7-21*2^(-50)'
operands[104]='7-24*2^(-50)'
operands[105]='7-28*2^(-50)'
operands[106]='7-30*2^(-50)'
operands[107]='7-31*2^(-50)'
operands[108]='7-32*2^(-50)'
operands[109]='7-33*2^(-50)'
operands[110]='7-34*2^(-50)'
operands[111]='7-35*2^(-50)'
operands[112]='7-36*2^(-50)'
operands[113]='7-40*2^(-50)'
operands[114]='7-48*2^(-50)'
operands[115]='7-56*2^(-50)'
operands[116]='7-64*2^(-50)'
operands[117]='7-80*2^(-50)'
operands[118]='7-96*2^(-50)'
operands[119]='7-128*2^(-50)'
operands[120]='8-21*2^(-50)'
operands[121]='8-24*2^(-50)'
operands[122]='8-28*2^(-50)'
operands[123]='8-30*2^(-50)'
operands[124]='8-31*2^(-50)'
operands[125]='8-32*2^(-50)'
operands[126]='8-33*2^(-50)'
operands[127]='8-34*2^(-50)'
operands[128]='8-35*2^(-50)'
operands[129]='8-36*2^(-50)'
operands[130]='8-40*2^(-50)'
operands[131]='8-48*2^(-50)'
operands[132]='8-56*2^(-50)'
operands[133]='8-64*2^(-50)'
operands[134]='8-80*2^(-50)'
operands[135]='8-96*2^(-50)'
operands[136]='8-128*2^(-50)'
operands[137]='9-21*2^(-49)'
operands[138]='9-24*2^(-49)'
operands[139]='9-28*2^(-49)'
operands[140]='9-30*2^(-49)'
operands[141]='9-31*2^(-49)'
operands[142]='9-32*2^(-49)'
operands[143]='9-33*2^(-49)'
operands[144]='9-34*2^(-49)'
operands[145]='9-35*2^(-49)'
operands[146]='9-36*2^(-49)'
operands[147]='9-40*2^(-49)'
operands[148]='9-48*2^(-49)'
operands[149]='9-56*2^(-49)'
operands[150]='9-64*2^(-49)'
operands[151]='9-80*2^(-49)'
operands[152]='9-96*2^(-49)'
operands[153]='9-128*2^(-49)'
ids[1]=11
ids[2]=22
LOCAL classNo,idNo,argNo,rowNo,colNo,ctrl,kind,target,operand,value,result
LOCAL stage,errorNo,errorMessage,typeBefore,typeAfter,bitsBefore,bitsAfter
LOCAL before,after,returned,contextBefore,contextAfter,foundRow,total,outputPath
outputPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\precision-threshold\native-results.txt'
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
