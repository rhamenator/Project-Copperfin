# BITXOR Numeric/exact operand conversion

RQ-CF-PRG-BITXOR-NUMERIC-001 is mapped BEFORE helper/test creation and
dispatch migration. Bounded admitted scope: #5611/#6776/#6871, base main
b4cd165e39ac64f5e474d88a141d991c206efcae. Not a new defect-hunt lane.

## Independent recovery

Installed Wine VFP9 SP2 reports 09.00.0000.7423. Shipped help topic
b7858458-a38e-492c-b8b6-0d6c42324fb6.htm describes same-type Numeric or
binary overloads, 2..26 arguments, integer conversion before XOR, and distinct
right-zero-padding for binary inputs. No binary/decompiled implementation input.
The complete probe and generated outputs are retained here. Initial output and
three fresh serial repeats each contain112 lines/110 calls and match byte-for-byte.
Probe SHA-256 ceed86d416a62730421d1996feb81fffe5e7745f892b14959ca768df943a55d2;
initial/probe/run1/run2/run3 output SHA-256
7370c524aaa39a1620bc51b4effa9d04af8fe2fd2358293fd90a28a089da337e.
The existing25s harness was run nice10/ionice2:7, serially, with no VM.

Identity0 in first/second slots independently exposes truncation, signed-low32
aliases on both signs, huge finite and positive/negative infinity0. All26 slots
with1.9 amid0 return1. Mixed XOR controls:5,6 ->3;7.9,3.9,1.9 ->5;
-2.9,-3.9 ->3;0,4294967297,3 ->2;1.9,1.9,1.9 ->1;
-1,-1,1E300 ->0;3,3,4294967297 ->1;-1,-1,EXP(1000) ->0.
These literal observations, not another Copperfin helper, supply native results.
NaN has no native expression in this fixture; exact64 inputs are Copperfin-only.

## Derived requirement and implementation boundary

Parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and owner #6776 policy require default
COPPERFIN finite truncation/exact integer admission within signed32, otherwise
existing localized catchable11 before assignment. Explicit VFP9 uses observed
signed-low32/huge/infinity0 conversion. Exact64 low bits without Double round
trips and NaN rejection derive from the parent safety requirement, not native
extension evidence. Every operand must be checked, including after equal-prefix
cancellation; XOR has no absorbing operand. Fold in uint32 and produce a defined
signed32 result in int64, preserving the existing int64 result kind.

Architecture: distinct checked_bitxor_argument in helpers.h/.cpp was unused
by BITXOR throughout BOTH originals and delegates only independently matched
checked conversion. After both complete originals were recorded, ONLY BITXOR
in dispatch_general.inl migrated; selected session mode read once, every operand
checked before uint32 XOR and defined signed32 arithmetic in int64.
Shared bitwise_value/bit_position, sibling bit functions,
type/Currency/binary/arity/globalNULL policy remain unchanged. Checked ordinary
other-type coercions are temporary preservation controls, NOT native parity.

Separate follow-up #7115 records the unchanged source arity ceiling255 versus
shipped help26 at this main revision. The retained native probe covers valid26,
not native27 rejection/error-number evidence. #7115 is unadmitted/unimplemented;
no arity-table or binary/type migration belongs to this slice.

## Verification chronology and local proof

VR-5611-BITXOR-NUMERIC-001/002: plan104 direct checks and488 fresh public cases
across both modes and selected sessions1/2, independent literals, first/second/
all26 slots, mixed signs/cancellation/late operands, exact64/nextafter/NaN/
infinity, assignment/error/continuation/cursor/mode/reset controls. Reuse two
unchanged expression neighbor functions. Freeze helper/header/assertions/tests/
CMake/isolation with hashes BEFORE configuration; retain complete GNU and Clang
ASan/UBSan/float-cast-overflow unchanged-dispatch results BEFORE migration.
No frozen-input repair/removal/weakening without explicit re-freeze and BOTH
original repeats. Fixed/broader/localization/contracts and exact-head review/
hosted gates were outstanding at freeze; the chronological results below
distinguish completed focused proof from remaining acceptance gates.

GNU unchanged-original completed:104direct/488public,388 selected failures
(86error/302result;278COPPERFIN/110VFP9), zero unrelated assertions,
neighbors PASS. Total8.13s; full unedited log and frozen hash audit retained in
baseline-audit.md. At this GNU-only checkpoint Clang unchanged-original had
to complete BEFORE dispatch migration; expected failures are not acceptance.
The planned104direct/488public inputs are now frozen and registered; configuration
confirmed RUN_SERIAL and scoped private filesystem/environment classification.
No helper/assertion/test/CMake/isolation input has been changed after freeze.

