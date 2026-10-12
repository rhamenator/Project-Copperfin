# ListItemID Numeric assignment requirements recovery

Pre-code map recorded2026-10-12T01:51:13Z BEFORE helper/assertions/configuration
or production migration. Admitted OPEN owner-authored agent-approved5611/6776;
base origin/main d8005f9944cbc951846568be16f4d4b6dd0c6e1c. Only existing native
ComboBox/ListBox ListItemID Numeric/int64/uint64 direct and SETPEM assignments
in stable materialized RowSourceType0/unbound Character Value controls. Dense
IDs1/2/3, sparse11/22/33, empty controls, single selection and ListBox MultiSelect.
No production/helper/test assertion/registration/isolation edit preceded this map.

## Independent allowed sources

Installed Visual FoxPro09.00.0000.7423 in existing private Wine prefix
/home/rich/copperfin-wine-vfp9, NOT a VM. Unchanged /home/rich/bin/vfp9-probe
SHA2561e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06.
Only input enumeration/state-capture scaffolding reused; no Copperfin helper,
old code, sibling assertion or old native result is the ListItemID oracle.
Fresh main302-line/300-call matching pair SHA256
439b49ac16a30572cf27c2a357dde373d1bb0ccbce915af1ee5b0c3127727c0b;
edge272-line/270-call matching pair SHA256
9bd1bb099e8e5dd91b571d9c07b0ac9812954c511d802d66edc0a7878e707aae;
boundary146-line/144-call matching pair SHA256
3677c51b4e6f61d5fc56fe1ed001928aa7e7e10d4a285be3c5563a8e334e6165.
Full source/raw/runner logs and actual status are retained, paired again by
native-pairs.sh.txt. Probe-source boundary comment correction occurred before
helper/assertions/freeze; captured assignment always targeted ListItemID.

Shipped Microsoft VFP9SP2 ListItemID Property help completely read441lines:
/home/rich/temp/vfp9-help-len/html/2db61c48-6d96-42c7-9222-c5dc6939f3e4.htm,
SHA2561d7a48cdeb59d160e41b838b41bab574d1ff5d3528b8047d4db1287ef45abfbc.
It defines run-time read/write selected UNIQUE item ID, zero for no selection
(ComboBox zero also denotes text not in the list), identity distinct from
display-row order, stable IDs across RemoveListItem. Lifecycle is not migrated.

Fresh Numeric observations: both classes truncate ordinary fractions toward
zero. Dense2.9 selects row2/ID2; sparse2.9 clears focus0/ID0 and sparse22.9
selects row2/ID22. All absent IDs, ordinary negatives and zero clear focus0,
both classes and empty controls; there is NO ComboBox count+1 sentinel or
INT32_MIN assignment sentinel. Value/DisplayValue are the focused row's
Character cell or empty; rows/counts/cells/IDs/ItemData remain unchanged.
Single selection follows row or clears all bits. Critically, successful
ListItemID on ListBox MultiSelect clears ALL selection bits even when focusing
a valid ID; e.g. dense1.9/2.9/3.9 and sparse22.9 retain focused text but bits
false/false/false. It does NOT retain/add bits like ListIndex. Rejection46
preserves the initial focus2/text/IDs/payload and true/true/false MultiSelect bits.

Positive raw through4294967295 admitted;4294967295.1/.5/.9 and larger positive
values,1E300/+Infinity reject46 preserving full initial state. Negative low32
aliases -4294967295.9/-4294967294.5/-4294967293 select dense IDs1/2/3 but
are absent in sparse; -4294967285/-4294967274.9/-4294967263 select sparse11/22/33
but are absent in dense. Other signed-boundary aliases/negative huge/-Infinity
clear0. Logical/NULL/Character/Currency native1732 are retained as excluded
type-admission observations, not a claim that their old paths were migrated.

## Requirements, architecture and verification map before code

RQ-CF-PRG-LISTITEMID-NUMERIC-001 derives from admitted5611/6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, shipped ListItemID help and fresh native pairs.
Normal selection/identity/absent-ID/MultiSelect effects have plausible legacy
dependence and are the same in BOTH modes. Only overflow projection quirks are
explicit VFP9 mode. COPPERFIN default: finite Numeric truncation checked in
signed32, localized catchable31 on nonfinite/out-of-domain. Exact signed/unsigned
64-bit admission is lossless checked signed32, never round through double.
Admitted nonpositive/missing ID clears0; never allocate rows for caller input.
VFP9: positive RAW ceilingUINT32_MAX/error46, then defined signed-low32;
negative nonfinite/huge0 matches observations. Exact64 uses the same ceiling/
bit domain LOSSLESSLY as a named extension derivation, not native exact syntax.
NaN containment31 BOTH modes derives from parent/runtime hazard, not native
NaN syntax. Unsupported types/NULL/Currency remain outside selected writes.

