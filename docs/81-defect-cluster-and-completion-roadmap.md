# Defect Cluster & Completion Roadmap

## Purpose

This document bridges two roadmap layers that already exist separately in
this repo, neither of which tracks the other:

- **The macro layer** (`docs/05-roadmap.md`, `docs/31-specification-compliance-gap-analysis.md`,
  `docs/23-phase-a-dependency-breakdown.md`, `docs/RELEASE-READINESS-REVIEW.md`)
  organizes the whole product as a completion tree of ten lettered lanes
  (`A`-`J`, see [Lane Status](#lane-status) below), deliberately without
  citing individual issue numbers. It answers "what does v1 completion
  look like."
- **The micro layer** — this document — tracks the actual open,
  `agent-approved` bug/gap backlog (**934 open issues** as of 2026-09-23,
  up from ~280 when this tracking started 2026-09-19) as thematic
  *clusters*, since most bugs in this backlog share a root cause with
  several siblings, and fixing the shared cause or a shared helper often
  closes multiple issues for the cost of one. It answers "which issue to
  pick up next."

**Every cluster below is Lane A/C/G-adjacent runtime, format, and
command-surface work.** Lanes A and C are recorded "Closed" in docs/05 at
the MVP/implementation-complete level, but that closure was a coarse
milestone, not a claim that every command/expression/format edge case is
bug-free — this granular backlog is exactly the residue that ongoing
agent-driven bug hunts keep finding underneath an already-shipped surface.
That is expected, not a regression in lane status; docs/05 itself frames
completion as revisited "only if a regression, new compatibility evidence,
or release-validation failure creates a new acceptance gap," which is
precisely what each cluster below represents.

**How to use this:** when picking "next work" with no other explicit
instruction, take the next `NOT STARTED` cluster in [Cluster Order](#cluster-order)
below. Within a cluster, pick issues by shared-root-cause leverage — fix
the common helper first if one exists. Update this file's status markers
as clusters progress; the companion memory file
`project_copperfin_issue_cluster_roadmap.md` should stay a short pointer
to this document rather than duplicating its content going forward.

## Scale reality

The 12 clusters below, plus the two currently-tracked umbrella/singleton
groups, account for on the order of 150-250 of the 934 open issues (rough
estimate from original per-cluster counts, several already partly worked
down). **The remainder has not been clustered yet.** As clusters get
exhausted, or on request, re-run a full categorization sweep over
`gh issue list --label agent-approved --state open` rather than assuming
the list below is exhaustive. The 35 lettered-lane epics (`#1`-`#46`,
titled `A1:`.../`J3:`) are the natural top-level bucket every concrete bug
should roll up under once categorized; use them as the categorization
axis for any future sweep.

Only 3 open issues currently lack `agent-approved` (out of 937 total open):
`#4905` (owner-policy, unrelated to bug work) and `#6499`/`#6506` (both
self-filed this session, pending owner review — expected, not a backlog
gap). The Agent Issue Intake Boundary is not currently a meaningful
bottleneck; the backlog is effectively fully intake-cleared.

## Lane Status

Summarized from `docs/05-roadmap.md`'s Lettered Lane History table — see
that document for the authoritative, evidence-backed version. Not
reproduced here except as a locator for where cluster work fits:

| Lane | Theme | Status |
| --- | --- | --- |
| A | File/index fidelity, work areas/sessions, command/expression surface, OLE/COM automation | Closed (MVP level; this doc's clusters are the ongoing residue) |
| C | xAsset executable-model lifecycle: form/class, report/label, menu lifecycle | Closed (same caveat — clusters 6/7 below are residue) |
| D | Build/compiler/debug pipeline | Recorded slices closed; broader MVP/RC evidence remains |
| E | Shared design model and designer fidelity | Closed 2026-07-24 |
| F | VS extension parity + utility panes; standalone Studio as full IDE | MVP-complete for standalone shell; hosted UI evidence under RC gate |
| G | FoxPro language service: semantic resolution, navigation/refactoring, IntelliSense | Recorded slices closed; broader MVP scope remains |
| H | Relational backend translators, document/vector + AI planning, .NET/MCP/polyglot outputs | Active — cluster 3 (SQL federation) below is part of this lane |
| I | Runtime/project security depth, extension/host/AI-MCP security boundary | Active |
| J | Portable core boundary, macOS port, Linux port | Active |

The `RELEASE-READINESS-REVIEW.md` procedural checklist (the independent
human review gate before first stable release) has not started. Reaching
it requires the live tree under D/F/G to close and this backlog to be
substantially worked down — there is no shortcut around the cluster work
below.

## Cluster Order

Status legend: `NOT STARTED` / `IN PROGRESS` / `DONE`. Update in place as
work lands; when a cluster's last issue closes, move a one-line summary
into `agent-handoff.md`'s history rather than leaving a stale `DONE`
marker with no context here.

### 1. Reentrant-cursor-closure use-after-free — `IN PROGRESS`

Started 2026-09-18. Root cause: several PRG runtime code paths retain a
raw `CursorState*`/reference across evaluation of an expression that can
itself run arbitrary VFP code (a UDF closing the exact cursor being
operated on via `USE IN`/`CLOSE ALL`), then dereference the now-freed
cursor. Fixed via a shared `CursorGenerationReference` capture/revalidate
pattern, applied per-command.

**Closed:** #6243 (REPLACE/UPDATE, PR #6477), #6244 (DELETE/RECALL, PR
#6478 — zero new code, pure verify via already-hardened shared helper),
#6245 (COUNT/SUM/AVERAGE/MIN/MAX/TOTAL, PR #6479), #6246 (INSERT INTO
VALUES/SELECT, PR #6480), #6247 (SET RELATION/SET SKIP, PR #6481), #6269
(SET FILTER, PR #6482 — the one case in this cluster where matching real
VFP9 behavior means graceful non-error completion, not a catchable error;
check each remaining issue's own VFP9-comparison evidence before assuming
the catchable-error pattern), #6251 (SELECT, PR #6483 — the big one, a
single ~900-line shared query-materialization function), #6460 (SET SKIP
TO registration, fixed 2026-09-22).

**Remaining (7):** #6320 (GATHER), #6321/#6322 (APPEND FROM local/remote),
#6331 (SCAN), #6415 (property-read event handlers), #6420 (BINDEVENT),
#6192 (QueryUnload self-release — overlaps cluster 2 below).

**Disclosed, not yet fixed, needs a different mechanism:**
`aggregate_function_value()`'s bare-call form (`? SUM(field FOR cond)` as
opposed to the `SUM ... TO var` command form) and `evaluate_group_aggregate()`'s
`GROUP BY` analogue — both need error propagation through expression
evaluation rather than statement dispatch.

**Related, self-filed this session:** #6499 (cancellation inside an
explicit `FLOCK()`/`RLOCK()` retry loop is silently swallowed instead of
halting) — not `agent-approved` yet, pending owner review.

