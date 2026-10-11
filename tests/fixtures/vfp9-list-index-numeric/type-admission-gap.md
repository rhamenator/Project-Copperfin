# List one-argument reads admit Logical/NULL selectors that VFP9 rejects

Discovered while recovering the admitted #5611/#6776 one-argument Numeric/exact
List READ slice. This report is separate from the active conversion migration;
no type-admission change or agent-approved label is requested/applied.
Current origin/main: f77021174fe86210e8c71bf0c74b3413c7b15e47.

## Reproduction

For each native ListBox and ComboBox, create a fresh three-row control:

```foxpro
oList = CREATEOBJECT('ListBox') && repeat with ComboBox
oList.AddListItem('first', 11)
oList.AddListItem('second', 22)
oList.AddListItem('third', 33)
oList.ListIndex = 2
cResult = 'sentinel'
nCode = 0
TRY
    cResult = oList.List(.T.) && repeat with .NULL.
CATCH TO problem
    nCode = problem.ErrorNo
ENDTRY
```

Repeat with dense AddItem rows and both Copperfin NUMERICBEHAVIOR modes.

## Independent native result versus current Copperfin

Installed Visual FoxPro09.00.0000.7423 rejects both LogicalTRUE and NULL with
catchable error31 for both classes and identity shapes; sentinel is retained.
Current Copperfin one-argument reads accept LogicalTRUE as displayrow1,
returning Character first; NULL returns empty Character, no error. These
are existing coercion preservation controls, not Numeric requirements.

Retained evidence in tests/fixtures/vfp9-list-index-numeric:
probe.prg, unchanged vfp9-probe-snapshot.txt, matching native-first.out and
native-repeat.out302lines/300calls, SHA
87b64de6a995fd6b427afd790df06c9cb9dbf3e3967cec05f1b675fdad50124a.
The probe ran fresh objects and retained count, selection, item identities,
payloads and selection bits. Native matching error31 is visible on the .T.
and .NULL. records. The shipped Microsoft List Property help defines a
numeric display-order selector and Character result, not Logical/NULL coercion.

The complete GNU original44-subject run before ANY production migration
completed2026-10-11T16:43:04Z,40/44 pass/594.32s. All248direct/four944public
inputs and unsupported-coercion preservation assertions complete; only the
four new Numeric public subjects fail, with zero Logical/NULL control failure.
Full unedited original-gcc.log7626lines SHA
18d4b0d39c42a5ff97a61b9a47d523775b037a1f8c2c40e8721e98eb272cdde4.
Its original production method/reflection/expression/declarations are exact
main bytes; the new checked helper remains unused. Clang original is still
pending and no sanitizer acceptance is claimed here.

## Proposed follow-up boundary

Recover and decide one-argument List type admission independently, then add
fresh localized catchable-error and read-state regression coverage for both
controls and identity shapes. Keep explicit columns, writes, arity, list
lifecycle and generic parser/reflection expansion separate. ListItem's related
type-admission report #7151 does not admit this List change. No parent/family/
hazard/release closure is implied by this focused compatibility report.
