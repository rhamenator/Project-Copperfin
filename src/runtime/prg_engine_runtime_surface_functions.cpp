#include "prg_engine_runtime_surface_functions.h"

#include "prg_engine_file_io_functions.h"
#include "prg_engine_helpers.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace copperfin::runtime {

namespace {

std::uint32_t bitwise_value(const PrgValue& value) {
    return static_cast<std::uint32_t>(
        static_cast<std::int32_t>(std::llround(value_as_number(value))));
}

std::int64_t signed_bitwise_result(std::uint32_t value) {
    return static_cast<std::int64_t>(static_cast<std::int32_t>(value));
}

int bit_position(const PrgValue& value) {
    const int position = static_cast<int>(std::llround(value_as_number(value)));
    if (position < 0 || position > 31) {
        throw std::runtime_error("Bit position must be between 0 and 31");
    }
    return position;
}

std::string host_os_name() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#elif defined(__unix__)
    return "Unix";
#else
    return "Unknown";
#endif
}

std::filesystem::path filesystem_probe_path(const std::string& raw_path, const std::string& default_directory) {
    std::filesystem::path path(raw_path.empty() ? default_directory : raw_path);
    if (path.is_relative()) {
        path = std::filesystem::path(default_directory) / path;
    }
    return path.lexically_normal();
}

std::string strip_surrounding_quotes(std::string text) {
    text = trim_copy(std::move(text));
    if (text.size() >= 2U) {
        const char first = text.front();
        const char last = text.back();
        if ((first == '\'' && last == '\'') || (first == '"' && last == '"')) {
            return text.substr(1U, text.size() - 2U);
        }
    }
    return text;
}

std::vector<std::filesystem::path> parse_set_path_entries(const std::string& set_path_value,
                                                          const std::string& default_directory) {
    std::string value = trim_copy(set_path_value);
    if (starts_with_insensitive(value, "TO ")) {
        value = trim_copy(value.substr(3U));
    }
    value = strip_surrounding_quotes(std::move(value));

    std::vector<std::filesystem::path> entries;
    std::size_t token_start = 0U;
    while (token_start <= value.size()) {
        const std::size_t separator = value.find(';', token_start);
        std::string token = separator == std::string::npos
                                ? value.substr(token_start)
                                : value.substr(token_start, separator - token_start);
        token = strip_surrounding_quotes(std::move(token));
        if (!token.empty()) {
            std::filesystem::path entry(token);
            if (entry.is_relative()) {
                entry = std::filesystem::path(default_directory) / entry;
            }
            entries.push_back(entry.lexically_normal());
        }

        if (separator == std::string::npos) {
            break;
        }
        token_start = separator + 1U;
    }

    return entries;
}

std::filesystem::path resolve_runtime_file_probe_path(
    const std::string& raw_path,
    const std::string& default_directory,
    const std::function<std::string(const std::string&)>& set_callback) {
    std::error_code ignored;
    std::filesystem::path path(raw_path.empty() ? default_directory : raw_path);
    if (!path.is_relative()) {
        return path.lexically_normal();
    }

    const std::filesystem::path default_candidate =
        (std::filesystem::path(default_directory) / path).lexically_normal();
    if (std::filesystem::exists(default_candidate, ignored)) {
        return default_candidate;
    }

    const std::vector<std::filesystem::path> set_path_entries =
        parse_set_path_entries(set_callback("PATH"), default_directory);
    for (const auto& entry : set_path_entries) {
        const std::filesystem::path candidate = (entry / path).lexically_normal();
        if (std::filesystem::exists(candidate, ignored)) {
            return candidate;
        }
    }

    return default_candidate;
}

double available_disk_space(const std::string& raw_path, const std::string& default_directory) {
    std::error_code ignored;
    const auto info = std::filesystem::space(filesystem_probe_path(raw_path, default_directory), ignored);
    return ignored ? 0.0 : static_cast<double>(info.available);
}

int drive_type_value(const std::string& raw_path, const std::string& default_directory) {
    std::error_code ignored;
    const std::filesystem::path path = filesystem_probe_path(raw_path, default_directory);
    if (!std::filesystem::exists(path, ignored)) {
        return 0;
    }
    return std::filesystem::is_directory(path, ignored) || std::filesystem::is_regular_file(path, ignored)
               ? 3
               : 1;
}

