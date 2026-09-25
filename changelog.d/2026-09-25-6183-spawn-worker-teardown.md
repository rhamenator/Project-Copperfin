- 2026-09-25: A `SPAWN`ed worker left unawaited (no `AWAIT`/`CANCEL`) used
  to keep running in the background after its `PrgRuntimeSession` was
  destroyed, with no host-visible handle left to observe or stop it
  (#6183) -- including a bare `DOEVENTS` spin loop with no `SLEEP` at all,
  the issue's own repro. Destroying the root session now requests
  cooperative cancellation for every task across every data session and
  waits up to 2 seconds for each to actually stop, reaching an existing
  general per-statement cancellation checkpoint that simply had nothing
  triggering it at destruction time before now.
  `RQ-CF-PRG-SESSION-DESTROY-SPAWN-001`.
