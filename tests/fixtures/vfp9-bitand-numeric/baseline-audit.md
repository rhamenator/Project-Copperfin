# Frozen BITAND input and baseline audit

RQ-CF-PRG-BITAND-NUMERIC-001 mapped before helpers/tests/migration.
Freeze2026-10-09 BEFORE first configuration:104 direct checks and472 fresh
public cases (both modes, selected sessions1/2); two unchanged neighbor functions.
Independent literal native results, derived checked default error11, exact64
and NaN boundaries; no expected result derived from the new helper.
Public first/second/26-slot/mixed/zero-mask/exact64/preserved-type cases guard
result/error, continuation, selected-session/mode/cursor/record/content/reset.
Exact2^53+1 uses two exact64 seeds, not a rounded Numeric literal.
Infinity0 is freshly recovered in explicit VFP9; NaN rejection is derived.
Native/type/binary/invalid-arity parity and other bit functions stay separate.

## Frozen SHA-256

- helpers.h: f9b037b3728fd030fc02c15d91ef5df71c0e752daf2857d4b98e99b8048b81af
- helpers.cpp: 39f2431d137f75454f3d7e6c44dbff0f8133737c0cd6a668291cb657881a1033
- test_prg_engine_bitand_numeric.cpp: 9f5ada14f18ba7efbe881559d05942f888df4010ec6575aa05aadc51f09a9c22
- test_prg_engine_bitand_neighbors.cpp: 50788cdf7297fbf3c806375170ff35947cf34fb64cac161af468d35f72b021e2
- tests/CMakeLists.txt: 1eecabc3996c6bf9b6bfb7059db8fc600e2f402e0cdfe65ed03a855c6e4ebc5b
- tests/CopperfinTestIsolation.cmake: 0040595b39192a6d1a3c60c282aafb63e64db5905c45bc1834497be730bd5160

Original dispatcher SHA
f8dee07b722c2faeaf61a68a7ea9d2738198c6fa135b5c52f49601b7f5e10314
and shared platform helper SHA
639778a08a38ef7d89a53d8f04a7daec99faf32c0077867cb9cf12a277225ffe
match origin/main67cf4fbc3. The new checked helper is UNUSED by BITAND through
BOTH original baselines. Shared platform helper must remain original after
BITAND migration; other bit functions remain separate.

## Execution plan / result boundary

GNU15.2 Debug-g0 one-job build. Clang21.1.8 Debug-O1-g0 with matching
C/CXX/EXE/SHARED ASan/UBSan/float-cast-overflow flags and frame pointers.
Strictly serial Copperfin builds/tests, nice10/ionice2:7/private TMPDIR;
180s focused per-test cap. Sanitizer leaks disabled, abort/halt/stacktrace on
actual diagnostics. Retain complete original logs, count selected versus
unrelated failures, and finish BOTH original runs before BITAND-only migration.
No original/fixed pass or diagnostic result is claimed at this freeze point.

No repair/removal/weakening of frozen helper/assertions/tests/CMake/isolation
after this freeze without an explicit re-freeze and BOTH original repeats.
Broader/older Numeric/localization/contracts, fixed sanitizer results,
DQ/DV walkthrough/self-review and exact-head external CI/review remain pending.

## GNU unchanged-original result

GNU15.2 Debug-g0 configuration and one-job build completed successfully.
Frozen focused CTest exited8: Numeric FAILED5.42s, unchanged neighbors PASS0.13s,
total5.56s. All104 direct checks and472 public cases executed. The368 failures
are exclusively selected public error/result expectations:80 error and288
result;262 COPPERFIN and106 VFP9. No direct/helper, context/continuation/reset
or neighbor assertion failed. This unsanitized run makes no sanitizer claim.
Full unedited394-line original-gcc.log SHA-256:
be3dbdff51ff021b1c9fcde223fbf4a6cf7f695f1732e254b750c187aac55c00.
All six frozen inputs and original dispatcher/platform hashes were unchanged
after this run. No migration or frozen-input repair has occurred; the unchanged
Clang sanitizer original remains mandatory before migration.

## Clang unchanged-original result

