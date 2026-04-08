#include "copperfin/runtime/xasset_methods.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string_view>

namespace copperfin::runtime {

namespace {

const copperfin::vfp::DbfRecordValue* find_value(
    const copperfin::vfp::DbfRecord& record,
    std::string_view field_name) {
    for (const auto& value : record.values) {
        if (value.field_name == field_name) {
            return &value;
        }
    }
    return nullptr;
}

std::string value_or_empty(const copperfin::vfp::DbfRecord& record, std::string_view field_name) {
    const auto* value = find_value(record, field_name);
    if (value == nullptr || value->display_value == "<memo block 0>") {
        return {};
    }
    return value->display_value;
}

std::string trim_copy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }
    return value;
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

bool starts_with_insensitive(const std::string& value, const std::string& prefix) {
    if (value.size() < prefix.size()) {
        return false;
    }
    for (std::size_t index = 0; index < prefix.size(); ++index) {
        if (std::tolower(static_cast<unsigned char>(value[index])) !=
            std::tolower(static_cast<unsigned char>(prefix[index]))) {
            return false;
        }
    }
    return true;
}

std::string sanitize_routine_name(std::string value) {
    for (char& ch : value) {
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '_') {
            continue;
        }
        ch = '_';
    }
    return value;
}

std::string build_object_path(const copperfin::vfp::DbfRecord& record) {
    const std::string object_name = trim_copy(value_or_empty(record, "OBJNAME"));
    const std::string parent = trim_copy(value_or_empty(record, "PARENT"));
    if (object_name.empty()) {
        return parent;
    }
    if (parent.empty()) {
        return object_name;
    }
    return parent + "." + object_name;
}

int numeric_value_or_default(const copperfin::vfp::DbfRecord& record, std::string_view field_name, int fallback = 0) {
    const std::string value = trim_copy(value_or_empty(record, field_name));
    if (value.empty()) {
        return fallback;
    }
    try {
        return std::stoi(value);
    } catch (...) {
        return fallback;
    }
}

std::string build_menu_owner_path(const copperfin::vfp::DbfRecord& record) {
    const int object_type = numeric_value_or_default(record, "OBJTYPE");
    const std::string name = trim_copy(value_or_empty(record, "NAME"));
    const std::string level_name = trim_copy(value_or_empty(record, "LEVELNAME"));
    const std::string item_number = trim_copy(value_or_empty(record, "ITEMNUM"));
    const std::string prompt = trim_copy(value_or_empty(record, "PROMPT"));

    if (object_type == 4) {
        return name.empty() ? "shortcut" : name;
    }
    if (object_type == 5) {
        return name.empty() ? "sdi_menu" : name;
    }
    if (object_type == 1) {
        return name.empty() ? "menu" : name;
    }
    if (object_type == 2) {
        if (!name.empty()) {
            return name;
        }
        if (!level_name.empty()) {
            return level_name;
        }
        return "submenu";
    }
    if (!name.empty()) {
        return name;
    }
    if (!level_name.empty() && !item_number.empty() && item_number != "0" && item_number != "00" && item_number != "000") {
        return level_name + ".item" + item_number;
    }
    if (!level_name.empty()) {
        return level_name;
    }
    if (!prompt.empty()) {
        return prompt;
    }
    return "menu_record_" + std::to_string(record.record_index);
}

std::pair<std::string, std::string> split_owner_and_method(const std::string& raw_name, const std::string& record_object_path) {
    const auto separator = raw_name.rfind('.');
    if (separator == std::string::npos) {
        return {record_object_path, raw_name};
    }

    const std::string relative_owner = trim_copy(raw_name.substr(0U, separator));
    const std::string method_name = trim_copy(raw_name.substr(separator + 1U));
    if (relative_owner.empty()) {
        return {record_object_path, method_name};
    }
    if (record_object_path.empty()) {
        return {relative_owner, method_name};
    }
    return {record_object_path + "." + relative_owner, method_name};
}

