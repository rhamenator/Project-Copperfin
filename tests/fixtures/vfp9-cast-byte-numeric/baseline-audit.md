# BYTE/UINT8 CAST baseline audit

Admitted #5611/#6776; origin/main c1a6eef528c1d621196a6c7e232cf8e399174ea0.
RQ-CF-PRG-CAST-BYTE-NUMERIC-001 mapped in README/durable matrix BEFORE new
helper/header/tests/CMake/isolation or migration. This mapping was recorded
before original configuration/compilation. BOTH originals are complete and
retained before production migration; fixed acceptance remains pending.

## Independent native chronology

Installed Wine VFP9 SP2 7423, DISPLAY=:1/default prefix/executable
/home/rich/temp/copperfin-vfp9-wine-probe/vfp9.exe. Serial nice10/ionice2:7,
WINEDEBUG=-all, bounded55s, relative -cprobe.fpw or -crepeat.fpw from
build/cast-byte-native. No VM start, credentials or foreign-process changes.
No other local build/test/Wine was active at each launch.

Native1 exit0:77lines/75calls/DONE, stdout/stderr empty. Native2 exit124 after
55s:stdout/stderr empty, no native-2.out; native.out mtime remained native1
2026-10-10T10:19:25.467667843Z. Old native.out is NOT a new observation.
Root cause unconfirmed. Native3 fresh repeat basename/unique output and -t:
exit0,77lines/75calls/DONE, stdout/stderr empty, cmp-identical to native1.
Harness basename/output/-t variation is NOT timeout-root-cause/optimum proof.
Both successful observations contain25 INTEGER controls plus50 unsupported
BYTE/UINT8 errors11, including zero. No native unsigned8/exact64 policy claim.

SHA-256:
```
9585724a2f22999494d30edce7c5d913f3fceb56cf827c93a022d2072518465d probe.prg
3641021ef6314c212c5176661eb9c6fdcc703d7d421e42efc56111dd3ace38f6 probe.fpw
a6c0e957e9ad15b338a3245c373ccb9d6bb41c2922351c46f65ec138d017f60e repeat.prg
bc13926cf677440c7bca68356ae8a5137695de83a427e97018ea9109f8f1718d repeat.fpw
a2ff74ab1dc056ffdea0e451089f2559fbc1e4068d5c28c2f25919f67efe1d70 native-1.out/native-3.out each
```
Full unedited sources/results and all empty stdout/stderr retained in fixture;
copies cmp-verified before helper/tests. No Copperfin oracle/assertion repair.

## Freeze BEFORE configuration or original execution

2026-10-10T10:25Z: distinct checked_cast_byte_numeric_argument UNUSED by CAST;
source search confirms only declaration/definition/new direct tests use it.
Before any configure/compile/run, source inspection corrected an undeclared
test-factory name and added lower nextafter decimal cases/counts. No compiled
assertion/input repair or original baseline existed during those edits.

140 direct =2 policy labels x(39Numeric +1NaN +4nextafter +13signed +11unsigned
+2non-Numeric adapter guards). Helper intentionally has no mode parameter;
direct labels duplicate the common extension contract, NOT session execution.
126 fresh public per shard =2aliases x(39Numeric +14exact +6non-Numeric
preservation +4locale). Four mode/session shards in separate CTest processes
actually select both session policies. Each checks literal result/kind/error,
sentinel retention, continuation/control, cursor/context/reset; successful
message empty. No new public NaN syntax/type/Currency/arity requirement.

Frozen SHA-256:
```
b5f0f8880f13cecf068258b057fcc92fd7ad031be95072d3f73bec32caab145b src/runtime/prg_engine_helpers.h
befc95c2184859f00e8c5d2a81640ddbf209e48c74b64caaa996ca49d785e106 src/runtime/prg_engine_helpers.cpp
d62685cc7f0a3288f6bde1a69223964a6ead2a8442ab5fc83b46187f2b66bff8 tests/test_prg_engine_cast_byte_numeric.cpp
7763072c57867276409db19cbcf41f60a7a2bda63f8602af40cf9930115ad7ad tests/CMakeLists.txt
819c42a73c23ee5b122f6e8b5a47508fa15d9c250b80ed50e45cff30b1be7192 tests/CopperfinTestIsolation.cmake
```
Unchanged signed32 five shards/signed64 five/unsigned64 five/exact-expression
neighbor included; none supplies new BYTE requirements. Original Clang uses
recoverable float-cast-overflow/halt0 to finish all frozen cases; fixed halt1,
ASAN leaks disabled. Original/fixed raw logs retained unedited, not snippets.

