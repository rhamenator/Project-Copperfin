# ListIndex Numeric assignment requirements recovery

Pre-code map recorded2026-10-11T19:28Z BEFORE helper/assertions/configuration
or production migration. Admitted OPEN owner-authored agent-approved #5611/#6776;
base origin/main252f70becd49a8cfece235f4c0ff8da868046827. Bounded native
ComboBox/ListBox ListIndex Numeric/exact input admission on existing direct and
SETPEM writes. Stable materialized RowSourceType0/unbound Character Value,
single selection and existing ListBox MultiSelect state are the native controls.
Other types/NULL/Currency, indexed cell writes, row-ID lifecycle, callbacks,
ControlSource persistence, generic parser/reflection expansion are excluded.

## Independent allowed evidence

Fresh installed Visual FoxPro09.00.0000.7423 in the existing private Wine prefix
/home/rich/copperfin-wine-vfp9, not a VM. Unchanged /home/rich/bin/vfp9-probe
SHA1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06.
Three complete paired probes, each byte-identical first/repeat, retain full
source, raw output and runner logs; no credentials or decompilation.
The main pair has302lines/300calls, SHA
13841a3a43d69bdf7727966cfb0cfcc50dc03adc7149bc703c783aa6cf983409.
Edge pair272lines/270calls, SHA
399eed0adb7bce698b70cce6c187a6e30e6e7f98b1fcb71c95f7f6796a4a848e.
Boundary pair146lines/144calls, SHA
ca39f97f027ece649082c91a1125df5afb85a94dd800eb686fbc263fbd45994c.
Whitespace-only probe-source formatting preceded the final main/edge pairs, before
any freeze or original compiler measurement; native output is unchanged.

Shipped Microsoft VFP9SP2 ListIndex Property help completely read435lines:
/home/rich/temp/vfp9-help-len/html/231921f0-a9cd-4902-9750-031edc4e2ef3.htm,
SHA32eb250947d7f892de0380d14406d17b0b6d85b11cc8b8782e56f39b81d091e8.
It defines runtime read/write selected display index,0 for no selection,
1..ListCount for selected rows, and MultiSelect's focused selection border.
The observed ComboBox ListCount+1 sentinel below is native evidence beyond
the documented ordinary range, not a sibling implementation requirement.

Fresh objects retain three rows first/second/third, payload101/202/303,
sparse IDs11/22/33 or dense1/2/3, initially focusedrow2; edge probes add empty
objects and ListBox MultiSelect with initial bits true/true/false.
Both controls truncate ordinary fractions:1.9=>1/first,2.9=>2/second,
3.9=>3/third. Ordinary negative and fractions around zero clear focus0.
ListBox out-of-range positive input4/22.9 clears focus0; ComboBox records4
(ListCount+1), ItemID0, empty Character Value/DisplayValue, no selection bits.
An empty ComboBox likewise records1 for positive inputs; empty ListBox stays0.
Rows/cells/IDs/count/ItemData do not change. Single-selection bits follow the
selected row or clear; MultiSelect retains existing bits and adds an accepted
focused row, even when focus becomes0. Value/DisplayValue follow the focused
row, not the other retained bits.

Native positive raw values through4294967295 are admitted;4294967295.1/.5/.9,
4294967296, positive huge and +Infinity error46 and preserve all initial state.
Negative low32 aliases -4294967295.9/-4294967294.5/-4294967293 focus1/2/3;
-8589934590 focuses2. Negative huge/-Infinity clear0. Positive2147483648/.9
and negative-2147483648/-.9/-6442450944 produce ComboBox4 (empty1), but
2147483649/.9 and -6442450943 produce0; ListBox clears0 for all these.
This distinct signed-boundary behavior was independently measured, not
inferred from sibling read helpers. Logical/NULL/Character/Currency error1732
and preserve initial state natively; those admission differences are excluded
from the migration, not default or compatibility expectations for old paths.

## Pre-code requirement and architecture

RQ-CF-PRG-LISTINDEX-NUMERIC-001 derives from #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and active HZ-runtime-crash-01/
HZ-data-corruption-01. COPPERFIN finite truncation then checked signed32
admission, rejecting nonfinite/out-of-domain inputs with localized catchable31
before modeled selection mutation. Negative admitted integers clear0; positive
in-range rows select that display row; absent positive rows clear ListBox0 or
use ComboBox's bounded ListCount+1 sentinel. Default INT32_MIN clears0, not
the native signed-boundary alias. Exact signed/unsigned64 decisions are
lossless, never a Double roundtrip; NaN rejects both by parent containment.
These default/exact/NaN boundaries are explicit derived extension policy,
not claims that native VFP implements extended syntax.

Explicit VFP9 rejects positive RAW aboveUINT32_MAX with localized46, then uses
a defined signed-low32 projection (negative huge/-Infinity=>0). Projection is
an arithmetic model consistent with measured aliases, not an observed native
intermediate. ListBox keeps projected1..ListCount else0. ComboBox keeps positive
projected1..ListCount, caps larger positive values at ListCount+1, treats
projected INT32_MIN as that same sentinel, else clears0. That special boundary
is directly evidenced; it must use defined arithmetic, never signed subtraction
overflow. Native64 exact projection is parent-derived; e.g. INT64_MIN+2 projects
2 and selects second in either identity shape, default rejects31.

