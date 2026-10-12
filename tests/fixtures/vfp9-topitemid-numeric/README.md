# TopItemID Numeric assignment independent pre-code recovery

Current local completion: repeated identical full65 GNU1371.49s and Clang
ASan/UBSan/float-cast-overflow2731.66s PASS; both144-call excluded-scope
comparisons/both six preservation groups/all31 fixed hashes PASS; five
contracts5/5/324.11s/212 fragments PASS. Original/failure chronology below and
in baseline-audit.md is retained, not retroactively changed. Frozen five
unchanged; signed/postpush/external exact-head integration still required.

Admitted owner-authored OPEN/exact agent-approved #5611/#6776 checked
2026-10-12T12:46Z. Slice fix/topitemid-numeric-5611 starts from origin/main
bb31a11ef6d7724dcebb1f4cc30f8d685cb9f726. No helper, assertion, CMake,
isolation configuration or production migration was written before this map.

Shipped VFP9SP2 TopItemID help read completely:
/home/rich/temp/vfp9-help-len/html/1183ce1c-4aaa-432c-8032-a9548c914951.htm,
SHA25642101d16da63d7816d7f15d77979931d02c5ac0d2d82ed43c3baebe368616154.
It specifies the ID of the first displayed item, ComboBox always read-only,
ListBox writable at runtime. Its printed TopIItemID syntax typo is retained
as provenance, not copied into the implementation. Native default text is not
sufficient evidence for the actual empty-control representation.

Fresh real installed VFP9 09.00.0000.7423 observations through existing private
Wine runner /home/rich/bin/vfp9-probe (snapshot retained; SHA256
1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06).
No VM start, OS/installer change, credential access or decompiled input.
Veeam reset/VM cleanup completion remains unconfirmed. Source/input/state
enumeration scaffold reused from a prior probe, but ALL TopItemID assignments
and outputs are fresh, not TopIndex/ListItemID helper or expected-result data.

Both runs of each source returned status0, cmp0 and complete DONE counts:

| Pair | Calls/lines | SHA256 of identical full raw |
| --- | --- | --- |
| main | 675/677 | 15dccf060f44d19bc264b6a5598d1278e4403ead4c5f8b022706e7bb10f3d1b5 |
| heights18/50 edge | 348/350 | eaf8dd06f221baeada171d25067a6e5e03ff13af9df32f265604b78c90024ea0 |
| signed/raw boundary | 144/146 | 57ee9558a9bd01d392908fa0ca264ab29b4e42d085bead0fc3bbcb15652e5621 |
| high-ID setup diagnostic | 96/194 | 5955768474e0431402cc8c4e0466a4d3a7c241c3f09e50b58e53bf1f7c1ff3e8 |

The boundary raw coincides with earlier TopIndex raw because every chosen
identity was absent in those three small shapes; this is a fresh paired run,
not evidence that row addressing and item-ID addressing are interchangeable.
High-ID setup diagnostic catches AddListItem error11 for all eight attempted
IDs2147483646..4294967296; only first ID11 remains. Its subsequent96 assignments
are absent-ID no-ops. No positive high-ID lookup requirement or AddListItem
change is inferred, and this diagnostic is not a populated-high-ID oracle.

## Governing requirement before helper/assertions/config

RQ-CF-PRG-TOPITEMID-NUMERIC-001 parents: admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, shipped help, fresh native pairs above.
Scope ONLY non-NULL Numeric/int64/uint64 TopItemID assignment on native ListBox,
materialized unbound RowSourceType0, empty ControlSource and coherent key/item
storage. ComboBox read-only and excluded routes are preservation controls.

Native nonempty state is seeded at top row2/ID22 sparse or2 dense, focus2/
focusedID22 or2, Character second Value/DisplayValue. Fractions truncate:
1.9/2.9/3.9 address IDs1/2/3 in dense rows; sparse controls with IDs11/22/33
keep top2 for these requests. Sparse11.49/11.5/11.9 selects row1/ID11;
22.9 keeps row2/ID22;33.9 selects row3/ID33. ID21.9/34/zero/negative/other
absent IDs are accepted no-ops, not clear/reset/focus changes. Heights18/50
give the same tested effects. Selection bits (single F,T,F; multi T,T,F),
focus, text, all cells, payload101/202/303, count and identities remain intact
on successful assignment, no-op and rejection.

Positive RAW Numeric<=UINT32_MAX admits truncation/defined signed low32;
RAW UINT32_MAX+.1/.5/.9 and larger/+Inf reject46 before modeled top mutation.
Negative aliases -4294967295.9/-4294967294.5/-4294967293 address IDs1/2/3;
-4294967285/-4294967274.9/-4294967263 address IDs11/22/33.
Negative huge/-Inf projects zero and is an accepted absent-ID no-op. The
normal documented identity/pair-only/no-op/focus contract is plausibly relied
on by correctly written programs and shall hold in BOTH modes.

Explicit VFP9 mode reproduces the measured RAW ceiling46 and defined low32
quirks; COPPERFIN defaults to finite truncation checked in signed32 and raises
localized catchable31 for out-of-domain/nonfinite input. Exact signed/unsigned64
uses lossless signed32 default admission and explicit VFP9 RAW ceiling/low32
projection without Double conversion. Exact64/CAST and NaN31 BOTH modes are
named parent/hazard-derived Copperfin extensions, NOT native CAST/NaN syntax.
Helper accepts only selected non-NULL Numeric/exact values, no type expansion.
Native Logical/NULL/Character/Currency1732 observations remain exclusions.

