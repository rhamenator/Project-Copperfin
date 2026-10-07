# Installed VFP9 ALEN optional dimension evidence

Observed 2026-10-07 using installed VFP9 09.00.0000.7423 under the unchanged
owner-installed /home/rich/bin/vfp9-probe. Three fresh serial runs match
all 119 lines: 114 ALEN calls (57 operands on independent 3x4 and 5-element
arrays), VERSION, before/after dimensions, unchanged markers and session 1.
No binary inspection, VM, external backend or system change.

## Requirements and recovery boundary

RQ-CF-PRG-ALEN-DIMENSION-NUMERIC-001 derives from owner-admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Default COPPERFIN truncates finite Numeric and requires converted 0..2.
Exact int64/uint64 admission avoids a floating round trip. Explicit VFP9
truncates Numeric through signed low 32 bits, admitting only 0..2; Numeric
outside int64 and infinity produce the observed indefinite-zero alias.
NaN rejects in both modes as derived safety, not a native NaN observation.
Exact extended integers use exact low-32 modular policy in VFP9 mode; these
types are intentional extensions absent from VFP, not native claims.

Installed results distinguish truncation (0.5/0.9 become 0, 1.5/1.9 become 1,
2.5/2.9 become 2), negative sub-units become 0, and both-sign low-32 aliases:
4294967296/7/8 and -4294967296/-4294967295/-4294967294 become 0/1/2.
The 65536/32768 controls reject, ruling out narrower widths. Huge positive/
negative and infinite Numeric operands become 0; other converted indices
raise 11. Rejection uses the existing localized
Runtime.Prg.Expression.Error.InvalidArgument catalog and catchable error 11
before the length callback. No argument-sized allocation or loop is added.

Ordinary other coercions retain checked truncation into signed int32 and
the unchanged length callback fallback, not native type parity. Native $0.5
and Logical/NULL/Character controls reject 11; $1.5/$2.5 reach 1/2.
Native one-dimensional dimension 2 returns 0, while current Copperfin returns
1: existing #6302 owns that representation/result gap. Preserve this gap
truthfully; Numeric conversion is not authority to change array representation,
allocation/dimensions, name resolution, existence/type/arity/result policy or
other array callers.

Separate #7055 against exact main f8929616deb0c09b753671071e8718e1f2172ee4
retains the observed other-type admission gap, not an authorization to alter
those coercions inside this Numeric conversion slice.

## Native reproduction and identities

~~~sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-alen-dimension-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-alen-dimension-numeric-observation/probe.prg tests/fixtures/vfp9-alen-dimension-numeric-observation/probe.out
~~~

Source SHA-256: e06f798a31408e17de33907e414319ff057a7634cdbb96d69fcf3586dd7df839.
Output SHA-256: 20dd6160d6227845163e357eef677105dee0c8c44477b8e766356d8b48bd373a.
VR-5611-ALEN-NATIVE-001: three matching 114-call/119-line runs.

## Verification architecture

The existing numeric-behavior suite holds independent expected columns,
not values calculated with the production helper. Direct cases cover native
Numeric inputs, adjacent boundaries, NaN, exact extended integer extrema and
values beyond 2^53, and checked existing coercions. Fresh synthetic PRG cases
on both shapes compare result/error/message, independent before/after shape
and payload, selected guard cursor/count/marker, session, mode and cleanup.
One-dimensional result and other-type rows preserve current behavior solely
to prevent scope drift; they do not originate compatibility requirements.
Neighboring array tests independently exercise macro/name/member resolution.

VR-5611-ALEN-CONVERSION-001: 178 direct checks and 240 fresh cases /1,062 PRG
rows. GCC unchanged-dispatch baseline fails 109 selected output/error/message
assertions; all direct checks, ordinary-coercion and omitted-dimension controls,
and three neighbors pass. Every failed block retains identical before/after
shape/payload/cursor/session/mode/cleanup and call-state suffix (zero mismatches).
Numeric 42.71s, four suites 44.97s. Clang sanitizer unchanged-dispatch baseline
reports five earlier semantic failures then stops at expression.inl:2604:85
for 2147483648 outside the representable int range. Numeric 1.35s, three
neighbors pass, four total 29.51s. No completed sanitizer Numeric baseline or
unexecuted other-case success is claimed; the direct checks precede that stop.

~~~sh
cmake -S . -B /home/rich/temp/copperfin-alen-dimension-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-alen-dimension-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-alen-dimension-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|seek_index|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-alen-dimension-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-alen-dimension-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-alen-dimension-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|seek_index|arrays|string_math_functions)$' -j1
~~~

Run runtime suites SERIAL across build directories. Hosted cross-platform
checks and exact-head review remain PR merge gates.

## Documentation controls

DQ-5611-ALEN-CONVERSION-001: truthful native/derived boundaries, checked
pre-callback admission, serial verification and rollback. Development
procedure misuse severity is medium: unchecked conversion can crash the
runtime or select a false array shape, affecting downstream data handling.
Native-width aliases belong only to explicit VFP9 mode. Parent crash/data-
corruption hazards are mitigated by checked conversion, exact integers,
catchability and independent shape/payload/state controls.

Procedural delta: replace only the optional dimension conversion; retain
callback/array representation/allocation/name/type/arity/result paths.
Walkthrough: 1.9 selects row count; explicit VFP9 4294967298 selects columns,
while default rejects 11 before callback. Huge Numeric is a default error
and an explicit VFP9 total-count query. Rejection leaves both shapes,
markers, selected cursor and mode unchanged; normal queries still work,
then each fresh case resets mode and closes its owned cursor.

Rollback: reviewed revert with regressions retained; correct matrix/coverage/
handoff/release evidence and notify the owner before reusing incorrect guidance.
Development self-review/automated evidence must not be described as independent
human review, release qualification or full array/type/native parity.

VR-5611-ALEN-CONVERSION-002: fixed GCC 15.2 Debug passes 4/4 in 46.15s
(Numeric 43.80s). Fixed Clang 21.1.8 ASan/UBSan/float-cast-overflow passes
4/4 in 204.80s (Numeric 195.80s), without diagnostics. All 178 direct checks
and 240 fresh cases /1,062 PRG rows pass unchanged independent expectations.
Native source/output hashes remain unchanged.
VR-5611-ALEN-CONTRACTS-001: six locale-install, contributor-signoff,
changelog assembler/fragment-validation, agent-channel and native-isolation
contracts pass 6/6 initially in 5.12s and finally in 4.33s after documentation
completion; all 164 fragments are valid.

## Completed development documentation review

DV-5611-ALEN-CONVERSION-001: implementation-agent development self-review
completed 2026-10-07. Reviewed the selected helper/expression/test diff,
complete source/output and three matching native runs, all 48 independent
Numeric expected columns against installed 3x4 results (zero mismatches),
exact/NaN/adjacent boundaries, 240 fresh shape/payload/cursor/session/mode/
cleanup cases, GCC and sanitizer baseline-to-fixed evidence, catalog-backed
11, docs/22 and docs/32 bidirectional mappings, medium-severity misuse,
procedural delta, verified walkthrough and rollback. Rejected values cannot
reach the callback: admission is checked before the sole length invocation.
No argument-sized allocation/loop, array representation/allocation/name/
existence/type/arity/result callback or other callers changed. #6302 and
#7055 remain separate evidenced gaps, not rationalizations of current behavior.
NaN and exact extended integer policy remain derived, not native claims.
Git diff checks, native hashes and agent-channel verification pass.
This self-review plus automated evidence is development evidence, not
independent-human or release/native/type-completeness qualification.
