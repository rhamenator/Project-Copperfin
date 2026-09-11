// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/index_probe.h"

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::vfp {

enum class AssetFamily {
    unknown,
    project,
    form,
    class_library,
    report,
    label,
    menu,
    index,
    table,
    database_container,
    program,
    header
};

enum class AssetValidationSeverity {
    warning,
    error
};

struct AssetValidationIssue {
    AssetValidationSeverity severity = AssetValidationSeverity::warning;
    std::string code;
    std::string path;
    std::string message;
};

struct DatabaseContainerObjectPreview {
    std::size_t record_index = 0;
    bool deleted = false;
    std::string object_type_hint;
    std::string object_name_hint;
    std::string parent_name_hint;
};

struct DatabaseContainerMetadata {
    bool available = false;
    std::size_t total_objects = 0;
    std::size_t database_objects = 0;
    std::size_t table_objects = 0;
    std::size_t view_objects = 0;
    std::size_t relation_objects = 0;
    std::size_t connection_objects = 0;
    std::vector<DatabaseContainerObjectPreview> objects_preview;
};

struct AssetInspectionResult {
    struct IndexAsset {
        std::string path;
        IndexProbe probe{};
    };

    bool ok = false;
    std::string path;
    AssetFamily family = AssetFamily::unknown;
    bool header_available = false;
    DbfHeader header{};
    std::vector<IndexAsset> indexes;
    bool database_container_metadata_available = false;
    DatabaseContainerMetadata database_container_metadata{};
    std::vector<AssetValidationIssue> validation_issues;
    std::string error;

    [[nodiscard]] bool has_validation_issues() const {
        return !validation_issues.empty();
    }
};

// Optional immutable bytes keyed by the exact UTF-8 path being inspected.
// Runtime security paths use this to prevent a parser from reopening a path
// after the package verifier has admitted its contents.
using AssetByteOverrides = std::map<std::string, std::string>;

[[nodiscard]] AssetFamily asset_family_from_path(const std::string& path);
[[nodiscard]] const char* asset_family_name(AssetFamily family);
[[nodiscard]] const char* asset_validation_severity_name(AssetValidationSeverity severity);
AssetInspectionResult inspect_asset(
    const std::string& path,
    const std::string& memo_sidecar_path = {},
    const AssetByteOverrides* byte_overrides = nullptr);

// ---- Whole-database JSON export ----

// One decoded property from a DBC PROPERTIES memo blob.
// The value is always represented as a string; the type_hint records the
// original VFP storage type ('C', 'N', 'L', 'D', 'I') for consumers that
// need to round-trip the value back to binary form.
struct DbcProperty {
    std::string name;
    char type_hint = 'C';  // VFP type code: C N L D I …
    std::string value;
};

// One row from a DBC catalog (OBJECTTYPE / OBJECTNAME / PARENTNAME / PROPERTIES).
struct DbcCatalogObject {
    std::size_t record_index = 0;
    bool deleted = false;
    std::string object_type;   // normalised lowercase: "database", "table", "field", …
    std::string object_name;
    std::string parent_name;
    std::vector<DbcProperty> properties;  // decoded from the binary PROPERTIES memo
    // #5538: this row's CODE memo field, decoded through the DBC's own
    // code page (matching decode_dbf_text()'s established convention --
    // see extract_dbc_stored_procedures_source()'s own comment for the
    // documented row this is meaningful for). nullopt when the row has
    // no CODE memo content or the DBC has no field named CODE at all.
    std::optional<std::string> code_source;
};

// #5538 (parent #137, related #5471/#5537/#113): read-only extraction of a
// DBC's Stored Procedures source code. Real Visual FoxPro stores this in a
// dedicated catalog row named "StoredProceduresSource" whose CODE memo
// field holds the raw PRG source text (a sibling row,
// "StoredProceduresObject", holds compiled p-code, which this function
// deliberately does not attempt to decode as text -- see this struct's own
// comment). Grounded in Microsoft's own archived Visual FoxPro Knowledge
// Base (Q180028), which demonstrates opening a .dbc as a table and
// directly reading/writing that row's CODE memo -- not merely community
// speculation. See docs/73-vfp-dbc-stored-procedures-and-view-sql.md for
// the full evidence trail, including what this slice does NOT yet cover
// (SQL view definition text -- deferred pending real-fixture
// verification, see that document's own reasoning).
struct DbcStoredProceduresResult {
    bool ok = false;
    std::string error;
    // false when the DBC has no "StoredProceduresSource" catalog row (a
    // database with no stored procedures at all), OR when that row's
    // CODE memo content could not be resolved (e.g. a missing/unreadable
    // .dct memo sidecar) -- neither case is itself an error; source_code
    // stays empty either way.
    bool available = false;
    std::string source_code;  // raw PRG text, exactly as stored; never executed or parsed
};

// Scans dbc_path's catalog for a "StoredProceduresSource" row (case-
// insensitive, matching this codebase's existing catalog-field-name
// tolerance) and returns its CODE memo content verbatim. No execution, no
// interpretation of the PRG source -- read-only extraction only, per this
// issue's explicit non-goal.
[[nodiscard]] DbcStoredProceduresResult extract_dbc_stored_procedures_source(const std::string& dbc_path);

// Result of export_database_as_json.
struct DatabaseExportResult {
    bool ok = false;
    std::string error;
    std::string json;  // the full JSON document when ok == true
};

// Produces a single JSON document that captures:
//   • the DBC catalog (all non-deleted records with decoded PROPERTIES)
//   • the data rows of every TABLE object referenced in the catalog
// Tables are resolved relative to the directory of dbc_path.
// max_rows_per_table caps how many rows are exported per table (0 = no cap).
[[nodiscard]] DatabaseExportResult export_database_as_json(
    const std::string& dbc_path,
    std::size_t max_rows_per_table = 0U);

// Result of export_database_as_sql.
struct DatabaseSqlExportResult {
    bool ok = false;
    std::string error;
    std::string sql;  // the full SQL script when ok == true
};

// Produces a single portable/ANSI-ish SQL script (CREATE TABLE per table,
// followed by INSERT statements for its rows) from the same DBC catalog and
// table-resolution path export_database_as_json() uses. Tables are resolved
// relative to the directory of dbc_path. max_rows_per_table caps how many
// rows are exported per table (0 = no cap). No live database connection is
// involved -- this is static file-to-file export, distinct from the
// database-federation lane (see docs/21-database-federation-and-query-translation.md).
[[nodiscard]] DatabaseSqlExportResult export_database_as_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table = 0U);

