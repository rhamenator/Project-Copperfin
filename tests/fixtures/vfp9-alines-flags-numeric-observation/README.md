# Installed VFP9 ALINES Numeric-flag observations

Recovered 2026-10-05 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the existing `/home/rich/bin/vfp9-probe` without modification.
`alines.prg` is complete clean-room source; `alines.out` retains all 36 results
plus VERSION. The fresh process uses a local in-memory array and short text;
no persistent table, installer/system change or VM was used.

Native Numeric flags truncate toward zero, then require converted 0..31.
Negative sub-units (including -0.9) become zero; 31.9 becomes 31. Positive and
negative oversized inputs keep signed low-32 bits; huge/infinite inputs become
flag zero. The first array element distinguishes flag 1's trim behavior from
flag zero, rather than merely asserting successful completion.

RQ-CF-PRG-ALINES-FLAGS-NUMERIC-001 implements checked finite values truncating
to 0..31 in default COPPERFIN; only explicit VFP9 retains oversized aliases
and huge/infinite-to-zero quirks. Exact int64/uint64 and NaN behavior are derived
owner safety policy, not native claims. Invalid admission precedes output
array mutation (HZ-runtime-crash-01 / HZ-data-corruption-01).

Only Numeric/exact-integer flag conversion is recovered here. Existing other
coercions need checked preservation rounding, not inferred native type parity.
Splitting, parse-token behavior, array shape/content semantics and other array
function conversions are outside this slice. No completeness claim is made for
ALINES' full flag semantics.

## Verification (2026-10-05)

The slice starts at main `91467cdc9219d0f0d579d3ee2c7a05ff6e6187d7`.
The initial 92 new PRG rows produce 52 semantic failures against its old
dispatch: fractions round instead of truncate, invalid flags are admitted,
and rejected-flag expectations reveal destination-array replacement.
The unchanged neighboring array and string/math suites pass at that baseline.
This is semantic fail-before evidence; no sanitizer diagnosis is claimed for
the old, uninstrumented standard-library `llround` call.

The fixed regression has 138 direct helper calls (69 cases in each mode) and
92 PRG rows: all 36 native observations in both modes, eight catchable
rejection/unchanged-array checks, and twelve omitted-flag/custom-delimiter/
other-coercion controls. Adjacent Numeric boundaries, NaN, exact extended
integers, and checked non-Numeric rounding are tested directly.
All three focused suites pass with GCC 15.2 Debug (2.56 seconds total) and
Clang 21.1.8 ASan/UBSan/float-cast-overflow (10.13 seconds total), with no
sanitizer diagnostics.

Normal build: `/home/rich/temp/copperfin-alines-flags-5611-build`, Ninja,
`-DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'`.
Sanitizer build: `/home/rich/temp/copperfin-alines-flags-5611-sanitize`,
Clang/Clang++, Debug `-O0 -g1 -fsanitize=address,undefined,float-cast-overflow
-fno-omit-frame-pointer` for C/C++, with the same sanitizer linker flags.
Both build these targets:

```sh
cmake --build <build> --target test_prg_engine_numeric_behavior test_prg_engine_arrays test_prg_engine_string_math_functions -j 6
ctest --test-dir <build> -R '^test_prg_engine_(numeric_behavior|arrays|string_math_functions)$' --output-on-failure
```

For the sanitizer CTest run, set `ASAN_OPTIONS=detect_leaks=0` and
`UBSAN_OPTIONS=halt_on_error=1`. Full updated native/platform qualification
remains hosted PR CI; focused local results do not stand in for it.
Retain this fixture with release source/evidence. Scratch builds are removed
after merge; native observations and regression expectations remain tracked.
Rollback is reverting the slice; the retained tests expose restored unchecked
conversion. Applications should validate flags and catch error 11; selecting
VFP9 mode opts into aliases, not complete ALINES semantic parity.
