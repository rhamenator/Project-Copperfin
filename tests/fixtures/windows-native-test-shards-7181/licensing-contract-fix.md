# Exact-head macOS licensing-contract failure and notice-only correction

RQ-CF-CI-WINDOWS-NATIVE-BUDGET-001 / VR-CI-7181-001/002 /
DQ-DV-CI-7181-001. This is an actionable CI repair within PR7182, not a new
product scope or licensing-policy change.

Exact signed31423e48df15aea4a989a4e851e1b475b5f90fd3 macOS native
run38535447865/attempt1/job115757416885 failed2026-10-14T08:56:24Z.
Full599 result records:596 Passed,2 platform-condition Skipped,1 Failed.
The sole failure is the unchanged registered product-licensing-policy
contract: new scripts/native-test-shard-policy.ps1 has the first-party
copyright/SPDX header but omits the repository's additional-permission notice.
Full raw SHA-256:
6190edd69b6153191c1a8f65ea3ef66e5a10712154db09aab32b79610c1193e5.
Run/jobs/raw/download-status0 and initial local contract1/full raw retained in
build/ci-shard-verification. This is NOT full599 acceptance or a product cause.

Add the established Copperfin Application, Runtime, and Toolchain Exception1.0
notice to the three new first-party PowerShell files; add the same complete
copyright/SPDX/exception header to the three new first-party Python files.
No LICENSE, policy contract, executable statement, assertion, timeout, command,
property, workflow, configuration or partition change. Byte comparison after
removing ONLY these added header lines exactly reconstructs all six31423 files.
The earlier multi-file patch application was rejected before any file changed
because a Python docstring context differed; corrected exact context applied.

Local full registered licensing policy now0. Aggregate11 Python checks0;
whole workflow7 checks0; policy22 synthetic checks plus actual tiny three-shard
and default CTest execution and deliberate failure propagation0. Expected
negative-case CTest8 diagnostic remains retained, not claimed empty. New actual
tiny artifacts are in licensing-fix-actual-ctest; the source/raw/status files
and prior failed evidence remain unchanged. Initial-local-source.sha256 remains
a HISTORICAL initial snapshot, not a current-source passing manifest.

Procedural delta: satisfy the existing header policy, do not weaken its scan
or mistake notice-only verification for new exact-head hosted acceptance.
Existing bounded MEDIUM documentation misuse/walkthrough/rollback analysis in
README.md remains; this evidence supplies the additional local failure/fix
walkthrough. Delegated development self-review is not personal owner or
independent human sign-off. No policy, hazard or numeric-parent closure.

Push requires renewed Claude-FIRST exact-head review and hosted qualification.
The clean Codex31423 review does not qualify the new commit. All11 required
checks, all three exact-head Windows shards plus matching-inventory/exact-once
aggregate and all resolved conversations still gate merge/7181 closure.
Current numeric Clang95 remains running separately, source259 unchanged;
no VM/reset assumption or paid integration changes.
