# Runtime division implementation chronology

RQ-CF-PRG-NUMERIC-RUNTIME-DIVISION-001 / VR-5611-NUMERIC-RUNTIME-DIVISION-001.
This is an in-progress record, not completed runtime or integration acceptance.

Before hooks, the paired payload grid has12960 values/run, actual0, empty
stderr/no errors; normalized fields and quotient bits repeat. Raw hashes DIFFER
only in audited unused right-operand tagged slots (6636 rows), not relevant
metadata. First SHA5c857da60c5a7bab8234a0e1a0f1e26cf852c63ec9b9e23c78fc6cf3f80e0b8a;
repeat SHA8edbaffc8eb66c02c750a9ab8a945febc7738a094b41547f40333ec4084b2fc4.
Captures are stored as deterministic lossless gzip; the auditor decompresses
and verifies original raw SHA/runner-output identity without normalization.
The ordinary public grid has60 expressions/1200 values per run, actual0,
empty stderr/no errors and byte-identical SHA
0c5310c56d6834f793bbd2baf09313520df4b49af450dc3cfd1401350c8ed451.
Compiled/EVALUATE differences0 in both grids. `audit-runtime-increment.cjs`
imports no production helper; it actually matches all1200 native public rows
against the frozen60-row C++ constant header.

Unused helper GNU warnings-as-errors compile/run0 and Clang explicit
ASan/UBSan/float-cast-overflow/no-recovery compile/run0. Initial GNU compile1
was a misleading-indentation diagnostic in the test's final return/output;
formatting alone corrected it. Original diagnostic/status retained on the host.
No expected constants, assertions or native source/capture were changed.
Public test Clang warnings-as-errors syntax0. Original runtime source hashes
and exact public test/header/contract/helper inputs are frozen before hooks.
The unchanged runtime is being built to establish actual ORIGINAL failures;
compile success is not behavioral failure or sanitizer runtime acceptance.

Next required: finish baseline build/run, independently audit original failures,
apply only the bounded ordinary-division hook and transient context scope,
re-run SAME frozen tests and broader controls, preserve exact chronology and
obtain actual named hosted execution/clean exact-head review/strict gates.
No production include or arithmetic/context hook exists at this checkpoint.

Subsequent bounded hook: original build0, public test1 with exactly1220 native
descriptor failures; all payload/kind/unknown/NULL/type/zero/Currency/suppression/
paused-state/resume/callback controls passed. `audit-runtime-original.cjs`
actually returned0 and checks every exact failure label against60 independent
native rows,5 settings,2 FIXED modes,2 behavior modes and the memory prefix.
Original core source hashes verified0 before adding the include and two hooks.
SAME frozen public test after the hook: build0/test0/empty stderr; prewire
public/header/contract/helper SHA check0. GNU broader controls and actual
hosted/review/strict integration gates are still pending, not preclaimed.

Architecture delta is one include, a scoped aggregation at parse_multiplicative
entry, and the ordinary division result's bounded metadata assignment. Existing
NULL/type/Currency/exact64/zero checks precede it. No producer replay, operand
coercion, arithmetic payload change, other operator metadata or persistent
provenance. Both new targets are now in CMake/isolation, sanitizer build AND
actual CTest selection; selector's three methods pass. The new public test is
serial/test-owned-unique/scoped-process; helper is pure/parallel-safe.

Before any baseline test execution, static review added the same explicit
pause sentinel used by prior assignment tests: the breakpoint must not stop
before the last nScale18 initialization. The first source and pre-sentinel
manifest are retained on the host; expected constants/assertions unchanged.
The pre-code public/header/contract/helper manifest was refreshed before the
ORIGINAL run. A generated header's final blank EOF was likewise removed before
that run; its first bytes/manifest and staged diagnostic remain retained.

## Final local verification

Complete selected-target build0; GNU CTest28/28 PASS163.47s, including the new
helper/public tests, prior carrier/literal/unary/zero/Integer/assignment tests,
full control-flow93.76s, Numeric behavior65.16s, macro expressions, Currency
literal/function/overflow/exact-muldiv, exact64, isolation, licensing, changelog
and sanitizer-selection contracts. No assertions or timeouts changed. SAME
frozen public test/header/contract/helper inputs still match their manifest0;
current hook/CMake/isolation/driver/selector source manifest0. Native original
failure classification and both new capture audits independently replay0.
GNU runtime is a Debug -O0/-g0 build, not a public-runtime sanitizer claim.
The standalone unused Clang helper alone used explicit ASan/UBSan/
float-cast-overflow/no-recovery. Named hosted execution, clean exact-head review,
all strict checks and resolved conversations remain required before merge.

Local reproduction from the slice root:

    cmake -S . -B build/division-focused -DCMAKE_BUILD_TYPE=Debug \
      '-DCMAKE_CXX_FLAGS_DEBUG=-O0 -g0' -DCOPPERFIN_BUILD_TESTS=ON
    cmake --build build/division-focused --target \
      test_prg_engine_numeric_runtime_division test_prg_numeric_runtime_division_helper -j2
    ctest --test-dir build/division-focused --output-on-failure \
      -R '^test_prg_(engine_numeric_runtime_division|numeric_runtime_division_helper)$'

Build every target in final-ctest.log before reproducing its broader selection;
that log and exact-source manifests retain the actual complete selection.
