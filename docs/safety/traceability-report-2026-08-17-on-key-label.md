# ON KEY LABEL Runtime Traceability

## Scope

This report records the low-reach headless runtime slice `RQ-CF-PRG-001`.
The governing compatibility evidence is the installed VFP9 help topics for
`ON KEY LABEL`, `PUSH KEY`, and `POP KEY`, recorded in the requirements matrix.

## Derived Requirement And Boundary

`DQ-prg-on-key-label-001`: static key-label assignments are session state, not
host-global state. They may be dispatched only while the runtime is waiting in
`READ EVENTS`; dispatch must use the same bounded iterative frame/return path
as other headless event callbacks. `PUSH KEY` snapshots the complete assignment
set; `PUSH KEY CLEAR` then clears it; `POP KEY` restores the latest snapshot;
and `POP KEY ALL` clears both current and saved state.

No live keyboard hook, background thread, operating-system input injection,
form-local `KeyPress` path, mouse mapping, system-menu mapping, or renderer is
introduced. Macro-backed action text is not admitted by this static lane.

## Hazard, Misuse, And Rollback

`HZ-system-failure-01` applies: accidental dispatch outside an active event
loop could alter application state at an unexpected point. The public hook
therefore fails closed unless `READ EVENTS` is active, and action completion
returns to the existing event-loop pause. The implementation has no file,
process, network, credential, package, or concurrency authority.

Rollback is one coherent change: remove the parser/dispatcher/API state,
`test_prg_engine_on_key_label`, this report, and the matching documentation and
traceability rows together. The earlier keyboard telemetry behavior remains.

## Verification

`DV-prg-on-key-label-001` is the dedicated portable CTest target
`test_prg_engine_on_key_label`. GCC 15.2 Release built it and the focused run
passed `1/1` on Linux. It proves assignment dispatch and restoration,
case-insensitive label normalization, `PUSH KEY CLEAR`/`POP KEY` snapshots,
`ON KEY` all-clear, `POP KEY ALL`, event-loop restoration, and invariant
telemetry. Native and hosted platform keyboard/UI validation remains required
before any broader compatibility claim.

This is development-assurance evidence only; it is not a claim of formal
DO-178C compliance, certification, or safety-critical suitability.
