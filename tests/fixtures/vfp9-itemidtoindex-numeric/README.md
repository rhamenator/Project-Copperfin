# ItemIDToIndex Numeric item-ID read requirements recovery

Pre-code map recorded 2026-10-11T10:22Z, BEFORE helper/assertions/configuration
or production edits. Admitted OPEN owner-authored agent-approved #5611/#6776;
base origin/main13281678def7e8f77fcd8d7385abe09600697bf2. Only native
ComboBox/ListBox ItemIDToIndex first-argument Numeric/exact READ conversion.
Writes/ID creation/mutation, other methods, unsupported types/NULL/Currency,
arity, RowSource/lifecycle, generic reflection/parser are excluded.

## Independent allowed evidence

Fresh installed Visual FoxPro09.00.0000.7423, existing private Wine prefix
/home/rich/copperfin-wine-vfp9, not a local VM. Unchanged /home/rich/bin/vfp9-probe
SHA2561e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06;
snapshot retained, temporary harness/config auto-delete. No credentials,
decompilation, VM start/recreation or assumption that backup reset is complete.
Matching native.out/native-repeat.out:302 lines/300 calls, SHA256
2b0a276e4cb77a16d2e9545a58b934c8d2e39fa5fbd611170cbb1f501eaaeee0;
both full runner logs identical. Fresh object for every expression, both
controls and sparse IDs11/22/33 versus dense1/2/3. Count3, selected row2,
payloads101/202/303, bits false/true/false and identity controls unchanged.
Initial exploratory output formatted sparse identity controls as '*' via
TRANSFORM. That raw/source remain in owned build as native-first.out and
probe-first-snapshot.prg; final explicit STR(...,20,0) controls recover actual
11/22/33. Only formatting was corrected BEFORE freeze; no result was inferred
from '*', and no Copperfin result supplied a requirement.

Sparse11.9=>Numeric1,22.9=>2,33.9=>3,21.9=>0,2.9=>0. Dense1.9=>1,
2.9=>2,3.9=>3. Fractions truncate, input is a unique ITEM ID, returned value
is its one-based row position. Ordinary negative/zero/missing IDs return0.
Both-sign low32 aliases independently observed: positive4294967307=>sparse1,
4294967318.9=>sparse2, negative-4294967285=>sparse1,-4294967274.9=>sparse2,
-4294967263=>sparse3; dense4294967297/.9 and-4294967295/.9=>1,
-4294967294.5=>2,-4294967293=>3. No positive raw INT32_MAX gate;
INT32_MAX+.1/.5/.9 and2147483648 return0, as do huge/plus-minus Infinity.
These absent-ID outputs do not uniquely recover every intermediate converted
integer; representable-domain/adjacent/exact boundaries below are derived.
Logical TRUE, NULL, Character'22.9', Currency$22.9 error27 in native controls.
Those types remain outside implementation; preserved Copperfin coercion
controls are not native type-admission requirements. Both complete originals
confirmed the same-input coercions; separate gap#7149 is filed against exact
base13281678d without implementation admission or scope expansion.

Shipped Microsoft ItemIDToIndex Method help completely read:
/home/rich/temp/vfp9-help-len/html/e50ccf1d-fc0c-4467-b2de-ac1beef406b3.htm,
SHA2566ae417066f393f23a1c1ba9290d2e54cbe6222ae2503b13e0e0f1f525d01705b.
It distinguishes stable unique item IDs from mutable display positions,
starting at row1, and applies to both controls. Do not substitute sibling
SelectedID or IndexToItemID requirements as this method's oracle.

## Requirement and architecture before code

RQ-CF-PRG-ITEMIDTOINDEX-NUMERIC-001 derives from admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner default-mode policy and active
HZ-runtime-crash-01/HZ-data-corruption-01. Default COPPERFIN truncates finite
Numeric toward zero, admits converted signed32 INT32_MIN..INT32_MAX and
raises localized catchable31 on invalid/nonfinite/outside-domain values.
This rejection/domain/error is derived containment policy, not native error31.
Exact signed/unsigned64 compare losslessly without Double. Negative/zero/
absent converted IDs retain Numeric0; found ID returns one-based position,
not the ID/payload/selection bit. Explicit VFP9 uses independently observed
both-sign signed-low32 aliases, huge/Infinity0, without SelectedID's positive
ceiling/nonnegative gate. NaN rejects in both modes (derived containment,
not native/public NaN syntax). Exact64 low32 projection is parent-derived,
not native extended syntax; INT64_MIN+22 aliases sparse row2 only in VFP9,
UINT64_MAX gives signed-1/missing0. Nextafter/subnormal expected conversions
come from independent arithmetic, not native decimal-parser claims.

