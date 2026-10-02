- 2026-10-02: `STUFF`, `SUBSTR` and `SUBSTRC` start boundaries now match installed VFP9 SP2 (#3704, part of
  #6029/#5611). `STUFF('abcdef', 0, 2, 'X')` is `Xcdef` (a start below 1 acts as 1 and still replaces; the earlier
  "insert only" rule came from documentation, not from VFP9), and a start past the end appends
  (`STUFF('abcdef', 8, 2, 'X')` and a start of `1E20` give `abcdefX`). `SUBSTR` and `SUBSTRC` return an empty
  string for a start below 1 after truncation (`SUBSTR('abcdef', 0)` and `0.5` are empty, `1.9` is the whole
  string), so `SUBSTR(s, AT(x, s), n)` with no match yields an empty string instead of the whole string. Every
  remaining unchecked double-to-integer conversion in the string functions is now a defined, saturating
  conversion; in-range results are unchanged. Pinned expectations for `STUFF` zero/negative start and `SUBSTRC`
  zero/negative start were corrected to the VFP9 results. Evidence is retained under `~/temp/vfp9-probes`.
  `STR()` now raises error 1908 ("Width or decimal place argument is invalid.") for a decimals argument above 18, as
  VFP9 does, instead of formatting an unbounded number of digits; a negative decimals count is still clamped to 0.
