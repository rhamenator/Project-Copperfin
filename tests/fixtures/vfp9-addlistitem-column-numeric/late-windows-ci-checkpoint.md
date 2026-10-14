# Late Windows CI failures after PR7180 integration

Current main revision5e1b259a65f909fb4c5ce3ae30cb48e869098778; numeric activeWT
fix/addlistitem-column-numeric-5611 preserves the UNUSED helper/pre-code map.
No test table/freeze/caller change exists. CI diagnostics do not admit unrelated
production work or silently abandon the retained numeric slice.

PR7180 exacthead7b731ad9c56b5404033b107f1d085f5a50f6a770 Windows Native Validation
run38503273755/job115654892554 completed CANCELLED04:52:23Z, watch6420 harvested1.
Actual annotation: "The job has exceeded the maximum execution time of 2h0m0s".
Full4064-line/609545-byte raw SHA256
d3b10b6696e80cbf1a9f862b863cdc30599af83073a8ce415b76ba38e08dda0b.
Download0/full exacthead metadata retained under build/addlistitem-column-native.
575/600 tests reported Passed, no individual failure reported before cancellation.
All five AddItem THIRD-column subjects passed,53.25-61.30s public shards.
Build native targets3296.829s (~54m57s); native-test step cancelled after
3844.032s (~64m04s), last started test_prg_engine_on_selection_bar_numeric.
This is cumulative job budget exhaustion, NOT a proven hang in the last test.
Neither full600 completion nor late-Windows qualification is claimed.

Current-main push run38505514439/job115661998851 completed FAILURE04:48:04Z.
Full4145-line/623303-byte raw SHA256
ed7b448ac078ac06a6b9ad6897add7de87608c5ecd08fd4f09058869f864f010.
600 executed,598 passed/two timeouts: test_dbf_table198.83s and
test_visual_asset_editor181.38s, CTest8/2908.88s total. PR exacthead same tests
passed66.57s/92.58s respectively. These are observed timing differences, NOT
evidence of root cause, contention, product defect, safe timeout increase or
causation by AddItem. Generic CMake pthread probe "Failed" is not a test failure.
No assertion weakening/test skips/isolation reclassification/timeout change.

Structured issue searches returned existing unadmitted7054/7141 without
agent-approved; their content was not read and no work admitted from them.
An admitted search hit7049 was inspected and proved unrelated DBF NUL-reader
scope; no new authority, production change or detour follows from that hit.

One diagnostic FAILED-JOB rerun of exact current-main38505514439 requested0,
without source/workflow/timeout changes. Actual attempt metadata and bounded
watch retained separately; retry does not erase original failure or prove fix.
Initial CLI metadata request used unsupported --json attempt, failed and wrote
no valid snapshot; corrected REST metadata supplies run_attempt explicitly.
Pending result must be inspected before deciding a fix. No VM/credentials,
paid integration, test-phase assertion skip or owner-input blocker established.

Actual REST rerun metadata: attempt2, exactmain5e1b259a, IN_PROGRESS;
new job115690140975 started05:02:20Z. Blocking watcher30590 active; do not
launch duplicate watchers/reruns or claim completion before it is harvested.
All five AddItem-column subjects also passed in the failed main run
(four public shards37.04-47.42s); these are partial corroboration, not full600
or warning-free acceptance. Focused budget tracking issue7181 was created0:
https://github.com/rhamenator/Project-Copperfin/issues/7181
No agent-approved label was added or new production scope admitted from its
body. Current numeric slice remains the implementation target; same-head CI
retry is a diagnostic under the heartbeat's actionable-CI instruction.