First Clang unchanged-original build succeeded, but Numeric timed out180.38s
before its final counter; neighbors PASS0.10s. This incomplete attempt emitted
302 selected failures, not all-scenario evidence. Its full raw log and identical
timeout copy are retained, never overwritten. The SAME binary/flags/frozen inputs
then repeated with a local CLI600s allowance and a distinct full raw log;
workflow and test definitions remained unchanged. No BITXOR migration was permitted
until a complete unchanged Clang original was recorded. Read-only host capacity/
contention samples do not establish a timeout root cause; no VM was used.

The unchanged Clang repeat then completed365.45s, Numeric FAILED365.34s/
neighbors PASS0.10s, actual104direct/488public. All388 selected FAIL lines match
GNU byte-for-byte (86error/302result;278COPPERFIN/110VFP9), zero unrelated
assertions or sanitizer diagnostics. Both complete originals/raw hashes and
unchanged frozen/dispatcher/platform hashes are recorded in baseline-audit.md
BEFORE migration. The incomplete first attempt remains retained. No frozen
input was repaired, removed or weakened; no timing/root-cause/optimum claim.

Fixed GNU one-job rebuild and focused verification completed exit0,2/2 PASS,
Numeric347.92s/neighbors0.12s,total348.05s; actual104direct/488public.
Full raw log/hash and unchanged frozen/shared hashes are retained in the audit.
Broader GNU then completed16/16 PASS449.11s, including older Numeric80.58s,
NULL, BITOR/BITAND/BITNOT/Collection, localization13.55s and seven contracts.
Safety contract324.55s kept its existing1200s cap; other tests CLI600s.
Actual counters/full raw log/hash are in the audit. Fixed Clang then completed
8/8 PASS180.93s: BITXOR15.09s/neighbors0.10s, older Numeric131.99s, NULL,
BITOR/BITAND/BITNOT/Collection. Actual104direct/488public and shared counters
retained; no assertion or sanitizer diagnostics, leaks disabled. Full logs/hashes
and unchanged six frozen/shared/dispatcher hashes are in the audit. No frozen
input repair/removal/weakening. VR-5611-BITXOR-NUMERIC-001/002 local proof is
complete; signed postpush exact-head verification, actual clean external review,
required hosted checks and resolved conversations remain integration gates.
The existing int64 result kind is preserved by make_int64_value, not a new
runtime-kind assertion. No family/parent issue closure or platform/release claim.

## Hazard, documentation and rollback boundary

HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 remain active at
their registered severities. This bounded command-use delta has medium misuse:
legacy-mode wide aliases can conceal invalid/untrusted operands; default callers
must handle catchable11. DQ/DV-5611-BITXOR-NUMERIC-001 requires a completed
development self-review and automated walkthrough before guidance acceptance,
not independent human/high-hazard qualification. Walkthrough must distinguish
truncation/XOR cancellation, rejected late unsafe operand preserving12345,
continuation guard3^6=5, retained session/cursor and reset1/COPPERFIN.
Rollback: revert only this slice, correct command guidance and notify affected
integrators if incorrect; retain native/original/fixed evidence. No KBX exception
is applied; no additional allocation, change to the existing bounded operand
loop or persistence changes are selected.

## Bounded development assurance completed

DQ-5611-BITXOR-NUMERIC-001 procedural delta map: fractions change half-away
rounding to truncation; default wide/nonfinite Numeric or exact64 beyond
signed32 raises catchable11 before assignment; explicit VFP9 selects recovered
low32/huge/infinity0. XOR cancellation never permits skipping a later operand.
Misuse is medium for this bounded guidance: an untrusted wide value in VFP9
mode may silently alias a mask; a default caller must handle catchable11.

Implementation-agent delegated development self-review completed2026-10-09:
independent native literals and parent-derived safety expectations compared
with frozen helper and BITXOR-only dispatch; admission precedes narrowing,
exact64 avoids Double round trips, NaN rejects and native infinity0 is only
explicit VFP9. Every operand is checked; uint32 XOR and defined signed32
arithmetic preserve make_int64_value. The existing bounded argument loop,
shared bit helpers/siblings and type/Currency/binary/arity/NULL stay unchanged.
This is development self-review, not independent-human/high-hazard sign-off.

DV-5611-BITXOR-NUMERIC-001 focused GNU automated walkthrough passed:
1.9 in all26 slots amid0 ->1; mixed -2.9,-3.9 ->3; default late1E300 or
infinity after equal-prefix cancellation rejects with nResult12345 unchanged,
lAfter true/nGuard5, cursor/session/mode retained, reset1/COPPERFIN/closed
cursor. Explicit VFP9 gives recovered low32/0 and cancellation results.
Direct exact64/NaN/nextafter and safe coercion controls pass; coercion controls
are containment, not native type parity. Broader GNU and fixed Clang repeated
the same walkthrough successfully with actual frozen counters retained.
DQ/DV-5611-BITXOR-NUMERIC-001 bounded development self-review/automated
walkthrough/rollback are complete2026-10-09, not independent-human/high-hazard
qualification or acceptance of the active registered hazards. External
exact-head review/hosted gates remain, with no type/binary/arity/platform/leak/
release/family/parent acceptance.
