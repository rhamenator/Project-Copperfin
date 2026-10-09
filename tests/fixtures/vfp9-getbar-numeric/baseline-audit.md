# GETBAR frozen verification configuration

RQ-CF-PRG-GETBAR-NUMERIC-001 / VR-5611-GETBAR-NUMERIC-001/002.
Requirement mapped from allowed evidence BEFORE helper/callback migration.
At the freeze point, base7d5bbbac5e7096e5132792197a1144dc3dbaae1c's original
GETBAR callback used llround. Added unused helper/test scaffolding did not
migrate it. Completed original/fixed results are recorded below.

Frozen before first GNU configuration/build:

| Input | SHA-256 |
| --- | --- |
| Original src/runtime/prg_engine.cpp | 62e83e8503787deff146e4464eb7a4bb25243832e2f7fb23d1bc0d705794e6b9 |
| src/runtime/prg_engine_helpers.cpp | f6e3291ad25ebd1ddd5407cd51f8e49e92b47a024bd5e6bc74c4af04f5f150fc |
| src/runtime/prg_engine_helpers.h | f1a4bff28becbcb9e578ea17b1e295482d8ae15612828f14e420cad84ff51d96 |
| tests/test_prg_engine_getbar_numeric.cpp | 1542f340fd281ae89a25e9ebff545663456f4f0fa8faf323e9e76245783ce718 |
| tests/test_prg_engine_getbar_neighbors.cpp | 49e55b58f82ef2cec1d4af9a101ec752341b1062f863b24e9efbfda73e048da1 |
| tests/CMakeLists.txt | df74d91f7e19569eb9ecfb381a9ad9a9553e2ecb28224f2546319caea20b1455 |
| tests/CopperfinTestIsolation.cmake | 0d09e237ff499b86cf44c97a13a06a9bb172533651e38208736bdff8800692af |

112 direct checks (74 Numeric,6 nonfinite,6 adjacent boundaries,20 exact64,
6 other coercions);468 guarded public queries (37rows across both modes,
two disabled patterns, three known-key seeds plus opposite-origin session2
alias controls);148 inactive singleton/sparse cases; three unchanged neighbors.
Expected bar identities derive from final independent native layouts and
parent policy, not the helper or correlated setter. Callback fixture matches
native aligned5-bar layout4/13/47/100/MAX; repeated registered13/47/MAX actions,
mark only47 and independent skip flags verify no query mutation. Explicit
cursor/session/mode/count/prompts/release cleanup and old#4630 test-wait reap.
Inactive1/3-bar layouts retain first/last endpoints/count/missing-popup zero
and mode/release controls. Native ordering counterexample remains#6152.

Normal GNU15.2/Ninja/Debug/-g0 build owned at
/home/rich/temp/copperfin-getbar-5611-build; testsON/fuzz and robustnessOFF.
Compile one job; original numeric/neighbors tests serial/private mktemp TMPDIR
with180s per-test cap. Only after GNU original completes, separately build
Clang21.1.8 in copperfin-getbar-5611-sanitize, Debug/-O1/-g0,
ASan/UBSan/float-cast-overflow/frame pointers/matching link flags; one job,
same serial test inputs and cap. ASAN_OPTIONS=detect_leaks=0:abort_on_error=1,
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. No VM/overlapping build/test.
Retain all original failures/build diagnostics/source/test/log hashes BEFORE
migrating callback; no assertion/helper/CMake changes between original/fixed.
Any necessary repair must be documented and both originals repeated.
Fixed focused/shared-query/older Numeric/localization/repository contract and
sanitizer evidence was recorded only when complete below. The configuration
alone is not acceptance, platform/leak/release qualification or
independent review. All exact-head hosted gates still apply.

## Original GNU baseline before callback migration

