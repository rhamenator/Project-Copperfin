# Bounded unary attribute propagation: pre-code contract

RQ-CF-PRG-NUMERIC-UNARY-ATTRIBUTES-001 derives from the owner Numeric decision,
RQ-CF-PRG-NUMERIC-UNARY-METADATA-001 and admitted #5611/#6776. This is the next
representation migration increment, NOT complete unary Numeric compatibility.

Repeated160/192/240 native grids recover a width increment of exactly1 for
Numeric unary minus, preserving decimal places. Parentheses and unary plus
do not themselves add width. Direct widths41/42 prove that the literal width
cap40 does not apply to unary propagation. Assigned-variable descriptors have
different starting widths; this increment does not normalize assignment.

The additional `context-captures` pair contains192 observations per run:
24 expressions x DECIMALS2/14 x FIXED OFF/ON x EVALUATE/compiled PRG. Both
actual0/empty stderr,192 values/no errors, documented and byte repeat, SHA256
5b0cd6d52c370f5bb304a80cea399d6cc04bc54e50a13a23e946d89b046051ab.
`audit-context.cjs` checks source/static-case identity, framing, expected keys,
documented Numeric fields, bits and repeats. Compiled and EVALUATE have no
documented-field differences in these controls. Runtime functions/conditionals/
variables preserve zero sign under unary minus; constant-only arithmetic can
produce negative zero. This supports a context distinction, not a recovered
general constant-folding implementation. The raw unused integer slot is not
interpreted. No source/decompiled implementation was inspected.

Architecture for THIS bounded increment: a pure nonallocating helper consumes
optional value-owned attributes; absent stays absent, known(width,decimals)
becomes(width+1,decimals), with an explicit checked-overflow result before the
uint32 increment. No scale/payload rounding, width40 clamp or bit-keyed registry.
Connect only ordinary Numeric minus after existing NULL/type/exact-integer/
Currency handling in `parse_unary()`. Preserve its preexisting double negation
exactly, including its known signed-zero gaps; do not claim those outcomes are
native-compatible. Synthetic host descriptor UINT32_MAX is rejected with the
existing catchable Numeric-overflow39 rather than wrapping; this is derived
representation safety, NOT an observed native width-limit/error contract.
Unknown attributes remain unknown. No switch/allocation/conversion policy,
grammar, assignment, arithmetic/function/field/display or list consumer change.

VR-5611-NUMERIC-UNARY-ATTRIBUTES-001: freeze independent public watch cases
from native nonzero plain-decimal unary observations BEFORE wiring. Confirm
original metadata failures with kind/payload/format/state controls passing,
then SAME test fixed. Include nested widths41/42 and both modes/DECIMALS2/14.
Pure helper tests separately cover known/unknown/zero/max decimal/max width
and checked overflow under GNU plus Clang ASan/UBSan/float-cast-overflow.
These helper tests do not establish public assignment or payload-zero parity.
Focused existing literal, Numeric, equality, string/math, exact-integer and
Currency tests and actual named hosted sanitizer execution remain gates.

Mandatory next boundary: normalized expression context sufficient to match
native signed-zero behavior without tagging persistent values with guessed
provenance. Then assignment normalization, remaining producers/propagation,
display and exact ListItem conversion recovery/wiring. None is cancelled or
declared a residual exception. #7189 and parent/unary compatibility remain open.

DQ-DV-5611-NUMERIC-UNARY-ATTRIBUTES-001: procedural delta is repeated native
recovery -> unused helper -> independent original-fail -> one propagation
connection -> verification. Misuse severity MEDIUM: missing/incorrect precision
can affect downstream selection; widths are not allocation sizes. Walkthrough
must distinguish native nonzero acceptance from the retained zero-payload gap,
and checked host-overflow policy from native evidence. Developer self-review,
native/audit and tests are development evidence, not independent-human/system/
release qualification. Rollback removes hookup/helper/test registration and
coherently rebuilds while preserving evidence and the owner objective.
HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 unchanged.
