# VFP9 `FCREATE()` attribute-bound observation

`fcreate-attribute-bounds.prg` is the complete installed-VFP9 SP2 probe source
and `fcreate-attribute-bounds.out` is its complete output. The probe creates a
separate temporary file for each Numeric, Currency, or invalid-type attribute,
then records whether creation succeeds, whether a one-byte write succeeds, and
any catchable error.

Installed VFP9 accepts converted attributes 0 through 7. Numeric values at or
below the raw positive ceiling of 7 truncate toward zero and then pass through
signed low-32-bit conversion. A raw positive Numeric value above 7 raises
error 11 before wrap, while negative values can wrap to a valid attribute and
negative values outside signed 64-bit range, including negative infinity,
convert to zero. Currency rejects a nonzero sub-unit magnitude, otherwise
truncates to whole units, rejects a positive whole value above 7, and applies
the same conversion and range rule. Logical, Character, Empty, and NULL
attributes raise error 11. Attribute zero permits writing; every accepted nonzero
attribute blocks writes on the created handle, as documented.
