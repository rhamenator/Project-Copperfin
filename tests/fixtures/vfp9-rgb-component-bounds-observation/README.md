# VFP9 RGB component-boundary observation

These fixtures record clean-room behavior probes run on 2026-10-04 with the
installed Visual FoxPro 9 SP2 runtime (`09.00.0000.7423`). The complete sources
were run through `~/bin/vfp9-probe` and use only documented PRG expressions and
ordinary error handling, with no binary inspection or decompilation.

The observations establish truncation toward zero, component positions with
weights 1, 256, and 65,536, error 11 for a converted component outside 0..255,
and a positive raw-value ceiling of 255 before VFP9's signed-32-bit conversion.
Negative values use that conversion first, so negative fractions above -1 and
negative multiples of 2^32 become zero, `-4294967295` becomes 1, and the
integer-indefinite results for huge negative values and negative infinity also
become zero.

Each `.prg` file is the complete probe source and its matching `.out` file is
the complete stdout.
