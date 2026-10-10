# Frozen unsigned64 CAST baseline audit

Frozen2026-10-10T03:49Z BEFORE configure/build and before production migration,
from origin/main e2c16c3c22bbd08bece2c137af9c7dc9f0d8e5d6. Requirement/native/
independent math mapping precedes the distinct unused helper and new tests.
Native four observations completed before helper/tests; see README chronology.

Independent literal count: 33 Numeric rows + NaN/three nextafter calls + nine
exact unsigned/ten exact signed + six preservation controls =62 direct.
Public per alias:33 Numeric +12 exact +three coercions +one nonnumeric error1
preservation +four literal catalogs =53; threealiases =159 fresh cases per
mode/session shard. Four public shards =636; direct62 +public636 =698 new cases.
Each public call checks catchability/assignment, exact kind/value, continuation,
mode/session/cursor state and reset; locale checks add message equality.
NaN is direct-only; public EXP(1000) and its negation cover infinities.
Existing signed64 CAST five shards and exact-expression target stay unchanged.

Frozen hashes (SHA-256):

| File | Hash |
| --- | --- |
| src/runtime/prg_engine_helpers.h | 07d2b7c5ee5107e0b628d6aae79a1833e1b1310ffa0d0b5b9dc461a47b2f84d7 |
| src/runtime/prg_engine_helpers.cpp | a2fc1bf0b113428d36c5f710612b3e6dfc83c7c08405c9e38bae82691bda7181 |
| tests/test_prg_engine_cast_uint64_numeric.cpp | 68d91603f859642e204e1186bebbcd3595712deafa2d2a8f4371e4b8d0b6cf04 |
| tests/CMakeLists.txt | 7be1de435f321728aeb858252827b5f57630747e60328243b7f8e78de71d1871 |
| tests/CopperfinTestIsolation.cmake | 53142fa0225cc1efe5b58a81e4c627a546ec05a65ccf2c2f5984fdd4eadc865e |
| ORIGINAL general dispatcher | 6797a16d57edf25f1d6c9b6d3e3d980c6a21d2e1ed311facf5eee76f21adf71d |
| UNCHANGED platform helpers | 639778a08a38ef7d89a53d8f04a7daec99faf32c0077867cb9cf12a277225ffe |

Serial local execution: nice10/ionice2:7, build-j1, private RAM TMPDIR for
query fixtures. GNU Debug-O0-g0. Clang21.1.8 Debug-O1-g0, matching C/CXX
ASan/UBSan/float-cast-overflow/frame-pointer instrumentation and EXE/SHARED
sanitizer link flags; explicit float-cast-overflow recovery retained in binary.
Original UBSAN halt0 permits complete negative evidence; fixed halt1 rejects
any diagnostic. ASAN abort1/halt1/detect_leaks0, so no leak acceptance claim.
Focused600s and broader1200s command caps are operational bounds, not changes
to CTest/workflow timeouts or measured-optimum qualification.

At freeze, both complete originals and fixed/broader verification were pending;
no migration, sign-off, sanitizer cleanliness or acceptance was claimed then.

## Complete GNU original before Clang configuration or migration

Configure18811 exit0; build/CTest73625 completed build0/CTest8, harvested03:59Z.
All62 direct and4x159 public new counters complete; all56/4x141 unchanged
signed64 counters complete, plus exact-expression neighbor PASS. Seven of11
CTest targets PASS; only the four new public shards fail as expected.
Exactly960 selected assertions (276 error,276 assignment-kind,360 result,
48 localized-message) and zero other failures. No input/assertion/helper repair
or weakening; frozen hashes unchanged; dispatcher remains original.
Full unedited1413-line original-gcc.log retained byte-identical BEFORE Clang
configuration/migration, SHA-256:
c4d35e0e5322ba5e0617fa9c424a25deedde5239049aa266c6be2f7adba74b13.
CTest15.24s, complete negative evidence, not acceptance. Clang original follows.

## Complete Clang original BEFORE migration

Clang3372 configure0/build0/CTest8, harvested04:11Z. All62/4x159 new and
56/4x141 old signed64 counters complete; exact-expression neighbor PASS.
Same960 selected assertions/zero other as GNU,21.74s. Four actual UBSan
float-cast-overflow reports at original unsigned branch line677:65 for -1;
recovery permits completion. This diagnostic-bearing run is NOT clean proof.
Full unedited1532-line original-clang.log retained byte-identical, SHA-256:
cd09a9a6c1845458ab98b8c2f18efb76c37d12bf154343dca1af1a64e37d2ca2.
Both complete original logs are retained BEFORE selected migration; all frozen
hashes unchanged. No assertion/input/helper repair/removal/weakening occurred.

## Migration and fixed GNU

