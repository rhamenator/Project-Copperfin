# Agent Handoff

## DO NOT MERGE YET: PR #6505 and #6507 have real unresolved review findings

PR #6503 (#5567) and PR #6502 (#5631) are merged and closed -- fine. PR
#6505 (#6047 storage-layer `_NullFlags` support) and PR #6507 (docs handoff
update) are open with CI green, but review rounds (Codex `chatgpt-codex-
connector` + `copilot-pull-request-reviewer`) found substantial real bugs
in #6505 -- do not merge either until these are addressed. Full comment
text is on the PRs; summary of confirmed/highest-priority findings:

1. **Confirmed real bug, root cause found**: `read_raw_field_descriptors()`
   (`src/vfp/dbf_table.cpp:665`) never populates `RawFieldDescriptor::nullable`
   from the flags byte (it's a separate function from
   `parse_dbf_field_descriptor_block()`, which does decode it correctly for
   the read/display path). Every `RawFieldDescriptor` built by this function
   defaults `nullable=false`. `apply_null_flag_bit()` (line 727) skips any
   field where `!f.nullable`, so on every `REPLACE`-path call (both
   `replace_record_field_value_targeted()` and `_impl()`, which both call
   `read_raw_field_descriptors()` to build the `fields` vector passed to
   `apply_null_flag_bit()`), the loop finds `bit_index = -1` for every field
   and the function always returns early doing nothing. **REPLACE ... WITH
   .NULL. never actually sets or clears a bit today.** Needs a fix (decode
   the flags byte in `read_raw_field_descriptors()` too, or a shared helper
   both functions use).
2. **Open mystery, needs resolving before trusting any test in this PR**:
   despite (1), `test_create_table_replace_with_null_persists_null_flag`
   (`tests/test_prg_engine_table_structure.cpp:608`) passed locally,
   including for the Character field case, which has no other has no
   independent null-detection heuristic (unlike blank Numeric/Date, which
   might coincidentally read back as null through some pre-existing
   text-based heuristic in `decode_value()`, unverified). This needs
   investigation: either the test is a false-positive (asserting on the
   wrong thing, or exercising a different code path than assumed), or my
   analysis above is wrong somewhere -- resolve which before doing anything
   else. Do not assume the fix in (1) is correct without a fail-then-pass
   verification against this specific test.
3. Schema-rewrite (`ALTER TABLE`) duplicates `_NullFlags` when a table
   already has one physical bitmap field, since `parse_dbf_table_from_file()`
   returns it unfiltered in `table.fields` and the creator unconditionally
   appends a new synthetic one; the new bitmap is also initialized from
   display strings rather than decoded `is_null`, clearing existing NULL
   state. (Codex + Copilot, `dbf_table.cpp` ~2620-2650, ~2797, ~2804.)
4. `_NullFlags` is identified by `type == '0'` alone in multiple places
   (`apply_null_flag_bit`, the read-side lookup, the schema-rewrite
   duplicate-check); must also match the literal name (normalized), since a
   pre-existing opaque type-`0` field (e.g. the `NULLS` schema-rewrite
   fixture) would otherwise be misidentified as the bitmap.
5. Bitmap length/bit-index math uses `std::uint16_t` in both reader and
   writer (`dbf_table.cpp` ~2295-2299, ~2804); breaks (drops bits, possible
   UB from over-wide shifts) for any table with more than 16 nullable
   fields. Needs a byte-vector-based bitmap representation.
6. `ALTER TABLE ADD/ALTER COLUMN` dispatch passes `declaration->descriptor`
   directly to the schema writer instead of going through
   `table_field_descriptors()`, so an explicit `NULL` clause on an
   ALTER-added column never gets a physical bit.
   (`prg_engine_table_structure_helpers.cpp:218`-adjacent dispatch code.)
7. The flags-byte-at-offset-18 read is applied uniformly to every
   `DbfReadLayout`, but FoxBASE's field descriptors are 16 bytes (decimal
   count at offset 15), so offset 18 lands in the *next* descriptor or the
   terminator -- legacy tables can get spurious nullable flags.
   (`dbf_table.cpp:2043`.)
8. Buffered `TABLEUPDATE()` flush (`commit_buffered_record()`, verified
   staging, mode-4/5 loop in `prg_engine_records.inl` ~1544/~1612) still
   calls the writer with default `is_null=false`, not threading
   `field.is_null` through -- a buffered `REPLACE ... WITH .NULL.` followed
   by `TABLEUPDATE()` still loses the bit even though the earlier fix (this
   PR) made the in-memory buffered record itself correct.
9. `GETFLDSTATE()`/ordinal field APIs still see `_NullFlags` because
   `parse_cursor_table()`'s `DbfRecord::values` isn't filtered the way
   `cursor.local_fields` is (`prg_engine_free_functions.inl:1483`).
10. String-typed (C/V/Q) field `is_null` is lost when copied through the
    string-based `create_dbf_table_file()` API (`COPY TO` and similar
    internal copy paths) since that API only takes `std::vector<std::string>`
    rows with no per-cell null flag (`dbf_table.cpp:2797`).
11. Memo-field (`M`/`G`/`P`) NULL is a disclosed, explicit out-of-scope
    exception in this PR's own description -- Copilot flagged it
    (`dbf_table.cpp:4129`) as contradicting the "nullable-write contract"
    stated in the docs/32 row; reconcile by either narrowing that
    requirement-row wording to explicitly exclude memo fields, or by
    implementing memo-bit handling too.

None of these were fixed before this handoff -- work stopped here because
session usage hit 92% (reset 3:40 AM EDT). Next agent: start with #1/#2
(resolve the mystery first, since it may mean the whole verification
methodology used elsewhere in this PR needs re-checking), then work down
the list, fixing each with a real regression test and fail-then-pass
verification per this repo's established discipline, before touching
merge. PR #6503/#6502 do not need any of this -- they're merged and fine.

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
