# Bounded native Integer descriptor/literal development result

RQ-CF-PRG-NUMERIC-INTEGER-DESCRIPTOR-001 /
VR-DQ-DV-5611-NUMERIC-INTEGER-DESCRIPTOR-001. Governing frozen pre-code
contract is integer-contract.md; native648/384 pairs, type-aware audits and
counterexamples are in observations.md. Baseline origin/main a5c656f6efc96c5df79cddc0f824b68520db3438;
signed recovery checkpointf79f5be5d04757e5742bd4f0269663ce00f143a4 preceded code.

## Requirement to implementation and verification

Native Integer5 W1 and Numeric5.00 W4D2 share payload bits but have different
documented attributes. Native -0/-(-0) are Integer/positive zero, unlike Numeric
-0.0/negative zero. The typed discriminator is therefore semantic value state,
not inferred compiler provenance, unused SDK fields or a double-keyed registry.

PrgNumericMetadata appends default-real PrgNumericStorage for source-compatible
old two-field aggregate initialization. Integer has width/storage; its decimal
placeholder0 is ignored, NOT copied from a native unused slot. PrgValue remains
ordinary number/double, not an exact64 extension, and metadata absence remains
unspecified. Coherent rebuild required, no binary ABI promise. New pure
prg_numeric_integer_metadata.h classifies only unsigned1..10 ASCII whole digits
with value<=2147483647 by checked unsigned accumulation, no payload parse/cast/
rounding/allocation. Unsupported metadata classification does not reject PRG.
Known Integer unary metadata copies storage and checks width+1; payload unary
minus normalizes zero positive, nonzero negation unchanged. parse_number() adds
one fallback classifier AFTER existing successful payload/plain-decimal parsing;
the ordinary Number-minus route selects known Integer by descriptor, otherwise
preserving PR7198's real/unknown context behavior. Existing NULL/type/exact64/
Currency ordering, source grammar, switches, checked conversions and limits stay.

The new helper test first failed compilation against ORIGINAL public schema
(missing type/field), saved status1. This is schema evidence only, NOT runtime
failure. Then the carrier/helper functionality was added but remained UNPRODUCED:
no parser producer or Integer-payload hook. Standalone GNU Werror and Clang
Werror ASan/UBSan/float-cast-overflow/no recovery compile/run0. Previous carrier,
literal/unary/context pure tests also GNU Werror compile/run0 before wiring,
showing default-real semantics remained intact. Exact commands below.

The frozen public18-row native table independently matches Integer type/width/
bits at both native DECIMALS settings; correspondence audit0 never calls the
production classifier. Original runtime behavior was rebuilt with only the
unproduced schema/helper support: build0/test1,72 descriptor mismatches and4
Integer -0 payload-bit mismatches,76 total. Kind/nonzero/real-zero/unsupported-
producer/NULL/type/suppression/arithmetic-payload/paused-state/resume controls
passed. The parser CPP/inl hashes remained ORIGINAL until that run completed.
After the two hooks, SAME frozen public test0/empty stderr; contract/helper/
public-source hashes still match. No expected value or assertion changed.

GNU Debug -O0 -g0 focused build0/CTest21/21 PASS158.81sec, including new helper/
public, previous carrier/literal/unary/zero helpers/publics, full control-flow,
macro, Numeric behavior/equality, int64 exact expressions, Currency literals/
overflow and licensing/isolation/selection/changelog contracts. The final
subsequent source edit only clarifies two comments; postpush exact-head build/
focused verification is retained separately. Named executed hosted sanitizer
receipt, actual clean exact-head review and strict checks/resolved conversations
remain merge gates, not preclaimed by these local results.
Both new tests are selected in sanitizer build AND actual CTest filter; the
selection contract three methods passes. Native runner25-second limit and all
existing runtime test timeouts/expected controls are unchanged.

## Exact standalone helper commands

Compiler banners are retained; the repo GNU banner drops its final empty line
for Git whitespace hygiene. Original raw banner remains in the task build;
native captures/diagnostics/results are not normalized.

From the slice root (each compile then its resulting binary was executed;
stdout/stderr/status retained in integer-evidence):

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -I include tests/test_prg_numeric_integer_metadata_helper.cpp -o build/numeric-division-native/integer-helper-gnu
clang++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all -I include tests/test_prg_numeric_integer_metadata_helper.cpp -o build/numeric-division-native/integer-helper-clang
```

The same GNU flags were used for the four named previous standalone helper/
carrier tests, with matching source and output filename per test. The Clang
result is explicit local float-cast-overflow evidence, not a claim about flags
in a future hosted lane. Recovery's first console-only summary command had an
extra closing parenthesis and failed syntax after audit0; corrected display
did not change captures/audit. A failed documentation patch was atomic/no-op.

## DQ/DV, hazards, rollback and mandatory remaining work

MEDIUM delta/misuse/rollback are frozen in integer-contract.md. Walkthrough:
inspect documented typed fields rather than unused raw slots; verify independent
expected correspondence; distinguish original schema compile1 from rebuilt
original76 runtime failures; check same-source fixed0/default-real controls;
verify actual broader/hosted/review gates before merging. Implementing-agent
development self-review/automation, NOT independent-human/system/release
qualification. Existing HZ-data-corruption-01/HZ-runtime-crash-01/HZ-doc-command-01
apply. Revert producer/payload hooks/carrier/helper coherently and rebuild if
needed; retain native facts/owner objective and notify through PR/changelog.

This is NOT full Integer/Numeric/division/display/ListItem parity. Longer whole
forms, larger whole Numeric/scientific literals, native Integer/Numeric function/
field producers, assignment-width normalization, arithmetic metadata/default
display/exact conversion remain mandatory. Plain copy does not establish native
assigned width10, and a new storage tag alone does not establish propagation.
Re-enter the retained648 division counterexamples with typed operands; complete
a normalized arithmetic contract/independent original-fail tests before wiring.
Pending ListItem supplied-column READ is uncancelled; #7189/#5611/#6776 remain
open, not converted into residual exceptions. No VM/start/clone/reset-complete
claim or SDK/proprietary implementation copying.
