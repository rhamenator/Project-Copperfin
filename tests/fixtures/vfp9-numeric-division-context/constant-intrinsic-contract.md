# Intrinsic constant-division precision contract (pre-code)

RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-INTRINSIC-001 derives from the retained
constant-context requirement and the owner's Numeric policy under admitted
#5611/#6776/#7189. This is an unproduced support phase, NOT parser integration,
default display or ListItem completion. Freeze this contract and the independent
640-row native expected header before adding the unused helper. Runtime hooks
still require normalized materialization/context recovery and independently
frozen ORIGINAL-fail public tests. No acceptance follows from helper fields alone.

## Recovered bounded domain and intrinsic result

The future caller must establish a pure constant literal-derived Number/Number
division, not infer origin from equal bits, metadata width or storage. Inputs
are non-NULL Number, with known valid native typed metadata; finite whole-valued
payloads of magnitude<1e10; divisor magnitude>=1; native Integer payload magnitude
<=2147483647. Real scales0..18, source whole-width1..12, full width1..31; Integer
scale is unused and ignored. These are bounds of this whole-payload literal
foundation, not VFP's general Numeric limits. Other payloads/producers, small
divisors, malformed/unknown metadata and exact64/Currency remain unsupported.

Let q be the existing finite binary64 quotient, abs(q)<1e10. Intrinsic constant
zero is positive zero regardless of input zero signs. Nonzero quotient bits
are not rounded or replaced. Storage is native Integer iff both input storage
tags are Integer and q is integral with abs(q)<=2147483647; otherwise real.
No native type is inferred from q alone. Both NUMERICBEHAVIOR modes retain this
ordinary legacy precision contract; no selector conversion quirk is migrated.

DL/DR are relevant input scales (Integer0). D=min(18,DL+DR) for real results,
otherwise Integer's normalized placeholder0. Lwhole=WL-DL-(DL>0 ?1:0).
Qwhole counts digits of abs(trunc(q)) WITHOUT a leading-zero position when
abs(q)<1, plus one sign position iff q<0. W=max(Lwhole,Qwhole)+D+(D>0 ?1:0).
Source leading-zero/sign width is therefore retained on the left. W remains
bounded by31 in this domain. Width is never an allocation count.

Subsequent unary minus is a separate operation: existing known metadata adds
one width position, Integer zero stays positive and real constant zero becomes
negative zero. The native expected header contains both direct and grouped-minus
results to verify that composition without deriving expected fields from a helper.

## Materialization and integration NOT established here

The native root-direct observations apply a DECIMALS floor to real precision;
grouped minus preserves intrinsic scale instead. That observation alone does
not establish a general eager per-division floor, function-argument boundary,
nested folding or continuation algorithm. The helper must NOT apply SET DECIMALS,
read global state, change existing Number construction, attach persistent
provenance, infer source syntax or re-evaluate operands. No parser hook in this
phase. Recover semantic boundaries before choosing transient parser architecture;
do not extend the completed runtime formula to constant expressions.

## Evidence, verification and hazards

Two new paired6,400-observation batches cover10 payload pairs, eight scale/
padding forms, four input signs, direct/grouped-minus, five DECIMALS settings,
FIXED OFF/ON and compiled/EVALUATE. Both statuses0, empty stderr, complete typed
repeat/compiler equivalence; raw differences only independently audited unused
Integer real slots. First-grid2,940 observations add all retained5/2 spellings.
The independent post-capture model matches15,740 records. Its first sign-width
candidate has560 counterexamples (negative fractions do not require a leading
zero width position); the rejected magnitude/uncapped-scale candidate's1,140
earlier counterexamples remain retained. Neither failed candidate was a product
expectation or an accepted algorithm.

`audit-constant-intrinsic.cjs --check-header` independently reconstructs the640
literal expected C++ rows from actual native DECIMALS0 records, not the model
or production helper. The unused helper must pass those unchanged constants,
unsupported-domain/bounds/nonfinite/type/zero-divisor controls, GNU Werror and
Clang ASan/UBSan/float-cast-overflow/no-recovery before any integration work.
Retain actual failures and fixes; do not change expected native constants to fit.

MEDIUM DQ-DV-5611-NUMERIC-CONSTANT-DIVISION-INTRINSIC-001 delta: typed native
observations -> bounded intrinsic descriptor model -> unused support -> future
separate materialization/context recovery and ORIGINAL-fail hooks. Misuse:
eager flooring or inferring Integer from a double could silently alter arithmetic
metadata, display and later selectors; malformed widths could wrap arithmetic;
replay could duplicate callbacks. Mitigations: explicit eligibility bounds,
checked bounded unsigned arithmetic, pure optional result with no allocations,
separate intrinsic/materialized phases, frozen native constants and mandatory
future continuation/no-replay tests. HZ-data-corruption-01/HZ-runtime-crash-01/
HZ-doc-command-01 apply; #6776 switches, conversions and allocation limits stay
unchanged. Current walkthrough is native/source/typed/repeat/model/header audit0;
helper/public/integration outcomes must be recorded when actually run. Development
self-review/automation is not independent-human, system or release qualification.
Rollback of this support phase removes only unused helper/tests; existing runtime
behavior remains intact. No runtime field notification before any runtime change.

All non-whole payloads, smaller divisors, wider source domains, other constant
operators/producers, materialization, display/exact conversion and retained
ListItem supplied-column READ remain mandatory follow-ons. No residual-gap
shortcut, universal Numeric formula or parent closure is authorized.
