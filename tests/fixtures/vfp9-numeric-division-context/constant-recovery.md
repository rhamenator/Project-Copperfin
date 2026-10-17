# Constant-only division recovery checkpoint

2026-10-17; RQ-CF-PRG-NUMERIC-DIVISION-CONTEXT-001;
VR-DQ-DV-5611-NUMERIC-CONSTANT-DIVISION-001. Direct owner Numeric policy and
still-open owner-authored `agent-approved` #5611/#6776/#7189 govern this
bounded prerequisite. Baseline main ca77da12bb74cfd3aa31d7b081ce2a9f29424883
includes merged PR7203's bounded runtime division. That slice is complete;
it is not restarted or extended indiscriminately to constants.

## Inventory and independent audit

`constant-grid-cases.json` explicitly records 183 expression/operand controls.
The first 147 cover seven spellings each of positive 5 and 2 (whole,
leading-zero whole, and decimal scales1/2/6/14/18), with direct division,
leading minus on the numerator, and grouped minus on the entire division.
Thirty-six further controls cover integral/fractional quotients, zero and
negative zero, nonterminating division, magnitude9999999999, tiny decimals,
denominator/both signs, grouping/unary plus, nested division, addition and
multiplication, ABS/IIF/EVALUATE, runtime clone boundaries and assigned values.
These additional operations are controls, not newly recovered implementation
contracts for all of those producers.

Five DECIMALS settings0/2/6/14/18, FIXED OFF/ON, compiled PRG/EVALUATE give
3,660 observations per invocation. Two sequential invocations actually return0,
empty stderr, all3,660 values and no caught errors. The standalone auditor
checks the entire source inventory, exact corresponding compiled expressions,
actual literal operand types/widths/scales/payloads, complete unique framing,
result descriptor versus BRS bits, and display bytes. It imports no Copperfin
code or inferred precision helper. Documented typed fields/bits/display repeat
exactly; compiled-versus-EVALUATE differences0.

Raw captures are NOT byte-identical:1,162 rows differ. Every difference is
independently confined to unused Integer real slots (right operand1,162,
result54); Integer decimal/real slots are not interpreted. First raw SHA256
53374e373acc9caf3eb2276edfcc97512acc6976f8dd538d6bda67d33a17a1c3;
repeat d27e5d3a194a2905c0c0b62fe4107a0c1d3cf0f87b6a7b491821490924e36543.
The retained `.gz` files losslessly preserve both runner stdout and result
files; the audit decompresses them and proves equality within each invocation.
Host uncompressed originals and initial pre-capture source/native hashes remain
in the slice build directory. Native source and first-party generated FXP are
retained; no assertion, product code, runtime configuration or timeout changed.

Reproduce the retained audit from the repository root:

    node tests/fixtures/vfp9-numeric-division-context/audit-constant-grid.cjs \
      tests/fixtures/vfp9-numeric-division-context/constant-captures

## Constraints on the next normalized contract

At FIXED OFF, all four first expressions below have quotient magnitude2.5:

| Expression | DECIMALS2 storage/W/D | DECIMALS14 storage/W/D |
| --- | --- | --- |
| `5/2` | Numeric4/2 | Numeric16/14 |
| `05/2` | Numeric5/2 | Numeric17/14 |
| `-5/2` | Numeric5/2 | Numeric17/14 |
| `-(5/2)` | Numeric2/0 | Numeric2/0 |
| `5.00/2.00` | Numeric6/4 | Numeric16/14 |
| `-(5.00/2.00)` | Numeric7/4 | Numeric7/4 |
| `10/2` | Integer2 (no relevant D) | Integer2 (no relevant D) |

At DECIMALS0, `5/2` remains binary64 2.5 but has NumericW1/D0 and default
display `3`. A result's stored bits, precision metadata, and displayed text
therefore must remain separate. `0/2`, `10/2`, `100/2`, and `5/(2/2)` have
native Integer results in this inventory; nonintegral `5/2` is real Numeric.
Integer identity cannot be substituted from Copperfin exact64 extension kinds.

Leading zeros survive in result width on the numerator (`05/2`) but not in the
same way on the denominator (`5/02`). Scales saturate: the direct/leading-minus/
group-minus `5.00000000000000/2.00000000000000` variants report D18, not D28.
The post-capture naive hypothesis based only on quotient-magnitude width and
uncapped sum of operand scales has1,140 mismatches among the2,940 first-grid
observations. Its failed predictions are counted by the auditor, not silently
substituted for the native results or an accepted requirement.

Grouped minus bypasses the DECIMALS floor on these folded expressions, while
leading minus does not. Nested arithmetic and runtime controls retain further
distinctions. Observing a child division separately cannot establish the
descriptor of the same syntax within a larger folded expression. No universal
constant-folding, width, signed-zero, or integral-result promotion algorithm
is accepted by this checkpoint. Next recover payload/sign/width controls and
normalize a bounded compositional constant-expression contract before freezing
independent ORIGINAL-fail tests and adding any product hook. Do not implement
source-text special cases for these particular numbers or guess a tolerance.

## Provenance, DQ/DV, misuse and rollback

The unchanged installed VFP9 Wine runner and native executable still match the
prior provenance hashes. The first-party published-interface diagnostic FLL is
preserved OUTSIDE Trash in canonical host-only
`build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll`; the PRG
records its exact absolute path. No SDK/header/library/FLL implementation is
redistributed or linked into Copperfin. No VM, installation, backup process,
or reboot is involved. Pre-capture hashes are retained in `constant-captures`.

LOW DQ-DV-5611-NUMERIC-CONSTANT-DIVISION-001 delta: completed runtime increment
-> frozen explicit constant-context inventory -> paired native observations
-> complete independent typed/unused-slot audit -> retained counterexamples
-> pending normalized contract. Misuse: treating result bits as precision,
dropping source width, extending runtime formulas to constants, or inferring
Integer identity could silently change display and later selector semantics.
Mitigations are explicit counterexamples, separate documented native fields,
unknown fail-closed boundaries, and mandatory future original-fail verification.
Existing HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 apply;
#6776 per-area switches, checked conversions and allocation limits are unchanged.
Walkthrough: both actual native statuses0 and retained audit0 verify complete
coverage, input controls, typed repeat, exact compiler/EVAL equivalence and
the rejected candidate's1,140 mismatches. Implementing-agent development
self-review and automated cross-check are not independent-human, product,
system or release qualification. Rollback removes only this native recovery
increment; merged runtime behavior is unchanged and the owner objective remains.
No runtime field notification is required because no runtime changed.

Remaining constant/folded arithmetic, unsupported domains, producer propagation,
display, exact conversion and ListItem supplied-column READ remain mandatory.
No full Numeric parity, canonical-double shortcut, parent closure or ListItem
acceptance follows merely from adding metadata or retaining this fixture.