### 2. Session/task shutdown-cleanup — `NOT STARTED`

All 11 confirmed open: #6183 (unawaited SPAWN workers survive session
destruction), #6192 (QueryUnload self-release UAF — shared with cluster
1), #6193 (QueryUnload `RETURN .F.` incorrectly vetoes QUIT), #6194
(architecture: selectable VFP9/Copperfin shutdown modes), #6195 (CLOSE
ALL invalidates live objects without releasing), #6196 (CLOSE DATABASES
prematurely closes file handles), #6197 (scoped CLOSE discards unrelated
cursors), #6198 (CLOSE revokes Foxtools handles), #6199 (ON SHUTDOWN
ignores non-DO commands), #6263 (session destruction leaves uncommitted
transaction changes live), #6454 (Destroy-during-QUIT bypasses shutdown
cleanup). Directly adjacent to the #6453 (`cleanup_runtime_resources_for_shutdown()`/
`perform_quit()`) work already done.

### 3. SQL Federation / cross-backend translation — `NOT STARTED`

~25 issues, part of Lane H: #6200, #6231-#6238, #6275-#6277, #6284-#6287,
#6291, #6356-#6369. VFP operators (`#`, `$`, `==`, `%`, `^`, dotted
logicals), `NULL`/date literals, `ISNULL()`/`NVL()`/`ALLTRIM()`, `TOP`,
`IIF` mistranslate or produce invalid SQL across MySQL/Postgres/SQL
Server/Oracle/SQLite. Builds on prior vendor-dialect-exporter context
([[project_copperfin_db_federation_priority]]).

