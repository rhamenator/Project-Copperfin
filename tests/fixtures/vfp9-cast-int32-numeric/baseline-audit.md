# Signed32 CAST baseline audit

Admitted #5611/#6776; RQ-CF-PRG-CAST-INT32-NUMERIC-001 mapped in README and
durable matrix BEFORE helper/header/tests/CMake/isolation edits. No migration
yet. Baseline main8988225cf9bd34ac14b560a4b0edd44bb27fcb87.

## Independent native chronology

Wine installed VFP9 SP2 7423, DISPLAY=:1, default prefix; executable
/home/rich/temp/copperfin-vfp9-wine-probe/vfp9.exe. Serial nice10/ionice2:7,
WINEDEBUG=-all, -t; initial config uses absolute -c path, repeat uses
-crepeat.fpw from build/cast-int32-native. No VM started or credentials used.

Native1 bounded25s exit124, no result, stdout/stderr0bytes. Native compiler
reported line5 unrecognized phrase for the overlong literal: original probe
and normalized diagnostic retained separately. Only native string splitting
changed before successful observations (no Copperfin helper/tests then).
Native2 exit0,147lines/DONE, stdout/stderr0bytes.
Supplement1 exit0,14lines/DONE, stdout/stderr0bytes, all12 constructed/source
controls executed before helper/tests. STR output is not exact bit evidence.
Native3 bounded25s exit124, native4 bounded55s exit124, stdout/stderr0bytes;
probe.out remained the OLD native2 result (mtime02:50:31.447-0400), therefore
neither timeout is a new observation or acceptance. Root cause unconfirmed.
Native4 started while a foreign ctest desktop stretch batch was briefly active;
that batch ended independently. No foreign process was changed. This overlap
is recorded transparently and never performance/serial qualification evidence.
Fresh repeat basename/relative config/unique output then native5/native6 each
bounded55s exit0,147lines/DONE, stdout/stderr0bytes and no other active local
build/test/Wine at launch. Native5/6 outputs each cmp-identical to native2.
Changing harness cap/path is NOT evidence of a root cause or optimum timeout.
Scratch logs retain all empty stdout/stderr files. No successful original
production baseline existed or was overwritten during native harness work.

SHA-256:
```
5dd92a25c734c6fa08ad4ca3e6bc70ec99f43922f1b33f0e4e2520019fa2e478 initial-probe.prg
f2841de829ecce40cc6d31971593a187edc0125041bfe859da260e9aca988823 probe.prg
d9008eaffb4234650903ecaa33d6cc88b7cbfe11f7a5900fe35a95815541e4bd repeat.prg (native6 output name; native5 differs only output name)
fe02ff81292c2fad530c43d8f75dbb0ecefa1b6921541ffdb9571103b617f9fe supplement.prg
f46b3f932a55650cea548d40048492c93705beb5530ce8c19a3ecf7db0e99b3a native-2.out/native-5.out/native-6.out (each)
c9c367a4624a9b47b5f5d26542c9b9850efe43406a853e42d166a78b2ec789d9 supplement-1.out
```
Copies cmp-identical to scratch outputs. Sources/output full, not excerpts.
The normalized initial compile diagnostic is expressly not an unedited raw log.

## Freeze BEFORE configuration or original execution

2026-10-10T06:59Z: distinct checked_cast_int32_numeric_argument UNUSED by CAST.
Literal expectations from native/parent policy/independent math, never from
Copperfin or completed neighbors. Initial count bookkeeping corrected BEFORE
freeze/configuration: multiple rows per line count individually. No compiled
assertion/input was changed or original baseline run during this correction.

146 direct =2modes x(43 Numeric +1NaN +4nextafter +13signed +12unsigned).
264 fresh public per shard =4aliases x(43Numeric +13exact +6non-Numeric
preservation +4locale). Four mode/session shards, separate CTest processes.
Each public case checks completion, literal result/error/kind, assignment
retention, continuation/control, cursor/context/reset; success message empty.
Default locale rejection11 versus VFP9 successful huge0 checks do not claim
public NaN expression or a new non-Numeric policy. Wider Character2147483648
is preserved by the untouched non-Numeric branch; 'x' keeps coercion error1.
Old signed64/unsigned64 CAST and exact-expression are unchanged neighbors.

