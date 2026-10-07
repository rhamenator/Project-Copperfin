# Installed VFP9 SQLCANCEL handle observations

Recovered 2026-10-07 from installed Visual FoxPro 09.00.0000.7423 under Wine
through the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
sqlcancel.prg retains the complete clean-room source; sqlcancel.out has VERSION,
48 calls and unchanged session 1. Three fresh connection-free runs match all
50 lines; the final two are byte-identical. No external SQL connection, ODBC
backend, network, VM, installer, persistent table or system change was used.

## Requirement, recovery limit and derived policy

RQ-CF-PRG-SQLCANCEL-HANDLE-NUMERIC-001 derives from owner-approved #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SQLCANCEL's first-argument unchecked llround/int dispatch is selected.
Numeric/exact integer values truncate toward zero and require finite signed
int32 admission in **both** modes. NaN/infinity and oversized values reject
with localized catchable 1466 before cancellation callbacks, state mutations
or successful events. Exact int64/uint64 avoid double rounding. Other existing
coercions retain checked half-away rounding; their native type parity is not
implemented here. Safe round-trip Numeric diagnostic formatting avoids the
unchanged shared formatter gap #6997.

This is a **derived safe policy, not recovered native index conversion**.
All 41 Numeric native calls raise 1466, including zero, ordinary fractions,
huge values and both-sign low-32 candidates. No connected handle exists, so
these outputs cannot independently distinguish any converted index or prove
SQLGETPROP's negative-only aliases for SQLCANCEL. Both modes therefore use the
checked policy rather than speculate. Recovery of native connected-index
truncation/wrapping remains an explicit gap; future compatibility work must
retain a native fixture that actually distinguishes valid handles. Do not
claim native connected cancellation or backend behavior from synthetic tests.

Native Currency, Logical, NULL and Character controls raise 11. Current
callback/coercion results do not match those observations; #7025 records
absent-handle/type parity separately against exact main
4a65f1d7cf7eb8dc6a616765da561e97d1610d41, without implementation admission.
RQ-CF-PRG-SQLCANCEL-NATIVE-ADMISSION-001 records that durable parity/recovery
gap separately in docs/32. This slice preserves existing results as controls.
It neither fixes callback error-state semantics nor opens a backend connection.
No argument-sized allocation or loop is added.

