// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "visual_asset_editor_support.h"

#include "copperfin/platform/environment.h"
#include "copperfin/platform/path.h"

#include <mutex>

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
    const std::filesystem::path target = copperfin::platform::path_from_utf8_string(path);
    std::filesystem::path staged = target;
    staged += ".cptmp";
    std::filesystem::path backup = target;
    backup += ".cpbak";
    return {
        .target = target,
        .staged = std::move(staged),
        .backup = std::move(backup),
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

std::string lowercase_extension(const std::string& path) {
    std::string extension = copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(path).extension());
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return extension;
}

bool primary_requires_memo_sidecar(const std::string& path) {
    const std::string extension = lowercase_extension(path);
    return extension == ".pjx" || extension == ".scx" || extension == ".vcx" ||
           extension == ".frx" || extension == ".lbx" || extension == ".mnx" ||
           extension == ".dbc";
}

bool dbf_storage_requires_memo_sidecar(const std::string& table_path) {
    std::vector<std::uint8_t> table_bytes = read_binary_file(table_path);
    if (table_bytes.empty()) {
        table_bytes = read_binary_file(table_path + ".cpbak");
    }
    const DbfParseResult header_result = parse_dbf_header(table_bytes);
    if (!header_result.ok) {
        return false;
    }
    if (header_result.header.has_memo_file()) {
        return true;
    }
    if (header_result.header.header_length > table_bytes.size()) {
        return false;
    }

    constexpr std::size_t DbfFieldDescriptorSize = 32U;
    constexpr std::size_t DbfFieldTypeOffset = 11U;
    for (std::size_t descriptor_offset = 32U;
         descriptor_offset < header_result.header.header_length;
         descriptor_offset += DbfFieldDescriptorSize) {
        if (table_bytes[descriptor_offset] == 0x0DU) {
            return false;
        }
        if (DbfFieldDescriptorSize >
            header_result.header.header_length - descriptor_offset) {
            return false;
        }
        const char field_type = static_cast<char>(
            table_bytes[descriptor_offset + DbfFieldTypeOffset]);
        if (field_type == 'M' || field_type == 'G' || field_type == 'P') {
            return true;
        }
    }
    return false;
}

}  // namespace

copperfin::localization::LocalizedCatalog visual_asset_editor_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        copperfin::localization::LocalizedCatalog catalog;
    };

    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        copperfin::localization::load_catalogs(
            copperfin::localization::resolve_catalog_root(),
            copperfin::localization::default_locale)};
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root();
    const std::string locale = copperfin::localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = copperfin::localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
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
    std::ifstream input(
        copperfin::platform::path_from_utf8_string(path),
        std::ios::binary);
    if (!input) {
        return {};
    }

    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

bool write_binary_file(const std::string& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream output(
        copperfin::platform::path_from_utf8_string(path),
        std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }

    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(output);
}

VisualAssetEditResult recover_visual_asset_table_transaction(
    const std::string& table_path) {
    const auto table = transaction_file(table_path);
    std::error_code error;
    const bool target_exists = std::filesystem::exists(table.target, error);
    if (error) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed")};
    }
    const bool backup_exists = std::filesystem::exists(table.backup, error);
    if (error) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed")};
    }

    bool recovered = true;
    if (target_exists) {
        recovered = remove_transaction_path(table.staged) &&
            remove_transaction_path(table.backup);
    } else if (backup_exists) {
        recovered = rename_transaction_path(table.backup, table.target) &&
            remove_transaction_path(table.staged);
    } else {
        recovered = remove_transaction_path(table.staged);
    }
    if (!recovered) {
        return {
            .ok = false,
            .error = visual_asset_rollback_failed_text(
                visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed"),
                visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed"))
        };
    }
    return {.ok = true, .error = {}};
}

