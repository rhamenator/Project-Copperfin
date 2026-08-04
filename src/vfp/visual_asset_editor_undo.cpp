// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "visual_asset_editor_support.h"

#include "copperfin/platform/path.h"

#include <charconv>
#include <locale>
#include <new>
#include <stdexcept>

namespace copperfin::vfp {

namespace {

constexpr std::uint64_t kVisualAssetUndoGroupedHeaderBytes =
    sizeof(std::uint64_t) + sizeof(std::uint8_t) + 2U * sizeof(std::uint64_t);

bool read_undo_bytes(
    std::ifstream& input,
    char* destination,
    std::uint64_t length,
    std::uint64_t& remaining) {
    if (length > remaining ||
        length > static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max())) {
        return false;
    }
    if (length != 0U) {
        input.read(destination, static_cast<std::streamsize>(length));
        if (!input.good()) {
            return false;
        }
    }
    remaining -= length;
    return true;
}

template <typename Value>
bool read_undo_value(std::ifstream& input, Value& value, std::uint64_t& remaining) {
    return read_undo_bytes(
        input,
        reinterpret_cast<char*>(&value),
        sizeof(value),
        remaining);
}

bool read_undo_string(
    std::ifstream& input,
    std::uint64_t length,
    std::uint64_t& remaining,
    std::string& value) {
    if (length > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()) ||
        length > static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max()) ||
        length > remaining) {
        return false;
    }
    value.resize(static_cast<std::size_t>(length));
    return read_undo_bytes(input, value.data(), length, remaining);
}

bool undo_record_index_fits(std::uint64_t value) {
    return value <= static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max());
}

bool undo_change_targets_asset(
    const std::string& path,
    const VisualAssetUndoEntry& change,
    const DbfTable& table) {
    if (change.record_index >= table.records.size()) {
        return false;
    }
    if (change.property_name == kVisualAssetDeletedStateUndoPropertyName) {
        return change.prior_value_exists &&
            (change.prior_value == "0" || change.prior_value == "1");
    }
    if (trim_both(change.property_name).empty()) {
        return false;
    }
    return read_current_visual_property_state(
        path,
        change.record_index,
        change.property_name).has_value();
}

std::optional<std::uint64_t> parse_visual_asset_undo_index(const std::filesystem::path& path) {
    const auto stem = path.stem().string();
    if (stem.empty()) {
        return std::nullopt;
    }

    std::uint64_t index = 0U;
    const auto parsed = std::from_chars(stem.data(), stem.data() + stem.size(), index);
    if (parsed.ec != std::errc{} || parsed.ptr != stem.data() + stem.size()) {
        return std::nullopt;
    }
    return index;
}

}  // namespace

std::filesystem::path visual_asset_undo_root_directory(const std::string& path) {
    const auto normalized = copperfin::platform::path_to_utf8_string(
        std::filesystem::absolute(copperfin::platform::path_from_utf8_string(path)));
    const auto hash = static_cast<unsigned long long>(std::hash<std::string>{}(normalized));
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << "asset_" << std::hex << std::setw(16) << std::setfill('0') << hash;
    return std::filesystem::temp_directory_path() / "copperfin_visual_asset_undo" / stream.str();
}

std::filesystem::path visual_asset_undo_entries_directory(const std::string& path) {
    return visual_asset_undo_root_directory(path) / "entries";
}

