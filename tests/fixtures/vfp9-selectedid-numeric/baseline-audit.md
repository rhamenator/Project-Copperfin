# SelectedID baseline and phase audit

Admitted #5611/#6776; selected Numeric/exact item-ID READ only. Base origin/main
2cec530ddeb06abf70c7c30ffc83e8c04264c456. Pre-code RQ/VR/DQ/DV/HZ map in
README.md recorded06:18Z before helper/assertions/registration/isolation.
Four independent installed VFP9 paired probes complete, matching each pair;
full sources/raws and runner snapshot retained, help provenance in README.
The sparse2/2.9 False expectation was first derived from shipped item-ID
addressing and the sparse94-call observation; the extra identity pair completed
during preparatory test authoring and before the first freeze. It independently
confirmed the mapped expectation, without changing any test literal.
No VM/decompilation/credentials or native extended-integer syntax claim.
Preparatory orchestration stopped before helper copy because an incomplete
read slice failed its completion guard; no configure/build/measurement had
started, no accepted frozen inputs were altered or original evidence claimed.

Initial freeze2026-10-11T06:20:34Z BEFORE configure/build. New tests have200
direct cases and476 cases in EACH of four fresh public mode/session shards.
They retain literal dense-ID boundaries plus sparse11,22,33 identity and
distinguishing exact INT64_MIN+22 alias22True. Expected results come from
independent native/help/parent mathematics, never current helper output.
New distinct helper is UNUSED by production, old selectedid llround unchanged.
Five frozen inputs:

```text
dbb177054893569445e1d915a04539186e4d8095d68751fbe0e5ccf18b0dee31  src/runtime/prg_engine_helpers.h
9e74539b88c3648f1f18d4dd54c73715ea664b39010bb2befbaae054355c67a9  src/runtime/prg_engine_helpers.cpp
3e85b50c1c04cbdfd3828f25ca5eb0ae68faf6f04c2f94586c2cffb03f801afb  tests/test_prg_engine_selectedid_numeric.cpp
f3959ce2161c4e8201bad9098e963131f7f33856f09932819ee8bd9f8fe5120b  tests/CMakeLists.txt
72f468c3a475fbfe221bde1e934f5b5d0dac895da0f0eabaee088b357c99a313  tests/CopperfinTestIsolation.cmake
```

Original production/routes, unchanged base bytes:

```text
c44c95a601b1dc92edc22ae2972a7c6c9b57b844e9305e183e9849305b2ba818  src/runtime/prg_engine_runtime_surface_object_methods.inl
6ff99e261fbfbf9671223696556cb89dbac33ea182857dfadef6d4a785d90fb0  src/runtime/prg_engine_expression.inl
01a7369bdb0018e83fa55280da8787127e6c5457758c81e0e73adcb8238b6cb1  src/runtime/prg_engine.cpp
2d27323de5f777bf161d45d6a4acb693e8625a4c6f0dabbeb8fb8a6f56890937  src/runtime/prg_engine_runtime_surface_dispatch_object.inl
```

Neighbor Selected/ItemData/GETCOLOR tests and production untouched. Full23 CTest
subjects: five new SelectedID, five Selected, five ItemData, five GETCOLOR,
numeric behavior, exact-expression and native isolation. Each raw must include
all counters and final status; retain failure as well as success. First run is
not acceptance until full completion and counters are verified. Any frozen
helper/test/registration/isolation repair requires explicit refreeze and BOTH
full originals repeat before migration; no patched expected-pass shortcut.

Controlled original measurement in this dedicated WT:

```sh
mkdir -p build/selectedid-native build/selectedid-tmp
chmod 700 build/selectedid-tmp
export TMPDIR="$PWD/build/selectedid-tmp"
cmake -S . -B build/selectedid-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g0' -DCOPPERFIN_BUILD_TESTS=ON
nice -n 10 ionice -c 2 -n 7 cmake --build build/selectedid-gcc -j 1 --target \
  test_prg_engine_selectedid_numeric test_prg_engine_selected_index_numeric \
  test_prg_engine_itemdata_index_numeric test_prg_engine_getcolor_default_numeric \
  test_prg_engine_numeric_behavior test_prg_engine_int64_exact_expression
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/selectedid-gcc -V \
  --output-on-failure --timeout 600 -j 1 \
  -R '^(test_prg_engine_(selectedid_numeric|selected_index_numeric|itemdata_index_numeric|getcolor_default_numeric)(_(copperfin|vfp9)_session[12])?|test_prg_engine_numeric_behavior|test_prg_engine_int64_exact_expression|test_native_test_isolation_contract)$'
```

Clang separately, never concurrent with the owned GNU measurement:

