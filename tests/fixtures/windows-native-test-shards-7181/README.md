# Bounded Windows native CI budget repair

RQ-CF-CI-WINDOWS-NATIVE-BUDGET-001 / VR-CI-7181-001/002 /
DQ-DV-CI-7181-001. Authority is the owner's implementation heartbeat instruction
to investigate/fix actionable CI failures while preserving the numeric work.
Issue #7181 is tracking/provenance, not authority for unrelated product changes;
its owner-controlled approval label is unchanged. No production/test-assertion/
isolation classification, VM, credential or paid-integration changes.

## Observed failure, not a product-behavior requirement

At main5e1b259a65f909fb4c5ce3ae30cb48e869098778, unchanged diagnostic
[run38505514439 attempt2](https://github.com/rhamenator/Project-Copperfin/actions/runs/38505514439/attempts/2)
job115690140975 ran05:02:20Z–07:02:31Z on2026-10-14, then CANCELLED.
Actual annotation: "The job has exceeded the maximum execution time of 2h0m0s".
Full4002 lines/600829 bytes, raw SHA256
65bcb9d33430d6e53e5f75d27b562622967794c2a665080264dd4d9c13608b30.
550 unique of600 reported Passed/no individual failure before cancellation.
Previously timed-out dbf_table/visual_asset_editor now passed72.99/102.98s;
all five AddItem-column subjects passed (public54.62–55.25s).
This is cumulative job-budget exhaustion, not a proven product hang or root
cause for attempt1's two180s test timeouts. Original failed/cancelled evidence
is retained; no third unchanged rerun or retroactive full600 success claim.

The earlier PR7180 run38503273755 cancelled at the same120min budget after
575 reported passes (rawd3b10b66); main attempt1 executed600 but failed two
individual timeouts (rawed7b448a). Exact metadata/full raws/statuses remain on
the host in the retained numeric slice build/addlistitem-column-native, with
copies/archive hashes in this CI slice's build/ci-shard-verification. Partial
results cannot qualify the full inventory, and timings alone do not identify
the cause of the original individual timeouts.

## Derived requirement and implementing architecture

Parent: owner's actionable-CI instruction, verification/traceability quality
baseline, HZ-system-failure-01 and HZ-doc-command-01. Repair the cumulative
budget without dropping tests/assertions or weakening failure evidence. Chosen
bounded design retains120min per Windows job,180s per test, two build jobs and
two test workers per host, Release and Debug fixture builds, all commands,
properties and conservative RUN_SERIAL/isolation policy. It changes scheduling,
not product compatibility or test expectations.

Three independent standard hosted Windows jobs each build the same complete
revision. Select every third registered test using actual CTest `-I 1,,3`,
`2,,3`, `3,,3`. Each job verifies full-inventory names are unique/enabled,
rejects unreviewed cross-test fixtures/dependencies, verifies mathematical
disjoint/full partitions against actual CTest selection and unchanged
commands/properties, and requires exact successful JUnit execution for every
selected name. Record full/selected/after inventories, partition plan, JUnit
results and existing phase metrics as small14-day artifacts; no build cache or
large binary artifacts. Missing/failed/skipped/malformed reports fail closed.

An always-running aggregate check downloads ONLY this revision/attempt's three
artifacts. It requires identical ordered full inventories, exact bounded
partitions, unchanged per-job metadata and successful execution of every
full-inventory name exactly once. All three Windows jobs AND the aggregate
must pass before this repair is accepted/merged. An empty report, one green
shard, skipped review or only the11 branch-required contexts is insufficient.

Reverse links: workflow/native action → run-windows-native-tests.ps1 → pure
native-test-shard-policy.ps1 and existing metrics wrapper; aggregate workflow
→ audit-native-test-shards.py; both test scripts map back to this RQ/VR.
Other composite Windows consumers retain default1/1 full-suite scheduling;
Linux/macOS commands and release workflows are unchanged. No release-readiness
budget fix or high-hazard/independent-human/system certification is claimed.
Tradeoff: three full builds use more standard public hosted runner work, in
exchange for bounded independent test portions. This is not a measured global
optimum or guarantee against every future slow test/runner. Hosted evidence
must establish this exact change's result; individual180s failures stay failures.

## Verification chronology and remaining gate

Initial local runner syntax failed BEFORE product test execution: splatting
operator used inside an array expression. Exact initial source/raw/status1
retained; correct to a normal array variable without changing expectations.
Local policy test0:22 synthetic coverage checks (1/2/3/10/599/600/605 inventory
sizes), rejection of empty/duplicate/out-of-bound/disabled/dependency/changed
command-or-serial/skipped/failed/missing cases, actual tiny CTest three-shard
and default whole-suite passes, and actual CTest8 propagated/caught in the
deliberate negative test. Its stderr "Errors while running CTest" is expected
negative-case evidence, not an empty-diagnostics claim.

Retained actual tiny three-shard JUnit/inventories aggregate0:10 unique tests
executed once (4/3/3). Aggregate Python unit tests0 cover positive and ten
mutations, including three individually consistent but DIFFERENT inventories.
Configured portable inventory604 from the preserved numeric slice checks the
real CTest selector/property behavior without product execution; this differs
from baseline Windows600 and is NOT hosted Windows qualification.
Exact local logs/manifests/statuses retained in build/ci-shard-verification.
All three hosted native jobs, aggregate, exact-head review/required11/resolved
conversations are PENDING. Numeric source259 remains frozen in its separate
WT with original95 driver82619 running; no caller wiring before BOTH originals.

## Bounded documentation verification

Misuse MEDIUM: treat partial/one-shard/synthetic/inventory-only evidence as
full native or release acceptance; omit the aggregate; increase test timeout;
reclassify serial tests; erase previous failures; infer a product cause or
claim all platform/host/VM validation. Procedural delta: complete native
evidence now means3+aggregate, not one optional Windows check. Check per-job
artifact membership and current revision/attempt; preserve actual failures.
Walkthrough:22 policy coverage checks and deliberate failure propagation;
actual CTest4/3/3 execution → exact aggregate10; portable full-inventory
selection; static workflow/default/timeout checks; hosted gate still pending.
Delegated development self-review is not personal owner or independent human
sign-off. Hazard severities/statuses and release/domain assurance obligations
remain unchanged. Roll back with a scoped reviewed revert of CI scripts/action/
workflow, archive superseded reports, and notify the linked PR/issue/owner if
guidance or acceptance was wrong. No parent/numeric/hazard closure follows.
