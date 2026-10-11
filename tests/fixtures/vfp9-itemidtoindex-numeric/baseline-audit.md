# ItemIDToIndex phase audit

Initial freeze2026-10-11T10:25:48Z BEFORE configure/build/measurement.
Pre-code map README10:22Z and docs/22/docs/32 precede helper/assertions/config.
Independent matching302-line/300-call native pair retained; no code-derived
requirements. Base13281678def7e8f77fcd8d7385abe09600697bf2.
Frozen248 direct,944 public in EACH of four mode/session shards; independent
71 Numeric/22 exact/12 postfix literals, sparse/dense classes, locales and
preserved unsupported coercions. Counts are fixed, not inferred from failures.

Five frozen files:

```text
a6d31e138b6b5760d21997f5665b22a95cadb653cab80523b1d8dfe730a0ae84  src/runtime/prg_engine_helpers.h
617c6e0035b3432bee001750f73002d5076090e83a00d678b71fd5dd57a025df  src/runtime/prg_engine_helpers.cpp
c39df3cfc8a663672626a1970756cecd8d24cde28843897f048091a1c8432d7a  tests/test_prg_engine_itemidtoindex_numeric.cpp
e05bb31b1054ff2fe621698c01e1d1be7f152af97e6ba7e295b05151ad9ed97e  tests/CMakeLists.txt
2c7670a92cc4a151e8d0bdcce553e37e0ec5f7f20e7b9f1028fa61108ea86282  tests/CopperfinTestIsolation.cmake
```

Production methods/reflection/expression/functions-declarations unchanged:

```text
f4bd327f04da4679b4b6988a504203791e8171c707d1d31d51e9860a34a536f3  src/runtime/prg_engine_runtime_surface_object_methods.inl
c36664aad68824488b2b6bd4ce2d86a7c8e9235fbfd1424941696e65b076e930  src/runtime/prg_engine_native_reflection_dispatch.inl
6ff99e261fbfbf9671223696556cb89dbac33ea182857dfadef6d4a785d90fb0  src/runtime/prg_engine_expression.inl
ecba9528692224eb419e36b711fcfa8943ea5d545df56d4e1f890b438c970b23  src/runtime/prg_engine_runtime_surface_functions.h
```

Distinct helper UNUSED by production through BOTH complete originals.
No production read-adapter/configure/build has happened at this freeze.
Any frozen repair requires explicit refreeze and BOTH originals repeated.
Original full34 subjects:5new +5IndexToItemID +5SelectedID +5Selected +5ItemData
+5GETCOLOR +numeric/exact/isolation/existing itemid selectors. Preserve every
raw/status/hash/counter and compile warning BEFORE migration. Local600s bound
is diagnostic, not CI/performance policy; private700 TMP, nice10/ionice2:7/-j1,
toolchains serial. ASan leaks disabled; not leak/idle-host qualification.

GNU reproduction:

```sh
export TMPDIR="$PWD/build/itemidtoindex-tmp"
cmake -S . -B build/itemidtoindex-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug \
 -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g0' -DCOPPERFIN_BUILD_TESTS=ON
nice -n 10 ionice -c 2 -n 7 cmake --build build/itemidtoindex-gcc -j 1 --target \
 test_prg_engine_itemidtoindex_numeric test_prg_engine_indextoitemid_numeric \
 test_prg_engine_selectedid_numeric test_prg_engine_selected_index_numeric \
 test_prg_engine_itemdata_index_numeric test_prg_engine_getcolor_default_numeric \
 test_prg_engine_numeric_behavior test_prg_engine_int64_exact_expression \
 test_prg_engine_runtime_surface_itemid_selectors
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/itemidtoindex-gcc -V \
 --output-on-failure --timeout 600 -j 1 \
 -R '^(test_prg_engine_(itemidtoindex_numeric|indextoitemid_numeric|selectedid_numeric|selected_index_numeric|itemdata_index_numeric|getcolor_default_numeric)(_(copperfin|vfp9)_session[12])?|test_prg_engine_numeric_behavior|test_prg_engine_int64_exact_expression|test_native_test_isolation_contract|test_prg_engine_runtime_surface_itemid_selectors)$'
```

Clang separately, only AFTER GNU finishes:

