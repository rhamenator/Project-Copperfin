# Installed VFP9 SQLROLLBACK handle observations

Recovered 2026-10-07 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
sqlrollback.prg is the complete clean-room source. sqlrollback.out retains VERSION,
48 calls and unchanged session 1. Four fresh serial runs agree on all 50 lines,
including the final serial cross-check. No external SQL connection, ODBC
backend, VM, installer or system/calendar change was used.

## Requirement and recovery boundary

RQ-CF-PRG-SQLROLLBACK-HANDLE-NUMERIC-001 derives from owner-admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Select only SQLROLLBACK's first-argument unchecked llround/int dispatch.
Numeric/exact integers truncate toward zero into a finite signed-int32 domain
in both modes. NaN/infinite/oversized operands reject with localized catchable
1466 before transaction callbacks, state changes or successful events. Exact
int64/uint64 stay exact without double rounding. Other existing coercions keep
checked half-away rounding; native type admission remains outside this slice.
Safe original Numeric diagnostic formatting avoids unchanged gap #6997.

This is derived checked policy, **not recovered native connected-index parity**.
All 41 Numeric calls raise 1466, including zero, fractions and both-sign low-32
candidates. Identical absent errors cannot reveal converted indices or prove
SQLGETPROP aliases. Neither mode invents those aliases. Future compatibility
work must retain a native fixture that distinguishes valid handles.

Seven Currency/Logical/NULL/Character controls raise 11 in native VFP9.
Current callback/type behavior differs; #7030 records this separate parity and
connected-index recovery gap against exact main
371c5323e34f2cd3e40a1f20b579170967a75abf. This slice preserves those results
as controls, not native parity claims. Other SQL callers, transaction/backend behavior,
absent/default-handle semantics and arity stay separate. No argument-sized
allocation or loop is introduced.

## Reproduction and identities

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqlrollback-handle-numeric-observation/sqlrollback.prg
sha256sum tests/fixtures/vfp9-sqlrollback-handle-numeric-observation/sqlrollback.prg tests/fixtures/vfp9-sqlrollback-handle-numeric-observation/sqlrollback.out
```

Source SHA-256: b9c64bd9337c4f1fbe8713fe31cc07abc0a446ca824b341496c0b6a6e1fe976f.
Output SHA-256: f0ce4b2d8f8292884f70c454cb76a0ccda874642298d743eca28bfa56f55e1c9.
VR-5611-SQLROLLBACK-NATIVE-001: four fresh serial runs match all 50 lines. Generated
FXP files are ignored scratch, not inspected proprietary binaries.

## Verification architecture

134 direct calls use the checked expectation column independently in both
modes, with adjacent int32 doubles, NaN/infinities, exact int64/uint64 beyond
2^53 and extrema, and preserved coercions. The native error-only fixture does
not supply unobserved compatibility alias expectations.

104 fresh synthetic-session cases contain 320 PRG rows. Each creates only
in-process handle 1, performs a synthetic insert and cancellation, and asserts
TransactionDirty/CancelRequested true and LastSqlAction cancel before rollback.
Independent expected comparisons verify status/error, connection presence,
transaction/cancel flags and action after the call; successful sql.rollback
event counts verify callback reachability. Eight signed-huge/infinite message
cases additionally check original Numeric diagnostic text and the same state.
All cases reset the mode and disconnect. These do not prove native backend
rollback behavior. Tests run serially across builds because neighbors share
scratch; never run multiple build processes within one build directory.

```sh
cmake -S . -B /home/rich/temp/copperfin-sqlrollback-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqlrollback-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlrollback-handle-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-sqlrollback-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-sqlrollback-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlrollback-handle-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
```

## Documentation requirement and misuse

VR-5611-SQLROLLBACK-BASELINE-001: unchanged SQLROLLBACK dispatch fails
exactly 66 new conversion/state/event/message assertions in Clang 21.1.8
ASan/UBSan/float-cast-overflow (Numeric 50.25s; four suites 57.35s). The three
neighbors, direct helper checks and preserved controls pass. No sanitizer
diagnostic appears. In particular, 0.5 and low-32 wrapping candidates
unexpectedly roll back synthetic handle 1 and clear dirty/cancel flags, while
1.9 misses handle 1. Oversized/infinite operands bypass required pre-callback
1466 rejection. GCC 15.2 Debug fails the same 66 assertions (Numeric 10.99s;
four suites 12.72s), while all three neighbors/direct helper/preserved controls
pass. No expectation is changed after this failing baseline.

VR-5611-SQLROLLBACK-GCC-001: final GCC 15.2 Debug passes the same four suites
4/4 in 12.71s (Numeric 11.01s). All 134 direct checks and 104 fresh synthetic
cases/320 PRG rows pass without weakening the failing-baseline expectations.

VR-5611-SQLROLLBACK-SANITIZER-001: final Clang 21.1.8 ASan/UBSan/
float-cast-overflow with -fno-sanitize-recover=all and default leak detection
passes 4/4 in 54.96s (Numeric 48.01s), without diagnostics. Runtime tests ran
serially across builds. Native source/output hashes remain unchanged.

VR-5611-SQLROLLBACK-CONTRACTS-001: locale-install, contributor-signoff,
changelog assembler/fragment validation, agent-channel and native-test-isolation
contracts pass 6/6 in 4.37s; all 154 fragments validate.

DQ-5611-SQLROLLBACK-CONVERSION-001 requires explicit derived/native boundaries,
finite-domain pre-transaction rejection, safe diagnostic text, serial verification
and rollback guidance. Procedural delta: add checked first-argument conversion
before SQLROLLBACK; retain callback, type/coercion, arity and other SQL behavior.

Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01.
Unchecked or speculated aliases can roll back the wrong connection's transaction.
Mitigation: finite checked domain, exact integers, rejection before callbacks,
independent dirty/cancel/action/event comparisons and explicit recovery limits.
Verified walkthrough: 1.9 reaches synthetic handle 1 and clears dirty/cancel
flags with action rollback and one successful event; oversized operands reject
with 1466, preserve dirty/cancel true/action cancel and emit no rollback event.
Both modes reset and disconnect. Full native connected/backend parity is not
claimed; non-Numeric controls remain disclosed gaps.

Rollback reverts only this bounded slice through a reviewed PR while retaining
regressions. Correct matrix/coverage/handoff/release evidence and notify the
owner before reusing incorrect procedure guidance. No independent-human or
installed-product assurance is implied by implementation-agent checks.

DV-5611-SQLROLLBACK-CONVERSION-001: implementation-agent self-review completed
2026-10-07 against the exact pending diff, not independent-human approval.
Walked the 1.9 success and signed-huge/infinite rejection cases through the
retained PRG setup/state/event/message comparisons. Checked int32-adjacent
double and exact-integer extremes, both-mode derived expectations, native
hashes and separate #7030 recovery gap against docs/22 and docs/32.
Reviewed the reproduction commands for owned paths, one build per directory
and serial runtime tests; confirmed unchanged localization catalogs, callback/
type/arity paths, other SQL callers and no VM/backend/system side effects.
The completed GCC, sanitizer and six contracts above supply automated evidence
for this medium-severity development procedure; release qualification and
independent-human review are not claimed. Rollback requires a reviewed revert,
retained regressions and correction/notification of misleading guidance.
