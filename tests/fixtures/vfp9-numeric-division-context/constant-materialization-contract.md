# Bounded constant-division materialization contract (pre-hook)

RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-MATERIALIZATION-001 derives from the
intrinsic constant-division requirement, typed division parent, direct owner
Numeric policy and admitted #5611/#6776/#7189. Baseline mainca77da12 includes
completed runtime division PR7203. The intrinsic helper remains unused; this
checkpoint freezes further allowed native evidence, expected records and the
proposed bounded integration contract BEFORE any parser change/public result.

## Qualified boundary and side-effect evidence

`constant-boundary.prg`/cases cover150 complete expressions:10 division trees
under13 grouping/sign/function wrappers and20 runtime/constant mixtures. Five
DECIMALS settings0/2/6/14/18, FIXED OFF/ON and compiled/EVALUATE give3,000 rows
per invocation. Paired invocations actually return0/empty stderr/all values/no
errors; source/framing/typed/repeat audit0 and compiler/EVALUATE differences0.
Raw bytes repeat exactly, SHA256
402c38e12f268ac9392c046437616451610afac4fe94fd93b09153e367b7f75e.
These are COMPLETE-expression observations; separately evaluating a child is
not an oracle for the same child inside a folded expression.

At DECIMALS14, `-(5/2)` is realW2/D0, whereas `-(CFNCLONE(5/2))` is realW17/D14.
`CFNCLONE(-(5/2))` retainsW2/D0. IIF/EVALUATE confirm the same runtime boundary;
unary plus preserves the pending floor and repeated constant unary minus does
not reintroduce it. `(5/2)/(5/2)` has intrinsicD0, not two eagerly flooredD14
children. ABS's Integer-result width is a separate producer observation, not
authorization to migrate ABS in this bounded slice. Unknown producers stay gaps.

`constant-effects.prg` adds16 setting-mutating UDF controls. Each expression is
evaluated ONCE through CFNMETA, without additional BINTOC/TRANSFORM invocations
that would replay side effects. Paired320 rows/run actually return0/empty stderr,
all values/no errors; both routes match, every callback count is exactly1 and
the final DECIMALS setting matches the declared mutation. The observer parameter
descriptor is AFTER native LPARAMETERS normalization, not a raw call argument.
No default-display or independent BRS claim is made for this side-effect grid.
Raw bytes differ10 rows ONLY in independently audited unused Integer parameter
real slots. First SHA256dc123d468fd64d1e95c995b2351c3529dbd4533a766a7e7658416f9ea9521e42;
repeat d263d7a740ecc00886182d18a82643e0eb01d3b6377e2372351cb56f78ebb8b7.

Timing discriminator: startDECIMALS14, `(5/2)/CFMark(0)` finishesDECIMALS0 but
has realW17/D14; `CFMark(0)/(5/2)` has realW4/D2. CFMark returns literal2.00.
Re-reading DECIMALS after the right callback would destroy the first operand's
already established precision. At initial0 and mutation14, the corresponding
results differW18 versusW19 (bothD16). Observer UDFs separately confirm that the
argument scale is established before the callee changes the setting.

Additional `constant-continuation.prg` controls test ungrouped constant prefixes
`5/2/CFMark(0/14)` and `(10/2)/2/CFMark(0/14)`: paired80 values/run, actual0,
empty stderr/no errors, exactly1 callback and source/typed/compiler-route audit0.
Raw bytes repeat exactly, SHA256
5779e236c74ee68cd8cfa926bebaf356959365c387a75b8cfc6ff57e9d15975b.
At initial14 and mutation0 the first result remainsW17/D14 and chained result
W18/D14. A primary-only precision checkpoint is therefore insufficient when
the constant division has no enclosing group primary: preserve the prepared
descriptor of a completed division node across continuation too. This is a
required parser architecture constraint, not evidence that it is implemented.

The first effects probe was NOT qualified: wrapper status0 caught320 error11
records in each run. Its output formatter incorrectly passed Numeric
SET('DECIMALS') to ALLTRIM. A separate native VARTYPE query returnsN; changing
only that formatter to explicit STR(...,4,0) gives the qualified pairs above.
Original source/ownFXP/raw/status and the fixed auditor's rejection1 remain in
constant-effects-captures/format-first. No numeric expression, expected value,
callback body, assertion, timeout or product code was changed to fit a result.

## Bounded integration behavior

Admit only known, literal-derived constant division nodes whose operands fit
the frozen whole-payload intrinsic helper domain. Existing producers include
unsigned plain Integer literals of at most10 digits/value<=2147483647, bounded
plain decimal Numeric literals, grouping/signs, and eligible whole-valued
constant-division results. Non-whole operands, divisors below1, unknown/wider/
nonfinite metadata, other operators/producers, Currency and exact64 remain
outside this increment; keep their existing safe unknown behavior and mandatory
follow-ons. No source-number special case or type inference from equal bits.

