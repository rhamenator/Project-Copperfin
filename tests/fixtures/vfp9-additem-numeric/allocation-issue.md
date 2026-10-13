## Problem

Against current main `87b0fd854431381db977740e6d6af3a5b3b967ed`, valid
`AddItem('candidate',1)` on an unbound native ListBox/ComboBox containing
supplied stable ID32767 generates ID32768. Installed VFP9 09.00.0000.7423
generates ID1 for the same equivalent consumer and retains existing ID32767.
This is automatic-ID allocation, not supplied numeric-position conversion.

```foxpro
o=CREATEOBJECT('ListBox') && repeat with ComboBox
o.RowSourceType=0
o.ControlSource=''
o.ColumnCount=2
o.AddListItem('first',32767)
o.List(1,2)='cell1'
o.ItemData(1)=101
o.ListIndex=1
o.AddItem('candidate',1)
```

Both native captures complete/invoke/error0/count2, generated NewItemID1,
ordered IDs1,32767. Current-main diagnostic completes/error0/count2,
ordered IDs32768,32767 on BOTH classes; retained existing cell/payload intact.
The generated32768 is inconsistent with the separately recovered supplied-ID
admission ceiling32767; this report does not claim every downstream effect
has been independently reproduced or that successful focus/text/top/return
already matches native behavior.

## Source and evidence

- `src/runtime/prg_engine_runtime_surface_list_state_helpers.inl:1207`
  `next_native_list_control_item_id` uses max existing positive key +1;
  the AddItem insertion arm calls it in
  `src/runtime/prg_engine_runtime_surface_object_methods.inl`.
- Independent native AddItem440-call first/repeat pair, runner/cmp0,
  DONE440/stderr empty, both full raw SHA256
  `d42051d8ae41f3e7874b7fbbd7a07d02717eae66cb246ed2b10027490c74f077`.
  See BOTH `shape=3|1|` rows. Configure-before-population and full before/after
  states are retained; no decompilation or local VM was used.
- NON-NATIVE current-main diagnostic compile/run0, raw SHA256
  `baf598d72ca791d0561ed07cdf0851c222a5e520d93872733999a8973d001586`.
  It links nine retained signed2fb9b80455 GNU libraries: relevant src/include/
  test-support trees are byte-identical to current main87b, all nine library
  hashes unchanged before/after. This is not a pristine whole-main rebuild or
  a requirement oracle.
- Full source/raw/status/hash under the in-progress
  `tests/fixtures/vfp9-additem-numeric` evidence; corresponding ignored build
  retains executable. Shipped AddItem help and independent source are named
  in the pre-code checkpoint.

## Scope and acceptance

Recover automatic-ID allocation separately for sparse/high supplied IDs and
repair the bounded generated-ID behavior with independent native/help
requirements, preserved stable identities/cells/payload and both-control
regressions. Do not substitute display positions for unique IDs or reuse a
max+1 implementation as its own requirement source. Assess the shared allocator
callers before any wider change; this report only reproduces AddItem.

Found during admitted #5611/#6776 AddItem numeric-position recovery.
Automatic allocation/shared successful state is EXCLUDED from that active
slice. No production fix, agent-approved label, issue closure or scope expansion
is made by this report. Parents remain open; #7163 shared-state and other
unadmitted work remain separate.
