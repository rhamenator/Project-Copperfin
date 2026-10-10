# Frozen signed64 CAST inputs and original baseline audit

RQ-CF-PRG-CAST-INT64-NUMERIC-001 mapped before helper/tests/migration.
Freeze2026-10-10T00:55Z BEFORE configuration:56direct and4x141fresh public
cases (COPPERFIN/VFP9, selected sessions1/2), all3aliases, localized messages
in all4catalogs, assignment/kind/continuation/cursor/context/reset. Native
ordinary controls and extension absence were independently observed in3runs;
extension expectations are literal owner-derived integer mathematics, not
Copperfin output. Python big-integer/nextafter computations independently
confirmed the literal endpoints and exact constructor sums before freeze.

## Frozen hashes

```
7945091f61677868e0b6f45a88ffecd222580ac268e488b8334bb41f0f50ef94  src/runtime/prg_engine_helpers.h
b54976b694dc9e5e3c39929e5705604573c3238d4e8cb49fdee354acf1bc250c  src/runtime/prg_engine_helpers.cpp
138bd297f0be373050a8999bd18b731046878cf9dd2a0ffb0871503d148074c4  tests/test_prg_engine_cast_int64_numeric.cpp
16a7075de74c5af905a08c4beb429ca23ddbec4e899a90a007d4267d83583731  tests/CMakeLists.txt
e5075eb8a532f4200a8e2de2efada3172a5e859be20abd63f704cb950e3ffbb0  tests/CopperfinTestIsolation.cmake
0fe605bef5934d32318c2401d092a62a6bf54f3d03935e82ae603b3d0d6b2b09  src/runtime/prg_engine_runtime_surface_dispatch_general.inl
639778a08a38ef7d89a53d8f04a7daec99faf32c0077867cb9cf12a277225ffe  src/runtime/prg_engine_runtime_surface_platform_helpers.inl
09dba0553343c294b37fc90bc5f96b4387a7416f55e1916a0e7d96c8bf2ec032  tests/fixtures/vfp9-cast-int64-numeric/probe.prg
2d2e0d79855859167db6cd73f990f42bb42b364651d3c55e3c0be8cb969e4f71  tests/fixtures/vfp9-cast-int64-numeric/native-1.out
```

All3native outputs share the output hash. The distinct adapter is frozen UNUSED
by CAST; original dispatcher/shared platform match main8311e7af8. Shared
checked_declared_int64_argument implementation and allotherconsumers unchanged.
No frozen assertion/input repair/removal/weakening without explicit refreeze
and BOTH complete original repeats. Any incomplete log remains negative/partial,
never substituted for a complete baseline.

## Execution boundary

Serial nice10/ionice2:7 -j1 builds/tests, no VM, private RAM TMPDIR for bounded
non-persistence fixtures. GNU Debug-O0-g0 first, then Clang Debug-O1-g0 with
matching C/CXX ASan/UBSan/float-cast-overflow/framepointer flags and EXE/SHARED
sanitizer link flags. Explicit -fsanitize-recover=float-cast-overflow is frozen
before Clang configuration; original UBSAN halt_on_error=0 records the unsafe
CAST diagnostics while permitting every case to finish. Fixed verification
uses halt_on_error=1 with the same binary flags. ASAN leaks disabled/abort1.
Original diagnostic-bearing runs are NOT clean sanitizer acceptance. CLI caps
GNU600s/Clang1200s per shard are operational, not test/workflow timeout changes,
disk/persistence/optimal-timeout qualification or permission to weaken assertions.

Focused regex: ^test_prg_engine_(cast_int64_numeric.*|int64_exact_expression|bittest_neighbors)$.
The two existing neighbor tests are unchanged. BOTH complete original baselines,
full raw logs/counters and frozen hashes are required BEFORE CAST-only migration.
Fixed/broader/sanitizer/assurance/review/hosted acceptance remain pending.

## Initial incomplete GNU run and explicit preservation-control refreeze

