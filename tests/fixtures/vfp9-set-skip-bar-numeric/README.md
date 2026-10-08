# SET SKIP OF BAR Numeric literals (#5611/#6776/#5868)

RQ-CF-PRG-SET-SKIP-BAR-NUMERIC-001 derives from parent
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner policy and independent installed VFP9.
Mapped BEFORE helper/production migration; bounded requirement now defined.
Scope: existing parsed Numeric literal identifier only.

Default COPPERFIN finite truncation accepts converted1..INT32_MAX, otherwise
localized catchable167 before skip-state mutation or success telemetry.
Explicit VFP9 finite raw>=2^32 saturatesMAX; lower signed-low32 conversion
accepts positive keys and -1/-2. Those sentinels succeed without changing any
existing user bar in native controls; preserve that supported-lane no-op and
do not manufacture negative user-bar state or claim system-menu support.
Other nonpositive converted values reject167. Accepted positive conversion
must be used consistently for skip state and runtime.set_skip telemetry.
Nonfinite helper containment derives from owner safety policy; unchanged
literal parser does not admit NaN/inf/overflowed exponents.

Three unchanged serial installed VFP9 7423 runs match125 lines,40 operands
x singleton seeds1/2/MAX, exactly one RELATIVE bar/noactivation. Paired .T./.F.
trials reset the known seed to the opposite state before each command;
SKPBAR receives the known constant seed, not the operand.
1.9 changes only1; negative-wide1/2 aliases change matching seeds; raw>=2^32
changes onlyMAX; converted-1/-2 succeeds with seeded skip unchanged in both
paired trials, not lookup error.
Positive missing keys also succeed and leave the one seeded bar unchanged;
no new general target-admission change or defect is inferred.
Paired sourceSHA256 397d6b473b4f20418f930333da5e05dbbfc1ae2f5d12839fada85038b5f8aff3.
Paired outputSHA256 f023e19ff12609f6e567912d0cd7dc49acaa1e0ed237035c5ee154c978c61989.
Earlier three single-.T. runs established the mapping before helper creation;
paired extension independently confirms clearing the same converted identity.
Shipped help fb960559-060b-4ffa-aaa4-35514c8789dc read fully: numeric
user-bar identifier and logical enable/disable policy. Dynamic expressions,
system items, MENU/PAD/POPUP variants, SET MARK, bar queries, native GUI and
#4630 lifecycle remain separate. Native evidence contradicts #5868's blanket
fractional-rejection expectation.

VR-5611-SET-SKIP-BAR-NUMERIC-001: direct constants/nonfinite/bounds; both-mode
guarded .T./.F. state, reentrant host selection and accepted/suppressed callback,
telemetry/current-session/cursor/marks/prompts/count, locales and old neighbors.
70direct,124 guarded (116fresh+8opposite-origin session2),16locale cases,
three old neighbors. Queries use independent integer seed constants; host
acceptance/suppression and callback markers separately verify selected identity.
VR-002: byte-identical original-dispatch negative then fixed GCC/Clang sanitizer
replay, broader Numeric/localization/contracts. Original dispatcher412 selected
failures per GCC/Clang, zero unrelated/no diagnostics; unchanged helper/final
tests across comparison, retained hashes/reconstruction in baseline-audit.txt.
Fixed GCC focused2/2 PASS2.02s (numeric1.90/neighbors0.10), logSHA256
f2e45f8853bf8631d1156df85d40d0272cac238d5b29843d5378e42d7c6eebea.
Fixed GCC broader Numeric/localization/contracts and new targets11/11 PASS465.86s,
logSHA256 5151f13728233b6f08b0f29f5951c46cefbc37b5a54901c217474e1ab5c2514e.
Fixed Clang21.1.8 -O1 ASan/UBSan/float-cast-overflow2/2 PASS2.26s
(numeric2.16/neighbors0.09), no diagnostics; ASANdetect_leaks0/abort1,
UBSANhalt1/stack1. LogSHA256
08fa8d9eb47ba43db53438727addc68d806acfbb0efe2c58b60e01fb77a19dc3.
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01.
DQ/DV-5611-SET-SKIP-BAR-NUMERIC-001: medium misuse; procedural delta is
identifier conversion only, not expression/routing/precedence/lifecycle.
Development maintainer self-review/walkthrough completed under owner
authorization against the final unchanged tests and full GCC/focused sanitizer
results, not independent human review.
Walkthrough:1.9 .T. suppresses only bar1's callback, not bar2; .F. restores
only bar1 from an independently disabled baseline. Default0.5/2^31 raises167,
preserves flags/marks/prompts/cursor/session, emits no success event.
Legacy4294967297 targetsMAX, not1; negative-wide aliases target1/2;
converted-1/-2 succeeds without changing user flags, and success telemetry
reports the converted sentinel. Checks repeat host selection twice, with
stable action callback identities; opposite-origin session2 uses selected
mode. Four locales agree on ERROR/AERROR/ErrorNo/message. Explicit release/
cursor close/reset precedes test-owned cancel/reap; no #4630 lifecycle fix.
Self-review traced native/help/requirement/helper/single site/tests, verified
checked-before-state/event order and positive-only map keys, unchanged
accepted expression evaluation and unparsed syntax, bounded native allocation,
other menu state, rollback/field notice. Bounded development acceptance complete;
exact-head external review and hosted checks remain pull-request gates.
Rollback isolated helper/site/catalog/test delta; field notice truncation,
default167, opt-in aliases/MAX/supported-lane sentinels. No independent human,
full type/GUI/leak/platform/release qualification claim.