std::vector<std::filesystem::path> list_visual_asset_undo_entry_files(const std::string& path) {
    std::vector<std::filesystem::path> files;
    const auto entries_directory = visual_asset_undo_entries_directory(path);
    std::error_code error;
    if (!std::filesystem::exists(entries_directory, error)) {
        return files;
    }

    for (const auto& entry : std::filesystem::directory_iterator(entries_directory, error)) {
        if (error) {
            break;
        }
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

bool discard_visual_asset_undo_entries_after_depth(
    const std::string& path,
    std::size_t retained_depth,
    std::string& error) {
    auto files = list_visual_asset_undo_entry_files(path);
    while (files.size() > retained_depth) {
        std::error_code remove_error;
        std::filesystem::remove(files.back(), remove_error);
        if (remove_error) {
            error = visual_asset_text("VisualAssetEditor.Undo.PersistJournalFailed");
            return false;
        }
        files.pop_back();
    }

    const auto entries_directory = visual_asset_undo_entries_directory(path);
    std::error_code cleanup_error;
    if (std::filesystem::exists(entries_directory, cleanup_error) &&
        std::filesystem::is_empty(entries_directory, cleanup_error)) {
        std::filesystem::remove(entries_directory, cleanup_error);
        std::filesystem::remove(visual_asset_undo_root_directory(path), cleanup_error);
    }
    if (cleanup_error) {
        error = visual_asset_text("VisualAssetEditor.Undo.PersistJournalFailed");
        return false;
    }
    return true;
}

bool write_visual_asset_undo_entry(const std::filesystem::path& path, const VisualAssetUndoEntry& entry) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }

    const std::uint64_t record_index = static_cast<std::uint64_t>(entry.record_index);
    const std::uint8_t prior_exists = entry.prior_value_exists ? 1U : 0U;
    const std::uint64_t property_name_length = static_cast<std::uint64_t>(entry.property_name.size());
    const std::uint64_t prior_value_length = static_cast<std::uint64_t>(entry.prior_value.size());
    const std::uint64_t label_length = static_cast<std::uint64_t>(entry.label.size());

    output.write(reinterpret_cast<const char*>(&record_index), sizeof(record_index));
    output.write(reinterpret_cast<const char*>(&prior_exists), sizeof(prior_exists));
    output.write(reinterpret_cast<const char*>(&property_name_length), sizeof(property_name_length));
    output.write(reinterpret_cast<const char*>(&prior_value_length), sizeof(prior_value_length));
    output.write(reinterpret_cast<const char*>(&label_length), sizeof(label_length));
    output.write(entry.property_name.data(), static_cast<std::streamsize>(entry.property_name.size()));
    output.write(entry.prior_value.data(), static_cast<std::streamsize>(entry.prior_value.size()));
    output.write(entry.label.data(), static_cast<std::streamsize>(entry.label.size()));
    const std::uint64_t grouped_change_count = static_cast<std::uint64_t>(entry.grouped_changes.size());
    output.write(reinterpret_cast<const char*>(&grouped_change_count), sizeof(grouped_change_count));
    for (const auto& grouped_change : entry.grouped_changes) {
        const std::uint64_t grouped_record_index = static_cast<std::uint64_t>(grouped_change.record_index);
        const std::uint8_t grouped_prior_exists = grouped_change.prior_value_exists ? 1U : 0U;
        const std::uint64_t grouped_property_name_length = static_cast<std::uint64_t>(grouped_change.property_name.size());
        const std::uint64_t grouped_prior_value_length = static_cast<std::uint64_t>(grouped_change.prior_value.size());

        output.write(reinterpret_cast<const char*>(&grouped_record_index), sizeof(grouped_record_index));
        output.write(reinterpret_cast<const char*>(&grouped_prior_exists), sizeof(grouped_prior_exists));
        output.write(reinterpret_cast<const char*>(&grouped_property_name_length), sizeof(grouped_property_name_length));
        output.write(reinterpret_cast<const char*>(&grouped_prior_value_length), sizeof(grouped_prior_value_length));
        output.write(grouped_change.property_name.data(), static_cast<std::streamsize>(grouped_change.property_name.size()));
        output.write(grouped_change.prior_value.data(), static_cast<std::streamsize>(grouped_change.prior_value.size()));
    }
    return static_cast<bool>(output);
}