Distinct checked_listindex_numeric_argument must remain UNUSED by production
through BOTH complete originals. It handles conversion/admission only; a
bounded ListIndex write adapter applies the class-specific final selection,
current-session policy and existing state synchronizers. Intercept only native
ListIndex Numeric/int64/uint64 writes in write_native_property_if_present BEFORE
generic property mutation. Preserve visibility/_Assign/delegate/ControlSource
ordering; unsupported types retain the old path. Clear unbound Character Value
when the accepted assignment has no row, consistent with native observations;
do not alter generic selection synchronization for other consumers. Validate
RowSourceType0 controls, not arbitrary callback/materialization atomicity.
No input-sized allocation or iteration; clamp only against existing row count.
Keep existing direct and SETPEM routes; SETPEM is a Copperfin access route,
not a claim that VFP has that extension syntax.

VR-5611-LISTINDEX-NUMERIC-001: independently literal direct both-mode
Numeric/exact/NaN/Infinity/nextafter/subnormal projection and admission cases.
VR-5611-LISTINDEX-NUMERIC-002: both classes/modes/two sessions/direct+SETPEM,
sparse+dense+empty, single/multi selection, Numeric ListIndex kind/finalrow,
Value/DisplayValue kinds/text, ItemID/bits/rows/cells/payload/count/context/
sentinel/locales/continuation/reset; rejected assignment preserves initial state.
Preserve all44 completed numeric subjects plus six unchanged list-control groups.
Freeze helper/test/registration/isolation bytes BEFORE configure; retain BOTH
full GNU and Clang ASan/UBSan/float-cast-overflow originals before migration.
Any frozen repair requires explicit refreeze and BOTH full originals repeated.
Fixed identical suites, counters, preservation, contracts and signed exact-head
CI/review are later gates; none is claimed complete at this map.

Initial freeze2026-10-11T19:53:48Z precedes syntax/configuration/originals;
274 direct cases and1184 public cases in each of four mode/session subjects.
Full49-subject GNU and then Clang originals launched serially by
[retained driver](original-suites.sh.txt), private task TMPDIR/one build job.
The assignment adapter remains UNCHANGED; the new conversion helper is UNUSED
by production. Six production and eleven neighboring test hashes are retained
and checked before/after both originals. No result is acceptance while pending.

Five initial frozen SHA256 inputs (no refreeze or frozen repair):

- helpers.h:acbf03eeb3785b07f53c4f9d2cdd6d39b856f64fd451ca605bebd582309ff82e
- helpers.cpp:25f879b2f97c8526465a02253cee5b099c2fc27d06fadfca5becf11375ca078f
- test: b1c510f53dd980f6fd9698e1da2da7bc00149fe56bb8c8922e233ab385281f05
- registration:e8a17feb6eb0a2f48994319d5e12a641e745fe2b29de27c95d0d24cee8016f78
- isolation:af350e6e4b123afb6028ab208201df9c8a55400d41dc4d96c9aa9936003a5309

Partial original checkpoint2026-10-11T20:15:22Z: full GNU45/49/759.68s/status8,
only four new public subjects fail; all44 old subjects and newdirect pass.
Counters274/four1184 complete,16704 prefixed failed assertions
(COPPERFIN4466/4466,VFP93886/3886). No completion/context/reset/continuation/
count/external-call scaffolding failures; all five frozen/six production/eleven
neighbor hashes unchanged before/after. Full33985-line raw retained unchanged
in build/listindex-native/original-gcc.log, SHA256
e93c9fe38ae994072689963202654b7adadf84e2a74b97bdd999e05b5c3f0cea.
Clang original configure0/build pending in the same serial driver71532.
BOTH full originals are required before migration; this is not acceptance.

Clang full original completed20:52:10Z:45/49/1393.60s/status8, all49 subjects/
274/four1184 counters complete, same16704 prefixed assertions byte-identical to
GNU; onlyfournewpublicfail/all44oldsubjects/newdirect pass. Full33985line raw
SHA2562f59d50f32e5a0bbf1a96d719af90a727fec2de54396ed71c655ce3a562a49f7.
All five frozen/six production/eleven neighbor hashes checked before/after
both full originals and21:18Z before migration; driver71532 harvested0.
No sanitizer diagnostic, leaksdisabled. Narrow assignment adapter wired AFTER
both originals; tests/helper/registration/isolation unchanged. Defined extra
containment: reject31 before mutation if a ComboBox sentinel needs an existing
count aboveINT32_MAX, not a native or measured giant-allocation claim.
[Phase audit](baseline-audit.md) retains full chronology/manifests, failed
initial compile and equivalent local-guard repair. Identical fixed full49/
both six preservationgroups driver46697 running after the retained SETPEM-only
GNU failure and bounded route repair below; contracts/developmentDQ-DV/
signed exact-head integration/review still pending, not acceptance.

