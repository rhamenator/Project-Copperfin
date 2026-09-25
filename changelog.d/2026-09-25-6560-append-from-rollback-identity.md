- 2026-09-25: A local `APPEND FROM ... FOR` whose FOR callback reshaped the
  target (for example deleted the candidate and ran `PACK`) could truncate a
  pre-existing row that happened to be byte-identical to the rejected
  candidate (#6560). The rollback now truncates only when the target's
  record count, a new per-table row-set serial, the table file's size and
  modification time, and the candidate record's bytes are all unchanged, and
  otherwise fails catchably without removing any row. This also closes a
  follow-up gap where a callback overwrote the table file directly (`COPY
  FILE`, `STRTOFILE`, `FWRITE`) instead of through a runtime DBF writer,
  which the serial alone did not detect; the row-set key is now
  case-insensitive on Windows so another spelling of the same path still
  bumps it.
  `RQ-CF-PRG-APPEND-FROM-FOR-001`.
