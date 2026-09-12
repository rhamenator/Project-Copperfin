# VFP Asset Editing And Execution

## Goal

Copperfin should be able to open, inspect, edit, preserve, and progressively execute legacy VFP application assets.

This includes both source-like artifacts and designer metadata artifacts.

## Priority Asset Families

Projects:

- `.pjx`
- `.pjt`

Forms and classes:

- `.scx`
- `.sct`
- `.vcx`
- `.vct`

Reports:

- `.frx`
- `.frt`

Labels:

- `.lbx`
- `.lbt`

Menus:

- `.mnx`
- `.mnt`

Code and headers:

- `.prg`
- `.h`

Data and metadata:

- `.dbf`
- `.fpt`
- `.cdx`
- `.dbc`

## Product Promise

Copperfin should support these modes:

### 1. Inspect

- read asset metadata safely
- show dependencies and structure
- identify unsupported features

### 2. Round-Trip Edit

- edit supported properties and code
- preserve non-understood metadata where possible
- write files back without destructive reshaping

### 3. Normalize

- import assets into an internal editable workspace model
- generate source-control-safe text representations
- keep links back to original binary assets

### 4. Execute

- run PRG/business logic in the Copperfin runtime
- bind project/forms/reports/menus into a compatibility host
- mix legacy assets with modern connectors and .NET integrations

## Current Implementation Snapshot

The current repo state is no longer only aspirational in this area:

- asset inspection is implemented for `DBF`/`FPT`, `CDX`/`DCX`/`IDX`/`NDX`/`MDX`, and first-pass `DBC` companion discovery/validation
- native DBC catalog export now supports whole-database JSON snapshots that include database identity, catalog-object rows, and table data blocks for resolvable local `TABLE` entries
- DBC `PROPERTIES` memo payloads are now decoded from raw sidecar bytes (no text-mode filtering), enabling first-pass property parity for `DATABASE`/`TABLE`/`VIEW`/`RELATION`/`CONNECTION` metadata extraction
- local-table mutation coverage exists for the shipped runtime flow, including memo-backed writes, structural mutations, compaction/truncation paths, and indexed-table guardrails
- the runtime can execute a substantial first-pass PRG surface, including work areas, local queries/mutations, SQL pass-through / remote cursor behavior, and a growing expression/runtime-helper layer
- xAsset-backed runtime bootstraps now exist for forms/classes, reports/labels, and menus, but these remain first-pass compatibility lanes rather than full VFP parity

This document stays focused on the execution/editing model. Active slice selection lives in live GitHub issue state, `agents.md`, `agent-handoff.md`, and `docs/23-phase-a-dependency-breakdown.md`; `docs/22-vfp-language-reference-coverage.md` is reference coverage for runtime-language slices.

## Editing Strategy

### Binary Asset Rule

For binary VFP assets, Copperfin should not immediately rewrite everything into a new format.

Instead:

1. parse into normalized internal metadata
2. preserve raw/original records where needed
3. edit through structured models
4. write back only through a round-trip-safe serializer

### Source Control Rule

Each editable asset should have:

- original binary source
- normalized internal representation
- canonical text serialization for diff/review when possible

This mirrors the value proposition of tools like SCCText without making the binary asset the only truth humans can inspect.

## Execution Strategy

### Stage 1: Code Execution

Support:

- `.prg`
- command/eval logic
- work area and query operations
- report invocation hooks
- first-pass expression/runtime-helper compatibility used by shipped PRG flows

### Stage 2: Project And Asset Binding

Support:

- project loading
- form/class/report/menu registration
- dependency resolution
- asset identity and lookup
- index and metadata sidecar discovery where legacy assets depend on companion files

### Stage 3: Compatibility Host

Support:

- loading imported forms and class libraries into Copperfin Studio/runtime
- binding controls, data sessions, menus, and reports
- bridging unsupported areas with diagnostics rather than silent failure

### Stage 4: Mixed-Mode Runtime

Support:

