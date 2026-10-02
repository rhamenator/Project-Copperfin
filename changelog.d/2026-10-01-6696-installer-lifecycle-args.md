- 2026-10-01: The Windows installer lifecycle CI step failed with "Parameter
  set cannot be resolved" because the workflow array-splatted `-Name value`
  pairs, which PowerShell binds positionally; it now splats a hashtable
  (#6696). `CopperfinPackageVersion.txt` now follows
  `CPACK_PACKAGE_VERSION`, so the synthetic prior installer built with
  `COPPERFIN_PACKAGE_VERSION_OVERRIDE` is genuinely differently versioned
  (it was stamped with the current version), and the lifecycle step fails
  if the prior and current versions or registry keys are equal.
  The prior-version `-DCOPPERFIN_PACKAGE_VERSION_OVERRIDE=0.0.1` is now
  quoted because pwsh split it at the first dot, so CMake never saw the
  override; both configure calls now check their exit status.