```sh
export TMPDIR="$PWD/build/itemidtoindex-tmp"
cmake -S . -B build/itemidtoindex-clang -G Ninja -DCMAKE_C_COMPILER=clang \
 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DCOPPERFIN_BUILD_TESTS=ON \
 -DCMAKE_CXX_FLAGS_DEBUG='-O1 -g0 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer'
export ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
nice -n 10 ionice -c 2 -n 7 cmake --build build/itemidtoindex-clang -j 1 --target \
 test_prg_engine_itemidtoindex_numeric test_prg_engine_indextoitemid_numeric \
 test_prg_engine_selectedid_numeric test_prg_engine_selected_index_numeric \
 test_prg_engine_itemdata_index_numeric test_prg_engine_getcolor_default_numeric \
 test_prg_engine_numeric_behavior test_prg_engine_int64_exact_expression \
 test_prg_engine_runtime_surface_itemid_selectors
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/itemidtoindex-clang -V \
 --output-on-failure --timeout 600 -j 1 \
 -R '^(test_prg_engine_(itemidtoindex_numeric|indextoitemid_numeric|selectedid_numeric|selected_index_numeric|itemdata_index_numeric|getcolor_default_numeric)(_(copperfin|vfp9)_session[12])?|test_prg_engine_numeric_behavior|test_prg_engine_int64_exact_expression|test_native_test_isolation_contract|test_prg_engine_runtime_surface_itemid_selectors)$'
```

Both originals, full outcomes and counters, production migration, fixed runs,
preservation, DQ/DV completion/contracts and exact-head integration are pending.

## Explicit harness-only refreeze

2026-10-11T10:29:35Z: compile-only GNU syntax check failed1 because unqualified
lowercase_copy is exported by both runtime and test-support namespaces.
Full initial-syntax.log retained, SHA256
46fe00c7d0a246af4f6b5d7dc986253b90c47e29aef3a279a937132d19142500.
In-progress GNU build79420 was stopped only after exact owned cwd/PGID/command
verification (4142459); harvested143, no original CTest executed/accepted.
Full initial configure/interrupted compile and test-source snapshot retained
in owned build; interrupted compile SHA256
975401a49715853cee044609ab706614205d5febb364a3f6a3770cf97d78a7cd.
No unrelated process, CI run or watch was stopped.

Only that call now qualifies copperfin::runtime::lowercase_copy; no input,
expectation, assertion, count, helper, registration, isolation or production
change. Refrozen compile-only syntax check82862 passes0; full raw retained.
Final test-source hash superseding the initial hash above:
d96a169fdf428b44ecd36d02b6989262e6b625966507bb82a8efbbad5b6d776f.
All other four frozen hashes and all four base production hashes unchanged.
BOTH full34-subject originals must now execute on this exact refrozen source,
GNU first then Clang, with every raw/status/counter/hash retained before any
production migration. Initial incomplete build is not an original-suite pass.

## Refrozen GNU original result

Status file written2026-10-11T10:41:46Z; session16321 harvested exit8 this turn.
All34 subjects completed:
30/34 PASS345.32s; only four new public ItemIDToIndex shards fail as expected.
Actual prefixed failure assertions: COPPERFIN1268/session, VFP9 220/session;
CTest repeats failed output, so unprefixed duplicate text is not counted.
Direct248 and EACHfour944 public counters complete. All neighbor counters
complete: IndexToItemID194/four400, SelectedID200/four476, Selected190/four368,
ItemData188/four368, GETCOLOR174/four80; numeric/exact/isolation/existing
itemid selectors pass. No expectation repair/refreeze or production migration.
All five final frozen hashes reverified; four production files unchanged.
Full6364-line original-gcc.log SHA256
ceccb5070da1d17fa523d7d61ffceb9ad47bd6866cc1c4d4be109ca234060b41.
Status8 retained original-gcc.status; full configure/compile retained:
configure29501d2fe3d88acd8ce719fe143a3d6ee5c6861a1276f40d289e96e33bd63b30,
compileb3af47f3ea2c8f13410519e76b7bfadd0549d544ed15d6c11cec7399ae86de7a.
Existing dispatch/platform/string/sibling aggregate warnings retained, not a
warning-free claim. No sanitizer qualification from this ordinary GNU run.

Serial original Clang configure/build/full34 launched after GNU harvest as
session97906 at2026-10-11T10:49:37Z (shell4157245/cmake4157388).
Full configure-clang.log/compile-clang.log and
eventual original-clang.log/.status in owned build/itemidtoindex-native.
The helper remains UNUSED; BOTH complete originals must be retained before
production migration. Do not duplicate the active build or weaken expectations.

## Both complete originals retained before migration