Fixed attempt2 GNU completed21:35:15Z:45/49/839.34s/status8, all49 subjects/
274/four1184 counters complete. All44oldsubjects/newdirect and all direct
public assignment cases pass;8556 prefixed assertions fail on SETPEM routes
only. The source-less reflection path bypassed the native write callback.
Full rawSHA68ae61865d82f91073d36126aceeaf243df2afacf12e51a94af9825386031f08,
compile0/status/completion/24-input manifest preserved as attempt2 before
repair. Clang and preservation stages did not run. Routing now calls the
existing callback only for SETPEM/native ListIndex/nonNULL Numeric-int64-uint64;
PUTPEM/other types/members/generic reflection remain unchanged. Same fixed
driver repeats both full suites and preservation groups; no frozen change,
refreeze, weakened assertion or acceptance claim.

22:15Z checkpoint: identical fixed GNU49/49 PASS760.09s/status0 completed
22:01:38Z, all274/four1184 counters complete; six unchanged preservationgroups
complete/failures0. Full raw SHA695133509725af4b7f18e44bffa6cb768bad653eb5f8209d2e6e48236dcdf97e;
before/after fixed24 hashes checkedOK. Clang compile0/full49 serial suite still
running in46697; no Clang completion or implementation acceptance yet.

## Documentation assurance before code

DQ/DV-5611-LISTINDEX-NUMERIC-001 links runtime/data/HZ-doc-command-01 without
altering registered hazard severity/status. Bounded documentation misuse medium:
incorrect focus can misdirect downstream actions, but this slice does not
qualify persistence/callback/high-hazard deployment behavior. Procedural delta:
fractions truncate instead of old rounding; distinguish ListBox absent0 from
ComboBox sentinel, focus from MultiSelect bits and display rows from sparse IDs;
explicit VFP9 low32/UINT32 raw ceiling/signed-boundary alias is not default.
Planned walkthrough:2.9=>second;22.9=>ListBox0/ComboBox4; default2147483648
reject31 versus VFP9 ComboBox4; positiveUINT32_MAX+.1 native46/stateunchanged;
negativealias default31/VFP9row2; defaultINT32_MIN0/VFP9ComboBox4; NaNboth31.
Development self-review/walkthrough completed2026-10-11T22:47Z using BOTH
identical fixed49 suites: GNU760.09s and Clang ASan/UBSan/float-cast-overflow
1409.70s, all274/four1184 counters complete and both six preservationgroups
PASS. Review compared the independent literal map/source/native raw with
the adapter and documented deltas:2.9=>second;22.9=>ListBox0/ComboBox4;
default2147483648=>31 versus VFP9 ComboBox4; UINT32_MAX+.1=>46 and initial state
unchanged; negativealias default31/VFP9row2; defaultINT32_MIN0/VFP9ComboBox4;
NaNboth31. Public state/locales/sentinel/context/continuation/reset assertions
validate expected operator outcomes. This is delegated maintainer development
self-review plus automated evidence, not independent-human signoff,
arbitrary callback/persistence/giant-allocation or release qualification.
Contracts and signed exact-head review/CI remain integration gates.
Rollback the bounded adapter/helper AND selected SETPEM routing and rerun all
unchanged neighbors/preservation groups; correct docs/changelog before re-release
if evidence contradicts the map. No parent/family/KBX/hazard/release closure.

## Completed identical fixed suites

Driver46697 harvestedexit0. GNU49/49 PASS760.09s completed22:01:38Z; Clang49/49
PASS1409.70s completed22:28:36Z. Full566-line raws:
[GNU](fixed-gcc.log) SHA695133509725af4b7f18e44bffa6cb768bad653eb5f8209d2e6e48236dcdf97e;
[Clang](fixed-clang.log) SHA9c452df172db4ffd1f0e62186d7cf65b3cb139fbd182f0c412030a8719ee2108.
Both six unchanged preservationgroups complete/failures0/status0, raw
SHAbefec198fb62d7e5fdae2e816aff3dc0ebde67970c187f3c7c013a1a377e3f19.
All49 subjects/274direct/four1184public counters complete; all44 old subjects
pass. All24 fixed inputs checked before/after both suites and22:46Z. No
sanitizer diagnostic, leaksdisabled; compiler warnings/full raw preserved,
not warningfree/leak/performance qualification. No frozen repair/refreeze.
## Documentation contracts complete

All five contracts PASS332.88s/status0 completed2026-10-11T22:52:47Z:
contributor signoff, changelog assembler/209 valid fragments, live channel and
full safety-traceability workflow. [Full contract raw](contracts.log) SHA256
3b00ffe38cf486ad08211d6e0477694426341abe9f5bc49a172a360fb0977f60.
Before/after fixed24 hashes checkedOK. Source/docs whitespace check excludes
ONLY raw fixture.log/.out; raw spacing/warnings are preserved, not normalized.
Signed exact-head postpush/hosted/review/resolved conversations remain merge
gates. [Evidence checksum manifest](evidence.sha256) names actual retained
artifact paths, including renamed failed attempts; their original emitted
log.sha256 records retain the original build pathname as historical data.
