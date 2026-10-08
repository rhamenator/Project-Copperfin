# FV/PV unsupported optional-argument conversion boundary

## Governing requirement recovered before implementation

RQ-CF-PRG-FINANCIAL-OPTIONAL-ARITY-001 is a bounded slice under owner-admitted
#5611/#6776 and the separately verified OPEN, owner-authored, agent-approved
#5878. Its explicit acceptance requires rejection of non-VFP optional arguments.
Installed VFP9 7423 returns catchable error 1230 for all 26 probed four/five-
argument calls, including huge or NULL optional values. Three
fresh serial runs match all 37 lines and preserve guard41. Eight native three-
argument observations recover existing #5878 order/sign/zero-rate gaps; they
are not passing Copperfin financial compatibility claims.

The selected behavior: FV/PV with more than three evaluated arguments reject
localized catchable1230 in both NUMERICBEHAVIOR modes before optional coercion,
llround or narrowing. Remove both unsupported fifth-argument integral paths
and the unused fourth/fifth-argument formula branches. Preserve the exact
three-argument calculation, ordering and dispatch, even where #5878 records
an unfinished compatibility gap. Too-few arguments, type/domain/global NULL,
user-routine routing and PAYMENT remain unmodified.

The owner's extension policy does not make a conflicting VFP builtin signature
an approved extension. Microsoft documents the useful modern annuity form in
[Financial.FV](https://learn.microsoft.com/en-us/dotnet/api/microsoft.visualbasic.financial.fv?view=net-10.0),
[Financial.PV](https://learn.microsoft.com/en-us/dotnet/api/microsoft.visualbasic.financial.pv?view=net-10.0)
and [DueDate](https://learn.microsoft.com/en-us/dotnet/api/microsoft.visualbasic.duedate?view=net-10.0):
rate/periods/payment, optional balance and beginning/end timing. That is useful
extension design provenance, not authority to override #5878's explicit native
builtin contract. A future non-conflicting extension needs its own admitted
design; this slice does not cancel or demote modern financial capabilities.

Source SHA-256: `4bcd4cf2574ec3a2b436641e9b2c68cb1b329d299b72630cdd0e3e4c2df110ef`.
Output SHA-256: `6c34e6063f8046b2b822b768ddd7e850353d58458dc2293785f4422be4e87887`.

## Architecture and verification plan

Single numeric dispatcher precondition; existing TooManyArguments locale key.
512 direct calls:468 excess-arity boundaries,20 required/optional NULL
precedence controls and24 small three-argument arithmetic preservation calls.
240 fresh guarded script cases/1200 PRG rows cover errors, messages, state
and cleanup, including four/five/six arguments and NULL in every position.
Remove, rather than emulate or saturate, the two unreachable/non-contractual
llround-to-int paths. Direct tests pin arity precedence for exact integers,
adjacent bounds, NaN/infinities, huge values and other types, plus independent
three-argument preservation vectors. Fresh script cases check localized error,
unassigned result, unchanged guard cursor/pointer/session/settings and cleanup.
Tests precede production changes. Original/fixed serial GCC and Clang
ASan/UBSan/float-cast-overflow runs use private TMPDIR; library llround range
errors need not trigger float-cast instrumentation, so no invented sanitizer
UB finding is claimed. Explicit floating exception checks detect FE_INVALID
caused by the removed out-of-domain llround path. String/math, NULL and
date neighbors, repository contracts, exact-head review and hosted required
checks remain distinct acceptance/merge evidence.

HZ-runtime-crash-01 and HZ-data-corruption-01: reject unsupported inputs before
unsafe numeric conversion or misleading financial output; no argument-sized
allocation/loop or state mutation. Residual three-argument correctness remains
#5878, with existing duplicate report #6944. Requirements/code/tests reverse
links are retained here and in docs32. Development evidence is not independent
human review, complete financial compatibility, release or platform qualification.

## Completed development evidence

VR-5611-FINANCIAL-OPTIONAL-001: tests were added against unchanged production
main adfbec00d90ae99583b6d4221fe710aaf8582d74. The initial test accessor compile
typo was repaired before execution and is not a product failure. GCC15.2
full Numeric fails119.99s with776 selected assertions:468 excess-arity,
20 NULL precedence,240 script rejection and48 FE_INVALID checks; all older
rows,24 small arithmetic controls and three neighbors pass, total121.00s.
All240 script setup/post-state/cleanup/call-state suffix comparisons match.
Focused Clang21.1.8 ASan/UBSan/float-cast-overflow reproduces the same776,
zero state mismatches and no sanitizer diagnostics; three neighbors3/3 3.40s.
This focused invocation is not a complete Numeric sanitizer baseline.
No unsupported claim that library llround generated a float-cast UB report.
The retained original-regression-audit.txt is a condensed negative audit with
all48 direct floating-exception failures and one complete script mismatch,
not the full raw logs. SHA256:
`5f3b63c28166341d1cc6376370e46a57984201c597adf80083475882acdd5787`.
Full generated logs remain in the owned build directories until merged cleanup.

VR-5611-FINANCIAL-OPTIONAL-002: after the single dispatcher precondition and
removal of optional conversion branches, full GCC Numeric and three neighbors
pass4/4 84.76s (Numeric83.84). Focused sanitizer512 direct calls and240 fresh
cases/1200PRGrows pass174.67s; three sanitizer neighbors3/3 3.73s; no sanitizer
diagnostics. ASAN detect_leaks=0, so no leak-check qualification is claimed.
Both builds run serial runtime checks with fresh private TMPDIRs.
Six repository contracts pass6/6 5.28s;170 changelog fragments validate.
Compiler warnings in unrelated existing sources remain; not warning-free
build, independent-human verification, full VFP financial compatibility,
cross-platform/release qualification or complete parent-workstream evidence.

DQ-5611-FINANCIAL-OPTIONAL-001 / DV-5611-FINANCIAL-OPTIONAL-001: procedure
delta is rejection/exception evidence and reproduction guidance only; existing
three-argument calculations are explicitly not endorsed as correct VFP finance.
Misuse severity medium: confusing native arity acceptance with full financial
compatibility could lead to incorrect downstream calculations. Development
self-review by Codex in the owner-maintainer workflow compared all26 native
arity records, audited all240 original script state suffixes, reviewed the
single rejection-before-coercion delta and checked known core gaps #5878/#6944.
This is not a second qualified human or independent review. Walkthrough:
valid three-argument controls retain output; unsupported fifth1E300 becomes
caught1230/unassigned, with guard cursor/pointer/session/mode and cleanup
unchanged; fixed tests verify this in both modes. Native NULL optional arity
also rejects1230; supported-count NULL/type/domain behavior is not repaired here.
Rollback: a reviewed signed revert with the same gates, not a destructive
checkout/reset; review PR/changelog notification must retain the unfinished
financial core warning and must not restore unsafe optional conversions by
relabeling them as native quirks. Hosted checks and exact-head Claude/authorized
Codex review remain separate merge gates.

## Reproduction (private scratch builds; runtime checks serial)

Build only the Numeric, string/math, NULL and date test targets with GCC Debug
or Clang21 Debug plus `-fsanitize=address,undefined,float-cast-overflow
-fno-sanitize-recover=all -fno-omit-frame-pointer`. The original comparison
uses the pinned base production source with these regressions applied;
never reset or overwrite unrelated user changes.

For focused checks, set TMPDIR to a fresh `mktemp -d` directory inside the
owned scratch build, then invoke:

```sh
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
  /home/rich/temp/copperfin-annuity-5611-sanitize/tests/test_prg_engine_numeric_behavior \
  --financial-optional-arity-only
ctest --test-dir /home/rich/temp/copperfin-annuity-5611-build -j 1 \
  -R '^test_prg_engine_(numeric_behavior|string_math_functions|null_builtins|date_time_functions)$' \
  --output-on-failure
```

The CTest default still executes all Numeric workstreams. The focused flag
runs only the two new tests; no full Numeric sanitizer claim follows from it.
Do not overlap runtime runs from different builds. `probe.prg` runs with the
installed unchanged `/home/rich/bin/vfp9-probe` wrapper, serial three times.
Generated FXP and owned scratch builds move to recoverable trash after merge;
probe source/output/audit remain tracked. No credentials, decompiled source,
system-changing VM tests or operator data are involved.
