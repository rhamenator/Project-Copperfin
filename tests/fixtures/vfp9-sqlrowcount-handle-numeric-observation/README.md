# Installed VFP9 SQLROWCOUNT presence and Copperfin boundary evidence

Observed 2026-10-07 with installed Visual FoxPro 09.00.0000.7423 under Wine
and the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
Complete clean-room probe.prg and probe.out retain VERSION, a recognized
SQLTABLES control, 48 SQLROWCOUNT attempts, the first resolution message and
unchanged session 1. Four fresh serial runs match all 52 lines. No external
SQL/ODBC backend, VM, installer, proprietary binary inspection or system change.

## Derived requirement and native boundary

RQ-CF-PRG-SQLROWCOUNT-HANDLE-NUMERIC-001 derives from owner-admitted
#5611/#6776, parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001, direct owner extension
steering on 2026-10-06 and HZ-runtime-crash-01/HZ-data-corruption-01.
Copperfin extensions fill legacy gaps; lack of native syntax is not grounds
to remove or skip them. Both modes retain SQLROWCOUNT with finite checked
signed-int32 handle admission: Numeric truncates toward zero, exact int64/
uint64 stay exact without a floating round trip, and existing other coercions
keep checked half-away rounding. Reject NaN, infinities and oversized operands
with localized catchable 1466 before row-count reads. Safe original Numeric
diagnostics bypass the unchanged shared formatter gap #6997.
No wrapping alias is inferred, even in explicit VFP9 mode.

All 48 native attempts raise error 1 at function resolution. The first message
is "File 'sqlrowcount.prg' does not exist." The recognized SQLTABLES control
raises 1466. Native absence supplies no handle conversion, type, row-count or
connected-backend oracle. It is an intentional extension boundary, not a newly
discovered compatibility defect. Synthetic DML/SELECT controls preserve current
row-count lifecycle without claiming installed-backend parity. Callback return
narrowing/lifecycle, backend semantics, type/arity, absent/default behavior,
SQLEXEC, SQLPREPARE, CALLFN and other conversions remain separate.

