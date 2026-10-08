# SET MARK OF BAR Numeric literals (#5611/#6776/#5868)

RQ-CF-PRG-SET-MARK-BAR-NUMERIC-001 derives from parent
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner safety policy and independent VFP9.
Mapped BEFORE helper/production migration; bounded requirement now defined.
Scope: existing parsed Numeric-literal identifier only.

Default finite truncation accepts converted1..INT32_MAX; other admitted
Numeric values raise localized catchable167 before flag evaluation/mark-state/
success telemetry. Explicit VFP9 finite raw>=2^32 saturatesMAX; lower inputs
use defined signed-low32 conversion, preserving negative-wide positive aliases.
Converted-1/-2 raises1612 in the supported user-popup lane, unlike SET SKIP's
successful user-bar no-op. Other nonpositive values raise167. No system-menu
support inferred. Nonfinite helper containment derives from owner safety
policy; unchanged literal parser does not admit NaN/inf/overflowed exponents.

Three unchanged serial installed VFP9 7423 runs match125 lines/40operands
x singleton seeds1/2/MAX, exactly one RELATIVE bar/noactivation. Paired .T./.F.
trials reset the known seed to its opposite state; MRKBAR receives the known
integer seed, not the operand. Fractions select truncated matching keys;
negative-wide1/2 aliases and positive>=2^32 MAX are independently distinguished.
SourceSHA256 059b287f2dde795e3719f4ff906b19bead220ce95d777ae0d9b00a1e8a668081.
OutputSHA256 416c7895fa8bbdad7b89085dace1b7fe45fd2f1ea2c1214993e16e40c544aa82.
Shipped help64e4e2e2-84c7-4d60-b814-74ad4606e25b read fully: mark/clear
the numbered item using Logical TO expression; dynamic popup PROMPT clauses,
MENU/POPUP variants, rendering and system items are excluded.
Native contradicts #5868's blanket fractional-rejection expectation.
General missing-positive-target1612 gap #7093 against main
53f66b4f9686e98a92c33cf6fa672526a6826719 stays separate: positive state
lookup is not changed here. Source evidence for that issue is not a claimed
independent Copperfin runtime replay or authority to expand this slice.

VR-5611-SET-MARK-BAR-NUMERIC-001/002:70direct boundaries,
124guarded paired flags/current-session/cursor/prompts/count/skip/callback/
event/cleanup cases,32locale cases, three old neighbors. Independent fixed
skip policy (only bar2 disabled) and repeated host callback markers preserve
selection behavior while mark-state queries use only known constant keys.
Byte-identical original dispatcher372 selected failures per GCC/Clang,
zero unrelated/no sanitizer diagnostics; unchanged final helper/tests across
comparison. Full hashes/reconstruction and earlier formatting-only baseline
retained in baseline-audit.txt. Fixed GCC focused2/2 PASS2.24s
(numeric2.13/neighbors0.10), logSHA256
783b58f5e4950bb41684b025c189b3a91c74560d2f858c64c5f3519dabb1ac61.
Fixed GCC broader Numeric/localization/contracts11/11 PASS459.68s
(numeric2.15/neighbors0.10), logSHA256
11ff81f384be8393ecda57a1f667465def6c3abca2229937670a3795f6edb118.
Fixed Clang21.1.8 Debug -O1 ASan/UBSan/float-cast-overflow2/2 PASS2.68s
(numeric2.56/neighbors0.10), no diagnostics; detect_leaks0/abort1,
UBSANhalt1/stack1; logSHA256
6a7d3f17f48b53b1af5c932da43c9d12c6154ca18f76fc9106267ae310fbd4d5.
Runtime verification strictly serial/private TMPDIR, unchanged final tests.
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01.
DQ/DV-5611-SET-MARK-BAR-NUMERIC-001: medium misuse; procedural delta
identifier conversion only, not expression/target/routing/lifecycle policy.
Development maintainer self-review/walkthrough completed under owner
authorization against focused and broader/sanitizer results, not independent human review.
Walkthrough:1.9 .T. marks only1, not2; .F. clears only1 from an independently
marked baseline. Default0.5/2^31 raises167 and explicit VFP9 -1/4294967295
raises1612 before mutation or success telemetry, preserving initial mark
flags, skip policy, prompts/count/cursor/session and stable action callbacks.
Legacy4294967297 targetsMAX, not1; negative-wide aliases target1/2.
Four locales agree on ERROR/AERROR/ErrorNo/message. Host selections repeated
twice per known key, bar2 suppressed only by independently seeded skip state;
opposite-origin session2 uses selected mode. Explicit release/cursor close/
reset precedes test-owned cancel/reap; no #4630 lifecycle fix.
Self-review traced native/help/requirement/helper/single site/test expectations,
checked-before-flag/state/event order, separate negative1612 admission,
preserved accepted expression evaluation/unparsed syntax, bounded native
allocation, unchanged general positive lookup and rollback/field notice.
Bounded development acceptance complete; required host CI, exact-head external
review and resolved-conversation merge gates remain separate.
Rollback isolated helper/site/catalog/test delta; field notice truncation,
default167 and opt-in aliases/MAX/negative1612. No full type/GUI/leak/platform/
release qualification claim.
