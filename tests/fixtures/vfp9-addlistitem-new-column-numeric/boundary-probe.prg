* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* Independently observed fractional/binary/storage and signed-cycle boundary effects.
LOCAL ARRAY operands[36], sampleColumns[14]
operands[1]='32743.9'
operands[2]='32744'
operands[3]='32744.9'
operands[4]='32745'
operands[5]='32745.49'
operands[6]='32745.5'
operands[7]='32745.9'
operands[8]='32746-2^(-38)'
operands[9]='32746'
operands[10]='32746.5'
operands[11]='32747'
operands[12]='65535'
operands[13]='65535.9'
operands[14]='65536-2^(-37)'
operands[15]='65536'
operands[16]='65536.9'
operands[17]='65537'
operands[18]='65537.9'
operands[19]='65538.9'
operands[20]='4295000041'
operands[21]='4295000041.9'
operands[22]='4295000042'
operands[23]='4295032831'
operands[24]='4295032832'
operands[25]='-4294934551'
operands[26]='-4294934550'
operands[27]='-4294901761'
operands[28]='-4294901760'
operands[29]='2147483647.9'
operands[30]='2147483648-2^(-22)'
operands[31]='2147483648'
operands[32]='2147483648.9'
operands[33]='2147483649'
operands[34]='-2147483648'
operands[35]='-2147483648.9'
operands[36]='-2147483649'
sampleColumns[1]=1
sampleColumns[2]=2
sampleColumns[3]=3
sampleColumns[4]=9
sampleColumns[5]=32744
sampleColumns[6]=32745
sampleColumns[7]=32746
sampleColumns[8]=32767
sampleColumns[9]=32768
sampleColumns[10]=65534
sampleColumns[11]=65535
sampleColumns[12]=65536
sampleColumns[13]=65537
sampleColumns[14]=65538
LOCAL classNo,shapeNo,argNo,className,target,o,operand,value,bits,phase,err,expandedError
LOCAL before,after,rowIndex,cells,ix,valIndex,valID,errorIndex,errorID,idxCapture,idCapture,total,path
path='Z:\home\rich\.codex\worktrees\addlistitem-new-column-numeric-5611\Project-Copperfin\build\addlistitem-new-column-native\boundary-results.txt'
total=0
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),path,0)
FOR classNo=1 TO 2
 className=IIF(classNo=1,'ListBox','ComboBox')
 FOR shapeNo=0 TO 1
  target=IIF(shapeNo=0,11,44)
  FOR argNo=1 TO ALEN(operands)
   operand=operands[argNo]
   phase='setup'
   err=0
   expandedError=0
   before='<not-captured>'
   after='<not-captured>'
   rowIndex=0
   cells='<no-target>'
   bits='<not-evaluated>'
   o=.NULL.
   TRY
    o=CREATEOBJECT(className)
    o.RowSourceType=0
    o.ControlSource=''
    o.ColumnCount=2
    IF shapeNo=1
     o.AddListItem('first',11)
     o.AddListItem('middle',22)
     o.AddListItem('last',33)
     o.List(1,2)='base1'
     o.List(2,2)='base2'
     o.List(3,2)='base3'
     o.ListIndex=2
    ENDIF
    before=Metadata(o)
    phase='evaluate'
    value=EVALUATE(operand)
    bits=NumericBits(value)
    phase='invoke'
    o.AddListItem('candidate',target,value)
   CATCH TO callProblem
    err=callProblem.ErrorNo
   ENDTRY
   IF VARTYPE(o)='O'
    after=Metadata(o)
    TRY
     o.ColumnCount=65538
     rowIndex=o.ItemIDToIndex(target)
     IF rowIndex>0
      cells=''
      FOR ix=1 TO ALEN(sampleColumns)
       errorIndex=0
       errorID=0
       idxCapture='<not-read>'
       idCapture='<not-read>'
       TRY
        valIndex=o.List(rowIndex,sampleColumns[ix])
        idxCapture=VARTYPE(valIndex)+':'+TRANSFORM(valIndex)
       CATCH TO indexProblem
        errorIndex=indexProblem.ErrorNo
       ENDTRY
       TRY
        valID=o.ListItem(target,sampleColumns[ix])
        idCapture=VARTYPE(valID)+':'+TRANSFORM(valID)
       CATCH TO idProblem
        errorID=idProblem.ErrorNo
       ENDTRY
       cells=cells+'['+TRANSFORM(sampleColumns[ix])+';'+TRANSFORM(errorIndex)+';'+idxCapture+';'+TRANSFORM(errorID)+';'+idCapture+']'
      ENDFOR
     ENDIF
    CATCH TO expandProblem
     expandedError=expandProblem.ErrorNo
    ENDTRY
   ENDIF
   =STRTOFILE('READ|'+className+'|'+TRANSFORM(shapeNo)+'|'+TRANSFORM(target)+'|'+operand+'|'+phase+'|'+TRANSFORM(err)+'|'+TRANSFORM(expandedError)+'|'+bits+'|'+before+'|'+after+'|'+TRANSFORM(rowIndex)+'|'+cells+CHR(13)+CHR(10),path,1)
   total=total+1
   o=.NULL.
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+TRANSFORM(total)+CHR(13)+CHR(10),path,1)
RETURN

FUNCTION Metadata(o)
 RETURN 'count='+TRANSFORM(o.ListCount)+';columns='+TRANSFORM(o.ColumnCount)+';newid='+TRANSFORM(o.NewItemID)+';newindex='+TRANSFORM(o.NewIndex)+';index='+TRANSFORM(o.ListIndex)+';id='+TRANSFORM(o.ListItemID)+';display='+TRANSFORM(o.DisplayValue)+';value='+TRANSFORM(o.Value)
ENDFUNC

FUNCTION NumericBits(n)
 LOCAL raw,c,k
 raw=BINTOC(n,'BRS')
 c=''
 FOR k=1 TO LEN(raw)
  c=c+RIGHT(TRANSFORM(ASC(SUBSTR(raw,k,1)),'@0'),2)
 ENDFOR
 RETURN c
ENDFUNC
