# SKIP count conversion (#5611/#6776)

## Retained pre-implementation recovery

RQ-CF-PRG-SKIP-COUNT-NUMERIC-001 derives from admitted
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner #6776 default/explicit-quirk policy,
and HZ-runtime-crash-01/HZ-data-corruption-01. Base main:
6b584de65d1cb60e4c90829edc63a6195b99499d. Only skip_command's count
llround conversion is selected; current implementation is not a requirement
source. At this recovery point no production change or Copperfin acceptance
evidence existed; the implementing boundary and completed evidence follow.

Three final strictly serial installed VFP9 09.00.0000.7423 probe.prg runs
match all100 lines:98 count cases, VERSION and cleanup.49 operands each
exercise ordinary selected and explicit-IN cursors. Both signs of fractions
truncate toward zero, including +/-0.5 doing no movement. Positive overflow
2147483648 moves toward BOF;4294967295 aliases-1,4294967296 aliases0,
4294967297 aliases1. Negative oversized counts wrap in the opposite direction:
-4294967295 aliases1 and-4294967297 aliases-1. Finite +/-1E300 and the
overflowing multiplication +/-1E300*1E300 do no movement, consistent with
the shared signed-low32/indefinite-zero model. Inside signed64,
+/-9223372036854774784 retains a nonzero count direction; the signed64
overflow boundary +/-9223372036854775808 does no movement. Saturated BOF/EOF
observations distinguish count direction, not every interior count bit.
Currency/Character/Boolean/NULL reject native10 without movement; these
type rules are not production scope. Explicit-IN preserves selected OBSERVER
and data session1. Temporary cursors close.

Three final strictly serial pending.prg runs match all40 lines:36 count
cases across buffering4/5, VERSION and three cleanup lines. Starting pending
RECNO-2, truncated-1 selects pending-1; zero remains pending-2, positive1
moves EOF6. Positive4294967295 and negative-4294967297 both select pending-1;
4294967296 and1E300 preserve pending-2. Currency rejects10 without movement.
Payload/count/BOF/EOF/observer/session are retained. TABLEREVERT and cursor
close complete; MULTILOCKS returnsOFF. The unchanged Wine wrapper is used.
No decompiled binary, local VM, external DBF/ODBC/network or system changes,
overlap or failed native batch. An earlier three matching47-line preliminary
probe had fewer operands/no explicit-IN controls; it is superseded by the
final100-line fixture, not additional acceptance evidence.

SHA256:
- probe.prg:48c80adc145cdb3436155e9551df1014c6f2cb49789e99889f89ae0f7085ddf7
- probe.out:88a9b606d5c788eeb817864596ba56877d53846428f8fa932f25ec15a16b778e
- pending.prg:1327a6fba813e2a206575da5ef73a864e0dddb9ab59f8a1f0796e63879a95e8a
- pending.out:5c74ddc79873a62d24342699bb23b26a7bcb2854b4e8eff8ea42c93d04ac2da1

Reproduce each to completion before the next, including across build trees:
`/home/rich/bin/vfp9-probe /absolute/slice/tests/fixtures/vfp9-skip-count-numeric-observation/probe.prg`,
then pending.prg. Generated owned FXP is scratch, not evidence.

## Derived bounded plan

Use operation-specific checked signed32 count conversion for default
COPPERFIN: finite Numeric truncates; exact int64/uint64 compare without a
double round trip. Reject converted values outside INT32_MIN..INT32_MAX
before navigation. This default domain/error boundary is owner-derived
safer policy, not native rejection (native accepts wrapping). Its signed32
output also makes the existing widened long-long abs operation defined,
including INT32_MIN. Explicit VFP9 alone retains both-sign signed-low32
aliases and observed huge/infinite zero. Reject NaN both modes as a derived
non-native safety boundary; native NaN syntax is not recovered here.
Exact extended kinds retain their own low bits in VFP9 rather than round
through double, a Copperfin extension policy rather than native evidence.

