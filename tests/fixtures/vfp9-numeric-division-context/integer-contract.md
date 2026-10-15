# Pre-code bounded native Integer descriptor/literal contract

RQ-CF-PRG-NUMERIC-INTEGER-DESCRIPTOR-001 /
VR-DQ-DV-5611-NUMERIC-INTEGER-DESCRIPTOR-001 derives from owner Numeric policy,
the typed division recovery requirement and admitted #5611/#6776. This child
unblocks typed operand representation, not all division precision. The648/384
native pairs and normalized observations precede this contract and code.

## Normalized schema and staged behavior

Append a semantic PrgNumericStorage discriminator (real or integer) to the
existing value-owned PrgNumericMetadata. Default real preserves existing
two-member aggregate initialization. Storage is meaningful only for non-NULL
PrgValueKind::number, never a compiler-origin/provenance tag. Preserve the
existing double payload/number kind: native Integer is NOT Copperfin's exact64
extension. Native Integer's documented attributes are storage and width;
decimal_places is a normalized zero placeholder that consumers MUST ignore
for integer storage, not a copied/read native unused decimal slot. Existing
real descriptors keep width/decimal semantics. Absence stays unspecified.
No binary ABI guarantee is created; consumers must rebuild coherently.

Only unsigned plain ASCII whole literals of1..10 digits and value0..2147483647
produce known Integer attributes here. Width is their digit count, including
leading zeroes, normalized decimal placeholder0, storage integer. A pure
nonallocating helper validates that domain by checked unsigned accumulation,
without floating narrowing, parsing/rounding payload or accepting a suffix,
sign, exponent, decimal point or whitespace. Rejected metadata classification
does NOT reject otherwise valid PRG syntax: preserve existing parser behavior
and unknown metadata for unsupported producers. Longer whole forms and larger
whole Numeric literals are explicitly unmigrated, not asserted native-compatible.

Known Integer unary minus retains storage and increments width1 with the same
checked uint32 overflow39 as existing known real attributes. Its payload is
ordinary negation for nonzero operands and positive zero for either input
zero sign, independent of ephemeral expression context. Plus/grouping already
return the operand and preserve the typed descriptor. This zero rule is backed
by direct/nested/variable native Integer controls, NOT inferred from a double.
Staged producers yield only symmetric-range Integer payloads; malformed host-
injected metadata is outside native-parity claims and introduces no unchecked
conversion. Do not add INT/field/arithmetic producers or infer storage from an
unannotated double. Existing NULL/type/Currency/exact64 handling remains first.
Known real Numeric zero keeps PR7198's expression-context rule; unknown Numeric
zero, nonzero operations and source grammar remain unchanged.

These are ordinary representation semantics in both NUMERICBEHAVIOR modes,
not an out-of-range quirk or new switch. #6776 per-area policy and all checked
conversion/allocation limits remain unchanged. No tolerance or allocation
behavior is added, and display code is not changed or declared compatible.

## Architecture and verification before wiring

Public schema append; pure prg_numeric_integer_metadata.h classifier/zero helper;
existing unary metadata helper copies the typed descriptor before checked width
increment. Test the new typed carrier/helper support while no producer emits
it and no unary Integer-payload hook is active. An original-header compile
failure demonstrates the missing schema only, NOT runtime behavior failure.
After adding the unproduced carrier/helper support, rebuild original runtime
behavior with frozen native-backed public expectations and prove failures
before connecting parse_number()/ordinary Numeric-minus. This separates a
compile prerequisite from the required behavioral original-fail evidence.

Standalone GNU Werror and Clang ASan/UBSan/float-cast-overflow/no recovery must
cover domain edges/unsupported forms, checked width overflow, storage copy/
independence, positive Integer zero and unchanged real attributes/nonzero.
Independent public table uses retained native widths/storage/bits, both modes/
DECIMALS2/14, including -0/-(-0), bounds and leading zeroes. Kind/nonzero/NULL/
type/suppression/paused state/resume, previous real literal/unary/zero tests,
exact64/Currency and continuation/macro tests are controls. Freeze source and
contract before wiring, then require SAME test fixed0 and broader focused tests.
Both new tests must be selected in hosted sanitizer build AND actual filter;
require actual named executed receipt, strict green checks, Claude FIRST and
actual clean exact-head authorized review/resolved threads before merge.

## MEDIUM DQ/DV, hazards, rollback and re-entry

Delta: typed native recovery -> normalized carrier/helper -> frozen original
behavior failure -> two bounded parser/unary hooks -> same-test/broader/hosted
verification. Misuse: do not interpret unused native slots or normalized Integer
placeholder decimals as precision, confuse native Integer with exact64, infer
storage from equal bits, or claim copying establishes assigned-width parity.
Walkthrough verifies both native pairs/audits, staged schema-only original,
actual runtime original failures, fixed unchanged test and preserved controls.
This is development self-review/automated evidence, not independent-human,
system or release assurance. HZ-data-corruption-01/HZ-runtime-crash-01/
HZ-doc-command-01 apply. Rollback removes producer/payload hooks and typed
carrier/helper coherently, rebuilding consumers, while preserving native facts
and owner objective. Field notification through PR/changelog if shipped error.

After this child, re-enter the648 division counterexamples with typed operands
and complete normalized arithmetic precision rules/independent original-fail
tests before arithmetic wiring. Assignment normalization, longer/other literal
and function/field producers, display/exact conversion and retained ListItem
supplied-column READ remain mandatory. No #7189/parent/full Numeric closure,
canonical-double residual shortcut, VM start or backup-reset completion claim.
