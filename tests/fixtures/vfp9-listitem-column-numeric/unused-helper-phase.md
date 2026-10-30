# ListItem supplied-column unused-helper foundation

RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR-5611-LISTITEM-COLUMN-NUMERIC-001.

This bounded implementation step adds `checked_listitem_column_numeric_argument`
and a direct deterministic fixture, but does not wire any ListBox/ComboBox
caller. The helper admits only exact integral payloads whose conversion to a
positive selector is already unambiguous. It preserves explicit VFP9 signed-low32 aliases and
the default checked range, while rejecting NULL, unsupported types, nonfinite
values and all fractional Numeric payloads.

Rejecting all fractions is deliberate fail-closed staging, not a claim that VFP
rejects them. Retained native evidence proves that ordinary `2.5` selects column
2 and that the same near-one binary64 payload selects column 1 with decimal
metadata 2 but raises error 31 with decimal metadata 14. The complete
metadata-dependent conversion formula is still unrecovered. The helper and test
therefore freeze those three cases as deferred rather than inventing a tolerance
or silently applying binary truncation. No current runtime route calls the
helper, so shipped behavior is unchanged.

The direct fixture covers exact Numeric, signed/unsigned exact64 extensions,
native negative aliases, positive ceiling, the same-bits metadata pair,
ordinary fractional deferral, NULL/type/nonfinite containment and isolation
registration. GNU 15.2.0 and Clang 21.1.8 each configure and build the
registered target, and the isolated direct test passes under both compilers.
Existing unrelated build warnings remain visible. Both complete original
runtime suites and public caller verification remain future pre-wire gates.

This is a partial foundation only. It does not close #5611, #6776 or #7189,
does not claim the complete conversion formula, and does not change the status
or severity of HZ-runtime-crash-01, HZ-data-corruption-01 or HZ-doc-command-01.
