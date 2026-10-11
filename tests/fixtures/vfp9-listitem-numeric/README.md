# ListItem Numeric item-ID read requirements recovery

Pre-code map recorded2026-10-11T13:18Z BEFORE helper/assertions/configuration
or production edits. Admitted OPEN owner-authored agent-approved #5611/#6776;
base origin/mainc2073149c996474f1beb753e776a20df902c6897. Bounded slice:
native ComboBox/ListBox ONE-ARGUMENT ListItem Numeric/exact READ item ID,
implicit column1. Explicit column forms/conversion, writes/ID creation/removal,
types/NULL/Currency/arity, other consumers, RowSource/lifecycle and generic
reflection/parser excluded. This does not finish the whole ListItem family.

## Independent allowed evidence

Fresh installed Visual FoxPro09.00.0000.7423 in existing private Wine prefix
/home/rich/copperfin-wine-vfp9, not a VM. Unchanged /home/rich/bin/vfp9-probe
SHA1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06;
snapshot/full source/runner logs/raw matching pair retained. Matching302line/
300call native pair SHAd56fbc6bec870ab93a7ae5ab3cad564132494c3d0c05fdce96ff4cb747d9183c.
Fresh object per expression, both controls, sparse11/22/33 versus dense1/2/3.
Count3, selectedrow2, payloads101/202/303, bitsfalse/true/false and actual
identity controls unchanged. No credentials/decompilation/VM/backup assumption.