- legacy VFP assets
- SQL connectors
- .NET service or library calls
- modern security policy

## Round-Trip Safety Requirements

For supported asset types, Copperfin should:

- preserve object identity and ordering where required
- preserve unsupported fields as opaque data when necessary
- avoid destructive reformatting
- emit explicit warnings when write-back fidelity cannot be guaranteed

## First Milestones

### Milestone A

- inspect PJX/SCX/VCX/FRX/LBX/MNX structure
- produce dependency and property reports

### Milestone B

- round-trip edit a limited subset of properties safely
- serialize canonical text snapshots for diffing

### Milestone C

- load project and PRG execution together
- bind forms/reports into a compatibility workspace

### Milestone D

- mixed-mode execution with .NET and SQL connectors

## How To Use: DBC JSON/SQL/Access Export

Use the native asset-inspector API to export a database container (`.dbc`) into a
single JSON snapshot, a portable SQL script, or an Access/Jet-dialect SQL script.

Copperfin also provides an explicit modernization command for a PRG workflow:

```foxpro
EXPORT DATABASE 'northwind.dbc' TO 'northwind-snapshot' TYPE JSON
EXPORT DATABASE 'northwind.dbc' TO 'northwind-snapshot' TYPE SQL
EXPORT DATABASE 'northwind.dbc' TO 'northwind-snapshot' TYPE ACCESS
EXPORT DATABASE 'northwind.dbc' TO 'northwind-snapshot' TYPE POSTGRESQL
EXPORT DATABASE 'northwind.dbc' TO 'northwind-snapshot' TYPE SQLITE
EXPORT DATABASE 'northwind.dbc' TO 'northwind-snapshot' TYPE SQLSERVER
EXPORT DATABASE 'northwind.dbc' TO 'northwind-snapshot' TYPE ORACLE
```

