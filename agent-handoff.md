# Agent Handoff

## Last shipped slice

Issue #6519 (PR #6678, merge `37a77a193`) finished 2026-09-27: unquoted
Character data in CSV and DELIMITED imports now preserves following literal
quote bytes while retaining each format's enclosed-field rules. Focused and
hosted checks passed; both review conversations were fixed and resolved.

Issue #6522 (PR #6670, merge `b3c1ed73d`) finished 2026-09-27: enclosed doubled
quotes in `APPEND FROM TYPE CSV` now preserve both VFP9-observed quote bytes for
fixed-width Character targets without changing DELIMITED, TAB, unquoted,
Varchar, or Memo behavior. Focused and hosted checks passed; review completed
without actionable conversations.

Earlier: issue #6665 (PR #6667, merge `d99c78ac2`) finished 2026-09-27:
enclosed doubled quotes in `APPEND FROM TYPE DELIMITED` now follow VFP9's
format-specific truncation rule without changing TAB or unquoted-field
handling. Focused and hosted checks passed; all review conversations were
resolved.

Earlier: issue #6604 (PR #6666, merge `780d6ac12`) finished 2026-09-27: SDF
Logical export now emits VFP-compatible uppercase `T`/`F` bytes while
preserving adjacent fixed-width Numeric formatting. Focused and hosted checks
passed; the two review documentation mismatches were fixed and their
conversations resolved.

Earlier: issue #6623 (PR #6664, merge `91c0f7165`) finished 2026-09-27: blank SDF
Numeric/Float cells now import as non-NULL zero at the target field's scale,
including nullable Numeric targets, without shifting later fields. Focused and
hosted checks passed; all review conversations were resolved.

Earlier: issue #6636 (PR #6663, merge `21ba05355`) finished 2026-09-27:
nullable DIF numeric, Character, and Logical cells now match VFP9, while SYLK
emits blank `K` cells without shifting later fields. Review added explicit
stale-physical-byte protection for nullable Character/Varchar fields. Focused
and hosted checks passed; all review conversations were resolved.

Earlier: issue #6609 (PR #6662, merge `c3e62e4e7`) finished 2026-09-27:
text-to-Memo SDF/CSV imports now use an explicit session-scoped VFP/Copperfin
policy, strict CSV enclosure preflight, localized pre-mutation diagnostics,
and focused DBF/FPT atomicity coverage. Focused and hosted checks passed; all
review conversations were resolved.

Earlier: issue #6612 (PR #6661, merge `9b028f5eb`) finished 2026-09-27: SDF
Autoincrement fields now use VFP's 11-character printable layout while
preserving genuine `0x31` metadata and raw bytes. Focused and hosted checks
passed; all review conversations were resolved.

Earlier: issue #6614 (PR #6660, merge `cf2b95187`) finished 2026-09-27: SDF now uses
Varchar/Varbinary payload widths, uppercase hexadecimal Varbinary text, and
validated case-insensitive hex import with command-atomic rejection of bad
input. Focused and hosted checks passed; the review diagnostic fix was
verified and its conversation resolved.

Earlier: issue #6628 (PR #6659, merge `59fe248ed`) finished 2026-09-27:
DIF/SYLK Date and DateTime export now uses VFP-compatible typed cells,
including blank DateTime handling and the Excel-1900 leap-day discontinuity.
Focused and hosted checks passed; all review conversations were resolved.

Earlier: issue #6630 (PR #6658, merge `0b2a52244`) finished 2026-09-27: CSV and
DELIMITED export/import now project Blob (`W`) fields with the same native
rules as General/Picture, including the parallel SQL-result import path.
Focused and hosted checks passed; the review conversation was resolved.

Earlier: issue #6640 (PR #6657, merge `e3b765653`) finished 2026-09-27:
successful lossy `COPY TO` interchange projections now emit one localized,
non-blocking warning with stable structured output-type and ordered omitted-
field metadata, without disclosing payloads or changing output bytes. Review
extended the event metadata through debugger, Visual Studio, and headless host
protocols and added actual `TYPE TAB` coverage. Focused and hosted checks
passed; all conversations were resolved.

Earlier: issue #6654 (PR #6656, merge `c0a003b43`) finished native
CR-delimited DIF/SYLK imports with Varbinary targets and atomic rollback.

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

Issue #6602: make `APPEND FROM ... TYPE SDF` recognize CR-only physical records
alongside CRLF and LF without changing blank-record or Memo-target behavior.
Branch `codex/implement-6602-sdf-cr-records` contains the focused splitter,
portable delimiter regression, and traceability work. PR #6680 is open against
`v1-development`; the focused `test_prg_engine_data_io` suite passes, and
hosted validation/review are in progress.

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
