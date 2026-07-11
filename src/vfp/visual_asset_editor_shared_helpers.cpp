// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "visual_asset_editor_support.h"

#include "copperfin/platform/environment.h"

namespace copperfin::vfp {
namespace {

struct VisualAssetTransactionFile {
    std::filesystem::path target;
    std::filesystem::path staged;
    std::filesystem::path backup;
    const std::vector<std::uint8_t>* bytes = nullptr;
};

VisualAssetTransactionFile transaction_file(
    const std::string& path,
    const std::vector<std::uint8_t>* bytes = nullptr) {
    const std::filesystem::path target(path);
    return {
        .target = target,
        .staged = target.string() + ".cptmp",
        .backup = target.string() + ".cpbak",
        .bytes = bytes
    };
}

bool transaction_failure_requested(
    const std::string& table_path,
    const std::string& memo_path,
    std::string_view stage) {
    const auto marker =
        copperfin::platform::read_environment_variable("COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS");
    const auto requested_stage =
        copperfin::platform::read_environment_variable("COPPERFIN_TEST_FAIL_WRITE_STAGE");
    return marker.has_value() && requested_stage.has_value() &&
           !marker->empty() && *requested_stage == stage &&
           (table_path.find(*marker) != std::string::npos ||
            memo_path.find(*marker) != std::string::npos);
}

bool remove_transaction_path(const std::filesystem::path& path) {
    std::error_code error;
    if (!std::filesystem::exists(path, error)) {
        return !error;
    }
    return std::filesystem::remove(path, error) && !error;
}

bool rename_transaction_path(
    const std::filesystem::path& source,
    const std::filesystem::path& destination) {
    std::error_code error;
    std::filesystem::rename(source, destination, error);
    return !error;
}

bool stage_transaction_file(const VisualAssetTransactionFile& file) {
    std::ofstream output(file.staged, std::ios::binary | std::ios::trunc);
    if (!output || file.bytes == nullptr) {
        return false;
    }
    output.write(
        reinterpret_cast<const char*>(file.bytes->data()),
        static_cast<std::streamsize>(file.bytes->size()));
    output.flush();
    if (!output) {
        output.close();
        remove_transaction_path(file.staged);
        return false;
    }
    output.close();
    if (!output) {
        remove_transaction_path(file.staged);
        return false;
    }
    return true;
}

bool restore_transaction_file(const VisualAssetTransactionFile& file) {
    std::error_code error;
    if (std::filesystem::exists(file.backup, error)) {
        if (error) {
            return false;
        }
        if (std::filesystem::exists(file.target, error) &&
            (!remove_transaction_path(file.target) || error)) {
            return false;
        }
        if (!rename_transaction_path(file.backup, file.target)) {
            return false;
        }
    }
    return remove_transaction_path(file.staged);
}

bool rollback_visual_asset_transaction(
    const VisualAssetTransactionFile& table,
    const VisualAssetTransactionFile& memo,
    const std::filesystem::path& commit_marker,
    bool inject_failure) {
    const bool memo_restored = restore_transaction_file(memo);
    const bool table_restored = restore_transaction_file(table);
    const bool marker_removed = remove_transaction_path(commit_marker);
    return memo_restored && table_restored && marker_removed && !inject_failure;
}

std::string transaction_rollback_error() {
    return visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed") + " " +
           visual_asset_text("VisualAssetEditor.Storage.MemoSidecarWriteFailed");
}

VisualAssetEditResult transaction_failure(
    std::string error,
    const VisualAssetTransactionFile& table,
    const VisualAssetTransactionFile& memo,
    const std::filesystem::path& commit_marker,
    bool inject_rollback_failure = false) {
    if (!rollback_visual_asset_transaction(
            table,
            memo,
            commit_marker,
            inject_rollback_failure)) {
        return {
            .ok = false,
            .error = visual_asset_rollback_failed_text(
                std::move(error),
                transaction_rollback_error())
        };
    }
    return {.ok = false, .error = std::move(error)};
}

}  // namespace

const copperfin::localization::LocalizedCatalog& visual_asset_editor_catalog() {
    static const copperfin::localization::LocalizedCatalog catalog =
        copperfin::localization::load_catalogs(
            copperfin::localization::resolve_catalog_root(),
            copperfin::localization::select_locale());
    return catalog;
}

std::string visual_asset_text(std::string_view key) {
    return visual_asset_editor_catalog().translate(key);
}

std::string visual_asset_text(
    std::string_view key,
    const copperfin::localization::PlaceholderMap& placeholders) {
    return visual_asset_editor_catalog().translate(key, placeholders);
}

std::string visual_asset_property_non_negative_text(std::string property_name) {
    return visual_asset_text(
        "VisualAssetEditor.Property.NonNegativeRequired",
        {{"propertyName", std::move(property_name)}});
}

std::string visual_asset_rollback_failed_text(std::string error, std::string rollback_error) {
    return visual_asset_text(
        "VisualAssetEditor.Operation.RollbackFailed",
        {{"error", std::move(error)}, {"rollbackError", std::move(rollback_error)}});
}

std::string visual_asset_target_rollback_failed_text(std::string error, std::string rollback_error) {
    return visual_asset_text(
        "VisualAssetEditor.Operation.TargetRollbackFailed",
        {{"error", std::move(error)}, {"rollbackError", std::move(rollback_error)}});
}

std::uint32_t read_le_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8U) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24U);
}

