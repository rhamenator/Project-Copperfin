# Frozen BITXOR input and baseline audit

RQ-CF-PRG-BITXOR-NUMERIC-001 mapped BEFORE helper/test creation/migration.
Freeze2026-10-09 BEFORE first configuration:104 direct checks and488 fresh
public cases, both modes/selected sessions1/2, plus two unchanged expression
neighbor functions. Independent literal XOR identity0/native mixed results,
derived default11/exact64/NaN/nextafter safety; no helper-derived expectations.
All26 slots, first/second/late, mixed signs/cancellation, all-operand admission,
assignment/error/continuation/cursor/mode/reset and safe-coercion controls.
Exact2^53+1 uses two exact64 seeds, not a rounded Numeric literal.

## Frozen SHA-256

- helpers.h: f2a0bb827f70183682811bccc585d3bb3da5ca39f1703bb41714139f14a647ea
- helpers.cpp: 3e72842d3d74b8171d0d1c6b9c2e552ec08369aadb14200a43eca2f6ca7dfca7
- test_prg_engine_bitxor_numeric.cpp: 559d7867dfa9277705b5dc40a59b2242a0c78dc9d229c0eb3b21e96014662f98
- test_prg_engine_bitxor_neighbors.cpp: 50788cdf7297fbf3c806375170ff35947cf34fb64cac161af468d35f72b021e2
- tests/CMakeLists.txt: d81790ea464408df8ac573192d68c1d201fab989fd5a6b8cb71d5eda160a7703
- tests/CopperfinTestIsolation.cmake: d509ad55a7762326cb7281025b7f6264734ef9f7480c3c60f82cce38b696b6b3

Original dispatcher70ce05c99bdbe5d0489e8ae0191fad1fe17edc2dbf89016015d61488e74512f4
and shared platform639778a08a38ef7d89a53d8f04a7daec99faf32c0077867cb9cf12a277225ffe
match base origin/mainb4cd165e39ac64f5e474d88a141d991c206efcae.
New checked_bitxor_argument delegates independently matched conversion only;
UNUSED by BITXOR throughout BOTH original baselines. Existing BITOR/BITAND/
BITNOT/other branches and shared platform helpers remain unchanged.

## Execution boundary

GNU Debug-g0 and Clang Debug-O1-g0 ASan/UBSan/float-cast-overflow with matching
C/CXX/EXE/SHARED flags/framepointers. Strictly serial one-job builds/tests,
nice10/ionice2:7/private TMPDIR/180s focused test cap. Clang leaks disabled,
abort_on_error1/halt_on_error1:print_stacktrace1. Retain full raw GNU/Clang logs
and separate selected/unrelated failures; complete BOTH originals BEFORE XOR
dispatch migration. No original/fixed/pass/diagnostic result claimed at freeze.
No frozen helper/assertion/test/CMake/isolation repair/removal/weakening without
explicit re-freeze and BOTH original repeats. Fixed/broader/contract/DQDV proof,
signed exact-head verification, external review/hosted checks remain pending.

## GNU unchanged-original result

GNU15.2 Debug-g0 configuration and one-job build completed successfully.
Original focused CTest16467 exited8: Numeric FAILED7.99s, unchanged neighbors
PASS0.12s,total8.13s. All104 direct checks and488 public cases executed.
388 failures are exclusively selected public expectations:86 error/302 result,
278 COPPERFIN/110 VFP9. Zero direct/helper/context/continuation/reset/neighbor
failures. Full unedited439-line original-gcc.log SHA-256
c06ac791f68484c267c5c648c2e9418a9ef179a4dc6b6decedfc9cadd5c6849b.
All six frozen inputs and original dispatcher/platform hashes are unchanged.
No BITXOR dispatch migration or frozen-input repair has occurred; a complete
unchanged Clang sanitizer original remains mandatory before migration.

## First Clang original attempt: incomplete timeout

Clang21.1.8 one-job sanitizer build succeeded, then CTest50846 exited8:
Numeric TIMEOUT180.38s before its final104/488 counter, neighbors PASS0.10s,
total180.50s. Matching frozen C/CXX/EXE/SHARED ASan/UBSan/float-cast-overflow/
O1-g0/framepointer flags; leaksdisabled/abort/halt/stacktrace/privateTMPDIR.
302 selected FAIL lines were emitted (86error/216result), with no observed
direct/context/continuation/reset/neighbor failures or sanitizer diagnostic.
This is INCOMPLETE baseline evidence, not a completed original run or proof
that all scenarios executed. Full unedited352-line original-clang.log and
identical preserved original-clang-timeout.log SHA-256:
6b413db0647be9a3a90c0414c928f9ebc08a17653d313da4fd745ec099cd3ff4.
All six frozen/originaldispatcher/platform inputs still match. No input repair
or dispatch migration. Repeat uses the SAME binary/flags/inputs and serial
private-TMPDIR execution with CLI600s allowance, retaining a distinct full raw
original-clang-repeat.log. This is not a workflow/test-definition timeout change.
Read-only host snapshot showed284GB root/30GB tmp available and two subsequent
vmstat samples; it cannot establish the first run's timeout root cause.

## Complete unchanged Clang repeat BEFORE migration

