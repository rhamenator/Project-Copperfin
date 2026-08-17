# SETFLDSTATE Traceability

## Scope

This low-reach data-integrity slice records `RQ-CF-PRG-006`, recovered from
the installed VFP9 `SETFLDSTATE( ) Function` help topic
`html/4ef6b1d8-13f8-4947-b4a0-573589d83380.htm`.

## Derived Requirement And Boundary

`DQ-prg-setfldstate-001`: a current local cursor with row or table buffering
enabled shall accept a named or 1-based field selector, or selector `0` for the
record deletion state, and assign exactly state `1`, `2`, `3`, or `4`. The
assignment shall be observable through `GETFLDSTATE()`. For an existing record,
states `1` and `3` shall suppress that field/deletion's `TABLEUPDATE` write and
states `2` and `4` shall allow it. Appended records must still be fully
materialized.

## Hazard, Misuse, And Rollback

`HZ-data-corruption-01` applies proportionally. Ignoring an assigned unchanged
state would overwrite a concurrent value that the caller deliberately excluded;
skipping an appended field would instead create an incomplete record. The
implementation applies the same state gate to row-buffered and table-buffered
update lanes and explicitly retains full appended-record output.

Rollback is coherent: remove the state-assignment handling, both write gates,
the focused regression, and corresponding coverage, requirement, handoff,
changelog, and traceability entries together. Buffered data behavior then
returns to its previous GETFLDSTATE-only boundary.

## Verification

`DV-prg-setfldstate-001` is the portable CTest target
`test_prg_engine_runtime_surface_functions_buffering`. It proves named,
numbered, deletion, and explicit-work-area assignment; state readback; invalid
state/field rejection; persisted state-`2` data; and state-`1` field/deletion
suppression. Its machine-readable isolation is portable, serial, test-owned
filesystem only, with no environment, child-process, network, or sample
dependency.

This is development-assurance evidence only; it is not a claim of formal
DO-178C compliance, certification, safety-critical suitability, complete VFP
buffering parity, or remote-cursor support.
