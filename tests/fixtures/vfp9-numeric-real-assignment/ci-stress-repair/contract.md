# Pre-code CI contention synchronization contract

RQ-CF-CI-V2A-SYNC-001 / VR-CI-V2A-SYNC-001, derived from the owner's direct
actionable-CI repair instruction and HZ-system-failure-01/HZ-data-corruption-01,
not from Numeric implementation as a requirement source. No production change.
Hosted38836793953/job116767618283 fails seven original V2a assertions in its
second repetition. Original standalone0 -> controlled holder delay150ms exits1
with the exact seven hosted messages. Actual hosted scheduling is unrecorded;
the demonstrated gap is a missing synchronization precondition, not a proven
production locking/data-corruption defect. Original source matches mainffbce9d20.

Replace this test's fixed startup50ms/hold300ms assumptions with test-owned
filesystem readiness/release signals. Publish readiness only after RLOCK
succeeds; parent starts the transaction only after readiness. Holder waits for
explicit release after the parent's caught ROLLBACK. Both waits have finite
5000 one-ms iteration budgets; these are failure containment, not scheduling
proof or increased CTest timeout. Unexpected success/setup failure releases
the holder too. Keep all old lock-timeout/catch/transaction/record/journal
assertions byte-identical; add an explicit ready assertion and exercise normal,
150ms delayed-holder,500ms delayed-parent schedules. Do not skip or relax tests.

The markers are empty files within the existing exclusively test-owned root,
not shared variables or product seams. Readiness existence is sufficient:
creation occurs after successful lock acquisition; no content-read/write race.
No PRG observer, host callback or runtime/source change is needed. Existing
failure assertions still reject missed contention (including budget expiry).
Verify the focused function and full control-flow binary, unchanged Numeric
expected hashes/tests, and require fresh exact-head review/actual hosted gates.

LOW DQ-DV-CI-V2A-SYNC-001 procedural delta: timing assumption -> bounded
readiness/hold-release precondition -> same original invariants. Misuse:
calling a timer a barrier, mistaking this test failure for runtime corruption,
removing real contention assertions, or ignoring budget expiry. Walkthrough:
controlled original failure then all three synchronized schedules pass with
old assertions unchanged. Implementing-agent self-review plus automation,
not independent-human/system/release qualification. Rollback only the test
handshake; notify owner in tracking issue/PR on a regression. Numeric contract,
payloads, switch policy and allocation/conversion limits stay unchanged.