Preserve existing non-Numeric half-away coercions only when checked
signed32 conversion succeeds; no native10/type-parity claim. Missing
conversion raises catchable localized11 with safely formatted original
operand before move_by_visible_records, dirty-row commits, relation/filter
evaluation and successful runtime.skip event. Error11 for default rejection
is derived invalid-argument policy; native Numeric count probes do not reject.
The planned helper was checked_skip_count_argument in helpers.h/.cpp.
Wire only skip_command's llround site after resumable operand evaluation
and generation revalidation, within the existing target-session selection.
Do not change default/missing syntax, routing, shared navigation, BOF/EOF
policy, filters/relations/buffering/lifetime, GO, UNLOCK or aggregate scopes.

Planned independent verification: both-mode direct signed32/signed64/adjacent-
double/exact/NaN/infinite checks; guarded main/explicit-IN, pending buffering4/5,
zero/default and selected alias/session/mode/count/payload controls; rejected
dirty-row buffering2/3 disk persistence and successful-event suppression;
safe four-locale diagnostics. Run new guarded tests on byte-identical
original dispatcher first, retain selected failures and independent type/
navigation gap controls, then fixed GCC plus focused Clang
ASan/UBSan/float-cast-overflow strictly serial with privateTMPDIR.
Reuse GO navigation/negative-buffer neighbors and full older Numeric normal
suite; record contracts and complete traceability before PR/closure.
If actual main execution confirms a distinct type/navigation mismatch,
file a focused issue against the exact base without production expansion.

DQ-SKIPCOUNT-001: distinguish observed Numeric conversion from type/navigation
parity and explicit Copperfin defaults/extensions. DQ-SKIPCOUNT-002: preserve
reverse requirement/code/test links, exact baseline/acceptance hashes,
serial reproduction, cleanup and slice-only rollback/field notice.
Misuse severity medium: applying GO's positive-only alias rule to SKIP changes
legitimate backwards movement; confusing public pending identities with
physical delta changes buffered navigation; unchecked INT64_MIN abs is a
runtime availability hazard. Procedural delta replaces only conversion,
retaining shared navigation. Rollback reverts this slice's helper/dispatch/
locale wiring and retains the regression tests exposing unsafe conversion.
Field notice uses the PR and changelog. At initial recovery VR/DV completion
was pending; completed development evidence follows. No independent-human,
platform, leak, full native/type/navigation or release qualification is claimed.

## Implementing boundary and original-dispatch evidence

checked_skip_count_argument in helpers.h/.cpp implements the planned
operation-specific signed32 count. Only skip_command dispatch changed,
after existing resumable evaluation/generation revalidation and within
target-session RAII selection. Missing results throw localized11 before
the existing navigation call; four catalogs preserve original operand text.
Numeric diagnostics use format_round_trip_decimal, not an unchecked
integral-formatting path. Existing shared navigation remains byte-unchanged.

baseline-audit.txt retains original dispatcher blob
b3a9d865739cc2c766fd3647b6afc24d6cd8d5a6, source/test/helper/raw-log hashes
and exact compiler outcomes against main6b584de65. Both compilers fail exactly
292 selected assertions:272 guarded rows/12 event counts/four dirty snapshots/
four dirty events.98 helper checks, all suffix/setup/cleanup controls,
four dirty-row disk controls and four existing neighbors pass.
GCC older Numeric passes87.38s, total88.87s. Focused Clang total8.29s
reports signed-negation UB at unchanged records.inl:728 because huge SKIP
counts reach LLONG_MIN. Baseline alone uses UBSAN halt_on_error0 to retain
negative execution; fixed acceptance switches to halt_on_error1.
This is addressed at the selected count boundary, not by modifying shared abs.
Actual Currency/Character success versus native10 is OPEN/unadmitted#7083.
No preliminary harness failures or expectation corrections affect this audit.

## Completed development review and walkthrough

DV-SKIPCOUNT-001: Codex in the owner-authorized maintainer workflow completed
development self-review of native truncation, both-sign wrapped counts,
signed64 indefinite0 limits, explicit exact-kind/NaN extension boundaries,
range-before-narrowing and widened INT32_MIN abs safety.98 independent helper
constants do not reuse the implementation as their oracle. No type admission,
native rejection/error11 or saturated-pointer bit-precision inference is
misrepresented as native evidence. Both-sign aliases are SKIP-specific; GO's
positive-only rule is not reused. No argument-sized allocation or additional
navigation loop is introduced.