std::optional<VisualAssetUndoEntry> read_visual_asset_undo_entry(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }

    input.seekg(0, std::ios::end);
    const std::streamoff end = input.tellg();
    if (end < 0) {
        return std::nullopt;
    }
    std::uint64_t remaining = static_cast<std::uint64_t>(end);
    input.seekg(0, std::ios::beg);
    if (!input.good()) {
        return std::nullopt;
    }

    try {
        VisualAssetUndoEntry entry;
        std::uint64_t record_index = 0;
        std::uint8_t prior_exists = 0;
        std::uint64_t property_name_length = 0;
        std::uint64_t prior_value_length = 0;
        std::uint64_t label_length = 0;
        if (!read_undo_value(input, record_index, remaining) ||
            !read_undo_value(input, prior_exists, remaining) ||
            !read_undo_value(input, property_name_length, remaining) ||
            !read_undo_value(input, prior_value_length, remaining) ||
            !read_undo_value(input, label_length, remaining) ||
            !undo_record_index_fits(record_index) ||
            prior_exists > 1U ||
            !read_undo_string(input, property_name_length, remaining, entry.property_name) ||
            !read_undo_string(input, prior_value_length, remaining, entry.prior_value) ||
            !read_undo_string(input, label_length, remaining, entry.label)) {
            return std::nullopt;
        }

        entry.record_index = static_cast<std::size_t>(record_index);
        entry.prior_value_exists = prior_exists != 0U;

        // Journals created before grouped undo ended exactly after the scalar payload.
        if (remaining == 0U) {
            if (input.peek() != std::ifstream::traits_type::eof()) {
                return std::nullopt;
            }
            return entry;
        }
        if (remaining < sizeof(std::uint64_t)) {
            return std::nullopt;
        }

        std::uint64_t grouped_change_count = 0;
        if (!read_undo_value(input, grouped_change_count, remaining) ||
            grouped_change_count > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()) ||
            grouped_change_count > remaining / kVisualAssetUndoGroupedHeaderBytes) {
            return std::nullopt;
        }

        const auto grouped_change_size = static_cast<std::size_t>(grouped_change_count);
        if (grouped_change_size > entry.grouped_changes.max_size()) {
            return std::nullopt;
        }
        entry.grouped_changes.reserve(grouped_change_size);
        for (std::uint64_t index = 0; index < grouped_change_count; ++index) {
            VisualAssetUndoEntry grouped_change;
            std::uint64_t grouped_record_index = 0;
            std::uint8_t grouped_prior_exists = 0;
            std::uint64_t grouped_property_name_length = 0;
            std::uint64_t grouped_prior_value_length = 0;
            if (!read_undo_value(input, grouped_record_index, remaining) ||
                !read_undo_value(input, grouped_prior_exists, remaining) ||
                !read_undo_value(input, grouped_property_name_length, remaining) ||
                !read_undo_value(input, grouped_prior_value_length, remaining) ||
                !undo_record_index_fits(grouped_record_index) ||
                grouped_prior_exists > 1U ||
                !read_undo_string(
                    input,
                    grouped_property_name_length,
                    remaining,
                    grouped_change.property_name) ||
                !read_undo_string(
                    input,
                    grouped_prior_value_length,
                    remaining,
                    grouped_change.prior_value)) {
                return std::nullopt;
            }

            grouped_change.record_index = static_cast<std::size_t>(grouped_record_index);
            grouped_change.prior_value_exists = grouped_prior_exists != 0U;
            entry.grouped_changes.push_back(std::move(grouped_change));
        }

        if (remaining != 0U || input.peek() != std::ifstream::traits_type::eof()) {
            return std::nullopt;
        }
        return entry;
    } catch (const std::bad_alloc&) {
        return std::nullopt;
    } catch (const std::length_error&) {
        return std::nullopt;
    }
}