Only after retaining BOTH complete originals did the unsigned64 branch migrate
to the frozen helper/existing localized catchable11. Signed64/other CAST,
shared helpers, parser and arithmetic are unchanged. Fixed dispatcher SHA-256:
6dd9d79d194c21e6e869e5fd612c5c8d107eb226620d4b097fc7d92b3507fdaf.
GNU64076 build0/CTest0,11/11PASS15.93s, actual62/4x159 new and56/4x141 old
counters complete/exact-expression PASS; no test/input/helper changes.
Full unedited194-line fixed-gcc.log retained byte-identical, SHA-256:
0c09a643a20ae77f23a89808d27c3b84110edeaef861a35652d8b804c2f5a212.
Fixed Clang/broader/assurance/integration evidence remains pending.

Fixed Clang38724 build0/CTest0,11/11PASS19.71s with frozen instrumentation and
UBSAN halt1/ASAN abort1/detect_leaks0. All62/4x159 new and56/4x141 old counters
complete/exact-expression PASS; no sanitizer diagnostics or unexpected failures.
Full unedited195-line fixed-clang.log retained byte-identical, SHA-256:
dd52dce07a8762c85b12a818c31136f1afd497c876ca42d260e30f94cb7fcc07.
Frozen test/helper/header/CMake/isolation hashes remain unchanged. Broader/
assurance/integration gates pending; no full-platform/persistence/leak acceptance.

## Broader GNU and contracts

GNU71727 build0/CTest0,12/12PASS425.47s: unchanged Numeric/NULL/typed-NULL/
string-math/localization plus all seven locale-install/changelog assembler/
fragments/PowerShell-discovery/isolation/safety-workflow/portable-CLR contracts.
Full unedited294-line broader-gcc.log retained byte-identical, SHA-256:
8ec2a687bc8ef8764a2a98467c991562b6df87c302e970105f70bded144f1b45.
The locale-install contract's expected missing-pt-BR negative subprocess CMake
diagnostic is retained, not an unexpected CTest failure.
Broader Clang and integration remain pending. No CTest/workflow timeout changes.

Broader Clang26270 build0/CTest0,4/4PASS94.61s for the four unchanged runtime
neighbors, same ASan/UBSan/float-cast-overflow flags/halt1/detect_leaks0; no
sanitizer diagnostics or unexpected failures. Full unedited123-line
broader-clang.log retained byte-identical, SHA-256:
f446e28fc8a8ba3c2e4ae44d92f96a0bbab85f3d05577e70cf72862c75732f27.
Focused/broader local verification is complete; integration gates remain.

## Bounded documentation development review / walkthrough / rollback

DQ/DV-5611-CAST-UINT64-NUMERIC-001: Codex delegated development self-review,
not independent human or high-hazard qualification. Inspected finite-before-
truncation and truncated-domain checks before the Numeric cast, negative exact
signed rejection before unsigned cast, full exact unsigned identity, dispatcher
error11 before return/assignment, unchanged signed64 and other branches, and
reverse RQ/test links. Literal endpoint/fraction/constructor expectations match
the pre-implementation integer/binary64 math and explicit owner-derived policy,
not outputs used to redefine requirements. No frozen input/assertion changes.

Automated walkthrough, across all3aliases/both modes/sessions1/2:

| Step / initial state | Expected operator-visible outcome | Retained result |
| --- | --- | --- |
| Sentinel12345, cast Numeric -0.9 | uint64 zero; no error, guard/cursor/mode intact | GNU64076 and Clang38724 PASS, all frozen62/4x159 counters complete |
| Sentinel12345, cast Numeric -1 or exact signed -1 | catchable localized11, Number sentinel unchanged, subsequent guard1 | Same complete GNU/Clang focused logs PASS |
| Construct exact unsigned UINT64_MAX from18446744073709549568+2047, re-CAST | uint64 18446744073709551615, no Double precision loss | Same complete focused logs PASS; independently derived literal expected value |
| Cast Numeric spelling18446744073709551615 (rounds2^64),2^64,huge,infinity | error11 before assignment; sentinel retained; execution continues | Same complete focused logs PASS; direct NaN/nextafter also PASS |
| Re-CAST exact9007199254740993; change catalogs and run context/reset checks | Low bit retained, four exact localized messages; guard/cursor/session preserved then cleanup/reset1/COPPERFIN | Same complete focused logs PASS; broader GNU12/12 and Clang4/4 PASS |

Procedural delta/misuse severity: medium, limited to this extension's conversion
guidance. No new backup/recovery/external-process/operator-destructive procedure;
registered high/catastrophic hazards remain active, not reclassified or closed.
Rollback remains a scoped revert of this slice with original/fixed/native
evidence retained, followed by correction of conversion guidance and integrator
notification if published guidance was wrong. This completes the bounded
development self-review/automated walkthrough/rollback plan, not independent
human sign-off, platform/persistence/leak/release or family/parent acceptance.
Exact signed-head hosted checks, clean review and resolved conversations still
gate integration; post-push focused evidence follows the signed commit.
