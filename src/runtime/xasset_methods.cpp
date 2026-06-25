#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/localization/localization.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <sstream>
#include <string_view>

namespace copperfin::runtime {

namespace {

std::string xasset_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {}) {
    static const localization::LocalizedCatalog catalog =
        localization::load_catalogs(localization::resolve_catalog_root(), localization::select_locale());
    return catalog.translate(key, placeholders);
}

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

bool looks_like_unresolved_memo(const std::string& value) {
    return value.rfind("<memo block ", 0) == 0;
}

std::string value_or_empty(const copperfin::vfp::DbfRecord& record, std::string_view field_name) {
    const auto* value = find_value(record, field_name);
    if (value == nullptr || looks_like_unresolved_memo(value->display_value)) {
        return {};
    }
    return value->display_value;
}

std::size_t field_index_or_missing(const copperfin::vfp::DbfRecord& record, std::string_view field_name) {
    for (std::size_t index = 0U; index < record.values.size(); ++index) {
        if (record.values[index].field_name == field_name) {
            return index;
        }
    }
    return studio::StudioObjectMissingFieldIndex;
}

std::uint32_t memo_block_number_or_zero(const copperfin::vfp::DbfRecord& record, std::string_view field_name) {
    const auto* value = find_value(record, field_name);
    return value == nullptr ? 0U : value->memo_block_number;
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

std::string filename_stem_for_vfp_path(const std::string& value) {
    const std::size_t separator = value.find_last_of("/\\");
    const std::string leaf = separator == std::string::npos ? value : value.substr(separator + 1U);
    const std::size_t dot = leaf.find_last_of('.');
    if (dot == std::string::npos || dot == 0U) {
        return leaf;
    }
    return leaf.substr(0U, dot);
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
    std::size_t source_field_index,
    std::uint32_t source_memo_block_number,
    const std::string& object_path,
    const std::string& blob) {
    std::vector<XAssetMethod> methods;
    std::string current_name;
    std::size_t current_line_index = studio::StudioObjectMissingLineIndex;
    std::ostringstream current_source;

    auto flush = [&]() {
        if (current_name.empty()) {
            return;
        }
        const auto [owner_path, method_name] = split_owner_and_method(current_name, object_path);
        XAssetMethod method;
        method.record_index = record_index;
        method.source_field_index = source_field_index;
        method.source_line_index = current_line_index;
        method.source_memo_block_number = source_memo_block_number;
        method.object_path = owner_path;
        method.method_name = method_name;
        method.routine_name = sanitize_routine_name("__cf_" + owner_path + "_" + method_name);
        method.source_text = trim_copy(current_source.str());
        if (!method.method_name.empty() && !method.source_text.empty()) {
            methods.push_back(std::move(method));
        }
        current_name.clear();
        current_line_index = studio::StudioObjectMissingLineIndex;
        current_source.str({});
        current_source.clear();
    };

    const std::vector<std::string> lines = split_lines(blob);
    for (std::size_t line_index = 0U; line_index < lines.size(); ++line_index) {
        const auto& raw_line = lines[line_index];
        const std::string line = trim_copy(raw_line);
        if (starts_with_insensitive(line, "PROCEDURE ") || starts_with_insensitive(line, "FUNCTION ")) {
            flush();
            const auto separator = line.find(' ');
            current_name = trim_copy(line.substr(separator + 1U));
            current_line_index = line_index;
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
    std::size_t source_field_index,
    std::uint32_t source_memo_block_number,
    const std::string& object_path,
    const std::string& blob) {
    return parse_methods_blob(record_index, source_field_index, source_memo_block_number, object_path, blob);
}

bool contains_embedded_routine_declaration(const std::string& blob) {
    for (const auto& raw_line : split_lines(blob)) {
        const std::string line = trim_copy(raw_line);
        if (starts_with_insensitive(line, "PROCEDURE ") || starts_with_insensitive(line, "FUNCTION ")) {
            return true;
        }
    }
    return false;
}

XAssetMethod make_wrapped_method(
    std::size_t record_index,
    std::size_t source_field_index,
    std::size_t source_line_index,
    std::uint32_t source_memo_block_number,
    const std::string& object_path,
    std::string method_name,
    std::string source_text) {
    XAssetMethod method;
    method.record_index = record_index;
    method.source_field_index = source_field_index;
    method.source_line_index = source_line_index;
    method.source_memo_block_number = source_memo_block_number;
    method.object_path = object_path;
    method.method_name = std::move(method_name);
    method.routine_name = sanitize_routine_name("__cf_" + object_path + "_" + method.method_name);
    method.source_text = trim_copy(std::move(source_text));
    return method;
}

XAssetActionBinding make_action_binding(
    std::size_t record_index,
    std::size_t title_field_index,
    std::uint32_t title_memo_block_number,
    std::size_t routine_source_field_index,
    std::size_t routine_source_line_index,
    std::uint32_t routine_source_memo_block_number,
    std::string action_id,
    std::string title,
    std::string kind,
    std::string routine_name) {
    XAssetActionBinding binding;
    binding.record_index = record_index;
    binding.title_field_index = title_field_index;
    binding.title_memo_block_number = title_memo_block_number;
    binding.routine_source_field_index = routine_source_field_index;
    binding.routine_source_line_index = routine_source_line_index;
    binding.routine_source_memo_block_number = routine_source_memo_block_number;
    binding.action_id = lowercase_copy(trim_copy(std::move(action_id)));
    binding.title = trim_copy(std::move(title));
    binding.kind = trim_copy(std::move(kind));
    binding.routine_name = trim_copy(std::move(routine_name));
    return binding;
}

std::vector<XAssetMethod> parse_field_as_routines(
    std::size_t record_index,
    const std::string& object_path,
    const std::string& field_role,
    std::size_t source_field_index,
    std::uint32_t source_memo_block_number,
    const std::string& blob) {
    const std::string trimmed = trim_copy(blob);
    if (trimmed.empty()) {
        return {};
    }

    if (contains_embedded_routine_declaration(trimmed)) {
        return parse_embedded_routines(record_index, source_field_index, source_memo_block_number, object_path, trimmed);
    }

    return {make_wrapped_method(record_index, source_field_index, 0U, source_memo_block_number, object_path, field_role, trimmed)};
}

const XAssetMethod* find_method_by_object_method(
    const std::vector<XAssetMethod>& methods,
    const std::string& object_path,
    const std::string& method_name) {
    const std::string normalized_object = lowercase_copy(object_path);
    const std::string normalized_method = lowercase_copy(method_name);
    const auto found = std::find_if(methods.begin(), methods.end(), [&](const XAssetMethod& method) {
        return lowercase_copy(method.object_path) == normalized_object &&
            lowercase_copy(method.method_name) == normalized_method;
    });
    return found == methods.end() ? nullptr : &*found;
}

bool has_method(
    const std::vector<XAssetMethod>& methods,
    const std::string& object_path,
    const std::string& method_name,
    std::string& routine_name) {
    const auto* found = find_method_by_object_method(methods, object_path, method_name);
    if (found == nullptr) {
        return false;
    }
    routine_name = found->routine_name;
    return true;
}

void append_unique_line(std::vector<std::string>& lines, std::string line) {
    line = trim_copy(std::move(line));
    if (line.empty()) {
        return;
    }
    if (std::find(lines.begin(), lines.end(), line) == lines.end()) {
        lines.push_back(std::move(line));
    }
}

void append_lifecycle_step(
    std::vector<XAssetLifecycleStep>& steps,
    XAssetLifecycleStep step) {
    step.command_text = trim_copy(std::move(step.command_text));
    if (step.command_text.empty()) {
        return;
    }
    steps.push_back(std::move(step));
}

void append_if_method_exists(
    const std::vector<XAssetMethod>& methods,
    const std::string& object_path,
    const std::string& method_name,
    std::vector<std::string>& routines,
    std::vector<std::string>& lines,
    std::vector<XAssetLifecycleStep>& steps) {
    const auto* method = find_method_by_object_method(methods, object_path, method_name);
    if (method == nullptr) {
        return;
    }
    if (std::find(routines.begin(), routines.end(), method->routine_name) == routines.end()) {
        routines.push_back(method->routine_name);
    }
    const std::string command_text = "DO " + method->routine_name;
    append_unique_line(lines, command_text);
    append_lifecycle_step(steps, {
        .record_index = method->record_index,
        .source_field_index = method->source_field_index,
        .source_line_index = method->source_line_index,
        .source_memo_block_number = method->source_memo_block_number,
        .kind = "method",
        .command_text = command_text,
        .routine_name = method->routine_name
    });
}

void append_methods(std::vector<XAssetMethod>& destination, const std::vector<XAssetMethod>& methods) {
    destination.insert(destination.end(), methods.begin(), methods.end());
}

bool has_object_type(const studio::StudioDocumentModel& document, int expected_type) {
    return std::any_of(document.table_preview.records.begin(), document.table_preview.records.end(), [&](const copperfin::vfp::DbfRecord& record) {
        return !record.deleted && numeric_value_or_default(record, "OBJTYPE") == expected_type;
    });
}

bool is_menu_item_record(const copperfin::vfp::DbfRecord& record) {
    return numeric_value_or_default(record, "OBJTYPE") == 3;
}

std::optional<std::string> find_first_menu_container_name(const studio::StudioDocumentModel& document) {
    for (const auto& record : document.table_preview.records) {
        if (record.deleted) {
            continue;
        }
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

std::optional<std::string> find_following_submenu_name(
    const studio::StudioDocumentModel& document,
    std::size_t record_position) {
    for (std::size_t index = record_position + 1U; index < document.table_preview.records.size(); ++index) {
        const auto& record = document.table_preview.records[index];
        if (record.deleted) {
            continue;
        }
        if (numeric_value_or_default(record, "OBJTYPE") != 2) {
            continue;
        }

        const std::string name = trim_copy(value_or_empty(record, "NAME"));
        if (!name.empty()) {
            return name;
        }

        const std::string level_name = trim_copy(value_or_empty(record, "LEVELNAME"));
        if (!level_name.empty()) {
            return level_name;
        }
    }
    return std::nullopt;
}

std::optional<std::string> find_menu_action_routine(
    const std::vector<XAssetMethod>& methods,
    const std::string& object_path) {
    std::string routine_name;
    if (has_method(methods, object_path, "command", routine_name)) {
        return routine_name;
    }

    const std::string normalized_object = lowercase_copy(object_path);
    const auto found = std::find_if(methods.begin(), methods.end(), [&](const XAssetMethod& method) {
        if (lowercase_copy(method.object_path) != normalized_object) {
            return false;
        }
        const std::string normalized_method = lowercase_copy(method.method_name);
        return normalized_method != "setup" && normalized_method != "cleanup";
    });
    if (found == methods.end()) {
        return std::nullopt;
    }
    return found->routine_name;
}

const XAssetMethod* find_method_by_routine_name(
    const std::vector<XAssetMethod>& methods,
    const std::string& routine_name) {
    const auto found = std::find_if(methods.begin(), methods.end(), [&](const XAssetMethod& method) {
        return method.routine_name == routine_name;
    });
    return found == methods.end() ? nullptr : &*found;
}

}  // namespace

XAssetExecutableModel build_xasset_executable_model(const studio::StudioDocumentModel& document) {
    XAssetExecutableModel model;
    model.asset_path = document.path;

    if (!document.table_preview_available) {
        model.error = xasset_text("Runtime.XAsset.Error.TablePreviewMissing");
        return model;
    }

    const bool supported_family =
        document.kind == studio::StudioAssetKind::form ||
        document.kind == studio::StudioAssetKind::class_library ||
        document.kind == studio::StudioAssetKind::report ||
        document.kind == studio::StudioAssetKind::label ||
        document.kind == studio::StudioAssetKind::menu;
    if (!supported_family) {
        model.error = xasset_text("Runtime.XAsset.Error.UnsupportedExecutableFamily");
        return model;
    }

    for (std::size_t record_position = 0; record_position < document.table_preview.records.size(); ++record_position) {
        const auto& record = document.table_preview.records[record_position];
        if (record.deleted) {
            continue;
        }
        const std::string object_path = document.kind == studio::StudioAssetKind::menu
            ? build_menu_owner_path(record)
            : build_object_path(record);

        if (document.kind == studio::StudioAssetKind::menu) {
            append_methods(model.methods, parse_field_as_routines(
                record.record_index, object_path, "setup", field_index_or_missing(record, "SETUP"), memo_block_number_or_zero(record, "SETUP"), value_or_empty(record, "SETUP")));
            append_methods(model.methods, parse_field_as_routines(
                record.record_index, object_path, "command", field_index_or_missing(record, "COMMAND"), memo_block_number_or_zero(record, "COMMAND"), value_or_empty(record, "COMMAND")));
            append_methods(model.methods, parse_field_as_routines(
                record.record_index, object_path, "procedure", field_index_or_missing(record, "PROCEDURE"), memo_block_number_or_zero(record, "PROCEDURE"), value_or_empty(record, "PROCEDURE")));
            append_methods(model.methods, parse_field_as_routines(
                record.record_index, object_path, "cleanup", field_index_or_missing(record, "CLEANUP"), memo_block_number_or_zero(record, "CLEANUP"), value_or_empty(record, "CLEANUP")));

            if (is_menu_item_record(record)) {
                std::optional<std::string> action_routine = find_menu_action_routine(model.methods, object_path);
                std::string action_kind = "command";
                if (!action_routine.has_value()) {
                    if (const auto submenu_name = find_following_submenu_name(document, record_position)) {
                        const auto wrapped = make_wrapped_method(
                            record.record_index,
                            studio::StudioObjectMissingFieldIndex,
                            studio::StudioObjectMissingLineIndex,
                            0U,
                            object_path,
                            "activate_popup",
                            "ACTIVATE POPUP " + *submenu_name);
                        action_routine = wrapped.routine_name;
                        action_kind = "submenu";
                        model.methods.push_back(wrapped);
                    }
                }

                if (action_routine.has_value()) {
                    std::string title = trim_copy(value_or_empty(record, "PROMPT"));
                    const std::size_t title_field_index = title.empty()
                        ? studio::StudioObjectMissingFieldIndex
                        : field_index_or_missing(record, "PROMPT");
                    const std::uint32_t title_memo_block_number = title.empty()
                        ? 0U
                        : memo_block_number_or_zero(record, "PROMPT");
                    if (title.empty()) {
                        title = object_path;
                    }
                    const auto* routine_method = find_method_by_routine_name(model.methods, *action_routine);
                    model.actions.push_back(make_action_binding(
                        record.record_index,
                        title_field_index,
                        title_memo_block_number,
                        routine_method == nullptr ? studio::StudioObjectMissingFieldIndex : routine_method->source_field_index,
                        routine_method == nullptr ? studio::StudioObjectMissingLineIndex : routine_method->source_line_index,
                        routine_method == nullptr ? 0U : routine_method->source_memo_block_number,
                        object_path,
                        title,
                        action_kind,
                        *action_routine));
                }
            }
        } else {
            const std::string methods_blob = value_or_empty(record, "METHODS");
            if (!methods_blob.empty()) {
                append_methods(model.methods, parse_methods_blob(
                    record.record_index,
                    field_index_or_missing(record, "METHODS"),
                    memo_block_number_or_zero(record, "METHODS"),
                    object_path,
                    methods_blob));
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
        append_if_method_exists(model.methods, "Dataenvironment", "BeforeOpenTables", model.startup_routines, model.startup_lines, model.startup_steps);
        append_if_method_exists(model.methods, "Dataenvironment", "OpenTables", model.startup_routines, model.startup_lines, model.startup_steps);
        if (!model.root_object_path.empty()) {
            append_if_method_exists(model.methods, model.root_object_path, "Load", model.startup_routines, model.startup_lines, model.startup_steps);
            append_if_method_exists(model.methods, model.root_object_path, "Init", model.startup_routines, model.startup_lines, model.startup_steps);
            if (document.kind == studio::StudioAssetKind::form) {
                append_if_method_exists(model.methods, model.root_object_path, "Activate", model.startup_routines, model.startup_lines, model.startup_steps);
            }

            append_if_method_exists(model.methods, model.root_object_path, "Deactivate", model.shutdown_routines, model.shutdown_lines, model.shutdown_steps);
            append_if_method_exists(model.methods, model.root_object_path, "Destroy", model.shutdown_routines, model.shutdown_lines, model.shutdown_steps);
            append_if_method_exists(model.methods, model.root_object_path, "Unload", model.shutdown_routines, model.shutdown_lines, model.shutdown_steps);
        }
        append_if_method_exists(model.methods, "Dataenvironment", "CloseTables", model.shutdown_routines, model.shutdown_lines, model.shutdown_steps);
        model.runnable_startup = !model.startup_routines.empty();
    } else if (document.kind == studio::StudioAssetKind::menu) {
        for (const auto& method : model.methods) {
            if (lowercase_copy(method.method_name) == "setup") {
                model.startup_routines.push_back(method.routine_name);
                const std::string command_text = "DO " + method.routine_name;
                append_unique_line(model.startup_lines, command_text);
                append_lifecycle_step(model.startup_steps, {
                    .record_index = method.record_index,
                    .source_field_index = method.source_field_index,
                    .source_line_index = method.source_line_index,
                    .source_memo_block_number = method.source_memo_block_number,
                    .kind = "method",
                    .command_text = command_text,
                    .routine_name = method.routine_name
                });
            } else if (lowercase_copy(method.method_name) == "cleanup") {
                model.shutdown_routines.push_back(method.routine_name);
                const std::string command_text = "DO " + method.routine_name;
                append_unique_line(model.shutdown_lines, command_text);
                append_lifecycle_step(model.shutdown_steps, {
                    .record_index = method.record_index,
                    .source_field_index = method.source_field_index,
                    .source_line_index = method.source_line_index,
                    .source_memo_block_number = method.source_memo_block_number,
                    .kind = "method",
                    .command_text = command_text,
                    .routine_name = method.routine_name
                });
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
            model.activation_target = filename_stem_for_vfp_path(document.path);
        }

        if (!model.activation_target.empty()) {
            const std::string command_text = "ACTIVATE " + uppercase_copy(model.activation_kind) + " " + model.activation_target;
            model.startup_lines.push_back(command_text);
            append_lifecycle_step(model.startup_steps, {
                .kind = "activation",
                .command_text = command_text,
                .routine_name = ""
            });
            model.startup_enters_event_loop = true;
        }

        model.runnable_startup = !model.startup_lines.empty();
    } else if (document.kind == studio::StudioAssetKind::report || document.kind == studio::StudioAssetKind::label) {
        const std::string quoted_path = "'" + document.path + "'";
        std::string command_text;
        if (document.kind == studio::StudioAssetKind::report) {
            command_text = "REPORT FORM " + quoted_path + " PREVIEW";
        } else {
            command_text = "LABEL FORM " + quoted_path + " PREVIEW";
        }
        model.startup_lines.push_back(command_text);
        append_lifecycle_step(model.startup_steps, {
            .kind = "preview",
            .command_text = command_text,
            .routine_name = ""
        });
        model.startup_enters_event_loop = true;
        model.runnable_startup = true;
    }

    if (document.kind != studio::StudioAssetKind::menu) {
        for (const auto& method : model.methods) {
            std::string title = method.object_path;
            if (!title.empty()) {
                title += ".";
            }
            title += method.method_name;

            model.actions.push_back(make_action_binding(
                method.record_index,
                studio::StudioObjectMissingFieldIndex,
                0U,
                method.source_field_index,
                method.source_line_index,
                method.source_memo_block_number,
                title,
                title,
                "method",
                method.routine_name));
        }
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
    for (const auto& line : model.shutdown_lines) {
        stream << line << "\n";
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
