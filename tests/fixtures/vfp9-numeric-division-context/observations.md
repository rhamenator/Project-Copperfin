# Typed division observations and required prerequisite

RQ-CF-PRG-NUMERIC-DIVISION-CONTEXT-001 / VR-DQ-DV-5611-NUMERIC-DIVISION-CONTEXT-001.
Initial recovery at origin/main a5c656f6e preceded production wiring. The
bounded Integer descriptor/literal child now has its own contract/completion;
there is still no division precision algorithm or arithmetic wiring.

Two648-observation division runs: actual0/empty stderr,648 values/no errors,
64 native Integer and584 Numeric records per run. Every documented typed field,
payload and display repeats exactly; no compiled PRG/EVALUATE differences.
Raw SHA256 first494eaf60381821c05d114347ef8d1143923f1c5a50284e8b83b979caa47ebc54,
repeat535652ea64f565b6326232a1f994810011d30d4c0fa10594be6aabed5125b422.
Raw bytes are NOT equal: the audit independently verifies that changes are
confined to unused native tagged-interface slots, not Numeric semantics.

The paired48-expression Integer prerequisite grid adds384 observations each:
actual0/empty stderr,384 values/no errors,272 Integer/112 Numeric. Documented
typed fields/bits/display repeat, compiled/EVALUATE differences0. Raw first
a0e1489da6154432aa4c7a7838b1c2fb79f0e75976e8f80eab744d94f5445f40,
repeatb5a76c50a5443c30d85ece5d5442384e6a400730e75f14f68252f0041b9515ea;
only one unused Integer real slot differs. That slot is NOT interpreted.
Both capture directories retain source/own FXP/raw/stdout/stderr/status/audit0.

## Typed controls and counterexamples

At FIXED OFF (both DECIMALS settings), native literal5 is Integer W1 and
5.00 is Numeric W4 D2, with identical binary64 magnitude. Assigned iFive=5
is Integer W10; assigned nFive=5.00 is Numeric W13 D2. INT(5.00) is Integer
W4, while EVALUATE("5") is Integer W1. VAL("5.00") is Numeric W7 D2 or
W19 D14. These are documented typed descriptors, not unused union fields.

All following have magnitude2.5, but FIXED OFF result attributes differ:

| Expression | DECIMALS2 W/D | DECIMALS14 W/D |
| --- | --- | --- |
|5/2|4/2|16/14|
|5.0/2.00|5/3|16/14|
|5.00/2.00|6/4|16/14|
|iFive/iTwo|24/4|36/16|
|iFive/nTwo or nFive/iTwo|24/4|36/16|
|nFive/nTwo|25/4|37/16|
|INT(5.00)/INT(2.00)|12/4|24/16|
|EVALUATE("5")/EVALUATE("2")|6/4|18/16|
|-(5/2)|2/0|2/0|
|-(5/2.00)|5/2|5/2|
|-5/2.00|5/2|17/14|
|-(iFive/iTwo)|25/4|37/16|
|-((5/2)+0.00)|6/2|6/2|
|-((5.00/2.00)*1.00)|11/6|11/6|

Thus equal bits, a single minimum-decimals setting, a universal sum-of-scales
rule or observing the operand in a separate evaluation does not establish
parity. Constant-only versus runtime behavior remains distinct. No general
width/scale formula has been accepted; arithmetic wiring remains deferred
until a type-aware normalized contract can account for these observations.

## Integer literal/unary prerequisite

The48-expression pair establishes direct unsigned plain whole controls0,1,5,
9,10,99,100, leading-zero forms and2147483646/2147483647 as native Integer,
width equal to source digit count. Positive2147483648/4294967295/4294967296
are Numeric with width10/D0. Native minus on known Integer controls adds1
width, retains Integer and has positive-zero BRS even for -(-0); plain Numeric
-0.0 instead has negative-zero bits. Plus/grouping preserve the Integer
descriptor. Native -2147483648 and INT(-2147483648.00) are Numeric in this
pair, NOT evidence that Integer-minimum promotion has been recovered generally.
Assigned-width normalization is still separate, not implemented by copying.

Architecture consequence: the present optional width/decimal Numeric carrier
cannot distinguish these native Integer and Numeric descriptors; its absence
is unknown, not evidence for native Integer identity. Copperfin's exact64 kinds
are extensions with different contracts and must not silently stand in for
native Integer. A normalized typed descriptor can preserve that distinction
without copying SDK layout or changing the double payload. Integer's unused
native decimal slot must not become a reported precision value.

Selected bounded child: typed descriptor plus unsigned plain whole
Integer literal metadata for at most10 ASCII digits/value<=2147483647, with
known Integer unary-sign descriptor propagation/positive zero. Longer forms,
larger whole Numeric literals, assignment normalization, INT/function/field
producers and arithmetic propagation remain mandatory follow-ons. This is an
evidence-based prerequisite, not cancellation of division work. The
normalized schema/contract and independent expected tests were frozen;
unproduced typed support/helper tests and original76 runtime failures preceded
the hooks. SAME public test now passes; see integer-completion.md. Merely
adding a storage tag is not arithmetic or full Numeric acceptance.

## DQ/DV and provenance

LOW recovery walkthrough: both structural/type-aware audits0;648 and384 cases
per run, all values/no errors, normalized repeat, raw differences independently
confined to unused slots, explicit constant/runtime/type counterexamples above.
Implementing-agent development self-review/automation, not independent-human/
system/release qualification. Existing runner/FLL/native hashes in README and
native-provenance.sha256; no VM/SDK implementation/decompilation inputs.
README retains delta/misuse/rollback/hazards and unfinished owner objective.
