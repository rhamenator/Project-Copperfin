# Access Saved-Query Extraction

Written for issue #5479 (parent #138): `copperfin::vfp::scan_access_saved_queries()`
in `src/vfp/access_saved_queries.cpp`, which reconstructs Access saved
queries as SQL text.

## Why this slice was tractable when #5477/#5478 were not

This session also investigated #5477 (forms/reports structural inspection)
and #5478 (VBA/Access Basic extraction) and found both genuinely
undocumented by every currently-allowed evidence source: `mdbtools`'
`HACKING.md` has zero coverage of form/report or VBA-module physical
storage, and `oletools`/`olevba` (an independent, actively-maintained
open-source tool built specifically for VBA-macro extraction) has Access
support explicitly listed as an unimplemented TODO in its own source. See
the investigation comments on those two issues for the full record.

Saved queries turned out to need **no new physical-format
reverse-engineering at all**: a query's definition is stored as ordinary
rows in the `MSysQueries` system table -- a regular Jet table, readable
with this codebase's existing TDEF/row-decoding machinery (#5476, #5539)
plus, for the `Expression` memo column, #5549's long-value reader. No
opaque binary blob, no embedded OLE container.

## Evidence basis

Grounded in `mdbtools`' own GPL-licensed **source code** (`src/util/mdb-queries.c`,
`include/mdbtools.h.in`) -- not just its `HACKING.md` prose, which does
not mention queries at all. `HACKING.md` itself already establishes this
codebase's own precedent for treating `mdbtools` source files as
legitimate evidence ("See props.c for an example," in its Properties
section). Downloaded via `apt-get source mdbtools` for this slice's
development (GPL-2.0-or-later; not vendored into this repository).

- **Object-type mapping.** `include/mdbtools.h.in`'s enum: `MDB_FORM=0`,
  `MDB_TABLE=1`, `MDB_MACRO=2`, `MDB_SYSTEM_TABLE=3`, `MDB_REPORT=4`,
  `MDB_QUERY=5`, `MDB_LINKED_TABLE=6`, `MDB_MODULE=7`,
  `MDB_RELATIONSHIP=8`. `src/libmdb/catalog.c` derives this from
  `MSysObjects.Type`'s low 7 bits: `entry->object_type = (type & 0x7F)`.
  This codebase's own already-verified `Type == 1` == table convention
  (docs/72) matches `MDB_TABLE` exactly, corroborating the scheme.
- **`MSysQueries` row layout and SQL reconstruction.** `mdb-queries.c`
  binds columns `Attribute`, `Expression`, `Flag`, `Name1`, `Name2`,
  `ObjectId`, `Order`, and reconstructs one query's SQL by filtering
  `MSysQueries` rows to `ObjectId == <query's catalog Id>` and switching
  on each row's `Attribute` byte: `3` = predicate (`TOP n [PERCENT]` /
  `DISTINCT` / `DISTINCTROW`, from `Flag` bits and `Name1`), `5` = a
  table name (`Name1`) appended to `FROM`, `6` = a column expression
  (`Expression`) appended to the column list, `7` = a join/relationship
  clause (read but not incorporated -- `mdb-queries.c` itself only
  comments the values without using them), `8` = the `WHERE` clause
  (`Expression`), `11` = an `ORDER BY` expression (`Expression`, with
  `Name1 == "D"` appending `DESCENDING`). This slice's `reconstruct_sql()`
  matches this switch exactly, including its documented scope limits (no
  `Order`-based row sequencing, no special handling for
  action/crosstab/union/pass-through/data-definition queries).

### Real-fixture cross-validation

`mdb-queries` (installed locally, not a runtime dependency) was run
against two real local `.mdb` fixtures as independent-reader ground
truth:

- Jet3 fixture (0 saved queries): `scan_access_saved_queries()` correctly
  returns `ok=true` with an empty query list -- no false positives, no
  crash on an empty `MSysQueries`.
