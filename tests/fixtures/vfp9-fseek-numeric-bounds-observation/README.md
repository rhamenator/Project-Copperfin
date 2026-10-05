# Installed VFP9 FSEEK numeric observations

Recovered on 2026-10-05 with installed VFP9 SP2 under Wine, using
`/home/rich/bin/vfp9-probe fseek-numeric-bounds.prg`. The probe creates a
unique six-byte temporary file, resets its position to 3 before each call,
retains the seek result, FERROR, and resulting position, and closes/removes the
file. It does not allocate large files: positioning past EOF writes no bytes.

Numeric offsets truncate and use signed low-32-bit conversion, including
out-of-int64 and infinite values converting to zero. Currency rejects nonzero
sub-unit values and whole units outside [-4294967296, 4294967295], then uses
the same low bits. Origins have an additional positive ceiling: raw Numeric
above 2 or whole Currency above 2 raises error 11; converted values must be
0, 1, or 2. Logical, Character, NULL and Empty operands raise error 11.
Invalid offset/origin types raise error 11 even with an invalid handle.

RQ-CF-PRG-FSEEK-NUMERIC-BOUNDS-001 derives default COPPERFIN exact signed-32-bit
offsets and exact 0/1/2 origins from the owner policy in #6776. These checks
precede seeking and leave position unchanged on rejection. VFP9 quirks are
available only with NUMERICBEHAVIOR VFP9. NaN rejection and exact int64/uint64
extension coverage are derived policy, not claims about installed VFP9.

Failed seeks expose a separate return-value gap tracked by #6956: VFP9
returns the unchanged position and FERROR 25; Copperfin currently returns -1
and preserves position. This conversion slice preserves that existing result.
File-handle conversion is a subsequent #5611 slice.
