- 2026-09-25: A `SPAWN`ed worker left unawaited (no `AWAIT`/`CANCEL`) used
  to keep running in the background after its `PrgRuntimeSession` was
  destroyed, with no host-visible handle left to observe or stop it
  (#6183). Destroying the root session now requests cooperative
  cancellation for every task across every data session and waits up to
  2 seconds for each to actually stop. Scope note: cancellation is only
  observed at existing checkpoints (`SLEEP`, lock-retry waits) -- a tight
  loop with none (a bare `DOEVENTS` spin) is not covered by this change.
  `RQ-CF-PRG-SESSION-DESTROY-SPAWN-001`.