std::uint32_t read_be_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 8U) |
           static_cast<std::uint32_t>(bytes[offset + 3]);
}

std::uint16_t read_be_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[offset]) << 8U) |
                                      static_cast<std::uint16_t>(bytes[offset + 1]));
}

void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value & 0xFFU);
}

std::string trim_right(std::string text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.pop_back();
    }
    return text;
}

std::string trim_both(std::string text) {
    text = trim_right(std::move(text));
    const auto first = std::find_if(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
    text.erase(text.begin(), first);
    return text;
}

std::string read_ascii_name(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::size_t length) {
    std::string value;
    value.reserve(length);
    for (std::size_t index = 0; index < length && (offset + index) < bytes.size(); ++index) {
        const auto raw = bytes[offset + index];
        if (raw == 0U) {
            break;
        }
        value.push_back(static_cast<char>(raw));
    }
    return trim_right(std::move(value));
}

std::vector<std::uint8_t> read_binary_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }

    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

bool write_binary_file(const std::string& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }

    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(output);
}

VisualAssetEditResult recover_visual_asset_file_transaction(
    const std::string& table_path,
    const std::string& memo_path) {
    const auto table = transaction_file(table_path);
    const auto memo = transaction_file(memo_path);
    const std::filesystem::path commit_marker = table.target.string() + ".cpcommit";

    if (transaction_failure_requested(table_path, memo_path, "stale-cleanup")) {
        return {
            .ok = false,
            .error = visual_asset_rollback_failed_text(
                visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed"),
                transaction_rollback_error())
        };
    }

    const std::vector<std::uint8_t> marker_bytes = read_binary_file(commit_marker.string());
    const std::string marker_text(marker_bytes.begin(), marker_bytes.end());
    const bool commit_completed = marker_text == "committed\n";

    if (commit_completed) {
        std::error_code error;
        const bool targets_exist =
            std::filesystem::is_regular_file(table.target, error) && !error &&
            std::filesystem::is_regular_file(memo.target, error) && !error;
        if (targets_exist) {
            const bool table_stage_cleaned = remove_transaction_path(table.staged);
            const bool memo_stage_cleaned = remove_transaction_path(memo.staged);
            const bool table_backup_cleaned = remove_transaction_path(table.backup);
            const bool memo_backup_cleaned = remove_transaction_path(memo.backup);
            const bool marker_cleaned =
                table_stage_cleaned && memo_stage_cleaned &&
                table_backup_cleaned && memo_backup_cleaned &&
                remove_transaction_path(commit_marker);
            const bool cleaned =
                table_stage_cleaned && memo_stage_cleaned &&
                table_backup_cleaned && memo_backup_cleaned && marker_cleaned;
            if (cleaned) {
                return {.ok = true, .error = {}};
            }
            return {
                .ok = false,
                .error = visual_asset_rollback_failed_text(
                    visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed"),
                    transaction_rollback_error())
            };
        }
    }

    if (!rollback_visual_asset_transaction(table, memo, commit_marker, false)) {
        return {
            .ok = false,
            .error = visual_asset_rollback_failed_text(
                visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed"),
                transaction_rollback_error())
        };
    }
    return {.ok = true, .error = {}};
}

