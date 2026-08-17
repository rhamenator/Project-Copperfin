# GETFLDSTATE Traceability

## Scope

This low-reach data-integrity slice records `RQ-CF-PRG-005`, recovered from
the installed VFP9 `GETFLDSTATE( ) Function` help topic
`html/d52447bd-56d6-423f-80cf-ce73008bd2ea.htm`.

## Derived Requirement And Boundary

`DQ-prg-getfldstate-001`: a local cursor with row or table buffering enabled
shall return state for a named or 1-based numbered field, deletion state for
field number `0`, or a deletion-plus-all-fields string for `-1`. Existing rows
use `1`/`2`; appended rows use `3`/`4`; and EOF returns `.NULL.`. A mutation
shall remain changed after its value or deletion flag is restored.

The implementation stores field and deletion mutation state alongside the
already-owned buffered records. It does not alter cursor data, update policy,
record locking, disk writes, or transaction handling.

## Hazard, Misuse, And Rollback

`HZ-data-corruption-01` applies proportionally. Deriving field state by
comparing current bytes with the original record would falsely mark a restored
write or delete/recall sequence as unmodified, changing callers' update
decisions. The regression pins persistent mutation state and `TABLEREVERT`
clearing it.

Rollback is coherent: remove the per-buffer state, query implementation,
focused regression, and all corresponding coverage, matrix, handoff, changelog,
and traceability entries together. Buffered data and persistence behavior then
remain at their prior implementation boundary.

## Verification

`DV-prg-getfldstate-001` is the portable CTest target
`test_prg_engine_runtime_surface_functions_buffering`. It proves named and
numbered field queries, aggregate ordering, retained restored-value and
delete/recall state, reversion, appended states, and EOF null behavior. Its
machine-readable isolation is portable, serial, test-owned filesystem only,
with no environment, child-process, network, or sample dependency.

This is development-assurance evidence only; it is not a claim of formal
DO-178C compliance, certification, safety-critical suitability, complete VFP
buffering parity, or remote-cursor support.
