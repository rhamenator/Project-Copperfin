## Problem and exact revision

At current main `06d7b54107b5b22b20300108e1e19ef01357fb59`, native modeled
ListBox/ComboBox `AddListItem('candidate', 11)` ignores an already-existing
stable ID11. Installed VFP9 09.00.0000.7423 instead updates that row's first
column, retaining the same row count, IDs, second-column cells and ItemData.

This surfaced while recovering requirements for the admitted #5611/#6776
second-argument numeric-conversion slice, before helper/assertions/configuration
or production edits. The supplied ID here is ordinary integer11, so the gap
does not depend on rounding, overflow, alias emulation or numeric-mode policy.
It is a separate existing-ID row-update behavior, not authorization to expand
the active numeric-admission slice. No `agent-approved` label is requested or
applied, and no fix is included.

## Reproduction

For each `ListBox` and `ComboBox`, configure RowSourceType0/empty ControlSource
BEFORE populating the control, then run:

```foxpro
o = CREATEOBJECT('ListBox') && also repeat with ComboBox
o.RowSourceType = 0
o.ControlSource = ''
o.ColumnCount = 2
o.AddListItem('first', 11)
o.AddListItem('second', 22)
o.AddListItem('third', 33)
o.List(1,2) = 'cell1'
o.List(2,2) = 'cell2'
o.List(3,2) = 'cell3'
o.ItemData(1) = 101
o.ItemData(2) = 202
o.ItemData(3) = 303
o.ListIndex = 2
vReturn = o.AddListItem('candidate', 11)
? o.ListCount, o.List(1,1), o.List(1,2), o.ItemData(1)
```

VFP9: count3, first cell `candidate`, second cell `cell1`, payload101;
IDs11/22/33 and all other cells/payloads retained, return `.T.`, error0.
Current main observation: count3 and IDs11/22/33 retained, but first cell stays
`first`; all rows remain unchanged, return0, error0. Successful return/focus
semantics are recorded as context, not additional acceptance scope for this gap.

## Retained evidence and limits

Fresh independent native probe: both classes, empty/dense/sparse/high-ID
shapes,55 argument sources; **440 calls/full442 raw lines**, two fresh matching
full runs/runner0/cmp0/DONE440. RawSHA
`2bd7678d5bf6d57d3e25d902f6145a3286cef83602bed165fe6e91b12bc87d18`.
The sparse shape2/exact source11 records the above existing-ID update on both
classes. Complete source/raw/status/hash is retained under the dedicated
worktree's `build/addlistitem-native/`, to be included in its fixture/audit.

Current-main diagnostic compiles/runs0, both sessions complete/error0. It
links retained signed6566040df7 GNU libraries: relevant `src/`, `include/`
and test-support tree is identical to exact main06d7b5410, with empty diff
and all9 library hashes unchanged before-after. This is explicitly a
current-behavior observation, not a pristine whole-tree main build or native
requirement source. No VM, setup/credential or decompilation action occurred.

## Focused acceptance for a future admitted slice

Recover and verify existing-ID first-column update independently for both
native control classes, preserving row identity/count/other columns/ItemData
and documented exclusions. Keep explicit columns, binding, callbacks, sorting,
lifecycle, first-item types and return/focus policy separate unless independently
recovered and admitted. Add fail-before/pass-after verification with retained
evidence, traceability, sanitizer coverage, signed commits and required review/
CI gates. The active numeric-conversion work will not implement this behavior.
