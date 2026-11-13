- 2026-11-07: Make BITAND, BITOR, and BITXOR reject one-argument calls with
  VFP9 error 1229 instead of returning the function name, and add portable
  two-mode coverage for arity, fractional folds, and preserved mixed-type
  coercions (#6874/#6875/#5611/#6776).
