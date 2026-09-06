// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/index_probe.h"

#include <cstddef>
#include <map>
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
};

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
