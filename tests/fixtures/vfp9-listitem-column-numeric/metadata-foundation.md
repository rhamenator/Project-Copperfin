# Numeric metadata prerequisite: owner decision and first recovery slice

## Authority and retained scope

Direct repository-owner steering on 2026-10-15 requires VFP Numeric behavior
as closely as possible, even if Copperfin's Numeric representation must be
rewritten, and likewise VFP lists/dropdowns (ListBox/ComboBox). This resolves
the earlier #7189 choice toward the representation prerequisite, not the
canonical-double shortcut. #5611/#6776 remain the admitted implementation and
switch-policy parents. The selected ListItem supplied-column read is retained,
not completed or abandoned. Broader control behavior requires separate bounded
verification, not blanket changes to every caller.

RQ-CF-PRG-NUMERIC-METADATA-001 derives from that owner instruction and the
documented native Numeric descriptor, plus the retained installed-VFP9 cause
tests. Copperfin must preserve sufficient semantic information to reproduce
evidenced Numeric propagation, display and conversion behavior. It need not
copy Microsoft's private layout. No decompilation or proprietary implementation
copying is permitted. Existing #6776 mode policy, safe checked conversions and
allocation limits remain requirements; the owner did not waive them.

## First bounded native observations

`metadata-foundation.prg` evaluates 16 fixed expressions under DECIMALS 2/6/14
and FIXED OFF/ON: 96 cases per run. It captures published FLL descriptors for
the direct temporary, assigned variable, array copy, by-value UDF parameter,
UDF return and object property. It separately captures binary64 reconstructions
and direct/assigned TRANSFORM output plus explicit STR(value,30,6). These are
observations, not Copperfin acceptance assertions or a complete propagation
formula. The existing private Wine runner and host-only diagnostic FLL are
unchanged; no VM was started and no SDK-linked binary is redistributed.

Both invocations returned 0 with empty stderr, complete 96-case framing and
stdout matching each corresponding raw result. The independent structural
auditor returns 0: the full grid is unique/complete, all copied payloads match,
and every documented type-dependent descriptor field and display repeats.

Under DECIMALS 2 / FIXED OFF, notable actual observations include:

- Assigned literal 2.5 carries one decimal place and TRANSFORM returns `2.5`;
  assigned 5/2 carries two decimal places and returns `2.50`. Their double bits
  are identical. Display parity cannot be derived from a bare double alone.
- The two same-bits near-one values also display differently:
  `0.99999999999999` versus `1`.
- Literal 1.00 has width 4 as a direct temporary, but width 13 after assignment;
  both retain two decimal places. Merely adding fields without implementing
  the evidenced propagation rules will not establish compatibility.
- Some literal values cross the published FLL interface as type I rather than
  type N. Only type/width/integer are documented for I; the unused decimal and
  real slots are not evidence of Numeric precision or a second payload.

Raw captures are intentionally not normalized. The two raw files differ in
one unused ev_long slot of a type-N assigned value (-0.00 under FIXED ON /
DECIMALS 2). Consequently the full raw cmp returned 1: these captures are NOT
claimed byte-identical. The auditor compares all documented fields for each
type and retains original unused bytes verbatim. This distinction does not
alter the earlier byte-identical qualifying cause-factorial/settings pairs.

First raw SHA256: 1c28d5d4c013d20649960f1900e99758b1a7c061f2f6b5ccf3336d28a2ac15e7.
Repeat raw SHA256: 77ee17253ff057e3e575fdfb00bc4664d42ae341e91d392a93cf993a72e5aa7f.
The first auditor attempt returned 1 because its footer expected DONE|96;
FIXED ON / DECIMALS 14 also formats the probe's own TRANSFORM(total) as
DONE|96.00000000000000. The corrected auditor validates exact framing and
numeric count; original failure/status are retained, not relabeled as passing.

## Implementation and verification sequence

1. Extend allowed-source recovery only as needed to specify literal creation,
   copy/assignment and arithmetic/function propagation; do not infer all rules
   from these 16 expressions. Recover display consumers independently, including
   ?/?? where their policy differs from TRANSFORM and explicit STR pictures.
2. Define a normalized value-associated width/decimal descriptor, including
   specified behavior for host-created values and exact-integer extensions.
   Audit parser, constructors, operators, copies, fields and call boundaries.
3. Implement one bounded foundation slice with before/after native-backed
   regression, focused and broader sanitizer/regression evidence and traceability.
   Then migrate the selected ListItem column consumer with both class/mode
   coverage, retaining earlier baselines and checked-conversion guarantees.
4. Keep other list/combo consumers and prior #6144 width-propagation/display
   gaps visible for subsequent slices; do not mark the family complete from
   this selected read or make unrelated caller changes without recovery.

VR-5611-NUMERIC-METADATA-001: native descriptor/propagation/display observations
and structural/repeat audit completed for this first grid; production parity
and broader acceptance remain open. DQ-DV-5611-NUMERIC-METADATA-001 documents
scope/continuation only: procedural delta is replacing the owner-choice pause
with prerequisite recovery, misuse severity LOW for this documentation-only
step. Walkthrough: the same-bits 2.5/5/2 case remains distinguishable in retained
metadata/output; no runtime behavior is changed or claimed verified. Rollback
is to revert the steering documentation/checkpoint, not discard raw evidence.
This is implementing-agent self-review, not independent-human approval.
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 stay open/unchanged;
the future cross-runtime implementation needs proportional impact and safety
verification. No Numeric/list family, issue, hazard, system or release closure.
