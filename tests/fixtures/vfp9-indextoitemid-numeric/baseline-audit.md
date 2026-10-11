# IndexToItemID phase audit

Initial freeze 2026-10-11T08:25Z BEFORE configure/build/measurement.
Pre-code independent map README.md08:23Z; matching native pair120/118 retained.
Distinct helper UNUSED by production; original method/reflection bytes unchanged.
194 direct cases,400 public in EACH of four mode/session shards.
Expected conversions derive independently from native/help/parent mathematics;
signed missing rows preserve Numeric0, no SelectedID ceiling borrowed.
Five frozen files:

```text
211bfc57b0e6b87e8d31737c532e404af10feb0e821091ce6adb5a1d76c4d966  src/runtime/prg_engine_helpers.h
7332320d7c6ca3fc4632d1124c0404468b0027ffcb744d7bed2976f071477dfb  src/runtime/prg_engine_helpers.cpp
34fa0264c16816d76e9e6a8db16d844497502a661289d67ce7952c9f4ce6cabd  tests/test_prg_engine_indextoitemid_numeric.cpp
4932464feb25e025f8bab5d324c1332ca96c10eecd593d070471fac6877b9d59  tests/CMakeLists.txt
a38eedc14e1b816711301e5ecdb564b94f0f4e17a03e65cd4ab6465b0cad878b  tests/CopperfinTestIsolation.cmake
```

Base production:
7693363db8c27e3bbaf394a955ad214cea61a3f9f35d801c2d30e7d04bffd792 methods,
becbd493f8e25f1df211abd795c69507b0d43186f5bfd5625d4ed6f80a175a3d native reflection.
Base a803eb0243121858db987d2021702c4a25135eb3.

Both ORIGINAL complete29-subject GNU/Clang runs must be retained before
migration:5new,5SelectedID,5Selected,5ItemData,5GETCOLOR,numeric,exact,isolation,
existing itemid selectors. No existing assertion/CI timeout policy changed.
Any frozen repair requires explicit refreeze and BOTH originals repeated.
Private700-mode build/indextoitemid-tmp, serial nice10/ionice2:7/-j1,
600s LOCAL diagnostic bound, not CI/performance policy. Full configure/compile
warnings retained, no warning-free/idle-host/leak claim.

GNU:
```sh
export TMPDIR="$PWD/build/indextoitemid-tmp"
cmake -S . -B build/indextoitemid-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug \
 -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g0' -DCOPPERFIN_BUILD_TESTS=ON
nice -n 10 ionice -c 2 -n 7 cmake --build build/indextoitemid-gcc -j 1 --target \
 test_prg_engine_indextoitemid_numeric test_prg_engine_selectedid_numeric \
 test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric \
 test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior \
 test_prg_engine_int64_exact_expression test_prg_engine_runtime_surface_itemid_selectors
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/indextoitemid-gcc -V \
 --output-on-failure --timeout 600 -j 1 \
 -R '^(test_prg_engine_(indextoitemid_numeric|selectedid_numeric|selected_index_numeric|itemdata_index_numeric|getcolor_default_numeric)(_(copperfin|vfp9)_session[12])?|test_prg_engine_numeric_behavior|test_prg_engine_int64_exact_expression|test_native_test_isolation_contract|test_prg_engine_runtime_surface_itemid_selectors)$'
```

Clang separately/never concurrently:
```sh
export TMPDIR="$PWD/build/indextoitemid-tmp"
cmake -S . -B build/indextoitemid-clang -G Ninja -DCMAKE_C_COMPILER=clang \
 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DCOPPERFIN_BUILD_TESTS=ON \
 -DCMAKE_CXX_FLAGS_DEBUG='-O1 -g0 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer'
export ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
nice -n 10 ionice -c 2 -n 7 cmake --build build/indextoitemid-clang -j 1 --target \
 test_prg_engine_indextoitemid_numeric test_prg_engine_selectedid_numeric \
 test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric \
 test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior \
 test_prg_engine_int64_exact_expression test_prg_engine_runtime_surface_itemid_selectors
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/indextoitemid-clang -V \
 --output-on-failure --timeout 600 -j 1 \
 -R '^(test_prg_engine_(indextoitemid_numeric|selectedid_numeric|selected_index_numeric|itemdata_index_numeric|getcolor_default_numeric)(_(copperfin|vfp9)_session[12])?|test_prg_engine_numeric_behavior|test_prg_engine_int64_exact_expression|test_native_test_isolation_contract|test_prg_engine_runtime_surface_itemid_selectors)$'
```
Outputs/statuses/counters and any failures remain pending, not acceptance.

