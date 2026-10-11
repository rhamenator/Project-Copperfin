# Selected read-index requirements recovery

Pre-code mapping recorded 2026-10-11T03:49Z, before any Selected helper,
assertions, test registration or production change. Admitted owner-authored
OPEN/agent-approved #5611 and #6776; base main
7443fed880c1481524fb045223cc066633774111. Only native ComboBox/ListBox
Selected first-argument Numeric/exact row-index READ conversion is selected.
Writes, SelectedID, other list consumers, payloads, type/NULL/Currency/arity,
general grammar and row materialization/lifecycle/allocation are excluded.

## Independent native evidence

Installed Visual FoxPro 09.00.0000.7423 under the existing private Wine prefix
/home/rich/copperfin-wine-vfp9, using unchanged /home/rich/bin/vfp9-probe
(SHA256 1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06).
The runner uses private Xvfb, SCREEN=OFF/RESOURCE=OFF, a TRY/DO/CATCH/QUIT
harness and a bounded owned process group. Actual temporary config/harness
are auto-deleted; the retained runner snapshot describes their generation,
not retained literal files. No VM, desktop interaction, FXP decompilation,
credentials or native extended INT64/UINT64 syntax used. Probe source was
adapted from the observation harness, not old Copperfin expectations.

Two complete 98-line/96-call observations match byte-for-byte:
native-1.out/native-2.out SHA256
de1188fe09751f386646da523b1db4c2573744b7af17c7def8b08b9e3f7dd506.
Two complete 42-line/40-call boundary observations match byte-for-byte:
boundary-1.out/boundary-2.out SHA256
9121677b4af3da963ae69ce0926399557c66e9702a56ca56a374365d559abdc2.
All four runner invocations completed exit0; full raw stdout copies are
retained beside the source as native-{1,2}-runner.log and
boundary-{1,2}-runner.log, byte-identical to the corresponding .out files.
The slice build/selected-native also retains them. Raw CRLF outputs are not normalized.
Both classes agree; count3/ListIndex2/ItemData101,202,303 and Selected bits
.F.,.T.,.F. are unchanged after every call. Admitted results have Logical kind.
1.5/1.9 are False; 2.49/2.5/2.9 are True; 3.5/3.9/4 are False.
Negative sub-unit values are False, -1/-1.9 raise31. Positive raw values above
INT32_MAX, including INT32_MAX+.1/.5/.9, raise31. Negative low32 aliases
-4294967294.5 are True; -4294967295/.9, -4294967293 and -4294967296/.5
are False. -2147483648/.9 and -4294967297/.9 raise31; -2147483649/.9
are False. Negative huge finite/-Infinity are False; positive huge/+Infinity
raise31. +/-1e-300 are False. Unsupported Logical/NULL/Character/Currency
controls raise31 natively, retained as evidence only, not an admission change.
Textual near-integer decimals are not proof of identical binary representation.

Shipped Microsoft Selected Property help, fully read:
/home/rich/temp/vfp9-help-len/html/d90e5b62-769a-4a47-8c6e-d05e4e129ec7.htm
SHA256 2aa9a9b45485d2c11a9f8047f91b5257c35004e84d6d2c01342d4a415ad71d46.
It documents Numeric item index, Logical selection state and both classes;
write/event behavior in the help is explicitly outside this read slice.
Native observations use parentheses only. Copperfin bracket/postfix route
equivalence is derived from the parent's consistent per-session conversion
policy, not a claim of native syntax qualification. Generated probe.FXP and
boundary.FXP were moved to ignored build/selected-native without inspection;
the raw runner-source.txt snapshot retains the unchanged runner bytes.

## Requirement, architecture and verification before code

RQ-CF-PRG-SELECTED-INDEX-NUMERIC-001 derives from owner #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner extension intent 2026-10-06,
native/help evidence and HZ-runtime-crash-01/HZ-data-corruption-01.
Default COPPERFIN: finite Numeric truncation toward zero, admit converted
0..INT32_MAX, otherwise localized catchable31 with no wrap/clamp. Exact signed
and unsigned64 compare without Double. Converted0 or beyond actual count
returns Logical False; an existing row returns its selection bit. Fractions
below2147483648 but aboveINT32_MAX are admitted by default: explicitly derived
consistent truncation, not native raw-ceiling parity.
Explicit VFP9: independently observed positive RAW INT32_MAX ceiling, then
parent-defined signed-low32 conversion, rejecting negative converted indices
with31. Negative huge/-Infinity indefinite0 observed; NaN rejection and exact64
raw comparison/low32 projection are derived containment/extension requirements,
not native syntax evidence. Binary nextafter/subnormal literals are mathematical
boundary derivations, not claims about native decimal literal parsing.
In particular exact INT64_MIN+2 has low32=2 and must return True in VFP9;
this independently derived boundary distinguishes accidental Double rounding
to INT64_MIN/row0 False. It is not a native extended-integer observation.
Rejection precedes indexed lookup, destination assignment and method success/
action publication. Stable RowSource0 fixtures must retain row count/data,
ListIndex and all three selection bits. No whole-call callback/materialization
atomicity claim; existing selection-signature materialization occurs upstream.
No index-sized allocation or iteration is introduced.

