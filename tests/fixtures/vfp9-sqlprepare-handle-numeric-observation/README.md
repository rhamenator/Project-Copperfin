# Installed VFP9 SQLPREPARE admission and Copperfin handle boundary evidence

Observed 2026-10-07 with installed Visual FoxPro 09.00.0000.7423 under Wine
and the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
Complete clean-room probe.prg/probe.out retain VERSION, a recognized SQLTABLES
control, 48 SQLPREPARE attempts, first invalid-connection message and unchanged
session 1. Three fully compared fresh serial runs match all 52 lines.
No external SQL/ODBC backend, VM, installer, proprietary binary inspection or
system change. Generated FXP is ignored scratch, not binary evidence.

## Requirement recovery and bounded derived policy

RQ-CF-PRG-SQLPREPARE-HANDLE-NUMERIC-001 derives from owner-admitted
#5611/#6776, parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and
HZ-runtime-crash-01/HZ-data-corruption-01. Both modes require finite checked
signed-int32 handle admission: Numeric truncates toward zero, exact int64/
uint64 stay exact without a floating round trip, and existing other coercions
retain checked half-away rounding. Reject NaN, infinities and oversized
operands with localized catchable 1466 before prepared-command/cancel/action
mutation or successful events. Original Numeric diagnostics use safe
round-trip decimal formatting, bypassing unchanged formatter gap #6997.
No wrapping alias is inferred, even in explicit VFP9 mode.

RQ-CF-PRG-SQLPREPARE-NATIVE-ADMISSION-001 recovers native absent Numeric
error 1466 (41 calls) and other-type error 11 (seven Currency/Logical/NULL/
Character controls). The first message is "Connection handle is invalid.";
recognized SQLTABLES control raises 1466 and session 1 is unchanged.
Identical absent errors do not distinguish fractional routing, connected
indices or aliases. Separate #7045, against exact main
1f6a711b32fbccc515d95caa38d31f305122851f, retains callback/type admission
differences and connected-index recovery. Existing coerced/absent results are
preservation controls, not requirements or native parity evidence.
Command content, callback semantics, type/arity, backend, AERROR behavior,
SQLEXEC/CALLFN and other conversions remain outside this bounded slice.