DV-SKIPCOUNT-002: guarded walkthrough completed480 both-mode selected/IN
count cases on ordinary and buffering4/5 pending rows. Snapshot checks retain
payload/public negative RECNO/count/BOF/EOF/alias/session/mode and cleanup;
invalid count paths suppress successful SKIP events. Four dirty buffering2/3
sessions retain pending payload and unchanged disk records, then revert/close.
Twelve locale sessions verify11/ERROR/AERROR agreement and safe original
Numeric301-digit decimal or Character text. Two default/zero sessions retain
omitted count syntax. Four existing neighbors retain reentrant replacement,
filter-driven closure and pending identities/partial update behavior.
All native/runtime runs are strictly serial across builds with privateTMPDIR.

DV-SKIPCOUNT-003: completed rollback walkthrough specifies a reviewed revert
limited to this helper/dispatch/catalog wiring, retaining independent regression
tests and native evidence; repeat named Numeric/new/neighbor/locale/contracts
checks and publish correction through PR/changelog. Never reset unrelated work,
delete buffered user data or alter shared navigation. Medium misuse remains
confusing type/count admission, public pending identities and physical deltas.
This is development maintainer self-review, not independent-human review.
DQ-SKIPCOUNT-001 maps DV-001/-002 and VR-001/-002; DQ-SKIPCOUNT-002 maps
DV-003/VR-003. docs32 retains reverse architecture/code/test/evidence links.

## Local acceptance and reproduction

VR-SKIPCOUNT-001:98 independent helper checks pass signed32/signed64 adjacent
doubles, exact64 precision/extrema, NaN/infinities and retained coercions.
VR-SKIPCOUNT-002:480 guarded rows, four dirty/disk sessions, twelve locale
sessions, two default/zero sessions and four existing neighbors pass.
VR-SKIPCOUNT-003: GCC15.2 Debug-O0-g0 passes4/4 in105.90s: full localization
15.66s, full older Numeric88.28s, new1.65s and neighbors0.30s.
Focused Clang21.1.8 ASan/UBSan/float-cast-overflow passes2/2 in8.25s:
new6.95s/neighbors1.29s, no diagnostics, ASAN detect_leaks0/abort1,
UBSAN halt_on_error1/print_stacktrace1. No Clang full older Numeric or leak
qualification is claimed. Six short repository contracts pass in16.07s:
issue intake, contributor signoff, changelog assembler/validity, locale
installation and native test isolation. Safety-workflow negative fixtures
pass362.67s; all seven contracts total378.76s.175 fragments validate.

Build targets test_prg_engine_skip_count_numeric/test_prg_engine_go_record_neighbors
and (GCC) test_prg_engine_numeric_behavior/test_localization in owned
/home/rich/temp/copperfin-skip-count-5611-build or ...-sanitize.
Clang flags:-O0-g0 -fsanitize=address,undefined,float-cast-overflow
-fno-omit-frame-pointer. Use private TMPDIR under the owned build and ctest-j1
with the exact named targets above; never overlap native/runtime execution,
including across builds. After merge trash only owned builds/generated FXPs
and remove the validated slice branch/worktree, retaining unrelated files,
stashes and tracked evidence. Hosted required checks, clean exact-head review
and resolved conversations remain merge gates; no merge/release completion.

Acceptance test/audit/GCC/Clang log SHA256:
```text
a9dd7e51208189fb972f13a6d27646b42a07bc5487271b3d0d34c820af1f1162  tests/test_prg_engine_skip_count_numeric.cpp
a12442fde50dc08a9a9bcb3b5ee292781f038dd8bd5ebae0528b05276d49a8bf  tests/fixtures/vfp9-skip-count-numeric-observation/baseline-audit.txt
49bb334632a2459cfde4a37227a2f8caf7499dbb2858f59af41cdbe31c8cb961  /home/rich/temp/copperfin-skip-count-5611-build/acceptance.log
e80dad5fb5e30792b94d7affb66abb037b5d6d49bed18570f9c7bf895f251111  /home/rich/temp/copperfin-skip-count-5611-sanitize/acceptance.log
```
