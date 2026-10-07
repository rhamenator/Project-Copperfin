# Installed VFP9 SQLEXEC admission and checked handle evidence

Observed 2026-10-07 with installed Visual FoxPro 09.00.0000.7423 under Wine,
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
Complete clean-room probe.prg/probe.out retain VERSION, recognized SQLTABLES
control, 48 SQLEXEC attempts, first error, session 1 and absent native_exec.
Three fresh serial runs match all 53 lines. No external backend, VM,
installer, binary inspection or system change; generated FXP is ignored scratch.

## Governing requirements and scope

RQ-CF-PRG-SQLEXEC-HANDLE-NUMERIC-001 derives from owner-admitted #5611/#6776,
parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Numeric truncates into finite signed int32 in both modes. Exact int64/uint64
remain exact without a floating round trip. Existing other coercions preserve
checked half-away rounding. Reject NaN, infinities and oversized operands with
localized catchable 1466 before executable commands, cursor materialization,
count/cancel/alias/action mutation or successful SQL events. Preserve safe
original Numeric diagnostics, bypassing unchanged shared formatter gap #6997.
Neither mode infers native connected indices or wrapping aliases.

RQ-CF-PRG-SQLEXEC-NATIVE-ADMISSION-001 recovers 41 absent Numeric errors 1466
and seven Currency/Logical/NULL/Character errors 11. First message:
"Connection handle is invalid." Recognized control is 1466; session 1 and
cursor absence remain unchanged. Identical absent errors do not establish
fractional routing or connected/wrapped indices. Separate #7047 against exact
main adc1e460560aa32942276db1b8497f4dbb0891bf retains native callback/type
admission and distinguishing connected-index recovery. Existing callback/
command/prepared fallback results are preservation controls, not recovered
requirements. Callback/backend/AERROR/type/arity, CALLFN and other callers
remain outside this bounded first-argument conversion.

