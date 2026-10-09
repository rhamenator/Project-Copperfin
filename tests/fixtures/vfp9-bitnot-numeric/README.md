# BITNOT Numeric conversion recovery (bounded development verification)

Authority: OPEN/owner-authored/agent-approved #5611/#6776, metadata admitted
2026-10-09 before bodies. Base main419390ac7a88dbd68a1cfbcae4d464b8176857c9.
Only BITNOT first-operand Numeric/exact conversion is selected. Shared
bitwise_value, other bit functions, binary signatures, general type/arity
and Currency policy remain outside this slice.

## Independent requirements evidence

Installed Wine VFP9 SP2 09.00.0000.7423, existing bounded25s vfp9-probe harness:
three serial byte-identical49-line runs,47 calls. No VM/installer/dialog/native
binary inspection. Source SHA-256 fcf60777b31070d1bd8842af5574371c30edc8586f5a9fc614a1daa8a49fa51d;
probe.out/run1.out/run2.out/run3.out SHA-256
bc16398ccfe93b30ce02a324698319136902fc68429da45c06358f9b621f9263.
Before retention, an oversized VFP source string caused harness error36; it
was split into bounded source literals. TRANSFORM hid signed-minimum output
as stars, so the final source uses explicit STR width30. Neither preliminary
attempt is counted as a successful observation or Copperfin defect.

Shipped BITNOT help topic9881350f-b6bb-4a73-9772-95d448de5397 was read fully:
Numeric input is converted to integer before complement; 5 returns-6.
Varbinary/Blob have a distinct optional start/count signature, not recovered
or implemented here. Native fractions truncate toward zero in both signs;
signed32 extremes produce their32-bit complements. Wide values wrap low32,
including2147483648 ->2147483647, -2147483649 ->-2147483648,
4294967297/-4294967295 ->-2. Huge finite1E20/1E300 ->-1.

Currency$0.5 errors11, $1.5/$2.5/-$1.5 truncate; Logical/Character/Date/Object
error11; NULL propagates; absent/extra Numeric arguments error1229/1230.
Focused residual#7106 was filed from Currency/type source conflict against
this main, not a complete independent current-runtime residual reproduction.
The frozen original preservation controls subsequently passed for Logical .T.
->-2, Character '1.9' ->-3 and Currency$1.5 ->-3 (both modes/sessions); those
results still disagree with the native type/truncation evidence above. The
rest of that residual's runtime matrix remains unfinished. Keep checked existing
other coercions temporarily, not native Currency/type parity. No binary parity
claim or residual implementation/closure; no KBX exception applied.

## Governing requirement mapped before helper/tests/migration

RQ-CF-PRG-BITNOT-NUMERIC-001 derives default finite Numeric/exact truncation
into signed32 and localized catchable11 for rejected values from
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 (#5611/#6776). Ordinary truncation/complements
and explicit VFP9 low32/huge-indefinite-zero behavior are independently
observed above. NaN/infinity rejection and exact64 precision/low32 policy
derive from that parent and HZ-runtime-crash-01/HZ-data-corruption-01, not
native exact64 evidence. Preserve checked half-away/low32 other coercions,
global NULL, arity/routing, signed32 complement, selected-session mode, cursor,
continuation and reset state; no argument-sized allocation or loop.

Architecture: initially-unused checked_bitnot_argument in helpers.h/.cpp was
frozen with the independent tests before configuration. Only after both original baselines/logs were
retained, BITNOT dispatch was migrated to selected-session
numeric_behavior(set_callback), existing localized error11 and signed32
complement calculated in int64 without unsigned narrowing. Old shared
bitwise_value/bit_position untouched; global NULL/arity routing unchanged by
source inspection, not a newly implemented full-arity compatibility contract.
VR-5611-BITNOT-NUMERIC-001/002: freeze independent direct/public expectations,
helper/test/CMake/isolation hashes BEFORE original GNU and Clang
ASan/UBSan/float-cast-overflow verification. Retain both full original logs
before dispatch migration; no repairs/removal/weakening without documented
re-freeze and repeating both originals. Actual GNU/Clang originals have110
byte-identical selected failures, zero direct/unrelated failures;104direct/
156public executed, neighbors/context/continuation/reset passed. Totals1.25s/
1.92s, no sanitizer diagnostics/leaks disabled. Fixed GNU focused2/2 PASS1.26s;
Clang focused/shared5/5 PASS106.62s (older Numeric102.46s, NULL and Collection),
104direct/156public counters retained, no diagnostics/leaks disabled. Full logs
and hashes in [baseline audit](baseline-audit.md). Broader GNU13/13 PASS412.64s
includes focused/shared, older Numeric65.83s, NULL, existing localization and
seven contracts; verbose counters retained. Postpush exact-head proof and
external review/CI remain integration gates. Timings are not performance/
root-cause claims. No frozen helper/assertion/CMake repair or weakening.

DQ/DV-5611-BITNOT-NUMERIC-001: bounded medium-misuse development delta:
fractions change from half-away to truncation; unsafe defaults reject rather
than wrap; aliases are opt-in. Implementation-agent development self-review
completed2026-10-09: checked the independent literal native/parent-derived
requirements against helper/dispatch/tests, default admission before cast,
exact64 low bits without double round-trip, nonfinite rejection, bounded
Numeric execution and preserved other-function/state boundaries. This is
delegated development self-review, not independent-human qualification.
Automated walkthrough completed in fixed GNU focused and Clang runs: both
selected sessions show1.9 ->-2, default4294967297 catchable11 with result
unchanged/continuation, explicit VFP9 ->-2, cursor/mode unchanged, final
reset1/COPPERFIN/closed cursor. The preserved Currency/type controls are gaps,
not recovered native requirements or approval to broaden this slice.
Rollback confines selected helper/dispatch/tests/docs; publish corrected
guidance/release notification if wrong. Active registered hazards above plus
HZ-doc-command-01 retain severity; no high-hazard/independent-human/leak/
binary/full-native/platform/release/parent acceptance claim. No issue closure.