Clang21.1.8 configuration and one-job sanitizer build completed successfully.
ASan/UBSan/float-cast-overflow, matching C/CXX/EXE/SHARED flags, O1-g0/frame
pointers; leaks disabled, abort_on_error=1, halt_on_error=1:print_stacktrace=1.
Focused CTest exited8: Numeric FAILED10.59s, unchanged neighbors PASS0.10s,
total10.70s. All104 direct checks and472 public cases executed. The368 FAIL
lines are byte-identical to GNU:80 error/288 result,262 COPPERFIN/106 VFP9;
zero direct/context/continuation/reset/neighbor failures. No ASan, UBSan or
float-cast-overflow diagnostic. Full unedited394-line original-clang.log SHA:
f67d826ab60b23f22cbab215bf1dfe8ea9d2dd0ba353831dc361c56419f4a8a7.
All six frozen input and original dispatcher/platform hashes remained unchanged
after this run. Private TMPDIR/nice10/ionice2:7/-j1/180s cap; strictly after GNU.
BOTH original runs and raw logs are retained before any BITAND migration.
No frozen helper/assertion/test/CMake/isolation repair, removal or weakening.
Fixed checks remain pending at this recording point.

## BITAND-only migration and first fixed GNU check

Only the BITAND branch in prg_engine_runtime_surface_dispatch_general.inl was
migrated after both original logs were recorded. Selected-session mode is
read once; every operand is admitted by the frozen checked helper even after
an intermediate zero. Failure raises existing localized catchable11. The fold
uses uint32 and the signed result uses int64(result)-4294967296 above INT32_MAX,
avoiding out-of-range unsigned-to-signed narrowing and preserving int64 kind.
Global NULL/arity/routing and other bit functions are untouched by source
inspection, not newly recovered full native/type/binary/arity parity.
Dispatcher SHA a2543da1800dffba29dc1e8e3d6c71003d9df10bd91069e66f86af332b00745f;
shared platform-helper SHA remains the original above. Frozen inputs unchanged.
GNU fixed one-job build succeeded. Focused verbose serial/private-TMPDIR CTest
exit0,2/2 PASS: numeric5.78s, neighbors0.11s,total5.91s. Actual104direct/472public
counters retained in full unedited46-line fixed-focused-gcc.log, SHA:
15c86da103e099d043d6d992db36db661bcb5d1cc7ac46805202e1134e588992.
Broader GNU/fixed Clang/assurance and exact-head external gates remain pending.

## Fixed GNU broader verification

Verbose serial/private-TMPDIR CTest completed exit0,14/14 PASS,total426.82s:
BITAND numeric5.87s/neighbors0.12s, BITNOT1.13s, Collection1.79s,
older Numeric65.52s, NULL0.13s, localization13.54s and seven contracts
(intake, both changelog checks, agent channel, native isolation, safety workflow,
focused-path filters). Safety contract333.79s retained its existing1200s cap;
other tests used CLI600s. This is not a workflow timeout change or timing/cause
claim. Actual BITAND104direct/472public, BITNOT104/156 and Collection116/140/26
counters retained, no assertion failure. Full unedited182-line fixed-gcc.log SHA:
41e7424ba8d041816ec8e67b8d44726f9e5f4c39e1b8f644cc14601dcc225eca.
Raw verbose log trailing spaces after Environment variables headings are
preserved byte-for-byte, not source whitespace errors. Fixed Clang and
exact-head external gates remain pending; no frozen input repair/weakening.

## Fixed Clang focused/shared verification

Fixed one-job Clang build and verbose serial/private-TMPDIR CTest completed
strictly after GNU: exit0,6/6 PASS,total117.38s. BITAND numeric10.56s/neighbors
0.10s, BITNOT1.88s, Collection2.13s, NULL0.12s and older Numeric102.58s.
Actual BITAND104direct/472public, BITNOT104/156 and Collection116/140/26
counters retained. No assertion or ASan/UBSan/float-cast-overflow diagnostic;
leaks disabled, abort/halt/stacktrace flags unchanged. CLI600s broader cap,
not a workflow timeout change. Full unedited90-line fixed-clang.log SHA:
b6f757d797b8bb13ed46fe747c50952cf7f7120797f5a5808fc2faa8d005f337.
Timings are retained without performance/root-cause claims. All six frozen
inputs and shared platform helper remain unchanged through originals/fixed
runs; no helper/assertion/test/CMake/isolation repair/removal/weakening.
Completed bounded medium-misuse development DQ/DV walkthrough/self-review
and rollback plan are in README, not independent-human/high-hazard/full-native/
type/binary/arity/platform/leak/release/parent qualification. No issue closure.
Postpush exact-head proof, Claude-FIRST/authorized Codex review, required hosted
checks and resolved conversations remain integration gates.
