#pragma once

#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/index_probe.h"

#include <map>
#include <string>
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

[[nodiscard]] AssetFamily asset_family_from_path(const std::string& path);
[[nodiscard]] const char* asset_family_name(AssetFamily family);
[[nodiscard]] const char* asset_validation_severity_name(AssetValidationSeverity severity);
AssetInspectionResult inspect_asset(const std::string& path);

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

}  // namespace copperfin::vfp
