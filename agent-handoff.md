# Agent Handoff

## Last shipped slice

PR #6491 fixed #6459 and merged into `v1-development` as
`558f243b5518cb8a074c54b8453a78b269056021`; #6459 is closed (manually --
`v1-development` is not the default branch, so `Fixes #N` does not
auto-close). Before merging, the two Windows full-suite failures left open at
the prior takeover (`test_vfp_assets`, `test_staged_import_publish`) were
compared against a dedicated exact-base hosted Windows Native Validation run
dispatched at the PR's merge-base commit `18ac9c6e1` (temporary branch
`exact-base-6491-validation`, run 35788893430, deleted after use). Both
failures reproduced byte-for-byte identically at the exact base and at the PR
final head 15df3773f (same two tests, same assertion text, same `99% tests
passed, 2 tests failed out of 398`), confirming they are pre-existing and
unrelated to #6459's change. No new PR review comments existed beyond the
already-addressed snapshot-path finding at merge time. Scratch build
directory `~/temp/copperfin-6459-build` (565M) removed after merge.

PR #6490 fixed #6458 and merged into `v1-development` as
`18ac9c6e133249f6003154f014173b330aa158e4`; #6458 is closed. The final
head passed all 15 hosted checks, including ASan/UBSan and Windows validation.
The changed suites passed on full macOS and Windows native runs. Both macOS
full-suite failures and two of three Windows full-suite failures reproduced on
exact-base runs. The remaining Windows Access export failure is outside the
changed files and recorded in the PR discussion and requirements row. Earlier
#6460, #6389, #6388, #5680 partial, cloud-validation, and #6251 PRs also
merged; #5680 remains open. PR #6486 was closed without merge after review
found a macOS clone destination-identity gap.

## Active slice

The owner labeled `agent-approved` on the three coverage-cluster children of
umbrella #6498 (#6495, #6496, #6497) and directed starting with #6495
(concurrency state-sequence hunt), judged highest-priority since concurrency
defects are this codebase's most severe hazard class and are historically
under-caught by the existing fixed-sequence `stress` lane. PR TBD on branch
`codex/fix-6495-state-sequence-hunt` adds four deterministic, seed-replayable
state-sequence tests in `tests/test_prg_engine_control_flow_task_supervision_state_sequences.cpp`
(see `docs/38-prg-task-supervision.md`'s new State-Sequence Coverage section
for the full invariant/expected-result writeup). Real record/table-lock
contention plus `SET REPROCESS TO n` provide deterministic synchronization
without any production-code or build-system changes; the tests compile into
`test_prg_engine_control_flow`, so hosted `stress`-lane repetitions exercise
them automatically. The hunt surfaced and filed a real, not-yet-fixed defect,
#6499 (cancellation observed inside an explicit FLOCK()/RLOCK() retry loop is
silently swallowed instead of halting) -- deliberately not fixed as part of
this coverage-only slice; the affected test documents current behavior with
an inline citation instead. Local Linux Debug: 5 consecutive full runs of
`test_prg_engine_control_flow` passed cleanly (~110s each, no flakiness).
Next: push, open the PR, validate hosted checks, merge, then continue with
#6496 or #6497 (both also `agent-approved`) or re-check live GitHub state for
higher-priority work.

## Workspace preservation

Preserve unrelated untracked user files in the main checkout:

- `AGENTS.md`
- `Z:\\home\\rich\\temp\\vfp9-probes\\empty-object-205\\vfp.out`

The defect-fix takeover ended at 17:00 America/Detroit on 2026-09-22. The
first focused cycle of the owner's second-pass discovery prompt found and
reproduced issue #6492: `PREVIEW` inside a quoted REPORT/LABEL `TO FILE` output
pathname incorrectly enters preview mode and creates no output. A temporary
Linux regression failed for both commands; the test edit was removed, and no
production fix was committed. Per the owner's subsequent direction, the
discovery heartbeat now runs hourly; the older
`continue-copperfin-issue-loop` heartbeat is paused. Next discovery cycle:
inspect fixes since this pass or shift to an independent invariant if newly
filed issues are being resolved.
