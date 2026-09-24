# Defect Cluster & Completion Roadmap

## Purpose

This document bridges two roadmap layers that already exist separately in
this repo, neither of which tracks the other:

- **The macro layer** (`docs/05-roadmap.md`, `docs/31-specification-compliance-gap-analysis.md`,
  `docs/23-phase-a-dependency-breakdown.md`, `docs/RELEASE-READINESS-REVIEW.md`)
  organizes the whole product as a completion tree of nine lettered lanes
  — `A`, `C`-`J`; lane `B` was never assigned, see [Lane Status](#lane-status)
  below — deliberately without citing individual issue numbers. It answers
  "what does v1 completion look like."
- **The micro layer** — this document — tracks the actual open,
  `agent-approved` bug/gap backlog (**934 open issues** as of 2026-09-23,
  up from ~280 when this tracking started 2026-09-19) as thematic
  *clusters*, since most bugs in this backlog share a root cause with
  several siblings, and fixing the shared cause or a shared helper often
  closes multiple issues for the cost of one. It answers "which issue to
  pick up next."

**Clusters below span multiple lanes, not a single one** — most are Lane
A/C runtime, format, and command-surface work (clusters 3, 13, and 21 are
Lane H; cluster 14 is security work adjacent to Lane I; cluster 26 is CI
infrastructure rather than runtime/format/command-surface work). Check
each cluster's own text for its lane, not a blanket assumption. Where a
cluster is Lane A/C: that lane is recorded "Closed" in docs/05 at the
MVP/implementation-complete level, but that closure was a coarse
milestone, not a claim that every command/expression/format edge case is
bug-free — this granular backlog is exactly the residue that ongoing
agent-driven bug hunts keep finding underneath an already-shipped surface.
That is expected, not a regression in lane status; docs/05 itself frames
completion as revisited "only if a regression, new compatibility evidence,
or release-validation failure creates a new acceptance gap," which is
precisely what each Lane-A/C cluster below represents.

**How to use this:** when picking "next work" with no other explicit
instruction, follow [Recommended order](#recommended-order) below, not a
blind "next `NOT STARTED` cluster in listed order" scan — the two
diverge (e.g. the recommended order finishes in-progress cluster 1 and
reads cluster 27 before touching cluster 2, even though cluster 2 is
listed earlier and is itself `NOT STARTED`). First revalidate the chosen
issue's current state, author, and `agent-approved` label live via
`gh issue view`, per the
fail-closed Agent Issue Intake Boundary in `agents.md`; this document is a
dated snapshot, not a substitute for that check, and an issue cited here
can have been closed, relabeled, or edited since. Within a cluster, pick
issues by shared-root-cause leverage — fix the common helper first if one
exists. Update this file's status markers as clusters progress; the
companion memory file `project_copperfin_issue_cluster_roadmap.md` should
stay a short pointer to this document rather than duplicating its content
going forward.

## Scale reality

**Updated 2026-09-23** after a full categorization sweep of the backlog.
The 30 clusters below, plus the umbrella/singleton groups, now account
for roughly **740 of the 934 open issues** (185 from the original 12
clusters' cited numbers/ranges plus backfills, ~530 more from the 18
clusters added in the sweep — both figures approximate, since clusters
are cited as representative ranges, not exhaustively verified
issue-by-issue). The residual ~190 uncategorized issues are a genuine
long tail of scattered one-offs (documentation-only fixes, single
CI/tooling issues, single backlog-grooming issues) not worth forcing into
named clusters; a handful of small groups worth noting without full
cluster status: `#98`/`#105`/`#106`/`#271` (Lane A `SET()`-state-isolation
residual slices under umbrella `#8`), `#222`-`#235` (`[gap-XX]`-labeled
malformed-input test-coverage tracking, overlaps cluster 4's theme but is
itself about coverage gaps, not the underlying defects).

As clusters get exhausted, or periodically regardless, re-run
`gh issue list --label agent-approved --state open --limit 1000
--json number,title,labels,createdAt` and diff against every cluster's
cited numbers, since new waves of `agent-approved` labeling land
periodically and this categorization decays the same way the original
12-cluster list did between 2026-09-19 and
2026-09-23. The lettered-lane root issues cited in `docs/05-roadmap.md`'s
Lettered Lane History table (see [Lane Status](#lane-status) below for
the exact per-lane roots, e.g. Lane A `#7`-`#12`, Lane H `#113`) are the
top-level structural bucket every cluster above rolls up under —
excluded from the counts above since they're long-lived umbrellas, not
closeable bugs. Use that table, not a guessed issue range, as the
categorization axis for any future sweep.

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
human review gate before first stable release) has not started. Neither
that document nor `docs/05-roadmap.md` states D/F/G closure or a specific
backlog fraction as a formal precondition for starting it — this is this
document's own sequencing judgment (working the cluster backlog down
first reduces the chance of the review surfacing defects this doc already
tracks), not a release gate stated elsewhere.

## Cluster Order

Status legend: `NOT STARTED` / `IN PROGRESS` / `DONE`. Update in place as
work lands; when a cluster's last issue closes, move a one-line summary
into `agent-handoff.md`'s history rather than leaving a stale `DONE`
marker with no context here.

### 1. Reentrant-cursor-closure use-after-free — `DONE`

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

**Closed 2026-09-24:** #6320 (GATHER, PR #6520), #6321 (APPEND FROM local
source filter, PR #6521), #6322 (APPEND FROM remote predicates, PR #6524 --
predicate now sees the candidate through a generation-keyed record override
instead of a provisional row), #6331 (SCAN work-area reuse, PR #6526), #6242
(LOCATE/CONTINUE, PR #6529 -- also made `seek_visible_record()`/
`move_by_visible_records()` loss-aware for GO/SKIP and relation walking),
#6241 (COPY TO / COPY TO ARRAY, PR #6534), #6240 (REPORT/LABEL FORM, this
change -- rows now render before the output file is opened).