bool record_visual_asset_undo_entry(const std::string& path, const VisualAssetUndoEntry& entry, std::string& error) {
    std::error_code fs_error;
    const auto entries_directory = visual_asset_undo_entries_directory(path);
    std::filesystem::create_directories(entries_directory, fs_error);
    if (fs_error) {
        error = visual_asset_text("VisualAssetEditor.Undo.CreateJournalFailed");
        return false;
    }

    const auto existing_files = list_visual_asset_undo_entry_files(path);
    std::uint64_t next_index = 1U;
    for (const auto& existing_file : existing_files) {
        const auto existing_index = parse_visual_asset_undo_index(existing_file);
        if (existing_index.has_value() && *existing_index >= next_index) {
            if (*existing_index == std::numeric_limits<std::uint64_t>::max()) {
                error = visual_asset_text("VisualAssetEditor.Undo.PersistJournalFailed");
                return false;
            }
            next_index = *existing_index + 1U;
        }
    }

    std::filesystem::path entry_path;
    for (;;) {
        std::ostringstream file_name;
        file_name.imbue(std::locale::classic());
        file_name << std::setw(20) << std::setfill('0') << next_index << ".bin";
        entry_path = entries_directory / file_name.str();
        std::error_code exists_error;
        if (!std::filesystem::exists(entry_path, exists_error)) {
            if (exists_error) {
                error = visual_asset_text("VisualAssetEditor.Undo.PersistJournalFailed");
                return false;
            }
            break;
        }
        if (next_index == std::numeric_limits<std::uint64_t>::max()) {
            error = visual_asset_text("VisualAssetEditor.Undo.PersistJournalFailed");
            return false;
        }
        ++next_index;
    }
    if (!write_visual_asset_undo_entry(entry_path, entry)) {
        error = visual_asset_text("VisualAssetEditor.Undo.PersistJournalFailed");
        return false;
    }

    return true;
}

VisualAssetUndoStatus query_visual_asset_undo_status_internal(const std::string& path) {
    const auto files = list_visual_asset_undo_entry_files(path);
    if (files.empty()) {
        return {};
    }

    const auto entry = read_visual_asset_undo_entry(files.back());
    if (!entry.has_value()) {
        return {};
    }

    return {
        .available = true,
        .label = entry->label
    };
}

VisualAssetUndoStatus query_visual_object_undo(const std::string& path) {
    return query_visual_asset_undo_status_internal(path);
}

VisualAssetEditResult undo_visual_object_property(const std::string& path) {
    if (path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }

    const auto files = list_visual_asset_undo_entry_files(path);
    if (files.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Undo.HistoryUnavailable")};
    }

    const auto entry = read_visual_asset_undo_entry(files.back());
    if (!entry.has_value()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Undo.JournalReadFailed")};
    }

    const auto table_result = parse_dbf_table_from_file(
        path,
        std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    const auto change_targets_asset = [&](const VisualAssetUndoEntry& change) {
        return undo_change_targets_asset(path, change, table_result.table);
    };
    const bool targets_are_valid = entry->grouped_changes.empty()
        ? change_targets_asset(*entry)
        : std::all_of(
              entry->grouped_changes.begin(),
              entry->grouped_changes.end(),
              change_targets_asset);
    if (!targets_are_valid) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Undo.JournalReadFailed")};
    }

    const auto apply_undo_entry = [&path](const VisualAssetUndoEntry& change) -> VisualAssetEditResult {
        if (change.property_name == kVisualAssetDeletedStateUndoPropertyName) {
            const auto result = set_record_deleted_flag(path, change.record_index, change.prior_value == "1");
            return {.ok = result.ok, .error = result.error};
        }
        std::string property_value = change.prior_value;
        if (normalize_visual_property_name(change.property_name) == "order") {
            property_value = encode_report_order_value(change.prior_value);
        }
        return apply_visual_object_property_change(
            {
                .path = path,
                .record_index = change.record_index,
                .object_name = {},
                .unique_id = {},
                .property_name = change.property_name,
                .property_value = property_value
            },
            false,
            !change.prior_value_exists);
    };

    if (entry->grouped_changes.empty()) {
        const auto result = apply_undo_entry(*entry);
        if (!result.ok) {
            return result;
        }
    } else {
        for (auto it = entry->grouped_changes.rbegin(); it != entry->grouped_changes.rend(); ++it) {
            const auto result = apply_undo_entry(*it);
            if (!result.ok) {
                return result;
            }
        }
    }

    std::error_code error;
    std::filesystem::remove(files.back(), error);
    const auto entries_directory = visual_asset_undo_entries_directory(path);
    if (!error && std::filesystem::exists(entries_directory, error) && std::filesystem::is_empty(entries_directory, error)) {
        std::filesystem::remove(entries_directory, error);
        std::filesystem::remove(visual_asset_undo_root_directory(path), error);
    }

    return {.ok = true, .error = {}, .affected_object_count = 1U};
}

}  // namespace copperfin::vfp
