# RemoveItem numeric row admission: independent pre-code recovery

Map recorded 2026-10-12T17:53:05Z BEFORE helper/assertions/CMake/isolation
configuration or production migration. Base origin/main
6fe579e32dcf80e77e0c66e9b07845c908ebecef; OPEN/rhamenator/exact agent-approved
#5611/#6776 metadata revalidated at this turn's intake. Dedicated branch
fix/removeitem-numeric-5611. Existing code is verification evidence ONLY.

## Allowed independent evidence

Shipped VFP9SP2 RemoveItem help completely read at
/home/rich/temp/vfp9-help-len/html/995c88c6-f440-450a-a8a0-3cb639aa3414.htm,
SHA256 b65238ab069ac33feed2c85b6f21d35fd484895cc8d67b0c6bfb596146269308.
It applies to ComboBox/ListBox and defines nIndex as a 1-based integer in
display order, not the stable item ID. Full HTML remains local build evidence,
not proprietary content copied into product source.

Fresh installed Visual FoxPro 09.00.0000.7423 in the existing private Wine
prefix, using unchanged /home/rich/bin/vfp9-probe (snapshot retained), SHA256
1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06.
No VM start/recreation, credentials, OS/setup/installer change or decompilation;
VM cleanup/Veeam reset completion unconfirmed. Probe enumeration/snapshot
scaffold only is reusable; ALL RemoveItem observations are freshly measured.
Fresh object per argument, two-column cells, payload101/202/303, focus2,
ListBox top2 and single/multiple selection; dense1/2/3 vs sparse11/22/33
identities, empty and populated controls. No sorted/GUI/event completeness.

All three fresh matching pairs returned runner0/cmp0/complete DONE:

| Pair | Calls/full lines | Identical full raw SHA256 |
| --- | --- | --- |
| main | 423/425 | f1de30dbed06f009a1b50c388ba3dfccb8eacf6f6a22c34549e86142cb78cba0 |
| boundary | 297/299 | ae7ea531f3831f11fa18970440a865e37b319119aa20074be660a8890703e229 |
| argument range | 180/182 | b8a9f04a9e6a5afc97c24650f34efd2b8103e5dee2e00edc115e89e1180916e3 |

Native ordinary fractions1.4/1.5/1.9 remove row1,2.4/2.5/2.9 row2,
3.9 row3 IN BOTH identity shapes. Surviving rows retain their IDs, both
columns and ItemData in display order. Zero/-0.9/negative ordinary/huge/
both infinities error11 without removal. Projected positive1..32768 is the
argument domain: 32767.1/.9 and32768/.9 pass argument admission then raise
1234 for an absent row, whereas32769/.9 and larger projected ordinals raise11.
Valid-domain row4/4.9/11/22.9 raises1234 in three rows; empty valid-domain
requests also1234. Rejections preserve the entire native state snapshot.

Native signed-low32 aliases are observable for BOTH signs, unlike TopItemID's
raw-positive ceiling: positive4294967297,4294967298.9,4294967299,8589934593
remove row1/2/3/1; negative-4294967295.9/-4294967294.5/-4294967293 remove1/2/3;
-8589934590/-8589934591.9 remove2/1. Projected32768 from4295000064 or
-4294934528 gives1234; projected65535 from4295032831/-4294901761 gives11.
Negative-9007199254740991 removes1; +/-9007199254740992 and huge/infinite
inputs give11. Intermediate bit arithmetic and exact64/NaN are named
parent-derived extension rules, not native CAST/NaN syntax or expectations
from a sibling implementation.

## Governing requirement and bounded architecture

RQ-CF-PRG-REMOVEITEM-ROW-NUMERIC-001 parents: admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001; independent help/native observations above;
HZ-runtime-crash-01/HZ-data-corruption-01 containment. Scope is ONE supplied
non-NULL Numeric/int64/uint64 first argument on materialized/unbound native
ListBox/ComboBox with RowSourceType0 (or absent default0), emptyControlSource
and coherent keys/collection/row state. Omitted, extra-argument arity, other
types/RowSource/bound routes, RemoveListItem, AddItem/AddListItem, shared
removal synchronization, return values, events, parser and lifecycle excluded.

