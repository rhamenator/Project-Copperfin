# Hosted stress prerequisite repair (#7202)

RQ-CF-CI-V2A-SYNC-001 / VR-CI-V2A-SYNC-001, DQ-DV-CI-V2A-SYNC-001.
[Pre-code contract](contract.md), tracking #7202, owner actionable-CI instruction.
This repairs only an existing verification precondition exposed by PR7201 CI;
no production lock/Numeric implementation or compatibility requirement changes.

Original exactc9784d005 head, stress38836793953/job116767618283: first whole
control-flow repetition PASS, second FAIL with seven V2a assertions. Failed
raw transport log/artifact12019004944 retained. V2a source is byte-identical
to mainffbce9d20 and predates Numeric work. Actual hosted thread ordering is
unrecorded. The failure is NOT proof that correctly contended rollback corrupts
data; its intended contention precondition was never established by a barrier.

Focused original function extracted without changing assertions. GNU build0,
normal startup0; inject ONLY SLEEP150 at holder entry -> exit1 with exactly the
same seven messages as hosted. Independent audit-harness.cjs compares those
messages and proves all11 original assertions unchanged, including the exact
assertion tail SHA cc7d340d91df4e2f3ba534cd57d1397d173ac043462fdc4659692591acf1e546.
Original source and captured outputs/statuses are retained; this controlled
schedule demonstrates the timing defect, not the precise original CI schedule.

Replace V2a's startup50ms/hold300ms guesses with empty-file readiness AFTER
successful RLOCK and release AFTER caught ROLLBACK. Marker existence is the
signal, not concurrently written contents. Finite5000 one-ms iteration budgets
contain setup failure; unchanged contention assertions reject expiry/missed
contention. New explicit readiness assertion plus normal/delayed-holder150ms/
delayed-parent500ms schedules. No timeout increase, skipped/relaxed assertion,
blanket retry, runtime callback, or production seam.

Initial handshake draft failed locally (six assertions, two per schedule),
and a full repeat mistakenly launched despite that failed focused probe
exited8 in91.05s. Its unexpected-success
cleanup AWAIT ran again after ON ERROR resumed after the failing REPLACE,
reentering the handler after its transaction was already rolled back. Diagnostic
logs show the second handler entry and No active transaction. Guard that
cleanup with !error_seen: the actual fixed function, all3 schedules, then exits0
with empty stderr. Failed draft outputs are retained, not retroactively PASS;
the first draft source was not separately snapshotted (the guard delta above
records its defect), and is not claimed byte-frozen or independently qualified.
Final Clang Werror syntax0. Completed-build final CTest five selected targets
each actually run twice, all PASS, exit0 in182.06s: full control-flow twice,
both Numeric assignment publics, real helper and actual-name selector twice.
Source/Numeric frozen input manifests0 and independent assertion audit0;
no source/expected/timeout weakening between final builds/execution.

Numeric prewire input/final production+driver manifests remain exact, including
the46-row native table/18-row Integer table and all production/helper bytes.
Earlier full28/28 evidence remains for those unchanged inputs, not falsely
relabeled as execution of this changed control-flow harness. Fresh exact-head
review, actual hosted sanitizer/new harness execution and strict merge gates
are required after pushing the repair. Old-head clean receipts cannot qualify it.

First staged whitespace check2 reported timestamped empty transport-log lines
ending with a space. Preserve exact transport and verbatim diagnostic bytes
with two exact-file Git attributes (the copied diagnostic triggered the same
check until it too was attributed). No product/test/native bytes changed.

LOW procedural delta/misuse/boundary/walkthrough/rollback in contract; existing
HZ-system-failure-01/HZ-data-corruption-01. Implementing-agent self-review and
automation, not independent-human/system/release assurance. Signal files remain
inside the existing test-owned root and are removed by its cleanup. Rollback
only this harness repair; report regression through #7202/PR7201. No parent
Numeric/ListItem closure, no new work after7201 closes: owner stop condition.
