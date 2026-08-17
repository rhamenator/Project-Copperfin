# OLDVAL Traceability

## Scope

This record covers the bounded recovery of `OLDVAL()` for a local current record
that has a retained original image under row or table buffering. It is based on
the mounted Visual FoxPro 9 help topic
`html/fa81be19-03be-4e02-8af1-74f8c521b766.htm` (`OLDVAL( ) Function`).

## Requirement and verification

| Requirement / derived constraint | Verification | Hazard |
| --- | --- | --- |
| `RQ-CF-PRG-007`: a character expression is evaluated against a retained original local buffered record, with optional alias/work-area selection and its natural expression result type | `test_prg_engine_runtime_surface_functions_buffering` proves character and numeric field results, an expression result, and work-area selection | `HZ-data-corruption-01` |
| `DQ-prg-oldval-001`: the mutable current record must never be substituted for the retained original record while evaluating `OLDVAL()` | The focused regression changes both fields before querying and proves original rather than current values | `HZ-data-corruption-01` |
| `DQ-prg-oldval-002`: retained original data must be unavailable after revert or commit | The focused regression checks empty results after `TABLEREVERT()` and `TABLEUPDATE()` | `HZ-data-corruption-01` |
| `DQ-prg-oldval-003`: nested expression side effects must not invalidate the original-record evaluation image | The focused regression performs nested `TABLEREVERT()` and `TABLEUPDATE()` before reading the overridden field and proves the retained original remains readable | `HZ-data-corruption-01` |

## Boundary

This slice does not claim remote cursor support, validation-rule operation without
buffering, unmodified or appended-record behavior, default-field effects,
trigger/rule firing, or conflict resolution. Those behaviors need separate
permitted evidence and verification.