COPPERFIN default: finite truncation/lossless exact admission to1..32768;
out-of-domain/nonfinite/NaN raises localized catchable11 before modeled
removal. Explicit VFP9: defined signed-low32 conversion followed by the same
1..32768 admission; observed huge/infinite projection is rejected11. NaN11
in BOTH modes is parent/hazard-derived. Exact int64/uint64 mode conversion
must avoid a Double round trip: e.g. INT64_MIN+2 and UINT64_MAX-4294967293
have low32=2, default11/VFP9 remove2. This is extension-only parent policy.
Admitted ordinal above materialized count raises localized1234 before
removal; valid ordinal selects display row (not ID) and delegates to the
UNCHANGED shared removal helper. No argument-sized allocation/loop, no new
host ceiling or whole-call/callback atomicity claim. Relative rejected-state
preservation and independent row/cell/ID/data removal are acceptance evidence;
successful focus/text/top/return parity is explicitly not claimed.

Distinct checked_removeitem_row_argument stays UNUSED through BOTH complete
original GNU and Clang ASan/UBSan/float-cast-overflow suites. Only afterwards
wire one bounded arm in invoke_native_list_control_method, which already
receives current-session NumericBehavior; no signature/parser expansion.
Preserve existing materialization-before-invocation order and excluded
callback behavior. Separate indexed property reads remain unchanged.

VR-5611-REMOVEITEM-ROW-NUMERIC-001: independently enumerated direct literals
for both modes, fractions/domain transitions, both-sign wraps, exact64,
nextafter/subnormal/NaN/infinities and unsupported helper guards.
VR-5611-REMOVEITEM-ROW-NUMERIC-002: both classes/modes/two sessions/dense+sparse/
empty/multi; literal admission/error/row-count/surviving IDs/two-column cells/
payload, rejected before-after state, current-session/guard/cursor/reset,
continuation and four locale messages. Freeze helper/tests/config hashes;
retain all65 completed numeric/neighbor subjects plusfive new and both
original full raw runs/status/counters before production migration. Any
frozen repair requires explicit refreeze and BOTH originals repeated.
Fixed repeated identical suites, unchanged excluded-scope preservation,
documentation contracts, signed exact-head CI/review remain separate gates.

Fresh current-main excluded-scope preservation completed compile0/run0/
DONE144/scaffold0 before migration. Both controls/modes/sessions with RST1/2/5,
boundControlSource0, unsupported types and omitted/extra args; full raw and
source retained. These are existing-behavior preservation controls ONLY,
not native requirements. Both fixed compiler versions must match the entire
baseline raw, including #7163/#7164 excluded state/coercions; no input/oracle
regeneration. No absolute empty-state or unsupported-route parity claim.

## Independently exposed gaps: outside this slice