```sh
export TMPDIR="$PWD/build/selectedid-tmp"
cmake -S . -B build/selectedid-clang -G Ninja -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DCOPPERFIN_BUILD_TESTS=ON \
  -DCMAKE_CXX_FLAGS_DEBUG='-O1 -g0 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer'
export ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
nice -n 10 ionice -c 2 -n 7 cmake --build build/selectedid-clang -j 1 --target \
  test_prg_engine_selectedid_numeric test_prg_engine_selected_index_numeric \
  test_prg_engine_itemdata_index_numeric test_prg_engine_getcolor_default_numeric \
  test_prg_engine_numeric_behavior test_prg_engine_int64_exact_expression
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/selectedid-clang -V \
  --output-on-failure --timeout 600 -j 1 \
  -R '^(test_prg_engine_(selectedid_numeric|selected_index_numeric|itemdata_index_numeric|getcolor_default_numeric)(_(copperfin|vfp9)_session[12])?|test_prg_engine_numeric_behavior|test_prg_engine_int64_exact_expression|test_native_test_isolation_contract)$'
```

600s is a disclosed LOCAL diagnostic bound, not a source/CI timeout policy or
performance requirement. Leaks disabled; no leak qualification. Preserve full
configure/compile warnings, not warning-free or idle-host claims. Fixed runs
use exactly the same frozen inputs after both completed originals have been
retained. Requirements matrix, bounded documentation walkthrough/self-review,
contracts and actual exact-head external review/check gates remain unfinished.

GNU original job26873 harvested exit8 at06:34:49Z:19/23 PASS276.94s,
all200/four476 SelectedID, Selected190/four368, ItemData188/four368 and
GETCOLOR174/four80 counters complete. Numeric behavior PASS169.13s,
exact-expression/isolation and all existing neighbors pass. Only four new
SelectedID public shards fail. Full5651-line original-gcc.log copied/cmp BEFORE
Clang configure, SHA256
cacd12ee88aaa0008dd17e57e386720f74e8e672e89bba2cdf2607f0bafd3e0c.
Raw FAIL diagnostic lines5360, including output-on-failure repetition; prefixed
per-shard counts820/820/520/520, zero other FAIL lines. These are raw diagnostic
counts, not an invented unique-assertion count. Frozen five/base method hashes
unchanged, helper UNUSED. Full configure/compile warnings retained in owned
build/selectedid-native. Original Clang job50450 now running same frozen inputs,
ASan/UBSan/float-cast-overflow/halt1/leaksdisabled/serial/privateTMP/600sLOCAL;
no completed Clang baseline or production migration claimed yet.

## Both complete originals retained before migration

Clang50450 harvested8; full5651-line original-clang.log copied/cmp at
2026-10-11T06:52:37Z BEFORE any production migration. SHA256
e35c0afae0de4ea244a93725db28b1f57c22d624748a853cb6da757dc9a373ea.
19/23 PASS306.75s, numeric behavior135.99s, all200direct/four476public
SelectedID, Selected190/four368, ItemData188/four368 and GETCOLOR174/four80
counters complete. Only four new SelectedID public shards fail: rawFAIL5360
(output-on-failure repetition included), prefixed820/820/520/520, zero other
FAIL lines. No ASan/UBSan/float-cast-overflow report; leaks disabled, not leak
qualification. GNU original above already fully retained before Clang configure.

All five frozen inputs and four base production route hashes above rechecked
unchanged at this checkpoint. rg finds only helper declaration/definition in
src/runtime: distinct helper still UNUSED. Durable docs/32/docs/22/hazard
mapping and verification-only preservation driver added while original Clang
was running; no helper/assertion/input/registration/isolation repair/refreeze.
Production remains original until this completed gate has been retained.
Proceed only with the SelectedID method-arm Numeric/exact gate. Same frozen
full23 fixed GNU/Clang runs, preservation, documentation and exact-head external
review/check/conversation gates remain outstanding.

First production migration2026-10-11T06:53Z AFTER the full original Clang
copy/cmp/hash/counter inspection: only SelectedID's non-NULL Numeric/int64/
uint64 READ arm uses the frozen helper and localized catchable31 on rejection.
Existing selected-size sync, other coercions, zero/missing False, item-ID lookup,
writes and other routes unchanged. Method SHA256
7693363db8c27e3bbaf394a955ad214cea61a3f9f35d801c2d30e7d04bffd792.
All five frozen inputs and three other base route hashes unchanged.
Fixed GNU66824 launched same23 subjects/serial/privateTMP/600sLOCAL, full raw
build/selectedid-native/fixed-gcc.log. Fixed Clang/preservation/contracts and
integration remain gates, no fixed acceptance yet.

Fixed GNU66824 harvested0;23/23 PASS189.68s, numeric82.98s. All200direct/
four476public SelectedID and unchanged Selected190/four368, ItemData188/four368,
GETCOLOR174/four80 counters complete; zero assertion failures. Full280-line
fixed-gcc.log copied/cmp2026-10-11T06:56:42Z BEFORE fixed Clang build, SHA256
9da037394ab5a6a7bd5cee0fb7e00488a5b073f8ead64b4db933a3276ab6656a.
Five frozen inputs and method hash unchanged, no test/helper/config repair.
Fixed Clang33541 launched same23 subjects/serial/privateTMP/600sLOCAL/
ASan+UBSan+float-cast-overflow/halt1/leaksdisabled, full compile/fixed-clang.log
in owned build/selectedid-native. No fixed Clang/preservation/contracts pass
or exact-head integration acceptance claimed yet.

