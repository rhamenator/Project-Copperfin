# Agent Handoff

## Last shipped slice

Reentrant-cursor-closure cluster 1 (`docs/81`), cursor-lifetime part,
finished 2026-09-24: #6321 (PR #6521), #6322 (PR #6524), #6331 (PR #6526),
#6242 (PR #6529), #6241 (PR #6534), and #6240 (PR #6536, which also carries
the `docs/81` status update and the changelog catch-up). All
merged into `v1-development` with their issues closed manually. Shared
pattern: capture a `CursorGenerationReference` before any user-code
evaluation and re-resolve it afterwards (plus `current_data_session` where
the command must stay in its session); on loss, raise the catchable
`{command} target work area not found` error before touching the cursor.
Lessons from the review rounds: (a) positional rollback of a provisional
row is unsafe once callbacks can PACK the target, so evaluate against a
`RecordEvaluationOverride` instead (#6322); (b) a leading-`&` visibility
expression is evaluated twice, so the shared evaluator re-resolves between
passes; (c) a new loss flag on a shared helper must be threaded to every
caller (GO/SKIP/relation walking), not only the one under repair. Each fix
was verified fail-then-pass with a local Clang ASan/UBSan build; after
halt-on-error stops at the first failure, isolate newly added scenarios to
prove each one independently.

Earlier: PR #6505 fixed #6047 (nullable DBF writes silently losing NULL,
storage layer; `RQ-CF-PRG-059`), with #6506 filed for the `PrgValue` NULL
semantics redesign (not started). #6496 (PR #6516) and #6497 (PR #6517)
have since merged too.

Earlier: PR #6503 fixed #5567 (legacy DBF import silently reactivating
deleted source records, merged `565d66f32`) and PR #6502 fixed #5631
(JSON export of an unknown/NULL logical value as `false`, merged
`232b7e704`); both issues closed manually.

Earlier still: PR #6500 fixed #6495 (concurrency state-sequence hunt,
child of umbrella #6498, merged `bac49af30ba1e80e2707d6d1d1a8a7b4fa0d19e4`);
#6495 closed manually. Surfaced and filed #6499 (cancellation inside an
explicit `FLOCK()`/`RLOCK()` retry loop silently swallowed instead of
halting), deliberately not fixed as part of that coverage-only slice.

Earlier shipped slices (#6459/#6458/#6460/#6389/#6388/cloud-validation/#6251)
all merged; #5680 remains open (partial). PR #6486 was closed without merge
after review found a macOS clone destination-identity gap.

## Active slice

#6415 (property-read event handlers releasing their source) is fixed in
the current PR: the source handle (never reused) is re-checked in
`ole_objects` after every delegate/`_Access`/selector evaluation. Cluster 1
remaining: #6420 (BINDEVENT before-handlers releasing method sources; same
approach should apply to the method-dispatch path) and #6192 (QueryUnload
self-release, overlaps cluster 2). These are
object/event-lifetime use-after-frees, not cursor ones, so they need an
object-identity equivalent of the generation-reference pattern. Next: pick
per `docs/81`'s cluster order and the issue dependency graph. No
implementation started yet.

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
