# Installed VFP9 SQLTABLES handle observations

Recovered 2026-10-07 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
sqltables.prg is the complete clean-room source. sqltables.out retains VERSION,
48 explicit SQLTABLES(handle, 'TABLE', 'native_tables') calls, absent metadata
cursor and unchanged session 1. Four fresh serial runs agree on all 51 lines.
The preliminary 50-line source without the cursor-state row was exploratory,
not the retained fixture. No external SQL connection, ODBC backend, VM,
installer or system/calendar change was used.

## Requirement and recovery boundary

RQ-CF-PRG-SQLTABLES-HANDLE-NUMERIC-001 derives from owner-admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Select only SQLTABLES's first-argument unchecked llround/int dispatch.
Numeric/exact integers truncate toward zero into a finite signed-int32 domain
in both modes. NaN/infinite/oversized operands reject with localized catchable
1466 before metadata callbacks, cursor materialization, state or success events.
Exact int64/uint64 stay exact without double rounding. Other existing coercions
keep checked half-away rounding; native type admission remains outside scope.
Safe original Numeric diagnostic formatting avoids unchanged gap #6997.

This is derived checked policy, **not recovered native connected-index parity**.
All 41 Numeric calls raise 1466, including zero, fractions and both-sign low-32
candidates. Identical absent errors cannot reveal converted indices or prove
SQLGETPROP aliases. Neither mode invents those aliases. Future compatibility
work must retain a native fixture that distinguishes valid handles.

Seven Currency/Logical/NULL/Character controls raise 11 in native VFP9.
Current callback/type behavior differs; #7032 records this separate parity and
connected-index recovery gap against exact main
ad127c62acc20bb97445efbe5380fbaf39117ef1. This slice preserves those results as
controls, not native parity claims. Metadata/backend results, cursor allocation,
absent/default-handle semantics, type/arity and other SQL callers stay separate.
No argument-sized allocation or loop is introduced.

## Reproduction and identities

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqltables-handle-numeric-observation/sqltables.prg
sha256sum tests/fixtures/vfp9-sqltables-handle-numeric-observation/sqltables.prg tests/fixtures/vfp9-sqltables-handle-numeric-observation/sqltables.out
```

Source SHA-256: 5366fcabf42f40268e6c3a6d7217582708e4d9c44297327779560e701f67b5a9.
Output SHA-256: 39794122c2f49b4ee0164fc6915dc7bfc6f3bf5ce46f4d6fe375ccfb8e9a0e73.
VR-5611-SQLTABLES-NATIVE-001: four fresh serial runs match all 51 lines.
Generated FXP files are ignored scratch, not inspected proprietary binaries.

## Verification architecture

134 direct calls use the checked expectation column independently in both
modes, with adjacent int32 doubles, NaN/infinities, exact int64/uint64 beyond
2^53 and extrema, and preserved coercions. The native error-only fixture does
not supply unobserved compatibility alias expectations.

104 fresh synthetic-session cases contain 320 PRG rows. Each creates only
in-process handle 1, a local guard cursor with one unchanged record/field,
and a dirty transaction with cancellation. Independent expectations verify
status/error, connection, dirty/cancel/action flags, metadata cursor presence,
LastCursorAlias, two TABLE rows/five fields/first CUSTOMERS result, preserved
guard payload/shape and successful sql.tables/sql.cursor events. Eight signed
huge/infinite message cases also verify original Numeric diagnostics and state.
All cases reset the mode, close both cursors and disconnect. Synthetic catalog
shape is a preserved fixture control, not native backend parity. Tests run
serially across builds because neighbors share scratch; never run multiple
build processes within one build directory.

```sh
cmake -S . -B /home/rich/temp/copperfin-sqltables-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqltables-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqltables-handle-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-sqltables-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-sqltables-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqltables-handle-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
```

## Completed baseline and verification evidence

Unchanged-dispatch baselines: GCC four suites 14.01s (Numeric 12.29s),
Clang sanitizer four suites 61.56s (Numeric 54.62s). Both fail exactly 66
SQLTABLES assertions; all three neighbors pass. Setup, cleanup and preserved
coercion controls pass. Fractional/wrapped candidates expose erroneous cursor
materialization or omission and successful events; oversized arguments return
the old absent-handle status without catchable rejection. Baseline sanitizers
emit no diagnostics; llround library behavior is not claimed sanitizer-detected.

VR-5611-SQLTABLES-GCC-001: final GCC 15.2 Debug passes the same four suites
4/4 in 13.71s (Numeric 12.02s). All 134 direct checks and 104 fresh synthetic
cases/320 PRG rows pass without weakening the failing-baseline expectations.

VR-5611-SQLTABLES-SANITIZER-001: final Clang 21.1.8 ASan/UBSan/
float-cast-overflow with -fno-sanitize-recover=all and default leak detection
passes 4/4 in 62.27s (Numeric 55.36s), without diagnostics. Runtime tests ran
serially across builds. Native source/output hashes remain unchanged.

VR-5611-SQLTABLES-CONTRACTS-001: locale-install, contributor-signoff,
changelog assembler/fragment validation, agent-channel and native-test-isolation
contracts pass 6/6 in 5.19s; all 155 fragments validate.

## Documentation requirement and misuse

DQ-5611-SQLTABLES-CONVERSION-001 requires explicit derived/native boundaries,
finite-domain pre-metadata rejection, safe diagnostics, serial verification
and rollback guidance. Procedural delta: add checked first-argument conversion
before SQLTABLES; retain callback, metadata/cursor implementation, type/coercion,
arity and other SQL behavior.

Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01.
Unchecked or speculated aliases can read metadata from the wrong connection
and unexpectedly materialize/replace a cursor. Mitigation: finite checked
domain, exact integers, rejection before callbacks, independent metadata and
guard cursor/state/event comparisons and explicit recovery limits.
Verified walkthrough: 1.9 reaches synthetic handle 1, materializes the expected
metadata cursor, clears cancellation with action tables and one table/cursor
event each. Oversized operands reject with 1466, preserve dirty/cancel true/
action cancel and the local guard, materialize no metadata cursor and emit
no table/cursor event. Both modes reset, close cursors and disconnect.
Full native connected/backend parity is not claimed; non-Numeric controls
remain disclosed gaps.

Rollback reverts only this bounded slice through a reviewed PR while retaining
regressions. Correct matrix/coverage/handoff/release evidence and notify the
owner before reusing incorrect guidance. No independent-human or installed-
product assurance is implied by implementation-agent checks.

DV-5611-SQLTABLES-CONVERSION-001: implementation-agent self-review completed
2026-10-07 against the exact pending diff, not independent-human approval.
Walked 1.9 metadata success and signed-huge/infinite rejection through the
retained PRG setup/state/cursor/guard/event/message comparisons. Checked
int32-adjacent doubles, exact-integer extremes, both-mode derived expectations,
native hashes and the separate #7032 recovery gap against docs/22 and docs/32.
Reviewed reproduction commands for owned paths, one build per directory and
serial runtime tests; confirmed unchanged localization catalogs, metadata/
cursor implementation, type/arity paths, other SQL callers and no VM/backend/
system side effects. The completed GCC, sanitizer and six contracts above
supply automated evidence for this medium-severity development procedure.
Release qualification and independent-human review are not claimed. Rollback
requires a reviewed revert, retained regressions and correction/notification
of misleading guidance.