Frozen SHA-256:
```
581cd944b7a7ecb8c29cb8a345c9cb2c2160c482042947dea2715d63a89ffeaf src/runtime/prg_engine_helpers.h
76c6adae4951cebd04a6d554e815e4ab339ea7181b16e8a044ef1a4ce4be6183 src/runtime/prg_engine_helpers.cpp
650cf7e3e3b472366a91ad8b87cf74448ae3ed2b632ffeca3123f3398bdf7845 tests/test_prg_engine_cast_int32_numeric.cpp
1d83b9d7758615c8e448d4e088f1071224af184272f79db6fd3235cd0780e646 tests/CMakeLists.txt
aa94f0815c3386937e70ce6f2dd17af4675d658a22c0e396cea7e3a5b09ec51c tests/CopperfinTestIsolation.cmake
6dd9d79d194c21e6e869e5fd612c5c8d107eb226620d4b097fc7d92b3507fdaf ORIGINAL src/runtime/prg_engine_runtime_surface_dispatch_general.inl
639778a08a38ef7d89a53d8f04a7daec99faf32c0077867cb9cf12a277225ffe unchanged src/runtime/prg_engine_runtime_surface_platform_helpers.inl
```
Any frozen input/assertion change requires explicit refreeze and repeating BOTH
complete original GNU/Clang baselines BEFORE selected migration. Retain full
raw logs/counts/hashes and incomplete attempts, never trim failure diagnostics.

GNU Debug-O0-g0 and Clang Debug-O1-g0 C/CXX, ASan/UBSan/float-cast-overflow,
explicit -fsanitize-recover=float-cast-overflow, frame pointers and matching
EXE/SHARED linker flags. Original UBSAN halt0 permits full negative evidence;
fixed halt1 with same binary flags. ASAN leak checks disabled/halt1/abort1.
Serial nice10/ionice2:7/-j1; private RAM fixture TMPDIR. Operational CTest CLI
caps600sGNU/1200sClang are not workflow timeout changes or optimum claims.
Focused regex: ^test_prg_engine_(cast_int32_numeric.*|cast_int64_numeric.*|cast_uint64_numeric.*|int64_exact_expression)$.
BOTH complete original raw results and all counters remain PENDING; only then
selected four-alias Numeric dispatch migration, fixed/broader/contracts/DV/
signed postpush/exact-head clean review/hosted gates. No parent issue closure.

GNU configure exit0, original build started07:00Z with no other active local
build/test/Wine at launch. A foreign SoundCurrent DAW build subsequently started
while the GNU build was running; no foreign process/file was changed. At
07:06:54Z this task's original GNU wrapper session/SID2597538 and verified
descendants were SIGSTOP-paused to reduce contention. This is a recoverable
build pause, not a failure, clean baseline, assertion refreeze or performance
qualification. Resume only the validated task-owned SID after other local
builds/tests/Wine finish; unified process session65414 remains pending.
At07:10:42Z the foreign build/test was no longer active; the exact owned
wrapper signature/SID was revalidated and only its verified descendants were
SIGCONT-resumed. No input/flag change or migration. This pause/resume is not
an elapsed-build performance claim; complete production originals still pending.

At07:12Z a new foreign DAW build began after our resume. GNU configure/build
completed exit0; CTest had begun. At07:13:18Z only this task's verified SID
was paused again. To avoid leaving a live test suspended across heartbeats or
treating a capacity interruption as a timeout qualification, verified owned
CTest2603037/test2603062 were SIGTERM-stopped07:14:24Z and the owned wrapper
resumed to harvest exit143. No foreign process/input/flag was changed. Native
source tables and all frozen helper/test/CMake/isolation hashes remain unchanged.

Complete unedited1393-line original-gcc-incomplete-capacity.log, cmp-identical
to scratch raw, SHA-256:
5c85373931dc74f69bd7f0bca0b6948d8e5cfb0fd942d8f0f9b1b3dab333d1a9.
It retains all configure/build output/warnings and partial test diagnostics:
signed64 direct56/four141, new direct146/two default264 counters completed;
VFP9session1 was interrupted partway, VFP9session2/unsigned64 did not run.
This is NOT a complete original baseline or clean sanitizer/acceptance evidence.
Only after retaining/cmp-verifying it, unchanged GNU original repeat started
07:15Z with no other active local build/test/Wine at launch (session98842).
BOTH complete originals still required BEFORE any migration, no refreeze.

