// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "visual_asset_editor_support.h"

namespace copperfin::vfp {
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
        visual_asset_text("VisualAssetEditor.Method.NotFound"),
        visual_asset_text("VisualAssetEditor.Method.Ambiguous"),
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
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.RelativeNameRequired")};
        }
        if (normalize_visual_object_name(relative_method_name) == normalize_visual_object_name(requested_method_name)) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.SourceRelativeToSelf")};
        }

        std::size_t relative_index = 0U;
        const auto relative_result = find_unique_visual_method_index(
            methods,
            relative_method_name,
            visual_asset_text("VisualAssetEditor.Method.RelativeNotFound"),
            visual_asset_text("VisualAssetEditor.Method.RelativeAmbiguous"),
            relative_index);
        if (!relative_result.ok) {
            return relative_result;
        }
        insert_index = normalized_placement == "before" ? relative_index : relative_index + 1U;
    } else {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.PlacementUnsupported")};
    }

    methods.insert(
        methods.begin() + static_cast<std::ptrdiff_t>(insert_index),
        moving_method);
    updated_blob = serialize_visual_methods(methods);
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NamesCannotBeEmpty")};
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
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.TargetExists")};
        }
        if (normalized_declaration_name == normalized_requested_name) {
            source_line_indexes.push_back(line_index);
        }
    }

    if (source_line_indexes.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.NotFound")};
    }
    if (source_line_indexes.size() > 1U) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.Ambiguous")};
    }

    const std::string trimmed_line = trim_both(existing_lines[source_line_indexes.front()]);
    std::string declaration_kind;
    std::string declaration_name;
    if (!parse_visual_method_declaration(trimmed_line, declaration_kind, declaration_name)) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Method.DeclarationParseFailed")};
    }
    existing_lines[source_line_indexes.front()] = declaration_kind + " " + trim_both(new_method_name);
    updated_blob = serialize_visual_lines(existing_lines);
    return {.ok = true, .error = {}};
}

}  // namespace copperfin::vfp