## Complete GNU original retained BEFORE Clang configuration / migration

Unified session41173 exited8; GNU15.2.0 Debug/Ninja -O0 -g0, configure0/build0,
CTest8:17/21PASS,36.80s. All140direct/four126public counters complete; unchanged
signed32(146/four264), signed64(56/four141), unsigned64(62/four159) and
exact-expression neighbor complete/PASS. Actual872selected assertions:
280error+280result+280kind/assignment+32localized-message; zero other failures.
All completion/continuation/control/cursor/session/reset/count guards passed.
This is complete negative comparison evidence, NOT fixed acceptance.

Full1431-line original-gcc.log copied byte-identically/cmp-verified before
Clang configuration and BEFORE any dispatcher migration. SHA-256:
84fd27c539909432612778ec52e754eda33b694e0f3147a63fe9b959d5dc9c71.
All five frozen file hashes unchanged; dispatcher diff empty. No assertion/
input/helper/header/CMake/isolation change or refreeze after original began.
Serial lowpriority privateRAM /dev/shm/copperfin-cast-byte-5611-k2k91nVu;
no other local build/test/Wine observed before original or Clang launch.

## Complete Clang original retained BEFORE migration

Unified session63504 harvested exit8 during this11:15 heartbeat; Clang21.1.8 Debug/Ninja,
-O1 -g0, ASan/UBSan/float-cast-overflow with recovery/UBSANhalt0/leaksdisabled.
Configure0/build0/CTest8:17/21PASS50.56s. All140direct/four126public and
unchanged signed32(146/four264), signed64(56/four141), unsigned64(62/four159)
plus exact-expression neighbor complete. Actual872selected assertions:
280error+280result+280kind/assignment+32localized-message; zero other failures.
All completion/continuation/control/cursor/session/reset/count guards passed.
Four actual recoverable float-cast-overflow reports, one per public shard:
-1 outside unsigned long at original general dispatcher705:44. No other
sanitizer report. This is negative comparison evidence, NOT clean acceptance.

Full1526-line original-clang.log copied/cmp-verified BEFORE migration;
retained-file birth time11:18:46.825225500Z:
SHA-256 5d221d744f7cd57061a3a5490882bb5be0edf1c5ff07570815def5a28fe98e97.
All five freeze hashes unchanged and dispatcher diff empty at preservation.
No assertion/input/helper/header/CMake/isolation repair or refreeze. Owned
SID2795223 finished; no local build/test/Wine remained at harvest.

## Numeric-only migration and fixed focused verification

Independent expectation/architecture/VR/HZ/DQ/DV mapping is in README.
Both complete original raw logs/counters now retained BEFORE selected
Numeric-only migration. Fixed runner must use same frozen inputs and flags,
UBSANhalt1/ASANleaksdisabled; recovery was for originals only.
Any assertion/input repair requires explicit refreeze and BOTH originals again.
GNU fixed session39207 exit0: build0/CTest0,21/21PASS36.63s; all frozen
new/neighbor counters complete, zero assertions. Full274-line fixed-gcc.log
copied/cmp-verified, SHA-256
045b6f3f5473fde7185f0cacd88b0d275a2e614d63ac758622465a6128c6df79.
All five freeze hashes unchanged; no assertion/input repair/refreeze.
Only the BYTE/UINT8 Numeric-kind dispatcher gate changed at11:19:24.442385071Z; original
non-Numeric fallback and all other CAST/parser/settings/shared paths retained.
Both fixed logs confirm dispatcher-containing runtime-surface object rebuilt,
then library and all five focused executables relinked.
Clang fixed session24637 harvested0: build0/CTest0,21/21PASS46.05s;
all frozen new/neighbor counters complete, zero assertions or sanitizer reports.
UBSANhalt1/ASANleaksdisabled, same flags/frozen inputs/privateRAM/serial nice10
ionice2:7 -j1. Full275-line fixed-clang.log copied/cmp-verified, SHA-256
960ce45e5ec04d1b680c220eb4c35f36c8e280d94026434b0d08413337951a10.
Existing unused bitwise_value/bit_position warnings retained in both fixed
build logs, not warning-free qualification. No frozen input/assertion/helper/
header/CMake/isolation repair or refreeze. Log birth times independently retain
GNU fixed11:19:43.101464223Z, Clang fixed11:20:39.624703991Z and broader GNU
session1669 11:22:00.972049071Z. Broader launched only after focused Clang
finished; no other local build/test/Wine. Times are filesystem metadata,
not inferred command durations or approximate narrative launch labels.