Architecture: distinct checked_listitemid_numeric_argument projection helper
UNUSED through BOTH full original suites. After BOTH originals retained,
selected Numeric/exact arm of write_native_property_if_present/ListItemID uses
current-session NUMERICBEHAVIOR, checks before modeled selection mutation,
resolves only existing item identity and shared state coherently. SETPEM's
same bounded input route must use the existing native property-write callback.
Preserve visibility/_Assign/delegate/ControlSource and expression provenance
ordering; no arbitrary callbacks/ControlSource persistence/whole-call atomicity
or materialization-count claim. No generic synchronizer/read/type/lifecycle/
other-property migration. Shared IDs/payload remain bounded existing state.

VR-5611-LISTITEMID-NUMERIC-001: independent literal direct helper projections,
raw boundary nextafter, signed/unsigned64 extrema and unsupported adapter types.
VR-5611-LISTITEMID-NUMERIC-002: independently literal public final ROW outcomes
for both classes/modes/sessions/direct+SETPEM, dense/sparse/empty/MultiSelect,
fraction/boundary/nonfinite/exact64/NaN; all numeric errors31/46 in four locales,
continuation/context/reset, setter sentinel, Value/DisplayValue/ListIndex/
ListItemID/selection bits/count/cells/IDs/ItemData. No helper output is oracle.
Preserve ALL50 previous subjects including whole runtime_surface and six native
list-control preservation groups; add allfive new subjects, same full55 suite
GNU then Clang ASan/UBSan/float-cast-overflow before and after migration.
Freeze distinct helper/header/newtest/CMake/isolation BEFORE measurement.
Any frozen repair explicitly refreezes and repeats BOTH full originals.
No requirement/verification acceptance until full actual results retained.

## Documentation assurance

DQ-5611-LISTITEMID-NUMERIC-001 / DV-5611-LISTITEMID-NUMERIC-001 link active
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 (registered severity
and active status unchanged). Bounded procedure misuse medium: accidentally
reuse display-row/sentinel/MultiSelect oracle, execute extension-only CAST in
VFP or accept overflow/partial suites as success. Delta: distinct ID lookup,
no-row0 BOTH classes, clear ALL MultiSelect bits on accepted ListItemID, native
raw ceiling versus lossless derived default/exact64/NaN, explicit phase gates.
Development delegated maintainer self-review plus automated state walkthrough
required before completion; not independent human or high-hazard qualification.
Run five documentation contracts and exact-head postpush/hosted/review gates.
Initial five-input freeze recorded2026-10-12T01:57:29Z BEFORE syntax/configure/
measurement; [frozen.sha256](frozen.sha256) retains helper.h/helper.cpp/newtest/
CMake/isolation. Literal284direct/four1216public,86Numeric/28Exact/publicNaN/
29edge inputs/locales/context/state/sentinel; exact INT64_MIN+11/+22/+33
exercise sparse-ID low bits without double rounding. Allsix production and
fifteen old neighbor hashes checkOK; distinct helper remains UNUSED by production.
Original driver70061 started serial full55 GNU then Clang, syntax0/configureGNU0;
no original completion/production migration or implementation acceptance yet.
GNU subsequently completed51/55/966.60s at02:27:11Z: onlyfour new public fail,
all50 old/newdirect pass, all284/four1216 counters complete,15896 prefixed
failures retained. Full32436-line raw/hash/status/manifests and
[phase audit](baseline-audit.md) retained; Clang original compile active,
no completed Clang result or production migration/acceptance yet.
BOTH originals subsequently completed before migration: GNU51/55/966.60s,
Clang ASan/UBSan/float-cast-overflow51/55/1798.49s; ONLYfour new public fail,
ALL50 old/newdirect pass/all284-four1216 counters complete. All15896 prefixed
selected assertions byte-identical; both full32436-line raws/status/times/
before-after manifests retained. Driver70061 harvested0/no sanitizer report/
leaksdisabled. Allfive frozen/sixproduction/fifteen neighbors unchanged.
Existing SETPEM ListItemID already uses the native callback, so NO dispatcher
repair/expansion is required. PUTPEM already shares that native callback; its
same-property admission inherits the setter, but no separate PUTPEM acceptance
or reflection-language expansion is claimed.
Derived existing-control containment: before selected Numeric assignment,
reject31 if already materialized row count exceeds INT32_MAX, preventing a
selected display row from leaving the signed32 model domain/entering shared
llround unsafely. This is parent/hazard-derived defense, not a native observation
or tested allocation/performance claim; no caller-sized row allocation occurs.
Bounded Numeric/exact adapter wired03:27Z AFTER BOTH originals; ONLY
prg_engine.cpp changes, existing SETPEM callback/generic synchronizers unchanged.
Focused ID/text then accepted MultiSelect ALL-bit clearing and unbound ComboBox
no-row Character clearing implement fresh observations. Initial five frozen/
fifteen neighbors unchanged; fixed26 input manifest/full55 serial GNU then
Clang plus both sixgroups driver88536 now runs. No fixed acceptance yet.
Rollback: revert only bounded adapter/route and corrected linked guidance;
retain frozen probes/tests/failed evidence. Field notification: identify exact
head and affected mode/control/input on PR/channel; never claim parent/family/
KBX/hazard closure/release qualification. No VM/backup-reset completion assumed.

