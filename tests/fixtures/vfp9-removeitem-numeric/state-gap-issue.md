## Independent recovery gap against current main

Current main: `6fe579e32dcf80e77e0c66e9b07845c908ebecef` (PR7161 merge).
Discovered while recovering the bounded RemoveItem numeric argument slice
under #5611/#6776. This issue does not expand that slice or admit a fix.

Installed Visual FoxPro 09.00.0000.7423, using the existing private Wine
runner, disagrees with current Copperfin's ordinary valid removal return
and shared post-removal state. Fresh matching native pairs: 423 calls/425
lines (SHA256 f1de30dbed06f009a1b50c388ba3dfccb8eacf6f6a22c34549e86142cb78cba0),
297/299 boundary and 180/182 range; all runner0/cmp0/DONE. No decompiled input.

Reproducer (repeat separately with argument1,2,3 and both classes):

```foxpro
o=CREATEOBJECT('ListBox')
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
o.TopIndex=2
result=o.RemoveItem(1)
```

For this sparse ListBox, native valid removal returns Logical .T.; Copperfin
returns empty Character. Native remove1 gives count2/focus1/ID22, top2/ID33,
Character third Value/DisplayValue and surviving IDs22/33. Copperfin gives
same surviving IDs/focus but top1/ID22 and Character second. Native remove2
gives count2/focus0/ID0, top2/ID33, Character third and both selection bits
false. Copperfin instead focuses row2/ID33 with bits false/true, top1/ID11.
Native remove3 keeps focus2/ID22/top2/ID22/second and surviving IDs11/22;
the ordinary row/payload removal matches in that tested case, apart from
the method return. Both classes and modes were freshly diagnosed, not
inferred from a sibling helper. Native multiselect/dense/empty evidence also
retained; no claim of sorted/viewport/event completeness.

Fresh current-main 28-call diagnostic completed0/scaffold_failures0,
full raw SHA256 4a1490118d50e0fe5c80f76fd6ea100b92376f92294b452b30eb2189cc638c5d.
It links retained signed4355b990 GNU libraries, with matching relevant
source tree at current main (merge has no runtime/include/test-support delta),
and records their full hashes. This is current behavior evidence, not a
requirement oracle or a pristine whole-tree binary claim. Source/raw/status
and independent probes are retained for the upcoming bounded slice under
tests/fixtures/vfp9-removeitem-numeric/.

The numeric slice will not repair return values, focus/selection, text or
shared removal/top synchronization to make its conversion tests pass. Those
effects need their own owner-admitted recovery/implementation. A correct fix
must establish independent valid-removal state/return requirements across
both controls and preserve row identity, columns and ItemData atomically
within the documented boundary. No parent, hazard or release closure implied.
