- 2026-10-04: Fixed `BINTOC()` and `CTOBIN()` to emit and consume VFP-compatible signed-
  integer, sortable Numeric/Double, native float/double, reversed, raw-sign,
  and Currency byte representations. Invalid widths, lengths, flag grammars,
  value ranges, nonfinite selectors, and unsupported types now fail with a
  localized catchable error before allocation or conversion (#5766).
