# Unary Numeric expression-context recovery

RQ-CF-PRG-NUMERIC-UNARY-EXPRESSION-CONTEXT-001 / VR-DQ-DV-5611-UNARY-EXPRESSION-CONTEXT-001
derives from owner Numeric compatibility, parent unary metadata requirement
and admitted #5611/#6776. Baseline origin/main54a96438facc6e390e8649a6aefd9bf67fa5f62d
contains PR7197's bounded attribute propagation, not complete unary semantics.

Selected next slice: recover constant-only versus evaluated-operand boundaries
for payload zero sign AND division/parenthesized metadata, comparing compiled
PRG and EVALUATE under DECIMALS2/14 and FIXED OFF/ON.48 expressions include
the prior24 zero controls and24 division/parentheses/function/variable controls.
Do not infer a persistent value provenance flag, compiler implementation or
universal width/scale rule from a few samples. No product code change yet.

Use the existing unchanged host VFP9 private Wine runner and host-only first-
party published-interface FLL; see prior unary fixture provenance. No VM starts,
SDK source/library copying or proprietary implementation inspection. Pair
actual statuses/stderr/stdout/raw/source/own FXP and type-aware audit. Prior
auditor `../vfp9-numeric-unary-metadata/audit-context.cjs` takes the matching
source argument; it checks exact static-case/source correspondence and all
384 expected observation keys, not a guessed semantic formula.

LOW recovery-only DQ/DV delta: completed attribute increment -> recover this
remaining boundary -> architecture and independent original-fail tests before
payload/context wiring. Walkthrough must separate observed payload/descriptor/
display and compilation modes from inferred mechanisms. Failed attempts are
retained/unqualified. Developer self-review is not independent-human/system/
release evidence. Rollback removes this recovery fixture while preserving the
owner objective and prior implementation. Existing HZ-data-corruption-01,
HZ-runtime-crash-01, HZ-doc-command-01, #6776 switches and checked conversions/
allocation limits are unchanged. Assignment/display/full Numeric/ListItem and
#7189 remain unfinished, not a canonical-double residual-gap exception.
