# VFP9 `FOPEN()` mode-bound observation

`fopen-mode-bounds.prg` is the complete installed-VFP9 SP2 probe source and
`fopen-mode-bounds.out` is its complete output. The probe recreates a temporary
file for each case, opens it with a Numeric or Currency mode value, and records
whether the open succeeds, whether a one-byte write succeeds, and any
catchable error.

Installed VFP9 accepts the converted modes 0, 1, 2, 10, 11, and 12. Numeric
values at or below the raw positive ceiling of 12 truncate toward zero and
then pass through signed low-32-bit conversion; only the six supported results
are admitted. A raw positive Numeric value above 12 raises error 11 before
wrap, while negative values can wrap to a supported result and negative values
outside signed 64-bit range, including negative infinity, convert to zero.
Currency rejects a nonzero sub-unit magnitude, otherwise truncates to whole
units, rejects a positive whole value above 12, and applies the same low-32-bit
conversion and supported-mode admission rule.
