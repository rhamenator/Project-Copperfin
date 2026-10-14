## Observed failure at current main

Current main is `5e1b259a65f909fb4c5ce3ae30cb48e869098778` (PR #7180 merge).
The exact reviewed PR head is `7b731ad9c56b5404033b107f1d085f5a50f6a770`.
[Windows Native Validation run38503273755](https://github.com/rhamenator/Project-Copperfin/actions/runs/38503273755),
job115654892554, was cancelled at2026-10-14T04:52:23Z with the actual annotation:
"The job has exceeded the maximum execution time of 2h0m0s".

The native build completed in3296.829s, followed by3844.032s of tests before
the job limit stopped validation.575 of600 tests reported Passed, with no
individual test failure reported before cancellation. All five AddItem-column
subjects passed; the last started test was test_prg_engine_on_selection_bar_numeric.
Cancellation at that test does not establish that it hung. The remaining
inventory is unqualified; the late job does not retroactively change the
recorded required-check/exact-head-review merge gate.

The complete4064-line/609545-byte raw log is retained on the implementation
host under the active slice build/addlistitem-column-native,
SHA256`d3b10b6696e80cbf1a9f862b863cdc30599af83073a8ce415b76ba38e08dda0b`,
with exact-head metadata, actual watch1 and download0 statuses.

## Related current-main timing evidence, not a shared-cause claim

[Current-main push run38505514439](https://github.com/rhamenator/Project-Copperfin/actions/runs/38505514439)
completed600 tests in2908.88s but failed two180s test budgets:
test_dbf_table198.83s and test_visual_asset_editor181.38s. The PR run reported
the same tests passing66.57s and92.58s. This is run-to-run timing evidence,
not proof of contention or a product regression. Full current-main raw SHA256
`ed7b448ac078ac06a6b9ad6897add7de87608c5ecd08fd4f09058869f864f010` retained.
An unchanged exact-main failed-job diagnostic rerun was requested once;
its pending result must not be presented as success or a fix.

## Bounded follow-up / acceptance

- Recover the cause and a bounded CI scheduling/build/test remedy from retained
  timing and repeat evidence; do not guess from the last running test.
- Preserve the complete discovered CTest inventory, existing serial/isolation
  contracts, assertions,120min job and180s test budgets. Do not fix this by
  skipping tests, weakening expectations or extending timeouts.
- If independent hosted partitioning is selected, audit exact-once complete
  inventory coverage and fail-closed aggregate results; maintain required checks.
- Retain full exact-head logs, measured phase durations and negative failure
  evidence; require completed full Windows acceptance on the reviewed fix.

This issue records CI qualification budget exhaustion; it does not admit
unrelated DBF/editor or other production changes. The active #5611/#6776
AddListItem existing-ID third-column slice remains preserved and unfinished.
