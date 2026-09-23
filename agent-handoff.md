# Agent Handoff

## Last shipped slice

PR #6500 fixed #6495 (concurrency state-sequence hunt, child of umbrella
#6498) and merged into `v1-development` as
`bac49af30ba1e80e2707d6d1d1a8a7b4fa0d19e4`; #6495 is closed manually. Added
four state-sequence tests in
`tests/test_prg_engine_control_flow_task_supervision_state_sequences.cpp`
covering both required crossings (SPAWN/AWAIT/cancellation/teardown and
cursor-lock/transaction/caught-error/retry); see `docs/38-prg-task-supervision.md`'s
State-Sequence Coverage section for the full invariant/expected-result
writeup. No production runtime-code changes were needed -- real
record/table-lock contention plus `SET REPROCESS TO n` provided the
synchronization; `tests/CMakeLists.txt` did gain one new test-target source
registration. A review round (Codex + Copilot) correctly found that one
sequence lacked an assertion proving genuine contention occurred (a real
missing-evidence/false-pass risk); that was fixed before merge -- every
sequence now asserts on runtime-emitted contention/cancellation evidence.
Review also correctly noted that getting two spawned sides into position
still uses a fixed wait, since no injected-yield-point/barrier scheduler
exists in this subsystem -- that limitation was *not* removed, only
documented honestly and given wider timing margins for sanitizer/loaded-
runner robustness; a future slice could add a real synchronization seam. The
hunt surfaced and filed a real, not-yet-fixed defect, #6499 (cancellation
observed inside an explicit `FLOCK()`/`RLOCK()` retry loop is silently
swallowed instead of halting the calling script) -- deliberately not fixed
as part of this coverage-only slice; the affected test documents current
behavior with an inline citation. Local Linux Debug: 8 total consecutive
full runs across two rounds passed cleanly (~110s each, no flakiness).
Scratch build directory `~/temp/copperfin-6495-build` (654M) removed after
merge.

Earlier shipped slices (#6459/#6458/#6460/#6389/#6388/cloud-validation/#6251)
all merged; #5680 remains open (partial). PR #6486 was closed without merge
after review found a macOS clone destination-identity gap.

## Active slice

Working toward #6496 (migration fidelity coverage hunt: NULL, deleted rows,
memos, multi-file recovery), the second of the three owner-labeled
`agent-approved` coverage-cluster children of umbrella #6498 (#6495 done
above; #6497 remains `agent-approved` and not yet started). #6496's own
bounded campaign explicitly expects to hit three separate, independently
`agent-approved` P1 data-integrity defects in this exact area; per direct
owner instruction, fixing them is now part of this workstream rather than
being documented as known-failing gaps:

- #5631 (JSON export turns NULL/unknown logical values into `false`) --
  contained fix, no policy ambiguity.
- #5567 (legacy DBF import silently reactivates deleted source records) --
  the issue itself flagged this as needing an owner policy call (docs/69
  currently documents the opposite as intentional); owner confirmed:
  preserve the deleted flag through import, and update docs/69.
- #6047 (nullable DBF writes silently lose NULL) -- owner confirmed full
  scope: on-disk `_NullFlags` bitmap support across `CREATE TABLE`/`CURSOR`,
  `REPLACE`, `APPEND`, buffered updates, transactions/rollback, multiple
  field types spanning multiple bitmap bytes. This is substantial on its
  own; if it still feels too large once underway, split it further per
  `agents.md`'s slice-sizing rule rather than cutting acceptance criteria.

No implementation started yet this turn; plan is #5631 first (smallest,
cleanest), then #5567, then #6047, then #6496's own coverage tests on a
more-correct baseline.

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
