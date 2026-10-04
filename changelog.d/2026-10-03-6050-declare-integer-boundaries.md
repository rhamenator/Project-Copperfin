- 2026-10-03: Native and managed `DECLARE` INTEGER/LONG arguments now use a
  defined VFP9-compatible low-32-bit conversion, while INTEGER64 and unsafe
  nonfinite or out-of-range values are checked and rejected before ABI entry.