## Reproduction and identities

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqlrowcount-handle-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-sqlrowcount-handle-numeric-observation/probe.prg tests/fixtures/vfp9-sqlrowcount-handle-numeric-observation/probe.out
```

Source SHA-256: 7afd64a7812c7a75e38254dea890a9a19dde2f0413fdfce3cf4ea3849d26c46b.
Output SHA-256: c5b38fe49b78d3cd20fe82d39aa009c1500a2e911c1908847e58735b8328f966.
VR-5611-SQLROWCOUNT-NATIVE-001: four matching fresh serial runs, 48 attempts/
52 complete lines including control, first error and session.
Generated FXP is ignored scratch, not inspected proprietary binary evidence.

## Verification architecture

134 direct calls use independently specified checked expectations in both
modes: fractions, int32-adjacent doubles, NaN/infinities, exact int64/uint64
extrema/beyond 2^53 and preserved Currency/Logical/NULL/Character controls.
No native SQLGETPROP aliases or production converter generates expectations.

208 fresh synthetic cases contain 744 PRG rows across canceled DML and SELECT
setups in both modes. Each creates only in-process handle 1, a one-record/
one-field guard with unchanged payload and a dirty canceled transaction.
Independent comparisons check status/error/message, ConnectHandle,
LastResultCount (one DML or three SELECT rows), LastSqlAction, TransactionDirty,
CancelRequested, LastCursorAlias, selected alias, guard shape/payload, result
cursor presence/three-row count and unchanged total SQL event counts (five
DML/seven SELECT, including setup and disconnect). Sixteen huge/infinite
diagnostic cases retain original Numeric text. Every case resets mode, closes
owned cursors and disconnects. Neighbors cover SQL metadata, arrays and
string/math. Runtime suites run serially across builds; one build process per
build directory. No argument-sized loop/allocation is introduced.

```sh
cmake -S . -B /home/rich/temp/copperfin-sqlrowcount-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqlrowcount-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlrowcount-handle-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-sqlrowcount-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-sqlrowcount-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlrowcount-handle-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
```

## Baseline evidence

VR-5611-SQLROWCOUNT-CONVERSION-001: new helper/tests with unchanged expression
dispatch fail exactly 104 selected SQLROWCOUNT assertions in each build
(fractional routing, oversized alias/rejection and original diagnostics).
All FAIL lines are SQLROWCOUNT-only. GCC Numeric 21.36s/four suites 23.07s;
Clang sanitizer Numeric 97.47s/four suites 104.65s. Direct/helper/setup/
coercion/cleanup/event controls and three neighbors pass. No sanitizer
diagnostic appears; no report is claimed for uninstrumented llround.
Six baseline contracts pass 6/6 in 5.21s, with 160 valid fragments.

## Documentation requirement and misuse

DQ-5611-SQLROWCOUNT-CONVERSION-001 requires the explicit extension/native
boundary, finite pre-read admission, safe diagnostics, serial verification and
rollback guidance. Procedural delta: replace only first-argument unchecked
conversion; retain callback/lifecycle, type/coercion, arity and other callers.

Misuse severity is medium: a rounded/wrapped handle can report another
connection's count and mislead decisions about data operations. Linked parent
hazards HZ-runtime-crash-01/HZ-data-corruption-01 are mitigated by checked
admission, exact integers, catchable rejection and independent state controls.
No backend or argument-sized operation is admitted by this slice.

Walkthrough acceptance: 1.9 reads handle 1's one DML or three SELECT rows in
both modes, preserving dirty/cancel true/action cancel and the guard/result.
Oversized input raises 1466 with the same state, no added SQL events and safe
original huge/infinite diagnostics. Every case resets/cleans/disconnects.
Neither native absence nor existing callbacks are requirement sources.

Rollback is a reviewed revert of only this slice, retaining regressions.
Correct matrix/coverage/handoff/release evidence and notify the owner before
reusing incorrect guidance. Implementation-agent verification is not
independent-human approval or installed-product/release qualification.

## Completed final verification

VR-5611-SQLROWCOUNT-GCC-001: GCC 15.2 Debug passes four focused suites 4/4
in 23.18s (Numeric 21.46s). All 134 direct calls and 208 fresh synthetic
cases/744 PRG rows pass without changing baseline expectations.

VR-5611-SQLROWCOUNT-SANITIZER-001: Clang 21.1.8 ASan/UBSan/float-cast-overflow,
-fno-sanitize-recover=all and default leak detection passes the same four
suites 4/4 in 105.62s (Numeric 98.72s), without diagnostics. Runtime suites
ran serially across builds. Native source/output hashes remain unchanged.

VR-5611-SQLROWCOUNT-CONTRACTS-001: locale-install, contributor-signoff,
changelog assembler/fragment validation, agent-channel and native-test-isolation
pass 6/6 in 6.89s; all 160 fragments validate.

DV-5611-SQLROWCOUNT-CONVERSION-001: implementation-agent self-review completed
2026-10-07 against the bounded pending diff, not independent-human approval.
Walked 1.9 and signed huge/infinite input through both modes and DML/SELECT
setups; verified count, catchability/message, action/dirty/cancel/connection,
selected alias, guard/result cursor, events and reset/cleanup against independent
expected rows. Checked direct int32-adjacent/NaN/exact-integer/coercion boundaries
and complete native identities against docs/22/docs/32. Confirmed native absence
is a supported extension boundary, not converted-index parity or a skipped
feature. Reviewed one-build-per-directory and serial-runtime commands; confirmed
unchanged callback, type/arity/lifecycle/backend, localization catalogs, other
SQL callers, no argument-sized operations and no VM/backend/system effects.
Normal, sanitizer and six contract results supply automated evidence for this
medium-severity development procedure. Independent-human review and release
qualification are not claimed. Rollback requires reviewed revert, retained
regressions, corrected guidance and owner notification before reuse.
