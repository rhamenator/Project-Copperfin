# Installed VFP9 SQLCOLUMNS handle observations

Recovered 2026-10-07 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
probe.prg is complete clean-room source. probe.out retains VERSION, a native
SQLTABLES control, 48 explicit SQLCOLUMNS(handle, 'ORD*', 'FOXPRO',
'native_columns') calls, the first error message, absent cursor and unchanged
session 1. Four fresh serial runs match all 53 lines. No external SQL/ODBC
connection, VM, installer or system/calendar change was used.

## Requirement and recovery boundary

RQ-CF-PRG-SQLCOLUMNS-HANDLE-NUMERIC-001 derives from owner-admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SQLCOLUMNS's first-argument unchecked llround/int dispatch is selected.
Numeric/exact integers truncate toward zero into a finite signed-int32 domain
in both modes. NaN/infinite/oversized operands raise localized catchable 1466
before metadata callbacks, cursor materialization, state or success events.
Exact int64/uint64 stay exact without double rounding. Other existing coercions
retain checked half-away rounding. Safe original Numeric diagnostics avoid
unchanged formatter gap #6997.

This is derived checked policy, not recovered native connected-index parity.
All 41 native Numeric calls raise 1466, including zero, fractions and both-sign
low-32 candidates. The first message is "Connection handle is invalid."
Identical absent errors cannot reveal converted indices or prove SQLGETPROP
aliases. Neither mode invents those aliases; future compatibility work requires
a native fixture distinguishing valid connection handles.

Seven native Currency/Logical/NULL/Character controls raise 11. Current
callback/type behavior differs; separate focused recovery gap #7040 retains
these results against exact main c347045b6fc2614430b2e7e74f05f4b981ef2006
without expanding this conversion slice. FOXPRO/NATIVE metadata shapes below
are preserved synthetic controls, not recovered installed-backend requirements.
Table pattern, format, metadata/backend results, cursor allocation, type/arity,
absent/default-handle semantics and other SQL callers remain separate.
No argument-sized allocation or loop is introduced.

