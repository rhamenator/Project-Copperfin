# Agent Handoff

## Last shipped slice

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

Issue #6459 is open, repository-owner-authored, and carries the exact
`agent-approved` label. Branch `codex/fix-6459-mnx-runtime-symbol` derives a
bounded SHA-256 menu symbol from each non-shortcut MNX logical path while
retaining the source filename stem in the executable model. Generated DEFINE,
ACTIVATE, DEACTIVATE, and RELEASE lines use the same symbol; shortcut popup
names and synthetic submenu targets are validated before entering generated
source. Focused Linux `test_xasset_methods` and `test_prg_engine_work_areas`
passed, including actual generated-menu activation and teardown. Next: finish
requirements/documentation, review the compatibility boundary, push a PR,
run hosted validation, and address review comments before merge.

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