Fixed Clang33541 harvested0;23/23 PASS305.13s, numeric140.55s. All200direct/
four476public SelectedID, Selected190/four368, ItemData188/four368 and
GETCOLOR174/four80 counters complete. Zero assertions and no ASan/UBSan/
float-cast-overflow report; leaks disabled. Full280-line fixed-clang.log
copied/cmp2026-10-11T07:02:16Z, SHA256
e72fdc4a513e50024d371932468e64718cc3c6147f21e98bec92e3ab91c61c13.
All five frozen inputs/method hash/three other base routes unchanged;
git diff --check passes, no frozen repair/refreeze. Both fixed23-subject runs
are complete; six unchanged preservation groups/documentation/integration
remain. Each compiler's preservation compile/run uses its matching archives
and existing support object, same flags/privateTMP/locale/sanitizer environment,
separate full raw logs, serial GNU then Clang; no new assertions or type-policy
change, and no full runtime-surface/whole-call allocation qualification.

Preservation76289 harvested0 at07:04Z: GNU then Clang each completed all six
unchanged list_controls_03 groups, zero failures/no sanitizer report. Full
one-line preservation-gcc.log and preservation-clang.log copied/cmp, each
SHA256befec198fb62d7e5fdae2e816aff3dc0ebde67970c187f3c7c013a1a377e3f19.
Driver SHA256eb6755f2e458ecfc185c7675e1c20c0c55ef226b6b200e6505678405aac5fc38;
unchanged existing source SHA256
ad8f0df914ceb8f448087625f313873050e7d29a9b8ae1de57eb59adf578f021.
git diff confirms no assertion source edit; separate compiler warnings remain
in owned build logs, not a warning-free or whole-runtime-surface claim.

DQ/DV bounded medium delegated development maintainer self-review completed:
checked allowed-source/native-derived separation, sparse item identity versus
row position, exact64 without Double, four raw-selector/session routes,
localized31 before lookup/destination assignment/method-success publication,
preserved upstream sync/materialization/write/type boundary, full original/
fixed phase/raw/hash/counter retention and controlled rollback/reproduction.
Fixed frozen walkthrough verifies sparse22.9 True/2.9 False/negative alias22
modern31/VFP9 True and distinguishing exact INT64_MIN+22; dense1.9 False/
2.9 True/INT32_MAX+.5 modernFalse/VFP9 31 across both controls/routes/sessions,
all four locales and stable count/data/bits/IDs/ListIndex/ListItemID. Existing
six preservation groups also pass both builds. Not independent-human or
high-hazard/release signoff. Documentation contracts and signed-head external
integration gates remain.

Documentation55855 harvested0 at2026-10-11T07:10:32Z: all five contributor-
signoff/changelog-assembler/fragments-valid/channel/safety-workflow contracts
PASS in one5/5 invocation330.86s. Full86-line documentation-contracts.log
copied/cmp, SHA256
f13fab7c53cdf14d53510b93f8db781ad9ec193c93e2287f158d01cea7f014ac.
All five frozen inputs/migration unchanged, no contract/source repair.
Checkpoint matrix SHA25691b7cb5a7a7a840d2631c7c492964e642f3cf13612d046350f9cbac7c75390d6,
README71f164c0f29f8bb5037e38b7e9a2baee877e3be535bfa4a57b9612cdcacaa2b8
and audit8c7761bb4488089597cab05afea2a8948a22cb852b583f796a8ed96fba9aa69f
were unchanged throughout this run. Completion/handoff wording appended
afterward is not an exact-final-head contract rerun. Exact signed-head hosted
checks, actual clean Claude-FIRST/authorized fallback review and resolved
conversations remain integration gates; no family/release/parent closure.

Staging exposed retained raw CRLF and CTest `Environment variables: ` spacing
to git's whitespace checker; earlier unstaged diff checks did not include those
then-untracked fixture files. Initial local signed/DCO GoodG head
7c4b3e6d3b52fca0573a80a2bfefe71aca6f37a4 was not pushed. Full branch whitespace
check exits2; retained2258-line build/selectedid-native/raw-whitespace-check.log
SHA2564a0074e915f6c76394fb832a6facd125d2d31806be4d3e9bcd72bade128bd77f.
Every diagnostic is confined to the exact fixture's raw .log/.out evidence.
Source/docs/PRGs/runner/driver check passes with only those raw patterns
excluded; raw bytes/hashes are deliberately preserved, not silently trimmed.
No source/frozen input/CI policy repair or refreeze. This provenance-only audit
and continuity wording are amended before the first push; actual signed-head
verification/integration must bind the resulting head, not the local predecessor.
