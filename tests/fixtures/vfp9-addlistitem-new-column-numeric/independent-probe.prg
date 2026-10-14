* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Fresh installed-VFP9 missing-ID THIRD-column observations, no conversion oracle table.
LOCAL ARRAY args[65], columns[3]
args[1]='-3'
args[2]='-1.9'
args[3]='-0.9'
args[4]='0'
args[5]='0.49'
args[6]='0.5'
args[7]='0.99'
args[8]='1-2^(-52)'
args[9]='1-2^(-53)'
args[10]='1'
args[11]='1+2^(-52)'
args[12]='1.5'
args[13]='1.9'
args[14]='2.1'
args[15]='2.5'
args[16]='2.9'
args[17]='3.9'
args[18]='8.9'
args[19]='9'
args[20]='32744.9'
args[21]='32745'
args[22]='32745.9'
args[23]='32746'
args[24]='32767.9'
args[25]='32768'
args[26]='65534.9'
args[27]='65535'
args[28]='65535.9'
args[29]='65536'
args[30]='65536.9'
args[31]='65537'
args[32]='65537.9'
args[33]='65538.9'
args[34]='131071'
args[35]='131072'
args[36]='131073.9'
args[37]='2147483647'
args[38]='2147483647.9'
args[39]='2147483648-2^(-22)'
args[40]='2147483648-2^(-23)'
args[41]='2147483648'
args[42]='2147483648.9'
args[43]='2147483649'
args[44]='4294967295'
args[45]='4294967295.9'
args[46]='4294967296'
args[47]='4294967297.9'
args[48]='4294967298.9'
args[49]='-2147483648'
args[50]='-2147483648.9'
args[51]='-2147483649'
args[52]='-4294967295.9'
args[53]='-4294967294.9'
args[54]='-4294967296'
args[55]='6442450943.9'
args[56]='6442450944'
args[57]='-6442450944'
args[58]='-6442450943'
args[59]='1E20'
args[60]='-1E20'
args[61]='1E300'
args[62]='-1E300'
args[63]='(1E300*1E300)'
args[64]='(-1E300*1E300)'
args[65]='1E-300'
columns[1]=0
columns[2]=2
columns[3]=9
LOCAL classNo,configNo,shapeNo,argNo,rowNo,colNo,target,ctrl,className,operand
LOCAL value,bitsBefore,bitsAfter,typeBefore,typeAfter,stage,errorNo,errorMessage
LOCAL before,after,expanded,result,returnCapture,total
LOCAL outputPath
outputPath='Z:\home\rich\.codex\worktrees\addlistitem-new-column-numeric-5611\Project-Copperfin\build\addlistitem-new-column-native\native-results.txt'
total=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
FOR classNo=1 TO 2
 className=IIF(classNo=1,'ListBox','ComboBox')
 FOR configNo=1 TO ALEN(columns)
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
     ctrl.RowSourceType=0
     ctrl.ControlSource=''
     ctrl.ColumnCount=columns[configNo]
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
    =STRTOFILE('CASE|'+className+'|'+TRANSFORM(columns[configNo])+'|'+TRANSFORM(shapeNo)+'|'+TRANSFORM(target)+'|'+operand+'|'+typeBefore+'|'+bitsBefore+'|'+typeAfter+'|'+bitsAfter+'|'+stage+'|'+TRANSFORM(errorNo)+'|'+errorMessage+'|'+returnCapture+'|'+before+'|'+after+'|'+expanded+CHR(13)+CHR(10),outputPath,1)
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