First fixed GNU run54/55/958.28s: allfive new subjects PASS/all284/four1216
counts, only TWO obsolete non-frozen list_controls_02 smoke literals fail.
[Full phase audit](baseline-audit.md) retains failed attempt1, unchanged initial
five/fifteen neighbors, independent matching two-row VFP9 ID77 diagnostics and
the bounded old-literal correction to success/row0. Native direct assignment
is observed; SETPEM success is same-setter Copperfin route derivation, not native
SETPEM syntax. No production/frozen-input change to make this correction.
Repeated identical full55/sixgroups GNU then Clang runs in69036; no full fixed
acceptance yet. Failed raw remains preserved, not overwritten or normalized.

## Complete local verification and bounded documentation walkthrough

Both complete fixed55 suites subsequently PASS: GNU55/55/994.77s completed
04:06:59Z; Clang ASan/UBSan/float-cast-overflow55/55/1825.00s completed04:40:57Z.
Driver69036 harvested0; all284direct/four1216public and every neighbor counter
complete, both six unchanged preservationgroups PASS0. Initialfive/all27 fixed
inputs unchanged before/after; no fixed FAIL/sanitizer report, leaksdisabled,
not warningfree/leak/performance qualification. Full633-line raws/compiler/
status/times/hash manifests and failed attempts remain unedited; see
[full phase audit](baseline-audit.md). Five documentation contracts PASS5/5/
337.08s/status0 at04:46:43Z, driver81935 harvested0,210 valid fragments;
[full contract raw](contracts.log) SHA256
7c49674b8e4ce7dd1ef39bb5320a32d0c4416d8332974181f43d8e3f76b0bddb.

DQ/DV-5611-LISTITEMID-NUMERIC-001 boundedmedium delegated development
maintainer self-review/walkthrough completed2026-10-12T04:48Z by Codex under
owner-authorized slice judgment, NOT independent-human/high-hazard signoff.
Reviewed independent native sources/results/help, literal tests, narrow setter,
projection helper, CMake/isolation registration, linked RQ/VR/HZ/docs/rollback.
Automated state walkthrough: sparse2.9 ->focus0/ID0/empty BOTH classes versus
dense2.9 ->focus2/ID2/second; sparse22.9 ->focus2/ID22/second. Successful
MultiSelect valid-ID focus retains text but ALLbitsfalse, rejection preserves
initial focus2/text/true-true-false. Default2147483648 ->localized31; explicit
VFP9 UINT32_MAX+.1 ->46, continuation/context/reset/sentinel/state checked in
four locales. Exact INT64_MIN+22 ->legacy sparseID22 LOSSLESSLY but default31;
this and NaN31 BOTH modes are explicitly parent-derived, not VFP CAST/NaN
syntax observations. Native two-row77 clearing corroborates corrected oldsmoke;
reflection success is a Copperfin same-setter route derivation, not native
SETPEM observation. All expected state is independent literal evidence, not
helper-generated output. Existing count guard is unallocated defensive domain
containment only; no all-materialization/callback/ControlSource atomicity claim.
Procedural delta/misuse/rollback/notification and active hazard severity/status
remain as mapped above. Local slice evidence complete; signed exact-head
postpush/hosted checks/Claude-FIRST review/resolved conversations remain gates.
No parent/family/KBX/hazard/release qualification or VM/backup-reset closure.
