- 2026-11-06: Deduplicate BITLSHIFT/BITRSHIFT operand and count conversion behind
  one checked signed conversion core while preserving their distinct unsigned
  operand projection, 0..31 count validation, numeric-mode behavior and error
  paths (#6872/#5611/#6776). Non-shift BIT conversion remains separate because
  its recovered exact-kind and nonfinite policies differ.
