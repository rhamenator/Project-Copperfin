# GETFONT Numeric size baseline audit

Requirement/native evidence mapping preceded helper/tests/configuration at
2026-10-10T19:26Z; see README.md and the durable matrix/hazard linkage.
Admitted #5611/#6776, origin/main14e6188406a8e4289eccee5b6fe50614eba07792.
No VM, production dispatch change, commit, push or PR at this freeze.

## Freeze before configuration

Helper UNUSED by GETFONT: only definition, declaration and direct test use.
Dispatcher diff is empty versus origin/main. Test/isolation source inspection
and whitespace checks precede configuration. An unsupported isolation SCHEDULE
keyword was removed before this freeze; registrations intentionally remain
serial, scoped-locale and unique test-owned scratch. No compiled assertion or
input repair has occurred.

Independent literal counts: 172 direct =2 modes x(46 Numeric+1 derivedNaN+
6 adjacent/subnormal+18 exactsigned+12 exactunsigned+3 non-Numeric adapter).
Four fresh public mode/session shards each79 =46 Numeric+20 exact-expression+
6 unchanged non-Numeric+3 omitted/name/style controls+4 catalog cases.
Default public23 errors/56 successes; explicit VFP9 public16 errors/63 successes.
Every public case checks completion, error11/result assignment/kind, event
count/full checked-request payload (no event on error), continuation, cursor,
selected policy, neighboring area policy and reset. Literal expectations are
from the pre-code recovered/derived requirement, not this helper or old output.
NaN is direct only, with no invented public/native syntax. The exact negative
above2^53 and INT64_MIN+13 must retain low13 directly, not Double12/14/zero.

Frozen SHA-256:
```
d5ace4a635e258d82e06df1880fcf0cb58d9a16b2fb32f404780f7175783fa71 src/runtime/prg_engine_helpers.h
57217df44864a76947172ebb1452f85eefa82cec4f331b0a3a82a77f96815461 src/runtime/prg_engine_helpers.cpp
39401021dd07968b2799f32523a9b570d95d7dfae0b274cf01a284d7c66ed11f tests/test_prg_engine_getfont_size_numeric.cpp
770782c5a790b3c0785633ab18be811b1b97e5581ef1d1c59931b4ba908fa163 tests/CMakeLists.txt
56a97051b8b96e9cc0b659b0cb886ee0d1bef79e7626eca61d258f21bb60fec5 tests/CopperfinTestIsolation.cmake
```

Original verification selects13 registered tests: five GETFONT shards, five
unchanged INT16 shards, exact-int64-expression, numeric-behavior and isolation
contract. GNU Debug/Ninja-O0g0; Clang Debug/Ninja-O1g0 ASan/UBSan/
float-cast-overflow recovery/halt0/leaksdisabled for originals, halt1 for fixed.
Serial nice10/ionice2:7/-j1, private700 RAM TMPDIR
`/dev/shm/copperfin-getfont-size-5611-SJIXBLYl`. Record full configure/build/
CTest raws, complete counters, errors and hashes for BOTH compilers BEFORE
Numeric-only migration. std::llround is an uninstrumented library operation;
do not invent an original sanitizer finding if it only returns a bad value.
Any frozen assertion/input repair requires refreeze and BOTH original repeats.
Existing runtime-surface GETFONT smoke remains untouched and receives broader
fixed/hosted verification. No complete original/fixed/review acceptance yet.

## First complete original, before Clang configuration

GNU session11867 harvested exit8 at2026-10-10T20:00Z:9/13 PASS, only four
GETFONT public shards fail; direct172 and four public79 counters complete.
Unchanged INT16 direct162/four public152, exact-int64, numeric-behavior and
isolation contract pass. Full configure/build/CTest raw copied unedited/cmp
before any Clang configuration: original-gcc.log953 lines, SHA-256
f9d0b7ace38bf510b17ee2741dc9b07069a54886bb5a3454d2e5312cc03d9d16.
CTest78.96s;486 selected assertion failures (132/132 modern,111/111 legacy),
zero other failures. No sanitizer diagnostic claimed from uninstrumented llround.
All five frozen hashes unchanged; production dispatcher diff empty. No frozen
input repair, repeat or migration. Clang original and all fixed gates pending.

