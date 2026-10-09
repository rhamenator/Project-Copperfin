# PRMBAR frozen verification configuration

RQ-CF-PRG-PRMBAR-NUMERIC-001 / VR-5611-PRMBAR-NUMERIC-001/002.
Governing requirement mapped before helper or callback migration on2026-10-08.
Admission revalidated against live owner/open/agent-approved metadata.
Base3ed4ff15f3ad270a810e5a3dab688de6a42d01f4, original PRMBAR callback still
uses llround. New unused checked helper/test scaffolding does not migrate it.

Frozen inputs before initial GCC configuration/build:

| Input | SHA-256 |
| --- | --- |
| Original src/runtime/prg_engine.cpp | 9ec338536657ab687ea297225ae96fb672cbac32529f8a16eecff80bf3cb6acc |
| src/runtime/prg_engine_helpers.cpp | ee6082b71a62fb7c831db3821d4d0a9416cdd2eead6bf5664bc3d8b149734640 |
| src/runtime/prg_engine_helpers.h | f8212ac168275d98ebc014f528db815c48ca00ca1a4216d8fd6f63d62a31d7a4 |
| tests/test_prg_engine_prmbar_numeric.cpp | 593d2af700db9cb05cc32c272d2f5bb3f2eaa1e8b4546dbe579ea14c9b9dd935 |
| tests/test_prg_engine_prmbar_neighbors.cpp | 49e55b58f82ef2cec1d4af9a101ec752341b1062f863b24e9efbfda73e048da1 |
| tests/CMakeLists.txt | 7ddf49d79573f3440ab18a1d484e29beb49fd5d9c76383def6e7c2febfcf8674 |
| tests/CopperfinTestIsolation.cmake | 0bd138216a1364e3e61b6314a59ed03a2e30b5e704bef6bd289e950cd6fc881e |

96 direct helper checks (58 Numeric,6 nonfinite,6 adjacent boundaries,
20 exact integer,6 other coercions),372 guarded public-runtime queries and12
inactive normalization/missing-popup cases; three unchanged old menu neighbors.
Expected identity constants derive from allowed native/parent evidence, not
the helper or correlated setter. Guards independently seed all known prompts,
skip/mark flags and callbacks, preserve selected session/cursor/mode and cleanup.

Normal build: GNU15.2, Ninja, Debug with debug symbols disabled (-g0) to reduce
disk I/O; tests enabled/fuzz and robustness disabled. Owned directory
/home/rich/temp/copperfin-prmbar-5611-build. Compile strictly one job; original
build log build-original-gcc.log. Sanitizer build will use separate owned
copperfin-prmbar-5611-sanitize directory, Clang21.1.8, -O1/-g0,
ASan/UBSan/float-cast-overflow with frame pointers and matching link flags.
No concurrent compiler builds or runtime tests, no VM needed.

Before callback migration retain both compiler original baselines, all actual
selected and unrelated failures, sanitizer diagnostics and log hashes. Run
test_prg_engine_prmbar_numeric and test_prg_engine_prmbar_neighbors strictly
serially under a private mktemp TMPDIR. Freeze helper/assertions/CMake through
fixed execution; record any necessary repair separately and rerun baselines.
Fixed execution must additionally cover shared MRKBAR/SKPBAR helpers and older
Numeric/localization/repository contracts. No fixed pass result exists yet;
this record does not constitute acceptance evidence.

## Original GNU baseline, before callback migration

Original build completed successfully, one job. Build log SHA-256
7de2e666d83b8207ed2c356efd1909b6c5ae07d0531becd7b0ea00f7c5fd6341.
The frozen original callback above was run against the same frozen helper/
assertions/CMake under a private mktemp TMPDIR, serially with180s per-test cap.
CTest exit8, total4.29s: numeric test failed4.17s; old-neighbor test passed0.11s.

