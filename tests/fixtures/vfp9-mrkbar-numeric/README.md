# MRKBAR Numeric identifiers (#5611/#6776/#5868)

RQ-CF-PRG-MRKBAR-NUMERIC-001 derives from parent
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner safety/default policy, admitted
first-pass #4618 and independent installed VFP9. Mapped BEFORE helper/
production migration; bounded conversion/verification gap.

Scope: only existing MRKBAR callback's second-argument conversion.
Default finite Numeric truncates into1..INT32_MAX; other Numeric yields the
existing safe false fallback. Explicit VFP9 retains both-sign signed-low32
positive aliases, NOT SET MARK's positive>=2^32 saturation.
Exact int64/uint64 Copperfin extensions derive from the same parent policy:
check without double precision loss, admit1..MAX by default and preserve
exact low32 aliases only in explicit VFP9. Other existing coercions retain
checked half-away signed64 rounding; native type parity is not inferred.
Nonfinite containment derives from owner safety policy in both modes.

Three unchanged serial VFP9 7423 runs match125lines/40operands x singleton
1/2/MAX, exactly one RELATIVE bar/noactivation. SET MARK uses known integer
seed; MRKBAR operands query paired unmarked/marked states. Positive4294967297
selects1 (notMAX), negative-wide aliases select1/2, fractions truncate.
SourceSHA256 bf93a8387cf1172a3fb7e0ad1bda7ccbe58142f06610721ffac983e50cd1075a.
OutputSHA256 0ba923a5a2ad36470a7ea2592cf06cf29c55fb720f1d4c2162436bff087acf44.
Shipped help7f9a8ff8-b97d-4502-be18-98ef2f4a3d22 read fully: logical mark
query of defined bar; system-menu string alternative excluded.

Native missing/nonpositive lookup1612 contradicts first-pass #4618's false
fallback. Recovery gap #7095 against main1a7dd8a2a6fa9c62a50d7b46356c008776ac7b55
is separate; existing fallback/lookup is preserved here. Source-path evidence
in that report is not an independent Copperfin runtime baseline claim.
No broad MRKBAR/native-error parity claim; no admission label manufactured.
Type admission, other queries, popup order/count#6152, system menus and
#4630 lifecycle remain separate.

VR-5611-MRKBAR-NUMERIC-001/002: 96 direct checks (58 Numeric,6 nonfinite,
6 nextafter,20 exact-integer,6 other-coercion);372 guarded cases (348 paired
identity patterns plus24 opposite-origin session2 cases), three old neighbors.
Known integer mark seeds are independent of query operands; opposite marks
on1/2/MAX expose wrong identity. Skip only2, repeat host callbacks, preserve
prompts/count/cursor/session, zero mark mutation/events and explicit cleanup.
The unchanged original callback has exactly55 selected identity failures per
compiler, zero unrelated failures/diagnostics; all failures and source/test/
helper/log hashes retained in [baseline audit](baseline-audit.md).
Fixed GCC15.2 Debug11/11 PASS485.79s (numeric5.57s/neighbors0.15s);
logSHA256 7f51711bac9c71c978b92de9702231dd0d67c5a359ae0210c84101d4c5f38a2a.
Clang21.1.8 -O1 ASan/UBSan/float-cast-overflow2/2 PASS4.99s, no diagnostics;
logSHA256 5dc354647bd0b29884eae56bc28b4c1918cdbc484f659a63cc4c3a7bc4a67a5c.
ASAN_OPTIONS=detect_leaks=0:abort_on_error=1;
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. Runtime/native strictly
serial/private TMPDIR; frozen tests/helper unchanged before/after migration.
No new localized message/catalog change; broader numeric/localization and
intake/signoff/changelog/isolation/safety workflow contracts passed.
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01.
DQ/DV-5611-MRKBAR-NUMERIC-001 medium misuse: procedural delta numeric
conversion only; stable false lookup policy explicitly distinct from native
1612 gap. Development self-review complete: compared independent singleton
query controls, helper bounds/exact low bits, selected-session wiring and
every original failure against frozen expectations; reviewed scope exclusions
and bidirectional requirement/code/test/result links. Automated acceptance
above completed, not independent human review. Walkthrough complete: known
marked/unmarked targets queried at1.9 resolve1; default4294967297 safely false,
explicit VFP9 resolves1 without changing mark/skip/callback/cursor state;
missing/nonpositive still returns existing false, NOT native1612.
Rollback isolated helper/callback/tests; field notice truncation/default safe
false and opt-in both-sign aliases. No independent human/full type/GUI/leak/
platform/release qualification claim. Hosted CI, exact-head external review
and resolved conversations remain separate PR merge gates. Bounded
RQ-CF-PRG-MRKBAR-NUMERIC-001 defined; umbrella/general lookup work stays open.