## Identity and reproduction

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqlcancel-handle-numeric-observation/sqlcancel.prg
sha256sum tests/fixtures/vfp9-sqlcancel-handle-numeric-observation/sqlcancel.prg tests/fixtures/vfp9-sqlcancel-handle-numeric-observation/sqlcancel.out
```

Source SHA-256: 8023d557f4cb983b1baaa280ec9e07e63a8154a921bcaab82489e48f2395e279.
Output SHA-256: 8b90a384f4cabc6dfe743693bb4a05f42246b050a628b7e70b2cb1bead81c851.
VR-5611-SQLCANCEL-NATIVE-001: three fresh processes match all 50 lines.
Generated FXP files are ignored scratch, not inspected proprietary binaries.

## Verification architecture

The direct boundary harness runs 134 calls with explicit checked expectations
in both modes: 41 Numeric values plus 26 extended/coercion boundaries per mode.
SQLCANCEL selects the checked expectation column deliberately, not the
SQLGETPROP compatibility-alias column. Adjacent signed-int32 doubles, exact
int64/uint64 values beyond 2^53, integer extrema, NaN and preserved coercions
are covered without deriving expectations from the helper under test.

104 fresh synthetic-session cases contain 312 PRG rows, in both modes.
Each creates only in-process handle 1. Independent comparisons check return/
error, connection presence, LastSqlAction and CancelRequested; exact successful
sql.cancel event counts check callback reachability. Four signed finite-huge/
infinite caught-message cases per mode exercise safe original operand text.
Mode reset and disconnect cleanup are checked for every case. These are
conversion/state comparisons, not native connected-backend observations.
Run all tests serially across builds because neighboring suites share scratch.

```sh
cmake -S . -B /home/rich/temp/copperfin-sqlcancel-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqlcancel-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j3
ctest --test-dir /home/rich/temp/copperfin-sqlcancel-handle-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-sqlcancel-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-sqlcancel-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j3
ctest --test-dir /home/rich/temp/copperfin-sqlcancel-handle-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
```

## Documentation assurance

VR-5611-SQLCANCEL-REGRESSION-001: before changing expression dispatch,
the corrected Clang ASan/UBSan/float-cast-overflow baseline fails exactly
66 new SQLCANCEL conversion/state/event assertions (numeric 47.30s;
four-suite run 54.26s). Direct helpers, preserved controls and all three
neighbors pass. No sanitizer diagnostic was produced. An earlier 118-failure
run mixed in a Logical-formatting harness assumption and is discarded:
the corrected test uses explicit IIF T/F rather than TRANSFORM spelling.
The initially overlapping/interrupted normal build is not baseline evidence;
normal verification is claimed only after a completed non-overlapping build.
Five signoff/changelog/channel/isolation contracts pass 5/5 in 4.72s,
and all 152 fragments validate. The locale-install contract separately passes
1/1 in 0.06s, verifying the unchanged localized diagnostic catalogs.

VR-5611-SQLCANCEL-SANITIZER-001: final Clang 21.1.8 ASan/UBSan/
float-cast-overflow with -fno-sanitize-recover=all and default leak detection
passes 4/4 in 51.77s (numeric 44.94s), without diagnostics. The same corrected
134 direct checks and 104 fresh-session/312 PRG rows pass; no expectation was
weakened after the dispatch change. Runtime tests ran serially across builds.

VR-5611-SQLCANCEL-GCC-001: completed non-overlapping GCC 15.2 Debug build
passes the same four suites 4/4 in 11.78s (numeric 10.09s). The corrected
expectations and cancellation-event counts are unchanged from the failing
baseline. Native source/output hashes were rechecked after implementation.

DQ-5611-SQLCANCEL-CONVERSION-001 requires explicit native/derived boundaries,
failure-before-callback and diagnostic guidance, serial verification and
rollback. Procedural delta: add checked conversion before cancellation;
retain callback/type/arity behavior and refrain from inferring native aliases
from non-distinguishing absent-handle errors.

Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01.
Unchecked admission can address another connection; speculative aliases can
cancel the wrong operation. Mitigation is explicit finite-domain rejection,
exact integers, retained evidence limits and pre-mutation tests. Walkthrough:
Numeric 1.9 truncates to synthetic handle 1 and emits one cancellation event;
out-of-domain values raise 1466, leave CancelRequested false/LastSqlAction
connect and emit no successful event. Both modes then reset and disconnect
handle 1. Non-Numeric controls remain parity gaps, not evidence to reuse.

Rollback reverts only this slice through a reviewed PR while retaining
unchecked-path regressions. Correct incorrect guidance in matrix/handoff/
release evidence and notify the owner before reuse. No independent-human,
installed-product, backend or full SQLCANCEL parity assurance is implied.

DV-5611-SQLCANCEL-CONVERSION-001: implementation-agent self-review completed
2026-10-07 against the exact pending diff, not independent-human approval.
Walked the 1.9 success and signed-huge rejection cases through the retained
PRG state/event comparisons; checked finite int32 and exact-integer boundaries,
both-mode expectations, native hashes and the separate #7025 gap against the
matrix and coverage note. Reviewed every reproduction command for owned build
paths and serial tests, and confirmed unchanged catalogs, callback/type/arity
paths, other SQL callers and no VM/backend side effects. GCC, sanitizer and
the six documentation/isolation/localization contracts above complete the
automated evidence for this medium-severity development procedure; release
qualification and independent review are not claimed.
