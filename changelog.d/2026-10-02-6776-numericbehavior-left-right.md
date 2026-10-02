- 2026-10-02: New `SET NUMERICBEHAVIOR TO COPPERFIN | VFP9` (default COPPERFIN) controls what a Numeric
  argument that is out of range for an integer does (#6776, covering #5611 and #6029). `LEFT`, `RIGHT`, `LEFTC`
  and `RIGHTC` are the first functions to honor it. By default a huge count saturates (`LEFT('abc', 1E20)` is
  `abc`, `LEFT('abc', -1E20)` is empty) instead of being an undefined conversion; with `SET NUMERICBEHAVIOR TO
  VFP9` they reproduce installed VFP9, which converts through a 32-bit integer and so returns empty for
  `LEFT('abc', 1E20)` and for `4294967295`, but the whole string for `1E10`. VFP9's results were probed on a
  Windows 11 VM across 14 boundary values. In-range counts are identical in both modes. `SET COMPATIBLE`
  is unchanged and keeps its FoxBASE+/dBASE meaning (#6223). Other functions keep their current behavior until
  they are migrated (tracked on #6776).
