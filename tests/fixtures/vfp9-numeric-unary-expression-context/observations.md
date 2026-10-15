# Native expression-context observations: no universal formula inferred

RQ-CF-PRG-NUMERIC-UNARY-EXPRESSION-CONTEXT-001 /
VR-DQ-DV-5611-UNARY-EXPRESSION-CONTEXT-001; parent and scope in README.
The original384-observation checkpoint below preceded production changes.
The bounded zero child now has its own contract and completion record; it
does not claim to recover or implement the division precision shown here.

Two native384-observation runs: actual0, empty stderr,384 values/no errors,
complete framing and keys; documented fields and raw bytes repeat exactly.
Raw SHA256 cd58aacfa769ff959fb02855aaff1278abb5fffb848930a07725e176fafc64c4.
No documented-field compiled PRG versus EVALUATE differences in the48-expression
grid. Each expression is crossed with DECIMALS2/14 and FIXED OFF/ON in both
execution forms. `nSource=5.00`, `nDivisor=2.00`, `nZero=0.00`,
`nMinusZero=-0.00`; ZeroValue() returns0.00. Assigned descriptor normalization
is native behavior, not a claim that Copperfin already implements it.

## Nonzero division context

At FIXED OFF, the following all have magnitude2.5 and the expected sign bits.
The table records direct descriptors, NOT a universal precision algorithm.

| Expression | DECIMALS2 width/decimals | DECIMALS14 width/decimals |
| --- | --- | --- |
| `5/2`, `(5/2)`, `+(5/2)` |4/2|16/14|
| `-(5/2)`, `-((5/2))`, `-(+(5/2))` |2/0|2/0|
| `-5/2` |5/2|17/14|
| `-(-5/2)` |3/0|3/0|
| `5.0/2`, `5/2.0` |4/2|16/14|
| `-(5.0/2)`, `-(5/2.0)` |4/1|4/1|
| `5.00/2.00` |6/4|16/14|
| `-(5.00/2.00)` |7/4|7/4|
| `nSource/2`, `5/nDivisor` |15/4|27/16|
| `-(nSource/2)`, `-(5/nDivisor)` |16/4|28/16|
| `-IIF(.T.,5/2,1.00)`, `-EVALUATE("5/2")`, `-ABS(5/2)` |5/2|17/14|

For example, native default TRANSFORM of `-(5/2)` is `-3`, whereas
`-EVALUATE("5/2")` is `-2.50` or `-2.50000000000000` at FIXED OFF. Associated
decimals do not round the stored double: bits remain C004000000000000 for
both. Under FIXED ON, display is captured separately and can depend on current
settings even when direct descriptor fields are unchanged.

These observations rule out deriving all division metadata from the double,
one global SET DECIMALS minimum or simply the descriptor returned by a separate
evaluation of the operand source. A constant-expression optimization/evaluation
distinction is an inference from behavior, not knowledge of VFP implementation.
Native Integer versus Numeric operand descriptors and evaluation context must
remain distinct during recovery. The previous unsigned literal and unary
attribute increments do not claim to implement this boundary.

## Zero and function controls

The prior24 controls retain the signed-zero distinction: literal/constant-only
forms can yield8000000000000000; unary minus of positive-zero variables,
function returns, ABS/VAL/IIF/EVALUATE results or mixed runtime arithmetic
retains0000000000000000 in this grid. A negative-zero variable or EVALUATE
negative-zero operand retains8000000000000000 under further negation. All
payload/descriptor/display fields remain separately available in the captures.
Do not replace these observations with either universal IEEE sign toggling,
universal sign preservation, or an inferred constantness bit attached to every
persistent Numeric value. The grid does not establish all constant-folding,
underflow, macro/member or callback/resume semantics.

## Architecture constraints and next action

Retain public value-associated descriptor state, but distinguish it from any
future ephemeral expression-evaluation context. Context must not be inferred
from equal payload bits or width/decimals alone. Before using a parser-side
classification, establish which literal/operator/runtime-primary combinations
qualify; continuation/checkpoint cache replay and suppressed evaluation must
preserve classifications without re-evaluating callbacks or changing effects.
Before the child connection, ExpressionParser primary checkpoints preserved
values/end positions but no classification. The child now retains ephemeral
runtime-primary classification alongside those cached values, not in PrgValue.

Retained next recovery: complete native type-aware precision rules needed at
this boundary (Integer versus Numeric division operands, nested arithmetic and
SET DECIMALS changes), and freeze independent expected cases for a narrowly
defined context increment. Then select a normalized architecture and establish
original-fail evidence before production wiring. No formula or parser rewrite
is accepted merely because it fits this grid. See zero-contract.md for the
intervening bounded payload increment and its original-fail evidence.
Assignment normalization, other
producers/display/conversion and retained ListItem read remain mandatory work.

## Provenance and DQ/DV walkthrough

Existing unchanged runner SHA256
1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06;
installed VFP9 executable SHA256
cdc159a1e07e5208a0def267a94b8b3b007160641b442cb453aba8ecc4138560;
host-only published-interface first-party cause-probe.fll SHA256
ebb34d4e4f12c378e7ffd521d66915a249b45995535e6d37513deb711d5264a3.
No SDK binary/source or proprietary implementation was copied. Own FXP only.
No VM was started or recreated; backup-reset completion remains unconfirmed.

Walkthrough/self-review: paired audits0,384 expected values each, no errors,
raw repeat and no compiled/EVALUATE differences; the explicit nonzero table
and separate zero bits demonstrate why no universal formula is asserted.
This checkpoint is implementing-agent LOW recovery-documentation verification, not
independent-human/system/release assurance or acceptance of a production fix.
Delta/misuse/rollback/hazard and owner-continuity limits are in README.
