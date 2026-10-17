# Bounded runtime Numeric division contract (pre-code)

RQ-CF-PRG-NUMERIC-RUNTIME-DIVISION-001 derives from the retained division
parent, direct owner Numeric policy and admitted #5611/#6776/#7189. Freeze
this contract and independent native expected rows before the arithmetic hook.

For an ordinary non-NULL Number/Number division whose own multiplicative
subexpression contains a runtime primary, preserve the existing binary64
quotient and attach native real Numeric metadata when both operand descriptors
are known and within this recovered domain: widths1..40; real scales0..18 with
width>scale; native Integer scale ignored and finite integral magnitude at most
2147483647; real payload magnitude<1e10; finite quotient magnitude<1e10; finite
nonzero divisor. SET DECIMALS must be canonical integral text0..18. Unknown,
malformed or outside-domain descriptors/payloads remain unknown, not inferred.

With relevant operand scales DL/DR (Integer0), D=min(18,max(S+2,DL,DR)) and
W=min(40,WL+WR-DL-DR+D-point), where point is1 iff either relevant scale>0.
Result storage is real even for an integral quotient. Metadata must not round,
replace or replay operands. Width is never an allocation size. This ordinary
legacy precision behavior applies in both NUMERICBEHAVIOR modes under #6776;
no out-of-range conversion quirk is enabled by default or newly migrated here.

Native evidence before hooks: paired1620 controlled positive observations,
paired12960 payload/sign/zero/Integer-boundary observations, and paired1200
ordinary public-route observations (60 expressions/settings/fixed/compiler).
The payload pair differs only in independently audited unused tagged-interface
slots; no raw-byte equality claim for that pair. Public pair is byte-identical.
Public expected header has60 rows, independently matched against1200 native
records without any production helper. Coverage includes all real scales1..18,
negative zero, a tiny decimal, magnitude9999999999, nested runtime division,
unary signs, IIF and EVALUATE primaries. No VM/installation/proprietary code.

Architecture: pure eligibility/metadata helper; reuse the existing transient
runtime-primary aggregation, but isolate each parse_multiplicative call so an
earlier additive sibling or enclosing function cannot contaminate a constant
division. Merge observed runtime primaries back to enclosing scopes, including
existing cached primary continuation records. Hook only the existing ordinary
Number division branch AFTER NULL/Currency/exact64/type/zero-divisor handling.
No persistent provenance, operand reevaluation, source-pattern matching,
constant folding, multiplication or addition metadata change.

Verification sequence: compile/run the unused helper under GNU warnings-as-errors
and Clang ASan/UBSan/float-cast-overflow/no recovery; freeze public tests/expected
header and original runtime hashes; build unchanged runtime with unproduced
support; require ORIGINAL descriptor failures with other controls passing;
wire bounded hook; run SAME frozen tests and relevant broader regressions.
Host named sanitizer execution, exact-head independent review, strict green
checks and resolved conversations remain merge gates, not preclaimed.

Excluded/mandatory follow-ons: constant-only division and folded subtrees,
unknown producers, unsupported magnitudes/scales and remaining display/exact
conversion/ListItem supplied-column READ. This is neither full Numeric parity
nor authorization for a canonical-double residual-gap shortcut or parent closure.

MEDIUM DQ-DV-5611-NUMERIC-RUNTIME-DIVISION-001 delta: unknown runtime division
descriptors -> native-backed bounded propagation -> isolate transient context ->
retain unchanged payload and safe conversion/allocation boundaries. Misuse:
leaked runtime context could silently change precision of constant expressions;
malformed widths could wrap arithmetic; replay could duplicate user callbacks.
Mitigations: local scoped aggregation, validated bounded integer arithmetic,
unknown fail-closed result, frozen independent native constants, explicit
unknown/NULL/type/zero/Currency/exact64/suppression/continuation controls and
broader zero-context regressions. Existing HZ-data-corruption-01,
HZ-runtime-crash-01/HZ-doc-command-01 and #6776 switches remain applicable.
Walkthrough and development self-review must record actual fail/pass chronology;
not independent-human/system/release qualification. Rollback removes the helper
include, scoped/hook increment and its registrations; existing unknown metadata
behavior is restored while native captures/owner objective remain retained.
