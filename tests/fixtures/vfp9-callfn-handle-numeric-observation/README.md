# Installed VFP9 / FoxTools CALLFN handle evidence

Observed 2026-10-07 using installed VFP9 09.00.0000.7423 and installed
FoxTools 9.00 under the unchanged owner-installed /home/rich/bin/vfp9-probe.
Three fresh serial final runs match all 65 lines: 58 CALLFN attempts,
VERSION, FOXTOOLS, handle 1, control, first error, retained registration and
session 1. The sole registered function is harmless lstrlenA; both control
and retained registration return 9 for Copperfin. The library is unloaded
at completion. No binary inspection, VM, external backend or system change.

## Requirements and recovery boundary

RQ-CF-PRG-CALLFN-HANDLE-NUMERIC-001 derives from owner-admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Default COPPERFIN truncates finite Numeric to signed int32, preserving exact
int64/uint64 without floating round trips and rejecting unsafe values before
registered API invocation/events. VFP9 mode truncates finite in-int64 Numeric
and uses low-16 aliases. Native live results distinguish truncation (0.5/0.9
absent, 1.5/1.9 live) and both-sign aliases: 65537, -65535, 4294967297,
-4294967295, 2147483649 and -2147483647. Absent 32769/-32767 controls rule
out narrower alias widths. A low-16 unsigned representation for other absent
indices is explicit derived modular policy, not a recovered native index.

Exact extended integers use exact low-16 modular policy in VFP9 mode; native
VFP Numeric has no distinct extended integer types. NaN, infinity and Numeric
outside int64 reject in both modes. Native absent errors for those operands
do not establish an indefinite converted index. Rejection uses localized
catchable 1098 and safe original Numeric diagnostics, bypassing unchanged
formatter gap #6997. Existing other coercions preserve checked half-away
rounding, not native type admission.

Native unregistered handles raise 1098 (first message Unregistered function);
seven Currency/Logical/NULL/Character controls raise 9. Separate #7050 against
exact main 9748f049fd73ac14614e1eb246e10039cb56ae50 retains native callback/
type/arity admission; current absent -1/coercions are preservation controls,
not recovered requirements. REGFN/session/lifecycle/native ABI/return behavior,
callbacks and other conversion sites remain outside this bounded slice.

