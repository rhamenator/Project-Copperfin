#include "copperfin/vfp/visual_asset_editor.h"

#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/dbf_table.h"

#include <array>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <system_error>
#include <vector>

namespace copperfin::vfp {

namespace {

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

std::string infer_memo_sidecar_path(const std::string& path) {
    std::filesystem::path file_path(path);
    std::string ext = file_path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    if (ext == ".scx") {
        return file_path.replace_extension(".sct").string();
    }
    if (ext == ".vcx") {
        return file_path.replace_extension(".vct").string();
    }
    if (ext == ".frx") {
        return file_path.replace_extension(".frt").string();
    }
    if (ext == ".lbx") {
        return file_path.replace_extension(".lbt").string();
    }
    if (ext == ".mnx") {
        return file_path.replace_extension(".mnt").string();
    }
    if (ext == ".pjx") {
        return file_path.replace_extension(".pjt").string();
    }
    if (ext == ".dbc") {
        return file_path.replace_extension(".dct").string();
    }
    return {};
}

struct RawFieldDescriptor {
    std::string name;
    char type = '\0';
    std::uint32_t offset = 0;
    std::uint8_t length = 0;
};

struct VisualAssetUndoEntry {
    std::size_t record_index = 0;
    std::string property_name;
    std::string prior_value;
    bool prior_value_exists = false;
    std::string label;
};

struct VisualPropertyState {
    bool exists = false;
    bool direct_field = false;
    std::string property_name;
    std::string value;
    bool record_deleted = false;
};

std::string normalize_visual_object_name(std::string value) {
    value = trim_both(std::move(value));
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string normalize_visual_property_name(std::string value) {
    value = trim_both(std::move(value));
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

const DbfRecordValue* find_record_value(const DbfRecord& record, const std::string& field_name) {
    const std::string requested_field_name = normalize_visual_property_name(field_name);
    const auto value = std::find_if(record.values.begin(), record.values.end(), [&](const DbfRecordValue& candidate) {
        return normalize_visual_property_name(candidate.field_name) == requested_field_name;
    });
    return value == record.values.end() ? nullptr : &(*value);
}

std::vector<std::size_t> find_matching_record_indexes(
    const DbfTable& table,
    const std::string& field_name,
    const std::string& requested_value) {
    std::vector<std::size_t> matches;
    for (const auto& record : table.records) {
        const auto* value = find_record_value(record, field_name);
        if (value == nullptr) {
            continue;
        }
        if (normalize_visual_object_name(value->display_value) == requested_value) {
            matches.push_back(record.record_index);
        }
    }
    return matches;
}

VisualAssetEditResult resolve_visual_object_record_index(const VisualObjectEditRequest& request, std::size_t& record_index) {
    const std::string requested_unique_id = normalize_visual_object_name(request.unique_id);
    if (!requested_unique_id.empty()) {
        const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
        if (!table_result.ok) {
            return {.ok = false, .error = table_result.error};
        }

        const std::vector<std::size_t> matches = find_matching_record_indexes(
            table_result.table,
            "UNIQUEID",
            requested_unique_id);
        if (matches.empty()) {
            return {.ok = false, .error = "No visual object with the requested unique id was found."};
        }
        if (matches.size() > 1U) {
            return {.ok = false, .error = "The requested visual object unique id is ambiguous."};
        }

        record_index = matches.front();
        return {.ok = true, .error = {}};
    }

    if (request.object_name.empty()) {
        record_index = request.record_index;
        return {.ok = true, .error = {}};
    }

    const std::string requested_name = normalize_visual_object_name(request.object_name);
    if (requested_name.empty()) {
        return {.ok = false, .error = "No object name was provided."};
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }

    std::vector<std::size_t> matches = find_matching_record_indexes(table_result.table, "OBJNAME", requested_name);
    if (matches.empty()) {
        matches = find_matching_record_indexes(table_result.table, "NAME", requested_name);
    }

    if (matches.empty()) {
        return {.ok = false, .error = "No visual object with the requested name was found."};
    }
    if (matches.size() > 1U) {
        return {.ok = false, .error = "The requested visual object name is ambiguous."};
    }

    record_index = matches.front();
    return {.ok = true, .error = {}};
}

VisualAssetEditResult replace_memo_field_value(
    const std::string& table_path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& new_value);

std::optional<char> normalize_logical_value(std::string value) {
    value = trim_both(std::move(value));
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    if (value.empty() || value == "null" || value == "?") {
        return '?';
    }
    if (value == "t" || value == "true" || value == "y" || value == "yes" || value == ".t.") {
        return 'T';
    }
    if (value == "f" || value == "false" || value == "n" || value == "no" || value == ".f.") {
        return 'F';
    }
    return std::nullopt;
}

VisualAssetEditResult replace_non_memo_field_value(
    const std::string& table_path,
    std::size_t record_index,
    const RawFieldDescriptor& field,
    const std::string& new_value) {
    auto table_bytes = read_binary_file(table_path);
    if (table_bytes.empty()) {
        return {.ok = false, .error = "Unable to open the visual asset table."};
    }

    const auto header_result = parse_dbf_header(table_bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    if (record_index >= header_result.header.record_count) {
        return {.ok = false, .error = "Record index is out of range for the asset."};
    }

    const std::size_t record_offset = header_result.header.header_length +
                                      (record_index * header_result.header.record_length);
    const std::size_t field_offset = record_offset + field.offset;
    if ((field_offset + field.length) > table_bytes.size()) {
        return {.ok = false, .error = "Record data is truncated."};
    }

    std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset), field.length, static_cast<std::uint8_t>(' '));

    switch (field.type) {
        case 'C': {
            const std::string text = trim_both(new_value);
            if (text.size() > field.length) {
                return {.ok = false, .error = "Character value is too large for the target field."};
            }
            std::copy(text.begin(),
                      text.end(),
                      table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset));
            break;
        }
        case 'N':
        case 'F': {
            const std::string text = trim_both(new_value);
            if (text.empty()) {
                break;
            }
            if (text.size() > field.length) {
                return {.ok = false, .error = "Numeric value is too large for the target field."};
            }
            const auto padding = static_cast<std::ptrdiff_t>(field.length - text.size());
            std::copy(text.begin(),
                      text.end(),
                      table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset) + padding);
            break;
        }
        case 'L': {
            const auto logical_value = normalize_logical_value(new_value);
            if (!logical_value.has_value()) {
                return {.ok = false, .error = "Logical fields only accept true/false values."};
            }
            table_bytes[field_offset] = static_cast<std::uint8_t>(*logical_value);
            break;
        }
        default:
            return {.ok = false, .error = "Direct updates are not implemented for this field type yet."};
    }

