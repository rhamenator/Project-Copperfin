# Installed VFP9 AFONT Numeric-size observations

Recovered 2026-10-05 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the existing local `vfp9-probe` wrapper without modification.
`afont.prg` is complete clean-room source; `afont.out` retains VERSION,
the selected installed font and all 54 calls. The probe uses local arrays
only; no persistent tables, font installation, system changes or VM were used.

The selected native font was `@Droid Sans Fallback`. This scalable font returns
Logical true for every probed Numeric size. Converted -1 is distinguishable:
the array contains Numeric -1 instead of Logical true. In particular -1.5
and -1.9 select that sentinel, while negative sub-units do not. Positive
4294967295/4294967295.9 and negative -4294967297/-4294967297.9 select the
same sentinel. These observations support truncation and signed-low-32-bit
aliases; do not impose a nonnegative size domain. Most other sizes, including
huge/infinite inputs, have indistinguishable results for this font: their
exact converted values are not independently proven by these output rows.

The shipped Microsoft VFP9 SP2 [AFONT reference](https://www.vfphelp.com/help/_5WN12PPCM.htm)
describes -1 size enumeration, Logical results, scalable fonts and the fourth
size-versus-character-set flag. Return/array/font availability parity is
outside this selected conversion slice, as are second/third argument type
admission and the fourth flag. Existing issues #3252, #6955, #6958 and #6963
retain those boundaries; no duplicate issue or expanded production fix is
needed. Copperfin's current host-aware font-size list is not the native
requirement source and must not be relabeled full VFP parity.

## Selected conversion requirement

Parent `RQ-CF-PRG-NUMERIC-BEHAVIOR-001` and owner-approved #5611/#6776
authorize a bounded checked third-argument conversion at main
`3aa571eaad1dd169b97491c5c3bd260882a2d4a2`. Requirement
`RQ-CF-PRG-AFONT-SIZE-NUMERIC-001` admits finite Numeric values that truncate
into signed int32 in default COPPERFIN, including negative values. Only
explicit VFP9 retains the shared signed-low-32-bit conversion model. That
model's huge/infinite/NaN-to-zero behavior is derived shared-policy continuity,
not an independently distinguished AFONT native result. Exact int64/uint64
must avoid a floating round trip. Preserve other current coercions through
checked half-away rounding without fixing the separate type-admission issues.
Rejection must raise localized catchable error 11 before font enumeration or
array mutation (`HZ-runtime-crash-01` / `HZ-data-corruption-01`).

`checked_afont_size_argument` implements this boundary; the AFONT dispatch
checks its optional result before calling the unchanged font-array operation.
Direct helper assertions supplement the native observations with both signed
int32 edges, adjacent doubles, exact extended integers, NaN/infinities and
coercion controls. Public PRG rows compare admitted sizes to canonical inputs
and catchable rejection rows preserve array shape/content. These are current
font-result preservation checks, not recovered fixed-size-list parity.

## Verification (2026-10-05)

The initial 137 new PRG rows produce 40 assertion failures against the old
dispatch. Numeric fractions round instead of truncate, invalid values are
accepted, and rejection checks expose destination replacement. Neighboring
array and string/math suites pass at that baseline. No sanitizer diagnosis is
claimed for the old uninstrumented standard-library llround call.

The fixed regression has 198 direct helper calls (99 cases in each mode) and
137 PRG rows: the 54 retained native expressions in both modes, twelve catchable
rejection/unchanged-array checks, sixteen preservation controls and an unset-
default check. Exact helper expectations cover values that native font output
cannot distinguish, including negative -1 (not a rejection sentinel), signed
int32/int64 boundaries and exact 64-bit low bits above 2^53. No argument-sized
allocation or loop is added by the conversion.

All three focused suites pass with GCC 15.2 Debug (7.20 seconds total) and
Clang 21.1.8 ASan/UBSan/float-cast-overflow (30.63 seconds total), without
sanitizer diagnostics. These are focused local results, not completed hosted
cross-platform checks or independent human review.

To reproduce, use separate build directories outside the checkout; replace
the placeholders below with local paths. Run the focused tests sequentially
across directories because the numeric suite owns a shared scratch path.

```sh
cmake -S . -B <normal-build-dir> -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1' -DCOPPERFIN_BUILD_TESTS=ON
cmake -S . -B <sanitizer-build-dir> -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow' -DCOPPERFIN_BUILD_TESTS=ON
cmake --build <build> --target test_prg_engine_numeric_behavior test_prg_engine_arrays test_prg_engine_string_math_functions -j 6
ctest --test-dir <normal-build-dir> -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions)$' --output-on-failure
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir <sanitizer-build-dir> -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions)$' --output-on-failure
```

Build both directories by substituting each for `<build>`. Hosted Linux,
Windows and macOS checks provide broader evidence; all required exact-head
checks, automated review and resolved conversations remain merge gates.
Retain the source/output, tests and results with release evidence; remove
scratch builds after merge. No VM was started or altered.

`VR-5611-AFONT-NATIVE-001`: all 54 expressions complete in the installed probe;
retained source/output confirms fractional -1 and both-direction wrap sentinels.
To reproduce, run a local installed-VFP9 probe wrapper on `afont.prg` from
this directory and compare with `afont.out`, allowing the selected font name
and host/font-specific results to differ. Do not install/change fonts to force
this output. This recipe does not authorize system maintenance or VM changes.

`DQ-5611-AFONT-SCOPE-001` requires separating observed conversion evidence,
derived safety policy, runtime verification and known result/type/
flag gaps. `DV-5611-AFONT-SCOPE-001`: maintainer-authorized agent self-review
compared the complete source/output, shipped reference, selected dispatch and
the four admitted existing issue scopes, then checked the implemented helper,
matrix/coverage links and passing normal/instrumented regressions. This is not
independent human review.
Misuse severity is medium: treating an identical font result as proof of every
converted integer could admit incorrect aliases. Walkthrough: -1.9 and wrapped
-1 aliases are observable; huge-size exact conversion remains derived, and
current Copperfin Logical/array parity remains unclaimed. Default 0.5 becomes
0; oversized values raise error 11 without replacing the sentinel array;
wrapped aliases remain opt-in. Operators should validate sizes/catch the error
and must not infer full AFONT parity from the numeric-behavior setting.
Rollback is reverting the conversion slice while retaining regressions that
expose restored unchecked conversion. Corrections belong in changelog/release
notes. Persistent data, host fonts and system state are unchanged.

`VR-5611-AFONT-BOUNDARY-001` maps to the direct/PRG assertions;
`VR-5611-AFONT-SANITIZER-001` maps to the instrumented focused suites.
