# Unary attribute propagation: development evidence and limits

RQ-CF-PRG-NUMERIC-UNARY-ATTRIBUTES-001 /
VR-5611-NUMERIC-UNARY-ATTRIBUTES-001 / DQ-DV-5611-NUMERIC-UNARY-ATTRIBUTES-001.
[Frozen pre-code contract](attributes-requirements.md) and parent unary recovery
remain the governing scope; this is not complete unary Numeric compatibility.

## Original-fail and single connection

Baseline995b1a88f37665fdc4e93d5ff7a96f8d76a36ed7 (recovery checkpoint68d9a2072
changes no production). New independent public watch expected table uses only
retained native nonzero plain-decimal cases, including nested widths41/42,
both modes and DECIMALS2/14. It never invokes the production helper to form an
expectation. Its frozen SHA256 is
eb23251574b7ae3d59b51c24f7f8e78e52e5755365415f4499e12f017e127d89.
Original `prg_engine_expression.inl` SHA256 is
fbca4eebf5a00eccb2e3fd468b98062d57ff206f230dafd4adb590e6b8a55310.

Original focused GNU build completed0. New public test actual1 with68 metadata
assertion failures; ALL kind/non-NULL, exact payload, plus-only metadata,
explicit formatting, arithmetic, invalid syntax/type, NULL, suppressed branch,
paused-state and resume controls passed. Full failure output and freeze hashes
are retained under `attributes-evidence/`, not merely a compile failure.

Only after that failure, include the unused pure helper and replace the ordinary
Numeric-minus return with checked attribute propagation, preserving the exact
existing `-value_as_number(operand)` payload operation. NULL/type/exact-integer/
Currency paths remain unchanged. Helper overflow becomes the existing localized
catchable Numeric-overflow39; the synthetic uint32 host boundary is a derived
safety rule, not an observed native descriptor limit or error contract.

SAME frozen public test fixed actual0/empty stderr, both modes/DECIMALS2/14;
post-wire freeze check0. Focused GNU CTest14/14 PASS69.18sec, including both
new tests, carrier/literal tests, Numeric behavior/equality, string/math,
exact64-bit expression, Currency literal/overflow, selection, regenerated
isolation and changelog contracts. GNU helper compile/run0 with Werror; Clang helper
ASan/UBSan/float-cast-overflow compile/run0 with no recovery/Werror. These are
local development results; named hosted instrumented public execution,
exact-head review, strict required checks and resolved conversations are still
merge gates and are not preclaimed here.

Configuration failures are retained honestly: first newly named build target
was requested before regeneration (exit2, no rule); explicit CMake regeneration
then built the original successfully. Pre-wire contracts2/3 because the
generated isolation inventory was stale after registration edits; the selector
and changelog tests passed. Regenerate inventory, do not weaken the contract.
Full local build logs remain in the task-owned `build/numeric-unary-native/`;
their hashes are retained with the smaller committed results.

## Scope, rollback and remaining acceptance

Value-owned attributes propagate; absent stays unknown; known scale is never
used to round the payload or size an allocation. Checked width can exceed40.
The width increment is supported by the sampled plain-decimal and variable
controls, NOT a universal contract for arbitrary VFP constant expressions.
The retained `-5/2` versus `-(5/2)` descriptor counterexample still requires
constant-expression/arithmetic recovery; those producers remain unmigrated.
No lexer, grammar, allocation/count/conversion policy, generic constructor,
assignment normalization, function/field/arithmetic metadata or display route
is migrated. Both sanitizer target build list AND actual CTest filter include
the new helper/public tests; the AST selection contract checks their exact names.

Signed-zero compatibility is explicitly unfinished: the retained native
context pair shows differences between constant-only expressions and evaluated
variable/function/conditional expressions. This increment deliberately retains
the prior payload operation, not a guessed context tag or universal zero rule.
The public nonzero table is NOT acceptance for signed zero, variable assignment
widths or full unary/Numeric/ListItem behavior. Those mandatory follow-ons and
#7189 stay open; no canonical-double residual-gap shortcut is adopted.

MEDIUM DQ/DV walkthrough/self-review: verify original68 -> fixed0 with unchanged
expectations; distinguish attribute propagation from payload semantics; check
overflow before increment and absence from knownzero; retain original failures
and named-execution gates. This is implementing-agent development review, not
an independent-human/system/release sign-off. Rollback removes the include/
single connection/helper/registrations and coherently rebuilds; preserve native
evidence and the owner objective. Existing hazard statuses and #6776 policy stay.
