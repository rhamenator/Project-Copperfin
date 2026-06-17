#include "copperfin/studio/document_model.h"

#include "copperfin/vfp/visual_asset_editor.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <optional>
#include <string_view>

namespace copperfin::studio {

namespace {

std::string filename_of(const std::string& path) {
    const std::size_t separator = path.find_last_of("/\\");
    return separator == std::string::npos ? path : path.substr(separator + 1U);
}

const vfp::DbfRecordValue* find_value(const vfp::DbfRecord& record, std::string_view field_name) {
    for (const auto& value : record.values) {
        if (value.field_name == field_name) {
            return &value;
        }
    }
    return nullptr;
}

struct FieldSelection {
    std::string value{};
    std::size_t field_index = StudioObjectMissingFieldIndex;
    std::uint32_t memo_block_number = 0;
};

bool looks_like_unresolved_memo(const std::string& value) {
    return value.rfind("<memo block ", 0) == 0;
}

std::string usable_display_value(const vfp::DbfRecordValue& value) {
    return looks_like_unresolved_memo(value.display_value) ? std::string() : value.display_value;
}

std::string trim_copy(std::string text) {
    text.erase(text.begin(), std::find_if(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.pop_back();
    }
    return text;
}

std::string value_or_empty(const vfp::DbfRecord& record, std::string_view field_name) {
    const auto* value = find_value(record, field_name);
    if (value == nullptr) {
        return {};
    }
    return usable_display_value(*value);
}

std::optional<std::size_t> find_field_index(const vfp::DbfRecord& record, std::string_view field_name) {
    for (std::size_t index = 0U; index < record.values.size(); ++index) {
        if (record.values[index].field_name == field_name) {
            return index;
        }
    }
    return std::nullopt;
}

std::size_t field_index_or_missing(const vfp::DbfRecord& record, std::string_view field_name) {
    return find_field_index(record, field_name).value_or(StudioObjectMissingFieldIndex);
}

std::uint32_t memo_block_number_or_zero(const vfp::DbfRecord& record, std::string_view field_name) {
    const auto* value = find_value(record, field_name);
    return value == nullptr ? 0U : value->memo_block_number;
}

FieldSelection first_non_empty_selection(const vfp::DbfRecord& record, std::initializer_list<std::string_view> field_names) {
    for (const auto field_name : field_names) {
        const auto* value = find_value(record, field_name);
        if (value != nullptr) {
            const std::string usable_value = trim_copy(usable_display_value(*value));
            if (!usable_value.empty()) {
                return {
                    .value = usable_value,
                    .field_index = field_index_or_missing(record, field_name),
                    .memo_block_number = value->memo_block_number
                };
            }
        }
    }
    return {};
}

std::string lowercase_ascii(std::string_view value) {
    std::string lowered;
    lowered.reserve(value.size());
    for (const unsigned char ch : value) {
        lowered.push_back(static_cast<char>(std::tolower(ch)));
    }
    return lowered;
}

std::string first_non_empty(const vfp::DbfRecord& record, std::initializer_list<std::string_view> field_names) {
    for (const auto field_name : field_names) {
        const std::string value = trim_copy(value_or_empty(record, field_name));
        if (!value.empty()) {
            return value;
        }
    }
    return {};
}

std::optional<int> parse_scaled_int(const vfp::DbfRecord& record, std::string_view field_name) {
    const std::string raw = trim_copy(value_or_empty(record, field_name));
    if (raw.empty()) {
        return std::nullopt;
    }

    const auto dot = raw.find('.');
    const std::string integer_portion = dot == std::string::npos ? raw : raw.substr(0U, dot);
    if (integer_portion.empty()) {
        return std::nullopt;
    }

    int value = 0;
    const auto [ptr, ec] = std::from_chars(integer_portion.data(), integer_portion.data() + integer_portion.size(), value);
    if (ec != std::errc() || ptr != (integer_portion.data() + integer_portion.size())) {
        return std::nullopt;
    }

    return value;
}

bool supports_visual_property_blob(const StudioDocumentModel& document) {
    return document.kind == StudioAssetKind::form || document.kind == StudioAssetKind::class_library;
}

bool has_method_like_symbol(std::string_view symbol) {
    const std::size_t separator = symbol.find('.');
    return separator != std::string_view::npos &&
        separator > 0U &&
        (separator + 1U) < symbol.size();
}

std::string lowercase_copy(std::string_view text) {
    std::string lowered(text);
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lowered;
}

bool has_data_environment_symbol(std::string_view symbol) {
    const std::size_t separator = symbol.find('.');
    const std::string owner = lowercase_copy(symbol.substr(0U, separator));
    return owner == "dataenvironment" || owner == "data_environment";
}

bool is_data_environment_token(std::string_view value) {
    const std::string normalized = lowercase_copy(trim_copy(std::string(value)));
    return normalized == "dataenvironment" || normalized == "data_environment";
}

bool selected_record_is_data_environment(const StudioDocumentModel& document, std::size_t record_index) {
    if (!document.table_preview_available) {
        return false;
    }

    const auto record = std::find_if(
        document.table_preview.records.begin(),
        document.table_preview.records.end(),
        [&](const vfp::DbfRecord& preview_record) {
            return preview_record.record_index == record_index;
        });
    if (record == document.table_preview.records.end()) {
        return false;
    }

    return is_data_environment_token(value_or_empty(*record, "OBJNAME")) ||
        is_data_environment_token(value_or_empty(*record, "NAME")) ||
        is_data_environment_token(value_or_empty(*record, "BASECLASS")) ||
        is_data_environment_token(value_or_empty(*record, "CLASS"));
}

std::vector<StudioDesignerContextResult> default_designer_contexts_for_kind(StudioAssetKind kind) {
    switch (kind) {
        case StudioAssetKind::form:
            return {
                studio_designer_context_for_selection({
                    .selection_context = StudioEditorSelectionContext::visual_object
                })
            };
        case StudioAssetKind::class_library:
            return {
                studio_designer_context_for_selection({
                    .selection_context = StudioEditorSelectionContext::class_designer
                })
            };
        case StudioAssetKind::report:
            return {
                studio_designer_context_for_selection({
                    .selection_context = StudioEditorSelectionContext::report_expression
                })
            };
        case StudioAssetKind::label:
            return {
                studio_designer_context_for_selection({
                    .selection_context = StudioEditorSelectionContext::label_expression
                })
            };
        case StudioAssetKind::project:
            return {
                studio_designer_context_for_selection({
                    .selection_context = StudioEditorSelectionContext::project_item
                })
            };
        case StudioAssetKind::menu:
            return {
                studio_designer_context_for_selection({
                    .selection_context = StudioEditorSelectionContext::menu_item
                })
            };
        case StudioAssetKind::table:
        case StudioAssetKind::database_container:
            return {
                studio_designer_context_for_selection({
                    .selection_context = StudioEditorSelectionContext::data_environment
                })
            };
        case StudioAssetKind::index:
        case StudioAssetKind::program:
        case StudioAssetKind::header:
        case StudioAssetKind::unknown:
            return {};
    }
    return {};
}

std::vector<StudioDesignerContextResult> default_designer_contexts_for_request(
    const StudioDocumentModel& document,
    std::size_t record_index,
    std::string_view symbol) {
    const StudioAssetKind kind = document.kind;
    if ((kind == StudioAssetKind::form || kind == StudioAssetKind::class_library) && has_data_environment_symbol(symbol)) {
        return {
            studio_designer_context_for_selection({
                .selection_context = StudioEditorSelectionContext::data_environment
            })
        };
    }
    if ((kind == StudioAssetKind::form || kind == StudioAssetKind::class_library) &&
        selected_record_is_data_environment(document, record_index)) {
        return {
            studio_designer_context_for_selection({
                .selection_context = StudioEditorSelectionContext::data_environment
            })
        };
    }
    if ((kind == StudioAssetKind::form || kind == StudioAssetKind::class_library) && has_method_like_symbol(symbol)) {
        return {
            studio_designer_context_for_selection({
                .selection_context = StudioEditorSelectionContext::visual_method
            })
        };
    }
    return default_designer_contexts_for_kind(kind);
}

std::vector<StudioDesignerContextResult> requested_designer_contexts(
    const std::vector<StudioEditorSelectionContext>& selection_contexts) {
    std::vector<StudioDesignerContextResult> designer_contexts;
    for (const auto selection_context : selection_contexts) {
        const auto existing = std::find_if(
            designer_contexts.begin(),
            designer_contexts.end(),
            [&](const StudioDesignerContextResult& context) {
                return context.selection_context == selection_context;
            });
        if (existing != designer_contexts.end()) {
            continue;
        }

        designer_contexts.push_back(studio_designer_context_for_selection({
            .selection_context = selection_context
        }));
    }
    return designer_contexts;
}

void append_property_snapshots(
    const std::vector<vfp::VisualPropertyAssignment>& assignments,
    std::vector<StudioPropertySnapshot>& properties,
    std::size_t source_field_index,
    std::uint32_t source_memo_block_number) {
    for (const auto& assignment : assignments) {
        if (assignment.name.empty()) {
            continue;
        }

        const auto existing = std::find_if(properties.begin(), properties.end(), [&](const StudioPropertySnapshot& property) {
            return property.name == assignment.name;
        });
        if (existing != properties.end()) {
            continue;
        }

        properties.push_back({
            .name = assignment.name,
            .field_index = source_field_index,
            .type = 'P',
            .is_null = assignment.value.empty(),
            .derived_from_property_blob = true,
            .source_line_index = assignment.source_line_index,
            .memo_block_number = source_memo_block_number,
            .value = assignment.value
        });
    }
}

const StudioObjectSnapshot* find_object_by_record_index(
    const std::vector<StudioObjectSnapshot>& objects,
    std::size_t record_index) {
    const auto object = std::find_if(
        objects.begin(),
        objects.end(),
        [&](const StudioObjectSnapshot& candidate) {
            return candidate.record_index == record_index;
        });
    return object == objects.end() ? nullptr : &*object;
}

std::string build_object_path(
    const StudioObjectSnapshot& object,
    const std::vector<StudioObjectSnapshot>& objects) {
    if (object.object_name.empty()) {
        return {};
    }

    std::vector<std::string> names{object.object_name};
    std::size_t parent_record_index = object.parent_record_index;
    for (std::size_t depth = 0U;
         parent_record_index != StudioObjectMissingRecordIndex && depth < objects.size();
         ++depth) {
        const auto* parent = find_object_by_record_index(objects, parent_record_index);
        if (parent == nullptr || parent->object_name.empty()) {
            break;
        }
        names.push_back(parent->object_name);
        parent_record_index = parent->parent_record_index;
    }

    std::reverse(names.begin(), names.end());
    std::string path;
    for (std::size_t index = 0U; index < names.size(); ++index) {
        if (index != 0U) {
            path += ".";
        }
        path += names[index];
    }
    return path;
}

std::size_t build_object_depth(
    const StudioObjectSnapshot& object,
    const std::vector<StudioObjectSnapshot>& objects) {
    std::size_t depth = 0U;
    std::size_t parent_record_index = object.parent_record_index;
    while (parent_record_index != StudioObjectMissingRecordIndex && depth < objects.size()) {
        const auto* parent = find_object_by_record_index(objects, parent_record_index);
        if (parent == nullptr) {
            break;
        }
        ++depth;
        parent_record_index = parent->parent_record_index;
    }
    return depth;
}

std::vector<std::size_t> build_ancestor_record_indexes(
    const StudioObjectSnapshot& object,
    const std::vector<StudioObjectSnapshot>& objects) {
    std::vector<std::size_t> record_indexes;
    std::size_t parent_record_index = object.parent_record_index;
    while (parent_record_index != StudioObjectMissingRecordIndex && record_indexes.size() < objects.size()) {
        const auto* parent = find_object_by_record_index(objects, parent_record_index);
        if (parent == nullptr) {
            break;
        }
        record_indexes.push_back(parent->record_index);
        parent_record_index = parent->parent_record_index;
    }
    std::reverse(record_indexes.begin(), record_indexes.end());
    return record_indexes;
}

void assign_sibling_order(
    StudioObjectSnapshot& object,
    const std::vector<StudioObjectSnapshot>& objects) {
    object.sibling_index = 0U;
    object.sibling_count = 0U;
    for (const auto& candidate : objects) {
        if (candidate.parent_record_index != object.parent_record_index) {
            continue;
        }
        if (candidate.record_index == object.record_index) {
            object.sibling_index = object.sibling_count;
        }
        ++object.sibling_count;
    }
}

}  // namespace

StudioAssetKind studio_asset_kind_from_vfp_family(vfp::AssetFamily family) {
    switch (family) {
        case vfp::AssetFamily::project:
            return StudioAssetKind::project;
        case vfp::AssetFamily::form:
            return StudioAssetKind::form;
        case vfp::AssetFamily::class_library:
            return StudioAssetKind::class_library;
        case vfp::AssetFamily::report:
            return StudioAssetKind::report;
        case vfp::AssetFamily::label:
            return StudioAssetKind::label;
        case vfp::AssetFamily::menu:
            return StudioAssetKind::menu;
        case vfp::AssetFamily::index:
            return StudioAssetKind::index;
        case vfp::AssetFamily::table:
            return StudioAssetKind::table;
        case vfp::AssetFamily::database_container:
            return StudioAssetKind::database_container;
        case vfp::AssetFamily::program:
            return StudioAssetKind::program;
        case vfp::AssetFamily::header:
            return StudioAssetKind::header;
        case vfp::AssetFamily::unknown:
            return StudioAssetKind::unknown;
    }
    return StudioAssetKind::unknown;
}

const char* studio_asset_kind_name(StudioAssetKind kind) {
    switch (kind) {
        case StudioAssetKind::unknown:
            return "unknown";
        case StudioAssetKind::project:
            return "project";
        case StudioAssetKind::form:
            return "form";
        case StudioAssetKind::class_library:
            return "class_library";
        case StudioAssetKind::report:
            return "report";
        case StudioAssetKind::label:
            return "label";
        case StudioAssetKind::menu:
            return "menu";
        case StudioAssetKind::index:
            return "index";
        case StudioAssetKind::table:
            return "table";
        case StudioAssetKind::database_container:
            return "database_container";
        case StudioAssetKind::program:
            return "program";
        case StudioAssetKind::header:
            return "header";
    }
    return "unknown";
}

std::string infer_sidecar_path(const std::string& path, StudioAssetKind kind) {
    std::filesystem::path file_path(path);
    switch (kind) {
        case StudioAssetKind::project:
            return file_path.replace_extension(".pjt").string();
        case StudioAssetKind::form:
            return file_path.replace_extension(".sct").string();
        case StudioAssetKind::class_library:
            return file_path.replace_extension(".vct").string();
        case StudioAssetKind::report:
            return file_path.replace_extension(".frt").string();
        case StudioAssetKind::label:
            return file_path.replace_extension(".lbt").string();
        case StudioAssetKind::menu:
            return file_path.replace_extension(".mnt").string();
        case StudioAssetKind::index:
        case StudioAssetKind::table:
        case StudioAssetKind::database_container:
        case StudioAssetKind::program:
        case StudioAssetKind::header:
        case StudioAssetKind::unknown:
            return {};
    }
    return {};
}

std::vector<StudioObjectSnapshot> build_object_snapshot(const StudioDocumentModel& document) {
    std::vector<StudioObjectSnapshot> objects;
    if (!document.table_preview_available) {
        return objects;
    }

    objects.reserve(document.table_preview.records.size());
    for (const auto& record : document.table_preview.records) {
        StudioObjectSnapshot snapshot;
        snapshot.record_index = record.record_index;
        snapshot.deleted = record.deleted;
        snapshot.objtype_code = parse_scaled_int(record, "OBJTYPE").value_or(0);
        snapshot.objtype_field_index = field_index_or_missing(record, "OBJTYPE");
        snapshot.objtype_memo_block_number = memo_block_number_or_zero(record, "OBJTYPE");
        snapshot.objcode_code = parse_scaled_int(record, "OBJCODE").value_or(0);
        snapshot.objcode_field_index = field_index_or_missing(record, "OBJCODE");
        snapshot.objcode_memo_block_number = memo_block_number_or_zero(record, "OBJCODE");
        snapshot.platform = first_non_empty(record, {"PLATFORM"});
        snapshot.platform_field_index = field_index_or_missing(record, "PLATFORM");
        snapshot.platform_memo_block_number = memo_block_number_or_zero(record, "PLATFORM");
        const FieldSelection object_name = first_non_empty_selection(record, {"OBJNAME", "NAME"});
        snapshot.object_name = object_name.value;
        snapshot.object_name_field_index = object_name.field_index;
        snapshot.object_name_memo_block_number = object_name.memo_block_number;
        const FieldSelection unique_id = first_non_empty_selection(record, {"UNIQUEID"});
        snapshot.unique_id = unique_id.value;
        snapshot.unique_id_field_index = unique_id.field_index;
        snapshot.unique_id_memo_block_number = unique_id.memo_block_number;
        const FieldSelection parent_name = first_non_empty_selection(record, {"PARENT", "PARENTID"});
        snapshot.parent_name = parent_name.value;
        snapshot.parent_name_field_index = parent_name.field_index;
        snapshot.parent_name_memo_block_number = parent_name.memo_block_number;
        const FieldSelection class_name = first_non_empty_selection(record, {"CLASS"});
        snapshot.class_name = class_name.value;
        snapshot.class_name_field_index = class_name.field_index;
        snapshot.class_name_memo_block_number = class_name.memo_block_number;
        const FieldSelection baseclass_name = first_non_empty_selection(record, {"BASECLASS"});
        snapshot.baseclass_name = baseclass_name.value;
        snapshot.baseclass_name_field_index = baseclass_name.field_index;
        snapshot.baseclass_name_memo_block_number = baseclass_name.memo_block_number;
        if (document.kind == StudioAssetKind::menu) {
            snapshot.menu_prompt = first_non_empty(record, {"PROMPT"});
            snapshot.menu_prompt_field_index = field_index_or_missing(record, "PROMPT");
            snapshot.menu_prompt_memo_block_number = memo_block_number_or_zero(record, "PROMPT");
            snapshot.menu_level_name = first_non_empty(record, {"LEVELNAME"});
            snapshot.menu_level_name_field_index = field_index_or_missing(record, "LEVELNAME");
            snapshot.menu_level_name_memo_block_number = memo_block_number_or_zero(record, "LEVELNAME");
            snapshot.menu_command = first_non_empty(record, {"COMMAND"});
            snapshot.menu_command_field_index = field_index_or_missing(record, "COMMAND");
            snapshot.menu_command_memo_block_number = memo_block_number_or_zero(record, "COMMAND");
            snapshot.menu_message = first_non_empty(record, {"MESSAGE"});
            snapshot.menu_message_field_index = field_index_or_missing(record, "MESSAGE");
            snapshot.menu_message_memo_block_number = memo_block_number_or_zero(record, "MESSAGE");
        }
        switch (document.kind) {
            case StudioAssetKind::report:
            case StudioAssetKind::label:
                {
                    const FieldSelection title = first_non_empty_selection(record, {"EXPR", "NAME", "UNIQUEID"});
                    snapshot.title = title.value;
                    snapshot.title_field_index = title.field_index;
                    snapshot.title_memo_block_number = title.memo_block_number;
                    const FieldSelection subtitle = first_non_empty_selection(record, {"OBJTYPE", "OBJCODE", "FONTFACE", "PLATFORM"});
                    snapshot.subtitle = subtitle.value;
                    snapshot.subtitle_field_index = subtitle.field_index;
                    snapshot.subtitle_memo_block_number = subtitle.memo_block_number;
                }
                break;
            case StudioAssetKind::menu:
                {
                    const FieldSelection title = first_non_empty_selection(record, {"PROMPT", "NAME", "LEVELNAME"});
                    snapshot.title = title.value;
                    snapshot.title_field_index = title.field_index;
                    snapshot.title_memo_block_number = title.memo_block_number;
                    const FieldSelection subtitle = first_non_empty_selection(record, {"LEVELNAME", "OBJTYPE", "OBJCODE"});
                    snapshot.subtitle = subtitle.value;
                    snapshot.subtitle_field_index = subtitle.field_index;
                    snapshot.subtitle_memo_block_number = subtitle.memo_block_number;
                }
                break;
            case StudioAssetKind::project:
                {
                    const FieldSelection title = first_non_empty_selection(record, {"NAME", "KEY", "TYPE"});
                    snapshot.title = title.value;
                    snapshot.title_field_index = title.field_index;
                    snapshot.title_memo_block_number = title.memo_block_number;
                    const FieldSelection subtitle = first_non_empty_selection(record, {"TYPE", "KEY", "COMMENTS"});
                    snapshot.subtitle = subtitle.value;
                    snapshot.subtitle_field_index = subtitle.field_index;
                    snapshot.subtitle_memo_block_number = subtitle.memo_block_number;
                }
                break;
            case StudioAssetKind::form:
            case StudioAssetKind::class_library:
            case StudioAssetKind::index:
            case StudioAssetKind::table:
            case StudioAssetKind::database_container:
            case StudioAssetKind::program:
            case StudioAssetKind::header:
            case StudioAssetKind::unknown:
                {
                    const FieldSelection title = first_non_empty_selection(record, {"OBJNAME", "NAME", "TITLE", "UNIQUEID", "CLASS"});
                    snapshot.title = title.value;
                    snapshot.title_field_index = title.field_index;
                    snapshot.title_memo_block_number = title.memo_block_number;
                    const FieldSelection subtitle = first_non_empty_selection(record, {"BASECLASS", "CLASS", "OBJTYPE", "OBJCODE", "PLATFORM"});
                    snapshot.subtitle = subtitle.value;
                    snapshot.subtitle_field_index = subtitle.field_index;
                    snapshot.subtitle_memo_block_number = subtitle.memo_block_number;
                }
                break;
        }
        if (snapshot.title.empty()) {
            snapshot.title = "Record " + std::to_string(record.record_index);
        }

        for (std::size_t field_index = 0U; field_index < record.values.size(); ++field_index) {
            const auto& value = record.values[field_index];
            snapshot.properties.push_back({
                .name = value.field_name,
                .field_index = field_index,
                .type = value.field_type,
                .is_null = value.is_null,
                .memo_block_number = value.memo_block_number,
                .value = usable_display_value(value)
            });
        }

        if (supports_visual_property_blob(document)) {
            for (std::size_t field_index = 0U; field_index < record.values.size(); ++field_index) {
                const auto& property_blob = record.values[field_index];
                if (property_blob.field_name == "PROPERTIES" &&
                    !property_blob.display_value.empty() &&
                    !looks_like_unresolved_memo(property_blob.display_value)) {
                    append_property_snapshots(
                        vfp::parse_visual_property_blob(property_blob.display_value),
                        snapshot.properties,
                        field_index,
                        property_blob.memo_block_number);
                    break;
                }
            }
        }

        objects.push_back(std::move(snapshot));
    }

    for (auto& object : objects) {
        if (object.object_name.empty()) {
            continue;
        }

        const std::string normalized_object_name = lowercase_ascii(object.object_name);
        object.child_count = static_cast<std::size_t>(std::count_if(
            objects.begin(),
            objects.end(),
            [&](const StudioObjectSnapshot& candidate) {
                return !candidate.parent_name.empty() &&
                    lowercase_ascii(candidate.parent_name) == normalized_object_name;
            }));
        if (!object.parent_name.empty()) {
            const std::string normalized_parent_name = lowercase_ascii(object.parent_name);
            const auto parent = std::find_if(
                objects.begin(),
                objects.end(),
                [&](const StudioObjectSnapshot& candidate) {
                    return !candidate.object_name.empty() &&
                        lowercase_ascii(candidate.object_name) == normalized_parent_name;
                });
            if (parent != objects.end()) {
                object.parent_record_index = parent->record_index;
            }
        }
    }

    for (auto& object : objects) {
        object.object_depth = build_object_depth(object, objects);
        object.object_path = build_object_path(object, objects);
        object.ancestor_record_indexes = build_ancestor_record_indexes(object, objects);
        assign_sibling_order(object, objects);
        object.child_record_indexes.clear();
        for (const auto& candidate : objects) {
            if (candidate.parent_record_index == object.record_index) {
                object.child_record_indexes.push_back(candidate.record_index);
            }
        }
        object.child_count = object.child_record_indexes.size();
    }

    return objects;
}

StudioOpenResult open_document(const StudioOpenRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = "No path was provided."};
    }