std::string class_token_from_prog_id(const std::string& prog_id) {
    std::string token = trim_copy(prog_id);
    const std::size_t separator = token.find_last_of('.');
    if (separator != std::string::npos && separator + 1U < token.size()) {
        token = token.substr(separator + 1U);
    }
    token = uppercase_copy(trim_copy(std::move(token)));
    return token.empty() ? "CUSTOM" : token;
}

bool object_has_member(const std::vector<std::string>& members, const std::string& normalized_member_name) {
    return std::find_if(members.begin(), members.end(), [&](const std::string& member) {
               return normalize_identifier(member) == normalized_member_name;
           }) != members.end();
}

bool is_scripting_dictionary_object(const RuntimeOleObjectState& runtime_object) {
    return normalize_identifier(runtime_object.prog_id) == "scripting.dictionary";
}

bool looks_like_file_path(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    if (trimmed.empty()) {
        return false;
    }
    if (trimmed.find('/') != std::string::npos || trimmed.find('\\') != std::string::npos) {
        return true;
    }
    if (trimmed.size() >= 2U && std::isalpha(static_cast<unsigned char>(trimmed[0])) != 0 && trimmed[1] == ':') {
        return true;
    }
    const std::string lower = lowercase_copy(trimmed);
    const auto has_suffix = [&](const std::string& suffix) {
        return lower.size() >= suffix.size() && lower.compare(lower.size() - suffix.size(), suffix.size(), suffix) == 0;
    };
    return has_suffix(".xml") || has_suffix(".txt");
}

std::string xml_escape(std::string value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '&':
                escaped += "&amp;";
                break;
            case '<':
                escaped += "&lt;";
                break;
            case '>':
                escaped += "&gt;";
                break;
            case '"':
                escaped += "&quot;";
                break;
            case '\'':
                escaped += "&apos;";
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }
    return escaped;
}

std::string xml_unescape(std::string value) {
    const auto replace_all = [&](const std::string& token, const std::string& replacement) {
        std::size_t position = 0U;
        while ((position = value.find(token, position)) != std::string::npos) {
            value.replace(position, token.size(), replacement);
            position += replacement.size();
        }
    };
    replace_all("&lt;", "<");
    replace_all("&gt;", ">");
    replace_all("&quot;", "\"");
    replace_all("&apos;", "'");
    replace_all("&amp;", "&");
    return value;
}

std::string xml_attribute(const std::string& tag_text, const std::string& name) {
    const std::string needle = name + "=\"";
    const std::size_t start = tag_text.find(needle);
    if (start == std::string::npos) {
        return {};
    }
    const std::size_t value_start = start + needle.size();
    const std::size_t value_end = tag_text.find('"', value_start);
    if (value_end == std::string::npos) {
        return {};
    }
    return xml_unescape(tag_text.substr(value_start, value_end - value_start));
}

std::string serialize_cursor_snapshot_xml(const RuntimeSurfaceCursorSnapshot& snapshot) {
    std::ostringstream xml;
    xml << "<CopperfinCursor alias=\"" << xml_escape(snapshot.alias) << "\">\n";
    xml << "  <Fields>\n";
    for (const auto& field : snapshot.fields) {
        xml << "    <Field name=\"" << xml_escape(field.name)
            << "\" type=\"" << xml_escape(std::string(1U, field.type))
            << "\" width=\"" << field.width
            << "\" decimals=\"" << field.decimals
            << "\" />\n";
    }
    xml << "  </Fields>\n";
    xml << "  <Rows>\n";
    for (const auto& row : snapshot.rows) {
        xml << "    <Row>";
        for (const auto& value : row.values) {
            xml << "<Col>" << xml_escape(value) << "</Col>";
        }
        xml << "</Row>\n";
    }
    xml << "  </Rows>\n";
    xml << "</CopperfinCursor>\n";
    return xml.str();
}

