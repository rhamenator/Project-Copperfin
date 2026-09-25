- 2026-09-25: A `SPAWN`ed worker left unawaited (no `AWAIT`/`CANCEL`) used
  to keep running in the background after its `PrgRuntimeSession` was
  destroyed, with no host-visible handle left to observe or stop it
  (#6183) -- including a bare `DOEVENTS` spin loop with no `SLEEP` at all,
  the issue's own repro. Destroying the root session now requests
  cooperative cancellation for every task across every data session,
  reaching an existing general per-statement cancellation checkpoint that
  simply had nothing triggering it at destruction time before now, and
  gives the whole session one shared 2-second window to stop -- not a
  fresh window per task -- looping to also catch a task registered by a
  nested `SPAWN` mid-teardown. A worker that still hasn't stopped when
  that window closes is abandoned rather than joined, so a non-cooperative
  worker can no longer block the destructor indefinitely despite the
  advertised bound.
  `RQ-CF-PRG-SESSION-DESTROY-SPAWN-001`.