## Complete unchanged GNU original BEFORE Clang configuration/migration

Repeat98842 configure/build exit0, CTest exit8 (expected selected gap),
12/16PASS30.25s. New146 direct/four264 public counters complete; unchanged
signed64 direct56/four141, unsigned64 direct62/four159 and exact-expression
neighbors all PASS. Exactly1216 selected assertions:296error,
296kind/assignment,592result,32localized; zero other failures. No frozen input,
expectation, helper/header/CMake/isolation or original dispatcher changed.

Complete unedited1431-line original-gcc.log SHA-256:
0f59c584d5cafb37da80fef5eda9ab60d10c50ade50dbc2bb333a8a3857bf8b8.
Fixture copy cmp-identical to scratch raw, retained BEFORE starting original
Clang configuration/build session35476 (frozen sanitizer/recovery/halt0,
same private RAM TMPDIR, no other local build/test/Wine active at launch).
This is complete negative comparison evidence, NOT acceptance. Clang original
and BOTH-original-before-migration gate remain pending. No assertion refreeze.
Original Clang configure exit0. A foreign DAW full CTest batch began just after
our guarded launch. At07:18:39Z the exact owned wrapper/SID2605846 and its
verified descendants were SIGSTOP-paused during BUILD, before any Clang CTest,
to reduce contention. Unified session35476 remains live; no test timeout is
running in that paused build. Resume only that validated owned SID after other
local build/test/Wine completes. No foreign process/file/input/flag changed.
At07:19:43Z that foreign CTest was finished; exact wrapper/SID revalidated,
only owned descendants SIGCONT-resumed. Original Clang build now continues;
no paused owned process or Clang test result yet. Freeze still unchanged.

## Complete unchanged Clang original and selected migration

Session35476 harvested: configure/build0, CTest8 (expected selected gap),
12/16PASS42.47s. All new146/four264 counters and unchanged signed56/four141,
unsigned62/four159/exact-expression neighbors complete. Exactly the same1216
selected assertions as GNU (296error/296kind/592result/32localized), zero other.
Four actual recoverable float-cast-overflow reports at original general.inl
687:63 for9.22337e18, one per public process; originals are NOT clean acceptance.
No frozen helper/header/test/CMake/isolation or original dispatcher change.

Complete unedited1813-line original-clang.log SHA-256:
d7635ee7df23c812eb3c3498269207278be1bbe42d899fe9978d675b027077eb.
Fixture copy cmp-identical to scratch raw BEFORE migration. BOTH complete
originals retained, so the earlier pending descriptions are historical.
Only then four-alias Numeric/exact dispatch migrated to the frozen helper,
selected-session Numeric mode and localized11 before return/assignment.
Original non-Numeric path and separated INT16/SHORT branch unchanged;
other targets/shared conversions/parser/settings untouched. Migrated dispatcher
SHA-2568ce0b020b824876a868ba9dbda7450e3a54981fee0346baa4a57121047d3f7ec.
No assertion refreeze. Fixed GNU session53489 started07:31Z with no other local
build/test/Wine active; same flags/privateRAM/nice10/ionice2:7/-j1. Fixed/broader/
contracts/DV/signedpostpush/actual clean exact-head review/hosted gates pending.

## Fixed focused verification

GNU53489 build0/CTest0,16/16PASS30.33s; Clang66850 build0/CTest0,
16/16PASS38.16s with unchanged instrumentation, UBSAN halt1 and ASAN
halt1/abort1/detect_leaks0. Both all146/four264 new, signed56/four141,
unsigned62/four159 counters and exact-expression neighbor complete.
No unexpected failures or sanitizer diagnostics; no frozen input changes.
Full unedited byte-identical fixed-gcc.log218lines SHA-256:
3e73a347cb55a4539c546eca04fe28b45ea16b5806207f7712cdda8b2343dbf8.
Full unedited byte-identical fixed-clang.log219lines SHA-256:
1b2fdb61c447b2b55347526243144e880c071ee039d9e8dfd5bfd0b4d7a6794a.
Clang completed07:33:39Z. A foreign short stretch CTest was observed only
after this completion; broader launch waited for its completion. No foreign
process changed, and elapsed times are not performance/serial qualification.