All110 assertion failures are selected query-conversion identity mismatches:
102 guarded prompt identities and8 inactive prompt identities (2.9 rounded
onto missing bar3). The latter are not prefix-normalization defects: the old
callback never reaches seeded bar2. Zero direct-helper/setup/continuation/
menu-count/session/cursor/mark/skip/callback/cleanup/old-neighbor failures.
No unrelated failures or runtime diagnostic occurred. Fractions0.5/0.9,
1.5/1.9/2.9/MAX.9 expose rounding; explicit positive/negative aliases and
opposite-origin session2 controls expose mode-blind selection. Every actual
failure, guard result and elapsed time is retained in [original-gcc.log](original-gcc.log),
SHA-256323b5802f2563dc3fe215b1cf6ff3923a21fb808ecb71042d60c8276b673b401.
This is fail-before evidence, not a test-suite pass.

## Original Clang sanitizer baseline, before callback migration

Original one-job Clang21.1.8 sanitizer build completed successfully with the
same frozen inputs, Debug/-O1/-g0, ASan/UBSan/float-cast-overflow/frame pointers
and matching link flags. Build log SHA-256
5ab3a05eb8047188770a6a333a3b6bff3b0f2153d9f371d8a32933c26f194d8f.
Serial/private mktemp TMPDIR,180s per-test cap, ASAN_OPTIONS=detect_leaks=0:
abort_on_error=1 and UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1.
CTest exit8, total4.47s: numeric failed4.36s; old neighbors passed0.10s.
Exactly the same110 failure lines as GNU (102 guarded/8 inactive identities),
zero unrelated/direct/setup/state/callback/cleanup/neighbor failures and no
sanitizer diagnostics. Full [original-clang.log](original-clang.log) SHA-256
ed5b8ff5daf7fe3586d76ef0578dd05c22f189057ed0520c0c38bd77e22d0266.
All frozen source/helper/test/CMake hashes still match. This is fail-before
evidence, not a suite pass, leak check or platform/release qualification.

Both original compiler baselines were captured BEFORE callback migration.
Only the PRMBAR callback has now migrated: selected session's numericbehavior
is read through existing current_set_state(), checked optional identifier is
used for lookup, and all fallback/prefix/target/session logic remains unchanged.
Fixed callback source SHA-256
62e83e8503787deff146e4464eb7a4bb25243832e2f7fb23d1bc0d705794e6b9.
Helper/assertion/CMake hashes remain frozen as above; no baseline assertion was
removed, weakened or repaired. Fixed GNU13/13 PASS537.34s (six runtime/shared-
helper/older Numeric/localization plus seven repository contracts). PRMBAR
numeric4.53s/neighbors0.11s, shared SKPBAR4.34s/MRKBAR4.35s, older Numeric156.06s,
localization15.43s, safety traceability350.67s; retain actual longer times
without attributing them to an unproven host-load cause. Full [fixed-gcc.log](fixed-gcc.log)
SHA-2563681893daa365efa9e1f3d36a4d3902c92d7d46dcf91803fd059e05c6b3534f2;
build log2ea07de0c5c369406c60ef6fe3d053c7e04a0940cf2714cc898c191e7157ca05.

Only after GNU completed cleanly, one-job Clang build and four serial
focused/shared-helper sanitizer tests PASS13.93s: SKPBAR4.53s, PRMBAR4.49s/
neighbors0.09s, MRKBAR4.80s. No sanitizer diagnostics. Full
[fixed-clang.log](fixed-clang.log) SHA-256
ced2f9948736ec880d52c128976fec3e4cda79494341a16f2ad41d9cbfe46d5c;
build log40b9e37b28e0f398394ed26e4a6d56fa0da209f9f4732ccfde220722136a40b7.
All frozen helper/test/CMake hashes above still match after both fixed runs.
Each run uses a separate private TMPDIR, no overlapping builds/runtime tests.
Broad GNU suite cap1200s accommodates existing traceability/Numeric contracts;
Clang focused cap180s matches originals. ASANdetect_leaks0/abort_on_error1,
UBSANhalt_on_error1/print_stacktrace1. No leak/platform/full native-error/type/
system-menu/GUI/release qualification claimed; hosted CI and exact-head external
review/resolved conversations remain integration gates.