std::optional<RuntimeSurfaceCursorSnapshot> parse_cursor_snapshot_xml(const std::string& xml_text) {
    RuntimeSurfaceCursorSnapshot snapshot;

    const std::size_t root_start = xml_text.find("<CopperfinCursor");
    if (root_start == std::string::npos) {
        return std::nullopt;
    }
    const std::size_t root_tag_end = xml_text.find('>', root_start);
    const std::size_t root_end = xml_text.find("</CopperfinCursor>", root_tag_end == std::string::npos ? 0U : root_tag_end);
    if (root_tag_end == std::string::npos || root_end == std::string::npos) {
        return std::nullopt;
    }
    snapshot.alias = xml_attribute(xml_text.substr(root_start, root_tag_end - root_start + 1U), "alias");

    const std::size_t fields_start = xml_text.find("<Fields>", root_tag_end);
    const std::size_t fields_end = xml_text.find("</Fields>", fields_start == std::string::npos ? 0U : fields_start);
    if (fields_start == std::string::npos || fields_end == std::string::npos) {
        return std::nullopt;
    }

    std::size_t scan = fields_start;
    while (true) {
        const std::size_t field_start = xml_text.find("<Field ", scan);
        if (field_start == std::string::npos || field_start >= fields_end) {
            break;
        }
        const std::size_t field_end = xml_text.find("/>", field_start);
        if (field_end == std::string::npos || field_end > fields_end) {
            return std::nullopt;
        }
        const std::string field_tag = xml_text.substr(field_start, field_end - field_start + 2U);
        RuntimeSurfaceCursorField field;
        field.name = xml_attribute(field_tag, "name");
        const std::string type_text = xml_attribute(field_tag, "type");
        field.type = type_text.empty() ? 'C' : type_text.front();
        try {
            field.width = static_cast<std::size_t>(std::stoul(xml_attribute(field_tag, "width")));
        } catch (...) {
            field.width = 0U;
        }
        try {
            field.decimals = static_cast<std::size_t>(std::stoul(xml_attribute(field_tag, "decimals")));
        } catch (...) {
            field.decimals = 0U;
        }
        if (field.name.empty()) {
            return std::nullopt;
        }
        snapshot.fields.push_back(std::move(field));
        scan = field_end + 2U;
    }

    const std::size_t rows_start = xml_text.find("<Rows>", fields_end);
    const std::size_t rows_end = xml_text.find("</Rows>", rows_start == std::string::npos ? 0U : rows_start);
    if (rows_start == std::string::npos || rows_end == std::string::npos) {
        return std::nullopt;
    }

    scan = rows_start;
    while (true) {
        const std::size_t row_start = xml_text.find("<Row>", scan);
        if (row_start == std::string::npos || row_start >= rows_end) {
            break;
        }
        const std::size_t row_end = xml_text.find("</Row>", row_start);
        if (row_end == std::string::npos || row_end > rows_end) {
            return std::nullopt;
        }

        RuntimeSurfaceCursorRow row;
        std::size_t col_scan = row_start;
        while (true) {
            const std::size_t col_start = xml_text.find("<Col>", col_scan);
            if (col_start == std::string::npos || col_start >= row_end) {
                break;
            }
            const std::size_t col_value_start = col_start + 5U;
            const std::size_t col_end = xml_text.find("</Col>", col_value_start);
            if (col_end == std::string::npos || col_end > row_end) {
                return std::nullopt;
            }
            row.values.push_back(xml_unescape(xml_text.substr(col_value_start, col_end - col_value_start)));
            col_scan = col_end + 6U;
        }
        snapshot.rows.push_back(std::move(row));
        scan = row_end + 6U;
    }

    return snapshot;
}

std::vector<std::string> collect_object_member_names(const RuntimeOleObjectState& runtime_object, int flags) {
    const bool include_all = flags == 0;
    const bool include_properties = include_all || ((flags & 1) != 0);
    const bool include_methods = include_all || ((flags & 2) != 0);
    const bool include_events = include_all || ((flags & 4) != 0);

    std::set<std::string> unique_members;
    if (include_properties) {
        for (const auto& [name, value] : runtime_object.properties) {
            (void)value;
            unique_members.insert(normalize_identifier(name));
        }
    }
    if (include_methods) {
        for (const auto& method_name : runtime_object.methods) {
            unique_members.insert(normalize_identifier(method_name));
        }
    }
    if (include_events) {
        for (const auto& event_name : runtime_object.events) {
            unique_members.insert(normalize_identifier(event_name));
        }
    }

    std::vector<std::string> members;
    members.reserve(unique_members.size());
    for (const std::string& member_name : unique_members) {
        members.push_back(uppercase_copy(member_name));
    }

    std::sort(members.begin(), members.end(), [](const std::string& left, const std::string& right) {
        const std::string normalized_left = lowercase_copy(left);
        const std::string normalized_right = lowercase_copy(right);
        if (normalized_left == normalized_right) {
            return left < right;
        }
        return normalized_left < normalized_right;
    });
    return members;
}

}  // namespace

