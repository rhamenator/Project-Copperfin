# PRG Task Supervision Contract

Copperfin keeps FP/VFP source in control of asynchronous work without placing
foreign-language source inside `.prg` files. The first implemented control
plane extends the existing `SPAWN`/`AWAIT` task registry with four
Copperfin-specific, nonblocking functions:

| Function | Result |
| --- | --- |
| `CFTASKSTATUS(handle)` | Invariant status text: `running`, `cancel-requested`, a terminal runtime pause reason such as `completed` or `error`, or `unknown`. |
| `CFTASKCANCEL(handle)` | `.T.` only when cooperative cancellation was accepted for a live task; otherwise `.F.`. |
| `CFTASKRESULT(handle)` | The retained PRG `RETURN` value after completion; `EMPTY` while running, for an unknown handle, or when no value exists. |
| `CFTASKOUTPUT(handle)` | Completed task `?`/`??` output joined in emission order with line-feed separators; empty text when completed without print output, and `EMPTY` while running or unknown. |

These names are Copperfin extensions rather than claims of VFP9 syntax. Their
machine status values are invariant and are not localized.

## Lifetime And Isolation

- `SPAWN` creates a separate runtime instance and returns an exact-integer task
  handle scoped to the current data session.
- Handles increase monotonically and are never recycled during the owning
  runtime session. Allocation fails closed after the largest integer exactly
  representable by a VFP numeric value (`2^53 - 1`), so a stale handle cannot
  address a later task.
- Status, result, and output reads poll the shared completion future with a
  zero-duration wait. They do not block the PRG/runtime thread and do not
  consume the task.
- Cancellation sets only the child's cooperative cancellation token. It does
  not enter or mutate the child's evaluator state directly.
- `AWAIT` remains the explicit blocking join. It attempts its optional `TO`
  output assignment *before* merging child events or erasing the task, so
  consumption is atomic from the language program's perspective: a catchable
  assignment failure (a typo, a released object, an invalid property) leaves
  the task exactly as it was before the `AWAIT` -- still registered, its
  events not yet merged into the parent -- rather than partially consuming
  it. A later retry `AWAIT` on the same handle (with a valid target, or none)
  re-attempts the same sequence and completes consumption exactly once: it
  merges child events, assigns its completion flag, and erases the task.
  Later supervision calls on that handle then return the documented unknown
  values. Completion polling (`CFTASKSTATUS`/`CFTASKRESULT`/`CFTASKOUTPUT`) is
  unaffected by a failed `AWAIT` assignment and continues to report the
  task's retained terminal state until an `AWAIT` actually consumes it.
- A runtime created by `SPAWN` cannot execute `READ EVENTS`: it raises a
  catchable, source-located error and terminates that task's execution
  immediately instead of pausing. A spawned child's `run()` is invoked
  exactly once by the `std::async` closure that hosts it; there is no
  mechanism to resume it, route a host/COM event into it, or execute `CLEAR
  EVENTS` against it afterward, so a paused-in-`READ EVENTS` child has no
  path back to completion. This is a deliberate rejection of an unsupported
  construct, not a design for live spawned event-loop workers -- `AWAIT`
  observes the resulting fault exactly like any other task-side error
  (`CFTASKSTATUS` reports the terminal state, the task is fully consumed).
  `READ EVENTS` executed by the root/interactive session is unaffected.
  `ACTIVATE MENU`/`POPUP`/`WINDOW` share the same one-shot-publish mechanism
  in a spawned task but are not currently rejected; avoid them in `SPAWN`
  targets.
- A child owns its runtime state. The parent observes only the immutable
  `RuntimePauseState` published through the future after the child returns.
- Completion publication is serialized by a mutex owned by that task. Multiple
  sibling supervisors may poll one handle concurrently, but exactly one copies
  the ready future into the retained result; every successful observer crosses
  the same synchronization boundary before reading that immutable record.
  Unrelated tasks do not share this publication mutex.

`runtime.task.cancel_requested` records an accepted cancellation request with
the task handle. Existing `runtime.task.spawn`, `runtime.task.await`, and child
completion/error events retain their prior meanings.

## Output Boundary

Current task output means the child's retained `runtime.print` events. Output
is exposed only after completion, when the completion record is immutable.
There is no live-streaming callback into runtime state. The portable artifact
invocation adapter now captures bounded standard output and error outside this
task registry. A future PRG-facing route must publish admitted results through
this same completion boundary rather than entering the PRG evaluator from a
worker thread.

## Compatibility And Nonclaims

Existing `SPAWN` and `AWAIT` source continues to behave as before, with two
narrow exceptions documented above under Lifetime And Isolation: `AWAIT`'s
consumption is now atomic with respect to a failing optional `TO` assignment
(previously such a failure left the task partially, incoherently consumed),
and a spawned task executing `READ EVENTS` now raises a catchable error
immediately instead of silently pausing forever with no way to resume. The new
functions do not execute .NET, Python, R, or another external language; choose,
authorize, or hash an artifact; route a polyglot request; or weaken the
separate policy, audit, bounded-process, and response-admission requirements.
They provide the PRG-owned supervision seam that a future PRG-facing artifact
route must reuse without inventing a second task lifecycle. The portable
invocation adapter exists, but no PRG route submits it through this registry
yet.