GNU configure/build72824 exit0. Initial CTest19264 exit8 in8.39s; four public
shards completed141cases each and both unchanged neighbors passed. The direct
shard aborted before its56counter: the test incorrectly assumed nonnumeric
Character 'x' would coerce to0. Existing coercion throws std::invalid_argument,
and the unchanged public dispatcher catches it as error1 with assignment
retained. That is existing non-Numeric preservation evidence, not a new
requirement source or a production defect. No production dispatch change made.

Initial full unedited816-line original-gcc-incomplete-initial.log SHA-256:
ce381ff7423f801b71d0f2d1f89d5f63ce51b041ec348513c3bf55c4e25069fb.
Observed708 selected assertions include36 incorrect Character-preservation
assertions. The incomplete direct shard means this is NOT a complete original
baseline or clean acceptance. No diagnostic or failure was trimmed/overwritten.

Explicit refreeze2026-10-10T01:17Z BEFORE repeat GNU rebuild/Clang configuration:
keep every input and all Numeric/exact64 expectations/assertions. Only the
out-of-scope Character preservation control now checks the unchanged direct
exception/public error1 and retained assignment; its call is not removed.
Counts remain56direct/4x141public. New test source SHA-256:
cff432fb15e3c1435dd74243376027b64c5a991e91674c7e7a6bdf49584b1a93.
Unchanged Numeric table before/after SHA-256:
d0ad42528b220a2041e6e4fa8f8b7a4fd5155b3c95ae4f6a656234505664d34e.
Unchanged exact64 public table before/after SHA-256:
5d75fd44a188b07de0e7c47ea60e9cd2878e9e29e9cbf61fcffbcd51071d58c3.
All other frozen inputs/helpers/native/ORIGINAL dispatcher hashes still match.
Redo BOTH complete original GNU/Clang baselines before CAST-only migration.

## Complete refrozen GNU original BEFORE Clang configuration or migration

Rebuild/CTest72689 completed: build exit0, CTest exit8 (expected selected gap),
7.25s total. Actual56direct/4x141public reached their frozen counters; all direct
checks and both unchanged neighbors passed. Exactly672 selected assertions:
180domain-error,180assignment-kind,264result,48localized-message; zero other
assertions, including Character preservation/context/continuation/reset/counts.
Full unedited779-line original-gcc.log SHA-256:
e0c739c977d507b6f1c55d9b6abf832e5b2965a92e4bbd8570f8b815c34b0946.
No test input/expected/guard/helper changes after explicit refreeze. Original
dispatcher/shared platform still match main8311; helper remains unused by CAST.
This completed negative baseline establishes the selected gap, not acceptance.
Clang configuration/build/full original baseline follow with the frozen recovery
settings; no migration until both originals complete.

## Complete refrozen Clang original BEFORE migration

Configure/build52565 exit0; CTest exit8,13.57s total. Actual56direct/4x141public
reached all frozen counters; direct and both unchanged neighbors passed.
Exactly672 selected assertions, matching GNU:180domain-error,180assignment-kind,
264result,48localized-message; zero other assertions. Four actual UBSan
float-cast-overflow reports identify original selected dispatcherline668 (one
per public process). The frozen recovery setting permitted full completion;
this diagnostic-bearing negative baseline is NOT clean sanitizer acceptance.
Full unedited875-line original-clang.log SHA-256:
07a1adf22372e8bd8a59e607fadd2107c98fa1ac80e5a9ab187cb617c29cd2ff.
Test source stillcff432fb15e3c1435dd74243376027b64c5a991e91674c7e7a6bdf49584b1a93;
original dispatcher still0fe605bef5934d32318c2401d092a62a6bf54f3d03935e82ae603b3d0d6b2b09.
BOTH complete original logs retained byte-identical BEFORE the signed64-only
dispatcher migration; no input, expectation, guard or helper changes.

## Fixed focused validation after both complete originals

