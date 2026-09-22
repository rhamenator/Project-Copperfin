# Agent Handoff

## Last shipped slice

PR #6485 partially fixed #5680 and merged into `v1-development` as
`6d6c42697f44287b4d130418babfe5e50c0fee3d`; #5680 remains open. The
prior cloud-validation PR #6484 and #6251 fix PR #6483 also merged.

## Active slice

Issue #6388 is open, repository-owner-authored, and carries the exact
`agent-approved` label. Worktree:
`/home/rich/.codex/worktrees/fix-5680-staged-import-authority/Project-Copperfin`.
Branch: `codex/fix-6388-localization-cwd`, based on the merge of #6485. The
change makes locale discovery skip developer-tree probing when the current
directory is unavailable, then use an executable-root fallback. PATH lookup
also skips an unavailable current-directory entry. A POSIX subprocess removes
its current directory and proves direct catalog resolution, an Access asset
error, and a native runtime UNDO error remain structured and catchable.
The review revision also fixes the locale environment in the subprocess,
tries executable resources after unsuccessful developer-tree discovery, and
replaces a workspace-agent test's old dependency on the cwd exception with
a test-only policy fault. Focused Linux `test_localization` and
`test_workspace_agent_session` pass. The first hosted run had an expected
workspace-agent test failure from that obsolete fixture and an unrelated
macOS .NET benchmark failure; the revision needs hosted CI and review.

PR #6486 was closed without merge after review found an unsafe macOS clone
destination-identity gap: `fclonefileat()` binds the source but does not return
the clone's identity, so reopening the mutable destination could adopt or
remove another file. #5680 stays open for a safer design, network/removable
qualification, and remaining deterministic race evidence. Next: publish the
#6388 PR, inspect CI and review, fix real failures, then merge safely and close
#6388 only if all acceptance criteria are verified.

## Workspace preservation

Preserve unrelated untracked user files in the main checkout:

- `AGENTS.md`
- `Z:\\home\\rich\\temp\\vfp9-probes\\empty-object-205\\vfp.out`

The `copperfin-defect-takeover-until-17-edt` heartbeat resumes this task every
30 minutes and stays quiet while CI/review is merely pending. At 17:00
America/Detroit on 2026-09-22, cease takeover and begin one focused cycle of
the owner's second-pass discovery prompt at
`/home/rich/.codex/attachments/8240047a-fb25-4c54-acfc-f00d26d2f937/Pasted text.txt`.
Then change the heartbeat to a relaxed weekly discovery cadence. The older
`continue-copperfin-issue-loop` heartbeat is paused.
