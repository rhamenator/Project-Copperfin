# VFP9 Currency `MOD()` Numeric-Divisor Observation

This fixture records a controlled run against installed Visual FoxPro 9 SP2
(`09.00.0000.7423`) on 2026-10-04. The probe was executed through the local
`vfp9-probe` wrapper on the preserved Windows test VM. It observes public
language behavior only; no binary inspection, disassembly, or decompilation
was used.

The matrix distinguishes exact Currency divisors from Numeric divisors below
the Currency grid, ordinary five-decimal divisors, values around the scaled
signed-64-bit boundary, huge finite values, and computed infinities. It shows
that VFP9 computes with the Numeric divisor's precision, returns Currency,
rounds that result to four places, follows the divisor's sign, and exposes an
integer-indefinite zero for non-finite or out-of-Currency results.

`currency-mod-divisor.prg` is the exact source and
`currency-mod-divisor.out` is the captured console output.
