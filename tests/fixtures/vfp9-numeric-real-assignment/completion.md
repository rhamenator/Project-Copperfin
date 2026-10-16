# Bounded real assignment development verification

Later hosted stress exposed an older V2a test timing-precondition defect;
[separate CI repair #7202](ci-stress-repair/README.md) retains original failure,
controlled reproduction and unchanged Numeric inputs. Previous-head acceptance
does not qualify the repaired head; fresh exact-head integration gates apply.

RQ-CF-PRG-NUMERIC-REAL-ASSIGNMENT-001 / VR-5611-NUMERIC-REAL-ASSIGNMENT-001,
DQ-DV-5611-NUMERIC-REAL-ASSIGNMENT-001. Admitted #5611/#6776/#7189.
[Native recovery](README.md) and [frozen pre-code contract](real-assignment-contract.md)
were signed/DCO checkpointc820c1a6c30dca8a91ab7c9197110afa601985ac BEFORE
the helper, public test or runtime hook. Existing code is not the requirement.

## Architecture and preserved boundaries

src/runtime/prg_numeric_real_assignment.h validates optional real-storage
identity, scale1..18 and finite magnitude<1e10 before deriving bounded width
11+D+(payload<0). Negative zero's sign bits stay unchanged with no sign width.
Maximum derived width30, independent of old lexical width. No cast, rounding,
epsilon, allocation, formatting or callbacks. Unsupported descriptors remain
unchanged, not reclassified. Composition with the existing Integer helper is
only inside the scalar-memory '=' lambda in prg_engine_dispatch.inl, guarded
by Number/non-NULL and existing expression-text presence. Plain and m. routes
run it after array/property routing. General assign_variable/STORE/loop/other
transfers and RHS evaluation remain unchanged. Only a changed descriptor
copies PrgValue; unrelated/already-normalized values retain the old path.

## Verification chronology

1. Paired1344 native observations per run: actual0/empty stderr/allN/no errors,
   exact raw/typed repeat, no scope/copy/compiler differences. Rounded
   magnitude1e10 controls are explicitly excluded, not used to infer a rule.
2. Unused helper GNU -std=c++20 -Wall -Wextra -Wpedantic -Werror compile/run0;
   Clang same plus -fsanitize=address,undefined,float-cast-overflow and
   -fno-sanitize-recover=all compile/run0. New public Clang Werror syntax0.
   Helper includes unknown/Integer/Decimal0/19/max-scale, nonfinite/magnitude
   boundary/max-old-width controls. No baseline runtime claim from compilation.
3. Independent46-row expected constants match920 native assigned records;
   no production helper imported. Existing18 Integer-row table remains exact
   SHA256 c44cd2626e19817521c7559c4049075f24b4b025a143a25b63b2ff194322c15e.
   The old staged assigned -0.0 real control intentionally changes W4D1 ->
   native W12D1 BEFORE baseline testing; its bits assertion is unchanged.
   Direct-expression -0.0 control remains W4D1. No Integer assertion weakened.
4. ORIGINAL production CPP/dispatch hashes verified, configure/build0.
   Frozen new public test1:2816 descriptor-only failures (44 of46 rows;
   ten-digit literal widths already match). Updated Integer public test1:
   exactly16 staged-real-control failures; all Integer rows and other controls
   pass. Native expected/source/hash inputs verified before and after hook.
   audit-original.cjs rejects any other failure classification.
5. SAME frozen real and Integer public tests now exit0/empty stderr, all
   input hashes unchanged and expected/native audit0. The first broader run
   was mistakenly launched before the control-flow target finished building;
   CTest marked that missing executable Not Run. Retain the unsuccessful
   attempt, then rerun all28 after the build completes, with no source,
   expected-value, assertion or timeout change. Complete post-build GNU
   rerun28/28 PASS161.05s, exit0; final source and frozen input hashes0.
   Includes both new tests, previous metadata/Integer/unary/zero/literal,
   macro/control-flow/Numeric equality/behavior/exact64/four Currency tests
   and isolation/selection/licensing/changelog contracts. Actual
   named hosted sanitizer execution, clean exact-head Claude/authorized Codex
   review, all strict required checks and resolved conversations remain
   mandatory integration gates; selection is not execution.

The public grid covers both modes, DECIMALS2/14, FIXED OFF/ON, all three
scopes/copies/plain and m. namespace. Payload/storage/scale/NULL/unknown/
Integer/direct-expression/paused-watch/continuation controls are retained.
Build logs retain existing unrelated TemporaryAppendSource initializer
warnings; no new warning cleanup is folded into this compatibility child.
Compiler command flags above and version logs identify the explicit local
helper instrumentation; they are not claims about hosted flags.
The first staged whitespace check exited2 on GCC --version stdout's final
blank line. A fixture-local version-log-only attribute preserves that raw
compiler output; no product/test or native capture bytes were changed.

## DQ/DV, rollback and unfinished work

MEDIUM procedural delta/misuse/walkthrough/rollback is frozen in the contract.
Implementing-agent development self-review plus automated verification, not
independent-human, system or release assurance. No new conversion/allocation
limits or compatibility switches; #6776 policy unchanged. Frozen native rows
and original-only failures provide the before-hook walkthrough; unchanged
frozen tests and complete28/28 provide the after-hook half. These local
results do not replace actual hosted execution and exact-head review.
Rollback removes real helper/composition and restores the prior staged real
control; Integer foundation and native evidence remain. Decimal0/larger real
assignment, unmigrated producers/transfers, retained division propagation,
display/exact conversion and ListItem supplied-column READ remain mandatory.
This child does not close the parent issues or establish full Numeric parity.
For a discovered regression, notify the owner and affected consumers through
the issue/PR and release notes; do not silently relabel evidence as passing.
