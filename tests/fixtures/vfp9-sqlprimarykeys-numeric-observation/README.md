# SQLPRIMARYKEYS extension and installed VFP9 presence observations

Recovered 2026-10-07 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
probe.prg is complete clean-room source, named deliberately not
sqlprimarykeys.prg to avoid user-function lookup finding the probe itself.
probe.out retains VERSION, a native SQLTABLES control, 48 explicit
SQLPRIMARYKEYS(handle, 'ORD*', 'native_primarykeys') attempts, the first error message,
absent cursor and unchanged session 1. Four fresh serial runs match all 53
lines. Every run used the complete retained source, including the message
row. No VM, external SQL/ODBC backend, installer or
system/calendar change was used.

## Requirement and extension boundary

RQ-CF-PRG-SQLPRIMARYKEYS-HANDLE-NUMERIC-001 derives from owner-admitted
#5611/#6776, RQ-CF-PRG-NUMERIC-BEHAVIOR-001, direct owner extension steering
2026-10-06, and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SQLPRIMARYKEYS's first-argument unchecked llround/int dispatch is selected.
Numeric/exact integers truncate toward zero into a finite signed-int32 domain
in both modes. NaN/infinite/oversized values raise localized catchable 1466
before metadata callbacks, cursor materialization, state or success events.
Exact int64/uint64 stay exact without double rounding. Existing other coercions
keep checked half-away rounding. Original Numeric diagnostics use the safe
decimal formatter, avoiding unchanged formatter gap #6997.

The native SQLTABLES control raises the expected absent-handle 1466. All 48
SQLPRIMARYKEYS attempts instead raise 1; the first message is exactly
"File 'sqlprimarykeys.prg' does not exist." This observes missing native function
resolution, **not SQLPRIMARYKEYS argument admission or converted indices**.
There is no native conversion/result/type oracle from these attempts, and
no inferred aliases or emulation of a missing function in VFP9 mode.

The owner's extension policy explicitly permits filling VFP gaps without
native syntax. SQLPRIMARYKEYS remains available in both Numeric modes with
this derived checked policy. Its synthetic metadata shape/results are
preserved test controls, not requirements recovered from implementation
or installed external-backend parity. No defect issue is filed merely
because an intentional extension is absent from native VFP9.
Metadata/backend semantics, cursor allocation, absent/default-handle behavior,
type/arity and other SQL callers remain separate. No argument-sized allocation
or loop is introduced.