## Broader verification

GNU session1669 harvested0 at the11:45 heartbeat: build0/CTest0,
12/12PASS417.07s. All four runtime neighbors, localization and seven contracts
passed in one invocation. Safety traceability contract333.65s; complete
successive PowerShell fixtures, not a timeout/retry or diagnostic repair.
Full230-line broader-gcc.log copied/cmp-verified, SHA-256
828c1b93c0d581ba723fa41a329ec8fee6c47ea60f18234ec8abcdca3665d666.
Only after GNU completed, broader Clang session32447 launched; log birth
11:45:53.354184278Z, ownedSID2837955, same halt1/leaksdisabled flags and
serial low-priority RAM scratch. Session32447 harvested0: build0/CTest0,
4/4PASS114.85s, no assertions or sanitizer reports. Full80-line broader-clang.log
copied/cmp-verified, SHA-256
b0ee40a756a6abf22e836783151ee7f42853a264a8514cdf51ae62d1d0684859.
No local build/test/Wine remained after harvest. No runner interruption,
assertion/input repair, capacity pause, CI retry or VM start in these runs.

## DV-5611-CAST-BYTE-NUMERIC-001 development walkthrough

2026-10-10: Codex completed delegated maintainer-development self-review of
DQ-5611-CAST-BYTE-NUMERIC-001 against its named parent/independent native
controls and unsigned8 mathematics. This is agent/development self-review,
NOT a personally posted independent-human or high-hazard sign-off.
Both fixed focused runs complete140 direct/four126 public cases:
-0.9/lower-inner-nextafter=>uint64 zero;255.9/upper-inner-nextafter=>uint64 255;
-1/256/outer neighbors/huge/nonfinite=>localized catchable11, Numeric sentinel
12345 retained. Exact signed/unsigned endpoints0/255 accepted; wider/negative
exact values, including2^53+1/INT64_MAX/MIN/UINT64_MAX, rejected, not low8 aliases.
NaN is a direct helper containment case, not new public syntax. Both modes and
sessions1/2 continue to the independent INTEGER control, retain guard cursor/
record/label and selected context, then reset to1|COPPERFIN|. Four catalogs
match literal messages. Ordinary Boolean/Character/Currency controls stay on
the untouched path; Character '256' still masks to0, NOT Numeric policy.
The complete broader GNU12/12 and sanitizer Clang4/4 support the retained
Numeric/NULL/typed-NULL/string-math/localization/isolation/traceability boundaries.

Misuse review: unsupported native BYTE/UINT8 errors do not justify invented
VFP9 wrapping; either mode rejects Numeric256. Documented truncation permits
negative fractions greater than-1, not negative exact integers. Rounded source
Numeric spellings are never represented as exact64 operands. This bounded
documentation delta remains medium; registered three HZ severities/status
unchanged. No critical recovery/process/persistence/allocation procedure or KBX.
Rollback bounded helper/dispatcher/tests/docs together if policy is rejected;
preserve raw/native records, correct affected guidance and notify the owner
and affected users before relying on published guidance. Integrators still
own independent domain assurance. Signed postpush, required hosted checks,
actual clean exact-head Claude-first/Codex-fallback review and resolved
conversations remain integration gates. No family/parent/type/arity/leak/
platform/release acceptance is claimed by this local development evidence.

## Review repair refreeze, 2026-10-10T12:49:49Z

