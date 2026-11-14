- 2026-11-14: Bound `LIKE()` and `LIKEC()` wildcard matching work so long
  nonmatching suffixes no longer repeatedly rescan short PRG inputs, while
  preserving case-sensitive `*` and `?` results and honoring runtime
  cancellation (#7339).
