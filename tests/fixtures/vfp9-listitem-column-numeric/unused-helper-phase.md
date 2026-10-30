# ListItem supplied-column unused-helper foundation

RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR-5611-LISTITEM-COLUMN-NUMERIC-001.

This bounded implementation step adds `checked_listitem_column_numeric_argument`
and a direct deterministic fixture, but does not wire any ListBox/ComboBox
caller. The helper preserves explicit VFP9 signed-low32 aliases and the default
checked range. For a fractional Real Numeric in VFP9 mode, it decimal-quantizes
the value at its associated decimal-place count plus two and then projects the
integer part. COPPERFIN mode retains direct truncation. NULL, unsupported types,
nonfinite values, out-of-range values and malformed/missing fractional metadata
remain checked failures.

The retained repeated factorial/settings evidence recovers that formula without
inventing a binary tolerance: rounding to attached decimals plus two, followed
by truncation, explains all 748 factorial and 160 settings rows in each of the
identical first/repeat captures (1,816 invocations total). This includes ordinary
`2.5`, the same-bits decimal2/decimal14 near-one pair, near-two and near-nine
boundaries, and sub-one rejection. The audit continues to control out width and
current SET DECIMALS at read time. No current runtime route calls the helper, so
shipped behavior is unchanged.

The direct fixture covers exact Numeric, signed/unsigned exact64 extensions,
native negative aliases, positive ceiling, metadata-sensitive fractional
boundaries, ordinary fractions, malformed/missing metadata, checked decimal
limits, NULL/type/nonfinite containment and isolation registration. GNU 15.2.0
and Clang 21.1.8 each build the registered target, and the direct test passes
under both compilers.
Existing unrelated build warnings remain visible. Both complete original
runtime suites and public caller verification remain future pre-wire gates.

This is a partial foundation only. It does not close #5611, #6776 or #7189,
does not wire the recovered formula into public behavior, and does not change
the status or severity of HZ-runtime-crash-01, HZ-data-corruption-01 or
HZ-doc-command-01.
