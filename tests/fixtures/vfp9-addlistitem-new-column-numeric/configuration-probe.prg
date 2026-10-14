* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Independently observed supported configuration contexts set BEFORE population.
LOCAL ARRAY args[24]
args[1]='-0.9'
args[2]='0'
args[3]='0.99'
args[4]='1-2^(-53)'
args[5]='1'
args[6]='1.5'
args[7]='2.9'
args[8]='9'
args[9]='32745.9'
args[10]='32746'
args[11]='65535.9'
args[12]='65536.9'
args[13]='65537.9'
args[14]='65538.9'
args[15]='2147483647.9'
args[16]='2147483648.9'
args[17]='2147483649'
args[18]='4294967297.9'
args[19]='4294967298.9'
args[20]='-2147483648.9'
args[21]='-4294967295.9'
args[22]='6442450944'
args[23]='1E300'
args[24]='(1E300*1E300)'
LOCAL classNo,configNo,shapeNo,argNo,rowNo,colNo,target,ctrl,className,operand
LOCAL value,bitsBefore,bitsAfter,typeBefore,typeAfter,stage,errorNo,errorMessage
LOCAL before,after,expanded,result,returnCapture,total
LOCAL outputPath
outputPath='Z:\home\rich\.codex\worktrees\addlistitem-new-column-numeric-5611\Project-Copperfin\build\addlistitem-new-column-native\configuration-results.txt'
total=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
FOR classNo=1 TO 2
 className=IIF(classNo=1,'ListBox','ComboBox')
 FOR configNo=1 TO 4
  FOR shapeNo=0 TO 1
   target=IIF(shapeNo=0,11,44)
   FOR argNo=1 TO ALEN(args)
    operand=args[argNo]
    stage='setup'
    errorNo=0
    errorMessage=''
    returnCapture='<not-called>'
    before='<not-captured>'
    after='<not-captured>'
    expanded='<not-captured>'
    typeBefore='<not-evaluated>'
    typeAfter='<not-evaluated>'
    bitsBefore='<not-evaluated>'
    bitsAfter='<not-evaluated>'
    ctrl=.NULL.
    TRY
     ctrl=CREATEOBJECT(className)
     ctrl.ColumnCount=9
     DO CASE
      CASE configNo=1
       * Native defaults, no explicit binding/RST assignment.
      CASE configNo=2
       ctrl.RowSourceType=0
       ctrl.ControlSource=''
      CASE configNo=3
       ctrl.RowSourceType=0
       ctrl.ControlSource='  '
      CASE configNo=4
       ctrl.RowSourceType=0
       ctrl.ControlSource=''
       ctrl.Sorted=.T.
     ENDCASE
     IF shapeNo=1
      ctrl.AddListItem('first',11)
      ctrl.AddListItem('middle',22)
      ctrl.AddListItem('last',33)
      FOR rowNo=1 TO ctrl.ListCount
       FOR colNo=2 TO MAX(1,ctrl.ColumnCount)
        ctrl.List(rowNo,colNo)='base'+TRANSFORM(rowNo)+':'+TRANSFORM(colNo)
       ENDFOR
       ctrl.ItemData(rowNo)=rowNo*101
      ENDFOR
      ctrl.ListIndex=2
      IF className='ListBox'
       ctrl.TopIndex=2
      ENDIF
     ENDIF
     before=Capture(ctrl,className)
     stage='evaluate'
     value=EVALUATE(operand)
     typeBefore=VARTYPE(value)
     bitsBefore=NumericBits(value)
     stage='invoke'
     result=ctrl.AddListItem('candidate',target,value)
     returnCapture=VARTYPE(result)+':'+TRANSFORM(result)
    CATCH TO probeError
     errorNo=probeError.ErrorNo
     errorMessage=STRTRAN(probeError.Message,'|','/')
    ENDTRY
    IF VARTYPE(ctrl)='O'
     IF stage='invoke'
      typeAfter=VARTYPE(value)
      bitsAfter=NumericBits(value)
     ENDIF
     after=Capture(ctrl,className)
     TRY
      ctrl.ColumnCount=9
      expanded=Capture(ctrl,className)
     CATCH TO expandError
      expanded='EXPANDERROR:'+TRANSFORM(expandError.ErrorNo)
     ENDTRY
    ENDIF
    =STRTOFILE('CASE|'+className+'|'+TRANSFORM(configNo)+'|'+TRANSFORM(shapeNo)+'|'+TRANSFORM(target)+'|'+operand+'|'+typeBefore+'|'+bitsBefore+'|'+typeAfter+'|'+bitsAfter+'|'+stage+'|'+TRANSFORM(errorNo)+'|'+errorMessage+'|'+returnCapture+'|'+before+'|'+after+'|'+expanded+CHR(13)+CHR(10),outputPath,1)
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
  top=TRANSFORM(o.TopIndex)
 ENDIF
 captured='count='+TRANSFORM(o.ListCount)+';newid='+TRANSFORM(o.NewItemID)+';newindex='+TRANSFORM(o.NewIndex)+';index='+TRANSFORM(o.ListIndex)+';itemid='+TRANSFORM(o.ListItemID)+';display='+TRANSFORM(o.DisplayValue)+';value='+TRANSFORM(o.Value)+';top='+top+';columns='+TRANSFORM(o.ColumnCount)
 FOR row=1 TO o.ListCount
  captured=captured+';row='+TRANSFORM(row)+',id='+TRANSFORM(o.IndexToItemID(row))+',data='+TRANSFORM(o.ItemData(row))+',selected='+TRANSFORM(o.Selected(row))
  FOR col=1 TO MAX(1,o.ColumnCount)
   captured=captured+',c'+TRANSFORM(col)+'=['+TRANSFORM(o.List(row,col))+']'
  ENDFOR
 ENDFOR
 RETURN captured
ENDFUNC
