# Agent Handoff

## Last shipped slice

PR #6283 fixed issue #6270 and merged into `v1-development` as
`d3ed73cb0ca8474bcdc5e37a401f8894ea0d117f` on 2026-09-15. `CURVAL()` and
`OLDVAL()` now own record overrides and retain stable cursor-generation
references across reentrant PRG callbacks. Fields, metadata, nested buffering
calls, and query aliases reacquire the exact originating data-session/work-area
generation; source closure/replacement raises a catchable localized error before
stale or replacement state is read, while unrelated explicit cursors and a
still-open origin across data-session switches remain usable. Typed `LUPDATE()`
alias/work-area behavior is preserved.

Focused buffering, runtime-surface, work-area, SQL-cursor, locale/catalog, and
safety-traceability tests passed. Valgrind reported 0 errors and no leaks across
1,355,882 allocations. All hosted checks passed, every review conversation was
resolved, and a fresh Codex review found no major issues. Issue #6270 was
manually closed after merge.

## Active slice

Issue #6319 is open, repository-owner-authored, and carries the exact
`agent-approved` label. Branch:
`codex/fix-6319-navigation-cursor-lifetime`.

Expression-driven `GO`, `SKIP`, `SEEK`, and record-specific `UNLOCK` now capture
a stable data-session/work-area/cursor generation before expression evaluation.
The identity persists through direct PRG routine suspension and nested
`EVALUATE()`; each command reacquires it before navigation, lock mutation,
relation synchronization, or success-event formatting. Same-alias/work-area
replacement produces catchable localized error 1002, leaves the replacement
usable, and emits no false command-success event. Switching data sessions alone
continues against the still-open origin, including its record-lock ownership maps
and parent/child relation graph. `RQ-CF-PRG-035`, language coverage,
and the changelog are updated; exact VFP9 error parity is disclosed as an
evidence gap.

Focused and broader navigation, lock, control-flow, SQL-cursor, locale, and safety-traceability tests pass after the review correction. A fresh Clang ASan/UBSan focused build passes, and Valgrind reports 0 errors and no leaks across 1,477,267 allocations. Next action: commit with DCO/signature, push, resolve the review conversations, and request a fresh review.

## Workspace preservation

Preserve these unrelated untracked user files:

- `AGENTS.md`
- `Z:\home\rich\temp\vfp9-probes\empty-object-205\vfp.out`

The `continue-copperfin-issue-loop` heartbeat is active every 30 minutes. It
waits only while CI/CD or external review is pending; after a green, resolved PR
it merges, closes the issue manually when needed, syncs `v1-development`, and
selects the next approved issue.
