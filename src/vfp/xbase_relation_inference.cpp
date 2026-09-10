// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/xbase_relation_inference.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/index_probe.h"
#include "copperfin/vfp/sidecar_path.h"
#include "copperfin/platform/path.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <map>
#include <optional>
#include <set>

namespace copperfin::vfp {

namespace {

std::string uppercase_ascii_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::toupper(character));
    });
    return value;
}

std::string trim_copy_local(const std::string& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1U);
}

// A key expression maps to a plain column reference only when it is
// (trimmed, case-insensitively) an exact match for one of the table's own
// column names -- a composite expression (concatenation, function call,
// multiple columns) is not translatable to a single-column relation and
// is deliberately not guessed at, matching this same discipline already
// established for export_database_as_postgresql_sql()'s CREATE INDEX
// derivation (#5537, src/vfp/asset_inspector.cpp).
std::optional<std::string> plain_column_name_for_key_expression(
    const std::string& key_expression,
    const std::vector<DbfFieldDescriptor>& fields) {
    const std::string trimmed = trim_copy_local(key_expression);
    if (trimmed.empty()) {
        return std::nullopt;
    }
    const std::string upper_trimmed = uppercase_ascii_copy(trimmed);
    for (const DbfFieldDescriptor& field : fields) {
        if (uppercase_ascii_copy(field.name) == upper_trimmed) {
            return field.name;
        }
    }
    return std::nullopt;
}

// Every plain-column key this table's companion index file(s) reference,
// deduplicated. Only a same-base-name sidecar is consulted for each of
// the five documented xBase-family index extensions this codebase already
// has header probes for (index_probe.cpp), resolved case-insensitively
// (resolve_vfp_sidecar_path()) -- a table missing a given kind of
// companion, or one that fails to parse, simply contributes nothing for
// that kind rather than failing the whole scan.
std::set<std::string> indexed_plain_columns_for_table(
    const std::filesystem::path& dbf_path,
    const std::vector<DbfFieldDescriptor>& fields) {
    std::set<std::string> columns;
    static constexpr std::array<const char*, 5U> kIndexExtensions{
        ".cdx", ".idx", ".ndx", ".mdx", ".ntx"};
    for (const char* extension : kIndexExtensions) {
        const SidecarPathResolution resolution = resolve_vfp_sidecar_path(dbf_path, extension);
        if (!resolution.path.has_value()) {
            continue;
        }
        const IndexParseResult index_result =
            parse_index_probe_from_file(copperfin::platform::path_to_utf8_string(*resolution.path));
        if (!index_result.ok) {
            continue;
        }
        if (!index_result.probe.tags.empty()) {
            // A compound index (CDX/MDX): every tag's own key expression.
            for (const IndexTagProbe& tag : index_result.probe.tags) {
                if (const auto column = plain_column_name_for_key_expression(tag.key_expression_hint, fields);
                    column.has_value()) {
                    columns.insert(uppercase_ascii_copy(*column));
                }
            }
        } else if (const auto column =
                       plain_column_name_for_key_expression(index_result.probe.key_expression_hint, fields);
                   column.has_value()) {
            // A single-key index (IDX/NDX/NTX): one key expression at the
            // probe level, not per-tag.
            columns.insert(uppercase_ascii_copy(*column));
        }
    }
    return columns;
}

}  // namespace

std::vector<InferredTableRelation> infer_xbase_index_relations(
    const std::vector<XbaseImportedTable>& tables) {
    // column_name (uppercased) -> every table name observed indexing it.
    std::map<std::string, std::vector<std::string>> tables_by_column;

    for (const XbaseImportedTable& table : tables) {
        const DbfTableParseResult parsed = parse_dbf_table_from_file(table.dbf_path, 0U);
        if (!parsed.ok) {
            continue;
        }
        const std::set<std::string> columns = indexed_plain_columns_for_table(
            copperfin::platform::path_from_utf8_string(table.dbf_path), parsed.table.fields);
        for (const std::string& column : columns) {
            tables_by_column[column].push_back(table.name);
        }
    }

    std::vector<InferredTableRelation> relations;
    for (const auto& [column, table_names] : tables_by_column) {
        for (std::size_t i = 0U; i < table_names.size(); ++i) {
            for (std::size_t j = i + 1U; j < table_names.size(); ++j) {
                if (table_names[i] == table_names[j]) {
                    // The same table indexing the same column more than
                    // once (e.g. two tags, ascending and descending, over
                    // the same field) is not a cross-table relation.
                    continue;
                }
                relations.push_back({.table_a = table_names[i], .table_b = table_names[j], .column = column});
            }
        }
    }
    return relations;
}

}  // namespace copperfin::vfp
