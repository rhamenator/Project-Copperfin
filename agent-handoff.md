# Agent Handoff

## Last shipped slice

PR #6262 fixed issue #6253 and merged into `v1-development` as
`921f50a4f3e9148668370b245f711bd3c6e6d596` on 2026-09-14/15. `ASCAN()`
predicate evaluation now copies its candidate and validates stable array-binding
and mutation generations after reentrant expressions and synchronous VFP
routine callbacks. Source resize, release, rebind, element replacement,
`ACOPY()`, `ADEL()`, `AINS()`, and `SCATTER TO` raise catchable VFP error 11
before stale storage or bounds can be used. Predicate metadata and lambda
parameters restore against the original predicate frame on mutation and
callback exceptions.

`test_prg_engine_arrays`, localization/catalog tests, and the safety traceability
contract passed. Valgrind reported 0 errors and no leaks across 7,089,025
allocations. The callback-frame regression was verified fail-before/pass-after.
All hosted checks passed, both review conversations were resolved, and a fresh
Codex review found no major issues. Issue #6253 was manually closed after merge.

## Active slice

Issue #6270 is open, repository-owner-authored, and carries the exact
`agent-approved` label. Branch:
`codex/fix-6270-curval-oldval-cursor-lifetime`.

PR #6283 is open. Review found cross-data-session metadata/qualified-field gaps,
an over-broad ordinary-expression scope, a query-alias capture gap, and an
over-eager error for unrelated qualified fields after source closure; all fixes
are complete, with the latest awaiting commit/push. `CURVAL()` and
`OLDVAL()` now own record overrides and key them to a stable cursor identity.
Expression callbacks reacquire the exact data-session/work-area generation
before field, cursor, and nested-expression continuations. Closing or replacing
the cursor raises catchable VFP error 12; switching data sessions preserves the
still-open origin. Qualified alias/work-area fields and field/order/tag metadata
cannot bind a replacement generation or a conflicting alias in a newly selected
data session.
Explicit generation binding is scoped to CURVAL/OLDVAL; an ordinary-expression
non-regression proves explicit aliases still follow the callback-selected session.
Explicit unrelated fields continue resolving against still-open cursors after
the source closes. Exact localized error-message assertions cover compound
nested continuations.

Focused buffering, runtime-surface, work-area, and SQL-cursor tests pass.
Localization/catalog and safety-traceability contracts pass. The retained exact
CURVAL/OLDVAL probes now report `Variable 'NAME' is not found.`; focused
Valgrind reports 0 errors and no leaks across 1,310,742 allocations.

Next action: commit the review fix with DCO/signature, push, verify and resolve
the remaining review conversation, then merge when CI is green, close #6270, and
select the next approved issue.

## Workspace preservation

Preserve these unrelated untracked user files:

- `AGENTS.md`
- `Z:\home\rich\temp\vfp9-probes\empty-object-205\vfp.out`

The `continue-copperfin-issue-loop` heartbeat is active every 30 minutes. It
waits only while CI/CD or external review is pending; after a green, resolved PR
it merges, closes the issue manually when needed, syncs `v1-development`, and
selects the next approved issue.
