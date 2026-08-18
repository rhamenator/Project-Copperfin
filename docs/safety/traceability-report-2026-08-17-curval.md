# CURVAL Traceability

## Scope

This record covers the bounded recovery of `CURVAL()` for a local current DBF
record. It is based on the mounted Visual FoxPro 9 help topic
`html/d4dc65e0-a039-40ac-8df4-508a60a77228.htm` (`CURVAL( ) Function`).

## Requirement and verification

| Requirement / derived constraint | Verification | Hazard |
| --- | --- | --- |
| `RQ-CF-PRG-008`: a character expression is evaluated against the current persisted local record, with optional alias/work-area selection and its natural expression result type | `test_prg_engine_runtime_surface_functions_buffering` proves character and numeric values, an expression result, and work-area selection | `HZ-data-corruption-01` |
| `DQ-prg-curval-001`: a pending optimistic buffer must never be substituted for the persisted record while evaluating `CURVAL()` | The focused regression changes both fields before querying and proves the pre-commit disk values | `HZ-data-corruption-01` |
| `DQ-prg-curval-002`: a successful commit must make the newly persisted value available to `CURVAL()` | The focused regression checks the changed value after `TABLEUPDATE()` | `HZ-data-corruption-01` |
| `DQ-prg-curval-003`: verified-byte mode must retain a session-owned post-commit record image rather than reread mutable source bytes or stale admission bytes | The focused strict-mode regression commits a buffered value and proves `CURVAL()` reports the committed session image | `HZ-data-corruption-01`; `HZ-package-trust-01` |

## Boundary

This slice does not claim remote cursor support, view refresh behavior, EOF or
error compatibility parity, locking/concurrency semantics, or non-local data
sources. Those behaviors need separate permitted evidence and verification.
