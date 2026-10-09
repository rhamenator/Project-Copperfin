# Collection frozen baseline audit

RQ-CF-PRG-COLLECTION-SELECTOR-NUMERIC-001 / VR-5611-COLLECTION-SELECTOR-NUMERIC-001/002.
Base origin/main8a9f8f61a5e4267df769bb84ce7d5652fe024364. Requirements recovered
and mapped BEFORE helper/tests. Unused checked helper added; resolver/callers/
method declaration stayed byte-identical to base through BOTH original compiler
baselines. Narrow resolver/mode migration followed their retention; fixed
focused/shared/broader results are retained below. Frozen BEFORE first configuration:

| Input | SHA-256 |
| --- | --- |
| src/runtime/prg_engine_helpers.cpp | fd7deb506483f778e61910872473def1ed466729cb1fdf817cc799807bd386d4 |
| src/runtime/prg_engine_helpers.h | 26c27fd6e174da5bff90953122248a2c66b760ea00e2aef636ae14844d12eea0 |
| tests/test_prg_engine_collection_selector_numeric.cpp | 6dfee32969270cd1cd3f0a427bc9beb5ef92704db718985b21f25ffde6bc59a2 |
| tests/test_prg_engine_collection_selector_neighbors.cpp | b72d7ff2b86edd8838ab4432a703fb5ce43dacaf31273719a6b3b6c5ec82300d |
| tests/CMakeLists.txt | bb7717ad04a8be3450cfc0c3b64acd261fea375341fdf27c60c62c5d99f163d3 |
| tests/CopperfinTestIsolation.cmake | 29188956ede9139056b25a891c488887203164cd08951054575f8bc8a0cf0173 |
| src/runtime/prg_engine_runtime_surface_reflection_helpers.inl | 50b41712a989d40e83d1feda09b7a1c65a40f0d459e64655db08f7c47915d505 |
| src/runtime/prg_engine_runtime_surface_object_methods.inl | afd2aa6887e43c36c11109981bc08444c5c0c77e317f20aecb65b80b5e0d50e8 |
| src/runtime/prg_engine_expression.inl | 84387dbbd8e81cd796688a43643fa0384606606ce25e05fc59c2627209fdf2e3 |
| src/runtime/prg_engine_native_object_focus_dispatch.inl | 6d286cc1b2a79147118b0bdab69303d670ef928226ef7438ece1015a13e74d88 |
| src/runtime/prg_engine_runtime_surface_functions.h | 568ae7a3ad29e437298b6ba56ad6eacbc7ca269197168eae1ece4276e2960e77 |