**Closed 2026-09-24 (object/event lifetime):** #6415 (property-read
handlers releasing their source, PR #6538), #6420 (method before-handlers
releasing their source, PR #6540), #6192 (QueryUnload self-release, this
change). These use the never-reused object handle as the stable identity
and re-check `ole_objects` after each user callback.

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
Server/Oracle/SQLite. Builds on prior vendor-dialect-exporter context;
the Postgres/SQL Server/Oracle connectors are tracked, committed v1
scope under root issue #30 (Lane H), not optional/future work.

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

~17 issues: #6203-#6209, #6238-#6239, plus found in the 2026-09-23
full-backlog sweep: #5849 (REPORT/LABEL output not published atomically),
#5850 (cursor state not restored when a report expression fails), #6349
(`COPY TO` clears deletion markers and resurrects records), #6350 (`COPY
TO WITH CDX` drops the structural index), #6351 (`COPY TO DATABASE`
doesn't register the table), #6352 (`COPY STRUCTURE` fails on valid
nullable tables), #6413 (`?`/`??` drop every expression after the first).
Full result sets materialized in memory (DoS-shaped), `_TALLY` never
updated, scope/`WHILE` ignored on `COPY TO`, bare `REPORT FORM` becomes
interactive, saved data environments ignored.

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

Issue numbers backfilled by the 2026-09-23 full-backlog sweep (~64,
consistent with the "~60+" original estimate): #107 (SET FIELDS), #6213
(SAFETY), #6214 (ANSI), #6215 (MULTILOCKS), #6216 (OPTIMIZE), #6217
(TALK), #6220 (SYSMENU), #6221 (unknown SET options fabricate settings),
#6222 (ENGINEBEHAVIOR), #6223 (COMPATIBLE), #6224 (invalid operands
silently clamped), #6228 (CARRY), #6229 (NULL), #6265 (BLOCKSIZE), #6268
(ASSERTS/ASSERT), #6271 (STRICTDATE), #6323 (NOCPTRANS), #6324
(CPDIALOG), #6325 (COMPILE/CPCOMPILE), #6326 (EVENTLIST/EVENTTRACKING),
#6327 (COVERAGE), #6328 (DEBUGOUT), #6329 (CLASSLIB), #6330 (LIBRARY),
#6332 (SYSFORMATS), #6333 (TEXTMERGE), #6334 (ORDER), #6335
(SEEK/INDEXSEEK), #6336 (REPORTBEHAVIOR), #6337 (PROCEDURE), #6338
(RELEASE PROCEDURE misparsed), #6339 (KEY), #6341 (ESCAPE), #6384
(NULLDISPLAY), #6390 (KEYCOMP), #6391 (TABLEVALIDATE), #6392
(AUTOINCERROR), #6393 (SQLBUFFERING), #6395 (UNIQUE), #6396
(VARCHARMAPPING), #6397 (TABLEPROMPT), #6398 (TYPEAHEAD), #6399
(FULLPATH), #6400 (FUNCTION/CLEAR MACROS), #6401 (CONFIRM), #6402
(DEVELOPMENT), #6404 (OLEOBJECT), #6405 (RESOURCE), #6406 (BROWSEIME/
IMEMode/IMESTATUS), #6407 (NOTIFY), #6408 (STATUS/STATUS BAR/MESSAGE/
BRSTATUS), #6409 (BELL), #6410 (CURSOR/SYS(2002)), #6411 (STEP/ECHO/
TRBETWEEN), #6412 (VIEW), #6414 (HELP/SET HELP/TOPIC).

### 10. Access saved-query extraction — `NOT STARTED`

~5 issues: #6423-#6427. Drops joins/`GROUP BY`/aliases/`SELECT *`/`DESC`.
Small, tight, in Access-migration code already explored this project:
Access is rumored EOL so `IMPORT TYPE ACCESS` fidelity matters, VBA
translation (not just extraction) is confirmed genuinely hard and stays
out of scope, and forms/VBA reconnaissance for this area is solved via
`Application.SaveAsText` COM automation.

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

## Additional clusters (2026-09-23 full-backlog sweep)

The original 12 clusters above were built from a skim of the backlog on
2026-09-19, when it held ~280 open issues; the backlog has since grown to
934. This sweep categorized the previously-untracked remainder — 711
issues, concentrated in two ranges the original clusters never touched at
all (#5500-#5999: 391 issues; #1000-#5499: 69 issues) plus a further 220
in #6000-#6499 the original clusters only partially covered. Some of that
711 turned out to belong to the 12 clusters above by theme even though
the original text never cited their issue numbers (backfilled into
clusters 1, 6, and 9 above rather than duplicated here).

Ranges below are cited the same way as clusters 1-12 (representative,
not necessarily gap-free) and were spot-checked, not individually
verified issue-by-issue — see [Scale reality](#scale-reality) for the
residual-tail accounting.

### 13. EXPORT/IMPORT DATABASE data-integrity & cross-format fidelity — `NOT STARTED`

~70 issues, part of Lane H: #5563-#5657 (most of this range), #5664,
#5667, #5672, #5674, #5675, #5683, #5684, #5692, #5695, #5701, #5705,
#5706, #5708, #5712-#5716, #5742, #5748-#5750, #5760-#5763. The largest
single-root-cause-coherent new cluster (cluster 17 below is larger in
raw issue count but explicitly needs sub-splitting rather than being one
coherent unit). A systematic bug-hunt wave against `EXPORT
DATABASE`/`IMPORT DATABASE` across every target (JSON, SQL Server,
MySQL, XBASE, Access): silent data loss (NULL-as-zero, dropped Unicode,
dropped catalog properties/precision/DCT properties), false success
reporting (missing tables, unreadable catalogs, ambiguous case-folded
files, truncated DBC treated as valid), and outright corruption
(overlapping-field serialization, duplicate identities, impossible
numeric precision accepted). Overlaps cluster 4 (malformed-DBF-input) at
the edges — cluster 4 is about a hostile/corrupt *DBF* being read;
this cluster is about a *correct* source being mis-translated on export
or import.

### 14. Shared-temp-directory / symlink-race / TOCTOU security hardening — `NOT STARTED`

~45 issues, cross-cutting many subsystems rather than one root cause,
all `safety`+`security`-labeled: #5574, #5578, #5588, #5591, #5592,
#5597, #5599, #5600, #5605, #5607, #5613, #5614, #5616, #5621, #5627,
#5628, #5669, #5679, #5681, #5682, #5702, #5709, #5726, #5761, #5769,
#5770, #5771, #5774-#5780, #5784, #5791, #5792, #5795, #5834, #5835,
#5846, #6386. Designer undo journals, xAsset bootstrap wrappers,
`NEWOBJECT` VCX source, runtime-bridge bootstrap, CDX/DBF staging,
license loading, native-wrapper builds, and package staging all follow a
predictable shared-temp path or a symlink an attacker can pre-place,
instead of using an exclusively-created, identity-checked location. A
single shared fix pattern (matching this project's own established
"private, identity-receipted, exclusively-created staging" convention
used elsewhere) likely closes most of these at once if applied as a
shared helper rather than per-site.

### 15. Resource-exhaustion / unbounded-allocation DoS hardening — `NOT STARTED`

~45 issues: #5594, #5595, #5598, #5608, #5609, #5611, #5612, #5615,
#5623, #5628, #5642, #5686, #5687, #5703, #5728, #5731, #5740, #5741,
#5759, #5764-#5768, #5782, #5787, #5790, #5800, #5804, #5811, #5828,
#5946, #6003, #6004, #6029, #6030, #6050. Functions/paths that read an
entire file, buffer an entire request, or materialize an entire
structure into memory with no size/length/depth bound before validating
it — `FILETOSTR`, `XMLTOCURSOR`, `AGETFILEVERSION`, array/list-control
materialization, PRG include-chain depth, audit-log rewrites, project
inventory enumeration, `SPACE`/`REPLICATE`, `PADL`/`PADR`/`PADC`,
`FREAD`/`FGETS`. Distinct from cluster 14 (both are `safety`-labeled, but
this is about size/depth bounds, not path/identity races) and from
cluster 4 (this is a resource bound, not a data-integrity/corruption
outcome).

### 16. CDX/NDX/MDX/IDX index format integrity — `NOT STARTED`

~20 issues: #5589, #5603, #5604, #5615, #5618-#5622, #5676, #5855-#5857,
#6052, #6053, #6089, #6099-#6101, #6138. Index writers publish entries
that don't exist in the DBF, accept/produce tag-name and key-expression
mismatches between writer and reader, don't sync durably, can't replace
an existing destination on Windows, and readers accept zero-filled or
structurally invalid roots/blocks as valid. `INDEX ON`/`REINDEX`/`DELETE
TAG` are also silently no-ops (#6099-#6101) — could arguably split into
its own cluster if picked up separately from the byte-format issues.

### 17. Built-in function correctness — `NOT STARTED`

**By far the largest new cluster, ~150+ issues** spanning numeric
(`ROUND`, `MOD`, `CEILING`/`FLOOR`, `CHR`/`STR` truncation-vs-rounding),
string (`PAD*`, `SOUNDEX`, `STRCONV`, `PROPER`, `LTRIM`/`RTRIM`/`ALLTRIM`,
`STREXTRACT`, code-page/non-ASCII handling), array (`AFIELDS`, `ADIR`,
`AUSED`, `ACOPY`, `ALINES`, `ASORT`, `ASCAN`), file I/O (`FPUTS`,
`FGETS`, `FCHSIZE`, `FFLUSH`, `FCLOSE`, `FSEEK`, `FWRITE`), date/time
(`WEEK`, `DOW`, `QUARTER`, `MDY`, `TIME`, `SECONDS`, `DATE`/`DATETIME`
argument handling), and type-system (`VARTYPE`, `TYPE`, `TRANSFORM`,
`CAST`) built-ins. Representative range: #5565, #5570, #5704-#5716,
#5877-#5942, #5980-#5999, #6002, #6005, #6013-#6016, #6027, #6031-#6034,
#6039, #6042, #6046-#6062, #6144, #6145. **Needs sub-splitting by
function family when picked up** — this range is too large and too
low-leverage-shared (each function is its own independent bug, not a
shared root cause) to work as one slice; treat the family groupings in
this paragraph as the natural sub-cluster boundaries. Closely related to
cluster 24 (NULL/type-coercion) and cluster 25 (Currency precision)
below — those are kept separate because they share an actual root cause
across their own issues, unlike this grab-bag.

### 18. DBC (Database Container) command surface non-functional — `NOT STARTED`

~25 issues, unusually coherent for this size: #6099-#6126 (most of this
range). Nearly every DBC-scoped command is either a complete no-op that
reports success (`CREATE DATABASE`, `ADD TABLE`, `REMOVE TABLE`, `DROP
TABLE`, `CREATE CONNECTION`, `CREATE TRIGGER`), silently misrouted to
operate on the currently-selected table instead of the database
(`PACK DATABASE`, `DELETE DATABASE`, `DELETE VIEW`, `DELETE CONNECTION`),
or simply ignored (`CREATE SQL VIEW`, `RENAME TABLE`, `DBGETPROP`/
`DBSETPROP`, `ADBOBJECTS`, DBC field/record validation rules and
defaults, long field names, autoincrement `NextValue`). This reads as an
entire command family that was stubbed once and never implemented –
worth checking whether one shared DBC-dispatch entry point explains all
of it before fixing each command independently.

### 19. Concurrent/multi-process DBF mutation corruption — `NOT STARTED`

~8 issues: #5670, #5671, #6087, #6088, #6094, #6191. Concurrent
`APPEND`/field-`REPLACE` operations lose successful writes or mix rows
from different FPT generations; `APPEND FROM` variants bypass held table
locks; targeted append retries after a late write failure can duplicate
records. Distinct from cluster 1 (single-process reentrant UAF via
callback) — this is genuine multi-process/multi-session race behavior.

### 20. Access/Jet migration read-side gaps — `NOT STARTED`

~10 issues: #5538, #5539, #5541, #5547, #5688, #5760, #5831, #5832,
#6038. Multi-page Access TDEFs get omitted instead of reassembled, Jet3
text decoding corrupts non-ASCII, TDEF index/relationship metadata isn't
decoded, Jet4 compressed-Unicode decoding is incomplete. Same
Access-migration code area as cluster 10 (saved-query extraction), with
real Jet3/Jet4 MDB fixtures (including a suspected Access 2.0 specimen)
available locally.

### 21. .NET/C# polyglot code-gen correctness — `NOT STARTED`

~20 issues, part of Lane H: #5851, #5861-#5866, #5871, #5892, #6135,
#6137, #6248, #6249, #6305, #6378, #6428, #6429, #6431. Two related
sub-themes: generated C# quality (reserved-keyword identifier collisions,
duplicate type names across same-root forms, dropped procedure
arguments/return values, assignments copied instead of lowered, `WAIT`
printed as literal source text) and the LINQ descriptor generator
(empty/duplicated catalogs, syntactically-impossible SELECTs accepted,
`GROUP BY`/`HAVING`/join/`UNION` structure discarded).

### 22. xAsset/Studio real-fixture format-parity gaps — `NOT STARTED`

~20 issues: #5957-#5975, #6078-#6083. Studio flattens real VFP9 MNX menu
hierarchies, misplaces nearly every object in real FRX reports, drops
qualified SCX parent paths, and several generated output formats (APP,
FXP, OCX, FLL) are confirmed byte-format-invalid against real VFP9 via
this project's own fixture-based verification discipline (`bug,
test-coverage` labels). Also Visual Studio project-analysis scanners
corrupting legacy ANSI source and misinterpreting comments/TEXT payloads
as executable. Related to, but distinct from, cluster 11
(Studio/Designer/Builder, which is about designer UNDO/dry-run/read-only
violations rather than output-format fidelity) and cluster 7
(Menu/Popup, which is runtime MNX *behavior* rather than Studio's
MNX *parsing*).

### 23. COM/event/object-lifecycle reentrancy safety — `NOT STARTED`

~12 issues: #5841, #5847, #5858-#5860, #5870, #6098, #6278-#6280, #6375.
Native object construction UAF when `Init` releases the object mid-call,
UI objects and event sources not revalidated after a reentrant handler
releases them, COM event delivery deadlocking on reentrant disconnect,
form `Release()` destroying child controls before `Destroy` fires or
leaving a phantom reference. Thematically adjacent to cluster 1
(reentrant-cursor UAF) and cluster 2 (session/shutdown) but about
native/COM object lifetime rather than `CursorState`.

### 24. Type-coercion / three-valued NULL-logic correctness — `NOT STARTED`

~12 issues: #5934, #5936, #5939, #5940, #5942, #5979, #6034, #6140,
#6141, #6142. `EMPTY()`/`ISBLANK()` report `NULL` as empty/blank instead
of `.F.`, logical operators and arithmetic silently truth-test or
coerce non-Logical/NULL operands, `INLIST` coerces null comparisons,
numeric equality merges distinct values within an epsilon. **Directly
overlaps issue #6506** (already tracked as a singleton follow-up above,
filed from this session's #6047 work) — when #6506 is picked up, survey
this cluster's issues first, since several may already be covered by
whatever `PrgValue`/`is_null` representation change #6506 ends up
choosing.

### 25. Currency/numeric-precision type fidelity — `NOT STARTED`

~8 issues: #6035-#6041. `Currency` results from external functions and
aggregates lose fixed-point precision through `double` accumulators,
`CEILING`/`FLOOR` miscompute negative Currency, Date/DateTime/Character
`MIN`/`MAX` aggregates fabricate a Number zero instead of the correct
type. Could be folded into cluster 17 (built-in function correctness) if
picked up together, but kept separate here since these five share one
real root cause (`Currency`'s internal representation not being
preserved through the aggregate/function pipeline).

### 26. CI/build/test infrastructure fixes — `NOT STARTED` (not product bugs)

~15 issues: #5801, #5802, #5805, #5809, #5815, #5816, #5820-#5829.
Windows installer lane aborting on one bad mirror, sanitizer/exporter
coverage gaps, VSIX lifecycle gate failures on VS2026, test fixtures
colliding across concurrent build trees. Lower priority than the
product-facing clusters above — these affect development velocity, not
shipped behavior — but still open and `agent-approved`.

### 27. `[Architecture]` cross-cutting design proposals — `NOT STARTED` (not closeable bugs)

~9 issues: #6017-#6025, #6043. Proposals for durable-publication/
multi-file-recovery contract unification, shared callback/reentrancy/
shutdown ownership rules, legacy-bytes-vs-Unicode boundary separation,
schema-evolution standardization, explicit capability-admission
semantics, and optimizer-correctness/cost-ranking separation. These are
design decisions that would *inform* how several of the clusters above
get fixed, not independently closeable defects — read before starting
cluster 1, 2, 14, or 19 in particular, since their proposals overlap
those clusters' actual fix patterns.

### 28. Native PRG class-system parity residuals (`#3217` umbrella) — `NOT STARTED`

