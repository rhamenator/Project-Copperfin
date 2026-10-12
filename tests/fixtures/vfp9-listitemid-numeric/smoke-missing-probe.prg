* RQ-CF-PRG-LISTITEMID-NUMERIC-001: diagnostic for the old two-row smoke.
* Native direct assignment is the oracle; SETPEM success is a Copperfin route
* derivation, not claimed as observed native SETPEM syntax.
LOCAL oPlainCombo, nFirstItemId, nSecondItemId
? 'VERSION|' + VERSION()
oPlainCombo = CREATEOBJECT('ComboBox')
oPlainCombo.ColumnCount = 2
oPlainCombo.AddListItem('Alpha')
nFirstItemId = oPlainCombo.NewItemId
oPlainCombo.AddListItem('A', oPlainCombo.NewItemId, 2)
oPlainCombo.AddListItem('Beta')
nSecondItemId = oPlainCombo.NewItemId
oPlainCombo.AddListItem('B', oPlainCombo.NewItemId, 2)
oPlainCombo.ListItemID = nFirstItemId
? 'BEFORE|' + TRANSFORM(oPlainCombo.ListIndex) + '|' + TRANSFORM(oPlainCombo.ListItemID) + '|' + oPlainCombo.DisplayValue
oPlainCombo.ListItemID = 77
? 'AFTER|' + TRANSFORM(oPlainCombo.ListIndex) + '|' + TRANSFORM(oPlainCombo.ListItemID) + '|' + oPlainCombo.DisplayValue + '|' + TRANSFORM(oPlainCombo.ListCount)
? 'ROWS|' + oPlainCombo.List(1,1) + '|' + oPlainCombo.List(1,2) + '|' + oPlainCombo.List(2,1) + '|' + oPlainCombo.List(2,2)
? 'IDS|' + TRANSFORM(oPlainCombo.IndexToItemID(1)) + '|' + TRANSFORM(oPlainCombo.IndexToItemID(2))
