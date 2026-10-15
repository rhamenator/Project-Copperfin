* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Native SET DECIMALS before evaluation versus after evaluation controls.
* Variant is evaluation-decimals/read-decimals. Values and metadata captured unchanged.
LOCAL ARRAY operands[4], variants[10], ids[2]
operands[1]='1-1E-14'
operands[2]='1-90*2^(-53)'
operands[3]='2.5'
operands[4]='5/2'
variants[1]='0/0'
variants[2]='0/18'
variants[3]='2/2'
variants[4]='2/18'
variants[5]='6/6'
variants[6]='6/18'
variants[7]='14/14'
variants[8]='14/0'
variants[9]='18/18'
variants[10]='18/0'
ids[1]=11
ids[2]=22
LOCAL outputPath,libraryPath,classNo,idNo,argNo,variantNo,rowNo,colNo,total
LOCAL kind,target,operand,variant,ctrl,value,result,valueExpression,stage
LOCAL evalDecimals,readDecimals,settingCommand,slashAt,errorNo,errorMessage,returned
LOCAL originalMeta,argumentMeta,afterMeta,originalBits,argumentBits,afterBits
LOCAL before,after,contextBefore,contextAfter
outputPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\cause\settings-results.txt'
libraryPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\cause\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR classNo=1 TO 2
 kind=IIF(classNo=1,'ListBox','ComboBox')
 FOR idNo=1 TO ALEN(ids)
  target=ids[idNo]
  FOR argNo=1 TO ALEN(operands)
   operand=operands[argNo]
   FOR variantNo=1 TO ALEN(variants)
    variant=variants[variantNo]
    stage='setup'
    errorNo=0
    errorMessage=''
    returned='<not-called>'
    originalMeta='<not-evaluated>'
    argumentMeta='<not-evaluated>'
    afterMeta='<not-evaluated>'
    originalBits='<not-evaluated>'
    argumentBits='<not-evaluated>'
    afterBits='<not-evaluated>'
    before='<not-captured>'
    after='<not-captured>'
    contextBefore='<not-captured>'
    contextAfter='<not-captured>'
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
     before=Capture(ctrl,kind)
     contextBefore=TRANSFORM(SELECT())+';'+ALIAS()+';'+TRANSFORM(SET('DATASESSION'))
     stage='evaluate'
     slashAt=AT('/',variant)
     evalDecimals=VAL(LEFT(variant,slashAt-1))
     readDecimals=VAL(SUBSTR(variant,slashAt+1))
     settingCommand='SET DECIMALS TO '+TRANSFORM(evalDecimals)
     &settingCommand
     value=EVALUATE(operand)
     originalMeta=CFNMETA(value)
     originalBits=NumericBits(value)
     stage='intervene'
     settingCommand='SET DECIMALS TO '+TRANSFORM(readDecimals)
     &settingCommand
     argumentMeta=CFNMETA(value)
     argumentBits=NumericBits(value)
     stage='invoke'
     result=ctrl.ListItem(target,value)
     returned=VARTYPE(result)+':'+TRANSFORM(result)
    CATCH TO probeError
     errorNo=probeError.ErrorNo
     errorMessage=STRTRAN(probeError.Message,'|','/')
    ENDTRY
    IF VARTYPE(ctrl)='O'
     IF stage='invoke'
      afterMeta=CFNMETA(value)
      afterBits=NumericBits(value)
     ENDIF
     after=Capture(ctrl,kind)
     contextAfter=TRANSFORM(SELECT())+';'+ALIAS()+';'+TRANSFORM(SET('DATASESSION'))
    ENDIF
    =STRTOFILE('CAUSE|'+kind+'|'+TRANSFORM(target)+'|'+operand+'|'+variant+'|'+originalMeta+'|'+argumentMeta+'|'+originalBits+'|'+argumentBits+'|'+stage+'|'+TRANSFORM(errorNo)+'|'+errorMessage+'|'+returned+'|'+afterMeta+'|'+afterBits+'|'+contextBefore+'|'+contextAfter+'|'+before+'|'+after+CHR(13)+CHR(10),outputPath,1)
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
 ENDFOR
 RETURN captured
ENDFUNC
