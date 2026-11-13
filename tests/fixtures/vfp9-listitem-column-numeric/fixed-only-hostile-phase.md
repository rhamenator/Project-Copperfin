# ListItem supplied-column fixed-only hostile gate

RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 /
VR-5611-LISTITEM-COLUMN-NUMERIC-001/002.

This gate was authored after both complete original106 compiler suites and
before caller migration. It is deliberately excluded from CTest and must never
run against the legacy caller. Each mode/session shard performs 56 reads across
ListBox and ComboBox, stable IDs 11/22, six extreme/nonfinite Numeric sources,
seven exact64 boundary/low32 sources, and one derived NaN source.

The rejection and exact64 expectations come from the admitted #5611/#6776
checked-conversion parent policy. They are not claims that installed VFP9 can
spell every source. The runner refuses to execute until both fixed compiler
suites and their complete audit have status zero, then verifies empty stderr,
the exact public counter, source hashes, and each compiler binary hash.
