- 2026-11-13: Reject an explicitly empty `WHERE` on `UPDATE` and `DELETE FROM`
  with catchable syntax error 10 before mutation, while preserving bare all-row
  commands and populated filters (#7254).
