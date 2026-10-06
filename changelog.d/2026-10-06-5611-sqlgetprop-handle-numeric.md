- 2026-10-06: Record SELECT PR #6995 merge and recover SQLGETPROP Numeric/exact-integer handle conversion under #5611/#6776.
  Retain connection-free native observations and scoped regressions; other SQL callers, property values, backend/connection semantics and non-Numeric admission remain separate.
  Check finite signed-int32 conversion, preserve negative-only VFP9 aliases, and raise localized catchable 1466 before query callbacks; 43 old-dispatch failures now pass with all four normal and sanitizer suites.
  Retain 48 native calls, 134 direct boundaries and 119 PRG rows with completed reproduction/VR/DQ/DV evidence; report the separate default/invalid-query and type-admission gap as #6996 against current main, without expanding implementation scope.
  Keep rejected Numeric handle text on the existing safe decimal formatter, with two failing-before diagnostic regressions; report the general shared-formatter gap as #6997 without changing other callers.
