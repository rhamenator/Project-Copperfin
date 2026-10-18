Observed against current main `4b6cc734fb1b70cf7cbcdd1f083fafa1317fa4f9`, discovered while qualifying the admitted non-whole Numeric division continuation tests under #5611/#6776/#7189. This focused report does not expand that active slice or authorize a query-type fix there.

Reproduction: `SET DECIMALS TO 14`, then evaluate `SET('DECIMALS')` and `VARTYPE(SET('DECIMALS'))`.

Installed VFP9 09.00.0000.7423, two unchanged native invocations at D0/D14, reports VARTYPE `N`; the public documented FLL observer sees native Integer (`type=49`), width10, integer0/14. Unused Integer real/decimal slots are not semantic evidence. Both native wrappers return0/empty stderr and normalized `VAL(TRANSFORM(SET('DECIMALS')))` returns0/14.

An actual host public-API diagnostic linked to retained, hash-verified unchanged-main runtime archives reproduces Character in BOTH NUMERICBEHAVIOR modes: `kind=3`, string `"0"`/`"14"`, `VARTYPE` `"C"`. Its inactive numeric field is0. Normalized readback is ordinary Number0/14 in both modes, so this is a result-type compatibility gap, not evidence the setting mutation failed. Compile/run0, no runtime modification.

Source corroboration: `src/runtime/prg_engine_runtime_surface_dispatch_general.inl:587` unconditionally wraps the general SET callback in `make_string_value`, except the separately handled CENTURY variant. Other query names have not been audited here; do not generalize their types from this case.

Impact: native callers using type inspection, arithmetic/type admission or Numeric metadata can diverge. The earlier constant-division test's D0-only numeric-field check could pass accidentally on an inactive field; the new D14 control exposed it. The bounded division tests will normalize the observation through the existing `VAL(TRANSFORM(...))` adapter and explicitly require Number, keeping exact0/14 final-state assertions. That does NOT establish native SET query-type parity or close this issue.

Evidence: the retained non-whole branch `fix/numeric-nonwhole-division-5611` will preserve `setting-query.prg/.FXP`, independent host diagnostic source, exact runner/executable/FLL and archive identities, all raw/status records and REA controlled-replay evidence `ev_cceee29f338ac9ddffe80be9ad40708568efa44596966e8a593a2bdef59afca2`. No VM, decompilation, timeout change, paid integration or approval-label mutation.

Follow-on acceptance: recover this query's native type/metadata and public consumers before a bounded typed implementation; preserve per-area compatibility policy, checked conversions/limits and other query behavior. No parent closure or general Numeric compatibility claim.
