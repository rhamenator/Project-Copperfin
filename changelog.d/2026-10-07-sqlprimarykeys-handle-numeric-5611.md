- 2026-10-07: Bound SQLPRIMARYKEYS Numeric/exact-integer handles before metadata callbacks
  under #5611/#6776; preserve the owner-directed extension in both numeric modes,
  reject nonfinite/oversized values with localized catchable diagnostics, and retain
  native presence, direct/synthetic cursor-state-event and sanitizer evidence.