### 4. Malformed-DBF-input data integrity — `NOT STARTED`

~15 issues: #6127, #6158-#6162, #6176, #6184-#6190, #6218-#6219,
#6254-#6257. Corrupt/malformed bytes silently become plausible-looking
wrong values instead of failing closed (Numeric/Logical/Date retyped as
Character, bad deletion markers, undersized fields, bad memo pointers,
NUL field names, unsupported version bytes, DBC backlinks). Directly
adjacent to this session's `_NullFlags`/DBF-format work (#6047) — the DBF
byte-format context is warm.

### 5. Native object/reflection model — `NOT STARTED`

~15 issues: #6289-#6304. `AddObject` leaks/duplicates children, Grid
`AddColumn`/`DeleteColumn` no-ops, missing methods/properties fabricate
values instead of erroring, `ADDPROPERTY`/`AMEMBERS`/`ALEN`/`COMPOBJ`
wrong semantics.

### 6. REPORT/COPY TO memory & correctness — `NOT STARTED`

~10 issues: #6203-#6209, #6238-#6239. Full result sets materialized in
memory (DoS-shaped), `_TALLY` never updated, scope/`WHILE` ignored on
`COPY TO`, bare `REPORT FORM` becomes interactive, saved data
environments ignored.

### 7. Menu/Popup subsystem — `NOT STARTED` (feature-gap, not a bug cluster)