std::vector<std::string> split_lines(const std::string& text) {
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

std::vector<XAssetMethod> parse_methods_blob(
    std::size_t record_index,
    const std::string& object_path,
    const std::string& blob) {
    std::vector<XAssetMethod> methods;
    std::string current_name;
    std::ostringstream current_source;

    auto flush = [&]() {
        if (current_name.empty()) {
            return;
        }
        const auto [owner_path, method_name] = split_owner_and_method(current_name, object_path);
        XAssetMethod method;
        method.record_index = record_index;
        method.object_path = owner_path;
        method.method_name = method_name;
        method.routine_name = sanitize_routine_name("__cf_" + owner_path + "_" + method_name);
        method.source_text = trim_copy(current_source.str());
        if (!method.method_name.empty() && !method.source_text.empty()) {
            methods.push_back(std::move(method));
        }
        current_name.clear();
        current_source.str({});
        current_source.clear();
    };

    for (const auto& raw_line : split_lines(blob)) {
        const std::string line = trim_copy(raw_line);
        if (starts_with_insensitive(line, "PROCEDURE ") || starts_with_insensitive(line, "FUNCTION ")) {
            flush();
            const auto separator = line.find(' ');
            current_name = trim_copy(line.substr(separator + 1U));
            continue;
        }
        if (starts_with_insensitive(line, "ENDPROC") || starts_with_insensitive(line, "ENDFUNC") || starts_with_insensitive(line, "END FUNC")) {
            flush();
            continue;
        }
        if (!current_name.empty()) {
            current_source << raw_line << "\n";
        }
    }

    flush();
    return methods;
}

std::vector<XAssetMethod> parse_embedded_routines(
    std::size_t record_index,
    const std::string& object_path,
    const std::string& blob) {
    return parse_methods_blob(record_index, object_path, blob);
}

XAssetMethod make_wrapped_method(
    std::size_t record_index,
    const std::string& object_path,
    std::string method_name,
    std::string source_text) {
    XAssetMethod method;
    method.record_index = record_index;
    method.object_path = object_path;
    method.method_name = std::move(method_name);
    method.routine_name = sanitize_routine_name("__cf_" + object_path + "_" + method.method_name);
    method.source_text = trim_copy(std::move(source_text));
    return method;
}

std::vector<XAssetMethod> parse_field_as_routines(
    std::size_t record_index,
    const std::string& object_path,
    const std::string& field_role,
    const std::string& blob) {
    const std::string trimmed = trim_copy(blob);
    if (trimmed.empty()) {
        return {};
    }

    if (starts_with_insensitive(trimmed, "PROCEDURE ") || starts_with_insensitive(trimmed, "FUNCTION ")) {
        return parse_embedded_routines(record_index, object_path, trimmed);
    }

    return {make_wrapped_method(record_index, object_path, field_role, trimmed)};
}

bool has_method(
    const std::vector<XAssetMethod>& methods,
    const std::string& object_path,
    const std::string& method_name,
    std::string& routine_name) {
    const std::string normalized_object = lowercase_copy(object_path);
    const std::string normalized_method = lowercase_copy(method_name);
    const auto found = std::find_if(methods.begin(), methods.end(), [&](const XAssetMethod& method) {
        return lowercase_copy(method.object_path) == normalized_object &&
            lowercase_copy(method.method_name) == normalized_method;
    });
    if (found == methods.end()) {
        return false;
    }
    routine_name = found->routine_name;
    return true;
}

void append_methods(std::vector<XAssetMethod>& destination, const std::vector<XAssetMethod>& methods) {
    destination.insert(destination.end(), methods.begin(), methods.end());
}

bool has_object_type(const studio::StudioDocumentModel& document, int expected_type) {
    return std::any_of(document.table_preview.records.begin(), document.table_preview.records.end(), [&](const copperfin::vfp::DbfRecord& record) {
        return numeric_value_or_default(record, "OBJTYPE") == expected_type;
    });
}

std::optional<std::string> find_first_menu_container_name(const studio::StudioDocumentModel& document) {
    for (const auto& record : document.table_preview.records) {
        if (numeric_value_or_default(record, "OBJTYPE") != 2) {
            continue;
        }
        const std::string name = trim_copy(value_or_empty(record, "NAME"));
        if (!name.empty()) {
            return name;
        }
    }
    return std::nullopt;
}

}  // namespace

