- 2026-09-25: `BINDEVENT()` delegate timing now matches VFP9 (#6547). Flag
  0 (the default) runs the delegate before the event code and flag 1 runs it
  after, as the VFP 9 help and a real VFP 9.0 SP2 probe show; the mapping had
  been inverted since #3688. This covers method dispatch, `RAISEEVENT()`,
  property reads, and (added on review, which caught the gap) property
  writes. Existing tests keep their intended timing with corrected flags.
  `RQ-CF-PRG-BINDEVENT-FLAGS-001`.