## Native reproduction and identities

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqlprepare-handle-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-sqlprepare-handle-numeric-observation/probe.prg tests/fixtures/vfp9-sqlprepare-handle-numeric-observation/probe.out
```

Source SHA-256: 77bce347136ff82f299f084d93441c2eacc3b32ac0c706ba94b34ce88533d83e.
Output SHA-256: 5aeb6db244be7b14c5fc1ae48d0cf575f21bede26a3fa3447db12a474ad237ea.
VR-5611-SQLPREPARE-NATIVE-001: three fully compared serial runs match 48
calls/52 complete lines, including control, message and session.

## Verification architecture and reproduction

134 direct calls use independently specified checked expectations in both
modes: fractions, int32-adjacent doubles, NaN/infinities, exact int64/uint64
extrema/beyond 2^53 and existing Currency/Logical/NULL/Character controls.
No production converter or native SQLGETPROP alias generates expectations.

416 fresh synthetic cases/1,488 PRG rows cross canceled DML/SELECT setup,
empty/nonempty prepared commands and both modes. Each creates only in-process
handle 1, a one-field/one-record guard, dirty transaction, seeded prepared
command and cancellation. Independent comparisons check status/error/message,
ConnectHandle, LastResultCount (one DML or three SELECT rows), PreparedCommand,
LastSqlAction, TransactionDirty, CancelRequested, LastCursorAlias, selected
alias, guard shape/payload, result presence/three-row count, exact successful
prepare events and total SQL event counts. Thirty-two signed huge/infinite
diagnostics retain original Numeric text. All cases reset mode, close owned
cursors and disconnect. Three neighbors cover SQL metadata, arrays and
string/math. No argument-sized allocation/loop is introduced.

```sh
cmake -S . -B /home/rich/temp/copperfin-sqlprepare-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqlprepare-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlprepare-handle-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-sqlprepare-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-sqlprepare-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlprepare-handle-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
```

Runtime suites run serially across builds; one build process per directory.

## Documentation requirement, misuse and rollback

DQ-5611-SQLPREPARE-CONVERSION-001 requires native/derived boundaries, finite
pre-mutation admission, original diagnostics, serial verification and rollback.
Procedural delta: replace only first-argument unchecked conversion; retain
command, callback, existing coercion/arity/backend and other callers.

Misuse severity is medium for this development guidance: an incorrectly
rounded/wrapped handle may replace another connection's command or clear its
cancellation, leading to unintended downstream data operations. Parent hazards
HZ-runtime-crash-01/HZ-data-corruption-01 are mitigated by finite checked
admission, exact integers, catchable rejection and independent state controls.
No external backend or argument-sized operation is admitted.

Walkthrough: 1.9 targets synthetic handle 1, replacing only its seeded command,
clearing cancellation and changing action to prepare, while preserving dirty
transaction, result count, selected alias and guard/result. Empty command
replacement is separately controlled. Oversized input raises 1466, retaining
seed/action/cancel/count/cursors and emitting no new prepare/SQL events.
Reset/closure/disconnect is verified on every case. Native errors cannot
supply a connected-index oracle.

Rollback is a reviewed revert of only this slice, retaining regressions.
Correct matrix/coverage/handoff/release evidence and notify the owner before
reusing incorrect guidance. Implementation-agent self-review/automated evidence
is not independent-human approval or installed-product/release qualification.

## Unchanged-dispatch baseline evidence

VR-5611-SQLPREPARE-CONVERSION-001: new helper/tests with unchanged expression
dispatch fail exactly 320 selected assertions in each toolchain: 208 combined
result/state/diagnostic comparisons, 56 successful prepare-event counts and
56 total SQL-event counts. Full baseline logs contain no other FAIL lines.
GCC Numeric 28.91s/four suites 30.62s; Clang sanitizer Numeric 135.51s/four
142.62s. All direct/helper/setup/coercion/cleanup controls and three neighbors
pass. No sanitizer diagnostic appears; no claim is made for instrumenting
the library llround operation. Six baseline contracts pass 6/6 in 4.50s,
and all 161 fragments validate.

## Completed final verification

VR-5611-SQLPREPARE-GCC-001: GCC 15.2 Debug passes four focused suites 4/4
in 30.80s (Numeric 29.10s). All 134 direct calls and 416 fresh synthetic
cases/1,488 PRG rows pass without altering baseline expectations.

VR-5611-SQLPREPARE-SANITIZER-001: Clang 21.1.8 ASan/UBSan/float-cast-overflow,
-fno-sanitize-recover=all and default leak detection passes the same four
suites 4/4 in 139.53s (Numeric 132.57s), without diagnostics. Runtime suites
ran serially across builds; native fixture source/output hashes are unchanged.

VR-5611-SQLPREPARE-CONTRACTS-001: locale-install, contributor-signoff,
changelog assembler/fragment validation, agent-channel and native-test-isolation
pass 6/6 in 4.49s; all 161 fragments validate.

DV-5611-SQLPREPARE-CONVERSION-001: implementation-agent self-review completed
2026-10-07 against the bounded pending diff, not independent-human approval.
Walked 1.9 and signed huge/infinite inputs through both modes, both DML/SELECT
setups and empty/nonempty commands. Verified replacement/retention of seeded
command, catchability/message, count/connection/dirty/cancel/action, selected
alias, guard/result cursor, exact prepare/total SQL events and reset/cleanup
against independent rows. Compared setup/cleanup/result-count controls in all
208 failed baseline output cases per toolchain: zero control mismatches.
Checked direct int32-adjacent/NaN/exact-integer/coercion boundaries and native
identities against docs/22/docs/32. Confirmed native errors recover admission
only, not connected-index/alias parity. Reviewed serial runtime commands and
unchanged command/callback, type/arity/backend/AERROR, localization catalogs,
SQLEXEC/CALLFN and other callers. No argument-sized operations or VM/backend/
system changes. Normal, sanitizer and six contracts supply automated evidence
for this medium-severity development procedure. Independent-human and release
qualification are not claimed. Rollback requires reviewed revert, retained
regressions, corrected guidance and owner notification before reuse.