Signed64 dispatcher-only migration uses the frozen distinct adapter and throws
the existing localized PrgCompatibilityError11 on rejection before assignment.
GNU build/CTest47156 exit0:7/7PASS7.54s. Clang build/CTest10419 exit0:
7/7PASS9.55s with the same frozen binary flags and UBSAN halt_on_error=1.
Actual56direct/4x141public counts complete, neighbors PASS, no failures or
sanitizer diagnostics; leaks disabled as declared. Tests/helpers unchanged.
Full102-line fixed-gcc.log SHA-256:
eaf509949c789b7b1ae432eda262e7261a719eb972c186cb17cd992b56648ea4.
Full102-line fixed-clang.log SHA-256:
1be26a860f54a6b281fd6596a9986172e2d848c8f8cb60a12baf627f4e414ac5.
Copies cmp-identical to raw build outputs. Broader/assurance/integration pending.

## Broader verification and completed bounded documentation walkthrough

GNU80498 build/CTestexit0:11/11PASS421.84s, covering Numeric, NULL, typed-NULL,
string-math, localization and six selected contracts. GNU90647 first phase:
portable CLR host boundary contract1/1PASS0.04s; the prior regex missed its
actual name, so the full separate raw log is retained rather than claiming it
ran in the11-test batch. Seven selected contracts pass in total. The expected
locale-install missing-catalog negative subprocess diagnostic is preserved in
the full passing batch log, not silently filtered or an unexpected failure.
Clang90647 build/CTestexit0:4/4PASS106.67s for the four unchanged runtime
neighbors with the frozen sanitizer binary flags, UBSANhalt1, no diagnostics
and leaks disabled. No VM or parallel local builds/tests used.

Full raw logs, cmp-identical to their build outputs, SHA-256:
1ccf819ecd88cf1b08e3dda781486c5fec57820f5ab49e5a03406c33a70dfc8b  broader-gcc.log (162lines)
ac9e0dba4c93ccda501f04aa56de3803b8aebc3c5e2af722f2a52f8abc798258  portable-clr-gcc.log (35lines)
57d6322ee33fbe21321bf272fabfed5b6de719089145d461d489ed206542812a  broader-clang.log (69lines)
Fixed dispatcher SHA-256:
6797a16d57edf25f1d6c9b6d3e3d980c6a21d2e1ed311facf5eee76f21adf71d.
All frozen helper/header/test/CMake/isolation/shared-platform hashes unchanged.

DQ-5611-CAST-INT64-NUMERIC-001 procedural delta reviewed: use exact64 values
when preserving identifiers above2^53; a rounded Numeric spelling of INT64_MAX
is already2^63 and rejects. Fractional Numeric truncates, never rounds; invalid
Numeric/exact uint64 domain raises11 atomically, without wrap/clamp even in VFP9
mode. Ordinary Character 'x' still follows existing coercion error1; no new
native type/Currency/unsigned-target/arity policy is advertised.

DV-5611-CAST-INT64-NUMERIC-001 completed2026-10-10 by Codex under delegated
development authority: inspected the selected dispatcher, exact-kind-first
checked_declared_int64_argument and finite/range guard before the native cast;
reviewed reverse requirement links, native unsupported-type boundary, literal
math-derived expectations, test isolation and full unfiltered raw evidence.
Automated GNU/Clang focused walkthrough reached56direct/4x141fresh public
cases each:1.9=>signed64 1; re-CAST exact9007199254740993 keeps its low bit;
Numeric2^63 and exact uint64 aboveINT64_MAX reject11 while Numeric12345
sentinel remains, continuation/control executes; constructed exactINT64_MAX
succeeds with exact kind. Both modes/sessions/cursor/reset and four catalog
messages pass. Existing exact-expression/runtime-surface/Numeric/NULL/typed-NULL
neighbors and localization/contracts pass. Bounded medium documentation-misuse
self-review plus automated walkthrough complete, NOT independent human or
high-hazard signoff. HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01
remain active; no KBX or full-platform/persistence/leak/release acceptance.

Rollback reviewed: revert this slice only, retain original/fixed/native evidence
and boundary tests; correct guidance and notify affected integrators if a
published procedure is wrong. Exact-head clean Claude/authorized Codex review,
required hosted checks and resolved conversations remain before integration.