Numeric fractions truncate: sparse11.9=>Character first,22.9=>second,
33.9=>third; row-index2.9=>empty Character, not second. Dense2.9=>second.
Converted zero/missing nonnegative IDs return empty Character; ordinary
negative converted IDs error31. Unlike ItemIDToIndex, raw positive values above
INT32_MAX (including2147483647.1/.5/.9,4294967307,huge,+Infinity) error31.
Negative signed-low32 aliases independently observed: -4294967285=>sparse first,
-4294967274.9=>second,-4294967263=>third; dense-4294967295/.9=>first,
-4294967294.5=>second,-4294967293=>third. Converted negative lowbits reject31:
-2147483648/-.9,-4294967297. Negative huge/-Infinity=>empty Character.
Ordinary fractions(-1,0)=>empty. Absent outputs do not uniquely recover every
intermediate integer; boundary/exact64 cases below are parent-derived.
LogicalTRUE/NULL/Character22.9/Currency22.9 error31 natively; outside migration.
Existing Copperfin unsupported coercions are preservation controls only, not
requirements. Evidenced unsupported-type mismatch is separate focused
unadmitted [#7151](https://github.com/rhamenator/Project-Copperfin/issues/7151),
filed against exact current mainc2073149c; no type-admission implementation.

Shipped Microsoft ListItem Property help completely read:
 /home/rich/temp/vfp9-help-len/html/4684309c-c0ed-48f6-8476-3d0603b0b69b.htm
SHA4abe44598f18c289d9de4101b788d92e6d3199b8c5f2cc33edfceb80ad14f8bd.
It defines a Character item-ID lookup for both controls, implicit column1,
distinguishing List's display positions from ListItem's item identities.
Do not substitute sibling method code, tests or requirements as an oracle.

## Pre-code requirement and architecture

RQ-CF-PRG-LISTITEM-NUMERIC-001 derives from #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and active HZ-runtime-crash-01/
HZ-data-corruption-01. Default COPPERFIN truncates finite Numeric, then admits
converted0..INT32_MAX; other values reject localized catchable31. Default upper
fraction2147483647.9=>missing empty is deliberate parent-derived checked-domain
policy, unlike native raw ceiling. Negative/positive aliases and huge values
reject default31. Explicit VFP9 first rejects positiveRAW aboveINT32_MAX,
then defined signed-low32/nonnegative admission; negative huge/-Infinity=>0,
zero/missing=>empty, found uniqueID=>first-column Character. Exact64 comparison/
projection without Double is parent-derived, not native extended syntax:
INT64_MIN+22 default31/VFP9 sparse second; positive exact aliases reject both.
NaN rejects both (containment, not a native expressible NaN claim).
Nextafter/subnormal expectations derive from independent arithmetic.

Distinct checked_listitem_numeric_argument stays UNUSED through BOTH complete
original GNU and Clang sanitizer suites. Only afterward wire an operation-
specific read_native_list_control_item_cell_numeric adapter into one-argument
method arm and raw property-first reflection before legacy cell resolution.
Expression already passes raw PrgValue for one selector only: ordinary
parentheses/brackets and postfix parentheses/brackets. No parser/signature
widening needed; explicit column/multiargument forms and textual GETPEM retain
old paths. Preserve visibility/_Access/delegate/current-session ordering and
existing bounded materialized-row lookup. No input-sized allocation/iteration.
Raw invalid selectors will not fall back through old method materialization;
stableRowSource0 only, no arbitrary callback-count/whole-call atomicity claim.

VR-5611-LISTITEM-NUMERIC-001: freeze independently enumerated both-mode direct
Numeric/exact64/NaN/Infinity/nextafter/subnormal/unsupported-helper expectations.
VR-5611-LISTITEM-NUMERIC-002: both controls/modes/two sessions/sparse+dense IDs,
four one-argument public routes, Character type/empty/error/locales/sentinel/
continuation/cursor/mode/reset and stable rows/count/data/bits/keys/ListIndex/
ListItemID. Keep all unchanged ItemIDToIndex/IndexToItemID/SelectedID/Selected/
ItemData/GETCOLOR/numeric/exact/isolation/itemid-selector neighbors.
Full original outputs/status/hash/counters BEFORE migration, no selective
measurement. Any frozen repair requires explicit refreeze and BOTH originals
repeated before migration. Identical fixed suites, both unchanged six list-
control preservation groups, documentation contracts and exact-head hosted
checks/review are independent subsequent gates.

## Documentation assurance before code

DQ/DV-5611-LISTITEM-NUMERIC-001 links runtime/data/HZ-doc-command-01 without
changing registered severity/status. Bounded documentation misuse medium:
misaddressed Character can misdirect downstream actions; READ-only slice does
not persist data or qualify high-hazard application operations.
Procedural delta/walkthrough: sparse22.9=>second (not missing rounded23),
2.9=>empty (not second),-1=>31,zero/missing=>empty; positive4294967307 both31,
negative-4294967274.9 default31/VFP9 second; positiveInfinity both31,
negativeInfinity default31/VFP9 empty; directNaN both31. Walk all four routes,
both sessions/modes/locales and preserved state; review default-upper-fraction
extension explicitly. Retain delegated maintainer development self-review and
automated contract results AFTER verification, never independenthuman/release
qualification. Rollback only bounded adapter/helper, rerun frozen neighbors,
notify integrators through changelog before re-release. No parent/KBX/hazard
closure. The bounded development walkthrough/self-review is completed below;
documentation contracts and signed exact-head integration are separate gates.

Reproduce native evidence by inspecting probe.prg's exact task-owned output
path; use retained runner --result for that path and preserve full raw/source.
No source/frozen assertion change merely to make an original pass.

## Original evidence and bounded migration checkpoint

Both complete39-subject originals finished before migration: GNU35/39/472.75s
and Clang sanitizer35/39/834.09s, onlyfournewpublicfail. All248direct/four944
public and all unchanged six neighbor counters completed. Full raw7523line
logs/configuration/compiler warnings/chronology retained in baseline-audit.md;
every actual prefixed failure assertion is identical across compilers. Old
coercion controls pass and independently disagree with native type admission,
hence #7151; they remain unchanged in this slice.

At2026-10-11T14:17Z after full retention, wired the narrow one-argument method
arm and raw-selector property-first adapter to the distinct checked helper.
The adapter returns first-column Character by unique ID, empty for zero/missing,
or localized catchable31 for rejected Numeric/exact selectors. No parser edit,
no explicit-column or type-path migration. It materializes before conversion;
no callback-count or whole-call atomicity qualification. Visibility/_Access/
delegate and session policy ordering remain outside/above the new adapter.
Five frozen helper/assertion/registration hashes and unchanged expression hash
still verify; no refreeze/weakening. Fixed identical39-subject suites and both
six unchanged list-control preservation groups ran serially in the retained
fixed-suites.sh.txt driver. GNU39/39/468.01s and Clang sanitizer39/39/821.67s
pass, complete248direct/four944public and every unchanged neighbor counter.
Both six-group preservation runs report0failures. Full unedited raw/configure/
compile/warnings and SHA-256 chronology retained in baseline-audit.md. No leak,
warningfree, performance or arbitrary RowSource callback qualification.

## Completed bounded documentation walkthrough

DQ/DV-5611-LISTITEM-NUMERIC-001: Codex, acting under the owner's delegated
implementation authority, completed development maintainer self-review on
2026-10-11 after both fixed suites and unchanged preservation groups passed.
This is not a personally posted owner/human sign-off or independent review.
The registered HZ severities/status remain unchanged; this medium bounded
documentation change does not close or qualify their high-hazard applications.

Reviewed the independent native/help map against literal expectations and all
four public routes. Sparse22.9 returns second, sparse2.9 returns empty Character
instead of row2; dense2.9 returns second. Negative1 produces localized catchable31
and preserves the sentinel while execution continues. Zero/missing IDs return
empty Character. Positive4294967307 rejects in both modes; -4294967274.9
rejects in default but returns sparse second in explicit VFP9. PositiveInfinity
rejects both, negativeInfinity rejects default and returns empty in VFP9.
Direct NaN containment rejects both, not a native/public NaN syntax claim.
The deliberate default converted-domain upper fraction2147483647.9 is empty,
where VFP9's raw ceiling rejects31. Exact INT64_MIN+22 is admitted without a
Double roundtrip only in VFP9 and returns sparse second. Both sessions/modes,
four catalogs, catchability/continuation/cursor/reset, Character kind and
unchanged count/data/bits/keys/ListIndex/ListItemID assertions all pass.

Verified review boundaries: implicit column1 only; unsupported coercions are
preserved, with #7151 separate/unadmitted. Explicit columns/arity/writes,
ID lifecycle, textualGETPEM, generic parser/reflection and arbitrary materialized
RowSource callbacks remain outside this slice. READ conversion and selected
stable-state preservation are not whole-call atomicity or persistence assurance.
Rollback/field notification remain the pre-code bounded plan above. Full fixed
raws and both six unchanged preservation outputs substantiate this walkthrough;
Five documentation contracts passed5/5 in333.19s, including the safety
traceability workflow contract. Full unedited contracts.log/status retained;
SHA9686c07ffb27e303b7a459089713433d30c9e63bbc1975c6fd94f9c66b0f728d.
Signed exact-head review/CI remain separate integration gates.