GNU17433 harvested exit8, retained/cmp at2026-10-11T08:37:20Z BEFORE Clang
configure or migration:25/29 PASS231.64s; only four new public shards fail.
All194/four400 IndexToItemID,200/four476 SelectedID,190/four368 Selected,
188/four368 ItemData and174/four80 GETCOLOR counters complete. Existing
numeric/exact/isolation/itemid selectors pass. Raw3301lines SHA256
932ab0985e2a120192437cb9adeefa4e9c6c283108eb1b9f0067da05d8f8215f.
2944 raw IndexToItemID FAIL diagnostic lines including output-on-failure
repetition, ZERO other FAIL lines; not a unique-assertion count.
Preserved .T./.F./.NULL. controls pass Numeric11/0/0 with code0 in both modes,
both controls and ordinary parentheses/brackets; native same-input controls
error27. This is a separate type-admission gap, not permission to widen scope.
All five frozen/base-production hashes unchanged. Helper UNUSED.
Full Clang original remains required; no migration/acceptance claimed.
Clang original configure/build/29subjects now running in session67365;
GNU17433 completed before it started, not simultaneous toolchains.
Focused same-input Logical/NULL recovery gap7147 filed against exact base
main with no labels; outside active Numeric/exact implementation.

## Both complete originals retained before migration

Clang67365 harvested exit8; full3301-line original-clang.log copied/cmp at
2026-10-11T08:57:22Z BEFORE production migration. SHA256
af59cbdcb916cb31337b654cb951cd98280f0163ed44c58dfc569630c77e9324.
25/29 PASS378.38s, numeric behavior149.11s; all194direct/four400public
IndexToItemID,200/four476 SelectedID,190/four368 Selected,188/four368 ItemData
and174/four80 GETCOLOR counters complete. Existing numeric/exact/isolation/
itemid selectors pass. Only four new public IndexToItemID shards fail:
2944 rawown FAIL diagnostics including output-on-failure repetitions, zero
other FAIL lines. No ASan/UBSan/float-cast-overflow report; leaks disabled.
GNU original already fully retained before Clang configure. All five frozen
and both base-production hashes above rechecked unchanged. No fixture repair
or refreeze; helper still UNUSED at this evidence checkpoint.
Next: only raw Numeric/exact method and property-first adapter migration,
then identical fixed29-subject GNU/Clang runs, unchanged preservation groups,
documentation contracts and exact-head hosted review/check gates.

## Bounded production migration

After both original raws/counters/status/hash were retained, only the method's
raw Numeric/exact first-argument read and property-first raw-selector route
were wired to read_native_list_control_item_id_numeric. The new adapter uses
the frozen helper and existing localized catchable31; invalid conversion
precedes indexed access, destination assignment and method success publication.
No frozen helper/test/registration/isolation edit. No non-Numeric/NULL/Currency,
textual GETPEM, parser, other method, write or ID-creation change.

Method entry's existing materialization remains, then adapter materializes at
the old arm's second materialization point before checked conversion/lookup.
Property-first positive reads still materialize once. Raw Numeric nonpositive
selectors now use the same typed property read and return Numeric0, rather
than falling through to the method's two materializations. That physical
fallback count is not claimed preserved or qualified for arbitrary RowSource
callbacks; stable RowSource0 state and existing control preservation are the
bounded verification. _Access/metadata/visibility/delegate ordering remains
upstream and untouched. Whole-call allocation/materialization atomicity is
not claimed. There is no argument-sized allocation or iteration in the adapter.

