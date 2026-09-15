# Agent Handoff

## Last shipped slice

PR #6372 fixed issue #6319 and merged into `v1-development` as
`bdbc046fe4b974555c71c44527edaed19f28a9db` on 2026-09-15. Expression-driven
`GO`, `SKIP`, `SEEK`, and record-specific `UNLOCK` now retain and reacquire the
origin cursor generation and data-session state across reentrant evaluation.
All hosted checks passed after one unrelated Windows Python-sidecar rerun, every
review conversation was resolved, issue #6319 was manually closed, and its
scratch sanitizer build was removed.

## Active slice

Issue #6288 is open, repository-owner-authored, and carries the exact
`agent-approved` label. Branch: `codex/fix-6288-removeobject-lifecycle`.

Native `RemoveObject()` now routes a validated child through the existing
subtree release lifecycle. Child-first `Destroy` callbacks run exactly once;
runtime handles, event/COM/window bindings, active-form metadata, and references
held by variables, arrays, collections, and object properties are retired.
Missing, empty, and hidden targets preserve the graph and raise localized error
1925. Declared visibility is enforced before mutation. A release-in-progress
ownership guard supports reentrant owner release without invalidating the active
child’s `THIS` frame or clearing its outer reservation. Direct invocation
arguments and suspended expression/command continuations also discard retired
identities before resuming while ordinary handle-shaped character data remains
unchanged. Queued sibling releases run synchronously, and `AddObject()` rejects
new children on an owner already reserved for retirement. The obsolete detached-child test matrix was
replaced with focused differential lifecycle coverage; adjacent child collection
and standalone `Release()` tests remain active. `RQ-CF-PRG-036`, language
coverage, locale catalogs, and the changelog are updated.

Validation passes for the runtime-surface, parser-classes, control-flow, locale,
and safety-traceability tests. A fresh Clang ASan/UBSan build passes the complete
runtime-surface executable after correcting an unrelated malformed `SYS(2021)`
DBF test initializer exposed by ASan. After the second review-hardening pass,
Valgrind reports zero errors and no leaks across 18,401,056 allocations. PR #6403 is open; next:
push the signed/DCO review-fix commit, resolve its review conversations, request
fresh review, then monitor required checks and conversation resolution.

## Workspace preservation

Preserve these unrelated untracked user files:

- `AGENTS.md`
- `Z:\\home\\rich\\temp\\vfp9-probes\\empty-object-205\\vfp.out`

The `continue-copperfin-issue-loop` heartbeat is active every 30 minutes. It
waits only while CI/CD or external review is pending; after a green, resolved PR
it merges, closes the issue manually when needed, syncs `v1-development`, and
selects the next approved issue.