One-job GNU15.2 build completed successfully; stdout build log SHA-256
7656172c2573a00a97202a4cc30e944d6158fb91ae1425f01fd4b062da18f33f.
Existing compiler warnings outside the selected callback were emitted on
stderr (TemporaryAppendSource missing initializer and unused Currency parser);
that stdout log is not a full compiler-diagnostic archive. No runtime or
sanitizer diagnostic claim is inferred from compiler warnings.
Same frozen inputs, serial/private mktemp TMPDIR and180s per-test cap:
CTest exit8, total5.76s, numeric failed5.64s and old neighbors passed0.10s.
All172 failures are selected identity mismatches:144 guarded and28 inactive,
zero direct-helper/setup/continuation/session/cursor/count/prompts/mark/skip/
callback/cleanup/old-neighbor failures. Fractions expose old rounding onto
the next position; positive/negative aliases and opposite-origin session2
expose mode-blind conversion. Full [original-gcc.log](original-gcc.log), SHA-256
c7d880ea3dc01402cfdda7fb1b68a815ebbe27bab4cdeee9456eeca7c1c493f8,
is byte-identical to the build-directory log. No assertions/helpers/CMake or
callback were changed. This is fail-before evidence, not acceptance.
The next separately configured Clang original retained stderr along with
stdout and captured the same frozen assertions before callback migration.

## Original Clang sanitizer baseline before callback migration

One-job Clang21.1.8 build completed with the frozen configuration and inputs.
Combined stdout/stderr build log SHA-256
75f5b4e664070ae8e535a807051bc5564850a3dff6c4e34d13e5dd36ddb83faf;
existing unrelated compiler warnings remain recorded, not repaired here.
Same serial/private TMPDIR/180s cap and ASan/UBSan options above: CTest exit8,
total6.21s; numeric failed6.10s and old neighbors passed0.10s. Exactly the same
172 failure lines as GNU (144 guarded/28 inactive identities), zero unrelated
assertion failures and no ASan/UBSan/float-cast-overflow diagnostic. Full
[original-clang.log](original-clang.log), SHA-256
2cceeb0fbbc31d0b0fc921f1c80fd9c7fe00cee8605e97f41c925fe898f95392,
matches the build-directory log byte-for-byte. All frozen hashes still match;
neither compiler required a helper/assertion/CMake repair. Both originals
were captured BEFORE callback migration; fail-before is not acceptance,
leak checking, independent review or platform/release qualification.

## Fixed GNU verification

Only the GETBAR callback migrated after both originals. It reads selected-
session numericbehavior through existing current_set_state(), converts to a
checked optional position, compares its positive value with count without
narrowing size, and preserves lookup/order/zero fallback. Runtime source SHA-256
530c04f6043e9d261f25702a38a6f59112d63e5c890daacebe2413d02a057ec7.
Helper/assertions/CMake remain frozen as above; nothing removed or weakened.
One-job GNU rebuild succeeded, combined build log SHA-256
c153beeaffa98ae7ca0f6daebeb8e99d69891a630b249c78e61f2d78f1942cfd.
Serial/private TMPDIR/1200s broader cap:14/14 PASS455.76s. Seven runtime tests:
GETBAR5.58s/neighbors0.11s, SKPBAR4.28s/PRMBAR4.32s/MRKBAR4.44s,
older Numeric79.56s/localization14.63s. Seven repository contracts include
safety traceability340.92s, issue intake, channel, isolation, changelog assembler/
validation and focused workflow paths. Retain actual elapsed times without
guessing a host-load cause. Full [fixed-gcc.log](fixed-gcc.log), SHA-256
ad64368c8ba09fa9d6644cb99abe70601dddd68c3ac337b0e56f7ff3496d6421,
matches the build-directory log byte-for-byte. Separate Clang fixed rebuild/
five focused/shared sanitizer tests started only after GNU completed.

## Fixed Clang sanitizer verification

One-job unchanged-config rebuild succeeded, combined build log SHA-256
ae5eacae580933474127836b4b9023152d47b0cb2b398ac9933fc0afed75d92c.
Same private TMPDIR/serial/180s cap and ASan/UBSan options as originals:
5/5 PASS18.84s, GETBAR5.59s/neighbors0.09s, SKPBAR4.32s/PRMBAR4.29s/MRKBAR4.53s.
No ASan/UBSan/float-cast-overflow diagnostic. Full
[fixed-clang.log](fixed-clang.log), SHA-256
ae5dcca536b267a32c462edb9ec66b098ec7890e6d8b0e903320149e055e6711,
matches the build-directory log byte-for-byte. All frozen helper/test/CMake
hashes still match after both fixed runs; fixed runtime hash remains530c04f6.
No overlapping builds/tests or VM. Existing compiler warnings were retained,
not conflated with sanitizer diagnostics or repaired outside this slice.
No leak/full native-error/type/order/system-menu/GUI/platform/release
qualification or independent human review claimed. Hosted exact-head required
checks, clean external review and resolved conversations remain PR gates.
