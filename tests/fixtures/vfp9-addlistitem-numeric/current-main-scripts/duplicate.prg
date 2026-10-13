PUBLIC o,vReturn,nCode
o=CREATEOBJECT('ComboBox')
o.RowSourceType=0
o.ControlSource=''
o.ColumnCount=2
o.AddListItem('first',11)
o.AddListItem('second',22)
o.AddListItem('third',33)
o.List(1,2)='cell1'
o.List(2,2)='cell2'
o.List(3,2)='cell3'
o.ItemData(1)=101
o.ItemData(2)=202
o.ItemData(3)=303
o.ListIndex=2
nCode=0
TRY
vReturn=o.AddListItem('candidate',11)
CATCH TO problem
nCode=problem.ErrorNo
ENDTRY
RETURN
