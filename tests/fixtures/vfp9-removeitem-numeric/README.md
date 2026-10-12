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
