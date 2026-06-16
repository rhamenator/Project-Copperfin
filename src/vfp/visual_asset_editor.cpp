#include "copperfin/vfp/visual_asset_editor.h"

#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/dbf_table.h"

#include <array>
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <optional>
#include <sstream>
#include <system_error>
#include <utility>
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

struct VisualObjectGeometry {
    double hpos = 0.0;
    double vpos = 0.0;
    double width = 0.0;
    double height = 0.0;
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

std::vector<std::string> split_visual_lines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

std::vector<VisualObjectMethodSnapshot> parse_visual_methods_blob(
    const std::string& text,
    std::uint32_t source_memo_block_number) {
    std::vector<VisualObjectMethodSnapshot> methods;
    std::string current_name;
    std::string current_kind;
    std::size_t current_line_index = static_cast<std::size_t>(-1);
    std::ostringstream current_source;

    const auto flush = [&]() {
        if (current_name.empty()) {
            return;
        }
        methods.push_back({
            .method_name = current_name,
            .kind = current_kind,
            .source_text = trim_both(current_source.str()),
            .source_line_index = current_line_index,
            .source_memo_block_number = source_memo_block_number
        });
        current_name.clear();
        current_kind.clear();
        current_line_index = static_cast<std::size_t>(-1);
        current_source.str({});
        current_source.clear();
    };

    const std::vector<std::string> lines = split_visual_lines(text);
    for (std::size_t line_index = 0U; line_index < lines.size(); ++line_index) {
        const std::string line = trim_both(lines[line_index]);
        if (starts_with_insensitive(line, "PROCEDURE ") || starts_with_insensitive(line, "FUNCTION ")) {
            flush();
            const auto separator = line.find(' ');
            current_name = trim_both(line.substr(separator + 1U));
            current_kind = starts_with_insensitive(line, "FUNCTION ") ? "function" : "procedure";
            current_line_index = line_index;
            continue;
        }
        if (starts_with_insensitive(line, "ENDPROC") ||
            starts_with_insensitive(line, "ENDFUNC") ||
            starts_with_insensitive(line, "END FUNC")) {
            flush();
            continue;
        }
        if (!current_name.empty()) {
            current_source << lines[line_index] << "\n";
        }
    }

    flush();
    return methods;
}

std::vector<std::string> split_replacement_source_lines(const std::string& source_text) {
    const std::string trimmed = trim_both(source_text);
    if (trimmed.empty()) {
        return {};
    }
    return split_visual_lines(trimmed);
}

bool is_visual_method_end_line(const std::string& line) {
    return starts_with_insensitive(line, "ENDPROC") ||
        starts_with_insensitive(line, "ENDFUNC") ||
        starts_with_insensitive(line, "END FUNC");
}

bool parse_visual_method_declaration(const std::string& line, std::string& kind, std::string& method_name) {
    if (starts_with_insensitive(line, "PROCEDURE ")) {
        const auto separator = line.find(' ');
        kind = "procedure";
        method_name = separator == std::string::npos ? std::string{} : trim_both(line.substr(separator + 1U));
        return !method_name.empty();
    }
    if (starts_with_insensitive(line, "FUNCTION ")) {
        const auto separator = line.find(' ');
        kind = "function";
        method_name = separator == std::string::npos ? std::string{} : trim_both(line.substr(separator + 1U));
        return !method_name.empty();
    }
    return false;
}

std::string serialize_visual_lines(const std::vector<std::string>& lines) {
    std::ostringstream output;
    for (const auto& line : lines) {
        output << line << "\r\n";
    }
    return output.str();
}

std::string update_visual_methods_blob(
    const std::string& existing_blob,
    const std::string& requested_method_name,
    const std::string& requested_kind,
    const std::string& replacement_source) {
    const std::string normalized_requested_name = normalize_visual_object_name(requested_method_name);
    const std::vector<std::string> replacement_lines = split_replacement_source_lines(replacement_source);
    const std::vector<std::string> existing_lines = split_visual_lines(existing_blob);
    std::vector<std::string> output_lines;
    bool replaced = false;
    bool skipping_replaced_body = false;
    std::string replaced_kind;

    for (const auto& raw_line : existing_lines) {
        const std::string trimmed_line = trim_both(raw_line);
        std::string declaration_kind;
        std::string declaration_name;
        if (!skipping_replaced_body &&
            parse_visual_method_declaration(trimmed_line, declaration_kind, declaration_name) &&
            normalize_visual_object_name(declaration_name) == normalized_requested_name) {
            replaced = true;
            skipping_replaced_body = true;
            replaced_kind = declaration_kind;
            output_lines.push_back(raw_line);
            output_lines.insert(output_lines.end(), replacement_lines.begin(), replacement_lines.end());
            continue;
        }

        if (skipping_replaced_body) {
            if (is_visual_method_end_line(trimmed_line)) {
                output_lines.push_back(raw_line);
                skipping_replaced_body = false;
            }
            continue;
        }

        output_lines.push_back(raw_line);
    }

    if (skipping_replaced_body) {
        output_lines.push_back(replaced_kind == "function" ? "ENDFUNC" : "ENDPROC");
    }

    if (!replaced) {
        if (!output_lines.empty() && !trim_both(output_lines.back()).empty()) {
            output_lines.push_back({});
        }
        const std::string normalized_kind = normalize_visual_object_name(requested_kind);
        const bool append_function = normalized_kind == "function";
        output_lines.push_back((append_function ? "FUNCTION " : "PROCEDURE ") + trim_both(requested_method_name));
        output_lines.insert(output_lines.end(), replacement_lines.begin(), replacement_lines.end());
        output_lines.push_back(append_function ? "ENDFUNC" : "ENDPROC");
    }

    return serialize_visual_lines(output_lines);
}

std::string serialize_visual_methods(const std::vector<VisualObjectMethodSnapshot>& methods) {
    std::vector<std::string> output_lines;
    for (std::size_t index = 0U; index < methods.size(); ++index) {
        const auto& method = methods[index];
        if (index != 0U) {
            output_lines.push_back({});
        }
        const bool is_function = normalize_visual_object_name(method.kind) == "function";
        output_lines.push_back((is_function ? "FUNCTION " : "PROCEDURE ") + trim_both(method.method_name));
        const std::vector<std::string> source_lines = split_replacement_source_lines(method.source_text);
        output_lines.insert(output_lines.end(), source_lines.begin(), source_lines.end());
        output_lines.push_back(is_function ? "ENDFUNC" : "ENDPROC");
    }
    return serialize_visual_lines(output_lines);
}

VisualAssetEditResult find_unique_visual_method_index(
    const std::vector<VisualObjectMethodSnapshot>& methods,
    const std::string& method_name,
    const std::string& missing_error,
    const std::string& ambiguous_error,
    std::size_t& method_index) {
    const std::string normalized_method_name = normalize_visual_object_name(method_name);
    std::vector<std::size_t> matches;
    for (std::size_t index = 0U; index < methods.size(); ++index) {
        if (normalize_visual_object_name(methods[index].method_name) == normalized_method_name) {
            matches.push_back(index);
        }
    }
    if (matches.empty()) {
        return {.ok = false, .error = missing_error};
    }
    if (matches.size() > 1U) {
        return {.ok = false, .error = ambiguous_error};
    }
    method_index = matches.front();
    return {.ok = true, .error = {}};
}

VisualAssetEditResult reorder_visual_methods_blob(
    const std::string& existing_blob,
    const std::string& requested_method_name,
    const std::string& placement,
    const std::string& relative_method_name,
    std::string& updated_blob) {
    std::vector<VisualObjectMethodSnapshot> methods = parse_visual_methods_blob(existing_blob, 0U);
    std::size_t source_index = 0U;
    const auto source_result = find_unique_visual_method_index(
        methods,
        requested_method_name,
        "The requested method was not found.",
        "The requested method name is ambiguous.",
        source_index);
    if (!source_result.ok) {
        return source_result;
    }

    const std::string normalized_placement = normalize_visual_object_name(placement);
    const auto moving_method = methods[source_index];
    methods.erase(methods.begin() + static_cast<std::ptrdiff_t>(source_index));

    std::size_t insert_index = methods.size();
    if (normalized_placement == "first") {
        insert_index = 0U;
    } else if (normalized_placement == "last") {
        insert_index = methods.size();
    } else if (normalized_placement == "before" || normalized_placement == "after") {
        if (trim_both(relative_method_name).empty()) {
            return {.ok = false, .error = "No relative method name was provided."};
        }
        if (normalize_visual_object_name(relative_method_name) == normalize_visual_object_name(requested_method_name)) {
            return {.ok = false, .error = "The source method cannot be positioned relative to itself."};
        }

        std::size_t relative_index = 0U;
        const auto relative_result = find_unique_visual_method_index(
            methods,
            relative_method_name,
            "The relative method was not found.",
            "The relative method name is ambiguous.",
            relative_index);
        if (!relative_result.ok) {
            return relative_result;
        }
        insert_index = normalized_placement == "before" ? relative_index : relative_index + 1U;
    } else {
        return {.ok = false, .error = "Unknown method placement was requested."};
    }

    methods.insert(
        methods.begin() + static_cast<std::ptrdiff_t>(insert_index),
        moving_method);
    updated_blob = serialize_visual_methods(methods);
    return {.ok = true, .error = {}};
}

VisualAssetEditResult find_unique_visual_property_assignment_index(
    const std::vector<VisualPropertyAssignment>& assignments,
    const std::string& property_name,
    const std::string& missing_error,
    const std::string& ambiguous_error,
    std::size_t& property_index) {
    const std::string normalized_property_name = normalize_visual_property_name(property_name);
    std::vector<std::size_t> matches;
    for (std::size_t index = 0U; index < assignments.size(); ++index) {
        if (normalize_visual_property_name(assignments[index].name) == normalized_property_name) {
            matches.push_back(index);
        }
    }
    if (matches.empty()) {
        return {.ok = false, .error = missing_error};
    }
    if (matches.size() > 1U) {
        return {.ok = false, .error = ambiguous_error};
    }
    property_index = matches.front();
    return {.ok = true, .error = {}};
}

VisualAssetEditResult reorder_visual_property_assignments(
    std::vector<VisualPropertyAssignment>& assignments,
    const std::string& requested_property_name,
    const std::string& placement,
    const std::string& relative_property_name) {
    std::size_t source_index = 0U;
    const auto source_result = find_unique_visual_property_assignment_index(
        assignments,
        requested_property_name,
        "The requested property was not found.",
        "The requested property name is ambiguous.",
        source_index);
    if (!source_result.ok) {
        return source_result;
    }

    const std::string normalized_placement = normalize_visual_property_name(placement);
    const auto moving_property = assignments[source_index];
    assignments.erase(assignments.begin() + static_cast<std::ptrdiff_t>(source_index));

    std::size_t insert_index = assignments.size();
    if (normalized_placement == "first") {
        insert_index = 0U;
    } else if (normalized_placement == "last") {
        insert_index = assignments.size();
    } else if (normalized_placement == "before" || normalized_placement == "after") {
        if (trim_both(relative_property_name).empty()) {
            return {.ok = false, .error = "No relative property name was provided."};
        }
        if (normalize_visual_property_name(relative_property_name) == normalize_visual_property_name(requested_property_name)) {
            return {.ok = false, .error = "The source property cannot be positioned relative to itself."};
        }

        std::size_t relative_index = 0U;
        const auto relative_result = find_unique_visual_property_assignment_index(
            assignments,
            relative_property_name,
            "The relative property was not found.",
            "The relative property name is ambiguous.",
            relative_index);
        if (!relative_result.ok) {
            return relative_result;
        }
        insert_index = normalized_placement == "before" ? relative_index : relative_index + 1U;
    } else {
        return {.ok = false, .error = "Unknown property placement was requested."};
    }

    assignments.insert(
        assignments.begin() + static_cast<std::ptrdiff_t>(insert_index),
        moving_property);
    return {.ok = true, .error = {}};
}

std::pair<bool, std::string> delete_visual_method_from_blob(
    const std::string& existing_blob,
    const std::string& requested_method_name) {
    const std::string normalized_requested_name = normalize_visual_object_name(requested_method_name);
    const std::vector<std::string> existing_lines = split_visual_lines(existing_blob);
    std::vector<std::string> output_lines;
    bool deleted = false;
    bool skipping_deleted_body = false;

    for (const auto& raw_line : existing_lines) {
        const std::string trimmed_line = trim_both(raw_line);
        std::string declaration_kind;
        std::string declaration_name;
        if (!skipping_deleted_body &&
            parse_visual_method_declaration(trimmed_line, declaration_kind, declaration_name) &&
            normalize_visual_object_name(declaration_name) == normalized_requested_name) {
            deleted = true;
            skipping_deleted_body = true;
            continue;
        }

        if (skipping_deleted_body) {
            if (is_visual_method_end_line(trimmed_line)) {
                skipping_deleted_body = false;
            }
            continue;
        }

        output_lines.push_back(raw_line);
    }

    return {deleted, serialize_visual_lines(output_lines)};
}

VisualAssetEditResult rename_visual_method_in_blob(
    const std::string& existing_blob,
    const std::string& requested_method_name,
    const std::string& new_method_name,
    std::string& updated_blob) {
    const std::string normalized_requested_name = normalize_visual_object_name(requested_method_name);
    const std::string normalized_new_name = normalize_visual_object_name(new_method_name);
    if (normalized_requested_name.empty() || normalized_new_name.empty()) {
        return {.ok = false, .error = "Method names cannot be empty."};
    }

    std::vector<std::string> existing_lines = split_visual_lines(existing_blob);
    std::vector<std::size_t> source_line_indexes;
    for (std::size_t line_index = 0U; line_index < existing_lines.size(); ++line_index) {
        const std::string trimmed_line = trim_both(existing_lines[line_index]);
        std::string declaration_kind;
        std::string declaration_name;
        if (!parse_visual_method_declaration(trimmed_line, declaration_kind, declaration_name)) {
            continue;
        }
        const std::string normalized_declaration_name = normalize_visual_object_name(declaration_name);
        if (normalized_declaration_name == normalized_new_name &&
            normalized_declaration_name != normalized_requested_name) {
            return {.ok = false, .error = "The requested target method already exists."};
        }
        if (normalized_declaration_name == normalized_requested_name) {
            source_line_indexes.push_back(line_index);
        }
    }

    if (source_line_indexes.empty()) {
        return {.ok = false, .error = "The requested method was not found."};
    }
    if (source_line_indexes.size() > 1U) {
        return {.ok = false, .error = "The requested method name is ambiguous."};
    }

    const std::string trimmed_line = trim_both(existing_lines[source_line_indexes.front()]);
    std::string declaration_kind;
    std::string declaration_name;
    if (!parse_visual_method_declaration(trimmed_line, declaration_kind, declaration_name)) {
        return {.ok = false, .error = "The requested method declaration could not be parsed."};
    }
    existing_lines[source_line_indexes.front()] = declaration_kind + " " + trim_both(new_method_name);
    updated_blob = serialize_visual_lines(existing_lines);
    return {.ok = true, .error = {}};
}

const DbfRecordValue* find_record_value(const DbfRecord& record, const std::string& field_name) {
    const std::string requested_field_name = normalize_visual_property_name(field_name);
    const auto value = std::find_if(record.values.begin(), record.values.end(), [&](const DbfRecordValue& candidate) {
        return normalize_visual_property_name(candidate.field_name) == requested_field_name;
    });
    return value == record.values.end() ? nullptr : &(*value);
}

std::optional<std::size_t> find_field_index(const DbfTable& table, const std::string& field_name) {
    const std::string requested_field_name = normalize_visual_property_name(field_name);
    for (std::size_t index = 0U; index < table.fields.size(); ++index) {
        if (normalize_visual_property_name(table.fields[index].name) == requested_field_name) {
            return index;
        }
    }
    return std::nullopt;
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

VisualObjectDuplicateResult reject_identity_collision(
    const DbfTable& table,
    const std::string& field_name,
    const std::string& requested_value) {
    if (normalize_visual_object_name(requested_value).empty()) {
        return {.ok = true, .error = {}, .record_index = 0U};
    }
    if (!find_field_index(table, field_name).has_value()) {
        return {
            .ok = false,
            .error = "The requested replacement identity field is not present in the asset.",
            .record_index = 0U
        };
    }
    if (!find_matching_record_indexes(table, field_name, normalize_visual_object_name(requested_value)).empty()) {
        return {
            .ok = false,
            .error = "The requested replacement identity already exists in the asset.",
            .record_index = 0U
        };
    }
    return {.ok = true, .error = {}, .record_index = 0U};
}

VisualAssetEditResult reject_identity_collision_excluding_record(
    const DbfTable& table,
    const std::string& field_name,
    const std::string& requested_value,
    std::size_t excluded_record_index) {
    const std::string normalized_value = normalize_visual_object_name(requested_value);
    if (normalized_value.empty()) {
        return {.ok = true, .error = {}};
    }
    if (!find_field_index(table, field_name).has_value()) {
        return {.ok = false, .error = "The requested identity field is not present in the asset."};
    }
    const auto matches = find_matching_record_indexes(table, field_name, normalized_value);
    const auto collision = std::find_if(matches.begin(), matches.end(), [&](std::size_t record_index) {
        return record_index != excluded_record_index;
    });
    if (collision != matches.end()) {
        return {.ok = false, .error = "The requested identity value already exists in the asset."};
    }
    return {.ok = true, .error = {}};
}

void replace_duplicate_field_value(
    const DbfTable& table,
    std::vector<std::string>& values,
    const std::string& field_name,
    const std::string& replacement_value) {
    if (replacement_value.empty()) {
        return;
    }
    const auto field_index = find_field_index(table, field_name);
    if (field_index.has_value() && *field_index < values.size()) {
        values[*field_index] = replacement_value;
    }
}

std::string duplicate_field_value(
    const DbfTable& table,
    const std::vector<std::string>& values,
    const std::string& field_name) {
    const auto field_index = find_field_index(table, field_name);
    if (!field_index.has_value() || *field_index >= values.size()) {
        return {};
    }
    return values[*field_index];
}

const VisualObjectSubtreeDuplicateReplacement* find_subtree_duplicate_replacement(
    const std::vector<VisualObjectSubtreeDuplicateReplacement>& replacements,
    const std::string& source_unique_id) {
    const std::string normalized_source_unique_id = normalize_visual_object_name(source_unique_id);
    const auto replacement = std::find_if(
        replacements.begin(),
        replacements.end(),
        [&](const VisualObjectSubtreeDuplicateReplacement& candidate) {
            return normalize_visual_object_name(candidate.source_unique_id) == normalized_source_unique_id;
        });
    return replacement == replacements.end() ? nullptr : &(*replacement);
}

std::string visual_object_record_name(const DbfRecord& record) {
    const auto* objname = find_record_value(record, "OBJNAME");
    std::string object_name = objname == nullptr ? std::string{} : trim_both(objname->display_value);
    if (!object_name.empty()) {
        return object_name;
    }
    const auto* name = find_record_value(record, "NAME");
    return name == nullptr ? std::string{} : trim_both(name->display_value);
}

std::optional<double> parse_visual_geometry_number(const std::string& text) {
    const std::string trimmed = trim_both(text);
    if (trimmed.empty()) {
        return std::nullopt;
    }

    errno = 0;
    char* parse_end = nullptr;
    const double value = std::strtod(trimmed.c_str(), &parse_end);
    if (parse_end == trimmed.c_str() || parse_end == nullptr || *parse_end != '\0' || errno == ERANGE) {
        return std::nullopt;
    }
    return value;
}

std::string format_visual_geometry_number(double value) {
    const double rounded = std::round(value);
    std::ostringstream stream;
    if (std::abs(value - rounded) < 0.0005) {
        stream << static_cast<long long>(rounded);
        return stream.str();
    }

    stream << std::fixed << std::setprecision(3) << value;
    std::string text = stream.str();
    while (!text.empty() && text.back() == '0') {
        text.pop_back();
    }
    if (!text.empty() && text.back() == '.') {
        text.pop_back();
    }
    return text;
}

VisualAssetEditResult read_visual_object_geometry(
    const std::string& path,
    std::size_t record_index,
    const std::string& object_name,
    const std::string& unique_id,
    VisualObjectGeometry& geometry) {
    const auto read_property = [&](const std::string& property_name, double& output) -> VisualAssetEditResult {
        const auto property_result = query_visual_object_property({
            .path = path,
            .record_index = record_index,
            .object_name = object_name,
            .unique_id = unique_id,
            .property_name = property_name
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = "The selected object does not expose required geometry fields."};
        }
        const auto parsed_value = parse_visual_geometry_number(property_result.value);
        if (!parsed_value.has_value()) {
            return {.ok = false, .error = "The selected object geometry is not numeric."};
        }
        output = *parsed_value;
        return {.ok = true, .error = {}};
    };

    for (const auto& result : {
             read_property("HPOS", geometry.hpos),
             read_property("VPOS", geometry.vpos),
             read_property("WIDTH", geometry.width),
             read_property("HEIGHT", geometry.height)
         }) {
        if (!result.ok) {
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult read_visual_object_geometry_coordinate(
    const std::string& path,
    const VisualObjectAlignmentTarget& object,
    const std::string& property_name,
    double& coordinate) {
    const auto property_result = query_visual_object_property({
        .path = path,
        .record_index = object.record_index,
        .object_name = object.object_name,
        .unique_id = object.unique_id,
        .property_name = property_name
    });
    if (!property_result.ok) {
        return {.ok = false, .error = property_result.error};
    }
    if (!property_result.exists) {
        return {.ok = false, .error = "The selected object does not expose required distribution coordinates."};
    }

    const auto parsed_value = parse_visual_geometry_number(property_result.value);
    if (!parsed_value.has_value()) {
        return {.ok = false, .error = "The selected object distribution coordinate is not numeric."};
    }
    coordinate = *parsed_value;
    return {.ok = true, .error = {}};
}

VisualObjectSnapshot build_visual_object_snapshot(const DbfRecord& record) {
    const auto* unique_id = find_record_value(record, "UNIQUEID");
    const auto* parent_name = find_record_value(record, "PARENT");
    const auto* class_name = find_record_value(record, "CLASS");
    const auto* baseclass_name = find_record_value(record, "BASECLASS");
    const auto* properties = find_record_value(record, "PROPERTIES");

    std::string caption;
    if (properties != nullptr) {
        const auto assignments = parse_visual_property_blob(properties->display_value);
        const auto caption_it = std::find_if(assignments.begin(), assignments.end(), [](const VisualPropertyAssignment& assignment) {
            return normalize_visual_property_name(assignment.name) == "caption";
        });
        if (caption_it != assignments.end()) {
            caption = caption_it->value;
        }
    }

    return {
        .record_index = record.record_index,
        .deleted = record.deleted,
        .object_name = visual_object_record_name(record),
        .unique_id = unique_id == nullptr ? std::string{} : trim_both(unique_id->display_value),
        .parent_name = parent_name == nullptr ? std::string{} : trim_both(parent_name->display_value),
        .class_name = class_name == nullptr ? std::string{} : trim_both(class_name->display_value),
        .baseclass_name = baseclass_name == nullptr ? std::string{} : trim_both(baseclass_name->display_value),
        .caption = caption
    };
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

VisualAssetEditResult resolve_visual_object_record_index_from_records(
    const std::vector<DbfRecord>& records,
    std::size_t requested_record_index,
    const std::string& object_name,
    const std::string& unique_id,
    std::size_t& record_index) {
    const std::string requested_unique_id = normalize_visual_object_name(unique_id);
    if (!requested_unique_id.empty()) {
        std::vector<std::size_t> matches;
        for (std::size_t index = 0U; index < records.size(); ++index) {
            const auto* value = find_record_value(records[index], "UNIQUEID");
            if (value != nullptr && normalize_visual_object_name(value->display_value) == requested_unique_id) {
                matches.push_back(index);
            }
        }
        if (matches.empty()) {
            return {.ok = false, .error = "No visual object with the requested unique id was found."};
        }
        if (matches.size() > 1U) {
            return {.ok = false, .error = "The requested visual object unique id is ambiguous."};
        }

        record_index = matches.front();
        return {.ok = true, .error = {}};
    }

    if (object_name.empty()) {
        record_index = requested_record_index;
        return {.ok = true, .error = {}};
    }

    const std::string requested_name = normalize_visual_object_name(object_name);
    if (requested_name.empty()) {
        return {.ok = false, .error = "No object name was provided."};
    }

    std::vector<std::size_t> matches;
    for (std::size_t index = 0U; index < records.size(); ++index) {
        const auto* value = find_record_value(records[index], "OBJNAME");
        if (value != nullptr && normalize_visual_object_name(value->display_value) == requested_name) {
            matches.push_back(index);
        }
    }
    if (matches.empty()) {
        for (std::size_t index = 0U; index < records.size(); ++index) {
            const auto* value = find_record_value(records[index], "NAME");
            if (value != nullptr && normalize_visual_object_name(value->display_value) == requested_name) {
                matches.push_back(index);
            }
        }
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

VisualAssetEditResult apply_visual_object_reorder_to_records(
    std::vector<DbfRecord>& records,
    const VisualObjectReorderBatchItem& request) {
    const std::string placement = normalize_visual_property_name(request.placement);
    if (placement != "front" && placement != "back" && placement != "before" && placement != "after") {
        return {.ok = false, .error = "Unsupported visual object placement."};
    }

    std::size_t source_record_index = 0U;
    const auto source_resolution = resolve_visual_object_record_index_from_records(
        records,
        request.record_index,
        request.object_name,
        request.unique_id,
        source_record_index);
    if (!source_resolution.ok) {
        return source_resolution;
    }
    if (source_record_index >= records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available."};
    }

    std::size_t target_record_index = 0U;
    if (placement == "before" || placement == "after") {
        if (trim_both(request.target_object_name).empty() && trim_both(request.target_unique_id).empty()) {
            return {.ok = false, .error = "No target object selector was provided."};
        }
        const auto target_resolution = resolve_visual_object_record_index_from_records(
            records,
            0U,
            request.target_object_name,
            request.target_unique_id,
            target_record_index);
        if (!target_resolution.ok) {
            return target_resolution;
        }
        if (target_record_index >= records.size()) {
            return {.ok = false, .error = "The requested target record is not currently available."};
        }
        if (target_record_index == source_record_index) {
            return {.ok = false, .error = "A visual object cannot be reordered relative to itself."};
        }
    }

    std::vector<std::size_t> order;
    order.reserve(records.size());
    for (std::size_t index = 0U; index < records.size(); ++index) {
        if (index != source_record_index) {
            order.push_back(index);
        }
    }

    std::size_t insert_position = 0U;
    if (placement == "back") {
        insert_position = order.size();
    } else if (placement == "before" || placement == "after") {
        const auto target = std::find(order.begin(), order.end(), target_record_index);
        if (target == order.end()) {
            return {.ok = false, .error = "The requested target record is not currently available."};
        }
        insert_position = static_cast<std::size_t>(std::distance(order.begin(), target));
        if (placement == "after") {
            ++insert_position;
        }
    }
    order.insert(order.begin() + static_cast<std::ptrdiff_t>(insert_position), source_record_index);

    std::vector<DbfRecord> reordered_records;
    reordered_records.reserve(records.size());
    for (const auto record_index : order) {
        reordered_records.push_back(records[record_index]);
    }
    records = std::move(reordered_records);
    return {.ok = true, .error = {}};
}

std::vector<std::vector<std::string>> visual_record_values_for_write(
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<DbfRecord>& records) {
    std::vector<std::vector<std::string>> values;
    values.reserve(records.size());
    for (const auto& record : records) {
        std::vector<std::string> record_values;
        record_values.reserve(fields.size());
        for (const auto& field : fields) {
            const auto* value = find_record_value(record, field.name);
            record_values.push_back(value == nullptr ? std::string{} : value->display_value);
        }
        values.push_back(std::move(record_values));
    }
    return values;
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
    const auto* direct_field_value = find_direct_visual_property_value(record.values, property_name);
    if (direct_field_value != nullptr) {
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
    if (trim_both(request.property_name).empty()) {
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
    const auto direct_field_it = find_direct_visual_property_field(fields, request.property_name);
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
    const std::string requested_property_name = normalize_visual_property_name(request.property_name);
    auto assignment_it = std::find_if(assignments.begin(), assignments.end(), [&](const VisualPropertyAssignment& property) {
        return normalize_visual_property_name(property.name) == requested_property_name;
    });

    if (record_undo_entry) {
        const bool exists = assignment_it != assignments.end();
        const std::string prior_value = exists ? assignment_it->value : std::string{};
        if (!exists && remove_property_if_missing) {
            return {.ok = true, .error = {}};
        }
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

VisualAssetEditResult set_visual_object_text_property(
    const std::string& path,
    const std::vector<VisualObjectAlignmentTarget>& objects,
    const std::string& property_name,
    const std::string& property_label,
    const std::string& text) {
    if (path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for " + property_label + " assignment."};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(objects.size());
    for (const auto& object : objects) {
        const auto property_result = query_visual_object_property({
            .path = path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = property_name
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = "The same visual object was selected more than once for " +
                property_label + " assignment."};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = property_name,
                    .property_value = property_result.direct_field
                        ? text
                        : format_visual_string_property_value(text)
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_scalar_property(
    const std::string& path,
    const std::vector<VisualObjectAlignmentTarget>& objects,
    const std::string& property_name,
    const std::string& property_label,
    const std::string& property_value) {
    if (path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for " + property_label + " assignment."};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(objects.size());
    for (const auto& object : objects) {
        const auto property_result = query_visual_object_property({
            .path = path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = property_name
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = "The same visual object was selected more than once for " +
                property_label + " assignment."};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = property_name,
                    .property_value = property_value
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = path,
        .objects = edits
    });
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

VisualAssetEditResult clear_visual_object_property(const VisualObjectPropertyClearRequest& request) {
    return apply_visual_object_property_change({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = request.property_name,
        .property_value = {}
    }, true, true);
}

VisualAssetEditResult clear_visual_object_properties(const VisualObjectPropertyClearBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = "No property clears were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_clears = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& property : request.properties) {
        if (trim_both(property.property_name).empty()) {
            const auto rollback_result = rollback_batch_clears();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No property name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No property name was provided."};
        }

        const auto result = clear_visual_object_property({
            .path = request.path,
            .record_index = property.record_index,
            .object_name = property.object_name,
            .unique_id = property.unique_id,
            .property_name = property.property_name
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_clears();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult copy_visual_object_property(const VisualObjectPropertyCopyRequest& request) {
    if (!request.target_property_name.empty() && trim_both(request.target_property_name).empty()) {
        return {.ok = false, .error = "No target property name was provided."};
    }

    const auto source_property = query_visual_object_property({
        .path = request.path,
        .record_index = request.source_record_index,
        .object_name = request.source_object_name,
        .unique_id = request.source_unique_id,
        .property_name = request.source_property_name
    });
    if (!source_property.ok) {
        return {.ok = false, .error = source_property.error};
    }
    if (!source_property.exists) {
        return {.ok = false, .error = "The source property was not found."};
    }

    const std::string target_property_name = request.target_property_name.empty()
        ? source_property.property_name
        : trim_both(request.target_property_name);
    const auto target_property = query_visual_object_property({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .property_name = target_property_name
    });
    if (!target_property.ok) {
        return {.ok = false, .error = target_property.error};
    }
    if (target_property.exists && !request.replace_existing) {
        return {.ok = false, .error = "The target object already has the requested property."};
    }

    return update_visual_object_property({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .property_name = target_property_name,
        .property_value = source_property.value
    });
}

VisualAssetEditResult copy_visual_object_properties(const VisualObjectPropertyCopyBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = "No property copies were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_copies = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& property : request.properties) {
        if (trim_both(property.source_property_name).empty()) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No property name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No property name was provided."};
        }
        if (!property.target_property_name.empty() && trim_both(property.target_property_name).empty()) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No target property name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No target property name was provided."};
        }

        const auto result = copy_visual_object_property({
            .path = request.path,
            .source_record_index = property.source_record_index,
            .source_object_name = property.source_object_name,
            .source_unique_id = property.source_unique_id,
            .source_property_name = property.source_property_name,
            .target_record_index = property.target_record_index,
            .target_object_name = property.target_object_name,
            .target_unique_id = property.target_unique_id,
            .target_property_name = property.target_property_name,
            .replace_existing = property.replace_existing
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult move_visual_object_property(const VisualObjectPropertyMoveRequest& request) {
    if (!request.target_property_name.empty() && trim_both(request.target_property_name).empty()) {
        return {.ok = false, .error = "No target property name was provided."};
    }

    const auto source_property = query_visual_object_property({
        .path = request.path,
        .record_index = request.source_record_index,
        .object_name = request.source_object_name,
        .unique_id = request.source_unique_id,
        .property_name = request.source_property_name
    });
    if (!source_property.ok) {
        return {.ok = false, .error = source_property.error};
    }
    if (!source_property.exists) {
        return {.ok = false, .error = "The source property was not found."};
    }

    const std::string target_property_name = request.target_property_name.empty()
        ? source_property.property_name
        : trim_both(request.target_property_name);
    const auto target_property = query_visual_object_property({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .property_name = target_property_name
    });
    if (!target_property.ok) {
        return {.ok = false, .error = target_property.error};
    }
    if (target_property.record_index == source_property.record_index &&
        normalize_visual_property_name(target_property_name) == normalize_visual_property_name(source_property.property_name)) {
        return {.ok = false, .error = "The source property cannot be moved onto itself."};
    }
    if (target_property.exists && !request.replace_existing) {
        return {.ok = false, .error = "The target object already has the requested property."};
    }

    const auto copy_result = copy_visual_object_property({
        .path = request.path,
        .source_record_index = request.source_record_index,
        .source_object_name = request.source_object_name,
        .source_unique_id = request.source_unique_id,
        .source_property_name = request.source_property_name,
        .target_record_index = request.target_record_index,
        .target_object_name = request.target_object_name,
        .target_unique_id = request.target_unique_id,
        .target_property_name = request.target_property_name,
        .replace_existing = request.replace_existing
    });
    if (!copy_result.ok) {
        return copy_result;
    }

    const auto clear_result = clear_visual_object_property({
        .path = request.path,
        .record_index = request.source_record_index,
        .object_name = request.source_object_name,
        .unique_id = request.source_unique_id,
        .property_name = request.source_property_name
    });
    if (!clear_result.ok) {
        const auto rollback_result = undo_visual_object_property(request.path);
        if (!rollback_result.ok) {
            return {.ok = false, .error = clear_result.error + " Target rollback failed: " + rollback_result.error};
        }
        return {.ok = false, .error = clear_result.error};
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult move_visual_object_properties(const VisualObjectPropertyMoveBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = "No property moves were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_moves = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& property : request.properties) {
        if (trim_both(property.source_property_name).empty()) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No property name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No property name was provided."};
        }
        if (!property.target_property_name.empty() && trim_both(property.target_property_name).empty()) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No target property name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No target property name was provided."};
        }

        const auto result = move_visual_object_property({
            .path = request.path,
            .source_record_index = property.source_record_index,
            .source_object_name = property.source_object_name,
            .source_unique_id = property.source_unique_id,
            .source_property_name = property.source_property_name,
            .target_record_index = property.target_record_index,
            .target_object_name = property.target_object_name,
            .target_unique_id = property.target_unique_id,
            .target_property_name = property.target_property_name,
            .replace_existing = property.replace_existing
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult rename_visual_object_property(const VisualObjectPropertyRenameRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }

    const std::string source_property_name = trim_both(request.property_name);
    const std::string target_property_name = trim_both(request.new_property_name);
    if (source_property_name.empty()) {
        return {.ok = false, .error = "No property name was provided."};
    }
    if (target_property_name.empty()) {
        return {.ok = false, .error = "No target property name was provided."};
    }
    if (normalize_visual_property_name(source_property_name) == normalize_visual_property_name(target_property_name)) {
        return {.ok = false, .error = "The source property cannot be renamed to itself."};
    }

    std::size_t record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = source_property_name,
        .property_value = {}
    }, record_index);
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

    const std::string normalized_source = normalize_visual_property_name(source_property_name);
    const auto fields = read_raw_field_descriptors(table_bytes);
    const auto direct_field_it = std::find_if(fields.begin(), fields.end(), [&](const RawFieldDescriptor& field) {
        return normalize_visual_property_name(field.name) == normalized_source;
    });
    if (direct_field_it != fields.end()) {
        return {.ok = false, .error = "Direct DBF-backed fields cannot be renamed per object."};
    }

    if (!is_property_blob_asset_path(request.path)) {
        return {.ok = false, .error = "The requested property is not exposed as a renameable memo-backed property on this asset."};
    }

    const auto& record = table_result.table.records[record_index];
    const auto properties_it = std::find_if(record.values.begin(), record.values.end(), [](const DbfRecordValue& value) {
        return value.field_name == "PROPERTIES";
    });
    if (properties_it == record.values.end()) {
        return {.ok = false, .error = "The selected object does not expose a PROPERTIES memo field."};
    }

    auto assignments = parse_visual_property_blob(properties_it->display_value);
    const std::string normalized_target = normalize_visual_property_name(target_property_name);
    std::size_t source_count = 0U;
    std::size_t source_index = 0U;
    bool target_exists = false;
    for (std::size_t index = 0U; index < assignments.size(); ++index) {
        const std::string normalized_name = normalize_visual_property_name(assignments[index].name);
        if (normalized_name == normalized_source) {
            ++source_count;
            source_index = index;
        }
        if (normalized_name == normalized_target) {
            target_exists = true;
        }
    }

    if (source_count == 0U) {
        return {.ok = false, .error = "The source property was not found."};
    }
    if (source_count > 1U) {
        return {.ok = false, .error = "The source property is ambiguous in the selected object."};
    }
    if (target_exists) {
        return {.ok = false, .error = "The target property already exists in the selected object."};
    }

    std::string error;
    if (!record_visual_asset_undo_entry(request.path, {
            .record_index = record_index,
            .property_name = "PROPERTIES",
            .prior_value = properties_it->display_value,
            .prior_value_exists = true,
            .label = "Rename property " + source_property_name
        }, error)) {
        return {.ok = false, .error = error};
    }

    assignments[source_index].name = target_property_name;
    return replace_memo_field_value(
        request.path,
        record_index,
        "PROPERTIES",
        serialize_visual_property_blob(assignments));
}

VisualAssetEditResult rename_visual_object_properties(const VisualObjectPropertyRenameBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = "No property renames were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_renames = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& property : request.properties) {
        if (trim_both(property.property_name).empty()) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No property name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No property name was provided."};
        }
        if (trim_both(property.new_property_name).empty()) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No target property name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No target property name was provided."};
        }

        const auto result = rename_visual_object_property({
            .path = request.path,
            .record_index = property.record_index,
            .object_name = property.object_name,
            .unique_id = property.unique_id,
            .property_name = property.property_name,
            .new_property_name = property.new_property_name
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult reorder_visual_object_property(const VisualObjectPropertyReorderRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (trim_both(request.property_name).empty()) {
        return {.ok = false, .error = "No property name was provided."};
    }

    const std::string placement = normalize_visual_property_name(request.placement);
    if (placement != "first" && placement != "last" && placement != "before" && placement != "after") {
        return {.ok = false, .error = "Unknown property placement was requested."};
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
    const auto is_direct_field_name = [&](const std::string& property_name) {
        const std::string normalized_property_name = normalize_visual_property_name(property_name);
        return std::any_of(fields.begin(), fields.end(), [&](const RawFieldDescriptor& field) {
            return normalize_visual_property_name(field.name) == normalized_property_name;
        });
    };
    if (is_direct_field_name(request.property_name)) {
        return {.ok = false, .error = "Direct DBF-backed fields cannot be reordered per object."};
    }
    if ((placement == "before" || placement == "after") &&
        !trim_both(request.relative_property_name).empty() &&
        is_direct_field_name(request.relative_property_name)) {
        return {.ok = false, .error = "Direct DBF-backed fields cannot be reordered per object."};
    }

    if (!is_property_blob_asset_path(request.path)) {
        return {.ok = false, .error = "The requested property is not exposed as a reorderable memo-backed property on this asset."};
    }

    const auto* properties_field = find_record_value(table_result.table.records[record_index], "PROPERTIES");
    if (properties_field == nullptr) {
        return {.ok = false, .error = "The selected object does not expose a PROPERTIES memo field."};
    }

    auto assignments = parse_visual_property_blob(properties_field->display_value);
    const auto reorder_result = reorder_visual_property_assignments(
        assignments,
        request.property_name,
        request.placement,
        request.relative_property_name);
    if (!reorder_result.ok) {
        return reorder_result;
    }

    return update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "PROPERTIES",
        .property_value = serialize_visual_property_blob(assignments)
    });
}

VisualAssetEditResult reorder_visual_object_properties(const VisualObjectPropertyReorderBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = "No property reorders were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_reorders = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& property : request.properties) {
        if (trim_both(property.property_name).empty()) {
            const auto rollback_result = rollback_batch_reorders();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No property name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No property name was provided."};
        }

        const std::string placement = normalize_visual_property_name(property.placement);
        if ((placement == "before" || placement == "after") &&
            trim_both(property.relative_property_name).empty()) {
            const auto rollback_result = rollback_batch_reorders();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No relative property name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No relative property name was provided."};
        }

        const auto result = reorder_visual_object_property({
            .path = request.path,
            .record_index = property.record_index,
            .object_name = property.object_name,
            .unique_id = property.unique_id,
            .property_name = property.property_name,
            .placement = property.placement,
            .relative_property_name = property.relative_property_name
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_reorders();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
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
    if (trim_both(request.property_name).empty()) {
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

VisualObjectListResult list_visual_objects(const std::string& path) {
    if (path.empty()) {
        return {
            .ok = false,
            .error = "No asset path was provided.",
            .objects = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .objects = {}
        };
    }

    std::vector<VisualObjectSnapshot> objects;
    objects.reserve(table_result.table.records.size());
    for (const auto& record : table_result.table.records) {
        objects.push_back(build_visual_object_snapshot(record));
    }

    return {
        .ok = true,
        .error = {},
        .objects = std::move(objects)
    };
}

VisualObjectChildrenListResult list_visual_object_children(const VisualObjectChildrenListRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = "No asset path was provided.",
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }

    std::size_t parent_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, parent_record_index);
    if (!resolution.ok) {
        return {
            .ok = false,
            .error = resolution.error,
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }
    if (parent_record_index >= table_result.table.records.size()) {
        return {
            .ok = false,
            .error = "The requested parent record is not currently available.",
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }

    const std::string parent_name = visual_object_record_name(table_result.table.records[parent_record_index]);
    if (parent_name.empty()) {
        return {
            .ok = false,
            .error = "The selected parent does not expose an object name.",
            .parent_record_index = 0U,
            .parent_name = {},
            .children = {}
        };
    }

    std::vector<VisualObjectSnapshot> children;
    const std::string normalized_parent_name = normalize_visual_object_name(parent_name);
    for (const auto& record : table_result.table.records) {
        const auto* record_parent = find_record_value(record, "PARENT");
        if (record_parent == nullptr) {
            continue;
        }
        if (normalize_visual_object_name(record_parent->display_value) == normalized_parent_name) {
            children.push_back(build_visual_object_snapshot(record));
        }
    }

    return {
        .ok = true,
        .error = {},
        .parent_record_index = parent_record_index,
        .parent_name = parent_name,
        .children = std::move(children)
    };
}

VisualObjectDescendantsListResult list_visual_object_descendants(const VisualObjectDescendantsListRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = "No asset path was provided.",
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }

    std::size_t parent_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, parent_record_index);
    if (!resolution.ok) {
        return {
            .ok = false,
            .error = resolution.error,
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }
    if (parent_record_index >= table_result.table.records.size()) {
        return {
            .ok = false,
            .error = "The requested parent record is not currently available.",
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }

    const auto& table = table_result.table;
    const std::string parent_name = visual_object_record_name(table.records[parent_record_index]);
    if (parent_name.empty()) {
        return {
            .ok = false,
            .error = "The selected parent does not expose an object name.",
            .parent_record_index = 0U,
            .parent_name = {},
            .descendants = {}
        };
    }

    std::vector<VisualObjectDescendantSnapshot> descendants;
    std::vector<bool> visited(table.records.size(), false);
    visited[parent_record_index] = true;

    std::function<void(const std::string&, std::size_t)> append_descendants =
        [&](const std::string& current_parent_name, std::size_t depth) {
            const std::string normalized_parent_name = normalize_visual_object_name(current_parent_name);
            if (normalized_parent_name.empty()) {
                return;
            }

            for (std::size_t record_index = 0U; record_index < table.records.size(); ++record_index) {
                if (visited[record_index]) {
                    continue;
                }
                const auto* record_parent = find_record_value(table.records[record_index], "PARENT");
                if (record_parent == nullptr ||
                    normalize_visual_object_name(record_parent->display_value) != normalized_parent_name) {
                    continue;
                }

                visited[record_index] = true;
                VisualObjectSnapshot snapshot = build_visual_object_snapshot(table.records[record_index]);
                descendants.push_back({
                    .object = snapshot,
                    .depth = depth
                });
                append_descendants(snapshot.object_name, depth + 1U);
            }
        };

    append_descendants(parent_name, 1U);

    return {
        .ok = true,
        .error = {},
        .parent_record_index = parent_record_index,
        .parent_name = parent_name,
        .descendants = std::move(descendants)
    };
}

VisualObjectAncestorsListResult list_visual_object_ancestors(const VisualObjectAncestorsListRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = "No asset path was provided.",
            .record_index = 0U,
            .ancestors = {}
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
            .ancestors = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .record_index = 0U,
            .ancestors = {}
        };
    }
    const auto& table = table_result.table;
    if (record_index >= table.records.size()) {
        return {
            .ok = false,
            .error = "The requested object record is not currently available.",
            .record_index = 0U,
            .ancestors = {}
        };
    }

    std::vector<VisualObjectAncestorSnapshot> ancestors;
    std::vector<bool> visited(table.records.size(), false);
    std::size_t current_record_index = record_index;
    visited[current_record_index] = true;

    for (std::size_t depth = 1U; depth <= table.records.size(); ++depth) {
        const auto* parent_value = find_record_value(table.records[current_record_index], "PARENT");
        const std::string parent_name = parent_value == nullptr ? std::string{} : trim_both(parent_value->display_value);
        if (parent_name.empty()) {
            break;
        }

        std::vector<std::size_t> parent_matches;
        const std::string normalized_parent_name = normalize_visual_object_name(parent_name);
        for (const auto& record : table.records) {
            if (normalize_visual_object_name(visual_object_record_name(record)) == normalized_parent_name) {
                parent_matches.push_back(record.record_index);
            }
        }
        if (parent_matches.empty()) {
            break;
        }
        if (parent_matches.size() > 1U) {
            return {
                .ok = false,
                .error = "The selected object's parent name is ambiguous.",
                .record_index = 0U,
                .ancestors = {}
            };
        }
        const std::size_t parent_record_index = parent_matches.front();
        if (parent_record_index >= visited.size() || visited[parent_record_index]) {
            return {
                .ok = false,
                .error = "The selected object's parent chain contains a cycle.",
                .record_index = 0U,
                .ancestors = {}
            };
        }

        visited[parent_record_index] = true;
        ancestors.push_back({
            .object = build_visual_object_snapshot(table.records[parent_record_index]),
            .depth = depth
        });
        current_record_index = parent_record_index;
    }

    return {
        .ok = true,
        .error = {},
        .record_index = record_index,
        .ancestors = std::move(ancestors)
    };
}

VisualObjectMethodListResult list_visual_object_methods(const VisualObjectMethodListRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = "No asset path was provided.",
            .record_index = 0U,
            .record_deleted = false,
            .methods = {}
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
            .methods = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .record_index = 0U,
            .record_deleted = false,
            .methods = {}
        };
    }
    if (record_index >= table_result.table.records.size()) {
        return {
            .ok = false,
            .error = "The requested object record is not currently available.",
            .record_index = 0U,
            .record_deleted = false,
            .methods = {}
        };
    }

    const auto& record = table_result.table.records[record_index];
    const auto* methods_field = find_record_value(record, "METHODS");
    std::vector<VisualObjectMethodSnapshot> methods;
    if (methods_field != nullptr && !trim_both(methods_field->display_value).empty()) {
        methods = parse_visual_methods_blob(
            methods_field->display_value,
            methods_field->memo_block_number);
    }

    return {
        .ok = true,
        .error = {},
        .record_index = record_index,
        .record_deleted = record.deleted,
        .methods = std::move(methods)
    };
}

VisualObjectMethodQueryResult query_visual_object_method(const VisualObjectMethodQueryRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = "No asset path was provided.",
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
        };
    }
    if (trim_both(request.method_name).empty()) {
        return {
            .ok = false,
            .error = "No method name was provided.",
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
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
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
        };
    }

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {
            .ok = false,
            .error = table_result.error,
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
        };
    }
    if (record_index >= table_result.table.records.size()) {
        return {
            .ok = false,
            .error = "The requested object record is not currently available.",
            .exists = false,
            .record_index = 0U,
            .record_deleted = false,
            .method = {}
        };
    }

    const auto& record = table_result.table.records[record_index];
    const auto* methods_field = find_record_value(record, "METHODS");
    if (methods_field == nullptr) {
        return {
            .ok = false,
            .error = "The selected object does not expose a METHODS memo field.",
            .exists = false,
            .record_index = record_index,
            .record_deleted = record.deleted,
            .method = {}
        };
    }

    const std::vector<VisualObjectMethodSnapshot> methods = parse_visual_methods_blob(
        methods_field->display_value,
        methods_field->memo_block_number);
    const std::string normalized_method_name = normalize_visual_object_name(request.method_name);
    std::vector<VisualObjectMethodSnapshot> matches;
    for (const auto& method : methods) {
        if (normalize_visual_object_name(method.method_name) == normalized_method_name) {
            matches.push_back(method);
        }
    }
    if (matches.size() > 1U) {
        return {
            .ok = false,
            .error = "The requested method name is ambiguous.",
            .exists = false,
            .record_index = record_index,
            .record_deleted = record.deleted,
            .method = {}
        };
    }
    if (matches.empty()) {
        return {
            .ok = true,
            .error = {},
            .exists = false,
            .record_index = record_index,
            .record_deleted = record.deleted,
            .method = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .exists = true,
        .record_index = record_index,
        .record_deleted = record.deleted,
        .method = matches.front()
    };
}

VisualAssetEditResult update_visual_object_method(const VisualObjectMethodEditRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (trim_both(request.method_name).empty()) {
        return {.ok = false, .error = "No method name was provided."};
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

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    if (record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available."};
    }

    const auto* methods_field = find_record_value(table_result.table.records[record_index], "METHODS");
    if (methods_field == nullptr) {
        return {.ok = false, .error = "The selected object does not expose a METHODS memo field."};
    }

    const std::string updated_blob = update_visual_methods_blob(
        methods_field->display_value,
        request.method_name,
        request.method_kind,
        request.source_text);

    return update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "METHODS",
        .property_value = updated_blob
    });
}

VisualAssetEditResult delete_visual_object_method(const VisualObjectMethodDeleteRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (trim_both(request.method_name).empty()) {
        return {.ok = false, .error = "No method name was provided."};
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

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    if (record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available."};
    }

    const auto* methods_field = find_record_value(table_result.table.records[record_index], "METHODS");
    if (methods_field == nullptr) {
        return {.ok = false, .error = "The selected object does not expose a METHODS memo field."};
    }

    const std::string normalized_method_name = normalize_visual_object_name(request.method_name);
    const auto methods = parse_visual_methods_blob(methods_field->display_value, 0U);
    const auto matching_count = std::count_if(methods.begin(), methods.end(), [&](const VisualObjectMethodSnapshot& method) {
        return normalize_visual_object_name(method.method_name) == normalized_method_name;
    });
    if (matching_count == 0) {
        return {.ok = false, .error = "The requested method was not found."};
    }
    if (matching_count > 1) {
        return {.ok = false, .error = "The requested method name is ambiguous."};
    }

    const auto [deleted, updated_blob] = delete_visual_method_from_blob(
        methods_field->display_value,
        request.method_name);
    if (!deleted) {
        return {.ok = false, .error = "The requested method was not found."};
    }

    return update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "METHODS",
        .property_value = updated_blob
    });
}

VisualAssetEditResult delete_visual_object_methods(const VisualObjectMethodDeleteBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = "No method deletes were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_deletes = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.method_name).empty()) {
            const auto rollback_result = rollback_batch_deletes();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No method name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No method name was provided."};
        }

        const auto result = delete_visual_object_method({
            .path = request.path,
            .record_index = method.record_index,
            .object_name = method.object_name,
            .unique_id = method.unique_id,
            .method_name = method.method_name
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_deletes();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult rename_visual_object_method(const VisualObjectMethodRenameRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (trim_both(request.method_name).empty()) {
        return {.ok = false, .error = "No method name was provided."};
    }
    if (trim_both(request.new_method_name).empty()) {
        return {.ok = false, .error = "No target method name was provided."};
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

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    if (record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available."};
    }

    const auto* methods_field = find_record_value(table_result.table.records[record_index], "METHODS");
    if (methods_field == nullptr) {
        return {.ok = false, .error = "The selected object does not expose a METHODS memo field."};
    }

    std::string updated_blob;
    const auto rename_result = rename_visual_method_in_blob(
        methods_field->display_value,
        request.method_name,
        request.new_method_name,
        updated_blob);
    if (!rename_result.ok) {
        return rename_result;
    }

    return update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "METHODS",
        .property_value = updated_blob
    });
}

VisualAssetEditResult rename_visual_object_methods(const VisualObjectMethodRenameBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = "No method renames were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_renames = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.method_name).empty()) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No method name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No method name was provided."};
        }
        if (trim_both(method.new_method_name).empty()) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No target method name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No target method name was provided."};
        }

        const auto result = rename_visual_object_method({
            .path = request.path,
            .record_index = method.record_index,
            .object_name = method.object_name,
            .unique_id = method.unique_id,
            .method_name = method.method_name,
            .new_method_name = method.new_method_name
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult copy_visual_object_method(const VisualObjectMethodCopyRequest& request) {
    if (!request.target_method_name.empty() && trim_both(request.target_method_name).empty()) {
        return {.ok = false, .error = "No target method name was provided."};
    }

    const auto source_method = query_visual_object_method({
        .path = request.path,
        .record_index = request.source_record_index,
        .object_name = request.source_object_name,
        .unique_id = request.source_unique_id,
        .method_name = request.source_method_name
    });
    if (!source_method.ok) {
        return {.ok = false, .error = source_method.error};
    }
    if (!source_method.exists) {
        return {.ok = false, .error = "The source method was not found."};
    }

    const std::string target_method_name = request.target_method_name.empty()
        ? source_method.method.method_name
        : trim_both(request.target_method_name);
    const auto target_method = query_visual_object_method({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .method_name = target_method_name
    });
    if (!target_method.ok) {
        return {.ok = false, .error = target_method.error};
    }
    if (target_method.exists && !request.replace_existing) {
        return {.ok = false, .error = "The target object already has a method with the requested name."};
    }

    return update_visual_object_method({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .method_name = target_method_name,
        .method_kind = target_method.exists ? target_method.method.kind : source_method.method.kind,
        .source_text = source_method.method.source_text
    });
}

VisualAssetEditResult copy_visual_object_methods(const VisualObjectMethodCopyBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = "No method copies were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_copies = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.source_method_name).empty()) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No method name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No method name was provided."};
        }
        if (!method.target_method_name.empty() && trim_both(method.target_method_name).empty()) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No target method name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No target method name was provided."};
        }

        const auto result = copy_visual_object_method({
            .path = request.path,
            .source_record_index = method.source_record_index,
            .source_object_name = method.source_object_name,
            .source_unique_id = method.source_unique_id,
            .source_method_name = method.source_method_name,
            .target_record_index = method.target_record_index,
            .target_object_name = method.target_object_name,
            .target_unique_id = method.target_unique_id,
            .target_method_name = method.target_method_name,
            .replace_existing = method.replace_existing
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult move_visual_object_method(const VisualObjectMethodMoveRequest& request) {
    if (!request.target_method_name.empty() && trim_both(request.target_method_name).empty()) {
        return {.ok = false, .error = "No target method name was provided."};
    }

    const auto source_method = query_visual_object_method({
        .path = request.path,
        .record_index = request.source_record_index,
        .object_name = request.source_object_name,
        .unique_id = request.source_unique_id,
        .method_name = request.source_method_name
    });
    if (!source_method.ok) {
        return {.ok = false, .error = source_method.error};
    }
    if (!source_method.exists) {
        return {.ok = false, .error = "The source method was not found."};
    }

    const std::string target_method_name = request.target_method_name.empty()
        ? source_method.method.method_name
        : trim_both(request.target_method_name);
    const auto target_method = query_visual_object_method({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .method_name = target_method_name
    });
    if (!target_method.ok) {
        return {.ok = false, .error = target_method.error};
    }
    if (target_method.record_index == source_method.record_index &&
        normalize_visual_object_name(target_method_name) == normalize_visual_object_name(source_method.method.method_name)) {
        return {.ok = false, .error = "The source method cannot be moved onto itself."};
    }
    if (target_method.exists && !request.replace_existing) {
        return {.ok = false, .error = "The target object already has a method with the requested name."};
    }

    const auto copy_result = copy_visual_object_method({
        .path = request.path,
        .source_record_index = request.source_record_index,
        .source_object_name = request.source_object_name,
        .source_unique_id = request.source_unique_id,
        .source_method_name = request.source_method_name,
        .target_record_index = request.target_record_index,
        .target_object_name = request.target_object_name,
        .target_unique_id = request.target_unique_id,
        .target_method_name = request.target_method_name,
        .replace_existing = request.replace_existing
    });
    if (!copy_result.ok) {
        return copy_result;
    }

    const auto delete_result = delete_visual_object_method({
        .path = request.path,
        .record_index = request.source_record_index,
        .object_name = request.source_object_name,
        .unique_id = request.source_unique_id,
        .method_name = request.source_method_name
    });
    if (!delete_result.ok) {
        const auto rollback_result = undo_visual_object_property(request.path);
        if (!rollback_result.ok) {
            return {.ok = false, .error = delete_result.error + " Target rollback failed: " + rollback_result.error};
        }
        return {.ok = false, .error = delete_result.error};
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult move_visual_object_methods(const VisualObjectMethodMoveBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = "No method moves were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_moves = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.source_method_name).empty()) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No method name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No method name was provided."};
        }
        if (!method.target_method_name.empty() && trim_both(method.target_method_name).empty()) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No target method name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No target method name was provided."};
        }

        const auto result = move_visual_object_method({
            .path = request.path,
            .source_record_index = method.source_record_index,
            .source_object_name = method.source_object_name,
            .source_unique_id = method.source_unique_id,
            .source_method_name = method.source_method_name,
            .target_record_index = method.target_record_index,
            .target_object_name = method.target_object_name,
            .target_unique_id = method.target_unique_id,
            .target_method_name = method.target_method_name,
            .replace_existing = method.replace_existing
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult reorder_visual_object_method(const VisualObjectMethodReorderRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (trim_both(request.method_name).empty()) {
        return {.ok = false, .error = "No method name was provided."};
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

    const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    if (record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available."};
    }

    const auto* methods_field = find_record_value(table_result.table.records[record_index], "METHODS");
    if (methods_field == nullptr) {
        return {.ok = false, .error = "The selected object does not expose a METHODS memo field."};
    }

    std::string updated_blob;
    const auto reorder_result = reorder_visual_methods_blob(
        methods_field->display_value,
        request.method_name,
        request.placement,
        request.relative_method_name,
        updated_blob);
    if (!reorder_result.ok) {
        return reorder_result;
    }

    return update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "METHODS",
        .property_value = updated_blob
    });
}

VisualAssetEditResult reorder_visual_object_methods(const VisualObjectMethodReorderBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.methods.empty()) {
        return {.ok = false, .error = "No method reorders were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_reorders = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& method : request.methods) {
        if (trim_both(method.method_name).empty()) {
            const auto rollback_result = rollback_batch_reorders();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = "No method name was provided. Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = "No method name was provided."};
        }

        const auto result = reorder_visual_object_method({
            .path = request.path,
            .record_index = method.record_index,
            .object_name = method.object_name,
            .unique_id = method.unique_id,
            .method_name = method.method_name,
            .placement = method.placement,
            .relative_method_name = method.relative_method_name
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_reorders();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualObjectDuplicateResult duplicate_visual_object(const VisualObjectDuplicateRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided.", .record_index = 0U};
    }

    std::size_t source_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, source_record_index);
    if (!resolution.ok) {
        return {.ok = false, .error = resolution.error, .record_index = 0U};
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error, .record_index = 0U};
    }
    const auto& table = table_result.table;
    if (source_record_index >= table.records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available.", .record_index = 0U};
    }

    const auto reject_missing_replacement_field = [&](const std::string& field_name, const std::string& value) -> VisualObjectDuplicateResult {
        if (value.empty() || find_field_index(table, field_name).has_value()) {
            return {.ok = true, .error = {}, .record_index = 0U};
        }
        return {
            .ok = false,
            .error = "The requested replacement identity field is not present in the asset.",
            .record_index = 0U
        };
    };
    for (const auto& check : {
             reject_missing_replacement_field("OBJNAME", request.new_object_name),
             reject_missing_replacement_field("NAME", request.new_name),
             reject_missing_replacement_field("UNIQUEID", request.new_unique_id)
         }) {
        if (!check.ok) {
            return check;
        }
    }

    std::vector<std::vector<std::string>> records;
    records.reserve(table.records.size() + 1U);
    std::vector<bool> deleted_flags;
    deleted_flags.reserve(table.records.size());
    for (const auto& record : table.records) {
        std::vector<std::string> values;
        values.reserve(table.fields.size());
        for (const auto& field : table.fields) {
            const auto* value = find_record_value(record, field.name);
            values.push_back(value == nullptr ? std::string{} : value->display_value);
        }
        records.push_back(std::move(values));
        deleted_flags.push_back(record.deleted);
    }

    std::vector<std::string> duplicate_values = records[source_record_index];
    replace_duplicate_field_value(table, duplicate_values, "OBJNAME", request.new_object_name);
    replace_duplicate_field_value(table, duplicate_values, "NAME", request.new_name);
    replace_duplicate_field_value(table, duplicate_values, "UNIQUEID", request.new_unique_id);

    for (const auto& identity_field : {"OBJNAME", "NAME", "UNIQUEID"}) {
        const std::string final_value = duplicate_field_value(table, duplicate_values, identity_field);
        if (normalize_visual_object_name(final_value).empty()) {
            continue;
        }
        const auto collision = reject_identity_collision(table, identity_field, final_value);
        if (!collision.ok) {
            return collision;
        }
    }

    const std::size_t duplicate_record_index = records.size();
    records.push_back(std::move(duplicate_values));

    const auto create_result = create_dbf_table_file(request.path, table.fields, records);
    if (!create_result.ok) {
        return {.ok = false, .error = create_result.error, .record_index = 0U};
    }
    for (std::size_t index = 0U; index < deleted_flags.size(); ++index) {
        if (!deleted_flags[index]) {
            continue;
        }
        const auto delete_result = set_record_deleted_flag(request.path, index, true);
        if (!delete_result.ok) {
            return {.ok = false, .error = delete_result.error, .record_index = 0U};
        }
    }

    return {.ok = true, .error = {}, .record_index = duplicate_record_index};
}

VisualObjectDuplicateBatchResult duplicate_visual_objects(const VisualObjectDuplicateBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided.", .record_indexes = {}};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual object duplicates were provided.", .record_indexes = {}};
    }

    const std::vector<std::uint8_t> original_table_bytes = read_binary_file(request.path);
    if (original_table_bytes.empty()) {
        return {.ok = false, .error = "Unable to open the visual asset table.", .record_indexes = {}};
    }
    const std::string memo_path = infer_memo_sidecar_path(request.path);
    const std::vector<std::uint8_t> original_memo_bytes = memo_path.empty()
        ? std::vector<std::uint8_t>{}
        : read_binary_file(memo_path);

    const auto restore_original_asset = [&]() {
        write_binary_file(request.path, original_table_bytes);
        if (!memo_path.empty() && !original_memo_bytes.empty()) {
            write_binary_file(memo_path, original_memo_bytes);
        }
    };

    std::vector<std::size_t> duplicated_record_indexes;
    duplicated_record_indexes.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto duplicate_result = duplicate_visual_object({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .new_object_name = object.new_object_name,
            .new_name = object.new_name,
            .new_unique_id = object.new_unique_id
        });
        if (!duplicate_result.ok) {
            restore_original_asset();
            return {.ok = false, .error = duplicate_result.error, .record_indexes = {}};
        }
        duplicated_record_indexes.push_back(duplicate_result.record_index);
    }

    return {.ok = true, .error = {}, .record_indexes = duplicated_record_indexes};
}

VisualObjectSubtreeDuplicateResult duplicate_visual_object_subtree(const VisualObjectSubtreeDuplicateRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided.", .root_record_index = 0U, .copied_count = 0U};
    }
    if (request.replacements.empty()) {
        return {.ok = false, .error = "No subtree replacement identities were provided.", .root_record_index = 0U, .copied_count = 0U};
    }

    std::size_t root_record_index = 0U;
    const auto root_resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, root_record_index);
    if (!root_resolution.ok) {
        return {.ok = false, .error = root_resolution.error, .root_record_index = 0U, .copied_count = 0U};
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error, .root_record_index = 0U, .copied_count = 0U};
    }
    const auto& table = table_result.table;
    if (root_record_index >= table.records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available.", .root_record_index = 0U, .copied_count = 0U};
    }

    const auto require_field = [&](const std::string& field_name) -> VisualObjectSubtreeDuplicateResult {
        if (find_field_index(table, field_name).has_value()) {
            return {.ok = true, .error = {}, .root_record_index = 0U, .copied_count = 0U};
        }
        return {
            .ok = false,
            .error = "The requested replacement identity field is not present in the asset.",
            .root_record_index = 0U,
            .copied_count = 0U
        };
    };
    for (const auto& check : {require_field("OBJNAME"), require_field("NAME"), require_field("UNIQUEID"), require_field("PARENT")}) {
        if (!check.ok) {
            return check;
        }
    }

    const auto descendants_result = list_visual_object_descendants({
        .path = request.path,
        .record_index = root_record_index,
        .object_name = {},
        .unique_id = {}
    });
    if (!descendants_result.ok) {
        return {.ok = false, .error = descendants_result.error, .root_record_index = 0U, .copied_count = 0U};
    }

    std::vector<std::size_t> copy_record_indexes;
    copy_record_indexes.reserve(descendants_result.descendants.size() + 1U);
    copy_record_indexes.push_back(root_record_index);
    for (const auto& descendant : descendants_result.descendants) {
        copy_record_indexes.push_back(descendant.object.record_index);
    }

    struct CopyPlan {
        std::size_t record_index = 0;
        std::string source_unique_id;
        const VisualObjectSubtreeDuplicateReplacement* replacement = nullptr;
        std::string original_object_name;
        std::string original_parent_name;
        std::string copied_parent_name;
    };
    std::vector<CopyPlan> copy_plan;
    copy_plan.reserve(copy_record_indexes.size());

    const auto unique_in_replacements = [&](const std::string& source_unique_id) {
        const std::string normalized_source_unique_id = normalize_visual_object_name(source_unique_id);
        return std::count_if(
            request.replacements.begin(),
            request.replacements.end(),
            [&](const VisualObjectSubtreeDuplicateReplacement& replacement) {
                return normalize_visual_object_name(replacement.source_unique_id) == normalized_source_unique_id;
            });
    };

    for (const auto record_index : copy_record_indexes) {
        const auto* unique_id = find_record_value(table.records[record_index], "UNIQUEID");
        const std::string source_unique_id = unique_id == nullptr ? std::string{} : trim_both(unique_id->display_value);
        if (source_unique_id.empty()) {
            return {.ok = false, .error = "Every copied row must expose a UNIQUEID.", .root_record_index = 0U, .copied_count = 0U};
        }
        if (unique_in_replacements(source_unique_id) != 1) {
            return {.ok = false, .error = "Missing or ambiguous subtree replacement identity.", .root_record_index = 0U, .copied_count = 0U};
        }

        const auto* replacement = find_subtree_duplicate_replacement(request.replacements, source_unique_id);
        if (replacement == nullptr ||
            trim_both(replacement->new_object_name).empty() ||
            trim_both(replacement->new_name).empty() ||
            trim_both(replacement->new_unique_id).empty()) {
            return {.ok = false, .error = "Missing subtree replacement identity data.", .root_record_index = 0U, .copied_count = 0U};
        }

        const auto* parent = find_record_value(table.records[record_index], "PARENT");
        copy_plan.push_back({
            .record_index = record_index,
            .source_unique_id = source_unique_id,
            .replacement = replacement,
            .original_object_name = visual_object_record_name(table.records[record_index]),
            .original_parent_name = parent == nullptr ? std::string{} : trim_both(parent->display_value),
            .copied_parent_name = parent == nullptr ? std::string{} : trim_both(parent->display_value)
        });
    }

    for (auto& plan : copy_plan) {
        const std::string normalized_parent_name = normalize_visual_object_name(plan.original_parent_name);
        if (normalized_parent_name.empty()) {
            continue;
        }
        const auto parent_plan = std::find_if(copy_plan.begin(), copy_plan.end(), [&](const CopyPlan& candidate) {
            return normalize_visual_object_name(candidate.original_object_name) == normalized_parent_name;
        });
        if (parent_plan != copy_plan.end()) {
            plan.copied_parent_name = parent_plan->replacement->new_object_name;
        }
    }

    const auto reject_duplicate_identity_values = [&](const std::string& field_name, const std::vector<std::string>& values) -> VisualObjectSubtreeDuplicateResult {
        std::vector<std::string> normalized_values;
        for (const auto& value : values) {
            const std::string normalized_value = normalize_visual_object_name(value);
            if (normalized_value.empty()) {
                continue;
            }
            if (!find_matching_record_indexes(table, field_name, normalized_value).empty()) {
                return {
                    .ok = false,
                    .error = "The requested replacement identity already exists in the asset.",
                    .root_record_index = 0U,
                    .copied_count = 0U
                };
            }
            if (std::find(normalized_values.begin(), normalized_values.end(), normalized_value) != normalized_values.end()) {
                return {
                    .ok = false,
                    .error = "The requested replacement identity is duplicated within the copied subtree.",
                    .root_record_index = 0U,
                    .copied_count = 0U
                };
            }
            normalized_values.push_back(normalized_value);
        }
        return {.ok = true, .error = {}, .root_record_index = 0U, .copied_count = 0U};
    };

    std::vector<std::string> new_objnames;
    std::vector<std::string> new_names;
    std::vector<std::string> new_unique_ids;
    new_objnames.reserve(copy_plan.size());
    new_names.reserve(copy_plan.size());
    new_unique_ids.reserve(copy_plan.size());
    for (const auto& plan : copy_plan) {
        new_objnames.push_back(plan.replacement->new_object_name);
        new_names.push_back(plan.replacement->new_name);
        new_unique_ids.push_back(plan.replacement->new_unique_id);
    }
    for (const auto& check : {
             reject_duplicate_identity_values("OBJNAME", new_objnames),
             reject_duplicate_identity_values("NAME", new_names),
             reject_duplicate_identity_values("UNIQUEID", new_unique_ids)
         }) {
        if (!check.ok) {
            return check;
        }
    }

    std::vector<std::vector<std::string>> records;
    records.reserve(table.records.size() + copy_plan.size());
    std::vector<bool> deleted_flags;
    deleted_flags.reserve(table.records.size() + copy_plan.size());
    for (const auto& record : table.records) {
        std::vector<std::string> values;
        values.reserve(table.fields.size());
        for (const auto& field : table.fields) {
            const auto* value = find_record_value(record, field.name);
            values.push_back(value == nullptr ? std::string{} : value->display_value);
        }
        records.push_back(std::move(values));
        deleted_flags.push_back(record.deleted);
    }

    const std::size_t copied_root_record_index = records.size();
    for (const auto& plan : copy_plan) {
        std::vector<std::string> values = records[plan.record_index];
        replace_duplicate_field_value(table, values, "OBJNAME", plan.replacement->new_object_name);
        replace_duplicate_field_value(table, values, "NAME", plan.replacement->new_name);
        replace_duplicate_field_value(table, values, "UNIQUEID", plan.replacement->new_unique_id);
        replace_duplicate_field_value(table, values, "PARENT", plan.copied_parent_name);
        records.push_back(std::move(values));
        deleted_flags.push_back(table.records[plan.record_index].deleted);
    }

    const auto create_result = create_dbf_table_file(request.path, table.fields, records);
    if (!create_result.ok) {
        return {.ok = false, .error = create_result.error, .root_record_index = 0U, .copied_count = 0U};
    }
    for (std::size_t index = 0U; index < deleted_flags.size(); ++index) {
        if (!deleted_flags[index]) {
            continue;
        }
        const auto delete_result = set_record_deleted_flag(request.path, index, true);
        if (!delete_result.ok) {
            return {.ok = false, .error = delete_result.error, .root_record_index = 0U, .copied_count = 0U};
        }
    }

    return {
        .ok = true,
        .error = {},
        .root_record_index = copied_root_record_index,
        .copied_count = copy_plan.size()
    };
}

VisualObjectCreateResult create_visual_object(const VisualObjectCreateRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided.", .record_index = 0U};
    }
    if (request.field_values.empty()) {
        return {.ok = false, .error = "No field values were provided.", .record_index = 0U};
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error, .record_index = 0U};
    }
    const auto& table = table_result.table;

    std::vector<std::string> created_values(table.fields.size());
    for (const auto& field_value : request.field_values) {
        if (trim_both(field_value.property_name).empty()) {
            return {.ok = false, .error = "Field names cannot be empty.", .record_index = 0U};
        }
        const auto field_index = find_field_index(table, field_value.property_name);
        if (!field_index.has_value()) {
            return {.ok = false, .error = "The requested field was not found in the asset.", .record_index = 0U};
        }
        created_values[*field_index] = field_value.property_value;
    }

    for (const auto& identity_field : {"OBJNAME", "NAME", "UNIQUEID"}) {
        const std::string final_value = duplicate_field_value(table, created_values, identity_field);
        if (normalize_visual_object_name(final_value).empty()) {
            continue;
        }
        const auto collision = reject_identity_collision(table, identity_field, final_value);
        if (!collision.ok) {
            return {.ok = false, .error = collision.error, .record_index = 0U};
        }
    }

    std::vector<std::vector<std::string>> records;
    records.reserve(table.records.size() + 1U);
    std::vector<bool> deleted_flags;
    deleted_flags.reserve(table.records.size());
    for (const auto& record : table.records) {
        std::vector<std::string> values;
        values.reserve(table.fields.size());
        for (const auto& field : table.fields) {
            const auto* value = find_record_value(record, field.name);
            values.push_back(value == nullptr ? std::string{} : value->display_value);
        }
        records.push_back(std::move(values));
        deleted_flags.push_back(record.deleted);
    }

    const std::size_t created_record_index = records.size();
    records.push_back(std::move(created_values));

    const auto create_result = create_dbf_table_file(request.path, table.fields, records);
    if (!create_result.ok) {
        return {.ok = false, .error = create_result.error, .record_index = 0U};
    }
    for (std::size_t index = 0U; index < deleted_flags.size(); ++index) {
        if (!deleted_flags[index]) {
            continue;
        }
        const auto delete_result = set_record_deleted_flag(request.path, index, true);
        if (!delete_result.ok) {
            return {.ok = false, .error = delete_result.error, .record_index = 0U};
        }
    }

    return {.ok = true, .error = {}, .record_index = created_record_index};
}

VisualObjectCreateBatchResult create_visual_objects(const VisualObjectCreateBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided.", .record_indexes = {}};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual object creates were provided.", .record_indexes = {}};
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error, .record_indexes = {}};
    }
    const auto& table = table_result.table;

    std::vector<std::vector<std::string>> records = visual_record_values_for_write(table.fields, table.records);
    std::vector<bool> deleted_flags;
    deleted_flags.reserve(table.records.size());
    for (const auto& record : table.records) {
        deleted_flags.push_back(record.deleted);
    }

    std::vector<std::size_t> created_record_indexes;
    created_record_indexes.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        if (object.field_values.empty()) {
            return {.ok = false, .error = "No field values were provided.", .record_indexes = {}};
        }

        std::vector<std::string> created_values(table.fields.size());
        for (const auto& field_value : object.field_values) {
            if (trim_both(field_value.property_name).empty()) {
                return {.ok = false, .error = "Field names cannot be empty.", .record_indexes = {}};
            }
            const auto field_index = find_field_index(table, field_value.property_name);
            if (!field_index.has_value()) {
                return {.ok = false, .error = "The requested field was not found in the asset.", .record_indexes = {}};
            }
            created_values[*field_index] = field_value.property_value;
        }

        for (const auto& identity_field : {"OBJNAME", "NAME", "UNIQUEID"}) {
            const auto identity_field_index = find_field_index(table, identity_field);
            if (!identity_field_index.has_value() || *identity_field_index >= created_values.size()) {
                continue;
            }
            const std::string final_value = created_values[*identity_field_index];
            const std::string normalized_final_value = normalize_visual_object_name(final_value);
            if (normalized_final_value.empty()) {
                continue;
            }
            const auto collision = std::find_if(records.begin(), records.end(), [&](const std::vector<std::string>& record_values) {
                if (*identity_field_index >= record_values.size()) {
                    return false;
                }
                return normalize_visual_object_name(record_values[*identity_field_index]) == normalized_final_value;
            });
            if (collision != records.end()) {
                return {
                    .ok = false,
                    .error = "The requested replacement identity already exists in the asset.",
                    .record_indexes = {}
                };
            }
        }

        created_record_indexes.push_back(records.size());
        records.push_back(std::move(created_values));
    }

    const std::vector<std::uint8_t> original_table_bytes = read_binary_file(request.path);
    if (original_table_bytes.empty()) {
        return {.ok = false, .error = "Unable to open the visual asset table.", .record_indexes = {}};
    }
    const std::string memo_path = infer_memo_sidecar_path(request.path);
    const std::vector<std::uint8_t> original_memo_bytes = memo_path.empty()
        ? std::vector<std::uint8_t>{}
        : read_binary_file(memo_path);

    const auto create_result = create_dbf_table_file(request.path, table.fields, records);
    if (!create_result.ok) {
        return {.ok = false, .error = create_result.error, .record_indexes = {}};
    }
    for (std::size_t index = 0U; index < deleted_flags.size(); ++index) {
        if (!deleted_flags[index]) {
            continue;
        }
        const auto delete_result = set_record_deleted_flag(request.path, index, true);
        if (!delete_result.ok) {
            write_binary_file(request.path, original_table_bytes);
            if (!memo_path.empty() && !original_memo_bytes.empty()) {
                write_binary_file(memo_path, original_memo_bytes);
            }
            return {.ok = false, .error = delete_result.error, .record_indexes = {}};
        }
    }

    return {.ok = true, .error = {}, .record_indexes = created_record_indexes};
}

VisualAssetEditResult align_visual_objects(const VisualObjectAlignmentRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual object alignment targets were provided."};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    if (mode != "left" &&
        mode != "right" &&
        mode != "top" &&
        mode != "bottom" &&
        mode != "horizontal-center" &&
        mode != "vertical-center") {
        return {.ok = false, .error = "Unsupported visual object alignment mode."};
    }

    VisualObjectGeometry anchor_geometry;
    const auto anchor_result = read_visual_object_geometry(
        request.path,
        request.anchor_record_index,
        request.anchor_object_name,
        request.anchor_unique_id,
        anchor_geometry);
    if (!anchor_result.ok) {
        return anchor_result;
    }

    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        VisualObjectGeometry object_geometry;
        const auto object_result = read_visual_object_geometry(
            request.path,
            object.record_index,
            object.object_name,
            object.unique_id,
            object_geometry);
        if (!object_result.ok) {
            return object_result;
        }

        std::string property_name;
        double aligned_value = 0.0;
        if (mode == "left") {
            property_name = "HPOS";
            aligned_value = anchor_geometry.hpos;
        } else if (mode == "right") {
            property_name = "HPOS";
            aligned_value = anchor_geometry.hpos + anchor_geometry.width - object_geometry.width;
        } else if (mode == "top") {
            property_name = "VPOS";
            aligned_value = anchor_geometry.vpos;
        } else if (mode == "bottom") {
            property_name = "VPOS";
            aligned_value = anchor_geometry.vpos + anchor_geometry.height - object_geometry.height;
        } else if (mode == "horizontal-center") {
            property_name = "HPOS";
            aligned_value = anchor_geometry.hpos + ((anchor_geometry.width - object_geometry.width) / 2.0);
        } else {
            property_name = "VPOS";
            aligned_value = anchor_geometry.vpos + ((anchor_geometry.height - object_geometry.height) / 2.0);
        }

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {.property_name = property_name, .property_value = format_visual_geometry_number(aligned_value)}
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult resize_visual_objects(const VisualObjectResizeRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual object resize targets were provided."};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    if (mode != "width" && mode != "height" && mode != "size") {
        return {.ok = false, .error = "Unsupported visual object resize mode."};
    }

    VisualObjectGeometry anchor_geometry;
    const auto anchor_result = read_visual_object_geometry(
        request.path,
        request.anchor_record_index,
        request.anchor_object_name,
        request.anchor_unique_id,
        anchor_geometry);
    if (!anchor_result.ok) {
        return anchor_result;
    }

    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        VisualObjectGeometry object_geometry;
        const auto object_result = read_visual_object_geometry(
            request.path,
            object.record_index,
            object.object_name,
            object.unique_id,
            object_geometry);
        if (!object_result.ok) {
            return object_result;
        }

        std::vector<VisualObjectPropertyChange> properties;
        if (mode == "width" || mode == "size") {
            properties.push_back({
                .property_name = "WIDTH",
                .property_value = format_visual_geometry_number(anchor_geometry.width)
            });
        }
        if (mode == "height" || mode == "size") {
            properties.push_back({
                .property_name = "HEIGHT",
                .property_value = format_visual_geometry_number(anchor_geometry.height)
            });
        }

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = std::move(properties)
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualObjectGroupResult group_visual_objects(const VisualObjectGroupRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided.", .container_record_index = 0U};
    }
    if (request.container_field_values.empty()) {
        return {.ok = false, .error = "No group container field values were provided.", .container_record_index = 0U};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for grouping.", .container_record_index = 0U};
    }

    const std::vector<std::uint8_t> original_table_bytes = read_binary_file(request.path);
    if (original_table_bytes.empty()) {
        return {.ok = false, .error = "Unable to open the visual asset table.", .container_record_index = 0U};
    }
    const std::string memo_path = infer_memo_sidecar_path(request.path);
    const std::vector<std::uint8_t> original_memo_bytes = memo_path.empty()
        ? std::vector<std::uint8_t>{}
        : read_binary_file(memo_path);

    const auto restore_original_asset = [&]() {
        write_binary_file(request.path, original_table_bytes);
        if (!memo_path.empty() && !original_memo_bytes.empty()) {
            write_binary_file(memo_path, original_memo_bytes);
        }
    };

    const auto create_result = create_visual_object({
        .path = request.path,
        .field_values = request.container_field_values
    });
    if (!create_result.ok) {
        restore_original_asset();
        return {.ok = false, .error = create_result.error, .container_record_index = 0U};
    }

    const auto table_result = parse_dbf_table_from_file(request.path, create_result.record_index + 1U);
    if (!table_result.ok || create_result.record_index >= table_result.table.records.size()) {
        restore_original_asset();
        return {.ok = false, .error = table_result.ok ? "The created group container is not available." : table_result.error, .container_record_index = 0U};
    }

    const std::string container_name = visual_object_record_name(table_result.table.records[create_result.record_index]);
    if (container_name.empty()) {
        restore_original_asset();
        return {.ok = false, .error = "The group container does not expose an object name.", .container_record_index = 0U};
    }

    std::vector<VisualObjectReparentBatchItem> reparent_items;
    reparent_items.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        reparent_items.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .parent_object_name = container_name,
            .parent_unique_id = {},
            .clear_parent = false
        });
    }

    const auto reparent_result = reparent_visual_objects({
        .path = request.path,
        .objects = reparent_items
    });
    if (!reparent_result.ok) {
        restore_original_asset();
        return {.ok = false, .error = reparent_result.error, .container_record_index = 0U};
    }

    return {.ok = true, .error = {}, .container_record_index = create_result.record_index};
}

VisualObjectUngroupResult ungroup_visual_object(const VisualObjectUngroupRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided.", .container_record_index = 0U, .child_count = 0U};
    }

    std::size_t container_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, container_record_index);
    if (!resolution.ok) {
        return {.ok = false, .error = resolution.error, .container_record_index = 0U, .child_count = 0U};
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error, .container_record_index = 0U, .child_count = 0U};
    }
    if (container_record_index >= table_result.table.records.size()) {
        return {.ok = false, .error = "The requested container record is not currently available.", .container_record_index = 0U, .child_count = 0U};
    }

    const std::string container_name = visual_object_record_name(table_result.table.records[container_record_index]);
    if (container_name.empty()) {
        return {.ok = false, .error = "The selected container does not expose an object name.", .container_record_index = 0U, .child_count = 0U};
    }
    const auto* parent_value = find_record_value(table_result.table.records[container_record_index], "PARENT");
    const std::string container_parent_name = parent_value == nullptr ? std::string{} : trim_both(parent_value->display_value);

    const auto children_result = list_visual_object_children({
        .path = request.path,
        .record_index = container_record_index,
        .object_name = {},
        .unique_id = {}
    });
    if (!children_result.ok) {
        return {.ok = false, .error = children_result.error, .container_record_index = 0U, .child_count = 0U};
    }
    if (children_result.children.empty()) {
        return {.ok = false, .error = "The selected container has no child objects to ungroup.", .container_record_index = 0U, .child_count = 0U};
    }

    const std::vector<std::uint8_t> original_table_bytes = read_binary_file(request.path);
    if (original_table_bytes.empty()) {
        return {.ok = false, .error = "Unable to open the visual asset table.", .container_record_index = 0U, .child_count = 0U};
    }
    const std::string memo_path = infer_memo_sidecar_path(request.path);
    const std::vector<std::uint8_t> original_memo_bytes = memo_path.empty()
        ? std::vector<std::uint8_t>{}
        : read_binary_file(memo_path);

    const auto restore_original_asset = [&]() {
        write_binary_file(request.path, original_table_bytes);
        if (!memo_path.empty() && !original_memo_bytes.empty()) {
            write_binary_file(memo_path, original_memo_bytes);
        }
    };

    std::vector<VisualObjectReparentBatchItem> reparent_items;
    reparent_items.reserve(children_result.children.size());
    for (const auto& child : children_result.children) {
        reparent_items.push_back({
            .record_index = child.record_index,
            .object_name = {},
            .unique_id = {},
            .parent_object_name = container_parent_name,
            .parent_unique_id = {},
            .clear_parent = container_parent_name.empty()
        });
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_reparents = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    const auto reparent_result = reparent_visual_objects({
        .path = request.path,
        .objects = reparent_items
    });
    if (!reparent_result.ok) {
        restore_original_asset();
        return {.ok = false, .error = reparent_result.error, .container_record_index = 0U, .child_count = 0U};
    }

    const auto delete_result = set_visual_object_deleted_state({
        .path = request.path,
        .record_index = container_record_index,
        .object_name = {},
        .unique_id = {},
        .deleted = true
    });
    if (!delete_result.ok) {
        const auto rollback_result = rollback_reparents();
        restore_original_asset();
        if (!rollback_result.ok) {
            return {
                .ok = false,
                .error = delete_result.error + " Rollback failed: " + rollback_result.error,
                .container_record_index = 0U,
                .child_count = 0U
            };
        }
        return {.ok = false, .error = delete_result.error, .container_record_index = 0U, .child_count = 0U};
    }

    return {
        .ok = true,
        .error = {},
        .container_record_index = container_record_index,
        .child_count = children_result.children.size()
    };
}

VisualAssetEditResult distribute_visual_objects(const VisualObjectDistributeRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.size() < 3U) {
        return {.ok = false, .error = "At least three visual objects are required for distribution."};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    std::string property_name;
    if (mode == "horizontal") {
        property_name = "HPOS";
    } else if (mode == "vertical") {
        property_name = "VPOS";
    } else {
        return {.ok = false, .error = "Unsupported visual object distribution mode."};
    }

    struct DistributionItem {
        VisualObjectAlignmentTarget object;
        double coordinate = 0.0;
        std::size_t original_index = 0;
    };
    std::vector<DistributionItem> items;
    items.reserve(request.objects.size());
    for (std::size_t index = 0U; index < request.objects.size(); ++index) {
        double coordinate = 0.0;
        const auto coordinate_result = read_visual_object_geometry_coordinate(
            request.path,
            request.objects[index],
            property_name,
            coordinate);
        if (!coordinate_result.ok) {
            return coordinate_result;
        }
        items.push_back({
            .object = request.objects[index],
            .coordinate = coordinate,
            .original_index = index
        });
    }

    std::stable_sort(items.begin(), items.end(), [](const DistributionItem& left, const DistributionItem& right) {
        if (left.coordinate == right.coordinate) {
            return left.original_index < right.original_index;
        }
        return left.coordinate < right.coordinate;
    });

    const double first_coordinate = items.front().coordinate;
    const double last_coordinate = items.back().coordinate;
    if (std::abs(last_coordinate - first_coordinate) < 0.0005) {
        return {.ok = false, .error = "Distribution endpoints must have distinct coordinates."};
    }

    const double step = (last_coordinate - first_coordinate) / static_cast<double>(items.size() - 1U);
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(items.size() - 2U);
    for (std::size_t index = 1U; index + 1U < items.size(); ++index) {
        const double distributed_coordinate = first_coordinate + (step * static_cast<double>(index));
        edits.push_back({
            .record_index = items[index].object.record_index,
            .object_name = items[index].object.object_name,
            .unique_id = items[index].object.unique_id,
            .properties = {
                {
                    .property_name = property_name,
                    .property_value = format_visual_geometry_number(distributed_coordinate)
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult snap_visual_objects_to_grid(const VisualObjectSnapToGridRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for grid snapping."};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    const bool snap_horizontal = mode == "horizontal" || mode == "both";
    const bool snap_vertical = mode == "vertical" || mode == "both";
    if (!snap_horizontal && !snap_vertical) {
        return {.ok = false, .error = "Unsupported visual object grid snapping mode."};
    }
    if (snap_horizontal && request.grid_width <= 0.0) {
        return {.ok = false, .error = "Grid width must be positive for horizontal snapping."};
    }
    if (snap_vertical && request.grid_height <= 0.0) {
        return {.ok = false, .error = "Grid height must be positive for vertical snapping."};
    }

    const auto snap_coordinate = [](double coordinate, double grid_size) {
        return std::round(coordinate / grid_size) * grid_size;
    };

    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        std::vector<VisualObjectPropertyChange> properties;
        properties.reserve((snap_horizontal ? 1U : 0U) + (snap_vertical ? 1U : 0U));
        if (snap_horizontal) {
            double coordinate = 0.0;
            const auto coordinate_result = read_visual_object_geometry_coordinate(
                request.path,
                object,
                "HPOS",
                coordinate);
            if (!coordinate_result.ok) {
                return coordinate_result;
            }
            properties.push_back({
                .property_name = "HPOS",
                .property_value = format_visual_geometry_number(snap_coordinate(coordinate, request.grid_width))
            });
        }
        if (snap_vertical) {
            double coordinate = 0.0;
            const auto coordinate_result = read_visual_object_geometry_coordinate(
                request.path,
                object,
                "VPOS",
                coordinate);
            if (!coordinate_result.ok) {
                return coordinate_result;
            }
            properties.push_back({
                .property_name = "VPOS",
                .property_value = format_visual_geometry_number(snap_coordinate(coordinate, request.grid_height))
            });
        }
        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = std::move(properties)
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult nudge_visual_objects(const VisualObjectNudgeRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for nudging."};
    }

    const std::string mode = normalize_visual_property_name(request.mode);
    const bool nudge_horizontal = mode == "horizontal" || mode == "both";
    const bool nudge_vertical = mode == "vertical" || mode == "both";
    if (!nudge_horizontal && !nudge_vertical) {
        return {.ok = false, .error = "Unsupported visual object nudge mode."};
    }
    if (nudge_horizontal && std::abs(request.delta_hpos) < 0.0000001) {
        return {.ok = false, .error = "Horizontal nudge delta must be non-zero."};
    }
    if (nudge_vertical && std::abs(request.delta_vpos) < 0.0000001) {
        return {.ok = false, .error = "Vertical nudge delta must be non-zero."};
    }

    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        std::vector<VisualObjectPropertyChange> properties;
        properties.reserve((nudge_horizontal ? 1U : 0U) + (nudge_vertical ? 1U : 0U));
        if (nudge_horizontal) {
            double coordinate = 0.0;
            const auto coordinate_result = read_visual_object_geometry_coordinate(
                request.path,
                object,
                "HPOS",
                coordinate);
            if (!coordinate_result.ok) {
                return coordinate_result;
            }
            properties.push_back({
                .property_name = "HPOS",
                .property_value = format_visual_geometry_number(coordinate + request.delta_hpos)
            });
        }
        if (nudge_vertical) {
            double coordinate = 0.0;
            const auto coordinate_result = read_visual_object_geometry_coordinate(
                request.path,
                object,
                "VPOS",
                coordinate);
            if (!coordinate_result.ok) {
                return coordinate_result;
            }
            properties.push_back({
                .property_name = "VPOS",
                .property_value = format_visual_geometry_number(coordinate + request.delta_vpos)
            });
        }
        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = std::move(properties)
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_tab_order(const VisualObjectTabOrderRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for tab-order assignment."};
    }
    if (request.starting_tab_index < 0) {
        return {.ok = false, .error = "Starting tab index must not be negative."};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (std::size_t index = 0U; index < request.objects.size(); ++index) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = request.objects[index].record_index,
            .object_name = request.objects[index].object_name,
            .unique_id = request.objects[index].unique_id,
            .property_name = "TABINDEX"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = "The selected object does not expose a TABINDEX field."};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = "The same visual object was selected more than once for tab-order assignment."};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = request.objects[index].record_index,
            .object_name = request.objects[index].object_name,
            .unique_id = request.objects[index].unique_id,
            .properties = {
                {
                    .property_name = "TABINDEX",
                    .property_value = std::to_string(request.starting_tab_index + static_cast<int>(index))
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_tab_stop(const VisualObjectTabStopRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for tab-stop assignment."};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "TABSTOP"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = "The selected object does not expose a TABSTOP field or property."};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = "The same visual object was selected more than once for tab-stop assignment."};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "TABSTOP",
                    .property_value = request.tab_stop ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_visibility(const VisualObjectVisibilityRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for visibility assignment."};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "VISIBLE"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = "The selected object does not expose a VISIBLE field or property."};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = "The same visual object was selected more than once for visibility assignment."};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "VISIBLE",
                    .property_value = request.visible ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_enabled(const VisualObjectEnabledRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for enabled-state assignment."};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "ENABLED"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = "The selected object does not expose an ENABLED field or property."};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = "The same visual object was selected more than once for enabled-state assignment."};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "ENABLED",
                    .property_value = request.enabled ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_read_only(const VisualObjectReadOnlyRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for read-only assignment."};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "READONLY"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = "The selected object does not expose a READONLY field or property."};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = "The same visual object was selected more than once for read-only assignment."};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "READONLY",
                    .property_value = request.read_only ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_locked(const VisualObjectLockedRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual objects were selected for locked-state assignment."};
    }

    std::vector<std::size_t> resolved_record_indexes;
    resolved_record_indexes.reserve(request.objects.size());
    std::vector<VisualObjectBatchEditItem> edits;
    edits.reserve(request.objects.size());
    for (const auto& object : request.objects) {
        const auto property_result = query_visual_object_property({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = "LOCKED"
        });
        if (!property_result.ok) {
            return {.ok = false, .error = property_result.error};
        }
        if (!property_result.exists) {
            return {.ok = false, .error = "The selected object does not expose a LOCKED field or property."};
        }
        if (std::find(resolved_record_indexes.begin(), resolved_record_indexes.end(), property_result.record_index) !=
            resolved_record_indexes.end()) {
            return {.ok = false, .error = "The same visual object was selected more than once for locked-state assignment."};
        }
        resolved_record_indexes.push_back(property_result.record_index);

        edits.push_back({
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = {
                {
                    .property_name = "LOCKED",
                    .property_value = request.locked ? ".T." : ".F."
                }
            }
        });
    }

    return update_visual_object_batch({
        .path = request.path,
        .objects = edits
    });
}

VisualAssetEditResult set_visual_object_caption(const VisualObjectCaptionRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "Caption",
        "caption",
        request.caption);
}

VisualAssetEditResult set_visual_object_tooltip_text(const VisualObjectToolTipTextRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "ToolTipText",
        "tooltip text",
        request.tooltip_text);
}

VisualAssetEditResult set_visual_object_status_bar_text(const VisualObjectStatusBarTextRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "StatusBarText",
        "status-bar text",
        request.status_bar_text);
}

VisualAssetEditResult set_visual_object_control_source(const VisualObjectControlSourceRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "ControlSource",
        "control-source",
        request.control_source);
}

VisualAssetEditResult set_visual_object_current_control(const VisualObjectCurrentControlRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "CurrentControl",
        "current control",
        request.current_control);
}

VisualAssetEditResult set_visual_object_closable(const VisualObjectClosableRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Closable",
        "closable",
        request.closable ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_control_box(const VisualObjectControlBoxRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ControlBox",
        "control-box",
        request.control_box ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_max_button(const VisualObjectMaxButtonRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MaxButton",
        "max-button",
        request.max_button ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_hide_selection(const VisualObjectHideSelectionRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HideSelection",
        "hide-selection",
        request.hide_selection ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_allow_cell_selection(const VisualObjectAllowCellSelectionRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AllowCellSelection",
        "allow-cell-selection",
        request.allow_cell_selection ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_sparse(const VisualObjectSparseRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Sparse",
        "sparse",
        request.sparse ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_add_line_feeds(const VisualObjectAddLineFeedsRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AddLineFeeds",
        "add-line-feeds",
        request.add_line_feeds ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_always_on_top(const VisualObjectAlwaysOnTopRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AlwaysOnTop",
        "always-on-top",
        request.always_on_top ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_always_on_bottom(const VisualObjectAlwaysOnBottomRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "AlwaysOnBottom",
        "always-on-bottom",
        request.always_on_bottom ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_picture(const VisualObjectPictureRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "Picture",
        "picture",
        request.picture);
}

VisualAssetEditResult set_visual_object_down_picture(const VisualObjectDownPictureRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "DownPicture",
        "down-picture",
        request.down_picture);
}

VisualAssetEditResult set_visual_object_disabled_picture(const VisualObjectDisabledPictureRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "DisabledPicture",
        "disabled-picture",
        request.disabled_picture);
}

VisualAssetEditResult set_visual_object_ole_drag_picture(const VisualObjectOleDragPictureRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "OLEDragPicture",
        "OLE drag-picture",
        request.ole_drag_picture);
}

VisualAssetEditResult set_visual_object_mouse_icon(const VisualObjectMouseIconRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "MouseIcon",
        "mouse-icon",
        request.mouse_icon);
}

VisualAssetEditResult set_visual_object_drag_icon(const VisualObjectDragIconRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "DragIcon",
        "drag-icon",
        request.drag_icon);
}

VisualAssetEditResult set_visual_object_drag_mode(const VisualObjectDragModeRequest& request) {
    if (request.drag_mode < 0) {
        return {.ok = false, .error = "DragMode must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DragMode",
        "drag-mode",
        std::to_string(request.drag_mode));
}

VisualAssetEditResult set_visual_object_ole_drag_mode(const VisualObjectOleDragModeRequest& request) {
    if (request.ole_drag_mode < 0) {
        return {.ok = false, .error = "OLEDragMode must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "OLEDragMode",
        "OLE drag-mode",
        std::to_string(request.ole_drag_mode));
}

VisualAssetEditResult set_visual_object_ole_drop_mode(const VisualObjectOleDropModeRequest& request) {
    if (request.ole_drop_mode < 0) {
        return {.ok = false, .error = "OLEDropMode must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "OLEDropMode",
        "OLE drop-mode",
        std::to_string(request.ole_drop_mode));
}

VisualAssetEditResult set_visual_object_ole_drop_effects(const VisualObjectOleDropEffectsRequest& request) {
    if (request.ole_drop_effects < 0) {
        return {.ok = false, .error = "OLEDropEffects must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "OLEDropEffects",
        "OLE drop-effects",
        std::to_string(request.ole_drop_effects));
}

VisualAssetEditResult set_visual_object_ole_drop_text_insertion(
    const VisualObjectOleDropTextInsertionRequest& request) {
    if (request.ole_drop_text_insertion < 0) {
        return {.ok = false, .error = "OLEDropTextInsertion must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "OLEDropTextInsertion",
        "OLE drop text-insertion",
        std::to_string(request.ole_drop_text_insertion));
}

VisualAssetEditResult set_visual_object_back_style(const VisualObjectBackStyleRequest& request) {
    if (request.back_style < 0) {
        return {.ok = false, .error = "BackStyle must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BackStyle",
        "back-style",
        std::to_string(request.back_style));
}

VisualAssetEditResult set_visual_object_border_style(const VisualObjectBorderStyleRequest& request) {
    if (request.border_style < 0) {
        return {.ok = false, .error = "BorderStyle must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BorderStyle",
        "border-style",
        std::to_string(request.border_style));
}

VisualAssetEditResult set_visual_object_border_width(const VisualObjectBorderWidthRequest& request) {
    if (request.border_width < 0) {
        return {.ok = false, .error = "BorderWidth must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BorderWidth",
        "border-width",
        std::to_string(request.border_width));
}

VisualAssetEditResult set_visual_object_border_color(const VisualObjectBorderColorRequest& request) {
    if (request.border_color < 0) {
        return {.ok = false, .error = "BorderColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BorderColor",
        "border-color",
        std::to_string(request.border_color));
}

VisualAssetEditResult set_visual_object_special_effect(const VisualObjectSpecialEffectRequest& request) {
    if (request.special_effect < 0) {
        return {.ok = false, .error = "SpecialEffect must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SpecialEffect",
        "special-effect",
        std::to_string(request.special_effect));
}

VisualAssetEditResult set_visual_object_curvature(const VisualObjectCurvatureRequest& request) {
    if (request.curvature < 0) {
        return {.ok = false, .error = "Curvature must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Curvature",
        "curvature",
        std::to_string(request.curvature));
}

VisualAssetEditResult set_visual_object_draw_mode(const VisualObjectDrawModeRequest& request) {
    if (request.draw_mode < 0) {
        return {.ok = false, .error = "DrawMode must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DrawMode",
        "draw-mode",
        std::to_string(request.draw_mode));
}

VisualAssetEditResult set_visual_object_draw_style(const VisualObjectDrawStyleRequest& request) {
    if (request.draw_style < 0) {
        return {.ok = false, .error = "DrawStyle must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DrawStyle",
        "draw-style",
        std::to_string(request.draw_style));
}

VisualAssetEditResult set_visual_object_draw_width(const VisualObjectDrawWidthRequest& request) {
    if (request.draw_width < 0) {
        return {.ok = false, .error = "DrawWidth must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DrawWidth",
        "draw-width",
        std::to_string(request.draw_width));
}

VisualAssetEditResult set_visual_object_fill_style(const VisualObjectFillStyleRequest& request) {
    if (request.fill_style < 0) {
        return {.ok = false, .error = "FillStyle must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FillStyle",
        "fill-style",
        std::to_string(request.fill_style));
}

VisualAssetEditResult set_visual_object_mouse_pointer(const VisualObjectMousePointerRequest& request) {
    if (request.mouse_pointer < 0) {
        return {.ok = false, .error = "MousePointer must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MousePointer",
        "mouse-pointer",
        std::to_string(request.mouse_pointer));
}

VisualAssetEditResult set_visual_object_picture_margin(const VisualObjectPictureMarginRequest& request) {
    if (request.picture_margin < 0) {
        return {.ok = false, .error = "PictureMargin must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "PictureMargin",
        "picture-margin",
        std::to_string(request.picture_margin));
}

VisualAssetEditResult set_visual_object_picture_position(const VisualObjectPicturePositionRequest& request) {
    if (request.picture_position < 0) {
        return {.ok = false, .error = "PicturePosition must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "PicturePosition",
        "picture-position",
        std::to_string(request.picture_position));
}

VisualAssetEditResult set_visual_object_picture_spacing(const VisualObjectPictureSpacingRequest& request) {
    if (request.picture_spacing < 0) {
        return {.ok = false, .error = "PictureSpacing must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "PictureSpacing",
        "picture-spacing",
        std::to_string(request.picture_spacing));
}

VisualAssetEditResult set_visual_object_picture_selection_display(
    const VisualObjectPictureSelectionDisplayRequest& request) {
    if (request.picture_selection_display < 0) {
        return {.ok = false, .error = "PictureSelectionDisplay must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "PictureSelectionDisplay",
        "picture-selection-display",
        std::to_string(request.picture_selection_display));
}

VisualAssetEditResult set_visual_object_input_mask(const VisualObjectInputMaskRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "InputMask",
        "input-mask",
        request.input_mask);
}

VisualAssetEditResult set_visual_object_dynamic_input_mask(const VisualObjectDynamicInputMaskRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicInputMask",
        "dynamic input mask",
        request.dynamic_input_mask);
}

VisualAssetEditResult set_visual_object_dynamic_line_height(const VisualObjectDynamicLineHeightRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicLineHeight",
        "dynamic line height",
        request.dynamic_line_height);
}

VisualAssetEditResult set_visual_object_format(const VisualObjectFormatRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "Format",
        "format",
        request.format);
}

VisualAssetEditResult set_visual_object_font_name(const VisualObjectFontNameRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "FontName",
        "font name",
        request.font_name);
}

VisualAssetEditResult set_visual_object_font_size(const VisualObjectFontSizeRequest& request) {
    if (!std::isfinite(request.font_size) || request.font_size < 0.0) {
        return {.ok = false, .error = "FontSize must be a finite non-negative value."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontSize",
        "font size",
        format_visual_geometry_number(request.font_size));
}

VisualAssetEditResult set_visual_object_font_bold(const VisualObjectFontBoldRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontBold",
        "font bold",
        request.font_bold ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_italic(const VisualObjectFontItalicRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontItalic",
        "font italic",
        request.font_italic ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_underline(const VisualObjectFontUnderlineRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontUnderline",
        "font underline",
        request.font_underline ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_strikethru(const VisualObjectFontStrikethruRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontStrikethru",
        "font strikethru",
        request.font_strikethru ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_outline(const VisualObjectFontOutlineRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontOutline",
        "font outline",
        request.font_outline ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_font_shadow(const VisualObjectFontShadowRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "FontShadow",
        "font shadow",
        request.font_shadow ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_dynamic_alignment(const VisualObjectDynamicAlignmentRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicAlignment",
        "dynamic alignment",
        request.dynamic_alignment);
}

VisualAssetEditResult set_visual_object_dynamic_current_control(
    const VisualObjectDynamicCurrentControlRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicCurrentControl",
        "dynamic current control",
        request.dynamic_current_control);
}

VisualAssetEditResult set_visual_object_dynamic_font_name(const VisualObjectDynamicFontNameRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontName",
        "dynamic font name",
        request.dynamic_font_name);
}

VisualAssetEditResult set_visual_object_dynamic_font_size(const VisualObjectDynamicFontSizeRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontSize",
        "dynamic font size",
        request.dynamic_font_size);
}

VisualAssetEditResult set_visual_object_dynamic_font_bold(const VisualObjectDynamicFontBoldRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontBold",
        "dynamic font bold",
        request.dynamic_font_bold);
}

VisualAssetEditResult set_visual_object_dynamic_font_italic(const VisualObjectDynamicFontItalicRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontItalic",
        "dynamic font italic",
        request.dynamic_font_italic);
}

VisualAssetEditResult set_visual_object_dynamic_font_underline(const VisualObjectDynamicFontUnderlineRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontUnderline",
        "dynamic font underline",
        request.dynamic_font_underline);
}

VisualAssetEditResult set_visual_object_dynamic_font_strikethru(const VisualObjectDynamicFontStrikethruRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontStrikethru",
        "dynamic font strikethru",
        request.dynamic_font_strikethru);
}

VisualAssetEditResult set_visual_object_dynamic_font_outline(const VisualObjectDynamicFontOutlineRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontOutline",
        "dynamic font outline",
        request.dynamic_font_outline);
}

VisualAssetEditResult set_visual_object_dynamic_font_shadow(const VisualObjectDynamicFontShadowRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicFontShadow",
        "dynamic font shadow",
        request.dynamic_font_shadow);
}

VisualAssetEditResult set_visual_object_row_source(const VisualObjectRowSourceRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "RowSource",
        "row-source",
        request.row_source);
}

VisualAssetEditResult set_visual_object_row_source_type(const VisualObjectRowSourceTypeRequest& request) {
    if (request.row_source_type < 0) {
        return {.ok = false, .error = "RowSourceType must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "RowSourceType",
        "row-source type",
        std::to_string(request.row_source_type));
}

VisualAssetEditResult set_visual_object_bound_column(const VisualObjectBoundColumnRequest& request) {
    if (request.bound_column < 0) {
        return {.ok = false, .error = "BoundColumn must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BoundColumn",
        "bound-column",
        std::to_string(request.bound_column));
}

VisualAssetEditResult set_visual_object_button_count(const VisualObjectButtonCountRequest& request) {
    if (request.button_count < 0) {
        return {.ok = false, .error = "ButtonCount must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ButtonCount",
        "button-count",
        std::to_string(request.button_count));
}

VisualAssetEditResult set_visual_object_column_count(const VisualObjectColumnCountRequest& request) {
    if (request.column_count < 0) {
        return {.ok = false, .error = "ColumnCount must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ColumnCount",
        "column-count",
        std::to_string(request.column_count));
}

VisualAssetEditResult set_visual_object_column_widths(const VisualObjectColumnWidthsRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "ColumnWidths",
        "column-widths",
        request.column_widths);
}

VisualAssetEditResult set_visual_object_column_lines(const VisualObjectColumnLinesRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ColumnLines",
        "column-lines",
        request.column_lines ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_integral_height(const VisualObjectIntegralHeightRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "IntegralHeight",
        "integral-height",
        request.integral_height ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_incremental_search(const VisualObjectIncrementalSearchRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "IncrementalSearch",
        "incremental-search",
        request.incremental_search ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_multi_select(const VisualObjectMultiSelectRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "MultiSelect",
        "multi-select",
        request.multi_select ? ".T." : ".F.");
}

VisualAssetEditResult set_visual_object_style(const VisualObjectStyleRequest& request) {
    if (request.style < 0) {
        return {.ok = false, .error = "Style must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "Style",
        "style",
        std::to_string(request.style));
}

VisualAssetEditResult set_visual_object_list_index(const VisualObjectListIndexRequest& request) {
    if (request.list_index < 0) {
        return {.ok = false, .error = "ListIndex must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ListIndex",
        "list-index",
        std::to_string(request.list_index));
}

VisualAssetEditResult set_visual_object_display_value(const VisualObjectDisplayValueRequest& request) {
    return set_visual_object_text_property(
        request.path,
        request.objects,
        "DisplayValue",
        "display-value",
        request.display_value);
}

VisualAssetEditResult set_visual_object_selected_back_color(const VisualObjectSelectedBackColorRequest& request) {
    if (request.selected_back_color < 0) {
        return {.ok = false, .error = "SelectedBackColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SelectedBackColor",
        "selected back-color",
        std::to_string(request.selected_back_color));
}

VisualAssetEditResult set_visual_object_selected_fore_color(const VisualObjectSelectedForeColorRequest& request) {
    if (request.selected_fore_color < 0) {
        return {.ok = false, .error = "SelectedForeColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SelectedForeColor",
        "selected fore-color",
        std::to_string(request.selected_fore_color));
}

VisualAssetEditResult set_visual_object_selected_item_back_color(
    const VisualObjectSelectedItemBackColorRequest& request) {
    if (request.selected_item_back_color < 0) {
        return {.ok = false, .error = "SelectedItemBackColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SelectedItemBackColor",
        "selected-item back-color",
        std::to_string(request.selected_item_back_color));
}

VisualAssetEditResult set_visual_object_selected_item_fore_color(
    const VisualObjectSelectedItemForeColorRequest& request) {
    if (request.selected_item_fore_color < 0) {
        return {.ok = false, .error = "SelectedItemForeColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "SelectedItemForeColor",
        "selected-item fore-color",
        std::to_string(request.selected_item_fore_color));
}

VisualAssetEditResult set_visual_object_disabled_item_back_color(
    const VisualObjectDisabledItemBackColorRequest& request) {
    if (request.disabled_item_back_color < 0) {
        return {.ok = false, .error = "DisabledItemBackColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DisabledItemBackColor",
        "disabled-item back-color",
        std::to_string(request.disabled_item_back_color));
}

VisualAssetEditResult set_visual_object_disabled_item_fore_color(
    const VisualObjectDisabledItemForeColorRequest& request) {
    if (request.disabled_item_fore_color < 0) {
        return {.ok = false, .error = "DisabledItemForeColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DisabledItemForeColor",
        "disabled-item fore-color",
        std::to_string(request.disabled_item_fore_color));
}

VisualAssetEditResult set_visual_object_item_back_color(
    const VisualObjectItemBackColorRequest& request) {
    if (request.item_back_color < 0) {
        return {.ok = false, .error = "ItemBackColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ItemBackColor",
        "item back-color",
        std::to_string(request.item_back_color));
}

VisualAssetEditResult set_visual_object_item_fore_color(
    const VisualObjectItemForeColorRequest& request) {
    if (request.item_fore_color < 0) {
        return {.ok = false, .error = "ItemForeColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ItemForeColor",
        "item fore-color",
        std::to_string(request.item_fore_color));
}

VisualAssetEditResult set_visual_object_highlight_back_color(
    const VisualObjectHighlightBackColorRequest& request) {
    if (request.highlight_back_color < 0) {
        return {.ok = false, .error = "HighlightBackColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HighlightBackColor",
        "highlight back-color",
        std::to_string(request.highlight_back_color));
}

VisualAssetEditResult set_visual_object_highlight_fore_color(
    const VisualObjectHighlightForeColorRequest& request) {
    if (request.highlight_fore_color < 0) {
        return {.ok = false, .error = "HighlightForeColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "HighlightForeColor",
        "highlight fore-color",
        std::to_string(request.highlight_fore_color));
}

VisualAssetEditResult set_visual_object_back_color(
    const VisualObjectBackColorRequest& request) {
    if (request.back_color < 0) {
        return {.ok = false, .error = "BackColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "BackColor",
        "back-color",
        std::to_string(request.back_color));
}

VisualAssetEditResult set_visual_object_fore_color(
    const VisualObjectForeColorRequest& request) {
    if (request.fore_color < 0) {
        return {.ok = false, .error = "ForeColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "ForeColor",
        "fore-color",
        std::to_string(request.fore_color));
}

VisualAssetEditResult set_visual_object_disabled_back_color(
    const VisualObjectDisabledBackColorRequest& request) {
    if (request.disabled_back_color < 0) {
        return {.ok = false, .error = "DisabledBackColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DisabledBackColor",
        "disabled back-color",
        std::to_string(request.disabled_back_color));
}

VisualAssetEditResult set_visual_object_disabled_fore_color(
    const VisualObjectDisabledForeColorRequest& request) {
    if (request.disabled_fore_color < 0) {
        return {.ok = false, .error = "DisabledForeColor must not be negative."};
    }

    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DisabledForeColor",
        "disabled fore-color",
        std::to_string(request.disabled_fore_color));
}

VisualAssetEditResult set_visual_object_dynamic_back_color(
    const VisualObjectDynamicBackColorRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicBackColor",
        "dynamic back-color",
        request.dynamic_back_color);
}

VisualAssetEditResult set_visual_object_dynamic_fore_color(
    const VisualObjectDynamicForeColorRequest& request) {
    return set_visual_object_scalar_property(
        request.path,
        request.objects,
        "DynamicForeColor",
        "dynamic fore-color",
        request.dynamic_fore_color);
}

VisualAssetEditResult reparent_visual_object(const VisualObjectReparentRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }

    std::size_t source_record_index = 0U;
    const auto source_resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, source_record_index);
    if (!source_resolution.ok) {
        return source_resolution;
    }

    std::string parent_name;
    if (!request.clear_parent) {
        if (trim_both(request.parent_object_name).empty() && trim_both(request.parent_unique_id).empty()) {
            return {.ok = false, .error = "No parent object selector was provided."};
        }

        std::size_t parent_record_index = 0U;
        const auto parent_resolution = resolve_visual_object_record_index({
            .path = request.path,
            .record_index = 0U,
            .object_name = request.parent_object_name,
            .unique_id = request.parent_unique_id,
            .property_name = {},
            .property_value = {}
        }, parent_record_index);
        if (!parent_resolution.ok) {
            return parent_resolution;
        }
        if (parent_record_index == source_record_index) {
            return {.ok = false, .error = "A visual object cannot be reparented to itself."};
        }

        const auto table_result = parse_dbf_table_from_file(
            request.path,
            std::max(source_record_index, parent_record_index) + 1U);
        if (!table_result.ok) {
            return {.ok = false, .error = table_result.error};
        }
        if (parent_record_index >= table_result.table.records.size()) {
            return {.ok = false, .error = "The requested parent record is not currently available."};
        }

        parent_name = visual_object_record_name(table_result.table.records[parent_record_index]);
        if (parent_name.empty()) {
            return {.ok = false, .error = "The selected parent does not expose an object name."};
        }
    }

    return update_visual_object_property({
        .path = request.path,
        .record_index = source_record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "PARENT",
        .property_value = parent_name
    });
}

VisualAssetEditResult reparent_visual_objects(const VisualObjectReparentBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual object reparent operations were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_reparents = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& object : request.objects) {
        const auto result = reparent_visual_object({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .parent_object_name = object.parent_object_name,
            .parent_unique_id = object.parent_unique_id,
            .clear_parent = object.clear_parent
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_reparents();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult rename_visual_object(const VisualObjectRenameRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (!request.update_object_name && !request.update_name && !request.update_unique_id) {
        return {.ok = false, .error = "No identity fields were provided."};
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

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    const auto& table = table_result.table;
    if (record_index >= table.records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available."};
    }

    const auto require_field = [&](const std::string& field_name) -> VisualAssetEditResult {
        if (find_field_index(table, field_name).has_value()) {
            return {.ok = true, .error = {}};
        }
        return {.ok = false, .error = "The requested identity field is not present in the asset."};
    };
    for (const auto& check : {
             request.update_object_name ? require_field("OBJNAME") : VisualAssetEditResult{.ok = true, .error = {}},
             request.update_name ? require_field("NAME") : VisualAssetEditResult{.ok = true, .error = {}},
             request.update_unique_id ? require_field("UNIQUEID") : VisualAssetEditResult{.ok = true, .error = {}}
         }) {
        if (!check.ok) {
            return check;
        }
    }

    const auto* current_objname = find_record_value(table.records[record_index], "OBJNAME");
    const auto* current_name = find_record_value(table.records[record_index], "NAME");
    const auto* current_unique_id = find_record_value(table.records[record_index], "UNIQUEID");
    const std::string final_objname = request.update_object_name
        ? request.new_object_name
        : (current_objname == nullptr ? std::string{} : current_objname->display_value);
    const std::string final_name = request.update_name
        ? request.new_name
        : (current_name == nullptr ? std::string{} : current_name->display_value);
    const std::string final_unique_id = request.update_unique_id
        ? request.new_unique_id
        : (current_unique_id == nullptr ? std::string{} : current_unique_id->display_value);

    for (const auto& check : {
             reject_identity_collision_excluding_record(table, "OBJNAME", final_objname, record_index),
             reject_identity_collision_excluding_record(table, "NAME", final_name, record_index),
             reject_identity_collision_excluding_record(table, "UNIQUEID", final_unique_id, record_index)
         }) {
        if (!check.ok) {
            return check;
        }
    }

    std::vector<VisualObjectPropertyChange> changes;
    if (request.update_object_name) {
        changes.push_back({.property_name = "OBJNAME", .property_value = request.new_object_name});
    }
    if (request.update_name) {
        changes.push_back({.property_name = "NAME", .property_value = request.new_name});
    }
    if (request.update_unique_id) {
        changes.push_back({.property_name = "UNIQUEID", .property_value = request.new_unique_id});
    }

    return update_visual_object_properties({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .properties = changes
    });
}

VisualAssetEditResult rename_visual_objects(const VisualObjectRenameBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual object renames were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    const auto rollback_batch_renames = [&]() -> VisualAssetEditResult {
        while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
            const auto rollback_result = undo_visual_object_property(request.path);
            if (!rollback_result.ok) {
                return rollback_result;
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& object : request.objects) {
        const auto result = rename_visual_object({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .update_object_name = object.update_object_name,
            .new_object_name = object.new_object_name,
            .update_name = object.update_name,
            .new_name = object.new_name,
            .update_unique_id = object.update_unique_id,
            .new_unique_id = object.new_unique_id
        });
        if (!result.ok) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult reorder_visual_object(const VisualObjectReorderRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }

    const std::string placement = normalize_visual_property_name(request.placement);
    if (placement != "front" && placement != "back" && placement != "before" && placement != "after") {
        return {.ok = false, .error = "Unsupported visual object placement."};
    }

    std::size_t source_record_index = 0U;
    const auto source_resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, source_record_index);
    if (!source_resolution.ok) {
        return source_resolution;
    }

    std::size_t target_record_index = 0U;
    if (placement == "before" || placement == "after") {
        if (trim_both(request.target_object_name).empty() && trim_both(request.target_unique_id).empty()) {
            return {.ok = false, .error = "No target object selector was provided."};
        }
        const auto target_resolution = resolve_visual_object_record_index({
            .path = request.path,
            .record_index = 0U,
            .object_name = request.target_object_name,
            .unique_id = request.target_unique_id,
            .property_name = {},
            .property_value = {}
        }, target_record_index);
        if (!target_resolution.ok) {
            return target_resolution;
        }
        if (target_record_index == source_record_index) {
            return {.ok = false, .error = "A visual object cannot be reordered relative to itself."};
        }
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }
    const auto& table = table_result.table;
    if (source_record_index >= table.records.size()) {
        return {.ok = false, .error = "The requested object record is not currently available."};
    }
    if ((placement == "before" || placement == "after") && target_record_index >= table.records.size()) {
        return {.ok = false, .error = "The requested target record is not currently available."};
    }

    std::vector<std::vector<std::string>> records;
    records.reserve(table.records.size());
    std::vector<bool> deleted_flags;
    deleted_flags.reserve(table.records.size());
    for (const auto& record : table.records) {
        std::vector<std::string> values;
        values.reserve(table.fields.size());
        for (const auto& field : table.fields) {
            const auto* value = find_record_value(record, field.name);
            values.push_back(value == nullptr ? std::string{} : value->display_value);
        }
        records.push_back(std::move(values));
        deleted_flags.push_back(record.deleted);
    }

    std::vector<std::size_t> order;
    order.reserve(records.size());
    for (std::size_t index = 0U; index < records.size(); ++index) {
        if (index != source_record_index) {
            order.push_back(index);
        }
    }

    std::size_t insert_position = 0U;
    if (placement == "back") {
        insert_position = order.size();
    } else if (placement == "before" || placement == "after") {
        const auto target = std::find(order.begin(), order.end(), target_record_index);
        if (target == order.end()) {
            return {.ok = false, .error = "The requested target record is not currently available."};
        }
        insert_position = static_cast<std::size_t>(std::distance(order.begin(), target));
        if (placement == "after") {
            ++insert_position;
        }
    }
    order.insert(order.begin() + static_cast<std::ptrdiff_t>(insert_position), source_record_index);

    std::vector<std::vector<std::string>> reordered_records;
    reordered_records.reserve(records.size());
    std::vector<bool> reordered_deleted_flags;
    reordered_deleted_flags.reserve(deleted_flags.size());
    for (const auto record_index : order) {
        reordered_records.push_back(records[record_index]);
        reordered_deleted_flags.push_back(deleted_flags[record_index]);
    }

    const auto create_result = create_dbf_table_file(request.path, table.fields, reordered_records);
    if (!create_result.ok) {
        return {.ok = false, .error = create_result.error};
    }
    for (std::size_t index = 0U; index < reordered_deleted_flags.size(); ++index) {
        if (!reordered_deleted_flags[index]) {
            continue;
        }
        const auto delete_result = set_record_deleted_flag(request.path, index, true);
        if (!delete_result.ok) {
            return {.ok = false, .error = delete_result.error};
        }
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult reorder_visual_objects(const VisualObjectReorderBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual object reorders were provided."};
    }

    const auto table_result = parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }

    std::vector<DbfRecord> reordered_records = table_result.table.records;
    for (const auto& object : request.objects) {
        const auto result = apply_visual_object_reorder_to_records(reordered_records, object);
        if (!result.ok) {
            return result;
        }
    }

    const std::vector<std::uint8_t> original_table_bytes = read_binary_file(request.path);
    if (original_table_bytes.empty()) {
        return {.ok = false, .error = "Unable to open the visual asset table."};
    }
    const std::string memo_path = infer_memo_sidecar_path(request.path);
    const std::vector<std::uint8_t> original_memo_bytes = memo_path.empty()
        ? std::vector<std::uint8_t>{}
        : read_binary_file(memo_path);

    const auto create_result = create_dbf_table_file(
        request.path,
        table_result.table.fields,
        visual_record_values_for_write(table_result.table.fields, reordered_records));
    if (!create_result.ok) {
        return {.ok = false, .error = create_result.error};
    }

    for (std::size_t index = 0U; index < reordered_records.size(); ++index) {
        if (!reordered_records[index].deleted) {
            continue;
        }
        const auto delete_result = set_record_deleted_flag(request.path, index, true);
        if (!delete_result.ok) {
            write_binary_file(request.path, original_table_bytes);
            if (!memo_path.empty() && !original_memo_bytes.empty()) {
                write_binary_file(memo_path, original_memo_bytes);
            }
            return {.ok = false, .error = delete_result.error};
        }
    }

    return {.ok = true, .error = {}};
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

VisualAssetEditResult set_visual_object_deleted_states(const VisualObjectDeletedStateBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual object deleted-state changes were provided."};
    }

    struct AppliedDeletedState {
        std::size_t record_index = 0;
        bool prior_deleted = false;
    };
    std::vector<AppliedDeletedState> applied;

    const auto rollback_applied_states = [&]() -> VisualAssetEditResult {
        for (auto item = applied.rbegin(); item != applied.rend(); ++item) {
            const auto rollback_result = set_record_deleted_flag(request.path, item->record_index, item->prior_deleted);
            if (!rollback_result.ok) {
                return {.ok = false, .error = rollback_result.error};
            }
        }
        return {.ok = true, .error = {}};
    };

    for (const auto& object : request.objects) {
        std::size_t record_index = 0U;
        const auto resolution = resolve_visual_object_record_index({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .property_name = {},
            .property_value = {}
        }, record_index);
        if (!resolution.ok) {
            const auto rollback_result = rollback_applied_states();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = resolution.error + " Rollback failed: " + rollback_result.error
                };
            }
            return resolution;
        }

        const auto table_result = parse_dbf_table_from_file(request.path, record_index + 1U);
        if (!table_result.ok || record_index >= table_result.table.records.size()) {
            const std::string error = table_result.ok
                ? "The requested object record is not currently available."
                : table_result.error;
            const auto rollback_result = rollback_applied_states();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = error + " Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = error};
        }

        const bool prior_deleted = table_result.table.records[record_index].deleted;
        if (prior_deleted == object.deleted) {
            continue;
        }

        const auto result = set_record_deleted_flag(request.path, record_index, object.deleted);
        if (!result.ok) {
            const auto rollback_result = rollback_applied_states();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = result.error + " Rollback failed: " + rollback_result.error
                };
            }
            return {.ok = false, .error = result.error};
        }
        applied.push_back({.record_index = record_index, .prior_deleted = prior_deleted});
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult set_visual_object_subtree_deleted_state(const VisualObjectSubtreeDeletedStateRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }

    std::size_t root_record_index = 0U;
    const auto resolution = resolve_visual_object_record_index({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = {},
        .property_value = {}
    }, root_record_index);
    if (!resolution.ok) {
        return resolution;
    }

    const auto descendants_result = list_visual_object_descendants({
        .path = request.path,
        .record_index = root_record_index,
        .object_name = {},
        .unique_id = {}
    });
    if (!descendants_result.ok) {
        return {.ok = false, .error = descendants_result.error};
    }

    std::vector<VisualObjectDeletedStateBatchItem> objects;
    objects.reserve(descendants_result.descendants.size() + 1U);
    objects.push_back({
        .record_index = root_record_index,
        .object_name = {},
        .unique_id = {},
        .deleted = request.deleted
    });
    for (const auto& descendant : descendants_result.descendants) {
        objects.push_back({
            .record_index = descendant.object.record_index,
            .object_name = {},
            .unique_id = {},
            .deleted = request.deleted
        });
    }

    return set_visual_object_deleted_states({
        .path = request.path,
        .objects = std::move(objects)
    });
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

VisualAssetEditResult update_visual_object_batch(const VisualObjectBatchEditRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }
    if (request.objects.empty()) {
        return {.ok = false, .error = "No visual object edits were provided."};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    for (const auto& object : request.objects) {
        if (object.properties.empty()) {
            while (list_visual_asset_undo_entry_files(request.path).size() > initial_undo_depth) {
                const auto rollback_result = undo_visual_object_property(request.path);
                if (!rollback_result.ok) {
                    return {
                        .ok = false,
                        .error = "No property changes were provided. Rollback failed: " + rollback_result.error
                    };
                }
            }
            return {.ok = false, .error = "No property changes were provided."};
        }

        const auto result = update_visual_object_properties({
            .path = request.path,
            .record_index = object.record_index,
            .object_name = object.object_name,
            .unique_id = object.unique_id,
            .properties = object.properties
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
