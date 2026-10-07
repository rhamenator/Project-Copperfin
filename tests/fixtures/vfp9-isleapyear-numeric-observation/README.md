# ISLEAPYEAR numeric extension boundary

## Governing derived requirements (before implementation)

RQ-CF-PRG-ISLEAPYEAR-NUMERIC-001 derives from admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, the owner's 2026-10-06 extension/judgment
steering and HZ-runtime-crash-01/HZ-data-corruption-01. Native VFP lacks this
extension: that is not a reason to drop it or invent a native wraparound rule.

ISLEAPYEAR is a pure proleptic Gregorian integer-year predicate. The public
[US Naval Observatory leap-year rule](https://aa.usno.navy.mil/faq/leap_years)
provides the positive-year divisibility rule: divisible by 4 except centuries
not divisible by 400. Extension policy applies that mathematical periodic
predicate to signed integer years, including zero/negative years; it does not
construct a Date, promise historical calendar adoption or extend Date ranges.

- Finite Numeric truncates toward zero into the signed 64-bit domain.
  Exact signed/unsigned integer inputs do not round-trip through double;
  unsigned values above INT64_MAX reject. Reject NaN/infinities/outside range
  with localized catchable error 11 before predicate evaluation.
- Both COPPERFIN and VFP9 modes use the same extension policy. No native
  ISLEAPYEAR low-32 alias/indefinite conversion exists to emulate. A legacy
  calendar rule is not a requirement for native extension syntax.
- The leap predicate repeats every 400 years. Reduce an admitted int64 year
  modulo 400 before calling the existing int-based calendar predicate; that
  bounded conversion is exact even at INT64_MIN and is not a VFP alias.
- This signed-64 pure-math envelope supports modern extended integer values
  without allocation, date construction, loops or settings mutation. It is a
  derived extension boundary, not an inference from existing unsafe code.
- Preserve ordinary existing Character/Logical/Currency coercions with checked
  truncation, global NULL propagation, omitted/extra-argument routing and all
  Date/Julian/session/settings behavior. These are preservation controls, not
  native type/arity compatibility claims.

## Independent native evidence

Three fresh serial runs through the unchanged installed /home/rich/bin/vfp9-probe
agree on all 23 lines (20 calls, VERSION, before/after guard73).
probe.prg checks ten ISLEAPYEAR call forms and ten equivalent ordinary
DAY(GOMONTH(DATE(year,1,31),1)) February lengths, including ordinary, century
and 400-year cases. The native results qualify only those equivalent legacy
consumers and extension absence, not the extension's extended-year domain.
No table/backend/VM/system change or decompilation.

Installed VERSION: Visual FoxPro 09.00.0000.7423 for Windows. All ten
ISLEAPYEAR call forms report ERR1, consistent with the previously recorded
native absence. Ten equivalent February lengths match independently explicit
Gregorian expectations (1800/1900/2026/2100/9999:28;
1996/2000/2024/2400/9996:29). Guard73 is unchanged.
Source SHA-256 cb13b81e2aaecd1f1cdb17667f6eb1be1fd0ec5be2728f2d21ee9e3ebf2cbf5c.
Output SHA-256 bb1ea694096415aedf179e0b13dcab806dc8b6973df3cb2e2106f4d9292f5efa.
VR-5611-ISLEAPYEAR-NATIVE-001: three matching23-line runs and ten equivalent
calendar comparisons, zero mismatches. No native extension-domain evidence.

~~~sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-isleapyear-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-isleapyear-numeric-observation/probe.prg tests/fixtures/vfp9-isleapyear-numeric-observation/probe.out
~~~

## Verification architecture and original-dispatch evidence

96 independent explicit direct-call checks cover ordinary/century/fractional,
signed-64 adjacent bounds, NaN/infinity, exact extended integers and ordinary
coercions. Fresh PRG rows in both modes assert result/error/message, unchanged
guard cursor/pointer/payload/session/mode/date settings and cleanup/reset;
global NULL is pinned separately. 70 fresh guarded cases/292PRGrows across
both modes. Compare original dispatch normal and
ASan/UBSan/float-cast-overflow baselines before the selected change, then serial
fixed acceptance plus date/type/NULL/string neighbors and six contracts.
Use private TMPDIR across runtime suites; no cross-build runtime overlap.
VR-5611-ISLEAPYEAR-CONVERSION-001: both original-dispatch binaries contain
the new regressions but the unchanged production cast from main
51c90be0466fdf2293ca0c6192c68c642ed5f4f1. GCC 15.2 Debug reports 74 selected
assertion failures (58 direct, 16 script), zero setup/after/cleanup/call-state
mismatches; older rows and all four neighbors pass. Numeric takes 160.18s,
five suites 161.33s. Clang 21.1.8 ASan/UBSan/float-cast-overflow aborts Numeric
in 0.03s at date_time_functions.cpp:1581:37 on 1E20 outside int. Four neighbors
pass, total 4.23s; no completed Numeric sanitizer baseline is claimed.
The complete textual negative record is original-sanitizer-failure.log
(trailing whitespace normalized), SHA-256
958718aa6205f83592a53a626e7f872701319db9d6fd7bc725ebd7b4ccd4591f.
An initial test error-accessor typo was corrected before either baseline
compiled. Existing unused-function, lambda-capture and missing-field warnings
are unrelated; the builds are not claimed warning-free.

VR-5611-ISLEAPYEAR-CONVERSION-002: fixed GCC 15.2 Debug passes all five
suites in 77.28s (Numeric 76.15s). Fixed Clang 21.1.8
ASan/UBSan/float-cast-overflow passes all five in 348.56s (Numeric 343.91s),
without diagnostics. Fresh private TMPDIRs and serial suites avoid cross-build
runtime overlap; sampled snapshots show only this task's Numeric process.
Six repository contracts pass 6/6 in 4.79s; 168 valid changelog fragments.

DQ-5611-ISLEAPYEAR-CONVERSION-001: distinguish native equivalent-calendar
evidence from derived extension policy; explain checked admission, exact
integer handling, catchability, unchanged state and serial/private-temp testing.
Development-procedure misuse severity is medium: an unchecked year can
terminate the runtime or silently misclassify a calendar predicate consumed
downstream. This bounded, non-mutating predicate is not a deployment-specific
safety assessment. Parent HZ-runtime-crash-01/HZ-data-corruption-01 mitigations
are finite/range checks, exact integers, localized catchable failure and
independent expected results/state guards.

Procedural delta map: replace only ISLEAPYEAR's direct year cast with existing
checked signed-64 admission followed by bounded modulo-400 evaluation. Native
ordinary February results remain controls, not an invented native extension
contract. Ordinary coercions and global NULL remain preservation controls;
routing, Date/Julian consumers, Date ranges, settings and sessions stay outside
this change. Build warnings unrelated to this slice remain disclosed.

Walkthrough: 2024.9 truncates to a leap year, 1900.9 to a non-leap century;
4294967300 is a non-leap century in both modes, not a hypothetical low-32 alias 4.
INT64_MIN is admitted exactly; unsigned INT64_MAX+1 and 1E300 reject error 11
before calendar evaluation. Every guarded PRG case retains leapguard/pointer1/
recordcount1/payload guard/session1 and YMD/OFF/-/1975 settings; NULL yields
NULL without error, mode resets COPPERFIN and the owned cursor closes.
Rollback: use a reviewed revert retaining regression/native evidence, correct
requirements/coverage/handoff/release guidance, and notify the owner before
reusing guidance shown to be incorrect.

DV-5611-ISLEAPYEAR-CONVERSION-001: completed 2026-10-07 implementation-agent
development self-review of source/delta/misuse/walkthrough, independently
explicit expected values and native columns, baseline failure/state audit,
normal/sanitizer acceptance and six automated contracts recorded above.
This is not independent qualified-human verification, formal certification,
release/platform qualification or native ISLEAPYEAR/type/arity parity.

## Reproduction commands

~~~sh
cmake -S . -B /home/rich/temp/copperfin-isleapyear-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-isleapyear-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_date_time_functions test_prg_engine_date_function_types test_prg_engine_null_builtins test_prg_engine_string_math_functions -j4
copperfin_leap_verify_tmp=$(mktemp -d /home/rich/temp/copperfin-isleapyear-5611-build/verify-tmp.XXXXXX)
TMPDIR="$copperfin_leap_verify_tmp" ctest --test-dir /home/rich/temp/copperfin-isleapyear-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|date_time_functions|date_function_types|null_builtins|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-isleapyear-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-isleapyear-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_date_time_functions test_prg_engine_date_function_types test_prg_engine_null_builtins test_prg_engine_string_math_functions -j4
copperfin_leap_san_verify_tmp=$(mktemp -d /home/rich/temp/copperfin-isleapyear-5611-sanitize/verify-tmp.XXXXXX)
TMPDIR="$copperfin_leap_san_verify_tmp" ctest --test-dir /home/rich/temp/copperfin-isleapyear-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|date_time_functions|date_function_types|null_builtins|string_math_functions)$' -j1
ctest --test-dir /home/rich/temp/copperfin-isleapyear-5611-build --output-on-failure -R '^test_(locale_catalog_install_contract|contributor_signoff_contract|changelog_fragment_assembler|changelog_fragments_valid|agent_channel_contract|native_test_isolation_contract)$' -j1
~~~
