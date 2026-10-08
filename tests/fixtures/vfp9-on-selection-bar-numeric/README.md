# ON SELECTION BAR Numeric literals (#5611/#6776/#5868)

RQ-CF-PRG-ON-SELECTION-BAR-NUMERIC-001 derives from admitted parent
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and independent installed VFP9 7423 observations.
Mapped before helper/production migration. Scope: parsed Numeric literal of
existing DO-handler and static-action registration kinds only.

COPPERFIN finite truncation accepts converted1..INT32_MAX; other admitted
Numeric rejects localized catchable167 before binding mutation or compiled-
action cache erasure. VFP9 finite raw>=2^32 saturatesMAX, lower values use
signed-low32; positive aliases bind corresponding keys. Native converted-1/-2
raises1604 on these supported user popups, not ON BAR's1612; other nonpositive
values reject167. Nonfinite helper rejection derives from owner containment;
unchanged literal parser does not admit NaN/inf/overflowed exponents, so no
catchable167 claim for unparsed syntax.

Three unchanged serial native runs completed250lines:40 operands x singleton
seeds1/2/MAX x DO/static-action, exactly ONE parent bar/noactivation. GUARD
stays0: registration does not execute handler/action. Existing-bar admission
distinguishes converted identities without correlated numeric queries:
ordinary1.9 admits only seed1; negative-wide aliases1/2 admit matching small
seed; wide positive admits onlyMAX. Native other positive missing bars also
raise1604; general admission remains #6226, not selected or claimed repaired.
#5868's fractional-rejection expectation is contradicted by native.
Source SHA256120d61ce3e0b0a9953e729a7f201493c88bd92711797fae1c1fc5fea8a739ef4.
Output SHA256ad15cad20bdb2072aa8d7625e4eec7cc96dc91016fb49b5b35c9c9b4835a986d.
Installed shipped help166d908f-4d20-4ea7-badd-29dca7004633 read in full:
selection-time command assignment, usual DO procedure, omitted-command release.
Omitted commands/dynamic expressions/system menus are outside this slice.
No timeout, GUI/VM/template/backend/system/credential maintenance.

VR-5611-ON-SELECTION-BAR-NUMERIC-001: independent helper constants, both-mode
guarded DO/static-action host selections, replacement after action cache
priming, locale ERROR/AERROR/ErrorNo, opposite-origin session2/old neighbors.
VR-002: byte-identical original dispatch negative evidence, fixed GCC/Clang
sanitizer replay, broader Numeric/localization and contracts. Selected dispatch
is implemented at the two registration kinds; requirement status defined for
this bounded conversion. Original byte-identical dispatcher296 selected failures
per GCC/Clang, zero unrelated/no diagnostics; unchanged helper/tests across the
final comparison. See baseline-audit.txt for hashes/reconstruction and the
excluded initial300-failure harness run (four cleanup/reset assertions).

HZ-runtime-crash-01 checked conversion; HZ-data-corruption-01 failure-atomic
binding/cache guards; HZ-doc-command-01 explicit command/error/mode/scope.
DQ/DV-5611-ON-SELECTION-BAR-NUMERIC-001: medium misuse; development maintainer
self-review/walkthrough completed under owner authorization with GCC focused
2/2 PASS2.34s (numeric2.23/neighbors0.09), logSHA256
373ae5703fa0a9753cf51a5b4c39cf8e5e3ce2bc943a92a3a2e7ce578d9d13bf.
Final GCC11/11 PASS457.26s includes seven contracts, older Numeric,
localization and both new targets; logSHA256
369786bd8fb7c6db5af938667c8b668a0c13e8289e105ccb499e0f72f0b9cf6d.
Final Clang ASan/UBSan/float-cast-overflow2/2 PASS2.30s
(numeric2.21/neighbors0.08), detect_leaks0/abort1/halt_on_error1,
no sanitizer diagnostics; logSHA256
8e943718b5042cbc455b10b6123805c83bd562e3cb4c08b55254eecaf8bb4a90.
Development self-review/walkthrough reconciled against these final results;
not independent human review.
Walkthrough:1.9 replaces bar1, not2; default0.5/2^31 rejects167; wide positive
4294967297 default rejects, legacy bindsMAX not1. Negative-wide aliases1/2
remain opt-in; legacy converted-1/-2 rejects1604 without losing cached action/
DO routes. Prime every seed, rebind inside an existing callback, then select
each twice: accepted action cannot use its stale compiled routine, rejected
conversion cannot change prior markers. Marker checks do not introspect cache
object identity; checked-before-erase ordering also verifies that rejected
conversion cannot erase a cached action. Opposite-origin session2 uses selected
policy; four locales agree on ERROR/AERROR/ErrorNo/message. Explicit release/
cursor close/reset precedes test-owned cancel/reap, not a #4630 fix.
Self-review traced requirement/native constants/helper/two sites/tests, checked
one-bar native bounds and unchanged unparsed syntax, lookup and precedence
limitations, rollback/field notice. Procedural delta: two conversions, not routing/
precedence/lifecycle. Rollback isolated delta; field notice truncation,
default167 and opt-in aliases/MAX/1604. No full native type/GUI/system-menu/
leak/platform/release qualification claim.
