- 2026-10-04: Hardened runtime array declaration, resizing, restoration,
  native-class initialization, and element read/growth paths with centralized
  finite/integral dimension validation, checked element-count and row-major
  offset arithmetic, a bounded host-allocation ceiling, failure-atomic
  metadata/content updates (including atomic `RESTORE FROM` validation and
  nested native-object rollback), preserved left-to-right visibility between
  targets of one `DIMENSION` statement, and localized catchable error 230
  diagnostics (#5594).
