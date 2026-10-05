# VFP9 RAND seed-boundary observation

These fixtures record a clean-room behavior probe run on 2026-10-04 with the
installed Visual FoxPro 9 SP2 runtime (`09.00.0000.7423`). The complete source
was run through `~/bin/vfp9-probe` and uses only ordinary PRG expressions and
error handling, with no binary inspection or decompilation.

The observation establishes that `RAND(nSeed)` truncates positive fractional
seeds toward zero, uses all 32 low bits of a positive seed, maps positive
values outside signed 64-bit range to low bits zero, and treats zero plus every
negative seed as the same reset request. Each call was followed by `RAND()` and
then repeated, proving that the first two values of each recovered sequence are
deterministic rather than a coincidental single-value match.

`rand-seed.prg` is the complete probe source and `rand-seed.out` is its complete
output.