[#7163](https://github.com/rhamenator/Project-Copperfin/issues/7163) records
ordinary successful removal return/focus/selection/text/top differences.
Native returns Logical .T.; current main empty Character. Sparse ListBox
remove1 native focus1/ID22 but Value/DisplayValue third, top2/ID33; current
main focus1/ID22/second/top1-ID22. Remove2 native focus0/ID0/top2-ID33/bitsFF,
current main focus2/ID33/top1-ID11/bitsFT. Do NOT migrate shared state or
weaken numeric tests to hide this. Successful focus/text/top/return controls
are current-behavior preservation evidence ONLY, never native requirements.
[#7164](https://github.com/rhamenator/Project-Copperfin/issues/7164) records
native error27 for Logical/NULL/Character/Currency vs current coercions;
types remain excluded and preservation controls must be labeled non-native.
Both focused issues filed against exact main6fe579e32; no agent-approved
label added and no owner admission/family/hazard/release closure inferred.

Fresh current-main diagnostic28calls completed0/scaffold_failures0,
full rawSHA4a1490118d50e0fe5c80f76fd6ea100b92376f92294b452b30eb2189cc638c5d;
compile0/source/raw/status/full library hashes retained. It links retained
signed4355b990 GNU libraries with relevant source tree identical to merge
main6fe579e32; not a pristine whole-tree/archive-byte claim or native oracle.

## Documentation assurance / boundaries before code

DQ/DV-5611-REMOVEITEM-ROW-NUMERIC-001 links runtime/data/HZ-doc-command-01
without changing registered severity/status. Bounded misuse medium:
misaddressed removal can destroy modeled transient rows or misdirect a
downstream action, not a persistent-storage or high-hazard qualification.
Procedural delta/walkthrough: sparse2.9 removes row2/ID22, not roundedthird
or ID2;22.9 gives1234,0 gives11;32768.9 gives1234/32769 gives11; signed aliases
default11/VFP9 remove1/2/3; same valid-domain/error rules on empty/both controls.
Review explicit separation of native row/cell/identity/data admission from
unadmitted7163 shared state/return and7164 types; no whole-call atomicity.
Require completed bounded delegated-development self-review plus automated
walkthrough/contracts before local completion. Documentation review evidence
is complete below; the subsequent full Clang70 result is recorded separately
under local runtime completion, not inferred from this documentation signoff.
Rollback: revert only checked argument integration and retain independent
raw/frozen/failure evidence; notify downstream numeric-policy consumers via
changelog/coverage/handoff and linked review. No independent human signoff,
DO-178C certification, system/tool/release or hazard closure claim.

## Completed bounded development documentation review

DQ/DV-5611-REMOVEITEM-ROW-NUMERIC-001: Codex delegated development self-review
and automated walkthrough completed2026-10-12T20:48:05Z, under the owner's
medium-severity development allowance. This is NOT an independent human
review or high-hazard/system qualification. Active hazard rows unchanged.

The procedural delta was checked against the independently recovered help/
three full paired native sources/raws/statuses and named parent derivations,
then against actual full GNU70/70 PASS521.24s and both-mode/session/class/
dense-sparse/empty-multi198direct/four556public counters. The walkthrough
confirms sparse2.9 selects displayrow2/ID22 and preserves surviving rows'
two columns/payload/identity;22.9 yields1234 rather than addressing ID22;
zero/nonfinite/NaN invalid11,32768.9 admitted-but-absent1234 and32769 invalid11.
Explicit VFP9 both-sign aliases address1/2/3; default rejects out-of-domain
aliases without removal. All four exact catalog message sets, catch/continue,
guard/cursor/mode/session/reset and rejected before-after state checks pass.
Exact64 and NaN boundaries are explicitly derived, not native syntax claims.

The boundary review confirms successful shared focus/text/top/return remain
unchanged implementation and explicitly unqualified native gaps7163; type
coercions remain separate7164. Both144-call excluded-scope full-rawcmp0 and
both six unchanged list-control preservation groups compile/run0/failures0
provide labeled NON-NATIVE preservation, not code-originated requirements.
Omitted/extras/RowSource/binding/RemoveListItem/AddItem/parser/events/lifecycle
are not admitted. Shared removal and other source consumers are unchanged;
only one guarded numeric admission arm delegates the checked row slot.
The no-argument-sized-allocation/iteration analysis does not claim global
callback atomicity, persistent storage, GUI/scrolling or release qualification.

Five documentation/signoff/channel/safety workflow contracts passed5/5/
339.63s/status0 at2026-10-12T20:22:34Z,213 valid changelog fragments,
full86-line rawSHAb0679e0c27ab3ec51c891f28fa76653dd782dc2774592a51e8dcba13d4797c77.
All33 fixed scoped inputs checked before-after these tests and at walkthrough.
Initial five frozen and all20 neighbor assertions remain unchanged; failed
selection/standalone compile chronology preserved. No weakening/refreeze.
Rollback/field notification remain as specified above: revert only admission,
retain raw/frozen/failure evidence, notify numeric-policy consumers through
changelog/coverage/handoff and exact-head review. Signed post-push/CI and
actual exact-head Claude or authorized Codex review remain separate
integration gates. No parent/KBX/hazard/release closure.

## Local runtime completion

Fixed driver16589 completed0 after BOTH identical full70 suites passed:
GNU70/70 PASS521.24s/status0 at2026-10-12T19:55:43Z and Clang
ASan/UBSan/float-cast-overflow70/70 PASS3230.17s/status0 at
2026-10-12T20:50:11Z. Each full798-line raw includes all198 direct/four556
public counters and all65 prior subjects. Clang full raw SHA256
f87c8e5ab7e602181c4decefec8ee00fd08285b78e6ad195a71694fef347a559;
GNU92fbc086982ebae59417028497b38a4f625ce3412c35ff208a4125ab78211976.
Configure/compile0 and all33 fixed input hashes verified before-after both
runs and again at this finalization. Initial five frozen unchanged, no
refreeze or old assertion correction. No sanitizer diagnostic; leak detection
disabled, not warning-free, leak, performance or whole-call atomicity proof.
Both excluded-scope144-call fullrawcmp0 and both six unchanged list-control
groups passed; five contracts5/5 and bounded delegated DQ/DV were completed
as recorded above. Exact signed-head post-push70, hosted required checks,
actual clean external exact-head review and resolved conversations still
gate integration. Separate7163/7164 remain unadmitted gaps.
Final repeated five contracts completed2026-10-12T21:24:59Z/status0/5of5
PASS340.08s/213valid fragments; full86-line rawSHA
0395f13f9d620d178c97c23faa571b886825ce3e0f869c5bebc82db7e78a0439.
All33 fixed scoped input hashes unchanged before-after. No frozen weakening.

## Review correction: effectively empty ControlSource

Codex exact-head4c83738685 review discussion4252485330 identified that a
space-only ControlSource is unbound but the numeric guard tested raw emptiness.
The recovery boundary is now clarified as trimmed-empty ControlSource, matching
binding semantics without changing bound routes. Existing code is supporting
verification evidence only: one NEW matching pair of independent installed
VFP9 7423 runs of36 calls measured empty, space and tab-plus-spaces bindings, both
native classes,2.9/0/4/32769/both-sign aliases. Runner0/cmp0/DONE36;
full38-line rawSHA73a15ab81c208e8dded39b83840d04e00e7c392e294a2ddb57e944f452fa4d6a.
All three binding shapes have identical native row/cell/identity/payload and
11/1234 outcomes. Default alias rejection remains named parent policy, not
the native syntax. Full source/raw/status retained; no VM/setup changes.

A SEPARATE supplemental72-case literal public regression was frozen before
the review production correction; initial five frozen files remain unchanged.
Supplemental GNU and Clang sanitizer pre-fix runs completed/run1 with identical
120 assertions/full122-line rawd99315f96a537414f2e254c9b12577751fb99ec851df72732e9323d94efb987b;
all72 cases completed, compile0/all prior33 input hashes before-after.
Only afterwards raw-empty guard changed to trim_copy(...).empty(). The new
regression is included/called from existing list_controls_07; its old assertions
are unchanged. CMake/isolation/helper/initial numeric test unchanged/no refreeze.
Review-fixed34-input manifest tracks the changed guard, additive07 hook and
new supplemental module separately; original33/five hashes stay immutable.

Full identical70 GNU then Clang sanitizer verification is now running against
the revised guard, with supplemental72 executed inside runtime_surface_functions.
Both excluded-scope comparisons/sixgroups/finalcontracts and renewed bounded
DQ-DV walkthrough remain review-fix gates, followed by signed fix push/exact
remote-change verification/conversation resolution and fresh actual clean
exact-head external review/postpush70/all11 checks. Old-head postpush70/70
PASS522.12s at21:34:47Z/raw1abe8177 is not new-head acceptance. No scope
expansion into7163/7164, no initial frozen weakening or parent/hazard closure.

Revised GNU full70/70 PASS516.71s/status0 at2026-10-12T21:57:34Z,
all198/four556/all65prior/supplemental72/all34before-afterhashes;
full799-line rawae525dd60bca810a20db56c54fac792998bb33c2e85b64a5b5eca6cd06ea0eac.
Revised Clang sanitizer full70 still running95432; both-scope/supplemental/
sixgroups/contracts queued98493, no BOTH revised completion or fix-push yet.
Old-head macOS full579 failed ONLY unchanged polyglot_runtime_host identity/
counts assertion; allfiveRemoveItem PASS. Focused7169 filed exactmain6fe579,
unadmitted/no production expansion/rootcause or flake claim. Full3702-line
raw d6065519c7ed4a8a55b863fe33be01a23bc8c12caadbc4570823c57aac718425
retained in build/removeitem-native/oldhead-platforms; sameheadLinux579/579
PASS986.12s/localGNU10/10PASS8.00s do not clear nativeMac failure. Follow
revised-head platform evidence without rewriting this historical failure.

## Review-fixed local completion

Revised GNU70/70 PASS516.71s/status0 at21:57:34Z and Clang ASan/UBSan/
float-cast-overflow70/70 PASS3213.58s/status0 completed22:51:47Z;
full799-line Clang rawSHA738ff2cdd7879f8a7bae8a4ba4c3f363902451f9fef83f474297ac427ea2cc5b,
GNUae525dd60bca810a20db56c54fac792998bb33c2e85b64a5b5eca6cd06ea0eac.
All198direct/four556public/all65prior plus supplemental72 pass; all34 input
hashes before-after. Initial five frozen/supplemental freeze unchanged.
Both144 excluded-scope compile/run/cmp0/DONE144/scaffold0/raw513560f4,
both supplemental72 compile/run0/failures0/rawbeb34879 and both six unchanged
list-control groups compile/run0/failures0/rawbefec198. Verification driver
attempts98493/80383 ran144 with24 RowSourceType2 setup failures from RELATIVE
scratch-root arguments; no source or assertion failure repair. Initial missing-
argument diagnosis was incorrect and is superseded in retained attempts1/2.
Only NON-FROZEN driver argument normalized to ABSOLUTE owned root; SAME binary/
source/literals fullrawcmp0. Corrected attempt3/89836 completed0.
Five contracts5/5 PASS345.35s/status0 completed2026-10-12T23:24:49Z/213fragments;
full86-line rawdfc7f205c0abf6a2702f959f4642cb83d70d124abe16988b75d3b4a42d082392,
all34 before-after hashes. No warning-free/leak/performance/global-atomicity claim;
no sanitizer diagnostic, leak detection disabled.

DQ/DV-5611-REMOVEITEM-ROW-NUMERIC-001 renewed by Codex delegated development
self-review/automated walkthrough completed2026-10-12T23:26:21Z. Bounded medium
procedure/misuse/rollback/field-notification review checks the newly confirmed
empty/space/tab-spaces binding equivalence against the independent paired36
source/raw/status and named default parent policy; supplemental72 and BOTH
full70/preservation/contracts verify 2.9 row2,0 invalid11,4 absent1234,32769
invalid11 and both-sign aliases/default rejection. Surviving IDs/cells/payload
and rejected before-after state remain distinct from7163 shared state/return
and7164 types. The guard change is one line, old07 assertions unchanged with
only an additive hook; helper/CMake/isolation/original numeric test unchanged.
This is NOT independenthuman/high-hazard/system/release qualification. Active
hazards/parents/7163/7164/7169 remain open/unadmitted as applicable. Existing
rollback/notification plans remain: revert only admission and preserve evidence,
notify numeric-policy consumers through changelog/coverage/handoff.
Signed fix push/exact remote verification/4252485330 resolution and fresh
actual clean exact-head review/postpush70/all11 required checks remain gates.

## Second review: exact-zero RowSourceType recovery BEFORE code

Fresh actualCodex f16e3a338f review4253248168/threadPRRT_kwDOR7wYLc6rp5Xq
flags exact-zero INT64/UINT64 RowSourceType bypassing Number-only guard.
Newthread unresolved/notclean; whitespace4252485330 staysresolved. Independent
review2 recovery BEFORE any new assertion/code: installedVFP9 explicitRST0
paired36/empty-space-tabspaces/bothclasses/6rowcases runner0/cmp0/DONE36,
full38-line raw34dec732e2d26ef0fe8382be97c07c41afa11ebf1165318e3510dbe838b395bc.
Source/raw/status/maptimestamp retained fixture+build. CAST configzero=0 is
named-owner #6776/RQ-CF-PRG-NUMERIC-BEHAVIOR-001 extension derivation, not
nativeCAST syntax/codeoracle. Configurationzero is unbound; rowargumentzero
STILLinvalid11. Next: separate NEW72 literals (bothclasses/modes, Numberzero/
INT64zero/UINT64zero,6rowcases), freeze then BOTH complete pre-fix GNU/Clang
runs using verified f16 libraries; initialfive/firstsupp72/9cf1ad04 unchanged.
Only AFTER bothfailures typed Numeric/int64/uint64 zero guard/additive hook,
not Logical/String/Currency/nonzeroRowSource expansion. Then full70/bothscope/
supplementals/sixgroups/contracts/renewedDQ-DV/signedpush/remoteverify/reply-
resolve4253248168/freshactualcleanexactheadreview/newpostpush/all11gates.
No review2 helper/test/config/production edit yet. f16postpush79200 and
requiredwatch30339 ongoing/no reported failure sofar, not futureheadacceptance.
No ownerinput/VM/reset/7163-7164-7169 admission or parent-hazard closure.

## Review2 audited refreeze and bounded migration

Review2 NEW72 module initially froze23:47:14Z/c0ec3776; BOTH f16 pre-fix
compile0/run1/72complete/136failures/raw7492a980. Audit found120 selected
failures+16 incorrect TOTAL-ole_objects==1 scaffold assertions on Number0
caught-error cases (CATCH adds an error object). Typed-zero guard/hook were
prematurely wired before this audit; own local fixed53679/queued83323 verified
PGIDs1713542/1713677 stoppedTERM143, incomplete raw/source retained first-freeze.
Only task-owned guard/hook reverted apply_patch to EXACT f16 source/gitdiff0,
GNU cf_xbase_runtime restored/rebuilt0, no acceptance of incomplete run.
Corrected ONLY review2 scaffold to count matching native-control class;
all row/error/kind/value/ID/cell/payload literals unchanged. Explicit refreeze
23:51:21Z/hash169cc097108a5aa3e507f10bab098eb0160d125a95d7359b3a41e1fda7ab3c8c;
initialfive/firstsupp9cf1ad04 unchanged. BOTH repeated pre-fix compile0/run1/
72complete/ONLY120 exact-zero failures/no scaffold/type/value/setup failure,
full122rawc996b6c39a600a7c430ef9e2b54c7525defe81b08e45113415d297d36414a785,
GNU23:51:24Z/Clang23:51:32Z/driver65506harvested0/all35before-afterhashes.
ONLYafter auditedrepeat BOTH typed Number/int64/uint64 zero equality guard
and additive07 hook wired23:52:55Z; no otherconfigurationtype/nonzeroRS
policy/source/parser/sharedstate expansion. Revised full70 serialGNU-Clang
69073 RUNNING/GNUconfigure-compile0; queued62996 waitsBOTHsuccess then both144
scope/second72/first72/sixgroups/fivecontracts/all35hashes. RenewDQ-DV only
with completed revised evidence, then signedpush/exactremoteverify/replyresolve
4253248168/freshactualcleanexactheadreview/newpostpush/all11gates. No completion
or conversation-resolution claim yet. Fullfirst-freeze/stop/restore/refreeze/
repeat raw/status/source/libraryhashes retained fixture+build. f16postpush79200
harvested0:70/70 PASS561.57s at23:38:44Z/full799raw9d0e6c58/all34hashes;
f16requiredwatch30339 harvested0/all11green NOT future review2-head acceptance.

## Second correction: completed bounded verification and renewed walkthrough

Both identical full70 suites completed/status0: GNU70/70 PASS514.10s at
2026-10-13T00:01:48Z (rawSHA
cf59e86296f998fbc3cc806efc5ee7769a766b05a142542334e4de1071069a4e) and Clang
ASan/UBSan/float-cast-overflow70/70 PASS3202.31s at00:55:53Z (rawSHA
7b6caff42736dd886cc5d557608302d52b464b8b0375f82d3ae1faa18b7bc4bc).
All198 direct/four556 public and BOTH72 supplemental counts pass; all35 input
hashes checked before-after both suites, preservation and contracts. Both144
NON-NATIVE excluded-scope fullrawcmp0/raw513560f4; both second72 compile/run0/
failures0/raw6963988e, both first72/rawbeb34879 and both six unchanged groups/
rawbefec198 pass. Drivers69073/62996 harvested0. Five contracts5/5 PASS339.04s/
status0 at2026-10-13T01:02:26Z/213fragments, fullrawSHA
d2430774e8b34b5cbe6495c2e2b0037c2d6804cf697ada66d1eb86b3e4dacfe8.
Full source/raw/status/timestamp/hash/driver evidence retained; ELF executables
are retained only in local build/fixture-executables, not product fixtures.

DQ/DV-5611-REMOVEITEM-ROW-NUMERIC-001 renewed2026-10-13T01:17:02Z by Codex
delegated development self-review and automated walkthrough, bounded medium,
NOT independent human/high-hazard/system/release qualification. Fresh paired36
native explicitRST0 evidence and named #6776/RQ-CF-PRG-NUMERIC-BEHAVIOR-001
extension derivation govern Number/int64/uint64 configuration zero equivalence.
The72-case walkthrough checks all three preserved configuration kinds in both
classes/modes:2.9 removes row2; argument0 invalid11;4 absent1234;32769 invalid11;
both-sign aliases reject in default/select row1/2 in VFP9. Configuration zero
does NOT make row argument zero valid. IDs/two-column cells/ItemData are checked
independently, without successful focus/text/top/return parity claims7163 or
unsupported type expansion7164. No Logical/String/Currency/nonzeroRS expansion,
shared remover/parser/callback change, input-sized allocation or global-call
atomicity claim. Initialfive/firstsupp freeze and old assertions unchanged;
explicit second-supp scaffold refreeze and BOTH repeated originals retained.
Rollback remains reverting only bounded admission, keeping failed/native/frozen
evidence, and notifying numeric-policy consumers through changelog/coverage/
handoff/linked review. Active hazards and parent/gap issues are not closed.
Signed push/exact remote-change verification must precede4253248168 resolution;
fresh actual clean exact-head Claude or authorized Codex review, postpush70 and
all11 uniquely SUCCESS required checks still gate integration. Publishedf16
all36 CI checks SUCCESS is historical evidence, not acceptance of this fix.

## Audit correction: initial explicitRST0 probe ran on emptied controls

At2026-10-13T01:20Z the final raw audit found that the original review2
paired36 raw34dec732 contained count0 in EVERY case: RowSourceType=0 was
assigned AFTER population and installed VFP9 clears those rows. That pair
supports EMPTY-control admission11/absent1234 only, NOT populated removal/
identity/cell/payload equivalence. Earlier statements that this pair verified
successful populated removal, including the01:17:02Z walkthrough attribution,
are explicitly superseded. The original raw/source/status/hash are retained
unchanged; no native evidence is rewritten or backdated.

A separate fresh corroborating pair configures RowSourceType0 and blank/
space/tab-spaces binding BEFORE population, then records before3 in all36
calls. Both runs0/fullrawcmp0/DONE36, full38-line rawSHA
c92fe8e1bba0158bcd68a19f61e1afb0078e4b4c3268fb4aee30b0fba0711699,
source review2-populated-rst0.prg. It observes2.9 remove row2,0 invalid11,
4 absent1234,32769 invalid11 and BOTH-sign aliases remove row1/2; surviving
IDs/two columns/ItemData agree with the existing literal requirements.
This is LATE corroboration after implementation, not pre-code recovery.
The governing populated-row literals were already independently observed by
the ORIGINAL three fresh matching native pairs before initial code/freeze;
the corrected setup does not alter them or either supplemental frozen module.
Exact configuration-kind equivalence remains a named parent-derived extension,
not native CAST syntax or property-assignment lifecycle parity.

Separate focused [#7171](https://github.com/rhamenator/Project-Copperfin/issues/7171)
records ordinary RowSourceType=0 reassignment clearing native rows versus main
retaining them, against main6fe579e32. Fresh native paired2 run0/cmp0/DONE2
rawe53f70f9 before3/after0; current-main supporting GNU diagnostic compile/run0/
DONE2/raw1c35d709 before3/after3, retained4355b990 libraries verified/relevant
property sources identical main. It is NOT admitted/fixed/closed in this slice.
The supplemental Numeric/int64/uint64 configuration writes retain existing
Copperfin lifecycle as NON-NATIVE setup preservation; only subsequent checked
RemoveItem argument admission has the recovered numeric contract.

DQ/DV renewed2026-10-13T01:24:24Z: Codex delegated bounded-medium development
self-review/automated walkthrough audits ALL corrected36 rows against the
original pre-code requirements and BOTH unchanged compiled70/supplemental72/
preservation results. Procedure explicitly configures a native control BEFORE
populating it when obtaining populated-removal evidence. Do not interpret
postpopulation property writes as native-preserving; do not confuse config0
with invalid row0 or claim7171 lifecycle parity. Initialfive/firstsupp/refrozen
second72 hashes unchanged; no numeric requirement/assertion or production
change is made from this correction. Retain misleading-probe chronology,
failed scaffold/refreeze, full correct/incorrect raws and focused issue.
Rollback/field notification remain the admission-only plan above, with this
correction carried into coverage/matrix/handoff/changelog/linked review.
No independenthuman/high-hazard/system/release/parent/hazard closure.
Postdoc fivecontracts5/5 PASS336.91s at01:23:54Z/213fragments/raw749fce90
precede this audit addendum; final current-document contracts repeat is required
before integration, alongside new exacthead review/postpush70/requiredCI.