## BOTH complete originals retained before production migration

2026-10-10T20:15:39Z: Clang86812 harvested8,9/13 PASS/only four GETFONT
public failures in113.53s. Direct172 and four public79 counters complete;
unchanged INT16 direct162/four public152 and all three other contracts pass.
Exactly486 selected assertion failures (132/132 modern,111/111 legacy),
zero other failures; no ASan/UBSan/float-cast-overflow report. This is bad
value/error/atomicity evidence, not an invented llround sanitizer finding.
Full952-line original-clang.log copied unedited/cmp BEFORE migration, SHA
2e8dcda67cb327077796a1c24591dfde66a51461c6b11713e78453c7f24fd687.
GNU953-line full raw remains retained/cmp. Five frozen hashes unchanged,
dispatcher diff still empty and helper UNUSED by it. No input/assertion repair
or original retry. No other local build/test/Wine at either clean serial launch.
Warnings/raw whitespace retained; no warning-free qualification claim.
The BOTH-original gate is now complete; bounded migration and fixed gates
remain pending at this record. No VM, commit, push, PR or review acceptance.

## First fixed focused run

Only after the20:15:39Z BOTH-original record, GETFONT's Numeric/exact second
argument was migrated to the frozen helper and existing localized error11.
Non-Numeric coercion, name/style/result/omitted input and other consumers
remain unchanged. GNU73659 harvested0 at20:17Z:13/13 PASS in78.08s,
all172/four79 GETFONT and162/four152 INT16 counters complete, zero failures.
Full184-line fixed-gcc.log copied unedited/cmp, SHA-256
44d79087cafb0e4e33202a61e4193c5c7a7c300b0d08f9f3646f2d70fbae0006.
All five frozen input hashes unchanged; no assertion/input repair or retry.
Clang fixed halt1 and broader/integration/review acceptance remain pending.

## BOTH fixed focused runs retained

Clang25268 harvested0 at2026-10-10T20:20Z:13/13 PASS in111.17s, all
172/four79 GETFONT and162/four152 INT16 counters complete. Halt1 ASan/UBSan/
float-cast-overflow/leaksdisabled: no sanitizer reports or assertion failures.
Full185-line fixed-clang.log copied unedited/cmp, SHA-256
5cd041ea852eaae9d26c67e092cfd607db64bfa22a45f5d79a83f00890526af4.
GNU fixed184-line raw retained/cmp. All five frozen inputs unchanged, no input
repair/refreeze/retry, no other local workload at either serial fixed launch.
Production dispatcher SHA-256 after the sole Numeric-only migration:
94a13179d4c87373d1964f580119e23e683cb8057392fa4fb184d2272a0d68a9.
All owned workloads harvested; no suspended native/build/test process remains.
Broader untouched GETFONT smoke/localization/contracts and exact-head hosted
full regression/review/checks/conversations remain integration gates, not
claimed by these focused runs. No family/parent/platform/release closure.

## Documentation contracts before publication

GNU44943 harvested0 at20:27Z:5/5 PASS in337.41s, one serial invocation of
issue intake, changelog assembler/fragments, native isolation and safety
traceability workflow contracts. Full86-line documentation-contracts.log
copied unedited/cmp, SHA-256
0b6f14dccb1116f48f38947db751d768ccd2641f28e5ae628e4e73449e49d642.
This validates development contracts, not release safety qualification or
independent-human sign-off. The standard native composite builds all targets
and runs the full inventory on Linux/macOS/Windows, including untouched
GETFONT smoke/localization; retain those exact-head results before merge.
