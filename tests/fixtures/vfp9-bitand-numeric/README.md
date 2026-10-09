# BITAND Numeric/exact requirements recovery (#5611/#6776/#6871)

RQ-CF-PRG-BITAND-NUMERIC-001 is mapped before adding helpers, regression tests,
or changing production dispatch. This is a bounded Numeric conversion slice,
not full BITAND/type/binary/arity acceptance. Base: main
`67cf4fbc32552b3a8ebee299f8aedbb39f6bd948`.

## Allowed evidence

The installed Wine VFP9 SP2 build is `09.00.0000.7423`. Shipped help
`e03290b3-e96a-421c-8efa-f9b1691b09cb.htm` was read completely. It specifies
Numeric and separate binary overloads, 2..26 operands, integer conversion
before AND, and signed Numeric results. No binary was inspected/decompiled.

`probe.prg` uses independent literal input expressions and records returned
values/error numbers; it never calculates expected values from Copperfin.
Three serial runs through the existing bounded `/home/rich/bin/vfp9-probe`
harness produced byte-identical 108-line outputs: version, 106 calls, DONE.
The 106 calls cover 36 Numeric expressions in each of the first two positions,
fraction 1.9 in each of 26 positions, and eight mixed/mask/zero controls.
No VM was started; runs used nice10/ionice2:7 and the existing Wine prefix.

SHA-256:

- Source: `91d2f825d18fa05b2f0aaf1eb2bef223ce822a66737db97329462b1a32169565`
- `probe.out`, `run1.out`, `run2.out`, `run3.out`:
  `84155ab97ad2fe97da026ffa649573f29c2d7df04c5e4aa524783c7b3dd8cdcb`

An initial finite-only 102-call/104-line source and three matching outputs are
retained as `finite-probe.prg` / `finite-run1.out`..`finite-run3.out`.
Initial source SHA `bed794fd527ad4891b0c81f17de67a138dc3661adaed0a1211ce19d44b7e3bc7`;
output SHA `b825ccaf9f99e6f583d8f2ee9b7e3ea86740a58d6c31a198433d1a6ba60bff7b`.
After admitting owner-authored child #6871 metadata, its infinity observation
was recovered with fresh EXP(1000)/-EXP(1000) calls and three repeated runs.
This source refinement occurred before any helper/test freeze or baseline.
Generated probe.FXP files are scratch artifacts, not evidence; source/output
bytes are retained independently.

## Recovered and derived contract

Ordinary Numeric fractions truncate toward zero, including negative fractions.
AND with -1 exposes the converted signed32 operand in either input position.
Native 2147483648 -> INT32_MIN; -2147483649 -> INT32_MAX; 4294967295 -> -1;
4294967296 -> 0; 4294967297 -> 1, with both-sign low32 aliases. Numeric
2^53 and both-sign 1E20/1E300 and infinity expressions yield converted0.
Native mixed controls include 3,6 -> 2; 7.9,3.9,1.9 -> 1;
-2.9,-3.9 -> -4; all 26 placements of 1.9 amid -1 masks -> 1.

Derived from RQ-CF-PRG-NUMERIC-BEHAVIOR-001, the admitted owner decision and
HZ-runtime-crash-01/HZ-data-corruption-01:

- Default COPPERFIN: finite Numeric/exact values truncate into signed32;
  reject a truncated value outside INT32_MIN..INT32_MAX or any nonfinite value
  with localized catchable11 before unsafe narrowing. This fail-closed domain
  is a bounded operation-specific default, not native wide-alias parity.
- Explicit VFP9: use recovered signed-low32/huge-indefinite0 and infinity0.
  Exact int64/uint64 operands preserve their low bits without a double round
  trip; exact extended integers are Copperfin-only parent-derived boundaries.
- NaN: reject in both modes; no installed expression producing NaN was observed.
  Do not incorrectly reject native-observed infinity in explicit VFP9 mode by
  blindly sharing BITNOT's nonfinite policy.
- Validate every operand, including after an intermediate AND becomes0.
  Return signed32 semantics in the existing int64 result kind using defined
  arithmetic, not out-of-range unsigned-to-signed narrowing.
- Preserve safe existing non-Numeric coercions behind checked conversions;
  this is containment, not a requirement derived from existing code or native
  type/Currency parity. Existing NULL/arity/evaluation/session routing stays.

