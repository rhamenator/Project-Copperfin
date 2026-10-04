# Installed VFP9 `STR()` argument-bound observation

This fixture records a black-box observation made on 2026-10-04 with Visual
FoxPro 9 SP2 (`09.00.0000.7423`) through the repository owner's local Windows
compatibility installation. No implementation internals were inspected.

`str-argument-bounds.prg` is the source submitted to VFP9 and
`str-argument-bounds.out` is its complete console output. The observation
establishes that widths 0 through 237 and decimals 0 through 18 are admitted
after truncation; invalid converted arguments raise error 1908. It also
captures the negative low-32-bit conversion at -4294967295 and -4294967296,
plus the integer-indefinite result for negative infinity. Positive infinity
raises error 1908.

This evidence governs `RQ-CF-PRG-STR-ARGUMENT-BOUNDS-001` under issues #5611
and #6776.
