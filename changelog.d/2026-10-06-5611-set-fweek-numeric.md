- 2026-10-06: Check SET FWEEK TO Numeric/exact-integer conversion under
  #5611/#6776: truncate into 1..3, retain negative-only aliases in VFP9 mode,
  and raise localized catchable error 46 before state/event mutation. Retain
  complete installed-VFP9 source/output, boundary/state/week-consumer tests,
  sanitizer and bidirectional requirements/documentation evidence. EPOCH,
  other settings, evaluation and non-Numeric admission remain separate.
  Correct the runtime-surface setup to use valid FWEEK 3 instead of an
  obsolete out-of-range clamp; retain session/default/restoration assertions.
