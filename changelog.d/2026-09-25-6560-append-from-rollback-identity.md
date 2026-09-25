- 2026-09-25: A local `APPEND FROM ... FOR` whose FOR callback reshaped the
  target (for example deleted the candidate and ran `PACK`) could truncate a
  pre-existing row that happened to be byte-identical to the rejected
  candidate (#6560). The rollback now truncates only when the target's
  record count and a new per-table row-set serial are both unchanged, and
  otherwise fails catchably without removing any row.
  `RQ-CF-PRG-APPEND-FROM-FOR-001`.