XAssetExecutableModel build_xasset_executable_model(const studio::StudioDocumentModel& document) {
    XAssetExecutableModel model;
    model.asset_path = document.path;

    if (!document.table_preview_available) {
        model.error = "Asset does not have a table preview.";
        return model;
    }

    const bool supported_family =
        document.kind == studio::StudioAssetKind::form ||
        document.kind == studio::StudioAssetKind::class_library ||
        document.kind == studio::StudioAssetKind::report ||
        document.kind == studio::StudioAssetKind::label ||
        document.kind == studio::StudioAssetKind::menu;
    if (!supported_family) {
        model.error = "Asset family is not a supported executable xAsset.";
        return model;
    }

    for (const auto& record : document.table_preview.records) {
        const std::string object_path = document.kind == studio::StudioAssetKind::menu
            ? build_menu_owner_path(record)
            : build_object_path(record);

        if (document.kind == studio::StudioAssetKind::menu) {
            append_methods(model.methods, parse_field_as_routines(record.record_index, object_path, "setup", value_or_empty(record, "SETUP")));
            append_methods(model.methods, parse_field_as_routines(record.record_index, object_path, "command", value_or_empty(record, "COMMAND")));
            append_methods(model.methods, parse_field_as_routines(record.record_index, object_path, "procedure", value_or_empty(record, "PROCEDURE")));
            append_methods(model.methods, parse_field_as_routines(record.record_index, object_path, "cleanup", value_or_empty(record, "CLEANUP")));
        } else {
            const std::string methods_blob = value_or_empty(record, "METHODS");
            if (!methods_blob.empty()) {
                append_methods(model.methods, parse_methods_blob(record.record_index, object_path, methods_blob));
            }

            const std::string baseclass = lowercase_copy(trim_copy(value_or_empty(record, "BASECLASS")));
            const std::string platform = lowercase_copy(trim_copy(value_or_empty(record, "PLATFORM")));
            if (model.root_object_path.empty() &&
                platform != "comment" &&
                baseclass != "dataenvironment" &&
                !object_path.empty() &&
                trim_copy(value_or_empty(record, "PARENT")).empty()) {
                model.root_object_path = object_path;
            }
        }
    }

    if (document.kind == studio::StudioAssetKind::form || document.kind == studio::StudioAssetKind::class_library) {
        std::string routine_name;
        if (has_method(model.methods, "Dataenvironment", "BeforeOpenTables", routine_name)) {
            model.startup_routines.push_back(routine_name);
        }
        if (!model.root_object_path.empty() && has_method(model.methods, model.root_object_path, "Load", routine_name)) {
            model.startup_routines.push_back(routine_name);
        }
        if (!model.root_object_path.empty() && has_method(model.methods, model.root_object_path, "Init", routine_name)) {
            model.startup_routines.push_back(routine_name);
        }
        for (const auto& startup_routine : model.startup_routines) {
            model.startup_lines.push_back("DO " + startup_routine);
        }
        model.runnable_startup = !model.startup_routines.empty();
    } else if (document.kind == studio::StudioAssetKind::menu) {
        for (const auto& method : model.methods) {
            if (lowercase_copy(method.method_name) == "setup") {
                model.startup_routines.push_back(method.routine_name);
                model.startup_lines.push_back("DO " + method.routine_name);
            }
        }

        const bool shortcut_menu = has_object_type(document, 4);
        model.activation_kind = shortcut_menu ? "popup" : "menu";
        if (shortcut_menu) {
            if (const auto first_container_name = find_first_menu_container_name(document)) {
                model.activation_target = *first_container_name;
            } else {
                model.activation_target = "shortcut";
            }
        } else {
            model.activation_target = std::filesystem::path(document.path).stem().string();
        }

        if (!model.activation_target.empty()) {
            model.startup_lines.push_back("ACTIVATE " + uppercase_copy(model.activation_kind) + " " + model.activation_target);
            model.startup_enters_event_loop = true;
        }

        model.runnable_startup = !model.startup_lines.empty();
    }

    model.ok = true;
    return model;
}

std::string build_xasset_bootstrap_source(const XAssetExecutableModel& model, bool include_read_events) {
    std::ostringstream stream;
    stream << "* Copperfin generated xAsset bootstrap\n";
    for (const auto& line : model.startup_lines) {
        stream << line << "\n";
    }
    if (include_read_events && !model.startup_enters_event_loop) {
        stream << "READ EVENTS\n";
    }
    stream << "RETURN\n";

    for (const auto& method : model.methods) {
        stream << "PROCEDURE " << method.routine_name << "\n";
        if (!method.source_text.empty()) {
            stream << method.source_text;
            if (method.source_text.back() != '\n') {
                stream << "\n";
            }
        }
        stream << "ENDPROC\n";
    }

    return stream.str();
}

}  // namespace copperfin::runtime
