# Collection Numeric selector recovery (bounded development verification)

Authority: OPEN/owner-authored/agent-approved #5611/#6776, revalidated
2026-10-09 before bodies. Base main `8a9f8f61a5e4267df769bb84ce7d5652fe024364`.
Only Numeric/exact-integer conversion in `resolve_native_collection_slot`
is selected. Item/default-item/Remove share this resolver; hidden read-only
collections retain their mutation guard. Helper and independent assertions were
frozen before both original baselines; only then were resolver/mode callers migrated.

## Independent evidence

Installed Wine VFP9 SP2 `09.00.0000.7423`, existing bounded vfp9-probe harness:
three serial byte-identical 78-line runs. Fresh three-member Collection for
every Item/Remove operand; independent alpha/beta/gamma identities, Count and
surviving order recorded. No VM, dialog, installer or argument-sized allocation.
Source SHA-256 `ab8901caf29915a9e1d6e16e285aa2163f10dae8d14cde15bdd900b89f866428`.
probe.out/run1.out/run2.out/run3.out SHA-256
`fdd68ec70e0fa9f077f5754d699513a9c097b9590840d575b0cd222cdccf8447`.

Numeric 1.4/1.5/1.9 selects alpha; 2.5/2.9 selects beta; 3.5/3.9 selects gamma.
Both-sign wide aliases4294967297/98/99 and -4294967295/94/93 select1/2/3.
Native missing/zero/most negative selectors raise2061 without mutation.
Remove(-1/-1.5/4294967295) clears the collection; this is a separate sentinel
semantic, NOT a positive-slot alias. Currency$1.5/$2.5 truncates; Item($0.5)
raises11 whereas Remove($0.5) raises2061. The focused native-error/sentinel/
Currency residual #7104 is OPEN and unadmitted; no repair is selected here.
The residual report distinguishes current-main source inspection from its
uncompleted independent Copperfin residual reproduction. This slice's original
and fixed Numeric scripts verify the preserved temporary soft fallback without
claiming native error parity. No Windows/platform qualification claim.

Shipped Collection help was read fully: Item topic
`3d6d88cb-99a9-4533-9667-f9213e21c448`, Remove topic
`2758cf89-5f74-4543-a43e-dca68c69f695`. Numeric selectors1..Count or string keys
select existing members; absent members/types error; Remove(-1) clears all.
Item is the default method. Existing Copperfin soft empty/false returns are
compatibility gaps, NOT recovered native requirements.

## Governing bounded requirement, mapped before migration

RQ-CF-PRG-COLLECTION-SELECTOR-NUMERIC-001 derives safe default finite Numeric
truncation into1..INT32_MAX and exact-integer precision/nonfinite containment
from RQ-CF-PRG-NUMERIC-BEHAVIOR-001 (#5611/#6776), with ordinary truncation and
explicit-VFP9 both-sign low32 positive aliases independently observed above.
No narrowing occurs before representability/positive-count checks. Preserve
string/key routing, accepted other coercions with checked containment, item
identity/order, Count/key alignment, selected session, object lifetime and
read-only hidden collection guards. Preserve existing temporary empty/false
fallbacks without claiming native error parity; #7104 tracks the conflict.
Native Remove sentinel handling, Currency parity, general type errors and
collection lifecycle/ordering expansion remain outside this conversion slice.

Architecture: checked_collection_selector_argument plus shared resolver
conversion only; explicitly route selected-session NUMERICBEHAVIOR from all
three expression callers and explicit method focus dispatch to Item/Remove
resolution without global/static state or changes to key routing. Do not reuse
menu conversion solely because it looks similar: compare every accepted class
and boundary against this fixture and parent policy first.

VR-5611-COLLECTION-SELECTOR-NUMERIC-001/002 bounded development results:
116direct/140public/26hidden cases and eight old neighbors frozen before first
configuration, hashes/full original and fixed logs in baseline-audit.md. GNU original exit8/2.39s,
exactly398 selected identity/removal/hidden failures, zero other failures,
neighbors0.13s PASS; full checksummed original log retained. Clang original
exit8/2.96s: the exact same398 FAIL lines/categories/order, zero other failures,
neighbors0.11s PASS and no sanitizer diagnostics (leaks disabled). Full
checksummed original-clang.log retained before resolver migration. No repairs
or weakening of helper/assertions/CMake between original and fixed inputs.
Fixed GNU two executables PASS2.41s, fixed Clang sanitizer two executables
PASS2.90s/no runtime diagnostics/leaks disabled. Shared/broader GNU15/15
PASS460.79s includes older Numeric, all four shared menu query consumers,
localization and seven repository contracts. Shared Clang sanitizer3/3
PASS74.92s/no diagnostics/leaks disabled (slower than focused pass, no inferred
cause). Full actual logs/hashes/timings are retained, not estimated.
Cover native identities/fractional endpoints, both modes,
nonfinite/adjacent representable limits/exact64, key routes and independent
post-Remove survivors/count/key alignment plus hidden read-only/pageframe/window
neighbors. Guard ordinary integer controls, selected-session mode isolation,
cleanup and no mutation for rejected operands. No test/assertion weakening
after freezing. Fixed focused/shared/broader GNU and ASan/UBSan/
float-cast-overflow verification completed; post-push exact-head proof, Claude-first/
authorized clean review, all required checks and resolved conversations.

DQ/DV-5611-COLLECTION-SELECTOR-NUMERIC-001 development review/walkthrough:
implementation-agent self-review of recovered source/parent, mode routing,
pre-narrowing count bound, untouched key/erase/read-only guards, frozen tests
and documentation scope completed; this is not independent-human review.
Automated procedural walkthrough completed on both focused fixed compilers:
1.9 selects alpha through all five public read surfaces, then Remove preserves
beta/gamma order and matching keys; default4294967297 rejects without mutation,
explicit VFP9 selects/removes alpha; selected session2 uses its own mode and
preserves cursor context; hidden Controls1.9 selects alpha while Remove stays
false/count3; release/reset restores session1/COPPERFIN/closed cursor. Invalid
-1 preserves the recorded soft fallback gap, not native clear-all parity.
Shared/broader results are retained above; exact-head external integration
gates remain pending and parents/residuals stay open.
Medium documentation-misuse delta: fractional
position conversion and opt-in aliases can otherwise select/remove the wrong
member; do not describe aliases as default or -1 as an ordinary slot. Active
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 retain registered
severity. Rollback confines helper/resolver/routing/tests/docs, retains other
Numeric work, and publishes corrected slice/release guidance if necessary.
No independent-human/high-hazard, full native/type/sentinel/leak/GUI/platform/
release acceptance; parent and residual issues stay open.
