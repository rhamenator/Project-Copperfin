# CURSORSETPROP BUFFERING Numeric boundary (#5611/#6776)

## Governing requirement recorded before implementation

RQ-CF-PRG-CURSORSETPROP-BUFFERING-NUMERIC-001 derives from admitted
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Three matching45-line installed VFP9 09.00.0000.7423 runs retain39 Numeric
operands,four type controls,version and cleanup. VFP truncates Numeric values; modes 1..5
succeed and invalid converted modes raise catchable 1469 without changing
buffering mode, cursor row/count/payload or active session. Native low32
aliases may select modes 1..5. Default COPPERFIN shall truncate and check
the finite 1..5 domain before integral conversion; only explicit VFP9 shall
retain defined low32/indefinite-zero conversion. Zero remains invalid.
Exact int64/uint64 follow derived extension policy without double loss.

Only the second-argument llround/int site is selected. Numeric rejection
shall use the existing localized unsupported-buffering-mode message with
safe original-operand formatting and error1469 before mutation. Other type
coercions retain checked half-away conversion and their existing false
return on invalid domain; native NULL/Character/Currency parity is separate.
Native NULL/Character/Currency differences are separately filed as #7077.
Property routing/arity, cursor lookup precedence, pending-edit rules, broader
buffering semantics and other callers remain outside scope. No new
argument-sized allocation or loop, external table or database service.

The independent native probe uses fresh temporary cursors only, MULTILOCKS
ON then OFF, and explicit cleanup. Two initial harness errors (unformatted
numeric SET(DATASESSION) and incorrectly quoted Character label) produced
partial output and are excluded from matching final-run evidence.

Verification plan: direct independent Numeric/exact/non-finite/adjacent
boundary expectations in both modes; explicit and implicit alias guarded
PRG cases, catch/message/result, buffering preservation, cursor payload/
row/count/session/mode, reset and cleanup; original unchanged-dispatch
failure audit then GCC and focused Clang sanitizer acceptance plus full
older Numeric and buffering/transaction neighbors. No completion or native
type/release/platform qualification is claimed until completed evidence.

DQ-CURSORBUFFER-001: distinguish native Numeric results from derived exact
extensions and preserved other coercions, with requirement/code/test links.
DQ-CURSORBUFFER-002: document serial private scratch reproduction, baseline
failures, limits, and reviewed rollback of only this slice. Medium misuse:
assuming a successful buffering-mode change can misroute edit handling.
Procedural delta will replace unchecked half-away/narrow with checked
truncation/error1469 for Numeric only; failed selection preserves old mode.
Automated VR/DV evidence below is required before behavior is marked complete;
this retained pre-implementation plan alone is not acceptance.

## Completed original-dispatch evidence

The dispatcher was byte-identical to main0f679faf71d1c2346e8500272b521dbab84a5a0e
with the new helper linked but unused. Both compiler runs fail exactly120
selected script-row assertions, zero other assertions; all124 helper calls,
16 preserved coercion controls, setup/cleanup/reset and state suffixes pass.
GCC flags1.49s/neighbor0.94s/total2.45s; Clang flags44.62s/neighbor2.65s/
total47.29s, no sanitizer diagnostic. Full older Numeric baseline not run.
The condensed audit retains original dispatcher and raw-log identities.
These failures are negative evidence, not completed fixed acceptance.

Native source SHA256:
9a7492c1ae2d8557a7be358d447c1403cc1f90a66517e1c28eabfb80e11e59fd
Native output SHA256:
9d30d4d9c4be876aca335ca27147ab3df0c7406791e063d6efd4ea8e6beff9c7

Original audit SHA256:
d0038e20597345140f0eed02475af318165f7dc151e6f7dea48252c905f59a3c

## Completed development documentation review

DV-CURSORBUFFER-001: Codex acting in the owner-authorized maintainer workflow
completed development self-review of native constants versus derived exact
integer/NaN boundaries, helper range-before-narrow paths, unchanged property/
lookup precedence, and validation before buffering/pending-record mutation.
Existing pending-edit refusal is preserved, not redefined; changed mode6
neighbor expectation follows fresh native1469 evidence. Other type controls
remain unchanged under #7077. This is not independent or human sign-off.

