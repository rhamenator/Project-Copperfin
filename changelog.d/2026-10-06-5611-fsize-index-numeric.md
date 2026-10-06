- 2026-10-06: Record FIELD PR #6989 merge and cleanup, then replace FSIZE Numeric/exact-integer index conversion with installed-VFP9 catchable error 11 admission in both numeric modes; Character name lookup and routing stay unchanged.
  Retain the complete cursor-only native fixture and independent direct/PRG, local-table and SQL-cursor regressions; other type/arity (#6990) and SET COMPATIBLE/file-size semantics (#6014) remain separate.
  Old dispatch fails 127 assertions; the selected Numeric-only contract removes conversion entirely before field lookup.
  All five focused suites pass normally and under ASan/UBSan/float-cast-overflow without diagnostics; all four documentation/signoff/channel contracts pass.