- Jet4 fixture (29 saved queries): cross-checked 8 individual queries'
  reconstructed SQL against `mdb-queries <file> "<query name>"`'s own
  output. **7 of 8 matched byte-for-byte** (`Media`, `Channels`,
  `Print Thank-you`, `Print Resumes`, `Lead Type`, and two others).
  One (`Employment Application`) diverged: `mdb-queries` returned an
  empty `SELECT  FROM` while this slice reconstructed a full,
  internally-consistent `SELECT ... FROM [Personal Data],[Application
  Information],...`. `mdb-queries.c`'s own `mdb_get_query_id()` resolves
  a query's `Id` via a **string comparison** of bound column text
  (`mdb_bind_column_by_name`'s own text-conversion layer), which is a
  materially more fragile mechanism than this slice's typed integer
  `ObjectId` comparison -- plausibly a `mdb-queries.c`-side limitation
  rather than a bug in this slice, though this was not independently
  proven beyond noting the mechanism difference. Documented here rather
  than silently resolved in either direction.

## PR review follow-up

Six findings from #5551's review round were addressed:

- **Multiple `ORDER BY` columns collapsed to one (Codex P1).** The
  original `sorting.empty()` guard kept only the first `Attribute == 11`
  row. Fixed to append every sort expression, comma-separated. Re-running
  against the same real Jet4 fixture confirms the fix is a genuine
  improvement, not just a synthetic-test concern: `Print Resumes` now
  reconstructs a 5-column `ORDER BY`, while `mdb-queries` itself still
  only emits the first column (`ORDER BY [Document List].MergeDocument`)
  -- this specific divergence from `mdb-queries`' own ground truth is
  expected and correct, since `mdb-queries.c` has this exact limitation.
- **Unsupported query types silently fabricated as `SELECT` (Codex P1,
  Copilot, duplicate finding).** Neither `mdb-queries.c` nor any
  currently-allowed evidence source documents how to positively identify
  a non-SELECT query type from `MSysQueries`'s own row structure, so
  guessing at undocumented `Attribute` values was not attempted. Added
  the one defensible, evidence-grounded minimum sanity bar instead: a
  query with clause rows but no `Attribute == 5` (table) row -- which no
  real SELECT-shaped query this slice's development observed ever lacked
  -- is now skipped rather than returned as a fabricated
  `SELECT ... FROM ` with an empty FROM clause. Documented as a partial
  mitigation, not full query-type detection (see this file's header
  comment).
- **Stale/freed MSysQueries page not checked against the TDEF's declared
  row count (Codex P2).** `scan_msysqueries_rows()` now tracks
  non-deleted row slots and fails the whole scan closed if that exceeds
  `MSysQueries`'s own declared `row_count`, matching
  `access_msysobjects.cpp`'s own `RowCountExceedsDeclared` precedent.
- **Crash on a non-dense `column_number` sequence (Copilot).** A gap in
  declared column numbers left an `ordered[]` slot null, which was then
  dereferenced unconditionally. Now fails closed with
  `UnexpectedSchema` instead.
- **`NULL` `ObjectId` silently treated as `0` (Copilot).** A
  structurally-decoded row with a `NULL` `ObjectId` could previously be
  misattributed to whatever query happens to have `Id == 0`. Now treated
  as an unattributable-row failure (the same category as other
  structural anomalies), not silently defaulted.

## What this slice implements

- `scan_access_saved_queries()`: for every `MSysObjects` catalog entry
  with `Type & 0x7F == MDB_QUERY (5)`, reconstructs that query's SQL from
  its own `MSysQueries` rows.
- The Jet4 "compressed unicode" text encoding (`0xFF 0xFE`-prefixed --
  #5539's own documented, deliberately-deferred gap for
  `MSysObjects.Name`) turned out to be the **common case** for
  `MSysQueries.Expression` values in real data (simple table/column
  identifiers like `Employers.LeadMedia`). Real-fixture testing found
  every observed compressed value to be pure single-byte-per-character
  content with no embedded `0x00` mode-switch byte (per `mdbtools`'
  `HACKING.md`'s own prose on this encoding) -- this slice decodes that
  verified common case, cross-checked byte-for-byte against
  `mdb-queries`, while failing closed (not guessing) on the mode-switch
  case, which no real fixture available during this slice's development
  ever exercised.
- **Fail-closed on incomplete reconstruction, not silent partial output.**
  A query with at least one of its own clause rows undecodable (e.g. the
  mode-switch case) is recorded in `skipped` -- the whole query, not
  included with silently-missing pieces (a dropped column, a missing
  `WHERE` clause) that would look complete but weren't. A
  structurally-undecodable row with no known `ObjectId` to attribute the
  failure to fails the **entire scan** closed, matching
  `access_msysobjects.cpp`'s own `RowCountExceedsDeclared` precedent for
  provenance uncertainty.

## What this slice deliberately does not implement

- **Writing/mutation of any kind.**
- **Translation into VFP/Copperfin SQL syntax** -- extraction only, per
  this issue's own explicit non-goal.
- **Execution of extracted queries** against any data source.
- **Non-`SELECT` query semantics** (action queries, crosstabs, unions,
  pass-through, data-definition queries) -- `mdb-queries.c`'s own switch
  does not specifically handle these either; this slice matches that
  scope rather than exceeding it without real-fixture evidence for the
  additional `Attribute` values those query types would use.
- **The Jet4 compressed-unicode mode-switch case** -- fails closed
  (see above) rather than guessing at semantics `HACKING.md`'s prose
  does not fully disambiguate.