VisualAssetEditResult write_visual_asset_table_transaction(
    const std::string& table_path,
    const std::vector<std::uint8_t>& table_bytes) {
    const auto recovery_result = recover_visual_asset_table_transaction(table_path);
    if (!recovery_result.ok) {
        return recovery_result;
    }

    const auto table = transaction_file(table_path, &table_bytes);
    const auto fail = [&](std::string error, bool inject_rollback_failure = false) {
        if (!restore_transaction_file(table) || inject_rollback_failure) {
            return VisualAssetEditResult{
                .ok = false,
                .error = visual_asset_rollback_failed_text(
                    std::move(error),
                    visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed"))
            };
        }
        return VisualAssetEditResult{.ok = false, .error = std::move(error)};
    };

    if (transaction_failure_requested(table_path, {}, "primary-stage") ||
        !stage_transaction_file(table)) {
        return fail(visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed"));
    }
    if (!rename_transaction_path(table.target, table.backup)) {
        return fail(visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed"));
    }
    const bool inject_rollback_failure =
        transaction_failure_requested(table_path, {}, "rollback");
    if (transaction_failure_requested(table_path, {}, "first-commit") ||
        inject_rollback_failure ||
        !rename_transaction_path(table.staged, table.target)) {
        return fail(
            visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed"),
            inject_rollback_failure);
    }

    (void)remove_transaction_path(table.backup);
    return {.ok = true, .error = {}};
}

VisualAssetEditResult recover_visual_asset_file_transaction(
    const std::string& table_path,
    const std::string& memo_path) {
    const auto table = transaction_file(table_path);
    const auto memo = transaction_file(memo_path);
    std::filesystem::path commit_marker = table.target;
    commit_marker += ".cpcommit";

    if (transaction_failure_requested(table_path, memo_path, "stale-cleanup")) {
        return {
            .ok = false,
            .error = visual_asset_rollback_failed_text(
                visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed"),
                transaction_rollback_error())
        };
    }

    const std::vector<std::uint8_t> marker_bytes = read_binary_file(
        copperfin::platform::path_to_utf8_string(commit_marker));
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
    std::filesystem::path commit_marker = table.target;
    commit_marker += ".cpcommit";

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

SidecarPathResolution infer_memo_sidecar_path(const std::string& path) {
    return resolve_vfp_memo_sidecar_path(
        copperfin::platform::path_from_utf8_string(path));
}

std::string selected_memo_sidecar_path(const SidecarPathResolution& resolution) {
    return copperfin::platform::path_to_utf8_string(
        resolution.path.value_or(resolution.requested_path));
}

std::string ambiguous_memo_sidecar_error(const SidecarPathResolution& resolution) {
    return visual_asset_text(
        "Vfp.Sidecar.Error.AmbiguousPath",
        {{"path", copperfin::platform::path_to_utf8_string(resolution.requested_path)}});
}

VisualAssetEditResult resolve_visual_asset_storage_memo_path(
    const std::string& table_path,
    std::string& memo_path) {
    memo_path.clear();
    const SidecarPathResolution memo_resolution = infer_memo_sidecar_path(table_path);
    const bool requires_memo_sidecar = primary_requires_memo_sidecar(table_path) ||
        (lowercase_extension(table_path) == ".dbf" &&
         dbf_storage_requires_memo_sidecar(table_path));
    if (!requires_memo_sidecar) {
        return {.ok = true, .error = {}};
    }
    if (memo_resolution.ambiguous) {
        return {.ok = false, .error = ambiguous_memo_sidecar_error(memo_resolution)};
    }
    memo_path = selected_memo_sidecar_path(memo_resolution);
    return {.ok = true, .error = {}};
}

VisualAssetEditResult recover_visual_asset_storage_transaction(
    const std::string& table_path) {
    const SidecarPathResolution memo_resolution = infer_memo_sidecar_path(table_path);

    for (const std::string_view suffix : {".cpbak", ".cptmp"}) {
        if (memo_resolution.requested_path.empty()) {
            break;
        }
        std::filesystem::path artifact_path = memo_resolution.requested_path;
        artifact_path += suffix;
        const SidecarPathResolution artifact_resolution = resolve_unique_casefold_path(
            artifact_path);
        if (artifact_resolution.ambiguous) {
            return {.ok = false, .error = ambiguous_memo_sidecar_error(artifact_resolution)};
        }
        if (artifact_resolution.path.has_value()) {
            std::filesystem::path recovered_memo_path = *artifact_resolution.path;
            recovered_memo_path.replace_extension();
            return recover_visual_asset_file_transaction(
                table_path,
                copperfin::platform::path_to_utf8_string(recovered_memo_path));
        }
    }

    const SidecarPathResolution commit_resolution = resolve_unique_casefold_path(
        copperfin::platform::path_from_utf8_string(table_path + ".cpcommit"));
    if (commit_resolution.ambiguous) {
        return {.ok = false, .error = ambiguous_memo_sidecar_error(commit_resolution)};
    }
    if (commit_resolution.path.has_value() && !memo_resolution.requested_path.empty()) {
        return recover_visual_asset_file_transaction(
            table_path,
            copperfin::platform::path_to_utf8_string(memo_resolution.requested_path));
    }

    std::string memo_path;
    const auto resolution_result = resolve_visual_asset_storage_memo_path(
        table_path,
        memo_path);
    if (!resolution_result.ok) {
        return resolution_result;
    }
    if (memo_path.empty()) {
        return recover_visual_asset_table_transaction(table_path);
    }
    return recover_visual_asset_file_transaction(table_path, memo_path);
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
    constexpr std::size_t VfpFreeTableFieldNameMaxBytes = 10U;
    return normalized_property_name.substr(
        0U,
        std::min(normalized_property_name.size(), VfpFreeTableFieldNameMaxBytes));
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
