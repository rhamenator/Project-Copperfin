# VFP9 `FDATE()` type-bound observation

`fdate-type-bounds.prg` is the complete installed-VFP9 SP2 probe source and
`fdate-type-bounds.out` is its complete output. The probe creates a temporary
file, passes Numeric and Currency values through the optional `nType`
argument, records the return type or catchable error, and removes the file.
It also confirms that Logical, Character, Empty, and NULL flags all raise
error 11 rather than being coerced to Numeric zero or one.

For Numeric values, VFP9 rejects a raw positive value above 1, otherwise
truncates toward zero through its signed low-32-bit conversion and accepts
only converted 0 (Date) or 1 (DateTime). Thus negative out-of-range and
negative-infinity values that convert to zero return Date, while their
positive counterparts raise error 11. Currency first rejects a nonzero
sub-unit magnitude, otherwise truncates to whole units, rejects a positive
whole value above 1, then applies the same low-32-bit conversion and admits
only 0 or 1.
