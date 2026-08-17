# ON() Runtime Traceability

## Scope

This report records the low-reach runtime slice `RQ-CF-PRG-002`. Its governing
compatibility evidence is the installed VFP9 `ON()` Function help topic
`html/421c6247-80e9-478a-a7a3-2f2bd4d816c0.htm`, which defines
`ON(cONCommand [, cKeyLabelName])` and states that the function returns an
assigned event command or an empty string.

## Derived Requirement And Boundary

`DQ-prg-on-function-001`: the existing first-pass `ON ERROR` and static,
data-session-scoped `ON KEY [LABEL]` state shall be observable without
mutation. `ON('ERROR')` returns the active error clause. `ON('KEY',
cKeyLabel)` normalizes the label case-insensitively and returns the exact
assigned static command in the current data session, or an empty string when
the label is omitted or unassigned.

The VFP9 topic also lists `ESCAPE` and `PAGE`. Copperfin does not implement
`ON ESCAPE` or `ON PAGE` assignment commands, so their current unassigned empty
result is not evidence of their event behavior. The existing `ON('SHUTDOWN')`
result is a Copperfin extension and is not used as VFP9 compatibility evidence.

No query dispatches a handler or changes assignment state. No host keyboard
capture, input injection, UI/form routing, thread, file, process, network, or
credential authority is introduced.

## Hazard, Misuse, And Rollback

`HZ-system-failure-01` applies: an incorrect query could report an assignment
from another data session or could accidentally activate it. The lookup reads
only the active session's normalized static-key map and returns a string; it
does not reuse the event-dispatch path.

Rollback is one coherent change: remove the lookup callback, runtime-surface
`KEY` branch, `test_prg_engine_on_function`, this report, and the matching
requirements/coverage/handoff/changelog records together. Existing `ON ERROR`,
static key dispatch, and the shutdown extension remain independently covered.

## Verification

`DV-prg-on-function-001` is the dedicated portable CTest target
`test_prg_engine_on_function`. GCC 15.2 Release built it and the focused run
passed `1/1` on Linux. The regression proves default, omitted-label, unassigned,
cleared, and unknown-topic empty results; exact `ON ERROR` clause readback;
case-insensitive static-key lookup; and independent key assignments across two
data sessions. Its machine-readable CTest isolation record is complete:
portable, parallel-safe, test-owned filesystem only, and no environment,
child-process, network, or sample dependency.

This is development-assurance evidence only; it is not a claim of formal
DO-178C compliance, certification, or safety-critical suitability.
