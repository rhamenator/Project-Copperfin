# IndexToItemID Numeric row-index read requirements recovery

Pre-code mapping recorded 2026-10-11T08:23Z BEFORE helper, assertions,
registration, isolation or production edits. Admitted OPEN owner-authored
agent-approved #5611/#6776, base origin/main
a803eb0243121858db987d2021702c4a25135eb3. Only native ComboBox/ListBox
IndexToItemID first-argument Numeric/exact READ conversion is selected.
Item-ID addressing, writes/ID creation/mutation, RowSource/lifecycle, other
methods, non-Numeric/NULL/Currency/arity and general grammar are excluded.

## Independent evidence and requirement

Installed Visual FoxPro 09.00.0000.7423, existing private Wine prefix
/home/rich/copperfin-wine-vfp9. Unchanged /home/rich/bin/vfp9-probe SHA256
1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06.
Runner snapshot retained; temporary harness/config auto-delete, not invented
persistent files. No VM start/recreation, credentials or decompilation.
Independent matching pair native.out/native-repeat.out:120 lines/118 calls,
SHA256 aada55aef903ed9903113ea60932f4728cb2a820e5612fade83ebd086c7abed5.
Fresh objects, both controls, sparse IDs11/22/33, count3/selection row2,
data101/202/303 and selection bits unchanged for every call.
Native1.9 returns Numeric11,2.9 returns22,3.9 returns33,22 returns0:
ROW position, not ID. Numeric0/negative/missing rows return Numeric0.
Both positive4294967297/.9 and negative-4294967295/.9 alias row1=>11;
-4294967294.5=>22, -4294967293=>33. No positive raw INT32_MAX ceiling:
INT32_MAX+.1/.5/.9 and2147483648 return0. Huge/plus-minus infinity return0.
Unsupported Logical/NULL/Character/Currency controls error27 natively; those
remain preserved Copperfin policies, not native admission claims.
Same-input Logical/NULL discrepancy is separately recorded in
[#7147](https://github.com/rhamenator/Project-Copperfin/issues/7147) against
base main a803eb0243121858db987d2021702c4a25135eb3; no admission label or
type-policy implementation change is requested. Character/Currency native
controls use2.9 while Copperfin preservation controls use1.9; no same-input
comparison is asserted for those two controls.
Shipped Microsoft IndexToItemID Method help completely read:
 /home/rich/temp/vfp9-help-len/html/6fa989be-43f7-48d8-a135-a6c24443668e.htm
SHA25615f751bad2e4bbef9d120bff41a7c2e218ea12432f2115f43c247115cadb1f7c.
It specifies row-position-to-unique-ID mapping for both controls, first row1.

RQ-CF-PRG-INDEXTOITEMID-NUMERIC-001 derives from admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner mode policy and
HZ-runtime-crash-01/HZ-data-corruption-01. Default COPPERFIN finite Numeric
truncation toward zero then checked signed32 domain INT32_MIN..INT32_MAX;
invalid/nonfinite/outside-domain raises localized catchable31. Exact signed/
unsigned64 compare without Double. Preserve negative/zero/missing-row Numeric0;
existing positive row returns its stable ID, not its payload or selection bit.
The signed domain preserves ordinary negative-row missing semantics, unlike
the nonnegative Selected/SelectedID contracts. Explicit VFP9 uses independently
observed signed-low32 conversion without a positive ceiling or nonnegative
admission gate; signed negative/zero still missing0. Plus-minus infinity/huge
indefinite0 observed; NaN rejection is derived containment policy, not a native
NaN observation. Exact64 low32 projection is derived from the parent defined
primitive, not native extended syntax: INT64_MIN+2 => row2/ID22 in VFP9,
default31; UINT64_MAX=>signed-1=>missing0. Nextafter/subnormal expectations
are independent mathematical boundaries, not native decimal-parser equivalence.

## Architecture and verification before code

All four ordinary/postfix parentheses/brackets preserve raw PrgValue already,
but IndexToItemID has a property-first textual reflection route unlike
SelectedID. Plan distinct checked_indextoitemid_numeric_argument plus narrow
Numeric/exact read_native_list_control_item_id_numeric adapter, shared by the
method and raw-selector native reflection route. Preserve property-first
_Access/visibility/delegate ordering, textual GETPEM, unsupported types,
current-session mode, return/key parsing, and existing materialize order where
applicable; no parser or generic reflection conversion changes.
Existing upstream materialization/callback behavior is NOT whole-call atomicity.
No argument-sized allocation/iteration introduced.
VR-5611-INDEXTOITEMID-NUMERIC-001: both-mode direct Numeric/exact64,
NaN/infinity/adjacent/subnormal/adapter cases with independent literal results.
VR-5611-INDEXTOITEMID-NUMERIC-002: both controls/modes/two sessions/four raw
routes, sparse11/22/33 identity, Numeric return type/row mapping, error31
locales/sentinel/continuation, context/cursor/reset, stable count/data/selection/
item keys/ListIndex/ListItemID, no external invoke success event.
Freeze helper/test/registration/isolation BEFORE configuring. Helper remains
UNUSED through BOTH complete original GNU and Clang sanitizer runs; retain
full raw failures/status/hash/counters BEFORE migration. Any frozen repair
requires explicit refreeze and BOTH originals repeated. Fixed neighbors and
documentation contracts then exact-head hosted checks/clean Claude-FIRST or
authorized Codex review/resolved conversations remain gates. No closure yet.

## Documentation assurance before code

DQ/DV-5611-INDEXTOITEMID-NUMERIC-001 link active HZ-runtime-crash-01,
HZ-data-corruption-01 and HZ-doc-command-01 without changing severity/status.
Bounded documentation misuse medium: wrong ID can misdirect downstream app
actions, but this read slice performs no persistence or system-safety test.
Walkthrough:2.9=>22 not old rounded33;22=>0 rather than ID addressing,
signed-1=>0; positive4294967297 and INT64_MIN+2 modern31/VFP9 row1/row2;
NaN both31, huge/Infinity modern31/VFP9 0. Check all routes, locales and stable
state. Delegated development maintainer self-review and applicable automation,
not independent-human or release qualification. Rollback only this bounded
slice, rerun frozen neighbors, notify integrators through changelog before
re-release. No parent/family/KBX/hazard closure inferred.
Controlled serial native reproduction: inspect probe.prg's owned output path,
copy it to build/indextoitemid-native, run retained vfp9-probe with --result
that exact native.out path; retain each raw before overwrite. FXP remains
uninterpreted scratch. VM cleanup/backup reset completion is unconfirmed;
never start/recreate VMs. GNU/Clang private TMP/nice10/ionice2:7/-j1;
600s disclosed LOCAL diagnostic bound, not CI/performance policy.
ASan/UBSan/float-cast-overflow/halt1/leaksdisabled; no leak qualification.
Complete configure/compile warnings and failed originals must be retained.

## Retained implementation checkpoint

Both complete original29-subject runs retained before migration: GNU25/29/
231.64s, Clang25/29/378.38s; onlyfournewpublicfailed, all counters and neighbors
complete. Each raw3301lines/2944 ownFAIL diagnostic lines (repetitions included),
zero otherFAIL; no baseline sanitizer report. Five frozen and two base-production
hashes rechecked unchanged; no assertion repair/refreeze. See baseline-audit.md
and full original-gcc.log/original-clang.log for investigation-ready evidence.
Only the Numeric/exact method and raw property-first adapter are now wired;
fixed GNU29/29 PASS234.05s and six unchanged GNU preservation groups PASS.
Frozen inputs/all counters unchanged, zero fixed FAIL; full raws/hash in audit.
Fixed Clang29/29 PASS378.05s and its six unchanged preservation groups PASS;
full raws/status/counters/hashes retained in baseline-audit.md. Bounded medium
DQ/DV development self-review/procedural-delta walkthrough is complete there;
five documentation contracts PASS335.83s, full86-line raw/hash retained.
Result-wording follows that checkpoint; exact-head integration remains pending.
Raw nonpositive Numeric selectors now return through the typed property read
instead of method fallback. No arbitrary RowSource callback/materialization
count qualification or whole-call atomicity claim; bounded stable-state checks
and unchanged existing control groups are the completed bounded verification.