    if (!write_binary_file(table_path, table_bytes)) {
        return {.ok = false, .error = "Unable to write the visual asset table."};
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult replace_field_value(
    const std::string& table_path,
    std::size_t record_index,
    const RawFieldDescriptor& field,
    const std::string& new_value) {
    if (field.type == 'M') {
        return replace_memo_field_value(table_path, record_index, field.name, new_value);
    }

    return replace_non_memo_field_value(table_path, record_index, field, new_value);
}

std::vector<RawFieldDescriptor> read_raw_field_descriptors(const std::vector<std::uint8_t>& table_bytes) {
    std::vector<RawFieldDescriptor> fields;
    std::size_t descriptor_offset = 32U;
    while ((descriptor_offset + 32U) <= table_bytes.size() && table_bytes[descriptor_offset] != 0x0DU) {
        fields.push_back({
            .name = read_ascii_name(table_bytes, descriptor_offset, 11U),
            .type = static_cast<char>(table_bytes[descriptor_offset + 11U]),
            .offset = read_le_u32(table_bytes, descriptor_offset + 12U),
            .length = table_bytes[descriptor_offset + 16U]
        });
        descriptor_offset += 32U;
    }
    return fields;
}

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
        error = "Unable to create the visual asset undo journal.";
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
        error = "Unable to persist the visual asset undo journal.";
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

std::optional<VisualPropertyState> read_current_visual_property_state(
    const std::string& path,
    std::size_t record_index,
    const std::string& property_name) {
    const auto table_result = parse_dbf_table_from_file(path, record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return std::nullopt;
    }

    const auto& record = table_result.table.records[record_index];
    const std::string requested_property_name = normalize_visual_property_name(property_name);
    const auto direct_field_value = std::find_if(record.values.begin(), record.values.end(), [&](const DbfRecordValue& value) {
        return normalize_visual_property_name(value.field_name) == requested_property_name;
    });
    if (direct_field_value != record.values.end()) {
        return VisualPropertyState{
            .exists = true,
            .direct_field = true,
            .property_name = direct_field_value->field_name,
            .value = direct_field_value->display_value,
            .record_deleted = record.deleted
        };
    }

    if (!is_property_blob_asset_path(path)) {
        return std::nullopt;
    }

    const auto properties_field = std::find_if(record.values.begin(), record.values.end(), [](const DbfRecordValue& value) {
        return value.field_name == "PROPERTIES";
    });
    if (properties_field == record.values.end()) {
        return std::nullopt;
    }

    const auto assignments = parse_visual_property_blob(properties_field->display_value);
    const auto property = std::find_if(assignments.begin(), assignments.end(), [&](const VisualPropertyAssignment& assignment) {
        return normalize_visual_property_name(assignment.name) == requested_property_name;
    });
    if (property == assignments.end()) {
        return VisualPropertyState{
            .exists = false,
            .direct_field = false,
            .property_name = trim_both(property_name),
            .value = {},
            .record_deleted = record.deleted
        };
    }

    return VisualPropertyState{
        .exists = true,
        .direct_field = false,
        .property_name = property->name,
        .value = property->value,
        .record_deleted = record.deleted
    };
}

VisualAssetEditResult apply_visual_object_property_change(
    const VisualObjectEditRequest& request,
    bool record_undo_entry,
    bool remove_property_if_missing) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.property_name.empty()) {
        return {.ok = false, .error = "No property name was provided."};
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index(request, record_index);
    if (!resolution.ok) {
        return resolution;
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    if (record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available."};
    }

    const auto table_bytes = read_binary_file(request.path);
    if (table_bytes.empty()) {
        return {.ok = false, .error = "Unable to open the visual asset table."};
    }

    const auto fields = read_raw_field_descriptors(table_bytes);
    const std::string requested_property_name = normalize_visual_property_name(request.property_name);
    const auto direct_field_it = std::find_if(fields.begin(), fields.end(), [&](const RawFieldDescriptor& field) {
        return normalize_visual_property_name(field.name) == requested_property_name;
    });
    if (direct_field_it != fields.end()) {
        if (record_undo_entry) {
            const auto property_state = read_current_visual_property_state(request.path, record_index, request.property_name);
            if (!property_state.has_value()) {
                return {.ok = false, .error = "Unable to read the current property value for undo."};
            }
            if (!property_state->direct_field) {
                return {.ok = false, .error = "Property lookup mismatch while recording undo."};
            }
            if (property_state->exists && property_state->value == request.property_value) {
                return {.ok = true, .error = {}};
            }

            std::string error;
            if (!record_visual_asset_undo_entry(request.path, {
                    .record_index = record_index,
                    .property_name = request.property_name,
                    .prior_value = property_state->value,
                    .prior_value_exists = property_state->exists,
                    .label = "Property " + request.property_name
                }, error)) {
                return {.ok = false, .error = error};
            }
        }

        return replace_field_value(request.path, record_index, *direct_field_it, request.property_value);
    }

    if (!is_property_blob_asset_path(request.path)) {
        return {.ok = false, .error = "The requested property is not exposed as a writable field on this asset."};
    }

    const auto& record = table_result.table.records[record_index];
    auto properties_it = std::find_if(record.values.begin(), record.values.end(), [](const DbfRecordValue& value) {
        return value.field_name == "PROPERTIES";
    });
    if (properties_it == record.values.end()) {
        return {.ok = false, .error = "The object does not expose a PROPERTIES memo field."};
    }

    auto assignments = parse_visual_property_blob(properties_it->display_value);
    auto assignment_it = std::find_if(assignments.begin(), assignments.end(), [&](const VisualPropertyAssignment& property) {
        return normalize_visual_property_name(property.name) == requested_property_name;
    });

    if (record_undo_entry) {
        const bool exists = assignment_it != assignments.end();
        const std::string prior_value = exists ? assignment_it->value : std::string{};
        if (exists && prior_value == request.property_value) {
            return {.ok = true, .error = {}};
        }

        std::string error;
        if (!record_visual_asset_undo_entry(request.path, {
                .record_index = record_index,
                .property_name = request.property_name,
                .prior_value = prior_value,
                .prior_value_exists = exists,
                .label = "Property " + request.property_name
            }, error)) {
            return {.ok = false, .error = error};
        }
    }

    if (assignment_it == assignments.end()) {
        if (!remove_property_if_missing) {
            assignments.push_back({.name = request.property_name, .value = request.property_value});
        }
    } else if (remove_property_if_missing) {
        assignments.erase(assignment_it);
    } else {
        assignment_it->value = request.property_value;
    }

    return replace_memo_field_value(
        request.path,
        record_index,
        "PROPERTIES",
        serialize_visual_property_blob(assignments));
}

VisualAssetEditResult replace_memo_field_value(
    const std::string& table_path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& new_value) {
    auto table_bytes = read_binary_file(table_path);
    if (table_bytes.empty()) {
        return {.ok = false, .error = "Unable to open the visual asset table."};
    }

    const auto header_result = parse_dbf_header(table_bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(table_bytes);

    const auto field_it = std::find_if(fields.begin(), fields.end(), [&](const RawFieldDescriptor& field) {
        return field.name == field_name;
    });
    if (field_it == fields.end()) {
        return {.ok = false, .error = "The target field was not found in the asset."};
    }
    if (field_it->type != 'M') {
        return {.ok = false, .error = "The target field is not a memo-backed field."};
    }

    if (record_index >= header_result.header.record_count) {
        return {.ok = false, .error = "Record index is out of range for the asset."};
    }

    const std::size_t record_offset = header_result.header.header_length +
                                      (record_index * header_result.header.record_length);
    const std::size_t field_offset = record_offset + field_it->offset;
    if ((field_offset + 4U) > table_bytes.size()) {
        return {.ok = false, .error = "Record data is truncated."};
    }

    const std::string memo_path = infer_memo_sidecar_path(table_path);
    if (memo_path.empty()) {
        return {.ok = false, .error = "No memo sidecar path could be inferred for the asset."};
    }

    auto memo_bytes = read_binary_file(memo_path);
    if (memo_bytes.size() < 8U) {
        return {.ok = false, .error = "Unable to open the memo sidecar."};
    }

    const std::uint16_t block_size = read_be_u16(memo_bytes, 6U);
    if (block_size == 0U) {
        return {.ok = false, .error = "Memo sidecar block size is invalid."};
    }

    std::uint32_t block_number = read_le_u32(table_bytes, field_offset);
    std::array<std::uint8_t, 4U> original_block_header = {0U, 0U, 0U, 1U};

    if (block_number != 0U) {
        const std::size_t old_block_offset = static_cast<std::size_t>(block_number) * block_size;
        if ((old_block_offset + 8U) <= memo_bytes.size()) {
            for (std::size_t index = 0; index < original_block_header.size(); ++index) {
                original_block_header[index] = memo_bytes[old_block_offset + index];
            }
        }
    }

    const std::uint32_t next_free_block = read_be_u32(memo_bytes, 0U);
    if (next_free_block == 0U) {
        return {.ok = false, .error = "Memo sidecar next-free-block pointer is invalid."};
    }

    const auto required_bytes = static_cast<std::size_t>(8U + new_value.size());
    const auto required_blocks = static_cast<std::uint32_t>((required_bytes + block_size - 1U) / block_size);
    const std::size_t new_block_offset = static_cast<std::size_t>(next_free_block) * block_size;
    const std::size_t new_total_size = new_block_offset + (static_cast<std::size_t>(required_blocks) * block_size);
    if (memo_bytes.size() < new_total_size) {
        memo_bytes.resize(new_total_size, 0U);
    }

    for (std::size_t index = 0; index < original_block_header.size(); ++index) {
        memo_bytes[new_block_offset + index] = original_block_header[index];
    }
    write_be_u32(memo_bytes, new_block_offset + 4U, static_cast<std::uint32_t>(new_value.size()));
    std::fill(
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(new_block_offset + 8U),
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(new_total_size),
        static_cast<std::uint8_t>(0U));
    std::copy(
        new_value.begin(),
        new_value.end(),
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(new_block_offset + 8U));

    write_be_u32(memo_bytes, 0U, next_free_block + required_blocks);
    write_le_u32(table_bytes, field_offset, next_free_block);

    if (!write_binary_file(memo_path, memo_bytes)) {
        return {.ok = false, .error = "Unable to write the memo sidecar."};
    }
    if (!write_binary_file(table_path, table_bytes)) {
        return {.ok = false, .error = "Unable to write the visual asset table."};
    }

    return {.ok = true, .error = {}};
}

}  // namespace

std::vector<VisualPropertyAssignment> parse_visual_property_blob(const std::string& text) {
    std::vector<VisualPropertyAssignment> properties;
    std::stringstream stream(text);
    std::string line;
    std::size_t line_index = 0U;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        const auto equals = line.find('=');
        if (equals == std::string::npos) {
            if (!trim_both(line).empty()) {
                properties.push_back({.name = trim_both(line), .value = {}, .source_line_index = line_index});
            }
            ++line_index;
            continue;
        }

        properties.push_back({
            .name = trim_both(line.substr(0U, equals)),
            .value = trim_both(line.substr(equals + 1U)),
            .source_line_index = line_index
        });
        ++line_index;
    }
    return properties;
}

std::string serialize_visual_property_blob(const std::vector<VisualPropertyAssignment>& properties) {
    std::ostringstream stream;
    for (const auto& property : properties) {
        if (property.name.empty()) {
            continue;
        }

        stream << property.name;
        if (!property.value.empty()) {
            stream << " = " << property.value;
        }
        stream << "\r\n";
    }
    return stream.str();
}

bool is_property_blob_asset_path(const std::string& path) {
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return ext == ".scx" || ext == ".vcx";
}

VisualAssetEditResult update_visual_object_property(const VisualObjectEditRequest& request) {
    return apply_visual_object_property_change(request, true, false);
}

VisualObjectPropertyQueryResult query_visual_object_property(const VisualObjectPropertyQueryRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = "No asset path was provided.",
            .exists = false,
            .direct_field = false,
            .record_index = 0U,
            .record_deleted = false,
            .property_name = {},
            .value = {}
        };
    }
    if (request.property_name.empty()) {
        return {
            .ok = false,
            .error = "No property name was provided.",
            .exists = false,
            .direct_field = false,
            .record_index = 0U,
            .record_deleted = false,
            .property_name = {},
            .value = {}
        };
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = request.property_name,
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return {
            .ok = false,
            .error = resolution.error,
            .exists = false,
            .direct_field = false,
            .record_index = 0U,
            .record_deleted = false,
            .property_name = {},
            .value = {}
        };
    }

