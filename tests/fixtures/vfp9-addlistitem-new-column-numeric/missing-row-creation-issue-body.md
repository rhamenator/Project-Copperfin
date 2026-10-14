## Evidence against current main

Current main:337156b342ff5e59db88859ededf6580066f3355.
While recovering the bounded THIRD-column numeric conversion requirements under
#5611/#6776, the native missing-ID case corroborates a separate successful-row
creation gap. This report does not authorize expanding the active numeric slice.

With an unbound ListBox or ComboBox, ColumnCount2, existing stable IDs11,22,33
and missing ordinary stable ID44, invoke:

```foxpro
o.AddListItem('candidate',44,2.9)
```

Two fresh installed VFP9 09.00.0000.7423 runs independently captured this among
780 calls. Native invocation succeeds, ListCount grows3->4, NewItemID is44 and
the new row reads candidate in column2 after truncation. The source, actual
Numeric BRS, full before/after/expanded sampled fields, complete raw/status/hash
and capture audit are retained in the active worktree fixture
tests/fixtures/vfp9-addlistitem-new-column-numeric/. Matching full raw SHA256:
8a5d71de09ddd892e7d86e51e38ee1ec832c0d19cd90a69dd4b720b20f890201.
Separate matching100-call native dual-route high-column pair corroborates
new-row effects, raw949565d54109f3da2c8afcb0e5b7af63d9a10f459180469621c9ab8eb0930bcc.
This is not a hidden-storage/global-capacity or full shared-state parity claim.

Copperfin's previously retained BOTH-compiler full304 sampled preservation
captures in tests/fixtures/vfp9-addlistitem-column-numeric/
preservation-fixed-gcc.out and preservation-fixed-clang.out contain eight
missing-id44 cases across both controls/modes/sessions: completed1/error0/
return0/ListCount remains3/no new row. They intentionally preserved, rather
than qualified or repaired, that excluded route. These are the prior captured
executions, NOT a claim of a fresh current-main build or a native oracle.

Current-main source inspection corroborates that route remains unchanged:
src/runtime/prg_engine_runtime_surface_object_methods.inl AddListItem creates a
row only in requested_column==1. For another positive column, it looks up the
requested stable ID and returns0 when absent. PR #7184's new admission explicitly
requires an already-existing ordinary ID, so it cannot repair this branch.
The discrepancy exists even for the safe integral third argument2; truncation,
large-value wrapping and allocation limits are not sufficient explanations.

## Follow-up boundary

After owner admission, recover and repair successful missing-row materialization
with explicit stable-ID, blank-cell, selection/payload/sort/return and failure
atomicity requirements; retain focused and broader regression evidence. Do not
fold it silently into numeric admission, alter the frozen numeric corpus, or
close #5611/#6776/other shared-list gaps from this report. No labels, VM use,
production change, independent-human or release qualification is implied.
