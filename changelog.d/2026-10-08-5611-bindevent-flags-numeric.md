- 2026-10-08: BINDEVENT object-method and routine-extension flags now use
  checked Numeric/exact-integer conversion under #5611/#6776: truncate,
  admit nonnegative signed32 flags by default, and retain defined low32
  aliases only in explicit VFP9 mode. Rejected flags raise localized 11
  before binding replacement, ordinal consumption or success events.
  Native observations and boundary/state/timing tests retain the supported
  routine extension; separate type/Currency gap #7070 remains unfinished.
