#include "visual_asset_editor_support.h"

namespace copperfin::vfp {
std::filesystem::path visual_asset_undo_root_directory(const std::string& path) {
    const auto normalized = std::filesystem::absolute(std::filesystem::path(path)).string();
    const auto hash = static_cast<unsigned long long>(std::hash<std::string>{}(normalized));
    std::ostringstream stream;
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
    return static_cast<bool>(output);
}

std::optional<VisualAssetUndoEntry> read_visual_asset_undo_entry(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }

    VisualAssetUndoEntry entry;
    std::uint64_t record_index = 0;
    std::uint8_t prior_exists = 0;
    std::uint64_t property_name_length = 0;
    std::uint64_t prior_value_length = 0;
    std::uint64_t label_length = 0;
    input.read(reinterpret_cast<char*>(&record_index), sizeof(record_index));
    input.read(reinterpret_cast<char*>(&prior_exists), sizeof(prior_exists));
    input.read(reinterpret_cast<char*>(&property_name_length), sizeof(property_name_length));
    input.read(reinterpret_cast<char*>(&prior_value_length), sizeof(prior_value_length));
    input.read(reinterpret_cast<char*>(&label_length), sizeof(label_length));
    if (!input.good()) {
        return std::nullopt;
    }

    entry.record_index = static_cast<std::size_t>(record_index);
    entry.prior_value_exists = prior_exists != 0U;
    entry.property_name.resize(static_cast<std::size_t>(property_name_length));
    entry.prior_value.resize(static_cast<std::size_t>(prior_value_length));
    entry.label.resize(static_cast<std::size_t>(label_length));
    input.read(entry.property_name.data(), static_cast<std::streamsize>(entry.property_name.size()));
    input.read(entry.prior_value.data(), static_cast<std::streamsize>(entry.prior_value.size()));
    input.read(entry.label.data(), static_cast<std::streamsize>(entry.label.size()));
    return input.good() ? std::optional<VisualAssetUndoEntry>(entry) : std::nullopt;
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
    if (!existing_files.empty()) {
        try {
            next_index = static_cast<std::uint64_t>(std::stoull(existing_files.back().stem().string())) + 1U;
        } catch (...) {
            next_index = static_cast<std::uint64_t>(existing_files.size()) + 1U;
        }
    }

    std::ostringstream file_name;
    file_name << std::setw(20) << std::setfill('0') << next_index << ".bin";
    const auto entry_path = entries_directory / file_name.str();
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

    const auto result = apply_visual_object_property_change(
        {
            .path = path,
            .record_index = entry->record_index,
            .object_name = {},
            .unique_id = {},
            .property_name = entry->property_name,
            .property_value = entry->prior_value
        },
        false,
        !entry->prior_value_exists);
    if (!result.ok) {
        return result;
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