## Native reproduction and identities

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-callfn-handle-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-callfn-handle-numeric-observation/probe.prg tests/fixtures/vfp9-callfn-handle-numeric-observation/probe.out
```

Source SHA-256: 413b2b7ab64a014a729a33fa9c891bfd16932046274f73fa2ab3629108385de3.
Output SHA-256: 7d03d15cbe6351fdd60b434a480551582d0ecf61e4b38b6bfa11bd1bf3f2a58b.
The raw FOXTOOLS version line includes one observed trailing space; this
fixture's exact-file Git attribute preserves it without relaxing code/docs
whitespace checks or normalizing the retained native bytes.
VR-5611-CALLFN-NATIVE-001: three matching final 58-call/65-line runs.
The earlier 56-call/63-line runs preceded the narrower-width controls;
only the final source/output above is the complete fixture.

## Verification architecture and retained results

166 direct independent checked-column tests cover 51 Numeric cases in both
modes plus NaN, adjacent integer ceilings, exact extrema/above-2^53 values
and checked existing coercions. 248 fresh synthetic cases / 1,008 PRG rows
compare live-session and empty-other-session routing, status/error/original
diagnostics, payload length, selected guard shape/payload/count, retained
registration, unchanged next handle, exact invocation/registration events,
mode/cursor/library cleanup. Sixteen message rows cover signed huge/infinite
original Numeric text. Native ABI is not emulated by these synthetic controls.

VR-5611-CALLFN-CONVERSION-001: GCC unchanged-dispatch baseline fails exactly
101 selected assertions (82 result/state/diagnostic comparisons, 19 successful
invocation counts). Numeric 40.97s/four suites 43.25s. All direct/helper and
existing-coercion controls and three neighbors pass. All 82 failed output
blocks retain identical setup, guard/session state, retained-registration/
counter and cleanup rows. Clang ASan/UBSan/float-cast-overflow unchanged-
dispatch baseline fails the same 101 assertions in Numeric 188.91s/four
suites 198.64s, with the same zero setup/guard/session/retained-registration/
counter/cleanup mismatches and all direct/coercion/neighbor controls passing.
Neither baseline reports sanitizer diagnostics; no library llround
instrumentation claim. Runtime suites ran SERIAL across build directories.

VR-5611-CALLFN-CONVERSION-002: fixed GCC 15.2 Debug four suites pass 4/4 in
44.08s (Numeric 41.81s). Fixed Clang 21.1.8 ASan/UBSan/float-cast-overflow
four suites pass 4/4 in 203.21s (Numeric 193.80s), without diagnostics.
All 166 direct checks and 248 cases/1,008 PRG rows pass unchanged independent
expectations; native hashes above remain unchanged.
VR-5611-CALLFN-CONTRACTS-001: six locale-install, contributor-signoff,
changelog assembler/fragment-validation, agent-channel and native-isolation
contracts pass 6/6 in 4.38s; all 163 fragments are valid.

```sh
cmake -S . -B /home/rich/temp/copperfin-callfn-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-callfn-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-callfn-handle-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|seek_index|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-callfn-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-callfn-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-callfn-handle-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|seek_index|arrays|string_math_functions)$' -j1
```

## Documentation controls

DQ-5611-CALLFN-CONVERSION-001: truthful native/derived boundaries, checked
pre-invocation admission, serial verification and rollback. Development
procedure misuse severity is medium: rounded/wrapped handles can invoke the
wrong registered API or misroute a session-scoped registration. Native-width
alias policy belongs only to explicit VFP9 mode. Parent runtime-crash/data-
corruption hazards are mitigated by checked conversion, exact integer handling,
catchability and independent registration/session/argument/event controls.

Procedural delta: replace only first-argument conversion; retain callback,
REGFN/lifecycle/session/native ABI/type/arity/return paths. Walkthrough:
1.9 selects live handle 1; explicit VFP9 65537 aliases 1, default retains
65537 as an absent handle. Unsafe operands raise 1098 before invocation,
preserving guard data and registrations; entering an empty session cannot
reach session 1's registration. Canonical calls still work afterwards and
next registration is handle 2. Each case resets mode and closes owned cursors.
These outcomes pass the retained automated baseline-to-fixed verification.

Rollback: reviewed revert with regressions retained; correct matrix/coverage/
handoff/release evidence and notify the owner before reusing incorrect guidance.
Development self-review/automated evidence must not be described as independent
human review, release qualification or real native ABI qualification.

## Completed development documentation review

DV-5611-CALLFN-CONVERSION-001: implementation-agent development self-review
completed 2026-10-07. Reviewed the selected expression/helper/test diff,
complete source/output and three matching native runs, exact independent
checked columns, setup/guard/session/registration/counter/cleanup controls,
baseline-to-fixed results, docs/22 and docs/32 bidirectional mappings,
original Numeric diagnostics, medium-severity misuse, procedural delta,
walkthrough and rollback. Production edits are confined to this conversion;
callback/REGFN/session/lifecycle/native ABI/type/arity/return paths are unchanged.
#7050 remains a truthful separate native admission gap, not justification for
existing callback/coercion behavior and not authority to expand this slice.
NaN/out-of-int64 rejection, absent low-16 indices and exact extended integers
remain explicitly derived, not claimed as recovered native observations.
The native output's exact-file whitespace attribute preserves its observed
FOXTOOLS trailing byte; code/document checks remain enforced. Git diff checks
and agent-channel verification pass. This self-review plus automated evidence
is development evidence, not independent-human or release qualification.