For an eligible real constant-division node, preserve BOTH intrinsic metadata
and a prepared materialized descriptor in TRANSIENT parser state. At node
evaluation time, let D'=max(intrinsicD,current validated DECIMALS), width increase
by D'-D plus a decimal-point position when D changes0->positive. Payload bits
are unchanged by materialization; intrinsic constant-zero normalization remains
the already recovered separate operation. Integer has no decimal floor.

Required semantic boundaries:

1. Preserve intrinsic metadata through grouping and subsequent constant folding.
   Unary plus preserves the prepared descriptor. Constant unary minus consumes
   intrinsic metadata, adds its existing width position and clears pending
   materialization; function-return negation instead sees the already materialized
   value. Existing typed zero rules still apply, including repeated constant -0.
2. Materialize at the outer expression result, runtime function-argument boundary,
   and when a constant operand joins a runtime division. Use the descriptor
   prepared BEFORE a later callback, never a fresh setting read after it.
3. Cache/restore that prepared descriptor alongside the existing primary value
   and runtime-primary flag AND at completed constant-division nodes which have
   no enclosing primary checkpoint. Never reevaluate an operand or
   duplicate callbacks to reconstruct precision. Pending state is parser/IR
   evaluation state, NOT a persistent PrgValue origin field.
4. Unsupported operators/results must not inherit a right operand's pending
   descriptor. Require coherent known Numeric metadata before applying an
   alternate; preserve unknown/type/NULL/error/suppression behavior.
5. After constant operands materialize, reuse the completed bounded runtime
   division contract for runtime mixtures. Keep its current setting timing,
   quotient arithmetic, checked conversions and allocation limits unchanged.

Proposed architecture: pure metadata materializer; an optional prepared descriptor
in ExpressionParser/ExpressionPrimaryCheckpoint plus completed-node precision
checkpoints owned by the existing expression continuation; explicit outer/argument versus
group expression boundaries; scoped consumption at unary and division operators.
No persistent provenance, source-text pattern matching, new constructor-wide
inference, display rewrite or unrelated arithmetic migration. Both NUMERICBEHAVIOR
modes retain ordinary legacy precision; FIXED remains separate display policy.

## Frozen public expectation frontier and remaining gates

`emit-constant-public-header.cjs --check` reconstructs846 complete native rows
at five settings directly from retained captures, with exact descriptor/bit
checks across FIXED/compiler routes. It imports no Copperfin helper or precision
equation. Of those,798 are in the declared current producer frontier. Forty-eight
rows still require large-whole Numeric or >10-digit Integer literal producers;
all48 native expectations stay in the header/captures, explicitly flagged, NOT
erased or accepted as parity. Non-whole nested division and ABS/FLL-only wrappers
remain additional retained native controls outside the selected public frontier.

The first generator check failed1 because a manually estimated public count806
was wrong: the unchanged source-producer predicate selects592 payload rows plus
147 initial-grid and59 boundary rows =798. The independent token-scope audit
lists all48 exclusions; the predicate and every native expected field remained
unchanged. This inventory-count diagnostic is retained before header freeze;
it is not a failing runtime expectation revised to fit implementation.

Before any hook: freeze public C++ tests against these exact native constants,
original runtime hashes and unsupported/side-effect/continuation controls;
require actual ORIGINAL failures and classify them without changing constants.
Then wire the bounded behavior and require SAME tests passing, relevant prior
Numeric/control-flow/Currency/exact64/macro/isolation regressions, actual named
hosted sanitizer execution, Claude-FIRST then verified authorized Codex fallback
if needed, exact-head clean review, strict green checks and resolved conversations.
The old runtime test's explicit unknown assertions for newly admitted constant
routes must be replaced by stronger native expectations before the baseline,
not silently weakened after implementation. Other unknown controls stay intact.

## DQ/DV, hazards and rollback

MEDIUM DQ-DV-5611-NUMERIC-CONSTANT-MATERIALIZATION-001 delta: intrinsic-only
support -> complete nested/function boundary observations -> single-evaluation
setting mutation controls -> prepared descriptor timing contract -> frozen
native public frontier -> pending original-fail/runtime integration. Misuse:
global unary suppression, eager per-child floors, stale/polluted pending state
or delayed setting reads could silently change metadata; replay duplicates UDF
effects. Mitigations: explicit semantic boundaries, prepared immutable metadata,
checkpoint restoration/coherence, literal/domain fail-closed guards, independent
native constants and mandatory side-effect/cache/unsupported-route tests.
Existing HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01 apply;
mode policy, conversion and allocation limits are unchanged. Walkthrough so far
is actual paired native/source/type/repeat/callback/header checks0, with initial
format/count failures retained. No public/helper/hook or hosted result is
preclaimed. Development self-review/automation is not independent-human/system/
release qualification. Rollback removes the future bounded include/context/hook/
registrations, restoring unknown precision while retaining native evidence and
the owner's uncancelled objective. No runtime field notification before a change.

Native executable/runner/FLL are unchanged; private diagnostic is host-only and
no SDK implementation is redistributed. No VM/reboot/backup/upload work. Remaining
producer/constant domains, display/exact conversion and ListItem supplied-column
READ stay mandatory; no parent closure or canonical-double residual shortcut.