Architecture: new `checked_bitand_argument` in helpers.h/.cpp was unused
during both originals. Only after both raw logs were retained, BITAND dispatch
was migrated to selected-session `numeric_behavior(set_callback)`, checked
all-operand admission/localized11 and uint32 AND with defined int64 signed32
arithmetic. Existing int64 result kind is preserved by source inspection.
Shared bitwise_value/bit_position, other bit functions and global NULL/arity/
routing are unchanged, not newly implemented full native parity.
No argument-sized allocation or new expression-evaluation ordering is needed.

## Verification and assurance disposition

VR-5611-BITAND-NUMERIC-001/002 completed locally with frozen104 direct checks and
472 fresh public cases plus two unchanged neighbor functions; independent literal
direct and fresh public regressions for both modes/sessions, first/second/late
operands, all 26 positions, signed/unsigned/exact64/nextafter/NaN/infinity,
zero intermediate masks and signed results, catchable error/result unchanged,
continuation/session/cursor/reset controls and unchanged neighbors. All six
helper/assertions/CMake/isolation inputs were frozen before configuration;
hashes in baseline-audit.md. BOTH originals completed BEFORE migration with368
byte-identical selected failures, zero direct/context/continuation/reset/neighbor
failures;104direct/472public executed, GNU5.56s/Clang10.70s. No sanitizer
diagnostics/leaks disabled. Both full original logs/hashes retained in the
[audit](baseline-audit.md). No frozen-input repair/removal/weakening occurred.
First fixed GNU focused2/2 PASS5.91s, actual104direct/472public counters retained.
Broader GNU14/14 PASS426.82s includes older Numeric, NULL, BITNOT, Collection,
localization and seven repository contracts. Fixed Clang sanitizer6/6 PASS117.38s
includes focused/shared and older Numeric; no diagnostics/leaks disabled.
Actual BITAND104/472, BITNOT104/156 and Collection116/140/26 counters retained
in both verbose broader logs, with hashes in the audit. No frozen input repair.
Postpush exact-head proof, clean Claude-FIRST/authorized Codex review, hosted
required checks and resolved conversations remain integration gates. Timings
are not performance/cause claims; no broader family/parent acceptance.

DQ/DV-5611-BITAND-NUMERIC-001: bounded medium-misuse procedural delta:
ordinary fractions round -> truncate; default wide/nonfinite -> catchable11;
explicit VFP9 aliases/infinity -> defined low32/0. Misuse: selecting VFP9 for
untrusted values can turn a wide mask into another mask; default rejection can
interrupt buggy callers and requires ordinary TRY/CATCH handling. Implementation-
agent delegated development self-review completed2026-10-09: independent native
literal and parent-derived expectations compared with frozen helper/dispatch,
admission before narrowing, exact64 low bits without double round-trip, NaN
rejection/native VFP9 infinity0, defined signed result, no zero short circuit,
bounded existing operand loop and preserved other-function/session boundaries.
This is development self-review, not independent-human qualification.
Automated walkthrough completed in fixed GNU focused/broader and Clang runs: both selected sessions
show1.9 amid -1 masks ->1; default4294967297/late1E300 after0 raises catchable11
without assignment, continuation2 and cursor/mode intact; explicit VFP9 alias1
or huge/infinity0; final reset1/COPPERFIN/closed cursor. Frozen preserved-type
controls are containment evidence, not approval of native type/Currency parity.
Rollback is reverting this BITAND-only helper/dispatch/test/docs slice; if
guidance is wrong, correct the mapped row and notify affected integrators.
HZ-runtime-crash-01 (high), HZ-data-corruption-01 (catastrophic), and
HZ-doc-command-01 (high) remain active, not downgraded or closed. No KBX entry
is applied. No independent-human/high-hazard/leak/platform/GUI/release or
parent acceptance claim.

## Separate seams

Source inspection found global BITAND arity2..255 versus shipped 2..26; focused
issue #7108 records that discrepancy against this base. It is unadmitted and
not implemented here. Native invalid-call/runtime arity-error evidence is not
yet recovered; do not imply the source prediction is a completed runtime matrix.
Type/Currency/binary semantics and other bit functions remain separate.
#6871 remains open for the unfinished family, as do #5611/#6776.