## Reproduction and identities

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqlprimarykeys-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-sqlprimarykeys-numeric-observation/probe.prg tests/fixtures/vfp9-sqlprimarykeys-numeric-observation/probe.out
```

Source SHA-256: 6e61b6138aea5658d69a3d58d3c57ce685039b400c2c78ef92e628285c7d6129.
Output SHA-256: 7ddcae6348911ee85c3e1e89b14a1ea37a6c8366ff17de761dd2034f7a2c7ea3.
VR-5611-SQLPRIMARYKEYS-NATIVE-PRESENCE-001: four fresh serial runs match all
53 lines, with the recognized native control and missing-function message.
Generated FXP files are ignored scratch, not inspected proprietary binaries.

## Verification architecture

134 direct calls independently use the checked expectation column in both
modes, including adjacent int32 doubles, NaN/infinities, exact int64/uint64
beyond 2^53/extrema and preserved coercions. Native missing-function errors
do not supply conversion expectations.

104 fresh synthetic-session cases contain 320 PRG rows. Each creates only
in-process handle 1, a local guard cursor with one unchanged record/field,
and a dirty transaction with cancellation. Independent expectations compare
status/error/message, connection/dirty/cancel/action flags, metadata cursor
presence, LastCursorAlias, one primary-key row/six fields/first ORDER_ID column,
preserved guard payload/shape and successful sql.primarykeys/sql.cursor events.
Eight signed huge/infinite message cases verify original Numeric diagnostics
and state. All cases reset the mode, close both cursors and disconnect.
Runtime tests run serially across builds because neighbors share scratch;
never overlap build processes within one build directory.

```sh
cmake -S . -B /home/rich/temp/copperfin-sqlprimarykeys-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqlprimarykeys-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlprimarykeys-handle-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-sqlprimarykeys-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-sqlprimarykeys-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlprimarykeys-handle-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
```

## Completed baseline and verification evidence

VR-5611-SQLPRIMARYKEYS-CONVERSION-001 baseline: with the new helper/tests but
unchanged expression dispatch, each build fails exactly 66 SQLPRIMARYKEYS
assertions (fractional routing, oversized alias/rejection, diagnostics and
success events). GCC Numeric 14.48s/four suites 16.15s; Clang sanitizer
Numeric 66.10s/four suites 73.18s. The three neighboring suites pass in both;
no sanitizer diagnostic appears. Direct helper, setup, coercion and cleanup
controls pass. The baseline cannot prove legacy native conversion semantics.
Six contracts pass (5.20s), including 157 valid changelog fragments.

VR-5611-SQLPRIMARYKEYS-GCC-001: final GCC 15.2 Debug passes the same four
suites 4/4 in 16.59s (Numeric 14.78s), including every direct and fresh-session
check. The baseline expectations remain unchanged.

VR-5611-SQLPRIMARYKEYS-SANITIZER-001: final Clang 21.1.8 ASan/UBSan/
float-cast-overflow with -fno-sanitize-recover=all and default leak detection
passes the same four suites 4/4 in 71.15s (Numeric 64.01s), without diagnostics.
All 134 direct checks and 104 fresh synthetic cases/320 PRG rows pass without
weakening the failing-baseline expectations. Runtime tests run serially
across builds; source/output hashes remain unchanged.

VR-5611-SQLPRIMARYKEYS-CONTRACTS-001: final locale-install, contributor-signoff,
changelog assembler/fragment validation, agent-channel and native-test-isolation
contracts pass 6/6 in 4.47s; all 157 fragments validate.

## Documentation requirement and misuse

DQ-5611-SQLPRIMARYKEYS-CONVERSION-001 requires explicit owner-derived extension
boundaries, finite-domain pre-metadata rejection, safe diagnostics, serial
verification and rollback guidance. Procedural delta: add checked conversion
only before SQLPRIMARYKEYS; retain callback, metadata/cursor implementation,
type/coercion, arity and other SQL behavior.

Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01.
Unchecked or speculative aliases can read the wrong connection's primary-key catalog
and unexpectedly materialize/replace a cursor. Mitigation: finite checked
domain, exact integers, pre-callback rejection, independent metadata/guard/
state/event comparisons and no native conversion claims.
Verified walkthrough: 1.9 reaches synthetic handle 1, materializes the
expected primary-key cursor, clears cancellation with action primarykeys and one
primary-key/cursor event each. Oversized values reject with 1466, preserve
dirty/cancel true/action cancel and the guard, create no metadata cursor
and emit no primary-key/cursor events. Both modes reset, close both cursors and
disconnect. Full installed-backend parity is not claimed.

Rollback reverts only this bounded slice through a reviewed PR while retaining
regressions. Correct matrix/coverage/handoff/release evidence and notify the
owner before reusing incorrect guidance. No independent-human or installed-
product assurance is implied by implementation-agent checks.

DV-5611-SQLPRIMARYKEYS-CONVERSION-001: implementation-agent self-review
completed 2026-10-07 against the exact pending diff, not independent-human
approval. Walked 1.9 metadata success and signed huge/infinite rejection
through the retained setup/state/cursor/guard/event/message comparisons.
Checked int32-adjacent doubles, exact-integer extrema, both-mode derived
expectations and native fixture hashes against docs/22 and docs/32. Confirmed
native missing-function resolution is only presence evidence, not conversion
or type admission, and intentional extension absence is not a defect.
Reviewed commands for owned paths, one build per directory and serial runtime
tests; confirmed unchanged localization catalogs, metadata/cursor implementation,
type/arity paths, other SQL callers and no VM/backend/system side effects.
Completed GCC, sanitizer and six contracts above supply automated evidence
for this medium-severity development procedure. Independent-human review and
release qualification are not claimed. Rollback requires a reviewed revert,
retained regressions and correction/notification of misleading guidance.
