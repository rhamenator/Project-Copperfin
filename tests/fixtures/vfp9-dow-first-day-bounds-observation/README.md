# VFP9 DOW first-day numeric-bound observation

This fixture records a clean-room behavior probe run on 2026-10-04 with the
installed Visual FoxPro 9 SP2 runtime (`09.00.0000.7423`). The source was run
through `~/bin/vfp9-probe`; it uses only documented PRG expressions and
ordinary error handling, with no binary inspection or decompilation.

The observation establishes that `DOW(dDate, nFirstDayOfWeek)` truncates the
numeric option toward zero, accepts 0 through 7, uses `SET FDOW` when the
explicit option truncates to zero, and raises catchable error 11 when the
converted option is outside that range. VFP9 converts through a signed 32-bit
value first, so huge/non-finite values and low-32-bit zero use `SET FDOW`,
while `-4294967295` becomes the explicit value 1.

`dow-bounds.prg` is the complete probe source and `dow-bounds.out` is its
complete stdout.