    const vfp::AssetInspectionResult inspection = vfp::inspect_asset(request.path);
    if (!inspection.ok) {
        return {.ok = false, .error = inspection.error};
    }

    StudioDocumentModel document;
    document.path = request.path;
    document.display_name = filename_of(request.path);
    document.selection_symbol = request.symbol;
    document.kind = studio_asset_kind_from_vfp_family(inspection.family);
    document.sidecar_path = infer_sidecar_path(request.path, document.kind);
    document.has_sidecar = !document.sidecar_path.empty() && std::filesystem::exists(document.sidecar_path);
    document.read_only = request.read_only;
    document.launched_from_visual_studio = request.launched_from_visual_studio;
    document.selection_record_available = request.selection_record_available;
    document.selection_line = request.line;
    document.selection_column = request.column;
    document.selection_record_index = request.record_index;
    document.inspection = inspection;
    if (document.kind == StudioAssetKind::program) {
        document.static_diagnostics = runtime::analyze_prg_file(request.path);
    }

    if (inspection.header_available) {
        const std::size_t max_records = request.load_full_table
            ? inspection.header.record_count
            : 8U;
        const auto table_result = vfp::parse_dbf_table_from_file(request.path, max_records);
        if (table_result.ok) {
            document.table_preview_available = true;
            document.table_preview = std::move(table_result.table);
        }
    }

    document.designer_contexts = request.designer_selection_contexts.empty()
        ? default_designer_contexts_for_request(
            document,
            request.selection_record_available ? request.record_index : StudioObjectMissingFieldIndex,
            request.symbol)
        : requested_designer_contexts(request.designer_selection_contexts);

    return {.ok = true, .document = document};
}

}  // namespace copperfin::studio