First broader GNU runner stopped before build/CTest because it mistakenly
named nonexistent test_prg_engine_null. Full unedited3-line
broader-gcc-incomplete-runner.log cmp-identical SHA-256:
53c9abab65c94c01b070cd3a853fcc5de9496462d58011182f14532ca4d7b1d6.
This is NOT acceptance. Corrected only ignored runner target/regex to existing
test_prg_engine_null_semantics for GNU/Clang; no test/input/assertion/production
change, no refreeze. After full retention, broader GNU73183 started07:35Z
with no other local build/test/Wine active. Twelve selected tests include
four unchanged runtime neighbors, localization and all seven planned contracts.
Broader/assurance/integration remain pending.

At07:38Z a new foreign DAW artifact build began after the guarded launch.
At07:39Z the exact owned wrapper/SID2625679 and CTest2626446 were validated;
only owned CTest/cmake/pwsh descendants were SIGTERM-stopped (no suspension
left across heartbeats). Wrapper73183 harvested143, build0, five preliminary
contracts PASS, safety-workflow in progress and runtime neighbors not run.
No foreign process/file changed. Full unedited136-line
broader-gcc-incomplete-capacity.log retained/cmp-verified SHA-256:
89e1eaf897d693f88938bd01417ffc0b5f493b08501d9729ca6b07f67a85c49a.
NOT complete/acceptance. After retention and with foreign work finished,
unchanged broader GNU repeat48740 started07:39Z, same flags/caps/inputs.
No production/test/assertion repair or refreeze.

At07:42Z another foreign DAW full build was active after the repeat launch.
Exact owned wrapper/SID2636011 validated; only owned descendants SIGSTOP-paused
while this turn remains active, with CTest's1200s cap unchanged. This operational
capacity pause is NOT elapsed-test/timeout-optimum or serial qualification.
Resume only the validated owned SID after foreign build/tests complete; do not
leave a suspended test across heartbeats. No foreign process or frozen input
changed. Complete broader results remain pending.

Foreign full DAW build continued, then its full CTest and a separate Copperfin
audit build were active07:45Z. To avoid leaving suspended tests across
heartbeats, at07:46Z only the revalidated owned SID2636011 CTest/cmake/pwsh
received SIGTERM, then only owned descendants SIGCONT to harvest48740 exit143.
No remaining owned active/paused process; no foreign process/file changed.
Full unedited82-line broader-gcc-incomplete-capacity-2.log cmp-identical SHA:
804be80880697fd9b55dbb5e3f70141462e14f040ba59f9736c97c60fe92d545.
Again build0/five preliminary contracts PASS, safety-workflow incomplete and
runtime neighbors not run; NOT acceptance/performance/timeout qualification.
Next repeat unchanged broader GNU only when other local builds/tests/Wine
finish, then broader Clang four runtime neighbors, DQ/DV and integration.
Frozen numerical inputs/assertions/helper/CMake/isolation unchanged throughout.

07:46 heartbeat: rules/current handoff/canonical unread channel(empty) reread,
origin/main8988225cf unchanged after fetch; admitted parents OPEN/owner/exact
agent-approved and bodies reread. Prior7126 MERGED/all36green, actual clean
6094132735 exactd10f223f35, REST reviews/inline empty and zero Graph threads.
No PR for active branch. Foreign build/tests finished by07:48Z; after verifying
the retained second incomplete raw, unchanged broader GNU19236 started07:48:45Z
with no other local build/test/Wine active. Same inputs/flags/caps/RAM/serial
priorities; no paused owned process. Complete broader/assurance still pending.

At07:49:58Z a new foreign full DAW CTest was active after our guarded launch.
Exact owned wrapper/SID2658443 validated; only owned descendants SIGSTOP-paused
while this turn remained active. At07:52:10Z foreign work had finished; exact
owned wrapper revalidated and only owned descendants SIGCONT-resumed. CTest
cap remains1200s; no suspended test left across heartbeats. No foreign process/
file or frozen input changed. This pause is not serial/performance/timeout-
optimum qualification; actual complete results still pending.

## Complete broader GNU and focused changelog repair

Session19236 build0/CTest8 completed all12 selected tests in544.16s, including
the documented capacity pause. Eleven PASS: four unchanged runtime neighbors,
localization and six of seven contracts (safety-workflow462.07s inclusive of
pause). Sole failure was our fragment's missing required date prefix; no
Numeric/runtime assertion failed. Full unedited181-line
broader-gcc-fragment-failure.log retained/cmp-identical SHA-256:
e964313fa4c6d8b5666d2052122041fef4b6c8d3e1026e6e57a98dd5fb26b0a8.
Locale-install's intentional missing-pt-BR negative subprocess CMake error is
retained and its enclosing test PASS, not an unexpected failure.