SAME binary, matching sanitizer flags, all six frozen inputs and original
dispatcher/platform hashes unchanged; CTest56048 completed exit8. Numeric
FAILED365.34s, unchanged neighbors PASS0.10s,total365.45s. Actual104 direct
checks/488 public cases executed. All388 selected failures match GNU byte-for-
byte (86error/302result;278COPPERFIN/110VFP9); zero unrelated assertions or
ASan/UBSan/float-cast-overflow diagnostics. Leaks disabled, not leak evidence.
Full unedited439-line original-clang-repeat.log SHA-256:
a10fea1c2f9cf513a8f72c68244fd2ae68e15e0711f65201293485bae7c11eb2.
The incomplete first180s attempt remains retained separately, never replaced.
No frozen-input repair/removal/weakening occurred. Both complete unchanged
originals and full logs are now recorded BEFORE any BITXOR dispatch migration.
Timings are observations, not performance/root-cause or timeout-optimum claims.

## BITXOR-only migration, fixed GNU pending

AFTER recording both complete originals, only BITXOR dispatch migrated:
selected-session mode once, checked_bitxor_argument for EVERY operand even
after prefix cancellation, localized catchable11 before assignment on rejection,
uint32 XOR fold and defined signed32 arithmetic in existing int64 result kind.
Fixed dispatcher SHA-256:
21bf6be3e0e4245e8653c1a1dc1011d254e66d98c6d44b3aa8795f53d7b96667.
All six frozen inputs/shared platform helper remain unchanged. No additional
allocation or change to the existing bounded operand loop; no shared/sibling/
type/Currency/binary/arity/NULL migration.
Fixed GNU one-job rebuild succeeded; focused CTest89562 is running serially
with CLI600s/private TMPDIR/nice10/ionice2:7 and full fixed-focused-gcc.log.
No fixed result or acceptance claim yet; broader GNU/Clang/assurance remain.

Read-only focused process sample while GNU was pending: own Numeric PID1446020
DN, wait channel jbd2_log_wait_commit,1s CPU at92s elapsed. This observes a
filesystem-journal wait only, not a diagnosed first Clang timeout cause or
product failure. No concurrent build/test or VM was started; broader work
waits for completion. Keep full raw results and actual counters before claims.

## Fixed GNU focused completed

CTest89562 completed exit0,2/2 PASS: Numeric347.92s, unchanged neighbors0.12s,
total348.05s. Actual104 direct checks/488 public cases, no assertion failure.
Full unedited46-line fixed-focused-gcc.log SHA-256:
1a85690d96e52d93e5438e477c3a014c30052cd152e53b50c178b919ed30b04d.
All six frozen inputs/shared platform helper match; fixed dispatcher retains
the hash above. No frozen repair/removal/weakening. Timings are retained without
performance/root-cause claims; broader GNU/fixed Clang/assurance remain pending.

## Fixed GNU broader completed

One-job rebuild and verbose serial/private-TMPDIR CTest63626 completed exit0,
16/16 PASS,total449.11s: BITXOR Numeric8.16s/neighbors0.11s, BITOR7.95s,
BITAND8.20s, BITNOT1.70s, Collection2.39s, older Numeric80.58s, NULL0.14s,
localization13.55s and seven repository contracts. Safety contract324.55s
retained its existing1200s cap; other tests used CLI600s, not workflow changes.
Actual BITXOR104direct/488public, BITOR/BITAND104/472, BITNOT104/156,
Collection116/140/26 counters retained; no assertion failure. All six frozen
inputs/shared platform and fixed dispatcher retain recorded hashes; no repair
or weakening. Full unedited204-line fixed-gcc.log SHA-256:
be02a6656a7a3d1927609750fcd517fb273a0691d560722104e2f0bb7b52e053.
Raw Environment variables headings retain trailing spaces byte-for-byte;
these logs are not source whitespace errors. Clang fixed build+CTest73151
then started strictly serially with matching unchanged sanitizer flags and
private TMPDIR, leaksdisabled/abort/halt/stacktrace, onejob/CLI600s/nice10/
ionice2:7. Fixed Clang and completed local DV acceptance remain pending.

## Fixed Clang and bounded development assurance completed

Matching unchanged Clang21.1.8 sanitizer flags/one-job rebuild and CTest73151
completed strictly after GNU: exit0,8/8 PASS,total180.93s. BITXOR Numeric15.09s/
neighbors0.10s, BITOR14.06s, BITAND14.02s, BITNOT2.52s, Collection2.99s,
older Numeric131.99s, NULL0.14s. Actual BITXOR104direct/488public,
BITOR/BITAND104/472, BITNOT104/156 and Collection116/140/26 counters retained.
No assertion or ASan/UBSan/float-cast-overflow diagnostic; leaks disabled, not
leak evidence. Full unedited112-line fixed-clang.log SHA-256:
88fc6383f8e99f724c6f731d1d53dc7d0698c50f521cf922c80fe3d0b3142e58.
All six frozen inputs/shared platform/fixed dispatcher retain recorded hashes
through BOTH originals/fixed runs; no repair/removal/weakening. Timings are not
performance/root-cause/timeout-optimum claims; incomplete first Clang original
remains distinct and is not complete evidence. DQ/DV development self-review/
automated GNU/Clang walkthrough/rollback completed in README for this bounded
medium-misuse delta, not independent-human/high-hazard qualification or hazard
acceptance. Unadmitted residualarity#7115/source table remains untouched.
Signed postpush exact-head proof, actual clean Claude-FIRST/authorized Codex
review, required hosted checks and resolved conversations remain gates.
No full-native/type/Currency/binary/arity/platform/leak/release/parent acceptance.
