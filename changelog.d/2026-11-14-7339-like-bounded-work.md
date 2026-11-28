- 2026-11-14: Bound `LIKE()` and `LIKEC()` wildcard matching work so long
  nonmatching suffixes no longer repeatedly rescan short PRG inputs, including
  text containing a literal `*`, while preserving case-sensitive wildcard
  results, historical aligned-star precedence, and runtime cancellation
  (#7339).