Clang status written2026-10-11T11:13:06Z; session97906 harvested8 this turn.
All34 subjects completed:
30/34 PASS624.15s; only the four new public ItemIDToIndex shards fail.
The same actual prefixed assertions as GNU:1268/COPPERFIN session and
220/VFP9 session, not duplicate unprefixed CTest output. Every frozen
248/four944 new counter and 194/four400,200/four476,190/four368,188/four368,
174/four80 neighbor counter completed. All other subjects pass; no sanitizer
diagnostic in the full original run. No leak/performance/warning-free claim.
Full6364-line original-clang.log SHA256
ee52f77166528350b8579de0336d028572d4f7e3f4d42a376e75bff430094b80;
actualstatus8 retained. Full compile warnings/configure retained:
compile552f7796b8479a420bb6bd1cc39256543d5de2f7851a2719adaf4223280469af,
configureae74d22417df8e189b65feb7e3722464bb7b6341ddfea9e394b79681c2542f07.
Five final frozen hashes reverified; method/reflection/expression/declarations
still byte-identical base and checked_itemidtoindex_numeric_argument UNUSED.
No new refreeze or expectation weakening. Both complete originals are now
retained BEFORE the bounded production migration, fixed suites and contracts.

## Bounded production migration

Only after both originals above: Numeric/exact read adapter declared in
runtime_surface_functions.h, defined in object_methods.inl and called from
the existing ItemIDToIndex method arm and raw-selector reflection arm.
Current session's numericbehavior map determines the mode. Checked conversion
is followed by existing bounded item-ID lookup and one-based Numeric result;
invalid conversion raises the existing localized catchable31. Unsupported
types return nullopt to the unchanged legacy path; empty arguments retain0.
No argument-sized allocation or iteration is introduced. Expression parser,
textual GETPEM, writes, ID creation, other methods and unsupported coercions
are unchanged. Entry materialization and method's second materialization are
retained for typed calls; raw property-first nonpositive reads may avoid the
old method fallback's additional materialization. Stable RowSource0 controls
do not qualify arbitrary callbacks/materialization-count or whole-call atomicity.
Initial raw-selector draft's nonexistent SET-state member was corrected to
the existing map lookup before any fixed configure/build/test; no frozen
source or expectation was changed and neither original was rewritten.

Production SHA-256 after migration:
8bb105fcd8c8755f73f3b248df95922e041b6b99ba0b3dd914882c36dddadaec object_methods.inl;
9dd380fa415cf12d8b5a01d10f82a4a9bf10ad7e5b5f0d0eb8eeb6a2369a5c83 native_reflection_dispatch.inl;
324b4ffed0b3b277676c7d8327aafe65698bee53256264b08209f9ba6c971e41 runtime_surface_functions.h.
Expression remains base6ff99e261fbfbf9671223696556cb89dbac33ea182857dfadef6d4a785d90fb0;
five frozen hashes unchanged. Same34-subject fixed GNU build/run session69279
is active; serial fixed Clang and preservation/contracts/integration follow.
Separately filed unsupported-type gap#7149 against exact current main;
no type-admission change or label-based implementation admission.

## Fixed GNU and preservation checkpoint

Session69279 harvested0; statuswritten2026-10-11T11:24:08Z. Same34 subjects
PASS348.49s, zero assertions, all248/four944 own and every unchanged neighbor
counter complete. Full401-line fixed-gcc.log copied/cmp into this fixture,
SHA2566c2552a6b70833a7b37d65796ffe0408bc7dc4c69c8b7e02faab9b581004e96c;
full compile-fixed-gcc.log retained, warnings not removed or claimed absent.
GNU preservation47571 harvested0, status11:24:29Z: six unchanged groups/zero
failures; full one-line raw copied/cmp, SHA256
befec198fb62d7e5fdae2e816aff3dc0ebde67970c187f3c7c013a1a377e3f19.
The verification-only driver calls existing list_controls_03 assertions;
that source remains byte-identical base, not a recovered requirement oracle.
Five frozen hashes and unchanged expression parser reverified afterward.

Serial fixed Clang build/same34 subjects now session44350, launched after
GNU/preservation harvest at2026-10-11T11:24Z. Full compile-fixed-clang.log and
eventual fixed-clang.log/.status retained in owned build/itemidtoindex-native.
Same frozen sanitizers/privateTMP/serial/local600s bound, no CI policy change.
Clang full results, Clang preservation, final documentation walkthrough/five
contracts, signed commit/push/postpush verification and exact-head hosted
Claude-FIRST/authorized Codex review/required-check/resolved-thread gates remain.