Exact pushed head152d8f8f0b7b09113a82b4f903c5526e2820ca7c received
Codex finding4237581146: the existing shared runtime-surface success script
still used Numeric513 and expected low8 result1. This contradicts the already
recovered unsigned8 domain; it is not a new production defect or permission
to restore wrapping. Linux run38049896945/job114206740904 and macOS
run38049896940/job114206740964 each completed496/504, failing the main
runtime-surface test and all seven bit-neighbor binaries at that shared script.
Full unedited host logs retained at build/cast-byte-native/linux-native-failure.log
(4254lines, SHA345bd3dbe85bafc784dbc252d07a5da44c731a2a4c4263a5aab19324d4b45099)
and macos-native-failure.log
(4096lines, SHA1e9b2cc3fea90fa8e87497647c6e2d52ce0e3d309066293a9c75166c82c3f2ea).
These failures are negative integration evidence, not clean acceptance.

The shared success-path input changes513 to1, retaining its expected exact
result1 and all later bit/coercion assertions. The dedicated frozen boundary
suite still rejects256 and larger Numeric/exact values with atomic11; none
of its expectations/counts change. Reverse link added to the smoke input.
Explicit revision2 refreeze BEFORE original repeats: shared fixture SHA
d4ae72a20273c73a38471c61838434324cf16f0813156c87b4f856cc890b7b0a;
all five original frozen helper/header/test/CMake/isolation hashes above
remain unchanged. Repeat both complete original focused21 plus seven affected
bit neighbors before restoring the Numeric gate; retain full results, not
repair assertions to match output. Main runtime-surface/full504+ tests remain
hosted integration gates. No production policy, helper, dedicated test,
configuration, isolation or non-Numeric path changes in the review repair.

GNU revision2 original83383 harvestedexit8 at12:52Z: configure0/build0,
24/28PASS38.03s. All21 focused shards completed with unchanged counters;
only four BYTE public shards failed with the same872selected/zeroother
assertions. All seven bit-neighbor binaries passed with admitted1, preserving
their subsequent expressions/property checks. Full1246-line raw
review-original-gcc.log copied/cmp-verified into this fixture BEFORE Clang
configuration or gate restoration, SHA-256
4cae4ef9708864f23c316338870f1546302dbba523ed011ecb5500e2f899355b.
Negative original comparison, not fixed acceptance. Six frozen hashes unchanged.

Clang revision2 original23664 harvestedexit8 at12:54:17Z: configure0/build0,
24/28PASS51.97s, same complete counters/872selected-zeroother assertions and
seven passing bit neighbors. Four actual recoverable float-cast-overflow
reports at original705:44 for -1; no additional sanitizer report. Full1343-line
review-original-clang.log copied/cmp-verified BEFORE gate restoration, SHA-256
953765a23964963555b5ae0eec1279ec6dd9c5768f54e5a617cbdb3e18cda5ed.
All six frozen hashes unchanged. Original dispatcher SHA
8ce0b020b824876a868ba9dbda7450e3a54981fee0346baa4a57121047d3f7ec
matched origin/main byte-for-byte through both repeats. Restored only the
identical pushed152d BYTE gate at12:54:31Z after both raw logs were retained;
dispatcher now has zero diff against pushed head. No production change in fix.

GNU revision2 fixed84136 harvestedexit0 at12:55:33Z: configure0/build0/CTest0,
28/28PASS38.42s, all frozen new/neighbor counters complete, zero assertions.
Full355-line review-fixed-gcc.log copied/cmp-verified into the fixture,
SHA514f65a6670c9b6e8965ea164ac29bcff9df404a5923af5c9c6e797e50e1196f.
Six frozen inputs unchanged; the log explicitly rebuilds the dispatch-containing
runtime-surface object and relinks all twelve tested binaries. The main
runtime-surface binary is not locally rebuilt here; its identical shared
function is covered by all seven neighbor binaries, and the full hosted main
test remains an integration gate. No full-platform acceptance inferred.