VisualAssetEditResult write_visual_asset_file_transaction(
    const std::string& table_path,
    const std::vector<std::uint8_t>& table_bytes,
    const std::string& memo_path,
    const std::vector<std::uint8_t>& memo_bytes) {
    const auto table = transaction_file(table_path, &table_bytes);
    const auto memo = transaction_file(memo_path, &memo_bytes);
    const std::filesystem::path commit_marker = table.target.string() + ".cpcommit";

    if (transaction_failure_requested(table_path, memo_path, "sidecar-stage") ||
        !stage_transaction_file(memo)) {
        return transaction_failure(
            visual_asset_text("VisualAssetEditor.Storage.MemoSidecarWriteFailed"),
            table,
            memo,
            commit_marker);
    }
    if (transaction_failure_requested(table_path, memo_path, "primary-stage") ||
        !stage_transaction_file(table)) {
        return transaction_failure(
            visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed"),
            table,
            memo,
            commit_marker);
    }

    if (!rename_transaction_path(table.target, table.backup)) {
        return transaction_failure(
            visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed"),
            table,
            memo,
            commit_marker);
    }
    if (!rename_transaction_path(memo.target, memo.backup)) {
        return transaction_failure(
            visual_asset_text("VisualAssetEditor.Storage.MemoSidecarWriteFailed"),
            table,
            memo,
            commit_marker);
    }

    if (transaction_failure_requested(table_path, memo_path, "first-commit") ||
        !rename_transaction_path(table.staged, table.target)) {
        return transaction_failure(
            visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed"),
            table,
            memo,
            commit_marker);
    }

    const bool inject_rollback_failure =
        transaction_failure_requested(table_path, memo_path, "rollback");
    if (transaction_failure_requested(table_path, memo_path, "second-commit") ||
        inject_rollback_failure ||
        !rename_transaction_path(memo.staged, memo.target)) {
        return transaction_failure(
            visual_asset_text("VisualAssetEditor.Storage.MemoSidecarWriteFailed"),
            table,
            memo,
            commit_marker,
            inject_rollback_failure);
    }

    {
        std::ofstream marker(commit_marker, std::ios::binary | std::ios::trunc);
        marker << "committed\n";
        marker.flush();
        marker.close();
        if (!marker) {
            return transaction_failure(
                visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed"),
                table,
                memo,
                commit_marker);
        }
    }

    const bool table_backup_cleaned = remove_transaction_path(table.backup);
    const bool memo_backup_cleaned = remove_transaction_path(memo.backup);
    if (table_backup_cleaned && memo_backup_cleaned) {
        (void)remove_transaction_path(commit_marker);
    }

    return {.ok = true, .error = {}};
}

std::string infer_memo_sidecar_path(const std::string& path) {
    std::filesystem::path file_path(path);
    std::string ext = file_path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    auto lowercase_copy = [](std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    };

    auto resolve_existing_path_casefold = [&](const std::filesystem::path& candidate) -> std::optional<std::filesystem::path> {
        std::error_code ignored;
        if (std::filesystem::exists(candidate, ignored)) {
            return candidate;
        }

        const std::filesystem::path directory =
            candidate.has_parent_path() ? candidate.parent_path() : std::filesystem::current_path(ignored);
        if (directory.empty() || !std::filesystem::exists(directory, ignored)) {
            return std::nullopt;
        }

        const std::string target_name = lowercase_copy(candidate.filename().string());
        for (const auto& entry : std::filesystem::directory_iterator(directory, ignored)) {
            if (ignored) {
                break;
            }

            if (lowercase_copy(entry.path().filename().string()) == target_name) {
                return entry.path();
            }
        }

        return std::nullopt;
    };

    auto resolve_sidecar = [&](std::string_view extension) {
        const auto candidate = file_path.replace_extension(extension).string();
        if (const auto resolved = resolve_existing_path_casefold(candidate); resolved.has_value()) {
            return resolved->string();
        }

        return candidate;
    };

    if (ext == ".scx") {
        return resolve_sidecar(".sct");
    }
    if (ext == ".vcx") {
        return resolve_sidecar(".vct");
    }
    if (ext == ".frx") {
        return resolve_sidecar(".frt");
    }
    if (ext == ".lbx") {
        return resolve_sidecar(".lbt");
    }
    if (ext == ".mnx") {
        return resolve_sidecar(".mnt");
    }
    if (ext == ".pjx") {
        return resolve_sidecar(".pjt");
    }
    if (ext == ".dbc") {
        return resolve_sidecar(".dct");
    }
    return {};
}

