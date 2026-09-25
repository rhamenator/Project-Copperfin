- 2026-09-25: A `BEGIN TRANSACTION` left open when a program returns
  normally (no `END TRANSACTION`/`ROLLBACK`), across every data session,
  used to leave its modified bytes live on disk once the runtime session
  was destroyed, with rollback happening only if some later, unrelated
  session happened to reuse the same temp directory (#6263). Destroying
  a session now rolls back every still-open transaction itself, and no
  longer lets an exception escape the destructor.
  `RQ-CF-PRG-SESSION-DESTROY-TRANSACTION-001`.