DV-CURSORBUFFER-002: completed guarded walkthrough creates a cursor and
selects prior mode3, attempts each selection, captures status/message/result,
then queries mode/row/count/payload/session/active switch. Rejected values
preserve prior mode; valid values select their independently expected mode.
Every case closes its cursor and all four runs restore MULTILOCKS OFF and
COPPERFIN. Original/fixed automated results below distinguish acceptance
from error. No runtime event-counter or suppressed-argument-evaluation claim.

DV-CURSORBUFFER-003: completed rollback walkthrough specifies a reviewed
revert limited to this slice, reruns the named boundary/Numeric/buffering/
table-mutation regressions and repository contracts, then republishes the
boundary notice through changelog/PR. No destructive reset or broad cleanup.
Notification retains #7077 and limits. Hosted checks, exact-head Claude or
authorized Codex review and resolved conversations remain merge gates.

Residual limits: script cases are Numeric, while exact integer/NaN/adjacent
boundaries use direct helper calls. Safe original-operand diagnostics reuse
the established round-trip formatter; error1469 and semantic localized text
are verified, not native byte-identical message text. General buffering,
pending-edit semantics, type/arity/routing and release/platform/integrator
safety assessment remain external to this boundary. No argument-sized loop
or allocation is added, and no external DBF/ODBC/network/VM is used.

## Completed local acceptance and reproduction

VR-CURSORBUFFER-001: all124 independent direct checks pass in both modes,
including exact int64/uint64 extrema/precision, adjacent domain/signed64
limits, NaN/infinities and checked other-type controls.
VR-CURSORBUFFER-002: all172 guarded cases across four mode/alias runs pass,
including catchable1469/safe English diagnostic/result and mode/cursor/
session/reset/cleanup preservation. The buffering neighbor also verifies
unsupported Numeric6 raises1469 and preserves prior mode5 after commit/revert.
VR-CURSORBUFFER-003: GCC four suites pass4/4 in107.84s (new1.32s,full
olderNumeric84.25s,buffering0.93s,tablemutation21.33s). Clang21 ASan/UBSan/
float-cast-overflow three suites pass3/3 in106.21s (new4.52s,buffering2.66s,
tablemutation99.02s), without diagnostics. Clang excludes full older Numeric;
detect_leaks=0, not leak/platform/release qualification. Six repository
contracts pass6/6 in4.33s; all172 fragments validate. Source-backed isolation
is test-owned unique scratch/scoped-restored locale/no child/network/samples/
shared resource, serial; duplicate concurrent invocations not supported.
Existing missing-field/unused-function/capture warnings remain.

Owned scratch builds (retained until merge, then recoverable trash):
 /home/rich/temp/copperfin-cursorsetprop-buffering-5611-build
 /home/rich/temp/copperfin-cursorsetprop-buffering-5611-sanitize
Build named targets using GCC15 Debug -O0 -g1 or Clang21 Debug with
-fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all
-fno-omit-frame-pointer. Set TMPDIR to a fresh mktemp -d directory inside
the selected owned build; ASAN_OPTIONS=detect_leaks=0 and
UBSAN_OPTIONS=halt_on_error=1 for Clang. Use ctest -j1 --output-on-failure:
GCC regex ^test_prg_engine_(cursor_buffering_numeric|numeric_behavior|runtime_surface_functions_buffering|table_mutation)$;
Clang uses the same selection without numeric_behavior. Runtime execution
is strictly serial across builds. The unchanged /home/rich/bin/vfp9-probe
wrapper executes probe.prg; three complete final runs match. Original
reproduction pins the base dispatcher with helper linked but unused; no
overwrite of unrelated changes. GeneratedFXP and owned builds move to trash
after merge; tracked evidence, unrelated files and all stashes remain intact.
