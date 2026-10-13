## Observed compatibility gap (outside active RemoveItem numeric admission)

Current main revision: `6fe579e32dcf80e77e0c66e9b07845c908ebecef`.
Found while auditing independent evidence for PR #7168, under admitted
numeric-conversion #5611/#6776. This issue is NOT an expansion of that slice
or an implementation admission; no production lifecycle fix is included there.

Installed VFP9 SP2 `09.00.0000.7423` clears a populated native ListBox/ComboBox
when its already-zero RowSourceType is assigned ordinary Numeric zero again:

```foxpro
o = CREATEOBJECT('ListBox') && also ComboBox
o.AddListItem('first', 11)
o.AddListItem('second', 22)
o.AddListItem('third', 33)
nBefore = o.ListCount && 3
o.RowSourceType = 0
nAfter = o.ListCount  && installed VFP9: 0; Copperfin main: 3
```

Fresh native two-control probe and full raw are retained in
`tests/fixtures/vfp9-removeitem-numeric/review2-rst0-assignment.prg` and paired
`review2-rst0-assignment-{first,repeat}.out`. Current-main supporting diagnostic
`review2-rst0-main-diagnostic.cpp`/`review2-rst0-main.out` completes both cases
and reports before3/after3. It links retained signed TopItemID slice4355b990
GNU libraries with relevant property-write sources unchanged from merged
main6fe579; full library hashes verified. This is supporting verification,
not a pristine whole-tree rebuild or the requirement oracle.

The original PR7168 review2 native probe assigned RowSourceType after adding
rows, so all calls actually operated on an empty native control. That raw is
preserved and its successful-populated-row interpretation explicitly corrected.
A separate fresh corroborating probe configures RowSourceType0 BEFORE adding
rows and verifies before3 for every RemoveItem call. Numeric row admission is
separate from property-assignment lifecycle; exact64/Copperfin mode remain
named owner-derived extension boundaries, not native syntax claims.

## Bounded future acceptance

- Independently recover ordinary same-value RowSourceType assignment lifecycle
  for ListBox/ComboBox, including identity/cells/payload/focus/selection and
  errors, without deriving expectations from Copperfin's current writer.
- Determine the intended compatibility contract and separate extension-only
  exact64 configuration semantics before implementation.
- Add fail-before/pass-after coverage and preservation of unrelated binding,
  materialization, callback and numeric-admission routes.
- Keep #5611/#6776 numeric admission and #7163/#7164 shared-state/type gaps
  distinct; do not close them from this lifecycle observation.

This report claims only the observed count mismatch, not a root cause, complete
RowSourceType family contract, policy decision or implementation authorization.
