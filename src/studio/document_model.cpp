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

FieldSelection first_non_empty_selection(const vfp::DbfRecord& record, std::initializer_list<std::string_view> field_names) {
    for (const auto field_name : field_names) {
        const auto* value = find_value(record, field_name);
        if (value != nullptr) {
            const std::string usable_value = trim_copy(usable_display_value(*value));
            if (!usable_value.empty()) {
                return {.value = usable_value, .field_index = field_index_or_missing(record, field_name)};
            }
        }
    }
    return {};
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

void append_property_snapshots(
    const std::vector<vfp::VisualPropertyAssignment>& assignments,
    std::vector<StudioPropertySnapshot>& properties,
    std::size_t source_field_index) {
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
            .value = assignment.value
        });
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
        snapshot.objcode_code = parse_scaled_int(record, "OBJCODE").value_or(0);
        snapshot.objcode_field_index = field_index_or_missing(record, "OBJCODE");
        snapshot.platform = first_non_empty(record, {"PLATFORM"});
        snapshot.platform_field_index = field_index_or_missing(record, "PLATFORM");
        const FieldSelection object_name = first_non_empty_selection(record, {"OBJNAME", "NAME"});
        snapshot.object_name = object_name.value;
        snapshot.object_name_field_index = object_name.field_index;
        const FieldSelection unique_id = first_non_empty_selection(record, {"UNIQUEID"});
        snapshot.unique_id = unique_id.value;
        snapshot.unique_id_field_index = unique_id.field_index;
        const FieldSelection parent_name = first_non_empty_selection(record, {"PARENT", "PARENTID"});
        snapshot.parent_name = parent_name.value;
        snapshot.parent_name_field_index = parent_name.field_index;
        const FieldSelection class_name = first_non_empty_selection(record, {"CLASS"});
        snapshot.class_name = class_name.value;
        snapshot.class_name_field_index = class_name.field_index;
        const FieldSelection baseclass_name = first_non_empty_selection(record, {"BASECLASS"});
        snapshot.baseclass_name = baseclass_name.value;
        snapshot.baseclass_name_field_index = baseclass_name.field_index;
        if (document.kind == StudioAssetKind::menu) {
            snapshot.menu_prompt = first_non_empty(record, {"PROMPT"});
            snapshot.menu_prompt_field_index = field_index_or_missing(record, "PROMPT");
            snapshot.menu_level_name = first_non_empty(record, {"LEVELNAME"});
            snapshot.menu_level_name_field_index = field_index_or_missing(record, "LEVELNAME");
            snapshot.menu_command = first_non_empty(record, {"COMMAND"});
            snapshot.menu_command_field_index = field_index_or_missing(record, "COMMAND");
            snapshot.menu_message = first_non_empty(record, {"MESSAGE"});
            snapshot.menu_message_field_index = field_index_or_missing(record, "MESSAGE");
        }
        switch (document.kind) {
            case StudioAssetKind::report:
            case StudioAssetKind::label:
                {
                    const FieldSelection title = first_non_empty_selection(record, {"EXPR", "NAME", "UNIQUEID"});
                    snapshot.title = title.value;
                    snapshot.title_field_index = title.field_index;
                    const FieldSelection subtitle = first_non_empty_selection(record, {"OBJTYPE", "OBJCODE", "FONTFACE", "PLATFORM"});
                    snapshot.subtitle = subtitle.value;
                    snapshot.subtitle_field_index = subtitle.field_index;
                }
                break;
            case StudioAssetKind::menu:
                {
                    const FieldSelection title = first_non_empty_selection(record, {"PROMPT", "NAME", "LEVELNAME"});
                    snapshot.title = title.value;
                    snapshot.title_field_index = title.field_index;
                    const FieldSelection subtitle = first_non_empty_selection(record, {"LEVELNAME", "OBJTYPE", "OBJCODE"});
                    snapshot.subtitle = subtitle.value;
                    snapshot.subtitle_field_index = subtitle.field_index;
                }
                break;
            case StudioAssetKind::project:
                {
                    const FieldSelection title = first_non_empty_selection(record, {"NAME", "KEY", "TYPE"});
                    snapshot.title = title.value;
                    snapshot.title_field_index = title.field_index;
                    const FieldSelection subtitle = first_non_empty_selection(record, {"TYPE", "KEY", "COMMENTS"});
                    snapshot.subtitle = subtitle.value;
                    snapshot.subtitle_field_index = subtitle.field_index;
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
                    const FieldSelection subtitle = first_non_empty_selection(record, {"BASECLASS", "CLASS", "OBJTYPE", "OBJCODE", "PLATFORM"});
                    snapshot.subtitle = subtitle.value;
                    snapshot.subtitle_field_index = subtitle.field_index;
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
                        field_index);
                    break;
                }
            }
        }

        objects.push_back(std::move(snapshot));
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
    document.kind = studio_asset_kind_from_vfp_family(inspection.family);
    document.sidecar_path = infer_sidecar_path(request.path, document.kind);
    document.has_sidecar = !document.sidecar_path.empty() && std::filesystem::exists(document.sidecar_path);
    document.read_only = request.read_only;
    document.launched_from_visual_studio = request.launched_from_visual_studio;
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

    return {.ok = true, .document = document};
}

}  // namespace copperfin::studio
