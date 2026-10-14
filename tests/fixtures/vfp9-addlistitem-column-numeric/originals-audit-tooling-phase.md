# Post-freeze original95 audit tooling; no caller migration

`audit-originals-complete.cjs` is authored after the original259 freeze while
the unchanged-source driver runs. It does not modify the frozen assertions,
helper, caller, registrations, isolation, drivers or expectations. It is an
evidence checker, not a native conversion oracle or completed acceptance.

The checker requires BOTH completed GNU/Clang originals: configure/compile0,
actual CTest8, completion timestamps, full95 ordered unique results/start IDs,
all prior90 and the new210-direct subject passed, only four new660-public
subjects failed, exact built command/isolation audits, no unexpected runtime
sanitizer diagnostics, full multiline selected assertion blocks identical
across compilers, both259 before/after checks and current hashes, unchanged
three caller inputs, and the driver completion0. Missing or partial files fail
closed. The full failure records are printed, not reduced to prefix counts.

Synthetic `audit-originals-selftest.cjs` tests the tool only. Initial status1
revealed that a wrong Start-ID/name mapping could escape detection even while
the result inventory matched. Exact initial tool/self-test sources, stdout,
stderr and status are retained in `build/addlistitem-column-native/` under
`originals-audit-initial-*`. Add a unique-start/result-ID/name check without
changing expected runtime values. Corrected self-test0 has one synthetic
positive and13 rejected mutations (including partial inventory), plus full
multiline hash sensitivity and rejection of missing completed evidence.
These synthetic logs must never be represented as real compiler executions.

The actual pending-evidence checker returns1 because the driver completion
status is not yet present. This is expected fail-closed pending evidence, NOT
an original gate pass or product regression. GNU configure/compile0 and built
commands/isolation audit0 are now present; its full95 execution is running.
Clang configure/build/full95 remain pending behind it. All259 frozen inputs
and the existing frozen-phase evidence rechecked0 after this tooling work.
NEVER run fixed-only-hostile vectors against the old caller. Do not wire until
BOTH complete actual original audits have passed; fixed verification, hostile
corroboration, preservation, DQ/DV, review and integration still remain.

This phase is linked to RQ-CF-PRG-ADDLISTITEM-COLUMN-NUMERIC-001 and
VR-5611-ADDLISTITEM-COLUMN-NUMERIC-001/002. No native-global-capacity,
independent-human review, hazard/parent closure or VM/reset completion claim.