Focused coverage in `test_prg_engine_control_flow` proves nonblocking running
observation, retained terminal status/result/ordered output, cancellation,
unknown-handle behavior, and unchanged `AWAIT` consumption. The local GCC
control-flow suite passes at implementation head `92f96dd64` in 95.20 seconds.
The same suite passes under Clang 21 ASan/UBSan in 345.26 seconds with no
sanitizer finding, and the package
document-install contract passes. Focused static analysis reports no finding
in this slice; its only emitted advisories are pre-existing unrelated struct
padding observations in the monolithic engine translation unit.

At exact implementation-and-local-evidence head `09c1b1046`, Linux Native run
`31307387144` passes `326/326`, macOS Native run `31307387146` passes
`326/326` plus both SET POINT targets under four locales (`8/8`), and Windows
Native run `31307387195` passes `325/325`. Every platform executes
`test_prg_engine_control_flow` successfully, and all eight protected PR checks
pass at that head.

A later concurrency audit reproduced an unsynchronized same-handle completion
publication under ThreadSanitizer. The corrective regression spawns one target
and two sibling watchers that simultaneously poll it, then proves retained
result/output integrity and unchanged `AWAIT` consumption. The focused GCC
control-flow target passes in 6.94 seconds. The complete target passes under
Clang 21 ThreadSanitizer in 382.70 seconds with no race or deadlock finding.
This correction changes synchronization only; all public task-supervision
semantics above remain unchanged.

At exact corrective candidate head `e93686326`, Linux Native run
`31329773481` and macOS Native run `31329773480` pass `331/331`, macOS also
passes both SET POINT targets under four locales (`8/8`), and Windows Native
run `31329773507` passes `330/330`. Every platform executes
`test_prg_engine_control_flow` successfully. Candidate protected checks
conclude seven successes plus the non-failing neutral Socket project report.

## State-Sequence Coverage (#6495)

`tests/test_prg_engine_control_flow_task_supervision_state_sequences.cpp`
adds four short sequences answering #6495's coverage gap that the `stress`
Cloud Defect Hunt lane only repeats fixed whole-test sequences rather than
constructing new interleavings. Getting two spawned sides into position
still uses a fixed, generous wait rather than a true injected
yield-point/barrier scheduler -- no such seam exists anywhere in the PRG
engine's SPAWN/lock/transaction subsystem yet, and adding one was judged out
of scope for this coverage-only slice. What these sequences do not do is
trust that wait blindly: three of the four use real record/table-lock
contention (with `SET REPROCESS TO n` widening `pause_for_lock_retry`'s real
linear backoff once contention has actually started), and every sequence
asserts on runtime-emitted evidence -- a `runtime.lock_retry`,
`runtime.lock_timeout`, or `runtime.task.cancelled` event, or an equivalent
state check -- that the intended contention or cancellation genuinely
occurred, not merely that the script ran to completion. A review round on an
earlier draft correctly found one sequence (the fourth, below) where that
evidence check was missing and a lost race could have silently passed
without exercising the crossing at all; it now requires a child-specific
`runtime.lock_retry` event before accepting the sequence. If scheduling
jitter ever causes two sides to miss each other despite the generous
margins, the affected assertion fails loudly instead of a false pass. They
compile into `test_prg_engine_control_flow`, so hosted `stress`-lane
repetitions (`scripts/run-cloud-validation.py`) exercise them automatically.

Two sequences extend the SPAWN/AWAIT/cancellation/teardown crossing:
`test_state_sequence_await_retry_after_cancellation_reuses_still_registered_task`
generalizes #6456's erase-only-on-success AWAIT ordering to a task that ends
via cancellation rather than natural completion, proving it too remains
registered and exactly-once-consumable after a failed output assignment.
`test_state_sequence_cancellation_during_widened_lock_retry_leaves_no_residual_lock_ownership`
proves a task cancelled while genuinely blocked mid explicit `FLOCK()` retry
leaves no residual shared lock-owner entry, extending #6453's
release-scoping invariant to the cancelled-while-waiting path -- and, in
doing so, surfaced a real, newly filed, and *not yet fixed* defect (#6499):
cancellation observed inside an explicit `FLOCK()`/`RLOCK()` retry loop is
silently swallowed by `pause_for_lock_retry` (the cancellation bookkeeping
runs and a `runtime.task.cancelled` event still fires, but the function
returns `false` exactly like an ordinary retry-budget timeout, so the calling
script's `RETURN FLOCK()` completes normally with `.F.` instead of halting).
This differs from the correctly-halting per-statement dispatch loop and
`SLEEP` cancellation checkpoints. The test documents today's actual behavior
with an inline citation rather than asserting the not-yet-fixed halt
semantics, per #6495's direction to file focused defects rather than hide or
paper over them.

Two more extend the cursor-lock/transaction-state/caught-error/retry
crossing: `test_state_sequence_retry_after_caught_rollback_succeeds_cleanly`
proves a transaction that fails on a genuinely record-lock-contended
`REPLACE`, gets caught and rolled back, leaves no journal/transaction-level
residue before an immediately following retry commits cleanly.
`test_state_sequence_record_lock_handoff_across_rollback_and_child_quit_leaves_no_residue`
proves an explicit `RLOCK()` released after a caught error extends #6453's
QUIT-releases-only-its-own-locks invariant from table-level `FLOCK()` to
record-level `RLOCK()` specifically, deliberately releasing the parent's lock
via an explicit `UNLOCK` after `ROLLBACK` rather than assuming an unverified
ROLLBACK-releases-explicit-locks parity claim.

Local Linux Debug: the full `test_prg_engine_control_flow` binary (all
existing coverage plus these four sequences) passed cleanly across 5
consecutive full runs (~110s each, no flakiness observed).
