# VFP9 WEEK option-semantics observation

These fixtures record clean-room behavior probes run on 2026-10-04 with the
installed Visual FoxPro 9 SP2 runtime (`09.00.0000.7423`). The sources were
run through `~/bin/vfp9-probe`; they use only documented PRG expressions and
ordinary error handling, with no binary inspection or decompilation.

The observations establish the documented argument order
`WEEK(date, nFirstWeek, nFirstDayOfWeek)`, the 0-through-3 and 0-through-7
ranges, truncation toward zero, and error 11 for invalid converted values.
Omitted options independently default to 1, while an explicit zero uses the
current `SET FWEEK` or `SET FDOW` value. VFP9 compatibility conversion goes
through a signed 32-bit value first.

The retained date-boundary rows also distinguish mode 2 (the first week with
at least four days in the year) from mode 3 (the first full seven-day week) and
show that weeks spanning either year boundary can belong to the adjacent
year's week numbering.

Each `.prg` file is the complete probe source and its matching `.out` file
is the complete stdout.