~12 issues, all legacy-numbered slices of umbrella #3217: #3223, #3256,
#3263, #3277, #3334, #3754, #3829, #4239, #4431, #4432, #4617, #4618,
#4630, #4686, #4747, #4753, #5123. `NEWOBJECT` forwarding reserved
slots into `Init`, declarative `ADD OBJECT` support, `ACLASS` appending
a nonexistent ancestor, `PageFrame` mutation parity, `SET SKIP OF
BAR`/`MRKBAR` state, one-to-many `SET SKIP` navigation, native list
collection/`ListBox` parity, defined-menu lifecycle.

### 29. Runtime surface stub replacement (`#3249` umbrella) — `NOT STARTED`

~5 issues: #3250, #3252, #3253, #3254, plus #6044 (found in this sweep,
same theme). `GETPICT`/`GETCOLOR`/`GETFONT`/`VARREAD` host-contract
depth, `AFONT`/`APRINTERS` fixed-stub enumeration, `AGETFILEVERSION`
metadata parity.

### 30. Foundational SQL SELECT / buffering-mode gaps (legacy-numbered) — `NOT STARTED`

~5 issues, high severity despite the small count: #3852 (`TABLEUPDATE()`/
`TABLEREVERT()` don't exist; `CURSORSETPROP`/`CURSORGETPROP` are a
complete no-op stub — buffering modes entirely unimplemented at the time
this was filed; note this may already be substantially addressed by this
session's buffered-`REPLACE`/`TABLEUPDATE()` work on #6047's PR #6505 —
verify current state before assuming it's still fully open), #3958
(`COPY STRUCTURE EXTENDED TO` entirely unimplemented), #3960 (`GROUP
BY`/`HAVING` entirely unimplemented, silently corrupts the query
instead), #3966 (`TOP n` not recognized, corrupts the projection clause
instead of limiting rows), #3972 (aggregate functions in `SELECT` don't
work at all). Predates cluster 3 (SQL federation, which is about
*translating* VFP SQL to other backends) — this is the *native* VFP SQL
engine's own foundational gaps.

## Umbrella and singleton tracking (outside the 12 clusters)

### Coverage-hunt umbrella #6498 — `IN PROGRESS`

A targeted coverage campaign (not a shared-root-cause cluster) linking 8
issues total: 3 newly-scoped children plus 5 pre-existing "do not
duplicate" issues the umbrella explicitly tracks to closure. Per the
umbrella's own text: "Close this cluster only when each linked issue is
closed with evidence or an explicit, owner-reviewed exclusion is
recorded" — so #6498 itself does not close on the 3 children alone.

Newly-scoped children:

- #6495 (concurrency state-sequence hunt) — `DONE`, PR #6500, merged
  `bac49af30`. Surfaced #6499 (filed, not fixed, see cluster 1 above).
- #6496 (migration fidelity: NULL, deleted rows, memos, multi-file
  recovery) — `NOT STARTED`, next up. Its bounded campaign already hit
  three prerequisite defects (#5631, #5567, #6047), all now fixed and
  merged, so it can proceed on a corrected baseline.
- #6497 (release-lifecycle coverage) — `NOT STARTED`, queued behind
  #6496.

Pre-existing linked issues (all confirmed `OPEN` as of 2026-09-23, not
otherwise clustered above):

- #5800 (fuzz coverage for most production parsers beyond the DBF
  header) — `NOT STARTED`.
- #5804 (deterministic allocation-failure coverage for critical
  operations) — `NOT STARTED`.
- #222 (`[gap-03]` disk I/O failure injection / staged-write rollback)
  — `NOT STARTED`.
- #5802 (continuous real-VFP9 interoperability validation provisioning)
  — `NOT STARTED`.
- #4403 (MVP release: archive runtime recovery traceability evidence)
  — `NOT STARTED`.

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

1. Finish #6496 then #6497 (already "next" per `agent-handoff.md`, warm
   context from #6047/#5567/#5631) — this does not close umbrella #6498
   by itself, since 5 pre-existing linked issues (#5800, #5804, #222,
   #5802, #4403) also gate its closure; picking those up is a separate,
   later decision.
2. Return to cluster 1's remaining 10 issues to close it out entirely —
   warm context, well-understood fix pattern, closest cluster to `DONE`
   (grew from 7 to 10 after the sweep found #6240/#6241/#6242 belong
   here too).
3. Cluster 27 (`[Architecture]` proposals) — read-only, small (9 issues),
   and several of its proposals directly inform how to approach clusters
   1, 2, 14, and 19; cheap to read now before committing to a fix
   pattern in those larger clusters.
4. Cluster 4 (malformed-DBF-input) and/or cluster 13 (EXPORT/IMPORT
   DATABASE fidelity) — DBF/DBC byte-format and migration context is
   warm from #6047/cluster-1 work; cluster 13 is the largest
   single-root-cause-coherent new cluster (~70 issues) and directly
   adjacent to #6496's own migration-fidelity coverage hunt, so doing
   them back-to-back has real shared-context leverage.
5. Cluster 18 (DBC command surface) — unusually coherent and
   self-contained (~25 issues) for its size; good candidate for a single
   focused pass once a shared DBC-dispatch entry point is confirmed.
6. Cluster 2 (session/shutdown) — adjacent to recently-closed #6453 work.
7. Cluster 24 (NULL/type-coercion) — read alongside singleton #6506
   before starting either, since they likely share a fix.
8. Clusters 14, 15 (shared-temp/symlink races; resource-exhaustion DoS)
   — both cross-cutting `safety`/`security` hardening passes, worth
   doing together since a shared helper may close much of both at once.
9. Clusters 3, 5, 6, 9, 10, 11, 12, 16, 19, 20, 21, 22, 23, 25, 28, 29,
   30 in roughly listed order — no strong sequencing dependency between
   them; pick by whichever has the most current context or owner
   interest at the time. Cluster 17 (built-in function correctness) is
   the largest of all (~150+ issues) but explicitly needs sub-splitting
   by function family before picking it up — don't take it as one slice.
10. Clusters 7, 8 (Menu/Popup, legacy interactive I/O) last — these are
    feature-gap areas (large implementation lifts), not bug clusters,
    and benefit from being tackled with more design headroom than a
    between-other-things slice affords.
11. Cluster 26 (CI/build/test infra) — lowest priority; doesn't affect
    shipped product behavior, pick up opportunistically.

Re-survey the full backlog periodically (see [Scale reality](#scale-reality)
for the exact command) — this categorization will itself go stale the
same way the original 12-cluster list did.