## Native reproduction and identities

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqlexec-handle-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-sqlexec-handle-numeric-observation/probe.prg tests/fixtures/vfp9-sqlexec-handle-numeric-observation/probe.out
```

Source SHA-256: e3b995b47f7f031ed70c71833950f748c9d0be16dd54c525b7898e7196b5447c.
Output SHA-256: c2a7714106416e8642664c1a002c617f01af79cbc625a0458a478c48cbae8d75.
VR-5611-SQLEXEC-NATIVE-001: three matching fresh serial 48-call/53-line runs.

## Verification architecture

134 direct checked-expectation calls exercise fractions, signed-int32 adjacent
double boundaries, NaN/infinities, exact signed/unsigned extrema and values
above 2^53, and checked existing Currency/Logical/NULL/Character coercions.
Expectations use independent checked columns, not production helpers or the
SQLGETPROP native-alias column.

416 fresh synthetic cases (1,724 PRG rows) cross both modes with explicit DML, explicit SELECT,
empty-command prepared SELECT fallback and empty command/no statement.
Every case independently checks dirty/canceled state seeded through DML/prior
SELECT, prepared command, status/error/message, connection/result count/
cancel/action/last alias, selected alias, guard shape/payload, prior/result
cursor presence and counts, exact successful exec/total SQL events and last
effective command detail, mode reset,
cursor closure and disconnect. Thirty-two signed huge/infinite message rows
retain original Numeric text. Only an in-process handle 1 is used; no SQL/ODBC
backend, argument-sized allocation or loop. Three neighbors cover SQL metadata,
arrays and string/math.

```sh
cmake -S . -B /home/rich/temp/copperfin-sqlexec-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqlexec-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlexec-handle-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-sqlexec-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-sqlexec-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-sqlexec-handle-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' -j1
```

Runtime suites run SERIAL across build directories, one build process per dir.

## Documentation, misuse and rollback

DQ-5611-SQLEXEC-CONVERSION-001 requires truthful native/derived boundaries,
checked pre-execution admission, safe original diagnostics, serial verification
and rollback. Procedural delta: replace only first-argument conversion; retain
command/callback/prepared fallback/backend/AERROR/type/arity and other sites.

Development guidance misuse severity is medium: rounded/wrapped input could
execute a command on the wrong connection, clear cancellation, change a dirty
transaction or materialize a misleading result cursor. Parent runtime-crash/
data-corruption hazards are mitigated by checked admission, exact integers,
catchability and independent command/state/cursor/event preservation controls.

Walkthrough expectations: 1.9 targets synthetic handle 1; DML/explicit SELECT/
prepared fallback preserve the selected command path and normal state updates.
Empty command/no statement retains its existing callback error/reset path.
Oversized/non-finite input raises 1466 before execution, retaining canceled
dirty state, prepared command, prior/guard cursors/counts and producing no
new exec/SQL events. Each case closes only owned cursors, resets mode and
disconnects. Native absent errors do not reveal connected indices.

Rollback is a reviewed revert of this slice with regressions retained. Correct
matrix/coverage/handoff/release evidence and notify the owner before reusing
incorrect guidance. Implementation-agent self-review plus automated evidence
is development evidence, not independent-human or release qualification.

## Unchanged-dispatch baseline evidence

VR-5611-SQLEXEC-CONVERSION-001: the full GCC baseline fails exactly 320 selected
assertions: 208 output/state/diagnostic comparisons, 42 successful exec-event
counts, 42 total SQL-event counts and 28 effective-command details.
Numeric 37.57s/four suites 39.30s. All direct/helper/coercion/setup/cleanup
controls and three neighbors pass. All 208 failed case outputs retain matching
setup, cleanup and prior-cursor three-row count. No sanitizer diagnostic in
either instrumented baseline. The refreshed full Clang ASan/UBSan/float-cast-
overflow baseline fails the same 320 assertions (Numeric 171.00s/four
177.88s), with all controls/neighbors passing and zero setup/cleanup/prior-
count mismatches in the 208 failed case outputs. This full run includes all
28 command-detail failures. No library llround instrumentation claim.

The first sanitizer baseline (without the later command-detail assertions)
fails 292 selected assertions in Numeric 172.22s/four suites 183.84s.
It is supplemental evidence only, not the full final baseline.

## Completed fixed verification and documentation review

VR-5611-SQLEXEC-CONVERSION-002 (2026-10-07): final GCC 15.2 Debug four
suites pass 4/4 in 40.07s (Numeric 38.31s). Final Clang 21.1.8 ASan/UBSan/
float-cast-overflow four suites pass 4/4 in 177.48s (Numeric 170.61s), with
no sanitizer diagnostics. All 134 direct checks and 416 cases/1,724 rows
pass unchanged independent expectations. The sanitizer test object was
explicitly refreshed to include the effective-command detail assertions.

VR-5611-SQLEXEC-CONTRACTS-001: six locale-install, contributor-signoff,
changelog assembler/fragment validation, agent-channel and native-isolation
contracts pass 6/6 in 5.06s; 162 fragments are valid. Native source/output
hashes above remain unchanged; git diff --check and channel verification pass.

DV-5611-SQLEXEC-CONVERSION-001: implementation-agent development self-review
completed 2026-10-07. Reviewed the exact selected expression/helper/test diff,
complete native fixture, independent checked columns, setup/cleanup controls,
baseline-to-fixed results, docs/22 and docs/32 bidirectional mappings, safe
original diagnostics, command/event/state/cursor walkthrough, medium-severity
misuse, procedural delta and rollback. Production edits are confined to this
one conversion; callback/command/prepared fallback/type/arity/backend/AERROR
are unchanged. #7047 remains a truthful native recovery gap; no absent-error
fixture is presented as connected-index evidence. Guidance and retained
results agree with the automated evidence and operator outcomes above.
This closes development DQ verification only; independent-human review,
release qualification and external-backend compatibility are not claimed.
