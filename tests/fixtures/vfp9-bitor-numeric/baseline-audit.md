# Frozen BITOR input and baseline audit

RQ-CF-PRG-BITOR-NUMERIC-001 mapped BEFORE helpers/tests/migration.
Freeze2026-10-09 BEFORE first configuration:104 direct checks and472 fresh
public cases, both modes/selected sessions1/2, plus two unchanged neighbor
functions. Independent literal native results (OR identity0), derived default
error11/exact64/NaN/nextafter safety; no helper-derived expected values.
All26 slots, first/second/late, absorbing-1 all-operand admission, mixed signed
results, preservation controls, assignment/error/continuation/cursor/mode/reset.
Exact2^53+1 uses two exact64 seeds, not a rounded Numeric literal.

## Frozen SHA-256

- helpers.h: 79cce3d88c15e2c2cd66001c680caffe9bc1ba8e36fc465b0a476ffc2420667d
- helpers.cpp: 71437e7a5e7c0695276428c72d1416ad1bb261fdf4e071d418c2e36f8a144992
- test_prg_engine_bitor_numeric.cpp: 81b617b7a81ad19e0508eb09767588005c33d7f973ed27c3f2c1c48fac6f2b50
- test_prg_engine_bitor_neighbors.cpp: 50788cdf7297fbf3c806375170ff35947cf34fb64cac161af468d35f72b021e2
- tests/CMakeLists.txt: 0db8bee78871de517f900d268a61b2826e3a52dd5ac46a15abd63e64c67730da
- tests/CopperfinTestIsolation.cmake: e2aeb0d966e8eddee6b442912d0017d2af141e2f586ad2c5ef26f58891273357

Original dispatcher a2543da1800dffba29dc1e8e3d6c71003d9df10bd91069e66f86af332b00745f
and shared platform helper639778a08a38ef7d89a53d8f04a7daec99faf32c0077867cb9cf12a277225ffe
match origin/maina9cd973ce144595e04df9ab30d41438eb9f36639.
New checked_bitor_argument delegates only independently matched conversion,
and is UNUSED by BITOR throughout BOTH original baselines. Shared helper and
all other bit function branches must remain unchanged after BITOR migration.

## Execution boundary

GNU Debug-g0 and Clang Debug-O1-g0 ASan/UBSan/float-cast-overflow with matching
C/CXX/EXE/SHARED flags/framepointers. Strictly serial one-job builds/tests,
nice10/ionice2:7/private TMPDIR, focused per-test180s cap. Clang leaks disabled,
abort_on_error1/halt_on_error1:print_stacktrace1. Retain full raw GNU/Clang logs
and separate selected/unrelated failures; complete BOTH originals BEFORE BITOR
dispatch migration. No original/fixed/pass/diagnostic result claimed at freeze.
No frozen helper/assertion/test/CMake/isolation repair/removal/weakening without
explicit re-freeze and BOTH original repeats. Fixed/broader/contract/DQDV proof,
signed exact-head verification, external review/hosted checks remain pending.

## GNU unchanged-original result

GNU15.2 Debug-g0 configuration and one-job build completed successfully.
Frozen focused CTest exited8: Numeric FAILED5.89s, unchanged neighbors PASS0.12s,
total6.03s. All104 direct checks and472 public cases executed. The372 failures
are exclusively selected public expectations:80 error and292 result;
264 COPPERFIN and108 VFP9. No direct/helper, context/continuation/reset or
neighbor assertion failed. This unsanitized run makes no sanitizer claim.
Full unedited423-line original-gcc.log SHA-256:
0868e6fcbcaa4570c0668aaa1a09c194afcca57b8c8cd18e7f29ff9154e5ef84.
All six frozen inputs and original dispatcher/platform hashes were unchanged
after this run. No migration or frozen-input repair has occurred; the unchanged
Clang sanitizer original remains mandatory before migration.

## Clang unchanged-original result

