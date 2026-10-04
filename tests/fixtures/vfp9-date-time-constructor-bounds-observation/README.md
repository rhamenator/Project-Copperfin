# VFP9 DATE/DATETIME constructor numeric-bound observation

This fixture records a clean-room behavior probe run on 2026-10-04 with the
installed Visual FoxPro 9 SP2 runtime (`09.00.0000.7423`). The source was run
through `~/bin/vfp9-probe`; it uses only documented PRG expressions and
ordinary error handling, with no binary inspection or decompilation.

The observation establishes that `DATE()` and `DATETIME()` truncate fractional
components toward zero, accept years 100 through 9999, and raise catchable
error 11 for invalid calendar/time components. Positive out-of-range values
are rejected. Negative values expose VFP9's signed-32-bit conversion quirk:
for example, `-4294967196` becomes year 100, `-4294967295` becomes hour 1,
and a negative integer-indefinite value becomes zero (valid for a time
component, invalid for a year/month/day).

`constructor-bounds.prg` is the complete probe source and
`constructor-bounds.out` is its complete stdout.