// #5475 (I-migration/#141 phase 1): produces the same shape of script as
// export_database_as_sql(), but using the Access/Jet SQL dialect --
// square-bracket `[identifier]` quoting, Access-native column types
// (TEXT/MEMO/LONG/DOUBLE/CURRENCY/DATETIME/YESNO), and `#...#`-delimited
// date/time literals -- so the output can be run directly against a real
// Access database (e.g. pasted into Access's SQL View, or fed to any tool
// that accepts Jet/ACE SQL text) rather than needing hand-translation from
// the ANSI-ish dialect export_database_as_sql() emits. Grounded in
// docs/66-access-container-format-notes.md's finding that the *logical*
// Access SQL/DDL dialect is citable public documentation even though the
// physical MDB/ACCDB byte format is not -- this is a phase-1 SQL-script
// export, not a native .accdb/.mdb binary writer (see #5475's own explicit
// non-goal). Tables are resolved the same way export_database_as_sql()
// resolves them; max_rows_per_table has the same meaning.
[[nodiscard]] DatabaseSqlExportResult export_database_as_access_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table = 0U);

// #5537 (parent #137, first vendor-dialect slice -- PostgreSQL): produces
// the same CREATE TABLE/INSERT shape as export_database_as_sql() -- real
// PostgreSQL already accepts that exporter's double-quoted identifiers,
// single-quoted string literals, and DECIMAL/INTEGER/DOUBLE PRECISION/
// BOOLEAN/DATE/TIMESTAMP/VARCHAR/TEXT column types verbatim, per
// PostgreSQL's own public SQL/DDL reference documentation, so this
// exporter is its own dedicated code path (matching "one TYPE <VENDOR>
// variant per target engine," #5537's own scope idea) rather than an
// alias, so a future vendor-specific type/quoting divergence has
// somewhere to go without touching the portable baseline. What this
// exporter adds beyond export_database_as_sql(): CREATE INDEX statements
// derived from each table's production CDX index tags
// (src/vfp/index_probe.cpp's existing header-probe reader), for a tag
// whose key expression is (trimmed, case-insensitively) exactly one of
// the table's own column names -- a composite/expression key (e.g. a
// concatenation or function call) does not map cleanly to a single-
// column CREATE INDEX and is recorded as a skipped-index comment instead
// of guessed at, per #5537's own explicit scope note. No per-tag
// uniqueness is currently captured by index_probe.cpp's CDX tag reader,
// so every emitted index is a plain (non-unique) CREATE INDEX -- never
// asserting UNIQUE without a captured signal for it. Tables are resolved
// the same way export_database_as_sql() resolves them; max_rows_per_table
// has the same meaning.
[[nodiscard]] DatabaseSqlExportResult export_database_as_postgresql_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table = 0U);

