# Installed VFP9 ADIR Numeric-display-flag observations

Recovered 2026-10-05 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the existing `/home/rich/bin/vfp9-probe` without modification.
`adir.prg` is complete clean-room source; `adir.out` retains all 46 results
plus VERSION. `MiXeD.txt` is a supplied harmless filename fixture. The fresh
process enumerates only that fixture into a local array; no persistent table,
installer/system change or VM was used.

Native Numeric display flags truncate toward zero, then require converted
0..3. Negative sub-units become zero; 3.9 becomes 3. Signed low-32-bit aliases
from both directions are retained; huge/infinite inputs become flag zero.
The printed filename distinguishes mode zero from mode one in this native
environment. In particular, native flag 3 is valid; do not guess a 0..2 limit.

RQ-CF-PRG-ADIR-DISPLAY-NUMERIC-001 implements checked finite values
truncating to 0..3 in default COPPERFIN; only explicit VFP9 retains oversized
aliases and huge/infinite-to-zero quirks. Exact int64/uint64 and NaN are derived
owner safety policy, not native claims. Invalid admission must precede
directory enumeration and output-array mutation (HZ-runtime-crash-01 /
HZ-data-corruption-01). Preserve existing other coercions through checked
rounding without inferring native type parity.

Only the optional Numeric/exact-integer display-flag conversion is selected.
Enumeration, skeleton/attributes, filename/8.3 rendering, volume-label
behavior, output-array semantics, AFONT and other array conversions are
outside this slice. Native flag-two filename text is platform-specific
observation, not a newly selected cross-platform rendering contract.

## Verification (2026-10-05)

The slice starts at main `2a0c811dc422480402d325c3d11afb18457eec0d`.
The initial 114 new PRG rows produce 60 semantic failures against the old
dispatch: fractions round instead of truncate, invalid flags are accepted,
and rejection checks expose destination-array replacement. The neighboring
array and string/math suites pass at that baseline. No sanitizer diagnosis
is claimed for the old, uninstrumented standard-library llround call.

The fixed regression has 170 direct helper calls (85 cases in each mode) and
114 PRG rows: all 46 native observations in both modes, ten catchable rejection/
unchanged-array checks, and twelve omitted-flag/other-coercion/no-match controls.
Admitted Numeric rows compare count, column count and filename with the
canonical converted flag, preserving the platform's existing rendering rather
than inventing flag-two parity. Direct coverage asserts the exact converted
flag even where two flags happen to render the same filename. It adds adjacent
domain/int64 boundaries, NaN and exact extended integer values above 2^53.

All three focused suites pass with GCC 15.2 Debug (2.70 seconds total) and
Clang 21.1.8 ASan/UBSan/float-cast-overflow (11.07 seconds total), without
sanitizer diagnostics. Normal build:
`/home/rich/temp/copperfin-adir-display-5611-build`, Ninja, Debug `-O0 -g1`.
Instrumented build: `/home/rich/temp/copperfin-adir-display-5611-sanitize`,
Clang/Clang++, Debug `-O0 -g1 -fsanitize=address,undefined,float-cast-overflow
-fno-omit-frame-pointer` for C/C++, with the same sanitizer linker flags.
Both build and run these targets:

```sh
cmake --build <build> --target test_prg_engine_numeric_behavior test_prg_engine_arrays test_prg_engine_string_math_functions -j 6
ctest --test-dir <build> -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions)$' --output-on-failure
```

The instrumented invocation sets `ASAN_OPTIONS=detect_leaks=0` and
`UBSAN_OPTIONS=halt_on_error=1`. Broader Linux/Windows/macOS validation and
exact-head automated review remain PR merge gates. Retain this source, output,
tests and result summary with release evidence; remove scratch builds after
merge. No VM was started or altered.

Verification IDs: `VR-5611-ADIR-NATIVE-001`, `VR-5611-ADIR-BOUNDARY-001`, and
`VR-5611-ADIR-SANITIZER-001` map respectively to the retained native table,
both-mode direct/PRG regressions and focused instrumented results above.
`DQ-5611-ADIR-SCOPE-001` requires docs to distinguish safe defaults, opt-in
aliases, derived exact/NaN policy and unrecovered rendering/type semantics.
`DV-5611-ADIR-SCOPE-001` compares fixture, matrix, coverage and handoff with
the 46 observations and passing direct/PRG assertions. Maintainer-authorized
agent self-review and automated verification are recorded, not independent
human sign-off. Severity is medium for this bounded in-memory conversion/docs
slice. Walkthrough: -0.9/3.9 become 0/3; wrapped aliases are opt-in; invalid
converted flags raise error 11 while retaining the sentinel array. Operators
should validate flags/catch that error and must not interpret VFP9 mode as
full ADIR parity. Rollback is reverting this slice, with its regressions
exposing restored unchecked conversion; any correction belongs in changelog/
release notes for users relying on this boundary. No new argument-sized
allocation or loop is added; persistent data/system behavior is unchanged.
