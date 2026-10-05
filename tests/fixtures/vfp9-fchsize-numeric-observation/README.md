# Installed VFP9 FCHSIZE numeric observations

Recovered on 2026-10-05 with installed VFP9 SP2 under Wine using
`/home/rich/bin/vfp9-probe fchsize-numeric.prg`. The retained 161 output lines
cover 41 arguments on read-only handles, 19 safe arguments on writable handles,
and all 41 arguments with an invalid handle (101 calls total). Each file call
starts with a unique six-byte temporary file positioned at 3; results, FERROR,
position and final length are retained. Handles and temporary files are cleaned
up. Large boundary/wrap/huge operands never get writable handles, so this probe
cannot create a giant file. No Windows VM was used or modified.

Numeric sizes truncate toward zero and convert through signed low-32-bit
conversion. Negative converted sizes raise error 11; negative sub-unit Numeric
values and huge/infinite values convert to zero. Positive wrapping and negative
wrapping values can become admissible nonnegative sizes. Currency rejects
nonzero sub-unit values and whole units outside [-4294967296, 4294967295],
then uses the same low bits and rejects a negative converted size. Logical,
Character, NULL and Empty sizes raise error 11. Size errors precede invalid
handle lookup. Rejected calls preserve file position and bytes; successful
small resizes also preserve position, even past the new EOF.

RQ-CF-PRG-FCHSIZE-NUMERIC-BOUNDS-001 derives default COPPERFIN admission of
exact numeric integers in [0, 2147483647] from the owner policy in #5611/#6776.
Both modes reject NaN as a derived safety policy (no retained installed-VFP9
expression), and extended int64/uint64 values are converted without a double
detour. Invalid sizes are rejected before lookup, flush or resize, preserving
FERROR, bytes and position. FSEEK shares the checked position conversion;
FCHSIZE adds the nonnegative size restriction. These quirks are opt-in through
SET NUMERICBEHAVIOR TO VFP9.

The separate return-value gap #5912 remains: installed VFP9 returns the new
size on success, whereas Copperfin still returns zero. This slice corrects
negative-size admission but does not change the existing success/failure
returns or platform filesystem-error mapping. Read-only FERROR 29 in this
Wine observation is not a claim about all Copperfin host platforms.