## Reproduction and identities

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqlcolumns-handle-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-sqlcolumns-handle-numeric-observation/probe.prg tests/fixtures/vfp9-sqlcolumns-handle-numeric-observation/probe.out
```

Source SHA-256: bd4ff3376e6c48916878b849efb9dfa11ff2602b9509aae2baa4f1796f6b2a35.
Output SHA-256: bb9b7a6cc3cafeefd08804d77b80b3d3468ddf65c4d160bab2c7fdd810be49d0.
VR-5611-SQLCOLUMNS-NATIVE-001: four fresh serial runs match all 53 lines,
including the first native invalid-connection message and native control.
Generated FXP files are ignored scratch, not inspected proprietary binaries.

## Verification architecture

134 direct calls independently use the checked expectation column in both
modes, including adjacent int32 doubles, NaN/infinities, exact int64/uint64
beyond 2^53/extrema and checked preserved coercions. Native absent-handle
errors cannot supply unobserved converted-index or compatibility expectations.

208 fresh synthetic-session cases contain 640 PRG rows, covering FOXPRO and
NATIVE in both modes. Each creates only in-process handle 1, a local guard
cursor with one unchanged record/field and a dirty transaction with
cancellation. Independent expectations compare status/error/message,
ConnectHandle/LastSqlAction/TransactionDirty/CancelRequested, metadata cursor
presence, LastCursorAlias, three rows with four FOXPRO/eight NATIVE fields
and first ORDER_ID column, preserved guard payload/shape and successful
sql.columns/sql.cursor events. Sixteen signed huge/infinite message cases
verify original Numeric diagnostics and state. All cases reset the mode,
close both cursors and disconnect. Existing metadata suite also preserves
missing-table and schema controls; no native backend parity is claimed.
Runtime tests run serially across builds because neighbors share scratch;
never overlap build processes within one build directory.

```sh
cmake -S . -B /home/rich/temp/copperfin-sqlcolumns-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqlcolumns-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlcolumns-handle-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-sqlcolumns-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-sqlcolumns-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlcolumns-handle-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
```


## Completed baseline and verification evidence

VR-5611-SQLCOLUMNS-CONVERSION-001 baseline: with new helper/tests but unchanged
expression dispatch, each build fails exactly 132 selected SQLCOLUMNS
assertions (fractional routing, oversized alias/rejection, diagnostics and
success events). All FAIL lines are SQLCOLUMNS-only. GCC Numeric 18.56s/four
suites 20.27s; Clang sanitizer Numeric 84.88s/four suites 91.75s.
Direct/helper/setup/coercion/cleanup controls and three neighbors pass.
No sanitizer diagnostic appears. The baseline cannot establish native indices.
Six baseline contracts pass 6/6 (4.63s), 159 valid fragments.

VR-5611-SQLCOLUMNS-GCC-001: final GCC 15.2 Debug passes the same four suites
4/4 in 20.52s (Numeric 18.78s), including 134 direct checks and every fresh
FOXPRO/NATIVE state/cursor/guard/event/error/message/reset/cleanup comparison.
The baseline expectations remain unchanged.

VR-5611-SQLCOLUMNS-SANITIZER-001: final Clang 21.1.8 ASan/UBSan/
float-cast-overflow with -fno-sanitize-recover=all and default leak detection
passes the same four suites 4/4 in 92.88s (Numeric 85.90s), without diagnostics.
All 134 direct checks and 208 fresh synthetic cases/640 PRG rows pass without
weakening baseline expectations. Runtime tests ran serially across builds.
Native source/output hashes remain unchanged.

VR-5611-SQLCOLUMNS-CONTRACTS-001: final locale-install, contributor-signoff,
changelog assembler/fragment validation, agent-channel and native-test-isolation
contracts pass 6/6 in 4.42s; all 159 fragments validate.

## Documentation requirement and misuse

DQ-5611-SQLCOLUMNS-CONVERSION-001 requires explicit derived/native boundaries,
finite-domain pre-metadata rejection, safe original Numeric diagnostics,
serial verification and rollback guidance. Procedural delta: add checked
conversion only before SQLCOLUMNS; retain table pattern, format, callback/
metadata/cursor implementation, type/coercion, arity and other SQL behavior.

Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01.
Unchecked or speculative aliases can query the wrong connection's column
catalog and unexpectedly materialize/replace a cursor. Mitigation: finite
checked domain, exact integers, pre-callback rejection, independent metadata/
guard/state/event comparisons and explicit native recovery limits.

Walkthrough acceptance: 1.9 reaches synthetic handle 1 in both modes/formats,
materializes three rows/four FOXPRO or eight NATIVE fields with first ORDER_ID,
clears cancellation with action columns and one column/cursor event each.
Oversized arguments raise 1466, preserve dirty/cancel true/action cancel and
the guard, create no metadata cursor and emit no column/cursor events.
Signed huge/infinite diagnostics preserve the original Numeric value.
Both modes reset, close both cursors and disconnect; installed connected/
backend parity is not claimed and non-Numeric admission remains #7040.

Rollback reverts only this bounded slice through a reviewed PR while retaining
regressions. Correct matrix/coverage/handoff/release evidence and notify the
owner before reusing incorrect guidance. No independent-human or installed-
product assurance is implied by implementation-agent checks.

DV-5611-SQLCOLUMNS-CONVERSION-001: implementation-agent self-review completed
2026-10-07 against the pending bounded diff, not independent-human approval.
Walked 1.9 metadata success in both FOXPRO/NATIVE formats and signed huge/
infinite rejection through retained setup/state/cursor/guard/event/message
comparisons. Verified all acceptance outcomes above pass unchanged in both
Numeric modes. Checked int32-adjacent doubles, exact-integer extrema, derived
checked expectations and fixture hashes against docs/22/docs/32.
Confirmed native absent errors establish only admission/errors, not converted
indices; callback/type parity and connected-index recovery remain #7040.
Reviewed commands for owned paths, one build per directory and serial runtime
tests; confirmed unchanged localization catalogs, table/format/metadata/cursor
implementation, type/arity paths, other SQL callers and no VM/backend/system
effects. Completed normal/sanitizer and contract results supply automated
evidence for this medium-severity development procedure. Independent-human
review and release qualification are not claimed. Rollback requires a
reviewed revert, retained regressions and correction/owner notification
before reusing incorrect guidance.