Native empty state before/after is TopIndex1/TopItemID-1; empty selected numeric
assignment no-ops relative to captured before-state. Existing absolute gap
#7157 remains outside/unadmitted: do not pin Copperfin0/0 as native compatibility
or change constructors/shared synchronizers to rationalize this conversion.
ComboBox rejects all225 observed main assignments1743 before conversion,
including overflow; existing SETPEM False/readonly metadata/fullstate controls
preserve this boundary without claiming native SETPEM syntax.

## Architecture and verification plan

Distinct checked_topitemid_numeric_argument SHALL remain UNUSED until BOTH
complete original65 subjects (all previous60 plus new direct/four public
mode-session subjects) finish with frozen inputs and unchanged production/
neighbors. Only afterwards wire selected native ListBox direct setter and
existing selected typed SETPEM/PUTPEM callback routes, using current session
mode. Find the projected positive existing ID, derive its bounded row, set ONLY
top pair; admitted absent/empty no-op succeeds. Parent-derived count>INT32_MAX
guard raises31 before new modeled pair mutation, avoiding unchecked row display
conversion, not a native allocation limit or tested huge-array claim.

Original writer, TopIndex adapter/shared synchronizer, other properties,
constructors/default representation/types/NULL/Currency/lifecycle/parser/
generic reflection/arbitrary callbacks/ControlSource persistence excluded.
Preserve existing visibility/_Assign/delegate/provenance ordering. No
whole-call materialization/allocation/callback atomicity or performance claim.
PUTPEM is an existing alias, not independently native-probed syntax.

VR-5611-TOPITEMID-NUMERIC-001/002: literal independent Numeric projection/final
dense+sparse top-row tables, signed/raw adjacency, exact64 extrema/aliases,
NaN containment, full state, single/multi/empty/readonly shapes, BOTH modes/
sessions/direct+SETPEM, four locales, catch/continuation/context/reset.
Freeze helper header/source, new literal test, CMake and isolation inputs BEFORE
BOTH full original GNU/Clang ASan/UBSan/float-cast-overflow65 runs. Then identical
fixed65, unchanged neighboring preservation groups, five documentation
contracts, signed/DCO postpush and clean exact-head external integration gates.
Any frozen repair requires explicit refreeze and BOTH complete originals again.
No helper-produced oracle or subset success. Acceptance is pending.

## Documentation assurance before code

DQ-5611-TOPITEMID-NUMERIC-001 / DV-5611-TOPITEMID-NUMERIC-001 link active
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 unchanged.
Bounded documentation misuse medium: confuse unique ID with relative row or
focus; falsely treat absent no-op as clear; claim empty absolute parity despite
#7157; copy unsupported native exact64 syntax; treat failed/subset checks as
acceptance. Delta: operation-specific ID lookup/top-only/no-op contract, RAW
ceiling vs default/exact derivations, readonly/excluded routes and phase gates.
Delegated development self-review + automated state walkthrough/five contracts
are required before completion, not independent human/high-hazard signoff.
Rollback only bounded adapter/typed entry and guidance; retain native/frozen/
failed raw evidence. Field notification names exact head and affected surface,
not parent/family/KBX/hazard closure or release qualification. DV pending.

## Completed delegated development assurance

DQ-5611-TOPITEMID-NUMERIC-001 / DV-5611-TOPITEMID-NUMERIC-001 bounded-medium
development self-review and automated procedural/state walkthrough completed
2026-10-12 after BOTH full fixed suites and all five contracts passed, using
owner-delegated implementation judgment. This is not independent qualified
human review or closure of registered high/catastrophic hazards.

The walkthrough compared fresh identity/fraction/absent-no-op observations
with separate literal dense+sparse fields (172 independently matched legacy
status/top-row entries). Complete284 direct/four1445 public mode-session cases
then exercised the actual writer, not helper-generated expectations. Both65
suites validate unchanged focus, bits, text, identities, cells/payload/count,
successful absent/empty no-op, localized31/46 and NaN31 catchability,
continuation/guard-cursor/session/mode reset, readonly ComboBox and four locales.
Both144-call diagnostics exactly preserve excluded RowSource/bound routes
against the independently captured unmigrated baseline; preservation evidence
is NEVER a native requirement. Both six existing selection/removal/sort/
multiselect/mover-bar groups pass unchanged.

Expected operator outcome: an existing unique ID moves ONLY the top pair;
admitted absent ID preserves prior state; rejected domain/nonfinite input is
catchable without modeled top mutation. IDs are not row numbers or selection.
Empty before/after is verified, NOT absolute VFP default parity despite7157.
Exact64/NaN are named parent-derived extensions, not native syntax. RAW positive
ceiling differs from truncation-domain default. The ONE old False-to-True
smoke literal has fresh exact999 native corroboration, not a code oracle.

Self-review checked both materialized-unbound entry gates/current-mode lookup,
bounded row derivation/callback-provenance ordering/no caller-sized allocation,
unchanged original writer/synchronizers, original-before-migration chronology,
unchanged frozen inputs and retention of failed phases. Five documentation/
signoff/channel/safety-workflow contracts pass. Rollback and field-notification
plan above applies ONLY to this adapter/guidance and names exact signed head.
No warning-free/leak/performance/whole-call materialization/allocation/callback
atomicity, independenthuman/high-hazard/parent/KBX/hazard/release qualification.