The command resolves relative source and destination paths from the current default
directory, adds `.json` (`TYPE JSON`) or `.sql` (`TYPE SQL`/`TYPE ACCESS`/
`TYPE POSTGRESQL`/`TYPE SQLITE`/`TYPE SQLSERVER`/`TYPE ORACLE`/`TYPE MYSQL`) when the
destination has no extension, and emits
`runtime.export_database_json`, `runtime.export_database_sql`,
`runtime.export_database_access_sql`, `runtime.export_database_postgresql_sql`,
`runtime.export_database_sqlite_sql`, `runtime.export_database_sqlserver_sql`,
`runtime.export_database_oracle_sql`, or `runtime.export_database_mysql_sql`
on success. It deliberately accepts only the literal `TYPE JSON`, `TYPE SQL`,
`TYPE ACCESS`, `TYPE POSTGRESQL`, `TYPE SQLITE`, `TYPE SQLSERVER`, `TYPE ORACLE`, or
`TYPE MYSQL` forms; the source and destination are quoted
path operands, not expressions. It reads the existing DBC/DBF data before opening
the requested output path, creates a missing output directory, and reports a
localized runtime failure if inspection or output writing fails. A destination
that would resolve to the same file as the source (e.g.
`EXPORT DATABASE 'data.dbc' TO 'data.dbc' TYPE ACCESS`, where an explicit
extension skips the `.sql`/`.json` default) is rejected before anything is read or
written, rather than truncating the source database once export succeeds. All
seven `TYPE` variants share the same DBC catalog/table-resolution path
(`export_database_as_json()`, `export_database_as_sql()`,
`export_database_as_access_sql()`, `export_database_as_postgresql_sql()`,
`export_database_as_sqlite_sql()`, `export_database_as_sqlserver_sql()`, and
`export_database_as_oracle_sql()` all
call the same internal loader) so they cannot silently drift apart on which
tables/rows are considered part of the database -- only the output serialization
differs. Every exporter also fails closed with a localized
`Vfp.AssetInspector.Validation.ExportTableHasNoFields` diagnostic if a cataloged
member DBF's field-descriptor block parses "successfully" with zero fields --
a real, accepted shape (an aligned terminator immediately after the 32-byte
header) that would otherwise emit invalid `CREATE TABLE "name" ( );` DDL on
every SQL dialect, or an ambiguous `"fields": []` JSON marker indistinguishable
from that exporter's own deliberate unreadable-source-table marker (#5697,
found by an automated Codex code-review pass). The source-level asset
inspector reports this same shape as a validation error
(`dbf.field_count_zero`) rather than silently returning no diagnostic.
`TYPE SQL` emits one portable/ANSI-ish dialect (`CREATE TABLE` per table,
`INSERT` per row); it does not target a specific database engine's SQL dialect
quirks. `TYPE POSTGRESQL` (#5537, first vendor-dialect slice of #141) emits the
same `CREATE TABLE`/`INSERT` shape -- real PostgreSQL already accepts `TYPE SQL`'s
double-quoted identifiers, single-quoted string literals, and
`DECIMAL`/`INTEGER`/`DOUBLE PRECISION`/`BOOLEAN`/`DATE`/`TIMESTAMP`/`VARCHAR`/`TEXT`
column types verbatim, per PostgreSQL's own public SQL/DDL documentation -- plus
`CREATE INDEX` statements derived from each table's production `.cdx` index tags,
for a tag whose key expression is a plain column reference (a composite/expression
key is recorded as a skipped-index comment instead of guessed at); index names are
disambiguated (`disambiguate_index_name()`, #5559) across the *entire* export,
since PostgreSQL's relation namespace is schema-wide across both tables and
indexes, truncating first to PostgreSQL's own 63-byte identifier limit so
disambiguation operates on what the real engine would actually see. `TYPE SQLITE`
(#5554, second vendor-dialect slice of #141) emits the identical `TYPE SQL` shape
again, plus the same `CREATE INDEX` derivation `TYPE POSTGRESQL` implements --
real SQLite (3.46.1) directly confirmed to accept this exact dialect: unlike
PostgreSQL's directly-documented types, SQLite is dynamically typed per value and
buckets every one of these type names into a definite "type affinity" by substring
match rather than rejecting an unrecognized one, and a representative two-table
export (with a `VARCHAR` containing an embedded quote, and `DECIMAL`/`BOOLEAN`/
`DATE` columns) loaded into a real SQLite database with zero errors, round-tripped
every row exactly, and a cross-table `JOIN` between the two tables returned the
correct result; index-name disambiguation is likewise schema-wide, matching
SQLite's own identical table/index namespace sharing.

`TYPE SQLSERVER` (#5554, third vendor-dialect slice of #141) is its own dedicated
code path, not a reuse of `TYPE SQL`'s shared table/data writer at all (the same
reason `TYPE ACCESS` has its own inline loop): T-SQL's dialect diverges enough --
square-bracket `[identifier]` quoting (the same bracket-escaping convention
`TYPE ACCESS` uses), `MONEY` for VFP currency (an exact match for its own
4-decimal-digit scale, the same reasoning `TYPE ACCESS` applies to `CURRENCY`),
`BIT` for logical (T-SQL has no `BOOLEAN` type, and -- unlike the other three
dialects, which all accept the `TRUE`/`FALSE` keyword -- a `BIT` column's literal
must be `1`/`0`; real SQL Server rejects `INSERT ... VALUES (TRUE)` outright, since
T-SQL has no boolean-literal syntax outside a predicate context), `DATE`/
`DATETIME2` for VFP date/datetime (`DATETIME2` specifically, Microsoft's own
documented modern replacement for the legacy `DATETIME` type), and `VARCHAR(MAX)`
-- not the deprecated `TEXT` type -- for memo/general/picture fields. It adds the
same `CREATE INDEX` generation `TYPE POSTGRESQL` implements, but scoped to SQL
Server's own 128-character identifier limit, and -- unlike `TYPE POSTGRESQL`/
`TYPE SQLITE` -- disambiguated *per table*, not across the whole export: a real
SQL Server engine directly confirmed that T-SQL index names are scoped to their
own table (two different tables may carry an identically-named index with no
error, and a table may share a name with an unrelated table's own index), so the
one collision that can still arise is truncation itself erasing a tag-derived
suffix when a table name alone is already at or beyond the 128-byte limit. Both
the dialect and the index-namespace scoping were directly confirmed against a
real local SQL Server 2022 (Developer Edition) engine during this issue's own
development: this exporter's own actual generated output for a representative
two-table (customers/orders) fixture -- covering every mapped column type,
`1`/`0` boolean literals, and a `CREATE INDEX` on each table -- loaded with zero
errors, and a cross-table `JOIN` between the two exported tables returned the
correct joined row.

`TYPE ORACLE` (#5554, fourth vendor-dialect slice of #141) is likewise its
own dedicated code path -- Oracle's dialect diverges enough from the
portable baseline that it does not reuse `TYPE SQL`'s shared writer either.
Double-quoted `"identifier"` quoting has *no* escape mechanism for an
embedded quote at all (a real, material difference from every other
dialect above, all of which escape one by doubling it -- directly
confirmed against a real Oracle engine that a doubled-quote identifier
fails outright, so an embedded quote is stripped instead), `NUMBER(19, 4)`
for VFP currency, `INTEGER` (Oracle's own documented `NUMBER(38)`
subtype), `BINARY_DOUBLE` for VFP's own double type, `NUMBER(1)` for
logical with `1`/`0` literals (Oracle has no dedicated table-column
`BOOLEAN` type before Oracle 23c, and even there a `NUMBER(1)` column's
own `TRUE` literal silently converts to `1` anyway, directly confirmed --
so this exporter targets the one literal form every Oracle version
accepts identically), and `CLOB` -- not `VARCHAR2`'s 4000-byte-default
text -- for memo/general/picture fields. `NUMBER`'s own precision is
clamped to Oracle's real 38-digit ceiling and its scale to Oracle's own
independent -84..127 range (a genuine difference from SQL Server's own
DECIMAL, whose scale must not exceed its own precision -- Oracle's own
scale is unrelated to precision entirely, both directly confirmed against
the real engine). A VFP date/datetime value is wrapped in Oracle's own
ANSI-style `DATE 'YYYY-MM-DD'`/`TIMESTAMP 'YYYY-MM-DD HH:MM:SS'` literal
syntax rather than a plain quoted string -- directly confirmed that a bare
ISO-shaped string is *not* safe here, since implicit string-to-date
conversion depends on the session's own `NLS_DATE_FORMAT` (which defaults
to `DD-MON-RR`, not `YYYY-MM-DD`), while the ANSI literal form always
parses in that exact ISO shape regardless of session settings. A blank
VFP date therefore resolves to `NULL` rather than a quoted empty string --
unlike SQL Server's own (data-corrupting-but-not-erroring) blank-date
case, `DATE ''` is invalid Oracle syntax outright, so this is a basic
correctness requirement here, not merely a data-integrity improvement.
`CREATE INDEX` generation is scoped to Oracle's own 128-*byte* identifier
limit (byte-counted like PostgreSQL's own 63-byte limit, not
character-counted like SQL Server's), with index-name disambiguation
threaded across the *whole* export like PostgreSQL/SQLite's own
schema-wide scoping (directly confirmed: two different tables cannot each
carry an index of the identical name) but -- a genuine hybrid of both
precedents above, not matching either exactly -- *not* seeded with
already-emitted table names the way PostgreSQL's own fix is, since Oracle
keeps tables and indexes in separate namespaces (directly confirmed: a
table can share its own name with an unrelated table's index with no
collision at all). All of the above was directly confirmed against a real
local Oracle 23ai engine during this issue's own development, including
this exporter's own actual generated output for a representative
two-table fixture loading with zero errors and a cross-table `JOIN`
returning the correct joined row.

A #5564 PR review round found three further real gaps, each independently
verified against the same real Oracle 23ai engine before being fixed.
Character columns now declare explicit `CHAR` length semantics --
`VARCHAR2(n CHAR)`, not a bare `VARCHAR2(n)` -- since Oracle's own default
`NLS_LENGTH_SEMANTICS=BYTE` counts a plain `VARCHAR2(n)` in bytes, and this
codebase decodes legacy DBF text to UTF-8 for every dialect's own output
(directly confirmed: a 10-byte `VARCHAR2(10)` rejects 10 non-ASCII
characters that decode to 20 UTF-8 bytes, while `VARCHAR2(10 CHAR)`
accepts them regardless of session settings). A CLOB column's own literal
is no longer a plain quoted string -- directly confirmed that an empty
string literal silently becomes `NULL` for a real CLOB column, and that
Oracle's own SQL text-literal limit is 4000 *bytes* (not characters,
confirmed with both ASCII and multi-byte UTF-8 content) -- so a blank
memo now emits `EMPTY_CLOB()` and a longer one emits one-or-more
`TO_CLOB('...')` chunks concatenated with `||`, each chunk split on a
UTF-8 character boundary so a chunk can never divide a multi-byte
character. And two distinct source names that sanitize to the identical
quoted identifier after this exporter's own embedded-quote stripping
(Oracle has no escape mechanism for one at all) now fail the whole export
closed with a diagnostic naming the identifier, rather than silently
emit a script with a duplicate or wrong-target table/column. A later,
independently-found gap (#5693): Oracle treats a zero-length `VARCHAR2`
literal as `NULL` (directly confirmed: `INSERT ... VALUES ('')` leaves a
`VARCHAR2` column `NULL`, not an empty string), and unlike a blank Date
or blank Memo -- each already given a real, distinction-preserving
fallback above -- no `VARCHAR2` literal exists that preserves a
genuinely non-null empty Character/Varchar value's own non-null-ness.
This exporter now fails the whole export closed with a diagnostic naming
the table and column, rather than report a successful export whose
ordinary load silently corrupts that distinction -- except for a table
that declares any nullable field at all (identified by its own
`_NullFlags` pseudo-field), since this codebase does not yet decode that
field's own record bitmap and so cannot currently tell a genuinely null
value apart from a genuinely non-null empty one there (#5718 tracks
proper bitmap decoding as its own foundational fix).

`TYPE MYSQL` (#5554, fifth and final vendor-dialect slice of #141) is close
enough in overall shape to `TYPE SQLSERVER`'s own precedent to share its
structure (per-table index-name scoping, `1`/`0` boolean literals, blank-date-
to-NULL, plain-ISO-string date/datetime literals) but keeps its own dedicated
code path per this family's established convention. Backtick `` `identifier` ``
quoting escapes an embedded backtick by doubling it -- like every dialect
above except Oracle's own lossy stripping (directly confirmed against a real
local MySQL 8.0 engine that `` `weird``name` `` round-trips losslessly through
`CREATE TABLE`) -- `DECIMAL(19, 4)` for VFP currency, `INT` (MySQL's own
4-byte integer), `DOUBLE` for VFP's own double type, `TINYINT(1)` for logical
(directly confirmed MySQL's own `BOOLEAN` keyword is merely a synonym for
`TINYINT(1)`, so this exporter declares the underlying type directly), `DATE`/
`DATETIME` for VFP date/datetime (a plain ISO string loads directly with no
special literal wrapper needed, unlike Oracle), `VARCHAR(length)` for
character fields, and `LONGTEXT` -- not the 64 KiB-capped `TEXT` -- for memo/
general/picture fields. `DECIMAL`'s own precision is clamped to MySQL's real
65-digit ceiling and its scale to MySQL's own 30-digit ceiling *and* to that
(already-clamped) precision -- like SQL Server's own `DECIMAL`, and a genuine
difference from Oracle's own independent scale range, both directly confirmed
against the real engine (`DECIMAL(66, 0)`/`DECIMAL(10, 31)` both fail;
`DECIMAL(10, 20)`, scale exceeding precision, fails with "M must be >= D"). A
blank VFP date/datetime value resolves to `NULL` rather than an empty string
literal -- directly confirmed against the real engine that MySQL 8.0's own
default `sql_mode` (including `STRICT_TRANS_TABLES`) rejects an empty-string
insert into a `DATE`/`DATETIME` column outright with error 1292, a third,
distinct failure mode from SQL Server's own silent-1900-01-01 invention and
Oracle's own invalid-syntax rejection, but the same NULL-instead-of-empty-
string fix applies. The one genuinely MySQL-specific finding: unlike every
other dialect above (all of which share one doubled-single-quote string-
literal quoting function), a character-ish value here is quoted through a
dedicated function that doubles *both* an embedded single quote and an
embedded backslash, since MySQL's own default `sql_mode` (no
`NO_BACKSLASH_ESCAPES`) treats a lone backslash as a live escape character
inside a string literal -- directly confirmed against the real engine that
plain doubled-quote-only escaping of a Windows path silently misinterprets
`\t` as an actual TAB character rather than a literal backslash followed by
`t`, corrupting a genuinely common case for this codebase; doubling the
backslash too stores the correct literal text. No identifier-collision
tracking is needed the way Oracle's own lossy quote-stripping requires, since
an embedded backtick is losslessly escaped. And unlike Oracle's own tight
4000-byte SQL text-literal ceiling, a plain (correctly escaped) string
literal is directly confirmed safe for a `LONGTEXT` value with no special
chunking, and an empty string literal correctly stores as an empty (non-NULL)
string, unlike Oracle's own silently-NULLed CLOB. `CREATE INDEX` generation
is scoped to MySQL's own 64-*character* identifier limit (directly confirmed
the real engine *rejects*, error 1059, rather than silently truncates,
character-counted like SQL Server's own `sysname` rather than byte-counted
like PostgreSQL's/Oracle's own limits), with disambiguation scoped *per
table* -- directly confirmed against the real engine that MySQL index names
are unique only within their own table, the same as SQL Server's own
precedent. All of the above was directly confirmed against a real local
MySQL 8.0 engine during this issue's own development, including this
exporter's own actual generated output for a representative two-table
fixture -- covering every mapped column type, a memo containing an actual
Windows path with both a backslash and an embedded single quote, and a blank
date/memo on a second row -- loading with zero errors and a cross-table
`JOIN` returning the correct joined row, with the backslash-containing memo
round-tripping to its exact original content.

`TYPE ACCESS`
(#5475, phase 1 of #141) emits
the same shape of script using the Access/Jet SQL dialect instead -- square-bracket
`[identifier]` quoting (with an embedded `]` escaped by doubling, the Jet/ACE
convention, since this exporter's input is DBC/DBF catalog metadata that a crafted
or corrupt source is not bound to keep free of it), Access-native column types
(`TEXT`/`MEMO`/`LONG`/`DOUBLE`/`CURRENCY`/`DATETIME`/`YESNO`, with `DECIMAL`'s
precision/scale clamped into Access-valid ranges rather than trusting an
untrustworthy header), and `#...#`-delimited date/time literals -- grounded in
`docs/66-access-container-format-notes.md`'s finding that the *logical* Access
SQL/DDL dialect is citable public documentation, distinct from the physical
MDB/ACCDB byte format, which is not. It is explicitly a SQL-script export, not a
native `.accdb`/`.mdb` binary writer. Unlike `TYPE SQL`, it emits no `-- ...`
comment lines at all -- independently verified research found native Jet/ACE SQL
(whether run through Access's interactive SQL View or a DAO/ADO `Execute()` call)
has no supported comment syntax, so embedding one would make the generated script
fail exactly where this exporter's purpose is to succeed; a skipped/unreadable
table simply contributes nothing to the output instead of a diagnostic comment.
Each `CREATE TABLE`/`INSERT` statement is valid Jet/ACE SQL text on its own, meant
to be executed in sequence (e.g. via DAO/ADO against a real Access database)
rather than pasted as one multi-statement block into Access's own SQL View, which
only ever holds a single statement.

This is a Copperfin modernization extension authorized by the owner-approved
scope in #140/#141 (see #5471 for the `TYPE SQL` slice, #5475 for the
`TYPE ACCESS` slice, #5537 for the `TYPE POSTGRESQL` slice, and #5554 for the
`TYPE SQLITE`/`TYPE SQLSERVER`/`TYPE ORACLE`/`TYPE MYSQL` slices specifically),
not a claimed Visual FoxPro 9 command. It does
not implement `IMPORT DATABASE` of any kind, provider connections, schema
mutation, or round-trip reconstruction back into a DBC/DBF from any exported
format.

The snapshot is a versioned machine contract. Every document begins with the
integer `schema_version: 1`; consumers must reject a missing or unsupported
version rather than infer compatibility from optional catalog or table content.
Version 1 preserves the existing `database`, `catalog`, and `tables` members.
It is an export interchange baseline only: no current Copperfin component treats
it as authority to create, replace, or modify a database file.

The native `build_database_json_import_plan()` API is the bounded first import
planning step. It admits only the exact version-1 envelope, validates the
database/catalog/table/field structure within the existing JSON parser limits,
rejects case-folded table and field-name collisions and field descriptors the
ordinary DBF writer cannot admit, retains catalog and row
payloads as inert JSON, and returns a deterministic in-memory table plan. It
parses the bounded source once for those repeated selections, rejects
out-of-range field numbers without integer wraparound, and preserves the
exporter's empty `fields` marker for an unreadable cataloged table as an
inert no-schema plan rather than treating it as reconstruction authority. It
does not expose `IMPORT DATABASE` syntax, interpret the stored source path,
open an output path, invoke a provider, or create, overwrite, or mutate a DBC,
DBF, memo, or index file. Future reconstruction needs its own target-layout,
overwrite, transaction, rollback, and recovery requirements.

Minimal C++ call:

```cpp
const auto export_result = copperfin::vfp::export_database_as_json("/path/to/northwind.dbc");
if (!export_result.ok) {
		// export_result.error contains the failure reason
}
// export_result.json contains the serialized snapshot
```

Example JSON shape:

```json
{
	"schema_version": 1,
	"database": {
		"path": "/data/northwind.dbc",
		"name": "northwind"
	},
	"catalog": [
		{
			"record_index": 0,
			"object_type": "database",
			"object_name": "northwind",
			"parent_name": "",
			"properties": {
				"Caption": "Northwind"
			}
		},
		{
			"record_index": 1,
			"object_type": "table",
			"object_name": "Customers",
			"parent_name": "northwind",
			"properties": {
				"Comment": "Active customers"
			}
		}
	],
	"tables": {
		"Customers": {
			"fields": [
				{"name": "CUSTID", "type": "N", "length": 8, "decimals": 0},
				{"name": "COMPANY", "type": "C", "length": 40, "decimals": 0}
			],
			"records": [
				{"CUSTID": 1, "COMPANY": "Acme Corp"}
			]
		}
	}
}
```

Notes:

- `catalog` comes from DBC rows, including decoded binary `PROPERTIES` memo payloads.
- `tables` is populated only for local table files that can be resolved next to the DBC.
- `max_rows_per_table` can be used to cap exported row counts for large datasets.

## Key Risks

- binary asset edge cases not documented in public help
- preserving designer metadata exactly enough for round-trip confidence
- unsupported ActiveX or COM behaviors
- hidden dependencies stored in project metadata or resource records

## Recommended Validation Artifacts

- golden copies of representative PJX/SCX/VCX/FRX/MNX files
- binary diff checks after no-op round trip
- property-level round-trip tests
- execution traces for imported sample applications
- DBC JSON export fixtures that assert decoded `PROPERTIES` round-trips for representative catalog object types