std::string normalize_visual_object_name(std::string value) {
    value = trim_both(std::move(value));
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string format_visual_string_property_value(const std::string& value) {
    std::string formatted;
    formatted.reserve(value.size() + 2U);
    formatted.push_back('"');
    for (const char ch : value) {
        if (ch == '"') {
            formatted.push_back('"');
        }
        formatted.push_back(ch);
    }
    formatted.push_back('"');
    return formatted;
}

std::string normalize_visual_property_name(std::string value) {
    value = trim_both(std::move(value));
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string lowercase_copy(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (const char ch : value) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    return result;
}

bool contains_case_insensitive(std::string_view value, const std::string& lowered_needle) {
    return lowered_needle.empty() || lowercase_copy(value).find(lowered_needle) != std::string::npos;
}

std::string direct_field_descriptor_prefix(const std::string& normalized_property_name) {
    constexpr std::size_t DbfDescriptorNameWidth = 11U;
    return normalized_property_name.substr(0U, std::min(normalized_property_name.size(), DbfDescriptorNameWidth));
}

const DbfRecordValue* find_direct_visual_property_value(
    const std::vector<DbfRecordValue>& values,
    const std::string& property_name) {
    const std::string requested_property_name = normalize_visual_property_name(property_name);
    const auto exact_match = std::find_if(values.begin(), values.end(), [&](const DbfRecordValue& value) {
        return normalize_visual_property_name(value.field_name) == requested_property_name;
    });
    if (exact_match != values.end()) {
        return &*exact_match;
    }

    const std::string descriptor_prefix = direct_field_descriptor_prefix(requested_property_name);
    if (descriptor_prefix == requested_property_name) {
        return nullptr;
    }

    const DbfRecordValue* matched_value = nullptr;
    for (const auto& value : values) {
        if (normalize_visual_property_name(value.field_name) != descriptor_prefix) {
            continue;
        }
        if (matched_value != nullptr) {
            return nullptr;
        }
        matched_value = &value;
    }
    return matched_value;
}

std::vector<RawFieldDescriptor>::const_iterator find_direct_visual_property_field(
    const std::vector<RawFieldDescriptor>& fields,
    const std::string& property_name) {
    const std::string requested_property_name = normalize_visual_property_name(property_name);
    const auto exact_match = std::find_if(fields.begin(), fields.end(), [&](const RawFieldDescriptor& field) {
        return normalize_visual_property_name(field.name) == requested_property_name;
    });
    if (exact_match != fields.end()) {
        return exact_match;
    }

    const std::string descriptor_prefix = direct_field_descriptor_prefix(requested_property_name);
    if (descriptor_prefix == requested_property_name) {
        return fields.end();
    }

    auto matched_field = fields.end();
    for (auto field = fields.begin(); field != fields.end(); ++field) {
        if (normalize_visual_property_name(field->name) != descriptor_prefix) {
            continue;
        }
        if (matched_field != fields.end()) {
            return fields.end();
        }
        matched_field = field;
    }
    return matched_field;
}

bool starts_with_insensitive(const std::string& text, const std::string& prefix) {
    if (text.size() < prefix.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < prefix.size(); ++index) {
        if (std::tolower(static_cast<unsigned char>(text[index])) !=
            std::tolower(static_cast<unsigned char>(prefix[index]))) {
            return false;
        }
    }
    return true;
}

}  // namespace copperfin::vfp