    const auto property_state = read_current_visual_property_state(
        request.path,
        record_index,
        request.property_name);
    if (!property_state.has_value()) {
        return {
            .ok = false,
            .error = "Unable to read the requested property.",
            .exists = false,
            .direct_field = false,
            .record_index = 0U,
            .record_deleted = false,
            .property_name = {},
            .value = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .exists = property_state->exists,
        .direct_field = property_state->direct_field,
        .record_index = record_index,
        .record_deleted = property_state->record_deleted,
        .property_name = property_state->property_name,
        .value = property_state->value
    };
}

VisualObjectPropertyListResult list_visual_object_properties(const VisualObjectPropertyListRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = "No asset path was provided.",
            .record_index = 0U,
            .record_deleted = false,
            .properties = {}
        };
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return {
            .ok = false,
            .error = resolution.error,
            .record_index = 0U,
            .record_deleted = false,
            .properties = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .record_index = 0U,
            .record_deleted = false,
            .properties = {}
        };
    }
    if (record_index >= table_result.table.records.size()) {
        return {
            .ok = false,
            .error = "The requested object record is not currently available.",
            .record_index = 0U,
            .record_deleted = false,
            .properties = {}
        };
    }

    std::vector<VisualObjectPropertySnapshot> properties;
    const auto& record = table_result.table.records[record_index];
    for (const auto& value : record.values) {
        if (normalize_visual_property_name(value.field_name) == "properties") {
            continue;
        }
        properties.push_back({
            .property_name = value.field_name,
            .value = value.display_value,
            .direct_field = true,
            .field_type = value.field_type,
            .source_line_index = static_cast<std::size_t>(-1)
        });
    }

    const auto properties_field = std::find_if(record.values.begin(), record.values.end(), [](const DbfRecordValue& value) {
        return normalize_visual_property_name(value.field_name) == "properties";
    });
    if (properties_field != record.values.end()) {
        for (const auto& assignment : parse_visual_property_blob(properties_field->display_value)) {
            properties.push_back({
                .property_name = assignment.name,
                .value = assignment.value,
                .direct_field = false,
                .field_type = '\0',
                .source_line_index = assignment.source_line_index
            });
        }
    }

    return {
        .ok = true,
        .error = {},
        .record_index = record_index,
        .record_deleted = record.deleted,
        .properties = std::move(properties)
    };
}

