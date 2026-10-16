- 2026-10-16: Normalize assigned width metadata for known Integer scalar
  memory-variable '=' assignments, without changing payloads or formatting.
  Retain native assignment recovery, original768 descriptor failures -> same
  fixed regression, and focused26/26 development verification.
  Retain the breakpoint test's native size type for Clang portability; no
  expected values, runtime behavior or verification limits changed.
  Real Numeric assignment, arithmetic precision/display, remaining producers
  and ListItem compatibility remain mandatory; no full-parity claim.
