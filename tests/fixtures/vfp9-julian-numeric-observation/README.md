# JTOD/JTOT numeric extension boundary

## Governing derived requirements (before implementation)

RQ-CF-PRG-JULIAN-NUMERIC-001 derives from owner-admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, the owner's 2026-10-06 extension/judgment
steering, the .NET-facing product intent and HZ-runtime-crash-01/
HZ-data-corruption-01. Requirements do not derive from the existing cast.

The [US Naval Observatory Gregorian/Julian conversion reference](https://aa.usno.navy.mil/faq/JD_formula)
defines an integer Julian day at noon for a Gregorian civil date (1970-01-01
is 2440588). This extension uses that integer day label to identify the whole
civil date; JTOT returns that date at wall-clock midnight, not an astronomical
noon instant. Fractional Numeric operands truncate toward zero; there is no
fractional-day/timezone/time-of-day conversion or historical calendar adoption
switch. The astronomical and civil interpretations are deliberately distinct.

Borrow [Microsoft's DateTime interoperability envelope](https://learn.microsoft.com/en-us/dotnet/api/system.datetime?view=net-10.0):
proleptic Gregorian years 0001 through 9999. Independent calendar expectations
are 1721426 for 0001-01-01 and 5373484 for 9999-12-31. The public USNO
1970 anchor plus independently evaluated ECMAScript UTC civil-day offsets
cross-check ten explicit date/JDN pairs, without a Copperfin calendar helper.
Installed VFP equivalent Date and midnight DateTime observations corroborate
the calendar values, not nonexistent native Julian extension semantics.

| Gregorian civil date | Integer Julian day | Equivalent native controls |
| --- | ---: | --- |
| 0001-01-01 | 1721426 | Date and midnight DateTime |
| 0100-01-01 | 1757585 | Date and midnight DateTime |
| 1601-01-01 | 2305814 | Date and midnight DateTime |
| 1900-02-28 | 2415079 | Date and midnight DateTime |
| 1900-03-01 | 2415080 | Date and midnight DateTime |
| 1970-01-01 | 2440588 | Date and midnight DateTime |
| 2000-01-01 | 2451545 | Date and midnight DateTime |
| 2024-02-29 | 2460370 | Date and midnight DateTime |
| 2026-04-18 | 2461149 | Date and midnight DateTime |
| 9999-12-31 | 5373484 | Date and midnight DateTime |

- Finite Numeric truncates into the signed-64 integer domain; exact signed/
  unsigned values avoid a double round trip. Unsigned above INT64_MAX,
  NaN/infinities and outside signed64 reject localized catchable error 11.
- Both COPPERFIN and VFP9 modes use the same derived extension policy; native
  absence does not create a low-32 alias. Admitted integer days outside the
  civil-date envelope produce a typed empty Date or DateTime, even outside int32.
  The parent permits a stable documented safe return; this preserves the
  useful empty-value idiom corroborated by native CTOD/CTOT invalid-date controls.
- Narrow to int only after proving it representable, then retain the existing
  bounded Gregorian converter and date-format callbacks. No argument-sized
  allocation/loop, timezone conversion, backend/storage changes or state mutation.
- Ordinary existing Character/Logical/Currency coercions and global NULL
  propagation are preservation controls, not native type/arity compatibility
  claims. Date constructors/ranges/settings/formatting, DTOJ/TTOJ, stored-millis
  conversion, omitted/extra routing and #4116 format work remain separate.

## Independent native evidence

The final probe contains 44 calls plus VERSION/before/after guard31. It tests
20 JTOD/JTOT forms, DTOJ/TTOJ absence and 22 independent Date/DateTime/empty
controls. A shorter 36-line pilot was expanded with midnight DateTime controls
before pinning the final source/output. Installed VFP9 7423 reports ERR1 for
the 22 extension forms. Three fresh serial final runs matched all 47 lines:
22 explicit equivalent-calendar column comparisons, zero mismatches, and
guard31 unchanged. These are not native Julian argument-domain observations.

Final source SHA-256: `8102ab95d09ca84f7657d273f2d56c9355c3566dd6495c4394c6f5d039556428`.
Final output SHA-256: `ffbf05aad93043221663016aadb4365b2e5ae39b42d1ce447acc722b18231c2e`.

## Verification architecture

236 direct calls in both modes and both functions pin explicit calendar
pairs, fractional/adjacent calendar and signed64 bounds, non-finite inputs,
exact extended integers, empty typed values and ordinary coercion controls.
156 fresh guarded PRG cases (648 rows) compare result/error/message and unchanged
cursor/pointer/payload/session/mode/date settings/cleanup, including global NULL.
Run new regressions with original dispatch before the selected source fix;
then serial GCC and Clang ASan/UBSan/float-cast-overflow focused acceptance plus
date, date-type, NULL and string/math neighbors and six repository contracts.
Use fresh private TMPDIRs; no cross-build runtime suite overlap.
Development acceptance is recorded below; hosted checks and exact-head review
remain separate merge gates.

VR-5611-JULIAN-CONVERSION-001: new regressions with unchanged dispatch from
main `8183da5a64c3100bfac33a8bffd0bc25b1b7ec7a`. GCC 15.2 Debug reports 120
selected assertions (96 direct, 24 script); zero setup/after/cleanup/call-state
mismatches, older rows and all four neighbors pass. Numeric takes 78.99s,
five suites 80.15s. Clang 21.1.8 ASan/UBSan/float-cast-overflow aborts Numeric
in 0.03s at date_time_functions.cpp:1561:39 on 2147483648 outside int;
four neighbors pass, total 4.51s. Not a completed Numeric sanitizer baseline.
The complete original-sanitizer-failure.log has textual trailing whitespace
normalized; SHA-256 `2a7193fe46bf61ad864917790566eacb14ae2dea796388ca4a0ffd5fa73ae959`.
Existing unused-function/lambda-capture/missing-field build warnings are
unrelated and retained; builds are not claimed warning-free.

VR-5611-JULIAN-NATIVE-001: the three matching final installed VFP9 runs and
22 literal equivalent-calendar comparisons above pass with guard31 unchanged;
extension absence is retained separately from derived Julian admission.

VR-5611-JULIAN-CONVERSION-002: fixed GCC 15.2 Debug passes all five focused
suites in 117.35s (Numeric 113.12s). Fixed Clang 21.1.8
ASan/UBSan/float-cast-overflow passes all five in 358.56s (Numeric 353.93s),
without diagnostics. Fresh private TMPDIRs and serial suites avoid cross-build
runtime overlap. Six repository contracts pass 6/6 in 4.43s; 169 valid changelog
fragments. Exact-head hosted CI/review remain separate merge gates.

DQ-5611-JULIAN-CONVERSION-001: distinguish equivalent native calendar/empty
observations, public mathematical provenance and derived extension policy;
document admission, empty versus error, exact integer handling, midnight/civil
interpretation and reproducible serial/private-temp verification.
Development-procedure misuse severity is medium: an unchecked argument can
terminate the runtime or produce an incorrect calendar value consumed
downstream. This bounded non-mutating conversion is not a deployment-specific
safety assessment. HZ-runtime-crash-01/HZ-data-corruption-01 mitigations are
finite/range checks before narrowing, exact integers, localized catchable
failure, independent calendar literals and unchanged-state guards.

Procedural delta map: replace only the shared JTOD/JTOT direct argument cast
with existing checked signed64 admission, proven-safe int narrowing, then the
unchanged bounded Gregorian converter and formatting callbacks. Both modes
share the deliberate extension contract. The existing Gregorian envelope is
verified against independently derived requirements, not used as its own
source. DTOJ/TTOJ arithmetic/storage, Date constructors/settings/ranges,
type/arity/global NULL routing and #4116 formatting are unchanged.

Walkthrough: 2461149.9 truncates to 2026-04-18; JTOT adds midnight, not
fractional-day time. 1900-02-28/1900-03-01 are consecutive Gregorian dates,
whereas 2024-02-29 is a leap day. 1721426 and 5373484 are valid calendar
endpoints; the integers immediately outside them are typed empty values.
2147483648 and exact INT64_MIN are admitted but outside the calendar and
return typed empty, without unsafe int narrowing. Unsigned INT64_MAX+1,
NaN/infinity and 1E300 reject localized error 11 before formatting.
236 direct calls and all 156 fresh guarded PRG cases pass: julianguard/pointer1/
count1/payload guard/session1 and YMD/OFF/-/1975 settings remain unchanged.
NULL yields NULL without error. Cleanup verifies the owned cursor is closed,
the alias is empty and numeric mode is COPPERFIN; it also executes reset
commands for MDY/ON/'/'/1950, without separately asserting those four post-reset
settings. Direct flavors/midnight payloads and PRG calendar components are
pinned separately; CENTURY OFF display does not change the PRG calendar result.
Rollback: use a reviewed revert preserving regression/native evidence, correct
requirements/coverage/handoff/release guidance, and notify the owner before
reusing any guidance shown to be incorrect.

DV-5611-JULIAN-CONVERSION-001: completed 2026-10-07 implementation-agent
development self-review of source/delta/misuse/walkthrough, explicit expected
calendar/admission values, native columns and independent UTC offsets,
original-dispatch failure/state audit, normal/sanitizer acceptance and six
automated contracts above. This is not independent qualified-human
verification, certification, release/platform qualification or native
JTOD/JTOT/type/arity parity.

## Reproduction commands

Run from the slice worktree. Build jobs may overlap, but run every Copperfin
runtime suite serially across all build directories with a fresh private TMPDIR.
For a negative replay, use the original main dispatch with these regressions,
not a previously fixed binary. The native fixture is an unchanged installed
VFP9 probe, not Copperfin-generated oracle output.

~~~sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-julian-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-julian-numeric-observation/probe.prg tests/fixtures/vfp9-julian-numeric-observation/probe.out
cmake -S . -B /home/rich/temp/copperfin-julian-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-julian-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_date_time_functions test_prg_engine_date_function_types test_prg_engine_null_builtins test_prg_engine_string_math_functions -j4
copperfin_julian_verify_tmp=$(mktemp -d /home/rich/temp/copperfin-julian-5611-build/verify-tmp.XXXXXX)
TMPDIR="$copperfin_julian_verify_tmp" ctest --test-dir /home/rich/temp/copperfin-julian-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|date_time_functions|date_function_types|null_builtins|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-julian-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-julian-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_date_time_functions test_prg_engine_date_function_types test_prg_engine_null_builtins test_prg_engine_string_math_functions -j4
copperfin_julian_san_verify_tmp=$(mktemp -d /home/rich/temp/copperfin-julian-5611-sanitize/verify-tmp.XXXXXX)
TMPDIR="$copperfin_julian_san_verify_tmp" ctest --test-dir /home/rich/temp/copperfin-julian-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|date_time_functions|date_function_types|null_builtins|string_math_functions)$' -j1
ctest --test-dir /home/rich/temp/copperfin-julian-5611-build --output-on-failure -R '^test_(locale_catalog_install_contract|contributor_signoff_contract|changelog_fragment_assembler|changelog_fragments_valid|agent_channel_contract|native_test_isolation_contract)$' -j1
~~~