std::optional<PrgValue> evaluate_runtime_surface_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::vector<std::string>& raw_arguments,
    const std::string& default_directory,
    const std::string& frame_file_path,
    const std::string& last_error_message,
    int last_error_code,
    const std::string& last_error_procedure,
    std::size_t last_error_line,
    const std::string& error_handler,
    const std::string& shutdown_handler,
    const std::function<int(const std::string&)>& aerror_callback,
    const std::function<PrgValue(const std::string&)>& eval_expression_callback,
    const std::function<std::string(const std::string&)>& set_callback,
    const std::function<std::optional<RuntimeSurfaceCursorSnapshot>(const std::string&)>& snapshot_cursor_callback,
    const std::function<std::optional<std::size_t>(const RuntimeSurfaceCursorSnapshot&, const std::string&)>& load_cursor_snapshot_callback,
    const std::function<RuntimeOleObjectState*(const PrgValue&)>& resolve_object_callback,
    const std::function<void(const std::string&, std::vector<PrgValue>)>& assign_array_callback,
    const std::function<void(const std::string&, const std::string&)>& record_event_callback) {
    auto record_runtime_warning = [&](const std::string& detail) {
        if (record_event_callback) {
            record_event_callback("runtime.warning", detail);
        }
    };

    if (const auto file_io_result = evaluate_file_io_function(function, arguments, default_directory)) {
        return file_io_result;
    }

    auto safe_int_argument = [&](std::size_t index, int default_value) {
        if (index >= arguments.size()) {
            return default_value;
        }
        const PrgValue& value = arguments[index];
        switch (value.kind) {
            case PrgValueKind::boolean:
                return value.boolean_value ? 1 : 0;
            case PrgValueKind::number:
                return static_cast<int>(std::llround(value.number_value));
            case PrgValueKind::int64:
                return static_cast<int>(value.int64_value);
            case PrgValueKind::uint64:
                return static_cast<int>(value.uint64_value);
            case PrgValueKind::string:
                try {
                    return value.string_value.empty() ? default_value : static_cast<int>(std::llround(std::stod(value.string_value)));
                } catch (...) {
                    return default_value;
                }
            case PrgValueKind::empty:
                return default_value;
        }
        return default_value;
    };

    if (function == "compobj" && arguments.size() >= 2U) {
        int handle_left = 0;
        int handle_right = 0;
        std::string prog_id_left;
        std::string prog_id_right;
        if (!parse_object_handle_reference(arguments[0], handle_left, prog_id_left) ||
            !parse_object_handle_reference(arguments[1], handle_right, prog_id_right)) {
            return make_boolean_value(false);
        }
        if (handle_left != handle_right) {
            return make_boolean_value(false);
        }
        // VFP: same handle means same object; use callback for pointer-level confirm when available
        if (resolve_object_callback) {
            RuntimeOleObjectState* left = resolve_object_callback(arguments[0]);
            RuntimeOleObjectState* right = resolve_object_callback(arguments[1]);
            return make_boolean_value(left != nullptr && right != nullptr && left == right);
        }
        return make_boolean_value(true);
    }

    if (function == "amembers" && arguments.size() >= 2U && !raw_arguments.empty()) {
        const std::string array_name = trim_copy(raw_arguments[0]);
        try {
            const int flags = safe_int_argument(2U, 0);
            if (!resolve_object_callback || !assign_array_callback) {
                record_runtime_warning("AMEMBERS() uses stub behavior (no object/array callback)");
                if (assign_array_callback && !array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[1]);
            if (runtime_object == nullptr || array_name.empty()) {
                if (!array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            std::vector<PrgValue> member_names;
            const std::vector<std::string> member_tokens = collect_object_member_names(*runtime_object, flags);
            member_names.reserve(member_tokens.size());
            for (const std::string& member_name : member_tokens) {
                member_names.push_back(make_string_value(member_name));
            }
            assign_array_callback(array_name, member_names);
            return make_number_value(static_cast<double>(member_names.size()));
        } catch (...) {
            record_runtime_warning("AMEMBERS() fallback: unable to enumerate members, returning empty array");
            if (assign_array_callback && !array_name.empty()) {
                assign_array_callback(array_name, {});
            }
            return make_number_value(0.0);
        }
    }

    if (function == "aclass" && arguments.size() >= 2U && !raw_arguments.empty()) {
        const std::string array_name = trim_copy(raw_arguments[0]);
        try {
            if (!resolve_object_callback || !assign_array_callback) {
                record_runtime_warning("ACLASS() uses stub behavior (no object/array callback)");
                if (assign_array_callback && !array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[1]);
            if (runtime_object == nullptr || array_name.empty()) {
                if (!array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            std::vector<PrgValue> class_chain;
            const std::string class_name = class_token_from_prog_id(runtime_object->prog_id);
            class_chain.push_back(make_string_value(class_name));
            class_chain.push_back(make_string_value("OBJECT"));
            assign_array_callback(array_name, class_chain);
            return make_number_value(static_cast<double>(class_chain.size()));
        } catch (...) {
            record_runtime_warning("ACLASS() fallback: unable to enumerate class chain");
            if (assign_array_callback && !array_name.empty()) {
                assign_array_callback(array_name, {});
            }
            return make_number_value(0.0);
        }
    }

    if (function == "pemstatus" && arguments.size() >= 3U) {
        if (!resolve_object_callback) {
            record_runtime_warning("PEMSTATUS() uses stub object resolution (no runtime object callback)");
            return make_boolean_value(false);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        const std::string member_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        const int attribute = safe_int_argument(2U, 0);
        if (runtime_object == nullptr || member_name.empty()) {
            return make_boolean_value(false);
        }
        if (attribute == 1) {
            const bool exists = runtime_object->properties.contains(member_name) ||
                                object_has_member(runtime_object->methods, member_name) ||
                                object_has_member(runtime_object->events, member_name);
            return make_boolean_value(exists);
        }
        if (attribute == 3) {
            return make_boolean_value(false);
        }
        if (attribute == 5) {
            const bool readonly = is_scripting_dictionary_object(*runtime_object) && member_name == "count";
            return make_boolean_value(readonly);
        }
        return make_boolean_value(false);
    }

    if (function == "addproperty" && arguments.size() >= 2U) {
        if (!resolve_object_callback) {
            record_runtime_warning("ADDPROPERTY() uses stub behavior (no runtime object callback)");
            return make_boolean_value(true);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        if (runtime_object == nullptr) {
            return make_boolean_value(false);
        }
        const std::string property_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (property_name.empty()) {
            return make_boolean_value(false);
        }
        const PrgValue initial_value = arguments.size() >= 3U ? arguments[2] : make_empty_value();
        runtime_object->properties[property_name] = initial_value;
        return make_boolean_value(true);
    }

    if (function == "getpem" && arguments.size() >= 2U) {
        if (!resolve_object_callback) {
            record_runtime_warning("GETPEM() uses stub behavior (no runtime object callback)");
            return make_empty_value();
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        const std::string member_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (runtime_object == nullptr || member_name.empty()) {
            return make_empty_value();
        }
        const auto prop_it = runtime_object->properties.find(member_name);
        if (prop_it != runtime_object->properties.end()) {
            return prop_it->second;
        }
        if (object_has_member(runtime_object->methods, member_name) ||
            object_has_member(runtime_object->events, member_name)) {
            return make_boolean_value(true);
        }
        return make_empty_value();
    }
    
    if (function == "setpem" && arguments.size() >= 3U) {
        if (!resolve_object_callback) {
            record_runtime_warning("SETPEM() uses stub behavior (no runtime object callback)");
            return make_boolean_value(false);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        const std::string member_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (runtime_object == nullptr || member_name.empty()) {
            return make_boolean_value(false);
        }
        if (is_scripting_dictionary_object(*runtime_object) && member_name == "count") {
            return make_boolean_value(false);
        }
        if (object_has_member(runtime_object->methods, member_name) ||
            object_has_member(runtime_object->events, member_name)) {
            runtime_object->properties[member_name] = arguments[2];
            return make_boolean_value(true);
        }
        if (runtime_object->properties.contains(member_name)) {
            runtime_object->properties[member_name] = arguments[2];
            return make_boolean_value(true);
        }
        return make_boolean_value(false);
    }

    if (function == "removeproperty" && arguments.size() >= 2U) {
        if (!resolve_object_callback) {
            record_runtime_warning("REMOVEPROPERTY() uses stub behavior (no runtime object callback)");
            return make_boolean_value(true);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        if (runtime_object == nullptr) {
            return make_boolean_value(false);
        }
        const std::string property_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (property_name.empty()) {
            return make_boolean_value(false);
        }
        const std::size_t removed = runtime_object->properties.erase(property_name);
        return make_boolean_value(removed != 0U);
    }

    if (function == "file" && !arguments.empty()) {
        std::error_code ignored;
        const std::filesystem::path path =
            resolve_runtime_file_probe_path(value_as_string(arguments[0]), default_directory, set_callback);
        return make_boolean_value(std::filesystem::exists(path, ignored));
    }
    if (function == "sys") {
        if (!arguments.empty()) {
            const long long sys_code = std::llround(value_as_number(arguments[0]));
            if (sys_code == 3) {
                return make_string_value("Copperfin Runtime 0.1");
            }
            if (sys_code == 5 || sys_code == 2003 || sys_code == 2004) {
                return make_string_value(default_directory);
            }
            if (sys_code == 7) {
                return make_string_value(host_os_name());
            }
            if (sys_code == 11) {
                return make_string_value("0");
            }
            if (sys_code == 13) {
                return make_string_value("0");
            }
            if (sys_code == 16) {
                return make_string_value(frame_file_path);
            }
            if (sys_code == 2018) {
                return make_string_value(uppercase_copy(runtime_error_parameter(last_error_message)));
            }
            if (sys_code == 2020) {
                return make_string_value(format_value(make_number_value(available_disk_space({}, default_directory))));
            }
            if (sys_code == 2023) {
                std::error_code ignored;
                return make_string_value(std::filesystem::temp_directory_path(ignored).string());
            }
        }
        return make_string_value("0");
    }
    if (function == "home") {
        return make_string_value(default_directory);
    }
    if (function == "os") {
        return make_string_value(host_os_name());
    }
    if (function == "diskspace") {
        const std::string path = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        return make_number_value(available_disk_space(path, default_directory));
    }
    if (function == "drivetype") {
        const std::string path = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        return make_number_value(static_cast<double>(drive_type_value(path, default_directory)));
    }
    if (function == "filesize") {
        if (arguments.empty()) {
            return make_number_value(0.0);
        }
        std::error_code ignored;
        const std::filesystem::path path =
            resolve_runtime_file_probe_path(value_as_string(arguments[0]), default_directory, set_callback);
        if (!std::filesystem::exists(path, ignored)) {
            return make_number_value(0.0);
        }
        const auto size = std::filesystem::file_size(path, ignored);
        return make_number_value(ignored ? 0.0 : static_cast<double>(size));
    }
    if (function == "message") {
        return make_string_value(last_error_message);
    }
    if (function == "aerror" && !raw_arguments.empty()) {
        return make_number_value(static_cast<double>(aerror_callback(raw_arguments[0])));
    }
    if ((function == "eval" || function == "evaluate") && !arguments.empty()) {
        return eval_expression_callback(value_as_string(arguments[0]));
    }
    if (function == "cursortoxml") {
        const std::string cursor_designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        const std::string output_target = arguments.size() >= 2U ? trim_copy(value_as_string(arguments[1])) : std::string{};
        if (!snapshot_cursor_callback) {
            record_runtime_warning("CURSORTOXML() unavailable (no cursor snapshot callback)");
            if (!output_target.empty() && looks_like_file_path(output_target)) {
                return make_boolean_value(false);
            }
            return make_string_value(std::string{});
        }

        const std::optional<RuntimeSurfaceCursorSnapshot> snapshot = snapshot_cursor_callback(cursor_designator);
        if (!snapshot.has_value()) {
            record_runtime_warning("CURSORTOXML() target cursor not found or unreadable");
            if (!output_target.empty() && looks_like_file_path(output_target)) {
                return make_boolean_value(false);
            }
            return make_string_value(std::string{});
        }

        const std::string xml_payload = serialize_cursor_snapshot_xml(*snapshot);
        if (record_event_callback) {
            record_event_callback(
                "runtime.cursortoxml",
                snapshot->alias + " rows=" + std::to_string(snapshot->rows.size()));
        }

        if (output_target.empty() || !looks_like_file_path(output_target)) {
            return make_string_value(xml_payload);
        }

        std::error_code ignored;
        std::filesystem::path output_path(output_target);
        if (output_path.is_relative()) {
            output_path = std::filesystem::path(default_directory) / output_path;
        }
        output_path = output_path.lexically_normal();
        std::filesystem::create_directories(output_path.parent_path(), ignored);
        std::ofstream output(output_path, std::ios::binary | std::ios::trunc);
        output << xml_payload;
        output.close();
        if (!output.good()) {
            record_runtime_warning("CURSORTOXML() failed to write target path");
            return make_boolean_value(false);
        }
        return make_boolean_value(true);
    }
    if (function == "xmltocursor") {
        if (arguments.size() < 2U) {
            record_runtime_warning("XMLTOCURSOR() requires XML input and destination alias");
            return make_number_value(0.0);
        }
        const std::string xml_or_path = value_as_string(arguments[0]);
        const std::string destination_alias = trim_copy(value_as_string(arguments[1]));
        if (destination_alias.empty()) {
            record_runtime_warning("XMLTOCURSOR() destination alias is required");
            return make_number_value(0.0);
        }
        if (!load_cursor_snapshot_callback) {
            record_runtime_warning("XMLTOCURSOR() unavailable (no cursor load callback)");
            return make_number_value(0.0);
        }

        std::string xml_payload = xml_or_path;
        std::error_code ignored;
        std::filesystem::path probe_path(xml_or_path);
        if (looks_like_file_path(xml_or_path)) {
            if (probe_path.is_relative()) {
                probe_path = std::filesystem::path(default_directory) / probe_path;
            }
            probe_path = probe_path.lexically_normal();
            if (std::filesystem::exists(probe_path, ignored)) {
                std::ifstream input(probe_path, std::ios::binary);
                std::ostringstream buffer;
                buffer << input.rdbuf();
                xml_payload = buffer.str();
            }
        }

        const std::optional<RuntimeSurfaceCursorSnapshot> parsed = parse_cursor_snapshot_xml(xml_payload);
        if (!parsed.has_value()) {
            record_runtime_warning("XMLTOCURSOR() could not parse the provided XML payload");
            return make_number_value(0.0);
        }

        std::optional<std::size_t> loaded_count = load_cursor_snapshot_callback(*parsed, destination_alias);
        if (!loaded_count.has_value()) {
            record_runtime_warning("XMLTOCURSOR() failed to materialize destination cursor");
            return make_number_value(0.0);
        }

        if (record_event_callback) {
            record_event_callback(
                "runtime.xmltocursor",
                destination_alias + " rows=" + std::to_string(*loaded_count));
        }
        return make_number_value(static_cast<double>(*loaded_count));
    }
    if (function == "set" && !arguments.empty()) {
        return make_string_value(set_callback(value_as_string(arguments[0])));
    }
    if (function == "error") {
        return make_number_value(static_cast<double>(last_error_code));
    }
    if (function == "program") {
        return make_string_value(last_error_procedure);
    }
    if (function == "lineno") {
        return make_number_value(static_cast<double>(last_error_line));
    }
    if (function == "version") {
        return make_number_value(arguments.empty() ? 9.0 : 0.0);
    }
    if (function == "on" && !arguments.empty()) {
        const std::string topic = uppercase_copy(value_as_string(arguments[0]));
        if (topic == "ERROR") {
            return make_string_value(error_handler);
        }
        if (topic == "SHUTDOWN") {
            return make_string_value(shutdown_handler);
        }
        return make_string_value(std::string{});
    }
    if (function == "messagebox" && !arguments.empty()) {
        return make_number_value(1.0);
    }

    if (function == "cast" && !arguments.empty()) {
        std::string type_name;
        if (!raw_arguments.empty()) {
            const std::string raw = uppercase_copy(raw_arguments[0]);
            const auto as_pos = raw.rfind(" AS ");
            if (as_pos != std::string::npos) {
                type_name = trim_copy(raw.substr(as_pos + 4U));
            }
        }

        const PrgValue source = arguments[0];
        if (type_name == "INT64" || type_name == "LONGLONG" || type_name == "BIGINT") {
            return make_int64_value(static_cast<std::int64_t>(value_as_number(source)));
        }
        if (type_name == "UINT64" || type_name == "ULONGLONG" || type_name == "UBIGINT") {
            return make_uint64_value(static_cast<std::uint64_t>(value_as_number(source)));
        }
        if (type_name == "INT" || type_name == "INT32" || type_name == "INTEGER" ||
            type_name == "LONG" || type_name == "INT16" || type_name == "SHORT") {
            return make_int64_value(static_cast<std::int64_t>(std::trunc(value_as_number(source))));
        }
        if (type_name == "BYTE" || type_name == "UINT8") {
            return make_uint64_value(
                static_cast<std::uint64_t>(value_as_number(source)) & 0xFFULL);
        }
        if (type_name == "FLOAT" || type_name == "SINGLE") {
            return make_number_value(
                static_cast<double>(static_cast<float>(value_as_number(source))));
        }
        if (type_name == "DOUBLE" || type_name == "NUMERIC") {
            return make_number_value(value_as_number(source));
        }
        if (type_name == "STRING" || type_name == "CHAR" || type_name == "VARCHAR" ||
            type_name == "CHARACTER") {
            return make_string_value(value_as_string(source));
        }
        if (type_name == "LOGICAL" || type_name == "BOOL" || type_name == "BOOLEAN") {
            return make_boolean_value(value_as_bool(source));
        }
        return source;
    }

    if (function == "bitand" && arguments.size() >= 2U) {
        std::uint32_t result = bitwise_value(arguments[0]);
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            result &= bitwise_value(arguments[index]);
        }
        return make_int64_value(signed_bitwise_result(result));
    }
    if (function == "bitor" && arguments.size() >= 2U) {
        std::uint32_t result = bitwise_value(arguments[0]);
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            result |= bitwise_value(arguments[index]);
        }
        return make_int64_value(signed_bitwise_result(result));
    }
    if (function == "bitxor" && arguments.size() >= 2U) {
        std::uint32_t result = bitwise_value(arguments[0]);
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            result ^= bitwise_value(arguments[index]);
        }
        return make_int64_value(signed_bitwise_result(result));
    }
    if (function == "bitnot" && !arguments.empty()) {
        return make_int64_value(signed_bitwise_result(~bitwise_value(arguments[0])));
    }
    if (function == "bitclear" && arguments.size() >= 2U) {
        const std::uint32_t value = bitwise_value(arguments[0]);
        const std::uint32_t mask = 1U << bit_position(arguments[1]);
        return make_int64_value(signed_bitwise_result(value & ~mask));
    }
    if (function == "bitset" && arguments.size() >= 2U) {
        const std::uint32_t value = bitwise_value(arguments[0]);
        const std::uint32_t mask = 1U << bit_position(arguments[1]);
        return make_int64_value(signed_bitwise_result(value | mask));
    }
    if (function == "bittest" && arguments.size() >= 2U) {
        const std::uint32_t value = bitwise_value(arguments[0]);
        const std::uint32_t mask = 1U << bit_position(arguments[1]);
        return make_boolean_value((value & mask) != 0U);
    }
    if (function == "bitlshift" && arguments.size() >= 2U) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const int count = static_cast<int>(value_as_number(arguments[1]));
        return make_int64_value(value << count);
    }
    if (function == "bitrshift" && arguments.size() >= 2U) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const int count = static_cast<int>(value_as_number(arguments[1]));
        return make_int64_value(value >> count);
    }

    if (function == "bintoc" && !arguments.empty()) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const int width = arguments.size() >= 2U
                              ? static_cast<int>(value_as_number(arguments[1]))
                              : 4;
        std::string result(static_cast<std::size_t>(std::max(width, 0)), '\0');
        std::uint64_t unsigned_value = static_cast<std::uint64_t>(value);
        for (int index = 0; index < width; ++index) {
            result[static_cast<std::size_t>(index)] =
                static_cast<char>(unsigned_value & 0xFFU);
            unsigned_value >>= 8;
        }
        return make_string_value(std::move(result));
    }
    if (function == "ctobin" && !arguments.empty()) {
        const std::string source = value_as_string(arguments[0]);
        const std::string type = arguments.size() >= 2U
                                     ? uppercase_copy(value_as_string(arguments[1]))
                                     : std::string("N");
        std::uint64_t unsigned_value = 0U;
        for (std::size_t index = source.size(); index-- > 0U;) {
            unsigned_value = (unsigned_value << 8) |
                             static_cast<std::uint8_t>(source[index]);
        }
        if (type == "N" || type == "INTEGER" || type == "INT") {
            return make_int64_value(static_cast<std::int64_t>(unsigned_value));
        }
        return make_uint64_value(unsigned_value);
    }

    if (function == "numlock" || function == "capslock" || function == "scrolllock") {
        return make_boolean_value(false);
    }
    if (function == "cursorsetprop" || function == "cursorgetprop") {
        return make_number_value(0.0);
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