~25 issues: #6151-#6182, #6211-#6212, #6225-#6227, #6430. `DEFINE
BAR/POPUP/MENU`, `ACTIVATE`, `ON SELECTION`, `HIDE/SHOW/RELEASE` are
largely no-ops.

### 8. Legacy interactive/screen I/O — `NOT STARTED` (feature-gap)

~15 issues: #6306-#6318, #6432-#6437. `INPUT`/`ACCEPT`/`WAIT`/`BROWSE`/
`@ SAY GET`/`SORT TO` never actually do anything; legacy `UPDATE ON`/
`JOIN WITH`/`RUN`/`!` misparsed.

### 9. SET command long tail — `NOT STARTED`

~60+ small, mostly independent issues (a grind-through backlog rather
than a shared-root-cause cluster): `SET LOCK/KEY/ORDER/CLASSLIB/RESOURCE/
COMPATIBLE/ENGINEBEHAVIOR/NULL/CARRY/ANSI/MULTILOCKS/OPTIMIZE/TALK/
SYSMENU/TYPEAHEAD/FULLPATH/FUNCTION/CONFIRM/DEVELOPMENT/OLEOBJECT/
BROWSEIME/NOTIFY/STATUS/BELL/CURSOR/STEP/ECHO/TRBETWEEN/VIEW/
REPORTBEHAVIOR/PROCEDURE/ESCAPE/AUTOINCERROR/SQLBUFFERING/UNIQUE/
VARCHARMAPPING/TABLEPROMPT/NOCPTRANS/CPDIALOG/COMPILE/EVENTLIST/
EVENTTRACKING/COVERAGE/DEBUGOUT/TABLEVALIDATE/KEYCOMP/STRICTDATE/
NULLDISPLAY` and more, inert or wrong-default. Sample spot-checked open:
#6264, #6266, #6267.

### 10. Access saved-query extraction — `NOT STARTED`

~5 issues: #6423-#6427. Drops joins/`GROUP BY`/aliases/`SELECT *`/`DESC`.
Small, tight, in Access-migration code already explored this project
([[project_copperfin_access_migration_priority]]).

### 11. Studio/Designer/Builder — `NOT STARTED`

~10 issues: #6147-#6149, #6281-#6282, #6416-#6419. Undo corruption,
dry-run misreporting, builder bypassing read-only document state, stale
snapshot after successful build.

### 12. DLL/native interop — `NOT STARTED`

~6 issues: #6128, #6134, #6202, #6353-#6355. Win64 DECLARE truncation,
managed DECLARE by-ref writeback loss (SPAWN-adjacent shape), COM event
binding leak, `CLEAR DLLS`/`ADLLS`.

**Not itemized as a cluster** (too broadly scattered / one-off): #6129-#6131
(FLOCK/RLOCK cross-process coordination), #6264/#6266-#6267 (SET
LOCK/KEY/INDEX, overlaps cluster 9), #6342-#6348/#6380-#6383 (APPEND
FROM/COPY TO format-specific gaps).

## Umbrella and singleton tracking (outside the 12 clusters)

### Coverage-hunt umbrella #6498 — `IN PROGRESS`

Three owner-labeled `agent-approved` children, a targeted coverage
campaign rather than a shared-root-cause cluster:

- #6495 (concurrency state-sequence hunt) — `DONE`, PR #6500, merged
  `bac49af30`. Surfaced #6499 (filed, not fixed, see cluster 1 above).
- #6496 (migration fidelity: NULL, deleted rows, memos, multi-file
  recovery) — `NOT STARTED`, next up. Its bounded campaign already hit
  three prerequisite defects (#5631, #5567, #6047), all now fixed and
  merged, so it can proceed on a corrected baseline.
- #6497 (release-lifecycle coverage) — `NOT STARTED`, queued behind
  #6496.

### Singleton follow-ups

- #6499 (FLOCK/RLOCK cancellation swallowed in retry loop) — filed, not
  fixed, not yet `agent-approved`. Belongs to cluster 1's pattern but
  needs its own fix.
- #6506 (`PrgValue`/`VARTYPE()`/`EMPTY()`/`NVL()`/`EVL()`/aggregate-on-NULL
  semantics redesign) — filed, not started, not yet `agent-approved`.
  Deliberately deferred out of #6047's storage-layer scope; needs VFP9
  VM verification for several behaviors before implementing (see the
  issue body for the specific open questions).
- #6492 (`PREVIEW` inside a quoted REPORT/LABEL `TO FILE` path incorrectly
  enters preview mode) — reproduced, not fixed. No production fix
  committed yet; a temporary regression test was removed after the
  reproduction.
- #5680 (staged-import authority) — open, partial. Not otherwise
  clustered; revisit scope when picked up.

## Recommended order

1. Finish #6496 then #6497 to close out umbrella #6498 (small, 2 issues,
   already "next" per `agent-handoff.md`, warm context from #6047/#5567/
   #5631).
2. Return to cluster 1's remaining 7 issues to close it out entirely —
   warm context, well-understood fix pattern, closest cluster to `DONE`.
3. Cluster 4 (malformed-DBF-input) — DBF byte-format context is warm from
   #6047/cluster-1 work; natural next pick even though it's a fresh
   cluster.
4. Cluster 2 (session/shutdown) — adjacent to recently-closed #6453 work.
5. Clusters 3, 5, 6, 9, 10, 11, 12 in roughly listed order — no strong
   sequencing dependency between them; pick by whichever has the most
   current context or owner interest at the time.
6. Clusters 7, 8 (Menu/Popup, legacy interactive I/O) last — these are
   feature-gap areas (large implementation lifts), not bug clusters, and
   benefit from being tackled with more design headroom than a
   between-other-things slice affords.

Re-survey the full backlog (`gh issue list --label agent-approved --state
open`) once clusters 1-4 are exhausted, since the >900-issue pool is far
larger than what's clustered here and new waves land periodically (see
[[project_copperfin_codex_availability]]).
