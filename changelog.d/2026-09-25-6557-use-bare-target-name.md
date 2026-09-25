- 2026-09-25: `USE <name>` treated a bare, unquoted table name as an
  expression instead of a literal file name (#6557), so `USE people` opened
  whatever `people` evaluated to (typically nothing) instead of
  `people.dbf`, and `USE people.dbf` was read as an object property
  access. Only a parenthesized `USE (expr)`, a macro (`&name`), or a
  quoted literal are now evaluated; a bare token opens the file/table
  literally, matching VFP9. `RQ-CF-PRG-USE-BARE-TARGET-001`.