Read-only route trace: expression parentheses/brackets first try reflection,
which has no Selected indexed getter; both fall back to the native method
callback with the original PrgValue. Supported expression-result postfix
IIF(.T.,oList,.NULL()).Selected parentheses/brackets also reach that method
with original PrgValue. Therefore only the Selected Numeric/exact method arm
needs migration; no expression/reflection routing edit is planned. Existing
visibility/accessor/metadata ordering, current-session behavior lookup and old
direct-call overloads remain unchanged. Textual GETPEM is not evaluated Numeric
and remains unchanged. The grouped-primary gap #7139 is not addressed.
Distinct checked_selected_index_numeric_argument in helpers.h/.cpp must remain
UNUSED through BOTH complete original GNU and Clang sanitizer runs. Reuse
existing localized Array.Error.SubscriptOutOfRange with explicit31; no catalogs
or unrelated array1234 mapping changes.

VR-5611-SELECTED-INDEX-NUMERIC-001: independently literal both-mode direct
Numeric/NaN/infinity/nextafter/subnormal/exact64 and unsupported adapter guards.
VR-5611-SELECTED-INDEX-NUMERIC-002: fresh both-mode/two-session public cases,
both classes and parentheses/brackets plus supported expression-result postfix;
Logical result/kind, catchable31, destination sentinel, continuation,
context/cursor/mode/reset, four locales, unchanged count/data/selection bits and
no external invocation event. Preserved coercion controls are not native type
parity. Necessary observed Selected type-admission mismatch is separately
reported in [#7142](https://github.com/rhamenator/Project-Copperfin/issues/7142)
against main7443fed88, with no admission label or selected-scope expansion.
Freeze helper/test/registration/isolation hashes BEFORE configure;
both full original raws and counters retained BEFORE migration. Any repair to
frozen input/helper/assertions/configuration requires explicit refreeze and
BOTH originals repeated. Fixed normal/sanitizer neighbors, documentation
contracts, hosted exact-head checks and actual clean review remain gates.

## Documentation assurance

DQ-5611-SELECTED-INDEX-NUMERIC-001 requires explicit modes/range/error/exact,
read/write/native/derived boundaries, reproduction and rollback.
DV-5611-SELECTED-INDEX-NUMERIC-001 requires retained native sources/raws and
bounded fixed walkthrough. Active HZ-runtime-crash-01, HZ-data-corruption-01 and
HZ-doc-command-01 retain registered severity/status; bounded documentation
misuse medium: an aliased selection bit could misdirect an application's later
action, but this slice performs no persistence writes and does not qualify
downstream safety. Procedural delta:1.9 returns row1 False rather than rounded
row2 True; modern invalid negatives/oversized inputs raise31 instead of
accidental lookup/False. Only explicit VFP9 admits negative aliases.
Both-class/route walkthrough:1.9 False,2.9 True,4 False,-1 error31;
-4294967294 modern31/VFP9 True;INT32_MAX+.5 modernFalse/VFP9 31;
+Infinity31,-Infinity modern31/VFP9 False; four locales and preserved context.
The fixed GNU18/18 and Clang sanitizer18/18 runs complete this bounded
automated walkthrough with all190direct/four368public counters and unchanged
neighbor counters. Delegated development maintainer self-review checked the
source-to-requirement delta, native/derived separation, error ordering,
four-route raw-value preservation, scope exclusions, phase/raw/hash chronology
and these results. This is self-review, not independent-human signoff or
release qualification. Documentation contracts5/5 pass at the retained
precommit checkpoint; external exact-head integration remains pending. The full
audit retains the earlier unexplained neighbor timeout.
Rollback: revert the bounded slice, rerun frozen neighbors, warn integrators
through the changelog before re-release. No KBX/hazard/parent/family closure.

## Controlled reproduction and phase ordering

Use a dedicated worktree based on main7443fed88, not a shared checkout. The
five final helper/test/registration/isolation hashes in baseline-audit.md must
match before measurement. For an original-behavior run the production method
hash must be5d7e46f9a3c50879bff25acb08f78bbbf777d7381b9ad9025d9c9c0baa7bd7c7
and the Selected helper must have no production caller. The retained
first-migration.patch records ONLY the subsequent method-arm delta; it is
not permission to revert unrelated files or reset a shared checkout.
Do not edit expectations to make an original run pass. Retain the full raw,
exit status, counters and input hashes before moving to the next phase.

From the dedicated worktree, create private local test storage:

```sh
mkdir -p build/selected-tmp build/selected-native
chmod 700 build/selected-tmp
export TMPDIR="$PWD/build/selected-tmp"
cmake -S . -B build/selected-index-gcc -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g0' \
  -DCOPPERFIN_BUILD_TESTS=ON
nice -n 10 ionice -c 2 -n 7 cmake --build build/selected-index-gcc --target \
  test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric \
  test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior \
  test_prg_engine_int64_exact_expression -j 1
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/selected-index-gcc \
  -V --output-on-failure --timeout 600 -j 1 \
  -R '^(test_prg_engine_(selected_index_numeric|itemdata_index_numeric|getcolor_default_numeric)(_(copperfin|vfp9)_session[12])?|test_prg_engine_numeric_behavior|test_prg_engine_int64_exact_expression|test_native_test_isolation_contract)$'
```

Run Clang separately, never concurrently with the owned GNU test run:

```sh
cmake -S . -B build/selected-index-clang -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug -DCOPPERFIN_BUILD_TESTS=ON \
  -DCMAKE_CXX_FLAGS_DEBUG='-O1 -g0 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer'
export ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
nice -n 10 ionice -c 2 -n 7 cmake --build build/selected-index-clang --target \
  test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric \
  test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior \
  test_prg_engine_int64_exact_expression -j 1
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/selected-index-clang \
  -V --output-on-failure --timeout 600 -j 1 \
  -R '^(test_prg_engine_(selected_index_numeric|itemdata_index_numeric|getcolor_default_numeric)(_(copperfin|vfp9)_session[12])?|test_prg_engine_numeric_behavior|test_prg_engine_int64_exact_expression|test_native_test_isolation_contract)$'
```

The600-second bound is a disclosed local diagnostic budget, not a repository
or CI timeout change, performance requirement or claim about the first
180-second numeric-neighbor timeout. Leak detection is disabled: do not infer
leak qualification. Preserve failure evidence even when a repeat completes.
Only after BOTH full original runs complete and their raws/hashes/counters
are retained may the method-only migration be applied. Fixed runs use the
same frozen inputs; source/helper/test/configuration repairs require explicit
refreeze and BOTH originals repeated. Documentation contracts and exact-head
hosted checks/clean review/resolved conversations still gate integration.

Native reproduction requires the existing installed Wine VFP9 environment
and unchanged retained runner, not a new VM. Both retained PRGs name the owned
build/selected-native output paths on this host; verify the exact output path
before executing, and preserve each output before the next run replaces it.
Use vfp9-probe <program.prg> --result <that exact output path> serially. These
programs write synthetic evidence only. Do not start/recreate local VMs while
cleanup/backup-reset completion is unconfirmed, or copy credentials into logs.

## Fixed evidence and preservation boundary

Both complete final original runs were retained before the method-only
migration. GNU and Clang each produced2144 selected assertion failures and
zero other assertion failures; the earlier Clang numeric-neighbor timeout is
retained separately and not erased by its completed repeat. The same frozen
inputs then pass GNU18/18/152.27s and Clang18/18/219.05s, with no fixed sanitizer
reports (leaks disabled). Full unedited logs, hashes, counters and exact ordering
are in baseline-audit.md; no helper or assertion inferred its own expectation.
Only the Selected Numeric/exact read arm changed. Existing non-Numeric
coercions and write paths remain deliberately unchanged, not native parity.

preservation-driver.cpp.txt calls six unchanged list_controls_03.cpp groups
against the corresponding fixed runtime, covering existing Selected writes,
selection modes, IDs, row removal/sorting and MoverBars. It adds no assertions
and is not another production or frozen-input edit. Compile with the compiler,
flags and includes above plus -Iinclude -Itests -Isrc/runtime, treating the
driver as C++ with -x c++ then restoring -x none for the existing .cpp/object/
archives. Link prg_engine_test_support.cpp.o and the cf_xbase_runtime,
cf_design_model, cf_prg_analysis, cf_runtime_text, cf_vfp_assets,
cf_platform_profile, cf_security, cf_localization and cf_platform_support
archives from the same build; run serially with COPPERFIN_LOCALE_DIR pointing
at resources/locales and the same private TMP/sanitizer environment.
The full runtime-surface regression remains a hosted exact-head gate.

Documentation contracts: contributor signoff, changelog assembler/validation,
live-channel and safety-traceability workflow all pass in one5/5 invocation
(334.99s), full raw/cmp/hash retained in the audit. Reproduce with the same
private TMP and serial scheduling using ctest -V --output-on-failure
--timeout 1200 -j 1 and the exact five names in documentation-contracts.log.
Results/handoff text appended afterward is not an exact-final-head rerun;
hosted checks and actual clean exact-head review still gate merge.
