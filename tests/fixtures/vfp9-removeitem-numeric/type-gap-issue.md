## Independent recovery gap against current main

Current main: `6fe579e32dcf80e77e0c66e9b07845c908ebecef` (PR7161 merge).
Discovered while recovering the bounded RemoveItem non-NULL Numeric/exact
argument slice under #5611/#6776. Other types remain excluded; this issue
does not admit a type fix or expand the active slice.

Fresh installed VFP9 09.00.0000.7423 rejects Logical .T./.F., NULL,
Character '2' and Currency $2.5 with error27 before removing any row on
both ListBox and ComboBox, empty/dense/sparse shapes. All state snapshots
remain unchanged. Main native 423-call matching pair runner0/cmp0/DONE,
full425-line raw SHA256
f1de30dbed06f009a1b50c388ba3dfccb8eacf6f6a22c34549e86142cb78cba0.

```foxpro
o=CREATEOBJECT('ListBox')
o.AddListItem('first',11)
o.AddListItem('second',22)
o.AddListItem('third',33)
o.ListIndex=2
o.RemoveItem(.T.)
```

Repeat with .NULL., '2', $2.5 and ComboBox. Fresh current-main diagnosis
in BOTH NUMERICBEHAVIOR modes accepts .T. and removes first, accepts '2'
and removes second, rounds $2.5 to third and removes third; NULL is an
accepted no-op rather than native error27. Full28-call diagnostic completed0/
scaffold_failures0, SHA256
4a1490118d50e0fe5c80f76fd6ea100b92376f92294b452b30eb2189cc638c5d.
It uses retained signed4355b990 GNU libraries with no relevant source delta
to current main and recorded library hashes; no pristine whole-tree claim.
Native source/raw/status and fresh current-behavior diagnostic retained under
tests/fixtures/vfp9-removeitem-numeric/ for the upcoming bounded slice.

Native observed types must not be replaced by Copperfin's current coercions
as requirements. Keep conversion-only tests explicitly separated from
non-native preservation controls. A later admitted type slice must verify
catchable localized error27 and unchanged row/payload/identity/selection
state without extending parser/arity/lifecycle or shared removal policy.
