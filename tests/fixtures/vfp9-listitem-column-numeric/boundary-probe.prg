* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Independent supplemental near-integer/signed-width COLUMN READ observations, not a conversion expected table.
LOCAL ARRAY operands[47], ids[2]
operands[1]='1-2^(-52)'
operands[2]='1-2^(-51)'
operands[3]='1-2^(-50)'
operands[4]='1-1E-14'
operands[5]='1-1E-12'
operands[6]='1-1E-10'
operands[7]='1-1E-8'
operands[8]='2-2^(-52)'
operands[9]='2-2^(-51)'
operands[10]='2-1E-14'
operands[11]='2-1E-12'
operands[12]='2-1E-10'
operands[13]='9-2^(-49)'
operands[14]='9-1E-12'
operands[15]='2147483646'
operands[16]='2147483646.5'
operands[17]='2147483646.9'
operands[18]='2147483647'
operands[19]='2147483647.1'
operands[20]='2147483647.49'
operands[21]='2147483647.5'
operands[22]='2147483647.9'
operands[23]='1073741823.9'
operands[24]='1073741824.9'
operands[25]='4294967295'
operands[26]='4294967296'
operands[27]='4294967297'
operands[28]='4294967298'
operands[29]='-2147483647'
operands[30]='-2147483647.9'
operands[31]='-2147483648.1'
operands[32]='-2147483648.5'
operands[33]='-2147483649'
operands[34]='-4294967297'
operands[35]='-4294967297.9'
operands[36]='-4294967298'
operands[37]='-4294967294.1'
operands[38]='-4294967295.1'
operands[39]='-8589934591'
operands[40]='-8589934590'
operands[41]='-8589934593'
operands[42]='1E20'
operands[43]='-1E20'
operands[44]='1E100'
operands[45]='-1E100'
operands[46]='1E300'
operands[47]='-1E300'
ids[1]=11
ids[2]=22
LOCAL classNo,idNo,argNo,rowNo,colNo,ctrl,kind,target,operand,value,result
LOCAL stage,errorNo,errorMessage,typeBefore,typeAfter,bitsBefore,bitsAfter
LOCAL before,after,returned,contextBefore,contextAfter,foundRow,total,outputPath
outputPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\boundary\native-results.txt'
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
