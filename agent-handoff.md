# Agent Handoff

## Last shipped slice

PR #6502 fixed #5631 (JSON export of an unknown/NULL logical value as
`false`, merged `232b7e704`) and PR #6503 fixed #5567 (legacy DBF import
silently reactivating deleted source records, merged `565d66f32`); both
issues closed manually. PR #6505 is open for #6047 (nullable-field
`_NullFlags` on-disk storage support): `CREATE TABLE`/`CREATE CURSOR`
fields with an explicit `NULL` clause now persist a real bit via a hidden
`_NullFlags` bitmap field (byte layout empirically recovered from a real
headless VFP9 9.0 SP2 instance under Wine, `docs/80-dbf-nullflags-field-
format-notes.md`); `REPLACE ... WITH .NULL.` sets/clears the bit and blanks
storage for both the immediate-write and buffered-update paths (the latter
fixing a separate pre-existing bug where a buffered NULL write was silently
discarded before `TABLEUPDATE()`); `COPY STRUCTURE EXTENDED`'s `FIELD_NULL`
column now reports real nullability instead of a hardcoded `"F"`. Scope is
storage-layer only, per explicit owner direction during scoping -- the
larger `PrgValue`/`VARTYPE()`/`EMPTY()`/`NVL()`/`EVL()`/aggregate-on-NULL
semantics redesign this surfaced is deliberately deferred, filed as #6506.
A dedicated research-agent pass mapped every touch point before
implementation began; cross-checked against the finished implementation and
confirmed already covered except the `COPY STRUCTURE EXTENDED` fix, which
was added and tested separately. PR #6505 CI in progress at handoff time.

Earlier: PR #6500 fixed #6495 (concurrency state-sequence hunt, child of
umbrella #6498) and merged into `v1-development` as
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

All three prerequisite `agent-approved` P1 data-integrity defects that
#6496's own migration-fidelity coverage campaign was expected to hit are
now fixed (#5631, #5567 merged; #6047's storage-layer scope in PR #6505,
pending CI/merge at handoff time). #6047's own full scope was split during
implementation: the on-disk storage layer (this PR) vs. the `PrgValue`/
`VARTYPE()`/`EMPTY()`/`NVL()`/`EVL()`/aggregate-on-NULL semantics redesign
(filed as #6506, not started) -- recommended and owner-approved as a split
rather than one oversized slice, per `agents.md`'s slice-sizing rule.

Next: once PR #6505 merges, start #6496 (migration fidelity coverage hunt:
NULL, deleted rows, memos, multi-file recovery) on this now-more-correct
baseline -- the second of the three owner-labeled `agent-approved`
coverage-cluster children of umbrella #6498 (#6495 done; #6497 remains
`agent-approved` and not yet started, queued behind #6496).

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