GNU fixed build/29subjects session18877 now running serially; full
compile-fixed-gcc.log/fixed-gcc.log retained in build/indextoitemid-native.
Fixed Clang, six unchanged list_controls_03 preservation groups under each
compiler, documentation contracts and actual integration gates remain pending.
Verification-only preservation-driver.cpp.txt calls the six existing groups;
it changes no assertion and is not the source of a recovered requirement.

Bounded production hashes after migration (frozen five above unchanged):

```text
ecba9528692224eb419e36b711fcfa8943ea5d545df56d4e1f890b438c970b23  src/runtime/prg_engine_runtime_surface_functions.h
f4bd327f04da4679b4b6988a504203791e8171c707d1d31d51e9860a34a536f3  src/runtime/prg_engine_runtime_surface_object_methods.inl
c36664aad68824488b2b6bd4ce2d86a7c8e9235fbfd1424941696e65b076e930  src/runtime/prg_engine_native_reflection_dispatch.inl
6ff99e261fbfbf9671223696556cb89dbac33ea182857dfadef6d4a785d90fb0  src/runtime/prg_engine_expression.inl (unchanged)
```

Preservation reproduction uses each matching compiler's archives, only after
its fixed runtime suite completes. GNU example (Clang adds the same sanitizer
flags and runtime environment as its frozen build above):

```sh
export TMPDIR="$PWD/build/indextoitemid-tmp"
export COPPERFIN_LOCALE_DIR="$PWD/resources/locales"
nice -n 10 ionice -c 2 -n 7 g++ -std=c++20 -O0 -g0 -Iinclude -Itests -Isrc/runtime \
 -x c++ tests/fixtures/vfp9-indextoitemid-numeric/preservation-driver.cpp.txt \
 tests/test_prg_engine_runtime_surface_functions_list_controls_03.cpp -x none \
 build/indextoitemid-gcc/tests/CMakeFiles/cf_prg_engine_test_support.dir/prg_engine_test_support.cpp.o \
 build/indextoitemid-gcc/libcf_xbase_runtime.a \
 build/indextoitemid-gcc/libcf_design_model.a \
 build/indextoitemid-gcc/libcf_prg_analysis.a \
 build/indextoitemid-gcc/libcf_runtime_text.a \
 build/indextoitemid-gcc/libcf_vfp_assets.a \
 build/indextoitemid-gcc/libcf_platform_profile.a \
 build/indextoitemid-gcc/libcf_security.a \
 build/indextoitemid-gcc/libcf_localization.a \
 build/indextoitemid-gcc/libcf_platform_support.a \
 -o build/indextoitemid-native/preservation-gcc
nice -n 10 ionice -c 2 -n 7 build/indextoitemid-native/preservation-gcc
```

## Fixed GNU checkpoint

Session18877 harvested0:29/29 PASS234.05s. Full346-line fixed-gcc.log copied/
cmp, SHA256736b27852fff6ee704280242629619e23d7f487322dfc31487f126b8bc061e1b.
All194direct/four400public IndexToItemID,200/four476 SelectedID,190/four368
Selected,188/four368 ItemData and174/four80 GETCOLOR counters complete;
numeric/exact/isolation/itemid selectors PASS, zero FAIL diagnostics.
No expectations changed after the original failures.
GNU preservation36938 harvested0 at09:03:40Z; all six unchanged groups
completed, zero failures, full one-line preservation-gcc.log copied/cmp,
SHAbefec198fb62d7e5fdae2e816aff3dc0ebde67970c187f3c7c013a1a377e3f19.
Matching GNU archives/driver/source used; compilation raw retained separately.
Fixed Clang build/29subjects now session98524, serial after GNU suite and
preservation completed; same frozen inputs/sanitizers/privateTMP/600sLOCAL.
Full compile-fixed-clang.log/fixed-clang.log remain in owned build directory.
Next harvest actual completion, retain full raw/status/counters/hash, then
matching Clang preservation driver and five documentation contracts. No fixed
Clang, DQ/DV/contracts, signed commit, PR or external acceptance claim yet.

## Fixed Clang and preservation checkpoint