Preservation reproduction after each matching fixed compiler suite:

```sh
export TMPDIR="$PWD/build/itemidtoindex-tmp"
export COPPERFIN_LOCALE_DIR="$PWD/resources/locales"
nice -n 10 ionice -c 2 -n 7 g++ -std=c++20 -O0 -g0 -Iinclude -Itests -Isrc/runtime \
 -x c++ tests/fixtures/vfp9-itemidtoindex-numeric/preservation-driver.cpp.txt \
 tests/test_prg_engine_runtime_surface_functions_list_controls_03.cpp -x none \
 build/itemidtoindex-gcc/tests/CMakeFiles/cf_prg_engine_test_support.dir/prg_engine_test_support.cpp.o \
 build/itemidtoindex-gcc/libcf_xbase_runtime.a \
 build/itemidtoindex-gcc/libcf_design_model.a build/itemidtoindex-gcc/libcf_prg_analysis.a \
 build/itemidtoindex-gcc/libcf_runtime_text.a build/itemidtoindex-gcc/libcf_vfp_assets.a \
 build/itemidtoindex-gcc/libcf_platform_profile.a build/itemidtoindex-gcc/libcf_security.a \
 build/itemidtoindex-gcc/libcf_localization.a build/itemidtoindex-gcc/libcf_platform_support.a \
 -o build/itemidtoindex-native/preservation-gcc
nice -n 10 ionice -c 2 -n 7 build/itemidtoindex-native/preservation-gcc
```

Clang substitutes clang++, itemidtoindex-clang archives and preservation-clang,
adds -O1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer,
and the same ASAN_OPTIONS/UBSAN_OPTIONS from its frozen reproduction above.
No simultaneous toolchains. Documentation selection (existing five subjects):

```sh
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/itemidtoindex-gcc -V \
 --output-on-failure --timeout 1200 -j 1 \
 -R '^(test_contributor_signoff_contract|test_changelog_fragment_assembler|test_changelog_fragments_valid|test_agent_channel_contract|test_safety_traceability_workflow_contract)$'
```

Full originals/configure/compile and initial/refrozen harness evidence now
copied into this fixture without replacing raw bytes; hashes verified.
Changelog206fragments valid at the intermediate checkpoint, not all contracts.

## Fixed Clang and final development walkthrough

Session44350 harvested0; statuswritten2026-10-11T11:39:24Z. Identical34 subjects
PASS627.52s. Every248/four944 own and unchanged194/four400,200/four476,
190/four368,188/four368,174/four80 neighbor counter completes; all otherfour
subjects pass, zero failed assertions/sanitizer diagnostics. Full401-line
fixed-clang.log copied/cmp without normalization, SHA256
cf598fc092f80c1de025c0af0217a0fbce91d09cec8510564145f8271495e2c5.
Full compile-fixed-clang.log warnings retained, SHA256
ea81e69ec2817f9e7153d0c14c536b0bbe11ad0790fd5a01032a48f1c808db47.
Clang preservation65843 harvested0: six unchanged groups/zero failures,
rawSHA256befec198fb62d7e5fdae2e816aff3dc0ebde67970c187f3c7c013a1a377e3f19.
Matching compile/raw/status retained; five frozen files and expression unchanged.
README records the completed boundedmedium DQ/DV delegated development
self-review/proceduraldelta/walkthrough and unchanged hazard boundaries.
Not independent-human, leak, warning-free, performance or release qualification.
Five documentation contracts and exact-head external integration remain gates.

Raw evidence spacing retained: staged whole-tree whitespace diagnostics are
in owned build/raw-spacing.log, not a whole-tree whitespace-pass claim.
Source/docs check passes excluding only this fixture's raw.log/.out and the
exact initial-test-snapshot.cpp.txt (frozen initialhashc39df3... above), whose
original blank final line remains intact. No source, frozen input, raw byte,
expectation or CI policy was changed to normalize historical evidence.

## Documentation contracts complete

Session19640 harvested0: identical five-subject selection PASS334.24s,
including206 valid fragments and the full safety-workflow negative-case contract.
Full86-line contracts.log retained, SHA256
de3f22191b6b3d235295d843887784f8c9897ba091da0437c5681d1b7c9b642f.
Development walkthrough/self-review complete; exact-head signed postpush,
Claude-FIRST/authorized Codex review/all11required/resolvedthreads remain gates.
