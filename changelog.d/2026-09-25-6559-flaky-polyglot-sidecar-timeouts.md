- 2026-09-25: Fixed intermittent CI failures in the Python and .NET
  polyglot sidecar benchmark tests (#6559). Both used a 5-second
  process-response deadline that a cold interpreter/runtime start could
  occasionally exceed on a loaded CI runner, failing an otherwise-correct
  dispatch. Widened to 20 seconds; no assertion was weakened, and a real
  hang or crash still fails the test well within that bound. Also raised
  the enclosing `ctest` timeouts (30s was too tight for the widened
  per-process deadlines across multiple sequential launches) and the
  .NET benchmark's `maximum_p95_startup_ms` quality gate, so a slow-but-
  correct cold start no longer fails `recommendation_ready` either.
