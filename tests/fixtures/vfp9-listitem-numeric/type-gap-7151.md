## Evidenced gap on current main

Current main: c2073149c996474f1beb753e776a20df902c6897.
This records a separate unsupported-type admission gap discovered during the
bounded #5611/#6776 one-argument ListItem Numeric/exact READ slice. It does not
authorize implementation or expand that slice. No agent-approved label is
requested or applied.

Fresh installed VFP9 09.00.0000.7423 reports catchable error 31 for each of
`.T.`, `.NULL.`, `'22.9'` and `$22.9` as the single ListItem argument. Both
ListBox and ComboBox, with dense IDs 1/2/3 and sparse IDs 11/22/33, agree.
Two independently repeated 302-line/300-call outputs match SHA-256
d56fbc6bec870ab93a7ae5ab3cad564132494c3d0c05fdce96ff4cb747d9183c.
These inputs are outside the active Numeric/exact migration.

Both full originals against unchanged main dispatch completed before migration:
GNU 35/39, 472.75s, and Clang ASan/UBSan/float-cast-overflow 35/39, 834.09s.
Only the four new Numeric/exact public subjects failed; every old-coercion
preservation assertion passed in both modes and sessions. Actual main behavior
for the same inputs is no error: `.T.` maps to ID1 (dense `first`, sparse empty),
NULL to ID0/empty, Character and Currency22.9 to rounded ID23/empty. Each result
is Character. This is verification evidence, not a requirement source.

## Reproduction

Run each input separately on a fresh control; substitute ComboBox and sparse
AddListItem('first',11)/('second',22)/('third',33) to repeat the other controls.

```foxpro
oList = CREATEOBJECT('ListBox')
oList.AddItem('first')
oList.AddItem('second')
oList.AddItem('third')
cResult = 'sentinel'
nCode = 0
TRY
    cResult = oList.ListItem(.T.)
CATCH TO problem
    nCode = problem.ErrorNo
ENDTRY
? nCode, cResult
```

Repeat `.T.` with `.NULL.`, `'22.9'` and `$22.9`. Native VFP9 keeps the sentinel
and reports31; current main returns Character without an error.

## Recovery and acceptance boundary

Recover operation-specific type admission and catchability from the installed
native oracle and shipped ListItem Property help, not from adjacent methods.
Define the Copperfin extension policy explicitly if different. Verify both
controls, sparse/dense IDs, both modes/sessions/routes, Character kind,
localized catchable31, sentinel/continuation and unchanged state. Explicit
columns, writes, ID lifecycle, arity, generic parser/reflection and other methods
need separate scope/evidence. No parent, hazard, KBX or release closure.

Independent native/help/probe, frozen assertions and full original logs are
retained in tests/fixtures/vfp9-listitem-numeric in the dedicated active slice
worktree. Original GNU raw SHA-256
a0706147b38370d29eb3edeaff5e8d38a393c02281aa0abefad09f9f793961b6;
original Clang raw SHA-256
94a2ba17018222ad01a94e3062a6e518946c777fb825700b760367a17a1fedbf.
The Numeric slice deliberately retains these coercions; this issue remains
unadmitted until owner-directed intake.
