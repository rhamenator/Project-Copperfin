- 2026-10-08: CURSORSETPROP BUFFERING Numeric/exact-integer modes now
  truncate with checked 1..5 admission under #5611/#6776, retaining defined
  low32 aliases only in explicit VFP9 mode. Rejected Numeric values raise
  localized 1469 before mode/pending-record mutation, with safe original
  operand diagnostics. Native fixture and guarded both-mode/alias tests
  preserve state and other coercions; separate type gap #7077 remains open.
