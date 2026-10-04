- 2026-10-03: Native and managed `DECLARE` INTEGER/LONG arguments now use a
  defined VFP9-compatible low-32-bit conversion. Default `NUMERICBEHAVIOR
  COPPERFIN` rejects finite values outside signed 64-bit range, explicit `VFP9`
  mode reproduces the probed integer-indefinite zero, and INTEGER64 or unsafe
  nonfinite values are checked and rejected before ABI entry.
