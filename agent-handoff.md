# Agent Handoff

## Last shipped slice

PR #6505 fixed #6047 (nullable DBF writes silently losing NULL, storage
layer) and merged into `v1-development` as `c8dac41b02343732361669f07983db68afb655dc`;
#6047 is closed manually. `CREATE TABLE`/`CREATE CURSOR` fields with an
explicit `NULL` clause now persist a real physical nullability bit (byte
layout recovered empirically from a real VFP9 9.0 SP2 instance under Wine,
`docs/80-dbf-nullflags-field-format-notes.md`); `REPLACE ... WITH .NULL.`
sets/clears it for both immediate-write and buffered/`TABLEUPDATE()`
paths; `COPY STRUCTURE EXTENDED`'s `FIELD_NULL` reports real nullability;
`ALTER TABLE ADD/ALTER COLUMN` respects an explicit `NULL` clause too,
without duplicating the bitmap field on schema rewrite. Scope is the
storage layer only, per owner direction during scoping -- the larger
`PrgValue`/`VARTYPE()`/`EMPTY()`/`NVL()`/`EVL()`/aggregate-on-NULL
semantics redesign this surfaced is filed as a separate follow-up, #6506,
not started.

A Codex + Copilot review round on the initial version of this PR found 11
real defects, the most severe being a confirmed root cause:
`read_raw_field_descriptors()` (used by every `REPLACE` writer, a function
distinct from the already-correct `parse_dbf_field_descriptor_block()`)
never decoded the nullable flags byte, so `REPLACE ... WITH .NULL.`
silently never set or cleared any bit -- masked by the *original*
regression test's own broken guard condition (`values.size() == 4U`,
which a real 5-value record including the unfiltered `_NullFlags` field
could never satisfy, so its assertions silently never ran; a lesson for
any test asserting an exact field/value count on a table that might have
hidden bookkeeping fields -- prefer name-based lookup). Also fixed: schema
rewrites (`ALTER TABLE`) duplicating `_NullFlags`; `_NullFlags` identified
by type alone (collision risk with a preserved opaque type-`0` field); a
`uint16_t` bitmap accumulator breaking past 16 nullable fields; `ALTER
TABLE ADD/ALTER COLUMN` never applying an explicit `NULL` clause's bit;
the flags-byte offset misreading legacy FoxBASE's 16-byte descriptors;
three buffered-`TABLEUPDATE()`-flush call sites not threading `is_null`
through; and `GETFLDSTATE()`/`SETFLDSTATE()` seeing the hidden field (plus
a self-caught `std::distance()`-against-the-wrong-container bug introduced
while fixing that last one). Every fix verified fail-then-pass; two new
regression tests added. Full account in docs/32's `RQ-CF-PRG-059` row.
Scratch build directory `~/temp/copperfin-6047-build` (958M) removed after
merge.

Two docs-only PRs from mid-investigation (#6507, #6508) were closed
without merging once superseded by the final fix -- their branches were
deleted. The owner explicitly decided not to hand this off to Codex mid-
investigation; it was finished solo per that direction.

Earlier: PR #6503 fixed #5567 (legacy DBF import silently reactivating
deleted source records, merged `565d66f32`) and PR #6502 fixed #5631
(JSON export of an unknown/NULL logical value as `false`, merged
`232b7e704`); both issues closed manually.

Earlier still: PR #6500 fixed #6495 (concurrency state-sequence hunt,
child of umbrella #6498) and merged into `v1-development` as
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
now fixed and merged: #5631, #5567, #6047 (full account above). #6047's
own full scope was deliberately split during implementation: the on-disk
storage layer (PR #6505, landed) vs. the `PrgValue`/`VARTYPE()`/`EMPTY()`/
`NVL()`/`EVL()`/aggregate-on-NULL semantics redesign (filed as #6506, not
started) -- an owner-approved split rather than one oversized slice, per
`agents.md`'s slice-sizing rule.

Next: start #6496 (migration fidelity coverage hunt: NULL, deleted rows,
memos, multi-file recovery) on this now-more-correct baseline -- the
second of the three owner-labeled `agent-approved` coverage-cluster
children of umbrella #6498 (#6495 done; #6497 remains `agent-approved` and
not yet started, queued behind #6496). No implementation started yet this
turn.

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
