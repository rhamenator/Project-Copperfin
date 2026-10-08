- 2026-10-08: Check ERROR Numeric/exact-integer conversion under #5611/#6776:
  truncate into the nonnegative signed32 domain, retain negative low32 aliases
  only in explicit VFP9 mode, and raise localized1941 for invalid conversions
  before parameter evaluation or for converted0 after it. Preserve checked
  Currency rounding, retain native/baseline/fixed traceability and both-mode
  metadata/state/locale regressions; full catalog/type gap #7079 stays separate.