Session98524 harvested0 at the next heartbeat:29/29 PASS378.05s. Full346-line
fixed-clang.log copied/cmp, SHA256
23810b407dc7cbe8706aced13a37fbf4771e45947d287851396ea3827941ec1e.
All194direct/four400public IndexToItemID and all frozen neighboring counters
listed above complete; numeric/exact/isolation/itemid selectors PASS, zero
FAIL diagnostics and no ASan/UBSan/float-cast-overflow report. This is bounded
sanitizer evidence with leaks disabled, not leak or warning-free qualification.
Five frozen inputs and the four production hashes above reverified unchanged.
Clang preservation23372 harvested0 at09:16:27Z, all six unchanged groups/zero
failures. Full one-line preservation-clang.log copied/cmp, same SHA256 as
GNU preservation:befec198fb62d7e5fdae2e816aff3dc0ebde67970c187f3c7c013a1a377e3f19.
Matching Clang archives and sanitizer flags used after the fixed Clang suite;
compile-preservation-clang.log retained separately. No simultaneous toolchains.

## Bounded documentation delta and development walkthrough

DQ/DV-5611-INDEXTOITEMID-NUMERIC-001 delegated maintainer self-review completed
against the independent pre-code map, original/fixed raw results and unchanged
preservation assertions. The procedural delta is explicit: finite fractions
truncate rather than round (2.9 returns sparse ID22 rather than old ID33),
input22 addresses absent row22 and returns Numeric0 rather than selecting
ID22, and ordinary signed-negative/zero/missing indices still return Numeric0.
Default invalid/nonfinite/outside converted signed32 throws localized31 before
indexed access/destination assignment; explicit VFP9 keeps both-sign low32
aliases and observed huge/Infinity0. Exact64 never passes through Double;
INT64_MIN+2 returns row2/ID22 only in VFP9. NaN containment is direct-helper
evidence and derived policy, not a public/native NaN syntax observation.

Walked both controls, modes, sessions, ordinary/postfix parentheses/brackets,
four locales, return kinds, error31 sentinel and continuation, current-session
reset/cursor state, and stable RowSource0 count/data/bits/IDs/ListIndex/ListItemID
using frozen400-case public shards, with194 independent direct cases. Existing
six unchanged groups under each compiler preserve selection/ID coherence,
removal, multiselect, sorting and moverbars behavior. These are not arbitrary
callback/materialization-count or whole-call allocation guarantees.

Misuse classification remains medium for this bounded documentation delta:
an unintended ID could misdirect a downstream application action. This READ
slice does not persist data or qualify high-hazard/system-safety operation.
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 remain active at
their existing severity. Recovery guidance is to roll back only this bounded
slice, rerun the frozen neighbors, and notify integrators through changelog
before re-release. Type gap7147, writes/lifecycle, native64, KBX and parent
completion remain separate. This is development self-review, not independent
human review or release qualification. Five documentation contracts and
exact-head hosted checks/clean review remain separate pending gates.

## Documentation contract completion and raw spacing

Documentation30004 harvested0:5/5 PASS335.83s, safety workflow332.07s.
Full86-line contracts.log copied/cmp, SHA256
3fba5c60f619ee3f3cbd675d7c90c4882388a7c7ee919dc4ce22aae60b39babb.
Sign-off self-test, changelog assembler/fragment validity, live-channel contract
and safety-traceability workflow contract all pass. Source/frozen inputs
unchanged. This checkpoint preceded the result-wording update; exact final-head
hosted checks are still gates, not represented by this local checkpoint.

Staged raw spacing diagnostics retained under owned build in
whitespace-raw.log (1192 lines, SHA256
358db1a3bbff7d9471f0ae1cb0a4683654ab986dfd095469fe31e24d1dff5257).
They identify only this fixture's raw .log/.out CTest spacing/native CRLF.
Source/docs/PRG/runner/driver whitespace check passes when ONLY those exact
fixture raw extensions are excluded; not a whole-tree whitespace-pass claim.
Raw bytes, frozen assertions and CI policy are not normalized or modified.
Signed commit/push, post-push exact-head suite, PR and clean exact-head hosted
review plus required checks/resolved conversations remain next gates.
