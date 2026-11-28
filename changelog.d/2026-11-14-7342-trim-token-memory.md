- 2026-11-14: Avoid duplicating already evaluated Character parse tokens in
  `LTRIM()`, `RTRIM()`, and `ALLTRIM()`, preventing large legal calls from
  exhausting bounded runtime memory while preserving whole-token matching
  semantics (#7342).