Clang21.1.8 configuration and one-job sanitizer build completed successfully.
ASan/UBSan/float-cast-overflow matching C/CXX/EXE/SHARED flags, O1-g0/frame
pointers; leaks disabled, abort_on_error=1, halt_on_error=1:print_stacktrace=1.
Frozen focused CTest exited8: Numeric FAILED10.68s, unchanged neighbors PASS0.10s,
total10.80s. All104 direct checks and472 public cases executed. The372 FAIL
lines are byte-identical to GNU:80 error/292 result,264 COPPERFIN/108 VFP9;
zero direct/context/continuation/reset/neighbor failures. No ASan, UBSan or
float-cast-overflow diagnostic. Full unedited423-line original-clang.log SHA:
ba255c06ff787dc744c13c5094194c12406a6e9742604443063138ad3cb86c2e.
All six frozen input and original dispatcher/platform hashes remained unchanged
after this run. Private TMPDIR/nice10/ionice2:7/-j1/180s cap; strictly after GNU.
BOTH original runs and raw logs are retained before any BITOR migration.
No frozen helper/assertion/test/CMake/isolation repair, removal or weakening.
Fixed checks remain pending at this recording point.

## BITOR-only migration and first fixed GNU check

Only the BITOR branch in prg_engine_runtime_surface_dispatch_general.inl was
migrated after both original logs were recorded. Selected-session mode is
read once; every operand is admitted by the frozen helper even after an
intermediate -1. Failure raises existing localized catchable11. The fold uses
uint32 and signed result int64(result)-4294967296 above INT32_MAX, avoiding
out-of-range unsigned-to-signed narrowing and preserving the int64 result kind.
Fixed dispatcher SHA70ce05c99bdbe5d0489e8ae0191fad1fe17edc2dbf89016015d61488e74512f4;
shared platform-helper hash remains the original above. Frozen inputs unchanged.
GNU fixed one-job build succeeded. Focused verbose serial/private-TMPDIR CTest
exit0,2/2 PASS: numeric5.53s, neighbors0.11s,total5.65s. Actual104direct/472public
counters retained in full unedited46-line fixed-focused-gcc.log, SHA:
047412fdc15036616dd10ce79a16bc36aeb71e20448a1dfea0af500fc3ce2aa1.
Broader GNU/fixed Clang/assurance and exact-head external gates remain pending.

## Fixed GNU broader verification

Verbose serial/private-TMPDIR CTest completed exit0,15/15 PASS,total429.38s:
BITOR numeric5.80s/neighbors0.11s, BITAND5.79s, BITNOT1.16s, Collection1.87s,
older Numeric65.95s, NULL0.13s, localization14.21s and seven contracts
(intake, both changelog checks, agent channel, native isolation, safety workflow,
focused-path filters). Safety contract331.01s retained its existing1200s cap;
other tests used CLI600s. This is not a workflow timeout change or timing/cause
claim. Actual BITOR104direct/472public, BITAND104/472, BITNOT104/156 and
Collection116/140/26 counters retained, no assertion failure.
Full unedited193-line fixed-gcc.log SHA:
4935a76be04774dcb4dd75b0787d0e27170823b02ad8916695e13059760ac9c8.
Raw verbose log trailing spaces after Environment variables headings are
preserved byte-for-byte, not source whitespace errors. All six frozen input
and shared platform-helper hashes still match. Fixed Clang and exact-head
external gates remain pending; no frozen input repair/weakening.

## Fixed Clang focused/shared verification

Fixed one-job Clang build and verbose serial/private-TMPDIR CTest completed
strictly after GNU: exit0,7/7 PASS,total131.91s. BITOR numeric10.65s/neighbors
0.10s, BITAND10.59s, BITNOT1.89s, Collection2.34s, NULL0.13s and older Numeric
106.19s. Actual BITOR104direct/472public, BITAND104/472, BITNOT104/156 and
Collection116/140/26 counters retained. No assertion or ASan/UBSan/
float-cast-overflow diagnostic; leaks disabled, abort/halt/stacktrace flags
unchanged. CLI600s broader cap, not a workflow timeout change.
Full unedited101-line fixed-clang.log SHA:
91c4cd8a1c61256fddf10bd219eb0dff48e4ac607bd963ecf6b10ba695355c19.
Timings are retained without performance/root-cause claims. All six frozen
inputs and shared platform helper remain unchanged through originals/fixed
runs; no helper/assertion/test/CMake/isolation repair/removal/weakening.
Completed bounded medium-misuse development DQ/DV walkthrough/self-review
and rollback plan are in README, not independent-human/high-hazard/full-native/
type/binary/arity/platform/leak/release/parent qualification. No issue closure.
Postpush exact-head proof, Claude-FIRST/authorized Codex review, required hosted
checks and resolved conversations remain integration gates.
