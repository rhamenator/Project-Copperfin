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
- `AWAIT` remains the explicit blocking join. It still merges child events,
  assigns its completion flag, and erases the task. Later supervision calls on
  that handle therefore return the documented unknown values.
- A child owns its runtime state. The parent observes only the immutable
  `RuntimePauseState` published through the future after the child returns.

`runtime.task.cancel_requested` records an accepted cancellation request with
the task handle. Existing `runtime.task.spawn`, `runtime.task.await`, and child
completion/error events retain their prior meanings.

## Output Boundary

Current task output means the child's retained `runtime.print` events. Output
is exposed only after completion, when the completion record is immutable.
There is no live-streaming callback into runtime state. A future external
artifact adapter may capture bounded standard output and error, but it must
publish those bytes through the same completion boundary rather than entering
the PRG evaluator from a worker thread.

## Compatibility And Nonclaims

Existing `SPAWN` and `AWAIT` source continues to behave as before. The new
functions do not execute .NET, Python, R, or another external language; choose,
authorize, or hash an artifact; route a polyglot request; or weaken the
separate policy, audit, bounded-process, and response-admission requirements.
They provide the PRG-owned supervision seam that a later admitted adapter can
use without inventing a second task lifecycle.

Focused coverage in `test_prg_engine_control_flow` proves nonblocking running
observation, retained terminal status/result/ordered output, cancellation,
unknown-handle behavior, and unchanged `AWAIT` consumption. The local GCC
control-flow suite passes in 95.20 seconds. The same suite passes under Clang
21 ASan/UBSan in 345.26 seconds with no sanitizer finding, and the package
document-install contract passes. Focused static analysis reports no finding
in this slice; its only emitted advisories are pre-existing unrelated struct
padding observations in the monolithic engine translation unit.
