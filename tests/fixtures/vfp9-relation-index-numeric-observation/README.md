# Installed VFP9 RELATION/TARGET Numeric-index observations

Recovered 2026-10-05 on the original `copperfin-access365-win11` using
Visual FoxPro 09.00.0000.7423 through PowerShell COM over the existing SSH key.
`relation-index.ps1` is the complete probe; `relation-index.out` retains all
68 results (34 arguments per function). Transport CLIXML/progress markup was
removed and CRLF normalized; result contents are unchanged. This uses only
temporary in-memory cursors in a new VFP process, released in finally.
No install, OS change, persistent table write, VM overlay or clone was needed.
The VM was gracefully shut down afterward.

Both functions truncate positive Numeric fractions toward zero, but validate
the raw positive value against the inclusive 9999 ceiling first: 9998.9
succeeds; 9999.49 and 9999.9 are error 11. Zero, sub-unit and ordinary negative
values are error 11. Oversized positive values do not wrap to an index.
Negative Numeric values convert through signed low-32 bits: -4294967295
selects index 1, -4294967294 selects 2, -4294957297 selects 9999, and a converted
10000 is rejected. Huge values and both infinities raise error 11.

`RQ-CF-PRG-RELATION-INDEX-NUMERIC-001` under #5611/#6776 therefore uses checked
1..9999 admission with truncation in default COPPERFIN; explicit VFP9 retains
the negative wrapping quirk. NaN and Copperfin exact int64/uint64 admission
are derived safety policy rather than native claims. Other coercions keep
existing rounding behind a checked signed-64-bit boundary; their type parity
was not probed. Optional work-area designators and cursor state are unchanged.

The probe also shows newly added relations first (ID+1 / PROBECHILDB), unlike
main `09555b5502a517777642fe8bf67411a90f66164c`'s insertion-order regression.
That separate relation-order recovery gap is #6971. Direct callback tests
verify converted indexes/designators without rationalizing current relation
order as a native requirement; the neighboring relation suite guards its
unchanged behavior.

An earlier Wine PRG attempt stalled even on the first valid control. Its
partial header is not compatibility evidence; the failed scratch source,
output and generated FXP were removed. The shared Wine runner was unchanged.
All compatibility results above are from the completed Windows COM probe.

## Verification

`test_relation_index_direct_numeric_boundaries` exercises 37 cases across
both functions and settings (148 calls), capturing the exact converted index
and optional designator and forbidding relation queries for rejected indexes.
Forty PRG rows check localized catchable error 11 and continued execution.
The original dispatch reports 208 semantic failures; neighboring relation and
string/math suites pass. After the fix, all three suites pass with GCC 15.2
Debug and Clang 21.1.8 ASan/UBSan/float-cast-overflow Debug:

```sh
ctest --test-dir /home/rich/temp/copperfin-relation-index-5611-build --output-on-failure -j1 -R '^test_prg_engine_(numeric_behavior|relations|string_math_functions)$'
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir /home/rich/temp/copperfin-relation-index-5611-sanitize --output-on-failure -j1 -R '^test_prg_engine_(numeric_behavior|relations|string_math_functions)$'
```

No sanitizer diagnostic is claimed for the former uninstrumented `llround`
library conversion. No floating-status-flag assertions are made: non-strict
optimized FP may speculate guarded casts. Hosted PR checks provide broader
Windows/macOS/Linux evidence. Reverting this bounded slice is the rollback;
the retained boundary tests identify restored unsafe conversion.