After retaining raw, read changelog.d/README.md and changed ONLY our fragment:
required dated single bullet plus indented continuation. No test, production
or numerical frozen input changed; no refreeze. Exact fragment check and
assembler then2/2PASS0.41s/exit0; complete unedited48-line fragment-fix-gcc.log
cmp-identical SHA-256:
7216bf38c113df868246bffd02343533d99dfb3b9de259db55807d95fdee48a5.
Together these completed runs verify all12 planned GNU subjects, including all
seven contracts; this is explicitly NOT a single all12-PASS invocation or a
runtime/sanitizer repair. Broader Clang32896 started07:59Z with no other local
build/test/Wine active, same flags/privateRAM/serial priorities and halt1.
Assurance/integration remain pending.

Broader Clang32896 build0/CTest0,4/4PASS97.84s with the same sanitizer flags,
halt1/abort1/detect_leaks0; four unchanged Numeric/NULL/typed-NULL/string-math
neighbors PASS, no sanitizer reports or unexpected failures. Complete unedited
80-line broader-clang.log cmp-identical SHA-256:
012d4baa203839b3e75acf17ca0261ce6d830d7886eada90ca4084527f356ccf.
Local focused/broader subjects are verified; signed-head integration remains.

## Bounded documentation development self-review and walkthrough

DQ/DV-5611-CAST-INT32-NUMERIC-001: Codex delegated development self-review,
not independent human or high-hazard qualification. Inspected default
finite-before-truncation and signed32 domain checks before conversion, exact
signed/unsigned admission without Double, NaN containment before VFP9 mapping,
defined guarded signed64/low32 primitive, selected-session mode and error11
before return/assignment. Reverse RQ links and only four-alias Numeric dispatch
delta checked; frozen numerical inputs/assertions/helper/header/CMake/isolation
unchanged. Non-Numeric coercion, INT16/SHORT and other targets remain separate.
Native decimal/source/STR limits, unsupported native INT32/LONG and parent-
derived modern exact-kind/NaN policy remain explicit, never filled by CF output.

Automated walkthrough, all four aliases and both modes/sessions1/2:

| Initial state / action | Expected outcome | Retained verification |
| --- | --- | --- |
| Guard cursor, Number sentinel12345; cast1.9 | Exact int64 value1, no error; context/continuation/reset preserved | Complete GNU53489 and Clang66850 focused16/16 PASS, all146/four264 counters |
| Default cast2^31, -2147483649 or huge/infinite Numeric | Catchable11 before assignment, sentinel retained, next guard1; four literal catalogs for huge rejection | Same complete focused runs; direct NaN/nextafter boundaries PASS |
| Explicit VFP9 cast2^31 or2^32+1; huge/overflow consumer | Signed32 MIN or1; huge/overflow0, no unsafe conversion; not saturation | Same complete focused runs, independent native observations and pre-code literals |
| Construct exact2^53+1 or full unsigned64 MAX | VFP9 low32 preserves1 or-1; default rejects before assignment; Numeric rounded spelling is a distinct boundary | Same complete focused runs; no native exact64/source-parsing parity claim |
| Cast safe non-Numeric Character2147483648 or invalid Character'x' | Preserved wide int64 or error1 respectively, not a new signed32/type contract | Same complete public preservation controls; broader GNU runtime/localization and Clang4/4 PASS |

All seven GNU contracts have completed PASS evidence across the retained full
11/12 invocation and exact fragment/assembler2/2 repair verification; do not
describe this as a single12/12 clean run. Procedural misuse severity remains
medium for this bounded conversion guidance (wrapped identifiers, confusing
wrap with saturation or claiming all CAST/type safety). Existing high/
catastrophic HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 remain
active, not reclassified/closed. No new destructive operator/recovery procedure.
Rollback the bounded code/tests/docs together, retain original/fixed/native
evidence, correct published conversion guidance and notify affected integrators
if guidance is wrong. This completes bounded development review/walkthrough,
not independent-human, leak, full-platform, persistence, release, CAST-family
or parent acceptance. Actual clean signed-head review/required hosted checks/
resolved conversations remain integration gates.