VisualAssetEditResult set_visual_object_deleted_state(const VisualObjectDeletedStateRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, record_index);
    if (!resolution.ok) {
        return resolution;
    }

    const auto result = set_record_deleted_flag(request.path, record_index, request.deleted);
    if (!result.ok) {
        return {.ok = false, .error = result.error};
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult update_visual_object_properties(const VisualObjectMultiEditRequest& request) {
    if (request.properties.empty()) {
        return {.ok = false, .error = "No property changes were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    for (const auto& property : request.properties) {
        const auto result = update_visual_object_property({
            .path = request.path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .property_name = property.property_name,
            .property_value = property.property_value
        });
        if (!result.ok) {
            while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
                const auto rollback_result = undo_visual_object_property(request.path);
                if (!rollback_result.ok) {
                    return {
                        .ok = false,
                        .error = result.error + " Rollback failed: " + rollback_result.error
                    };
                }
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetUndoStatus query_visual_object_undo(const std::string& path) {
    return query_visual_asset_undo_status_internal(path);
}

VisualAssetEditResult undo_visual_object_property(const std::string& path) {
    if (path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }

    const auto files = list_visual_asset_undo_entry_files(path);
    if (files.empty()) {
        return {.ok = false, .error = "No visual asset undo history is available."};
    }

    const auto entry = read_visual_asset_undo_entry(files.back());
    if (!entry.has_value()) {
        return {.ok = false, .error = "Unable to read the visual asset undo journal."};
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

    return {.ok = true, .error = {}};
}

}  // namespace copperfin::vfp
