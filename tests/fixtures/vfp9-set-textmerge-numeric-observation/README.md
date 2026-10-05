# Installed VFP9 SET(TEXTMERGE) Numeric-variant observations

Recovered 2026-10-05 from installed Visual FoxPro 09.00.0000.7423 under Wine,
using the existing `/home/rich/bin/vfp9-probe` runner without modifying it.
`set-textmerge.prg` is the complete clean-room source; `set-textmerge.out`
retains all 34 results plus VERSION. The probe performs read-only SET queries
in a fresh VFP process; it does not open an output destination. No VM was
started, and no installer, persistent table, system change or overlay was used.

Numeric variants truncate toward zero, then must select 1..4. Thus 3.9
selects SHOW and 4.9 selects Numeric recursion level zero, whereas zero,
sub-units, ordinary negatives and 5 raise error 11. Both oversized positive
and negative values wrap through signed low-32 bits: 4294967297 and
-4294967295 select 1; 4294967299 and -4294967293 select 3. Huge/infinite values
convert to invalid zero and raise error 11.

`RQ-CF-PRG-SET-TEXTMERGE-NUMERIC-001` under #5611/#6776 therefore requires
checked finite truncated variant 1..4 in default COPPERFIN. Only explicit
VFP9 retains low-32-bit wrapping. Exact int64/uint64 admission and NaN
rejection are derived owner safety policy, not installed-VFP9 claims.
Currency preserves existing half-away rounding behind checked signed-64-bit
admission; other coercions, omitted variants and other SET options remain
unchanged. Their type compatibility is not recovered by this Numeric probe.

The query results expose separate gap #6973 against main
`52daa2f7f6b39bfe232de3c65556be5aaf0fcdd6`: native variant 1 concatenates
delimiters without Copperfin's comma; variant 2 reports the output destination
and variant 4 returns a Numeric recursion level. Existing Copperfin variants
2/4 fall through to ordinary setting state and return Character.
This slice preserves those query-result paths; it does not rationalize them
as native requirements. Output-routing issue #6333 remains separate.

Direct callback tests isolate conversion: 33 boundary cases in each setting
(66 calls), plus seven preservation controls per setting (14 calls).
Twenty PRG rows prove catchable error 11/continued execution and the 3.9 SHOW
query in both settings. No floating-status-flag assertions are made, because
optimized non-strict FP may speculate guarded casts.

## Focused verification

Before the production change, the GCC Debug numeric-behavior regression failed
102 semantic assertions; neighboring string/math tests passed. After the change,
both suites passed in GCC 15.2 Debug and Clang 21.1.8 Debug with AddressSanitizer,
UndefinedBehaviorSanitizer and float-cast-overflow instrumentation (leak checks
disabled, UBSan halt-on-error enabled):

```sh
ctest --test-dir /home/rich/temp/copperfin-set-textmerge-5611-build --output-on-failure -j1 -R '^test_prg_engine_(numeric_behavior|string_math_functions)$'
env ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir /home/rich/temp/copperfin-set-textmerge-5611-sanitize --output-on-failure -j1 -R '^test_prg_engine_(numeric_behavior|string_math_functions)$'
```

An additional unchanged sanitizer control-flow baseline passed in 377.46
seconds. Its broader CTest invocation was then interrupted before the numeric
suite completed; this is not a fixed-build three-suite pass or a sanitizer
fail-before result. Full updated control-flow and cross-platform validation
remain hosted evidence required before release disposition.
