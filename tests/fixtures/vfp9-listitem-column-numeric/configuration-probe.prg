* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Independent configuration/visible-column COLUMN READ observations, not a conversion expected table.
LOCAL ARRAY operands[15], ids[2]
operands[1]='0'
operands[2]='0.99'
operands[3]='1-2^(-53)'
operands[4]='1'
operands[5]='1.9'
operands[6]='2'
operands[7]='9'
operands[8]='9.9'
operands[9]='10'
operands[10]='32'
operands[11]='33'
operands[12]='255'
operands[13]='256'
operands[14]='257'
operands[15]='65537'
ids[1]=11
ids[2]=22
LOCAL classNo,idNo,argNo,rowNo,colNo,ctrl,kind,target,operand,value,result
LOCAL configNo,configName,seedColumns
LOCAL stage,errorNo,errorMessage,typeBefore,typeAfter,bitsBefore,bitsAfter
LOCAL before,after,returned,contextBefore,contextAfter,foundRow,total,outputPath
outputPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\configuration\native-results.txt'
total=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
FOR classNo=1 TO 2
 kind=IIF(classNo=1,'ListBox','ComboBox')
 FOR idNo=1 TO ALEN(ids)
  target=ids[idNo]
  FOR argNo=1 TO ALEN(operands)
   FOR configNo=1 TO 6
   configName=TRANSFORM(configNo)
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
    IF configNo>1
     ctrl.RowSourceType=0
     ctrl.ControlSource=IIF(configNo=3,'   ','')
    ENDIF
    seedColumns=IIF(configNo=4 OR configNo=5,1,IIF(configNo=6,256,9))
    ctrl.ColumnCount=IIF(configNo=4,0,seedColumns)
    FOR rowNo=1 TO 3
     ctrl.AddListItem('id'+TRANSFORM(rowNo*11)+'c1',rowNo*11)
     FOR colNo=2 TO seedColumns
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
   =STRTOFILE('READ|'+kind+'|'+TRANSFORM(target)+'|'+TRANSFORM(foundRow)+'|'+configName+':'+operand+'|'+typeBefore+'|'+bitsBefore+'|'+typeAfter+'|'+bitsAfter+'|'+stage+'|'+TRANSFORM(errorNo)+'|'+errorMessage+'|'+returned+'|'+contextBefore+'|'+contextAfter+'|'+before+'|'+after+CHR(13)+CHR(10),outputPath,1)
   total=total+1
   ctrl=.NULL.
   ENDFOR
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
  captured=captured+',c255=['+TRANSFORM(o.List(row,255))+'],c256=['+TRANSFORM(o.List(row,256))+']'
 ENDFOR
 RETURN captured
ENDFUNC
