# GETBAR numeric position requirement and verification

Authority: OPEN/owner-authored/agent-approved #5611/#6776/#5868/#7096,
revalidated on2026-10-08 before issue bodies. This bounded slice converts
GETBAR second-argument Numeric/exact-integer positions only, preserving the
existing zero fallback and menu order. Native1612 remains #7096; structural
ordering remains #6152. This is not full GETBAR or parent acceptance.

## Independent installed evidence

Installed Wine VFP9 SP2 `09.00.0000.7423`, existing bounded vfp9-probe harness;
three strictly serial134-line final runs compare byte-identically.43 operands
per layout: singleton MAX; sparse13/47/MAX; and aligned-definition-order
4/13/47/100/MAX. No activation, VM, installer, clone or large allocation.
Each query retains count/first/last identity and known MAX prompt controls.
Source SHA-256 `5fa9c438fd8f5af50900f5affa09d467058ab3c8b152bcce955010ab3d91ffcf`;
probe.out/run1.out/run2.out/run3.out SHA-256
`53241cbf9a6cc2a03d0db25561a89f7e0838090558794681ac017dd264300119`.
1.4/1.5/1.9 select position1;2.9 selects2;3.9 selects3;4.9 selects4;
5.9 selects5 when present. Explicit native positive4294967297/98/99 and
negative4294967295/94/93 select positions1/2/3. This is not setter saturation.
Other operands raise catchable1612 without changing the independent controls;
no full type/system-menu/error publication claim follows from this probe.

Initial90-line two-layout runs were extended BEFORE any helper/test/production
migration to include the callback fixture. The first134-line five-bar variant
defined13/47 before4/100: native order13/47/4/100/MAX exposes the already tracked
#6152 structural-order boundary, not a conversion failure. Preserve its source/
output as order-observation.prg/out, SHA-256
`390bb707d86eec85801a67b659da57b1ff417b975f51d4ecdbbedf812301fb24` /
`85c7e3d1a07d0cab61a16022a4e575fdb73577674b7adc6ffa7322517ced0f28`.
Final conversion controls deliberately align definition order and numeric keys;
this neither repairs nor rationalizes the remaining structural-order gap.

Separate #7096 Access365 installed-COM fractional/native-error evidence is
read-only provenance, source/output SHA-256
`8c9089282ddcf81a50f1110840c39e98f009b27127cdbfe1d768c55522d51393` /
`e9bf4bb4f88a093ebc95c0cc7d59b44bd8e45e912562cae3fe6887f03767b722`.
Audit owns those files. New wide evidence is installed Wine, not a new Windows
VM/GUI/platform/release qualification run. Generated FXP recoverably trashed.

## Governing requirement mapped BEFORE migration

RQ-CF-PRG-GETBAR-NUMERIC-001 derives default finite truncation into1..INT32_MAX,
exact-integer precision and nonfinite containment from parent
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 (#5611/#6776). Explicit VFP9 Numeric/exact
integers use independently observed both-sign signed-low32 positive aliases.
Other accepted coercions retain checked signed64 half-away rounding. Unsafe or
absent positions keep this bounded slice's existing zero fallback; native1612
is a recorded compatibility gap, not a recovered zero-return requirement.

Shipped GETBAR help topic ad12f85b-767f-4a7a-9b65-c6f26364fcd5 was read fully:
position1..count returns the bar identifier occupying that position. Input is
not a bar identifier. Preserve integer/sparse order, lookup, selected session,
cursor, skip/mark state and registered callbacks. System menus, placement/
reordering, missing-popup/type/error policy are outside this conversion slice.

Architecture: checked_getbar_position_argument delegates independently matched
checked MRKBAR conversion. Only the GETBAR selected-session callback migrated
after BOTH frozen original compiler baselines; count comparison no longer
narrows size to signed64. Lookup, integer/sparse order and zero fallback remain.
VR-5611-GETBAR-NUMERIC-001/002:112 direct checks/468 guarded public queries/
148 inactive singleton/sparse cases and three unchanged old neighbors, frozen
before first configuration in [baseline-audit.md](baseline-audit.md).
Each original compiler has exactly172 identical selected identity failures
(144 guarded/28 inactive), zero unrelated/direct/setup/state/callback/cleanup
assertions; old neighbors pass. GNU total5.76s, Clang sanitizers6.21s with no
diagnostics. Full original/fixed logs and source/helper/test/CMake hashes
retained; no assertion/helper/CMake repairs, removals or weakening.
Fixed GNU14/14 PASS455.76s (GETBAR5.58/neighbors0.11), including shared MRKBAR/
SKPBAR/PRMBAR, older Numeric/localization and seven repository contracts
(safety traceability340.92s). Actual durations retained without guessed cause.
Fixed Clang ASan/UBSan/float-cast-overflow5/5 PASS18.84s (GETBAR5.59/neighbors0.09),
shared queries included, no diagnostics/detect_leaks0. All runs serial/private
TMPDIR and one-job builds, no overlap/VM. Bounded requirement is defined;
native1612/order/type/system-menu and full parent/platform/release acceptance
remain unfinished, and exact-head hosted review/checks remain separate gates.

## Development assurance boundary

DQ/DV-5611-GETBAR-NUMERIC-001: bounded medium documentation-misuse severity.
Delta: safe default position conversion versus explicit legacy aliases, not
bar-identifier input, setter saturation or complete native errors/order. Wrong
interpretation could choose a wrong action. HZ-runtime-crash-01/HZ-data-
corruption-01/HZ-doc-command-01 remain active at registered severity.
Controls: checked conversion, independent identity/state/cleanup verification,
explicit residual gaps and narrow rollback. Development maintainer self-review
under the owner's development policy completed2026-10-08: compared the delta
with installed observations, shipped help, parent policy and fixed evidence;
confirmed only conversion/count comparison changed, not lookup or actions.
Automated walkthrough passed in both fixed builds: aligned five-bar1.9 returns4,
2.9 returns13,3.9 returns47,4.9 returns100 and5.9 returnsMAX. Both-sign wide
aliases select1/2/3 only in explicit VFP9; the default keeps zero fallback.
Singleton/sparse endpoints, missing-popup zero and selected-session2 aliases
are independently guarded. Direct nonfinite/exact64/Character/Logical checks
pass. Public guards preserve count/prompts/cursor/session/mode/skip/mark events
and independently dispatch registered13/47/MAX actions afterward with expected
disabled suppression. Query read-only behavior is also checked by code review;
tests verify later callback identities, not an independent callback-invocation
count during the query. Cleanup/release/reset passed. No independent-human/
high-hazard sign-off, native1612 repair or leak/GUI/platform/release claim.
Rollback reverts only
this helper/callback/tests/docs, retains other Numeric work, publishes corrected
scope/evidence in same PR/release fragment if needed. Hosted exact-head checks,
Claude-first/authorized Codex clean review and resolved conversations are gates.