Independent native35 Numeric operand rows across both modes. Direct helper
boundaries include nonfinite/adjacent limits/exact64/Currency containment and
non-Numeric rejection (key routing remains the resolver's separate path).
Public140 fresh scripts (35rows ×2modes ×2selected sessions), each checks five
read surfaces and one Remove against alpha/beta/gamma and independently expected
survivor order/count/key alignment. Opposite-origin session2 mode, cursor,
continuation and release/reset guards retained. Hidden controls:26 fresh
scripts with two member-chain reads and read-only Remove/count/identity guards.
Eight unchanged original integer/key/default/subscript/duplicate/hidden/pageframe/
application-window neighbors. Test executable prints actual case counts.

GNU15.2/Ninja Debug-g0, testsON/fuzzOFF/robustnessOFF, one-job build owned at
/home/rich/temp/copperfin-collection-selector-5611-build. Strictly serial/private
mktemp TMPDIR,180s test cap. AFTER GNU original completes, separately configure
Clang21.1.8 Debug-O1-g0 ASan/UBSan/float-cast-overflow/frame pointers and matching
linker flags in copperfin-collection-selector-5611-sanitize, same frozen inputs
and private TMPDIR, one-job build. ASANdetect_leaks0/abort1 and UBSANhalt1/stack1.
Retain full actual original logs/results/hashes BEFORE resolver migration.
No assertion removal/weakening/helper/CMake repair between original/fixed;
if any repair is necessary, record it and repeat BOTH original baselines.
No overlapping Copperfin build/test/VM, platform/leak/GUI/release/independent-review claim.
Development review/walkthrough is retained in README; external integration
checks/review remain separate gates.

## Original GNU before migration

One-job GNU build succeeded; serial/private-TMPDIR CTest exit8, total2.39s.
Numeric failed2.25s, eight old neighbors passed0.13s. Actual frozen counters:
116direct/140public/26hidden. Exactly398 failed assertions, all selected lookup/
removal/hidden identities; no direct-helper/setup/continuation/context/read-only/
cleanup/neighbor failures and no runtime diagnostics. Full original-gcc.log
retained byte-identically in this fixture, SHA-256
82ca6358088be2a3e7ee9a4bec3a1cdb03782749e5a394107a59e80edeb8f86b.
Actual categories280identity/100removal/18hidden/0other. No assertion/helper/CMake repairs,
and resolver/callers/method declaration still original for both compiler baselines.

## Original Clang before migration

One-job Clang sanitizer build succeeded; serial/private-TMPDIR CTest exit8,
total2.96s, eight unchanged neighbors0.11s PASS. Actual counters116direct/
140public/26hidden. Exactly398 failed assertions, categories280identity/
100removal/18hidden/0other; every FAIL line byte-identical and in the same order
as GNU. Zero other/direct/setup/continuation/state/read-only/cleanup/neighbor
failures and no ASan/UBSan/float-cast-overflow runtime diagnostics; leaks disabled.
Full original-clang.log retained byte-identically, SHA-256
cb02930ae3a489ae33c102f8c2624b4742569f105f6fe3f9889bc1fb98f98303.
All eleven frozen inputs still match before migration; no helper/assertion/
CMake repairs/removal/weakening. Both original baselines now retained BEFORE
the narrow production resolver/caller migration. Fixed evidence follows.

## Fixed focused results

Only shared resolver conversion and explicit selected-session mode routing were
migrated after both original logs were retained. The six frozen helper/test/
CMake/isolation hashes still match; no repairs/removal/weakening. GNU two
executables PASS2.41s (Numeric2.27s/neighbors0.12s), full fixed-focused-gcc.log
SHA-256 d7f396c61ce41ed30e94414edc725a116b12482849b4d535d9fbf454295a9f05.
Clang ASan/UBSan/float-cast-overflow two executables PASS2.90s
(Numeric2.76s/neighbors0.12s), no runtime diagnostics, leaks disabled; full
fixed-focused-clang.log SHA-256
69b0d57047c220680c51cd090e408d26194a534daf1541fe5b21c40eebb43e70.
Same serial/private-TMPDIR/180s caps, one-job/low-priority builds.

## Fixed shared/broader results

GNU15/15 PASS460.79s: focused two executables, older Numeric, shared MRKBAR/
SKPBAR/PRMBAR/GETBAR, localization and seven repository contracts. Private
TMPDIR/strictly serial,600s cap accommodates the existing long safety contract
(342.38s PASS); Numeric behavior80.85s/localization14.31s. Full fixed-gcc.log
SHA-256 4237efa0a4ca307d3f9e3b78223ab3c67475c007350ff7fecb2be4f34c432ad8.
Clang ASan/UBSan/float-cast-overflow3/3 PASS74.92s: focused two plus shared
MRKBAR, same180s cap/private TMPDIR/strict serial/no runtime diagnostics/leaks
disabled. Actual Numeric37.78s/neighbors0.11s/MRKBAR37.01s; slower than first
focused2.90s pass, retained without attributing a cause or claiming performance
qualification. Full fixed-clang.log SHA-256
29698806fd2288ae11d55e6d708eeb320f4b57a5058aab2611557770e424fda0.
No assertion/helper/CMake repair/removal/weakening; all six original frozen
helper/test/CMake/isolation hashes still match after fixed runs. No overlapping
Copperfin build/test/VM. Hosted exact-head checks, clean external review and
resolved conversations remain integration gates, not inferred from local runs.