// #5554 (parent #137, second vendor-dialect slice -- SQLite, following
// #5537/PR #5545's PostgreSQL precedent): produces the same CREATE
// TABLE/INSERT shape as export_database_as_sql() -- real SQLite already
// accepts that exporter's double-quoted identifiers, single-quoted
// string literals, and DECIMAL/INTEGER/DOUBLE PRECISION/BOOLEAN/DATE/
// TIMESTAMP/VARCHAR/TEXT column types verbatim. Unlike PostgreSQL (whose
// public SQL/DDL reference documents these types directly), SQLite is
// dynamically typed per value: any column type declaration syntactically
// parses, and its own "type affinity" rules (SQLite's public "Datatypes
// In SQLite" documentation) bucket every one of these exact type names
// into a definite affinity by substring match (VARCHAR/TEXT -> TEXT via
// the "CHAR"/"TEXT" substring rule; DECIMAL/BOOLEAN/DATE/TIMESTAMP ->
// NUMERIC, the default bucket for a name matching none of SQLite's
// INTEGER/TEXT/BLOB/REAL substring rules) rather than being rejected as
// unrecognized -- directly confirmed against a real local `sqlite3`
// engine (3.46.1): this exporter's actual output for a representative
// two-table DBC fixture (customers/orders, a numeric key, a VARCHAR
// with an embedded quote, DECIMAL/BOOLEAN/DATE columns; that fixture
// had no companion `.cdx`, so this check did not exercise `CREATE
// INDEX` specifically) loaded into a real SQLite database with zero
// errors, round-tripped every row exactly (including the escaped quote
// and the TRUE/FALSE -> 1/0 boolean mapping), and a cross-table `JOIN`
// between the two exported tables returned the correct joined row --
// not just syntax acceptance, but genuine relational query correctness
// against the real target engine. `CREATE INDEX`'s own syntax was
// separately confirmed to load without error against real `sqlite3`
// during this same investigation (a standalone single-table check, see
// docs/32's own traceability row for this issue).
// What this exporter adds beyond export_database_as_sql(): the same
// CREATE INDEX generation export_database_as_postgresql_sql() already
// implements (SQLite's own CREATE INDEX syntax is identical to
// PostgreSQL's for this exporter's plain-column-reference case) -- see
// that function's own comment for the full index-generation scope and
// its documented non-goals (composite/expression keys skipped as a
// comment, no per-tag uniqueness captured). Tables are resolved the
// same way export_database_as_sql() resolves them; max_rows_per_table
// has the same meaning.
[[nodiscard]] DatabaseSqlExportResult export_database_as_sqlite_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table = 0U);

// ---- Whole-database JSON import planning ----

// A validated, in-memory description of a version-1 export snapshot. The
// catalog and record payloads remain JSON because this planning boundary has no
// authority to create, replace, or modify database files.
struct DatabaseJsonImportTablePlan {
    std::string name;
    std::vector<DbfFieldDescriptor> fields;
    std::string records_json;
};

struct DatabaseJsonImportPlan {
    std::string database_name;
    std::string catalog_json;
    std::vector<DatabaseJsonImportTablePlan> tables;
};

struct DatabaseJsonImportPlanResult {
    bool ok = false;
    std::string error_code;
    DatabaseJsonImportPlan plan;
};

// Validates and models the schema-version-1 export envelope in memory. The
// source document is bounded and never interpreted as a path, command, or
// provider connection. This is a planning primitive only; it does not expose
// IMPORT DATABASE syntax or perform database reconstruction.
[[nodiscard]] DatabaseJsonImportPlanResult build_database_json_import_plan(
    std::string_view document);

// Parses the narrow, fixed SQL dialect export_database_as_sql() itself
// produces (its required three-line header comment, then CREATE TABLE and
// INSERT INTO statements using exactly that function's identifier/literal
// quoting and column-type vocabulary) into the same DatabaseJsonImportPlan
// materialize_database_json_import_plan() already consumes -- a parser/
// adapter in front of the existing materializer, not a second write path.
// This is not a general-purpose SQL parser: anything outside that exact
// subset is rejected with a distinct error_code rather than guessed at.
// Some field-type precision is intentionally lost on the round trip (N, F,
// and Y all become 'N'; M, G, and P all become 'M'), matching the same
// narrowing export_database_as_sql() already performs on the way out.
[[nodiscard]] DatabaseJsonImportPlanResult build_database_sql_import_plan(
    std::string_view document);

// Result of materialize_database_json_import_plan.
struct DatabaseJsonImportResult {
    bool ok = false;
    std::string error;
    std::size_t table_count = 0;
};

// Materializes an already-validated import plan into a new DBC catalog and
// one DBF file per table, at dbc_path and <dbc_dir>/<table_name>.dbf
// respectively. Fails closed without writing anything if dbc_path or any
// derived table path already exists. All files are staged in a temporary
// directory beside dbc_path and verified there first; only once every file
// has been staged successfully are they committed into place one at a time
// (tables before the catalog, so a reader never observes a catalog
// referencing a not-yet-existing table). Any failure during staging or
// commit removes every already-committed file and all temporary artifacts,
// leaving nothing behind at the destination -- this is the first command in
// this family that mutates database files on disk (HZ-data-corruption-01).
// No index (CDX/IDX) or relation/container-metadata reconstruction is
// performed; only table structure and row data.
[[nodiscard]] DatabaseJsonImportResult materialize_database_json_import_plan(
    const DatabaseJsonImportPlan& plan,
    const std::string& dbc_path);

}  // namespace copperfin::vfp
