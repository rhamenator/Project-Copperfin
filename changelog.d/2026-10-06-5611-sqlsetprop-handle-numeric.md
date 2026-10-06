- 2026-10-06: Record SQLGETPROP PR #6998 merge and recover SQLSETPROP Numeric/exact-integer first-argument handle conversion under #5611/#6776.
  Retain 48 connection-free native setter observations, 134 direct boundaries and 118 PRG rows; Boolean-property resets independently check return values and mutation.
  Check finite signed-int32 admission, preserve negative-only explicit VFP9 aliases, and raise localized catchable 1466 before setter callbacks using safe original-Numeric diagnostic text.
  Retain completed normal/sanitizer reproduction and VR/DQ/DV evidence; the 47 old-dispatch failures now pass, while property-value conversion, other SQL callers, backend/session/connection behavior and native setter/type parity remain separate.
  Report the encountered default/invalid-setter and non-Numeric admission gap as #6999 against exact main e3bdb6458 without admitting new implementation scope; leave the general shared-formatter gap #6997 unchanged.