Clang revision2 fixed48573 harvestedexit0 at12:57:10Z: configure0/build0/CTest0,
28/28PASS48.63s, all frozen counters/zero assertions/no sanitizer report,
UBSANhalt1/leaksdisabled. Full356-line review-fixed-clang.log copied/cmp-verified,
SHAc3bb5141f32c6b91bfe2ed514e2f762c90cffedf004f68572827159cda40bafb.
Both fixed logs show the dispatch-containing object rebuild/library and all
twelve binary relinks; existing unused-function warnings retained, not
warning-free qualification. Dispatcher/helper/header/dedicated test/CMake/
isolation have zero diff against152d; shared refreeze hash unchanged.

DV review-repair walkthrough completed2026-10-10T12:57Z as delegated bounded
development self-review plus automated verification, not independent human
qualification. New admitted1 success result is already prescribed by the
unsigned8 mathematics; all seven shared-script consumers continue through
later bit/property assertions. Unchanged dedicated overflow shards still
prove localized atomic11 in both modes/sessions. The initial four-runtime
broader selection did NOT execute this shared smoke function; the first
full hosted suites exposed that missed integration dependency. Seven local
affected neighbors now supplement, not replace, full hosted main/platform
gates. No assertion changed to reproduce old wrapping, no policy expansion,
no hazard downgrade, no family/parent/release qualification. Preserve all
negative originals and failed CI raws; rollback/field-notification plan above
still applies. Fresh signed-head review/checks/resolved threads remain gates.

Revision2 reproduction adds targets test_prg_engine_bit{test,clear,set,xor,or,
and,not}_neighbors to the original five focused targets, and extends the
same CTest regex with bit(test|clear|set|xor|or|and|not)_neighbors. All four
repeat suites use cached GNU/Clang flags above, configure before build,
-j1/CTest--parallel1/-V/timeout1200; original Clang halt0, fixed halt1,
ASANleaksdisabled, nice10/ionice2:7 and the same private RAM TMPDIR. Full
28-test raw logs retain exact registered commands and environment. No VM.

Revision2 assembler/fragment/isolation contracts87698 harvested0 at12:58Z:
3/3PASS1.00s. Full60-line review-contracts.log copied/cmp-verified, SHA
bb261ca506f3662d3994fc71fa3e60dd568b0c2c3c02999d518043d51198a3ea.
Nonraw changed source/docs whitespace checks passed. All runtime source,
dedicated frozen test, CMake and isolation have zero diff against152d.


## Reproduction configuration

Configure separate GNU/Clang Ninja Debug build trees. GNU C/CXX compiler
gcc/g++, C/CXX Debug flags -O0 -g0. Clang compiler clang/clang++, C/CXX Debug
flags -O1 -g0 -fsanitize=address,undefined,float-cast-overflow
-fsanitize-recover=float-cast-overflow -fno-omit-frame-pointer; executable and
shared linker flags -fsanitize=address,undefined,float-cast-overflow.
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:abort_on_error=1; original
UBSAN_OPTIONS=halt_on_error=0:print_stacktrace=1, fixed/broader halt_on_error=1.
Use task-private writable TMPDIR and serial nice10/ionice2:7 builds -j1.
Focused build targets: test_prg_engine_cast_byte_numeric,
test_prg_engine_cast_int32_numeric, test_prg_engine_cast_int64_numeric,
test_prg_engine_cast_uint64_numeric, test_prg_engine_int64_exact_expression.
Focused CTest regex:
```
^test_prg_engine_(cast_byte_numeric.*|cast_int32_numeric.*|cast_int64_numeric.*|cast_uint64_numeric.*|int64_exact_expression)$
```
CTest --parallel 1 -V; original GNU timeout600s, all others1200s. Broader GNU
adds four runtime targets, test_localization and copperfin_inspect; four runtime
targets are numeric_behavior/null_semantics/typed_null/string_math_functions
with test_prg_engine_ prefix. Its seven contracts are locale_catalog_install,
changelog_fragment_assembler, changelog_fragments_valid, powershell_discovery,
native_test_isolation, safety_traceability_workflow and portable_clr_host_boundary
(exact CTest names and full test commands retained in broader-gcc.log).
Broader Clang builds/runs only those four runtime targets, same fixed sanitizer
environment and serial CTest. Reproduction does not infer native requirements
from these tests; original-before-migration chronology remains independently
retained above.
