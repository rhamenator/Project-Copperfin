* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Direct native descriptor factorial: exact variant comparisons, no intermediate assignment.
* After-meta is the unchanged ORIGINAL variable, not the temporary call argument.
LOCAL ARRAY operands[11], variants[17], ids[2]
operands[1]='1-1E-14'
operands[2]='1-90*2^(-53)'
operands[3]='2-1E-14'
operands[4]='2-45*2^(-52)'
operands[5]='9-1E-12'
operands[6]='9-563*2^(-49)'
operands[7]='0.9'
operands[8]='1.49'
operands[9]='1.5'
operands[10]='1.9'
operands[11]='2.9'
variants[1]='raw'
variants[2]='clone'
variants[3]='width25'
variants[4]='width30'
variants[5]='dec0'
variants[6]='dec1'
variants[7]='dec2'
variants[8]='dec8'
variants[9]='dec12'
variants[10]='dec13'
variants[11]='dec14'
variants[12]='dec15'
variants[13]='dec18'
variants[14]='d2w25'
variants[15]='d14w25'
variants[16]='d2w30'
variants[17]='d14w30'
ids[1]=11
ids[2]=22
LOCAL outputPath,libraryPath,classNo,idNo,argNo,variantNo,rowNo,colNo,total
LOCAL kind,target,operand,variant,ctrl,value,result,valueExpression,stage,errorNo,errorMessage,returned
LOCAL originalMeta,argumentMeta,afterMeta,originalBits,argumentBits,afterBits
LOCAL before,after,contextBefore,contextAfter
outputPath='Z:\home\rich\.codex\worktrees\listitem-column-numeric-5611\Project-Copperfin\build\listitem-column-native\cause\factorial-results.txt'
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
     value=EVALUATE(operand)
     originalMeta=CFNMETA(value)
     originalBits=NumericBits(value)
     stage='intervene'
     valueExpression='value'
     DO CASE
      CASE variant=='clone'
       valueExpression='CFNCLONE(value)'
      CASE variant=='width25'
       valueExpression='CFNWIDTH(value,25)'
      CASE variant=='width30'
       valueExpression='CFNWIDTH(value,30)'
      CASE variant=='dec0'
       valueExpression='CFNDEC(value,0)'
      CASE variant=='dec1'
       valueExpression='CFNDEC(value,1)'
      CASE variant=='dec2'
       valueExpression='CFNDEC(value,2)'
      CASE variant=='dec8'
       valueExpression='CFNDEC(value,8)'
      CASE variant=='dec12'
       valueExpression='CFNDEC(value,12)'
      CASE variant=='dec13'
       valueExpression='CFNDEC(value,13)'
      CASE variant=='dec14'
       valueExpression='CFNDEC(value,14)'
      CASE variant=='dec15'
       valueExpression='CFNDEC(value,15)'
      CASE variant=='dec18'
       valueExpression='CFNDEC(value,18)'
      CASE variant=='d2w25'
       valueExpression='CFNWIDTH(CFNDEC(value,2),25)'
      CASE variant=='d14w25'
       valueExpression='CFNWIDTH(CFNDEC(value,14),25)'
      CASE variant=='d2w30'
       valueExpression='CFNWIDTH(CFNDEC(value,2),30)'
      CASE variant=='d14w30'
       valueExpression='CFNWIDTH(CFNDEC(value,14),30)'
     ENDCASE
     argumentMeta=EVALUATE('CFNMETA('+valueExpression+')')
     argumentBits=EVALUATE('NumericBits('+valueExpression+')')
     stage='invoke'
     result=EVALUATE('ctrl.ListItem(target,'+valueExpression+')')
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
