#include "copperfin/runtime/xasset_methods.h"

#include <algorithm>
#include <cctype>
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
        const std::string object_path = build_object_path(record);
        const std::string methods_blob = value_or_empty(record, "METHODS");
        if (!methods_blob.empty()) {
            auto parsed = parse_methods_blob(record.record_index, object_path, methods_blob);
            model.methods.insert(model.methods.end(), parsed.begin(), parsed.end());
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
        model.runnable_startup = !model.startup_routines.empty();
    }

    model.ok = true;
    return model;
}

std::string build_xasset_bootstrap_source(const XAssetExecutableModel& model, bool include_read_events) {
    std::ostringstream stream;
    stream << "* Copperfin generated xAsset bootstrap\n";
    for (const auto& routine_name : model.startup_routines) {
        stream << "DO " << routine_name << "\n";
    }
    if (include_read_events) {
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
