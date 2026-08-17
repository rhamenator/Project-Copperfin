// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/migration/project_inventory.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string_view>
#include <system_error>
#include <utility>

namespace copperfin::migration {
namespace {

std::string normalized_extension(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char value) {
        return static_cast<char>(std::tolower(value));
    });
    return extension;
}

std::string classify_asset(const std::filesystem::path& path) {
    const std::string extension = normalized_extension(path);
    for (const auto& [candidate, kind] : std::array<std::pair<std::string_view, std::string_view>, 13U>{{
             {".prg", "prg"}, {".pjx", "project"}, {".scx", "form"},
             {".vcx", "class_library"}, {".frx", "report"}, {".lbx", "label"},
             {".mnx", "menu"}, {".dbf", "table"}, {".fpt", "memo"},
             {".cdx", "compound_index"}, {".idx", "index"}, {".dbc", "database_container"},
             {".app", "application"}}}) {
        if (extension == candidate) {
            return std::string(kind);
        }
    }
    return "other";
}

std::string portable_relative_path(const std::filesystem::path& path) {
    return path.generic_string();
}

bool is_valid_utf8(const std::string_view value) {
    for (std::size_t index = 0U; index < value.size();) {
        const auto lead = static_cast<unsigned char>(value[index]);
        if (lead <= 0x7fU) {
            ++index;
            continue;
        }

        std::size_t continuation_count = 0U;
        unsigned char minimum_second = 0x80U;
        unsigned char maximum_second = 0xbfU;
        if (lead >= 0xc2U && lead <= 0xdfU) {
            continuation_count = 1U;
        } else if (lead == 0xe0U) {
            continuation_count = 2U;
            minimum_second = 0xa0U;
        } else if (lead == 0xedU) {
            continuation_count = 2U;
            maximum_second = 0x9fU;
        } else if (lead >= 0xe1U && lead <= 0xefU) {
            continuation_count = 2U;
        } else if (lead == 0xf0U) {
            continuation_count = 3U;
            minimum_second = 0x90U;
        } else if (lead == 0xf4U) {
            continuation_count = 3U;
            maximum_second = 0x8fU;
        } else if (lead >= 0xf1U && lead <= 0xf3U) {
            continuation_count = 3U;
        } else {
            return false;
        }
        if (index + continuation_count >= value.size()) {
            return false;
        }
        const auto second = static_cast<unsigned char>(value[index + 1U]);
        if (second < minimum_second || second > maximum_second) {
            return false;
        }
        for (std::size_t offset = 2U; offset <= continuation_count; ++offset) {
            const auto continuation = static_cast<unsigned char>(value[index + offset]);
            if (continuation < 0x80U || continuation > 0xbfU) {
                return false;
            }
        }
        index += continuation_count + 1U;
    }
    return true;
}

void append_json_string(std::string& output, const std::string_view value) {
    output.push_back('"');
    for (const unsigned char character : value) {
        switch (character) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                if (character < 0x20U) {
                    constexpr char hexadecimal[] = "0123456789abcdef";
                    output += "\\u00";
                    output.push_back(hexadecimal[(character >> 4U) & 0x0fU]);
                    output.push_back(hexadecimal[character & 0x0fU]);
                } else {
                    output.push_back(static_cast<char>(character));
                }
        }
    }
    output.push_back('"');
}

}  // namespace

ProjectInventoryResult build_project_inventory(
    const std::filesystem::path& trusted_absolute_project_root) {
    ProjectInventoryResult result;
    if (trusted_absolute_project_root.empty() || !trusted_absolute_project_root.is_absolute()) {
        result.diagnostic_code = "migration.inventory.invalid_root";
        return result;
    }

    std::error_code error;
    const auto root_status = std::filesystem::symlink_status(trusted_absolute_project_root, error);
    if (error || std::filesystem::is_symlink(root_status) || !std::filesystem::is_directory(root_status)) {
        result.diagnostic_code = "migration.inventory.root_unavailable";
        return result;
    }
    const std::filesystem::path canonical_root = std::filesystem::canonical(trusted_absolute_project_root, error);
    if (error) {
        result.diagnostic_code = "migration.inventory.root_unavailable";
        return result;
    }

    bool complete = true;
    std::filesystem::recursive_directory_iterator iterator(
        canonical_root, std::filesystem::directory_options::skip_permission_denied, error);
    if (error) {
        result.diagnostic_code = "migration.inventory.scan_incomplete";
        return result;
    }
    const std::filesystem::recursive_directory_iterator end;
    while (iterator != end) {
        const std::filesystem::directory_entry entry = *iterator;
        const auto relative = entry.path().lexically_relative(canonical_root);
        const std::string relative_path = portable_relative_path(relative);
        if (!is_valid_utf8(relative_path)) {
            complete = false;
            iterator.increment(error);
            if (error) {
                error.clear();
            }
            continue;
        }
        const auto status = entry.symlink_status(error);
        if (error) {
            complete = false;
            error.clear();
            iterator.increment(error);
            if (error) {
                complete = false;
                error.clear();
            }
            continue;
        }
        if (std::filesystem::is_symlink(status)) {
            result.skipped_symlinks.push_back(relative_path);
            iterator.disable_recursion_pending();
        } else if (std::filesystem::is_regular_file(status)) {
            const std::uintmax_t size = entry.file_size(error);
            if (error) {
                complete = false;
                error.clear();
            } else {
                result.entries.push_back({relative_path, classify_asset(entry.path()), size});
            }
        }
        iterator.increment(error);
        if (error) {
            complete = false;
            error.clear();
        }
    }

    std::sort(result.entries.begin(), result.entries.end(), [](const auto& left, const auto& right) {
        return left.relative_path < right.relative_path;
    });
    std::sort(result.skipped_symlinks.begin(), result.skipped_symlinks.end());
    result.complete = complete;
    result.diagnostic_code = complete ? "migration.inventory.complete" : "migration.inventory.scan_incomplete";
    return result;
}

std::string serialize_project_inventory_json(const ProjectInventoryResult& inventory) {
    std::string output{"{\n  \"schemaVersion\": 1,\n  \"complete\": "};
    output += inventory.complete ? "true" : "false";
    output += ",\n  \"diagnosticCode\": ";
    append_json_string(output, inventory.diagnostic_code);
    output += ",\n  \"entries\": [";
    for (std::size_t index = 0U; index < inventory.entries.size(); ++index) {
        const auto& entry = inventory.entries[index];
        output += index == 0U ? "\n    {\"path\": " : ",\n    {\"path\": ";
        append_json_string(output, entry.relative_path);
        output += ", \"kind\": ";
        append_json_string(output, entry.asset_kind);
        output += ", \"sizeBytes\": " + std::to_string(entry.size_bytes) + "}";
    }
    output += inventory.entries.empty() ? "],\n  \"skippedSymlinks\": [" : "\n  ],\n  \"skippedSymlinks\": [";
    for (std::size_t index = 0U; index < inventory.skipped_symlinks.size(); ++index) {
        output += index == 0U ? "\n    " : ",\n    ";
        append_json_string(output, inventory.skipped_symlinks[index]);
    }
    output += inventory.skipped_symlinks.empty() ? "]\n}\n" : "\n  ]\n}\n";
    return output;
}

}  // namespace copperfin::migration