Distinct checked_itemidtoindex_numeric_argument remains UNUSED through BOTH
complete original GNU and Clang sanitizer suites. Then bounded raw Numeric/
exact read_native_list_control_index_for_item_id_numeric adapter for method
and property-first native reflection. Four existing public paths already retain
raw PrgValue: ordinary parentheses/brackets and postfix parentheses/brackets.
Keep _Access/visibility/delegate ordering, textual GETPEM/unsupported coercions,
current-session mode, existing lookup/result type and materialization order.
Property-first nonpositive raw selectors may bypass old method fallback;
no arbitrary callback/materialization-count or whole-call atomicity claim.
No argument-sized allocation/iteration; lookup bounded by materialized rows.

VR-5611-ITEMIDTOINDEX-NUMERIC-001: frozen both-mode direct Numeric/exact64,
NaN/Infinity/nextafter/subnormal/unsupported-adapter literal expectations.
VR-5611-ITEMIDTOINDEX-NUMERIC-002: frozen both controls/modes/two sessions,
sparse/dense IDs, four public routes, Numeric return/missing0, localized31,
sentinel/continuation, context/cursor/reset and stable count/data/bits/keys/
ListIndex/ListItemID. Full counters and every original failure retained BEFORE
migration, including all unchanged numeric/exact/isolation/itemid-selector and
IndexToItemID/SelectedID/Selected/ItemData/GETCOLOR neighbors. Any frozen repair
requires explicit refreeze and BOTH originals repeated. Fixed identical suites,
unchanged six list-control preservation groups under both compilers, docs
contracts and actual exact-head hosted checks/review are separate gates.

## Documentation assurance before code

DQ/DV-5611-ITEMIDTOINDEX-NUMERIC-001 links the active runtime/data hazards and
HZ-doc-command-01 without changing severity/status. Bounded documentation
misuse medium: unintended row could misdirect downstream application actions;
this READ slice does not persist data or qualify high-hazard operations.
Procedural delta/walkthrough: sparse22.9=>row2, not rounded missing23;
row-index2=>missing0, not row2; negative/zero missing0; positive4294967307
modern31/VFP9 sparse1 and INT64_MIN+22 modern31/VFP9 sparse2; huge/Infinity
modern31/VFP9 0, NaN both31. Walk all routes/modes/sessions/locales and stable
state, then retain results of delegated development maintainer self-review and
automated contracts, never independent-human or release qualification.
Rollback only this bounded slice, rerun frozen neighbors, notify integrators
through changelog before re-release; no parent/family/KBX/hazard closure.

Reproduction: inspect probe.prg's owned output path, copy to owned
build/itemidtoindex-native and run retained runner with --result at that exact
native.out, preserving every raw before overwrite. FXP is uninterpreted scratch.
Private700 TMP; serial nice10/ionice2:7/-j1 builds; LOCAL600s diagnostic test
bound, not CI/performance policy. Clang ASan/UBSan/float-cast-overflow/halt1,
leaks disabled; no leak/idle-host/warning-free qualification. Full configure/
compile warnings and original/fixed raw/status/hash/counters are required.

Mapping/freeze and both complete originals are retained in baseline-audit.md:
GNU30/34 PASS345.32s, Clang30/34 PASS624.15s; onlyfournewpublicconversion shards
fail, all248direct/four944public and every unchanged neighbor counter complete.
The three-file bounded method/raw-selector migration follows those originals;
five frozen files and expression parser remain unchanged. Fixed GNU34/34
PASS348.49s and Clang ASan/UBSan/float-cast-overflow34/34 PASS627.52s;
all248direct/four944public and every unchanged neighbor counter complete,
zero failed assertions or sanitizer diagnostics. Both compilers pass the six
unchanged list-control preservation groups. Full raw bytes/hashes are retained.

## Development DQ/DV walkthrough completion

Delegated maintainer self-review completed2026-10-11 after both fixed runs:
the independent sparse/dense requirements distinguish ID from row; the
procedural delta above matches all four routes, both sessions/modes and locales.
Positive4294967307 and INT64_MIN+22 reject31 in default and find sparse rows1/2
only in VFP9; huge/Infinity default31/VFP9 missing0 and direct-only NaN both31
match the frozen expectations. Error catchability, sentinel/continuation,
current-session mode and stable count/data/bits/keys/ListIndex/ListItemID pass.
Unsupported-type controls retain old coercions rather than claim native27;
#7149 remains separate. No parser, write, ID mutation or allocation policy
changed. Existing row materialization precedes conversion: this is not a claim
of arbitrary-callback count preservation or whole-call allocation atomicity.
Rollback remains bounded with frozen-neighbor rerun and integrator notification.
This medium development review is not independent-human or release qualification;
active hazards retain their severity/status. Five documentation contracts
PASS334.24s, including206 valid fragments and the full safety-workflow contract.
Full contracts.log SHA256de3f22191b6b3d235295d843887784f8c9897ba091da0437c5681d1b7c9b642f
retained unedited. Exact-head hosted checks/review remain separate pending gates.
