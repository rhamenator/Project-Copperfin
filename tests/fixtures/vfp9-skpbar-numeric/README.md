# SKPBAR Numeric identifiers (#5611/#6776/#5868)

RQ-CF-PRG-SKPBAR-NUMERIC-001 derives from parent
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner safety/default policy, admitted
first-pass #4617 and independent installed VFP9. Mapped BEFORE helper/
production migration; bounded conversion requirement, now verified below.

Scope: only existing SKPBAR callback second-argument conversion.
Default finite Numeric truncates into1..INT32_MAX; other Numeric yields
existing safe false fallback. Explicit VFP9 retains both-sign signed-low32
positive aliases, NOT SET SKIP positive>=2^32 saturation. Exact int64/uint64
Copperfin extensions derive from the parent: default checks1..MAX without
double precision loss; explicit VFP9 preserves exact low32 positive aliases.
Other existing coercions retain checked half-away signed64 rounding; native
type parity is not inferred. Nonfinite containment derives from owner safety
policy in both modes, not a native nonfinite-literal syntax claim.

Independent installed VFP9 7423 paired enabled/disabled singleton1/2/MAX
controls use only known integer SET SKIP seeds, one RELATIVE bar/noactivation.
40 operands/125 lines: positive4294967297 queries1, notMAX; negative-wide
aliases query1/2; ordinary fractions truncate. Three unchanged serial runs
match the same125-line output/source hashes below.
SourceSHA256432fec290b3d743e188b5347e49feb4328c8453acba8d01339f7629e9622efc4.
OutputSHA256050559431a92cd1e83c78de95abc0ed42b018c4c11f38397455e57e7bac7ed61.
Shipped SKPBAR help31c60e9b-b946-4acf-815d-bd2fda529c71 read fully: logical
enabled/disabled status of actual number assigned by DEFINE BAR.

Native missing/nonpositive lookup1612 conflicts with first-pass #4617 false
fallback. Focused #7098 against main89fd9ee5b6dcc14f2183cd893b2425340e49c243
is separate; existing fallback/lookup is preserved here. Source-path evidence
in that report is not a separately executed Copperfin general-lookup baseline.
No broad SKPBAR/native-error parity claim or new admission label. Type
admission, other queries, system menus, popup order/count#6152 and #4630
lifecycle remain separate.

VR-5611-SKPBAR-NUMERIC-001/002: 96 direct checks (58 Numeric, 6 nonfinite,
6 nextafter, 20 exact-integer, 6 other-coercion); 372 guarded cases (348 paired
identity patterns plus 24 opposite-origin session2 cases), three old neighbors.
Known integer skip seeds are independent of query operands; opposite skip
patterns on1/2/MAX expose wrong identity. Mark only2, repeat host callbacks,
preserve prompts/count/cursor/session, zero skip/mark mutation/events and
explicit cleanup. Original callback: exactly55 selected identity failures on
each compiler, zero unrelated failures and no runtime sanitizer diagnostics.
Every failure and source/test/helper/log hash is retained in the
[baseline audit](baseline-audit.md); no assertion removed or weakened.
Fixed GCC15.2 Debug12/12 PASS468.93s (SKPBAR4.34s/neighbors0.11s/shared
MRKBAR4.34s), including numeric/localization and six workflow contracts.
LogSHA2567d93d50e581959d494aa1b1e2202a0b15658f996157a7d66444f7995fd5ec2b4.
Clang21.1.8 -O1 ASan/UBSan/float-cast-overflow3/3 PASS95.56s
(SKPBAR61.53s/neighbors0.10s/shared MRKBAR33.92s), no diagnostics.
LogSHA25671b8d247db133dbc585702c9c8e01a6c11154b16d4fc38bd6504e0aba51f3d99.
Unchanged repeat3/3 PASS9.03s (SKPBAR4.44s/neighbors0.10s/MRKBAR4.48s),
no diagnostics; the slower first pass is retained, not replaced or attributed
to an unproven cause. Repeat logSHA256
5e712cc20c9e2a8a6a6c7e1911b57289c4a58c0468bad3961874437eb6f605da.
ASAN_OPTIONS=detect_leaks=0:abort_on_error=1;
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1.
Runtime/native strictly serial/private TMPDIR; tests/helper unchanged before/
after migration. CMake/isolation frozen from configuration through all runs.
Fixed callback sourceSHA256
9ec338536657ab687ea297225ae96fb672cbac32529f8a16eecff80bf3cb6acc.
No new localized message/catalog change; shared MRKBAR implementation unchanged.
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01.
DQ/DV-5611-SKPBAR-NUMERIC-001 medium misuse: procedural delta numeric
conversion only; preserved false fallback explicitly distinct from native1612
gap. Development self-review complete: compared independent singleton query
controls, helper bounds/exact low bits, selected-session wiring and every
original failure against frozen expectations; reviewed scope exclusions and
bidirectional requirement/code/test/result links. Automated acceptance above
complete, not independent human review. Walkthrough complete: independently
enabled/disabled targets queried at1.9 resolve1; default4294967297 safely
false, explicit VFP9 resolves1 without changing skip/mark/callback/cursor
state; missing/nonpositive still yields existing false, NOT native1612.
Rollback isolated helper/callback/tests; field notice default truncation/safe
false and explicit both-sign aliases. No independent human/full native-error/
type/system-menu/GUI/leak/platform/release qualification claim.
Hosted CI, exact-head external review and resolved conversations remain PR
merge gates. Bounded RQ-CF-PRG-SKPBAR-NUMERIC-001 defined; umbrella and
general lookup work stay open.
