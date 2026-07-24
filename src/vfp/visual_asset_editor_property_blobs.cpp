// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "visual_asset_editor_support.h"

#include "copperfin/platform/path.h"
#include "copperfin/vfp/report_layout_records.h"

#include <limits>
#include <set>

namespace copperfin::vfp {
namespace {

bool is_report_layout_asset_path(const std::string& path) {
    std::string ext = copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(path).extension());
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return ext == ".frx" || ext == ".lbx";
}

int parse_record_int_or_default(const DbfRecord& record, const std::string& field_name) {
    const auto* value = find_record_value(record, field_name);
    if (value == nullptr) {
        return 0;
    }

    const auto parsed = parse_visual_geometry_number(value->display_value);
    if (!parsed.has_value()) {
        return 0;
    }

    return truncate_report_layout_geometry(*parsed);
}

bool is_report_band_record(const DbfRecord& record) {
    return parse_record_int_or_default(record, "OBJTYPE") == 9;
}

bool is_report_layout_object_record(const DbfRecord& record) {
    const int objtype = parse_record_int_or_default(record, "OBJTYPE");
    return objtype == 5 || objtype == 6 || objtype == 7 || objtype == 8 || objtype == 17 || objtype == 18;
}

bool is_report_settings_record(const DbfRecord& record) {
    return is_report_settings_root_record(parse_record_int_or_default(record, "OBJTYPE"));
}

bool is_known_report_settings_expr_property(const std::string& normalized_property_name) {
    static constexpr std::array<std::string_view, 27> known_settings{
        "ascii",
        "botmargin",
        "collate",
        "color",
        "copies",
        "defaultsource",
        "device",
        "duplex",
        "driver",
        "gridh",
        "gridv",
        "orientation",
        "paperlength",
        "paperwidth",
        "output",
        "papersize",
        "printquality",
        "ttoption",
        "winspool",
        "yresolution",
        "topmargin",
        "leftmargin",
        "rightmargin",
        "tag",
        "cols",
        "colwidth",
        "colspacing"
    };

    return std::find(known_settings.begin(), known_settings.end(), normalized_property_name) != known_settings.end();
}

struct ReportSettingsExprLine {
    std::string raw_line;
    std::string name;
    std::string value;
    bool is_assignment = false;
};

bool is_report_settings_comment_like_name(const std::string& name) {
    return name.rfind("*", 0U) == 0U || name.rfind("&&", 0U) == 0U;
}

std::vector<ReportSettingsExprLine> parse_report_settings_expr_lines(const std::string& text) {
    std::vector<ReportSettingsExprLine> lines;
    std::size_t start = 0U;
    while (start < text.size()) {
        const std::size_t end = text.find_first_of("\r\n", start);
        const std::string line = end == std::string::npos ? text.substr(start) : text.substr(start, end - start);

        const auto equals = line.find('=');
        if (equals == std::string::npos) {
            lines.push_back({
                .raw_line = line,
                .name = {},
                .value = {},
                .is_assignment = false
            });
        } else {
            const std::string name = trim_both(line.substr(0U, equals));
            const std::string value = trim_both(line.substr(equals + 1U));
            if (name.empty() || is_report_settings_comment_like_name(name)) {
                lines.push_back({
                    .raw_line = line,
                    .name = {},
                    .value = {},
                    .is_assignment = false
                });
            } else {
                lines.push_back({
                    .raw_line = {},
                    .name = name,
                    .value = value,
                    .is_assignment = true
                });
            }
        }

        if (end == std::string::npos) {
            break;
        }

        start = end + 1U;
        if (start < text.size() &&
            ((text[end] == '\r' && text[start] == '\n') || (text[end] == '\n' && text[start] == '\r'))) {
            ++start;
        }
    }
    return lines;
}

const DbfRecordValue* find_report_settings_memo_field(
    const DbfRecord& record,
    std::string_view field_name) {
    const auto field = std::find_if(record.values.begin(), record.values.end(), [&](const DbfRecordValue& value) {
        return normalize_visual_property_name(value.field_name) == field_name;
    });
    return field == record.values.end() ? nullptr : &*field;
}

const DbfRecordValue* find_report_settings_property_field(
    const DbfRecord& record,
    const std::string& property_name) {
    const std::string requested_property_name = normalize_visual_property_name(property_name);
    for (const std::string_view field_name : {"picture", "expr"}) {
        const auto* field = find_report_settings_memo_field(record, field_name);
        if (field == nullptr) {
            continue;
        }

        const auto assignments = parse_visual_property_blob(field->display_value);
        const auto property = std::find_if(assignments.begin(), assignments.end(), [&](const VisualPropertyAssignment& assignment) {
            return normalize_visual_property_name(assignment.name) == requested_property_name;
        });
        if (property != assignments.end()) {
            return field;
        }
    }
    return nullptr;
}

std::string serialize_report_settings_blob(const std::vector<ReportSettingsExprLine>& lines) {
    std::ostringstream stream;
    for (const auto& line : lines) {
        if (line.is_assignment) {
            if (line.name.empty()) {
                continue;
            }

            stream << line.name << "=" << line.value;
        } else {
            stream << line.raw_line;
        }

        stream << "\r\n";
    }
    return stream.str();
}

std::optional<VisualPropertyState> read_report_settings_expr_property_state(
    const DbfRecord& record,
    const std::string& property_name) {
    if (!is_report_settings_record(record)) {
        return std::nullopt;
    }

    const auto* expr_field = find_report_settings_memo_field(record, "expr");
    const auto* picture_field = find_report_settings_memo_field(record, "picture");
    if (expr_field == nullptr && picture_field == nullptr) {
        return std::nullopt;
    }

    const std::string requested_property_name = normalize_visual_property_name(property_name);
    for (const auto* field : {picture_field, expr_field}) {
        if (field == nullptr) {
            continue;
        }

        const auto assignments = parse_visual_property_blob(field->display_value);
        const auto property = std::find_if(assignments.begin(), assignments.end(), [&](const VisualPropertyAssignment& assignment) {
            return normalize_visual_property_name(assignment.name) == requested_property_name;
        });
        if (property != assignments.end()) {
            return VisualPropertyState{
                .exists = true,
                .direct_field = false,
                .property_name = property->name,
                .value = property->value,
                .record_deleted = record.deleted
            };
        }
    }

    if (!is_known_report_settings_expr_property(requested_property_name)) {
        return std::nullopt;
    }

    return VisualPropertyState{
        .exists = false,
        .direct_field = false,
        .property_name = trim_both(property_name),
        .value = {},
        .record_deleted = record.deleted
    };
}

struct ReportSectionGeometry {
    std::size_t record_index = 0U;
    int top = 0;
    int height = 0;
};

std::size_t find_report_section_index(
    const std::vector<ReportSectionGeometry>& sections,
    int top,
    int height) {
    std::size_t best_index = sections.size();
    for (std::size_t index = 0U; index < sections.size(); ++index) {
        const auto& section = sections[index];
        const int section_bottom = section.top + std::max(section.height, 1);
        const int object_bottom = top + std::max(height, 1);
        const bool begins_inside = top >= section.top && top < section_bottom;
        const bool overlaps = object_bottom > section.top && top < section_bottom;
        if (begins_inside || overlaps) {
            if (best_index >= sections.size()) {
                best_index = index;
                continue;
            }

            const auto& best_section = sections[best_index];
            const int best_bottom = best_section.top + std::max(best_section.height, 1);
            const bool best_begins_inside = top >= best_section.top && top < best_bottom;
            if (begins_inside != best_begins_inside) {
                if (begins_inside) {
                    best_index = index;
                }
                continue;
            }

            if (section.top > best_section.top ||
                (section.top == best_section.top && section_bottom > best_bottom) ||
                (section.top == best_section.top && section_bottom == best_bottom &&
                 section.record_index > best_section.record_index)) {
                best_index = index;
            }
        }
    }

    return best_index;
}

std::optional<double> parse_effective_section_top_value(const std::string& property_value) {
    const std::string trimmed = trim_both(property_value);
    if (trimmed.empty()) {
        return 0.0;
    }

    return parse_visual_geometry_number(trimmed);
}

double read_visual_geometry_number_or_default(
    const DbfRecord& record,
    const std::string& field_name,
    double fallback_value) {
    const auto* value = find_record_value(record, field_name);
    if (value == nullptr) {
        return fallback_value;
    }

    const auto parsed = parse_visual_geometry_number(value->display_value);
    return parsed.has_value() ? *parsed : fallback_value;
}

std::optional<VisualObjectBatchEditRequest> build_section_top_batch_update(
    const VisualObjectEditRequest& request,
    const DbfTable& table,
    std::size_t section_record_index) {
    if (!is_report_layout_asset_path(request.path) ||
        section_record_index >= table.records.size() ||
        !is_report_band_record(table.records[section_record_index])) {
        return std::nullopt;
    }

    const auto new_top = parse_effective_section_top_value(request.property_value);
    if (!new_top.has_value()) {
        return std::nullopt;
    }

    const auto& target_record = table.records[section_record_index];
    const double current_top = read_visual_geometry_number_or_default(
        target_record,
        "VPOS",
        static_cast<double>(parse_record_int_or_default(target_record, "VPOS")));
    const double delta = *new_top - current_top;
    if (std::abs(delta) < 0.0000001) {
        return std::nullopt;
    }

    std::vector<ReportSectionGeometry> live_sections;
    std::vector<ReportSectionGeometry> deleted_sections;
    for (const auto& record : table.records) {
        if (!is_report_band_record(record)) {
            continue;
        }

        auto& sections = record.deleted ? deleted_sections : live_sections;
        sections.push_back(ReportSectionGeometry{
            record.record_index,
            parse_record_int_or_default(record, "VPOS"),
            std::max(0, parse_record_int_or_default(record, "HEIGHT"))
        });
    }

    const auto sort_sections = [](std::vector<ReportSectionGeometry>& sections) {
        std::sort(sections.begin(), sections.end(), [](const ReportSectionGeometry& left,
                                                       const ReportSectionGeometry& right) {
            if (left.top != right.top) {
                return left.top < right.top;
            }
            return left.record_index < right.record_index;
        });
    };
    sort_sections(live_sections);
    sort_sections(deleted_sections);

    const bool target_deleted = target_record.deleted;
    const auto& target_sections = target_deleted ? deleted_sections : live_sections;
    const auto target_it = std::find_if(
        target_sections.begin(),
        target_sections.end(),
        [&](const ReportSectionGeometry& candidate) {
            return candidate.record_index == section_record_index;
        });
    if (target_it == target_sections.end()) {
        return std::nullopt;
    }

    const std::size_t target_section_index =
        static_cast<std::size_t>(std::distance(target_sections.begin(), target_it));

    VisualObjectBatchEditRequest batch{
        .path = request.path,
        .objects = {
            {
                .record_index = section_record_index,
                .object_name = {},
                .unique_id = {},
                .properties = {
                    {
                        .property_name = request.property_name,
                        .property_value = request.property_value
                    }
                }
            }
        }
    };

    for (const auto& record : table.records) {
        if (!is_report_layout_object_record(record)) {
            continue;
        }

        const int object_top = parse_record_int_or_default(record, "VPOS");
        const int object_height = std::max(0, parse_record_int_or_default(record, "HEIGHT"));
        const std::size_t live_section_index = find_report_section_index(live_sections, object_top, object_height);
        const std::size_t deleted_section_index = find_report_section_index(deleted_sections, object_top, object_height);

        const bool belongs_to_target = target_deleted
            ? live_section_index >= live_sections.size() && deleted_section_index == target_section_index
            : live_section_index == target_section_index;
        if (!belongs_to_target) {
            continue;
        }

        const double current_object_top = read_visual_geometry_number_or_default(
            record,
            "VPOS",
            static_cast<double>(object_top));
        batch.objects.push_back({
            .record_index = record.record_index,
            .object_name = {},
            .unique_id = {},
            .properties = {
                {
                    .property_name = "VPOS",
                    .property_value = format_visual_geometry_number(current_object_top + delta)
                }
            }
        });
    }

    return batch.objects.size() > 1U ? std::optional<VisualObjectBatchEditRequest>(std::move(batch)) : std::nullopt;
}

}  // namespace

VisualAssetEditResult expand_report_section_top_batch_updates(
    const std::string& path,
    const std::vector<VisualObjectBatchEditItem>& objects,
    std::vector<VisualObjectBatchEditItem>& expanded_objects) {
    expanded_objects = objects;
    if (!is_report_layout_asset_path(path)) {
        return {.ok = true, .error = {}};
    }

    const auto table_result = parse_dbf_table_from_file(
        path,
        std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }

    struct ResolvedBatchProperty {
        std::size_t record_index = 0U;
        VisualObjectEditRequest request;
    };

    using PropertyKey = std::pair<std::size_t, std::string>;
    std::vector<ResolvedBatchProperty> resolved_properties;
    std::set<PropertyKey> explicitly_requested_properties;
    for (const auto& object : objects) {
        for (const auto& property : object.properties) {
            VisualObjectEditRequest edit_request{
                .path = path,
                .record_index = object.record_index,
                .object_name = object.object_name,
                .unique_id = object.unique_id,
                .property_name = property.property_name,
                .property_value = property.property_value
            };
            std::size_t record_index = 0U;
            const auto resolution = resolve_visual_object_record_index(edit_request, record_index);
            if (!resolution.ok) {
                return resolution;
            }

            resolved_properties.push_back({record_index, std::move(edit_request)});
            explicitly_requested_properties.emplace(
                record_index,
                normalize_visual_property_name(property.property_name));
        }
    }

    std::vector<VisualObjectBatchEditItem> generated_updates;
    std::set<PropertyKey> generated_properties;
    std::set<PropertyKey> processed_section_properties;
    for (auto it = resolved_properties.rbegin(); it != resolved_properties.rend(); ++it) {
        const std::string normalized_property_name =
            normalize_visual_property_name(it->request.property_name);
        const PropertyKey section_key{it->record_index, normalized_property_name};
        if (normalized_property_name != "vpos" ||
            it->record_index >= table_result.table.records.size() ||
            !is_report_band_record(table_result.table.records[it->record_index]) ||
            !processed_section_properties.insert(section_key).second) {
            continue;
        }

        const auto section_batch = build_section_top_batch_update(
            it->request,
            table_result.table,
            it->record_index);
        if (!section_batch.has_value()) {
            continue;
        }

        for (const auto& generated_object : section_batch->objects) {
            for (const auto& generated_property : generated_object.properties) {
                const PropertyKey generated_key{
                    generated_object.record_index,
                    normalize_visual_property_name(generated_property.property_name)
                };
                if (explicitly_requested_properties.contains(generated_key) ||
                    !generated_properties.insert(generated_key).second) {
                    continue;
                }

                generated_updates.push_back({
                    .record_index = generated_object.record_index,
                    .object_name = {},
                    .unique_id = {},
                    .properties = {generated_property}
                });
            }
        }
    }

    if (!generated_updates.empty()) {
        expanded_objects.clear();
        expanded_objects.reserve(generated_updates.size() + objects.size());
        expanded_objects.insert(
            expanded_objects.end(),
            std::make_move_iterator(generated_updates.begin()),
            std::make_move_iterator(generated_updates.end()));
        expanded_objects.insert(expanded_objects.end(), objects.begin(), objects.end());
    }

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
        visual_asset_text("VisualAssetEditor.Property.NotFound"),
        visual_asset_text("VisualAssetEditor.Property.Ambiguous"),
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
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.RelativeNameRequired")};
        }
        if (normalize_visual_property_name(relative_property_name) == normalize_visual_property_name(requested_property_name)) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.SourceRelativeToSelf")};
        }

        std::size_t relative_index = 0U;
        const auto relative_result = find_unique_visual_property_assignment_index(
            assignments,
            relative_property_name,
            visual_asset_text("VisualAssetEditor.Property.RelativeNotFound"),
            visual_asset_text("VisualAssetEditor.Property.RelativeAmbiguous"),
            relative_index);
        if (!relative_result.ok) {
            return relative_result;
        }
        insert_index = normalized_placement == "before" ? relative_index : relative_index + 1U;
    } else {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.PlacementUnsupported")};
    }

    assignments.insert(
        assignments.begin() + static_cast<std::ptrdiff_t>(insert_index),
        moving_property);
    return {.ok = true, .error = {}};
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
    if (is_report_layout_asset_path(path) &&
        find_report_settings_property_field(record, property_name) != nullptr) {
        const auto report_settings_state = read_report_settings_expr_property_state(record, property_name);
        if (report_settings_state.has_value()) {
            return report_settings_state;
        }
    }

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

    if (is_report_layout_asset_path(path)) {
        const auto report_settings_state = read_report_settings_expr_property_state(record, property_name);
        if (report_settings_state.has_value()) {
            return report_settings_state;
        }
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

VisualAssetEditResult replace_report_settings_expr_property(
    const VisualObjectEditRequest& request,
    const DbfRecord& record,
    std::size_t record_index,
    bool record_undo_entry,
    bool remove_property_if_missing) {
    const auto* expr_field = find_report_settings_memo_field(record, "expr");
    const auto* picture_field = find_report_settings_memo_field(record, "picture");
    if (expr_field == nullptr && picture_field == nullptr) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.MemoFieldMissing", {{"fieldName", "EXPR"}})};
    }

    const std::string requested_property_name = normalize_visual_property_name(request.property_name);
    const auto* target_field = find_report_settings_property_field(record, request.property_name);
    if (target_field == nullptr) {
        target_field = expr_field != nullptr ? expr_field : picture_field;
    }
    if (target_field == nullptr) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.MemoFieldMissing", {{"fieldName", "EXPR"}})};
    }

    auto assignments = parse_report_settings_expr_lines(target_field->display_value);
    auto assignment_it = std::find_if(assignments.begin(), assignments.end(), [&](const ReportSettingsExprLine& assignment) {
        return assignment.is_assignment &&
               normalize_visual_property_name(assignment.name) == requested_property_name;
    });

    const bool exists = assignment_it != assignments.end();
    const std::string prior_value = exists ? assignment_it->value : std::string{};
    if (!exists && remove_property_if_missing) {
        return {.ok = true, .error = {}};
    }
    if (exists && prior_value == request.property_value) {
        return {.ok = true, .error = {}};
    }

    if (record_undo_entry) {
        std::string error;
        if (!record_visual_asset_undo_entry(request.path, {
                .record_index = record_index,
                .property_name = request.property_name,
                .prior_value = prior_value,
                .prior_value_exists = exists,
                .label = visual_asset_text("VisualAssetEditor.Undo.PropertyLabel", {{"propertyName", request.property_name}}),
                .grouped_changes = {}
            }, error)) {
            return {.ok = false, .error = error};
        }
    }

    if (!exists) {
        assignments.push_back({
            .raw_line = {},
            .name = request.property_name,
            .value = request.property_value,
            .is_assignment = true
        });
    } else if (remove_property_if_missing) {
        assignments.erase(assignment_it);
    } else {
        assignment_it->value = request.property_value;
    }

    return replace_memo_field_value(
        request.path,
        record_index,
        target_field->field_name,
        serialize_report_settings_blob(assignments));
}

bool direct_field_change_is_noop(
    const std::string& path,
    const std::vector<std::uint8_t>& table_bytes,
    const DbfParseResult& header,
    std::size_t record_index,
    const RawFieldDescriptor& field,
    const std::string& new_value) {
    if (record_index >= header.header.record_count) {
        return false;
    }

    const std::size_t record_offset = header.header.header_length +
                                      (record_index * header.header.record_length);
    const std::size_t field_offset = record_offset + field.offset;
    if ((field_offset + field.length) > table_bytes.size()) {
        return false;
    }

    const auto raw_begin = table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset);
    const auto raw_end = raw_begin + static_cast<std::ptrdiff_t>(field.length);

    switch (field.type) {
        case 'C':
            return trim_right(std::string(raw_begin, raw_end)) == new_value;
        case 'N':
        case 'F':
            return trim_both(std::string(raw_begin, raw_end)) == trim_both(new_value);
        case 'L': {
            const auto logical_value = normalize_logical_value(new_value);
            return logical_value.has_value() &&
                   table_bytes[field_offset] == static_cast<std::uint8_t>(*logical_value);
        }
        case 'M': {
            const auto block_number = read_le_u32(table_bytes, field_offset);
            if (block_number == 0U) {
                return new_value.empty();
            }

            const SidecarPathResolution memo_resolution = infer_memo_sidecar_path(path);
            const std::string memo_path = selected_memo_sidecar_path(memo_resolution);
            if (memo_path.empty()) {
                return false;
            }

            const auto current_memo_bytes = read_memo_block_raw(memo_path, block_number);
            if (current_memo_bytes.empty() && !new_value.empty()) {
                return false;
            }

            return std::string(current_memo_bytes.begin(), current_memo_bytes.end()) == new_value;
        }
        default:
            return false;
    }
}

VisualAssetEditResult apply_visual_object_property_change(
    const VisualObjectEditRequest& request,
    bool record_undo_entry,
    bool remove_property_if_missing) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    const SidecarPathResolution memo_resolution = infer_memo_sidecar_path(request.path);
    if (memo_resolution.ambiguous) {
        return {.ok = false, .error = ambiguous_memo_sidecar_error(memo_resolution)};
    }
    const std::string memo_path = selected_memo_sidecar_path(memo_resolution);
    if (!memo_path.empty()) {
        const auto recovery_result = recover_visual_asset_file_transaction(request.path, memo_path);
        if (!recovery_result.ok) {
            return recovery_result;
        }
    }
    if (trim_both(request.property_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NameRequired")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }

    const std::string normalized_property_name = normalize_visual_property_name(request.property_name);
    if (record_undo_entry && normalized_property_name == "vpos") {
        const auto& target_record = table_result.table.records[record_index];
        if (is_report_layout_asset_path(request.path) && is_report_band_record(target_record)) {
            const auto full_table_result =
                parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
            if (!full_table_result.ok) {
                return {.ok = false, .error = full_table_result.error};
            }
            if (record_index >= full_table_result.table.records.size()) {
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
            }

            const auto batch_update =
                build_section_top_batch_update(request, full_table_result.table, record_index);
            if (batch_update.has_value()) {
                return update_visual_object_batch(*batch_update);
            }
        }
    }

    const auto table_bytes = read_binary_file(request.path);
    if (table_bytes.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed")};
    }

    const auto fields = read_raw_field_descriptors(table_bytes);
    const auto& record = table_result.table.records[record_index];
    if (is_report_layout_asset_path(request.path) &&
        find_report_settings_property_field(record, request.property_name) != nullptr) {
        return replace_report_settings_expr_property(
            request,
            record,
            record_index,
            record_undo_entry,
            remove_property_if_missing);
    }

    const auto direct_field_it = find_direct_visual_property_field(fields, request.property_name);
    if (direct_field_it != fields.end()) {
        const auto header_result = parse_dbf_header(table_bytes);
        if (!header_result.ok) {
            return {.ok = false, .error = header_result.error};
        }
        if (direct_field_change_is_noop(
                request.path,
                table_bytes,
                header_result,
                record_index,
                *direct_field_it,
                request.property_value)) {
            return {.ok = true, .error = {}};
        }

        if (record_undo_entry) {
            const auto property_state = read_current_visual_property_state(request.path, record_index, request.property_name);
            if (!property_state.has_value()) {
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Undo.CurrentPropertyReadFailed")};
            }
            if (!property_state->direct_field) {
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Undo.PropertyLookupMismatch")};
            }

            std::string error;
            if (!record_visual_asset_undo_entry(request.path, {
                    .record_index = record_index,
                    .property_name = request.property_name,
                    .prior_value = property_state->value,
                    .prior_value_exists = property_state->exists,
                    .label = visual_asset_text("VisualAssetEditor.Undo.PropertyLabel", {{"propertyName", request.property_name}}),
                    .grouped_changes = {}
                }, error)) {
                return {.ok = false, .error = error};
            }
        }

        return replace_field_value(request.path, record_index, *direct_field_it, request.property_value);
    }

    if (is_report_layout_asset_path(request.path)) {
        const auto report_settings_state = read_report_settings_expr_property_state(record, request.property_name);
        if (report_settings_state.has_value()) {
            return replace_report_settings_expr_property(
                request,
                record,
                record_index,
                record_undo_entry,
                remove_property_if_missing);
        }
    }

    if (!is_property_blob_asset_path(request.path)) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NotWritableField")};
    }

    auto properties_it = std::find_if(record.values.begin(), record.values.end(), [](const DbfRecordValue& value) {
        return value.field_name == "PROPERTIES";
    });
    if (properties_it == record.values.end()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.MemoFieldMissing", {{"fieldName", "PROPERTIES"}})};
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
                .label = visual_asset_text("VisualAssetEditor.Undo.PropertyLabel", {{"propertyName", request.property_name}}),
                .grouped_changes = {}
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

VisualAssetEditResult set_visual_object_text_property(
    const std::string& path,
    const std::vector<VisualObjectAlignmentTarget>& objects,
    const std::string& property_name,
    const std::string& property_label,
    const std::string& text) {
    if (path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (objects.empty()) {
        return {.ok = false, .error = visual_asset_text(
            "VisualAssetEditor.Object.PropertyAssignmentSelectionRequired",
            {{"propertyLabel", property_label}})};
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
            return {.ok = false, .error = visual_asset_text(
                "VisualAssetEditor.Object.PropertyAssignmentDuplicate",
                {{"propertyLabel", property_label}})};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (objects.empty()) {
        return {.ok = false, .error = visual_asset_text(
            "VisualAssetEditor.Object.PropertyAssignmentSelectionRequired",
            {{"propertyLabel", property_label}})};
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
            return {.ok = false, .error = visual_asset_text(
                "VisualAssetEditor.Object.PropertyAssignmentDuplicate",
                {{"propertyLabel", property_label}})};
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

        stream << property.name << " = " << property.value;
        stream << "\r\n";
    }
    return stream.str();
}

bool is_property_blob_asset_path(const std::string& path) {
    std::string ext = copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(path).extension());
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return ext == ".scx" || ext == ".vcx";
}

VisualAssetEditResult update_visual_object_property(const VisualObjectEditRequest& request) {
    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    auto update_result = apply_visual_object_property_change(request, true, false);
    if (update_result.ok) {
        update_result.affected_object_count = 1U;
    } else {
        std::string cleanup_error;
        if (!discard_visual_asset_undo_entries_after_depth(
                request.path,
                initial_undo_depth,
                cleanup_error)) {
            update_result.error = visual_asset_rollback_failed_text(
                std::move(update_result.error),
                std::move(cleanup_error));
        }
    }
    return update_result;
}

VisualAssetEditResult clear_visual_object_property(const VisualObjectPropertyClearRequest& request) {
    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    auto clear_result = apply_visual_object_property_change({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .property_name = request.property_name,
        .property_value = {}
    }, true, true);
    if (clear_result.ok) {
        clear_result.affected_object_count = 1U;
    } else {
        std::string cleanup_error;
        if (!discard_visual_asset_undo_entries_after_depth(
                request.path,
                initial_undo_depth,
                cleanup_error)) {
            clear_result.error = visual_asset_rollback_failed_text(
                std::move(clear_result.error),
                std::move(cleanup_error));
        }
    }
    return clear_result;
}

VisualAssetEditResult clear_visual_object_properties(const VisualObjectPropertyClearBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.ClearBatchRequired")};
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
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NameRequired")};
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
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.properties.size()};
}

VisualAssetEditResult copy_visual_object_property(const VisualObjectPropertyCopyRequest& request) {
    if (!request.target_property_name.empty() && trim_both(request.target_property_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.TargetNameRequired")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.SourceNotFound")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.TargetObjectAlreadyHasProperty")};
    }

    auto copy_result = update_visual_object_property({
        .path = request.path,
        .record_index = request.target_record_index,
        .object_name = request.target_object_name,
        .unique_id = request.target_unique_id,
        .property_name = target_property_name,
        .property_value = source_property.value
    });
    if (copy_result.ok) {
        copy_result.affected_object_count = 1U;
    }
    return copy_result;
}

VisualAssetEditResult copy_visual_object_properties(const VisualObjectPropertyCopyBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.CopyBatchRequired")};
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
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NameRequired")};
        }
        if (!property.target_property_name.empty() && trim_both(property.target_property_name).empty()) {
            const auto rollback_result = rollback_batch_copies();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.TargetNameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.TargetNameRequired")};
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
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.properties.size()};
}

VisualAssetEditResult move_visual_object_property(const VisualObjectPropertyMoveRequest& request) {
    if (!request.target_property_name.empty() && trim_both(request.target_property_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.TargetNameRequired")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.SourceNotFound")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.SourceMoveToSelf")};
    }
    if (target_property.exists && !request.replace_existing) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.TargetObjectAlreadyHasProperty")};
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
            return {.ok = false, .error = visual_asset_target_rollback_failed_text(clear_result.error, rollback_result.error)};
        }
        return {.ok = false, .error = clear_result.error};
    }

    return {.ok = true, .error = {}, .affected_object_count = 1U};
}

VisualAssetEditResult move_visual_object_properties(const VisualObjectPropertyMoveBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.MoveBatchRequired")};
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
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NameRequired")};
        }
        if (!property.target_property_name.empty() && trim_both(property.target_property_name).empty()) {
            const auto rollback_result = rollback_batch_moves();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.TargetNameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.TargetNameRequired")};
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
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.properties.size()};
}

VisualAssetEditResult rename_visual_object_property(const VisualObjectPropertyRenameRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }

    const std::string source_property_name = trim_both(request.property_name);
    const std::string target_property_name = trim_both(request.new_property_name);
    if (source_property_name.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NameRequired")};
    }
    if (target_property_name.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.TargetNameRequired")};
    }
    if (normalize_visual_property_name(source_property_name) == normalize_visual_property_name(target_property_name)) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.SourceRenameToSelf")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }

    const auto table_bytes = read_binary_file(request.path);
    if (table_bytes.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed")};
    }

    const std::string normalized_source = normalize_visual_property_name(source_property_name);
    const auto fields = read_raw_field_descriptors(table_bytes);
    const auto direct_field_it = std::find_if(fields.begin(), fields.end(), [&](const RawFieldDescriptor& field) {
        return normalize_visual_property_name(field.name) == normalized_source;
    });
    if (direct_field_it != fields.end()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.DirectFieldRenameUnsupported")};
    }

    if (!is_property_blob_asset_path(request.path)) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NotRenameableMemo")};
    }

    const auto& record = table_result.table.records[record_index];
    const auto properties_it = std::find_if(record.values.begin(), record.values.end(), [](const DbfRecordValue& value) {
        return value.field_name == "PROPERTIES";
    });
    if (properties_it == record.values.end()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedMemoFieldMissing", {{"fieldName", "PROPERTIES"}})};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.SourceNotFound")};
    }
    if (source_count > 1U) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.SourceAmbiguousInObject")};
    }
    if (target_exists) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.TargetExistsInObject")};
    }

    const std::size_t initial_undo_depth = list_visual_asset_undo_entry_files(request.path).size();
    std::string error;
    if (!record_visual_asset_undo_entry(request.path, {
            .record_index = record_index,
            .property_name = "PROPERTIES",
            .prior_value = properties_it->display_value,
            .prior_value_exists = true,
            .label = visual_asset_text("VisualAssetEditor.Undo.RenamePropertyLabel", {{"propertyName", source_property_name}}),
            .grouped_changes = {}
        }, error)) {
        return {.ok = false, .error = error};
    }

    assignments[source_index].name = target_property_name;
    auto rename_result = replace_memo_field_value(
        request.path,
        record_index,
        "PROPERTIES",
        serialize_visual_property_blob(assignments));
    if (rename_result.ok) {
        rename_result.affected_object_count = 1U;
    } else {
        std::string cleanup_error;
        if (!discard_visual_asset_undo_entries_after_depth(
                request.path,
                initial_undo_depth,
                cleanup_error)) {
            rename_result.error = visual_asset_rollback_failed_text(
                std::move(rename_result.error),
                std::move(cleanup_error));
        }
    }
    return rename_result;
}

VisualAssetEditResult rename_visual_object_properties(const VisualObjectPropertyRenameBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.RenameBatchRequired")};
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
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NameRequired")};
        }
        if (trim_both(property.new_property_name).empty()) {
            const auto rollback_result = rollback_batch_renames();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.TargetNameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.TargetNameRequired")};
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
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
    }

    return {.ok = true, .error = {}, .affected_object_count = request.properties.size()};
}

VisualAssetEditResult reorder_visual_object_property(const VisualObjectPropertyReorderRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (trim_both(request.property_name).empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NameRequired")};
    }

    const std::string placement = normalize_visual_property_name(request.placement);
    if (placement != "first" && placement != "last" && placement != "before" && placement != "after") {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.PlacementUnsupported")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }

    const auto table_bytes = read_binary_file(request.path);
    if (table_bytes.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed")};
    }

    const auto fields = read_raw_field_descriptors(table_bytes);
    const auto is_direct_field_name = [&](const std::string& property_name) {
        const std::string normalized_property_name = normalize_visual_property_name(property_name);
        return std::any_of(fields.begin(), fields.end(), [&](const RawFieldDescriptor& field) {
            return normalize_visual_property_name(field.name) == normalized_property_name;
        });
    };
    if (is_direct_field_name(request.property_name)) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.DirectFieldReorderUnsupported")};
    }
    if ((placement == "before" || placement == "after") &&
        !trim_both(request.relative_property_name).empty() &&
        is_direct_field_name(request.relative_property_name)) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.DirectFieldReorderUnsupported")};
    }

    if (!is_property_blob_asset_path(request.path)) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NotReorderableMemo")};
    }

    const auto* properties_field = find_record_value(table_result.table.records[record_index], "PROPERTIES");
    if (properties_field == nullptr) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.SelectedMemoFieldMissing", {{"fieldName", "PROPERTIES"}})};
    }

    auto assignments = parse_visual_property_blob(properties_field->display_value);
    const std::string prior_blob = serialize_visual_property_blob(assignments);
    const auto reorder_result = reorder_visual_property_assignments(
        assignments,
        request.property_name,
        request.placement,
        request.relative_property_name);
    if (!reorder_result.ok) {
        return reorder_result;
    }

    const std::string updated_blob = serialize_visual_property_blob(assignments);
    if (updated_blob == prior_blob) {
        return {.ok = true, .error = {}, .affected_object_count = 0U};
    }

    auto update_result = update_visual_object_property({
        .path = request.path,
        .record_index = record_index,
        .object_name = {},
        .unique_id = {},
        .property_name = "PROPERTIES",
        .property_value = updated_blob
    });
    if (update_result.ok) {
        update_result.affected_object_count = 1U;
    }
    return update_result;
}

VisualAssetEditResult reorder_visual_object_properties(const VisualObjectPropertyReorderBatchRequest& request) {
    if (request.path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired")};
    }
    if (request.properties.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.ReorderBatchRequired")};
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

    std::size_t affected_object_count = 0U;
    for (const auto& property : request.properties) {
        if (trim_both(property.property_name).empty()) {
            const auto rollback_result = rollback_batch_reorders();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.NameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.NameRequired")};
        }

        const std::string placement = normalize_visual_property_name(property.placement);
        if ((placement == "before" || placement == "after") &&
            trim_both(property.relative_property_name).empty()) {
            const auto rollback_result = rollback_batch_reorders();
            if (!rollback_result.ok) {
                return {
                    .ok = false,
                    .error = visual_asset_rollback_failed_text(visual_asset_text("VisualAssetEditor.Property.RelativeNameRequired"), rollback_result.error)
                };
            }
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Property.RelativeNameRequired")};
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
                    .error = visual_asset_rollback_failed_text(result.error, rollback_result.error)
                };
            }
            return result;
        }
        affected_object_count += result.affected_object_count;
    }

    return {.ok = true, .error = {}, .affected_object_count = affected_object_count};
}

VisualObjectPropertyQueryResult query_visual_object_property(const VisualObjectPropertyQueryRequest& request) {
    if (request.path.empty()) {
        return {
            .ok = false,
            .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"),
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
            .error = visual_asset_text("VisualAssetEditor.Property.NameRequired"),
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
            .error = visual_asset_text("VisualAssetEditor.Property.ReadFailed"),
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
            .error = visual_asset_text("VisualAssetEditor.Operation.AssetPathRequired"),
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
            .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable"),
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

bool matches_property_filter(const VisualObjectPropertySnapshot& property, const std::string& lowered_search_text) {
    if (lowered_search_text.empty()) {
        return true;
    }

    std::string field_type_text;
    if (property.field_type != '\0') {
        field_type_text.push_back(property.field_type);
    }

    const std::string source_line_text = property.source_line_index == static_cast<std::size_t>(-1)
        ? std::string{}
        : std::to_string(property.source_line_index);
    const std::string backing_kind = property.direct_field ? "direct field" : "memo property";

    return contains_case_insensitive(property.property_name, lowered_search_text) ||
        contains_case_insensitive(property.value, lowered_search_text) ||
        contains_case_insensitive(field_type_text, lowered_search_text) ||
        contains_case_insensitive(source_line_text, lowered_search_text) ||
        contains_case_insensitive(backing_kind, lowered_search_text);
}

VisualObjectPropertyListFilterResult filter_visual_object_properties(
    const VisualObjectPropertyListFilterRequest& request) {
    const auto list_result = list_visual_object_properties({
        .path = request.path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id
    });
    if (!list_result.ok) {
        return {
            .ok = false,
            .error = list_result.error,
            .record_index = 0U,
            .record_deleted = false,
            .search_text = request.search_text,
            .property_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .properties = {}
        };
    }

    const std::string lowered_search_text = lowercase_copy(request.search_text);
    std::vector<VisualObjectPropertySnapshot> filtered;
    std::copy_if(
        list_result.properties.begin(),
        list_result.properties.end(),
        std::back_inserter(filtered),
        [&](const VisualObjectPropertySnapshot& property) {
            return matches_property_filter(property, lowered_search_text);
        });

    const auto property_count = filtered.size();
    return {
        .ok = true,
        .error = {},
        .record_index = list_result.record_index,
        .record_deleted = list_result.record_deleted,
        .search_text = request.search_text,
        .property_count = property_count,
        .dry_run = true,
        .mutates_asset = false,
        .properties = std::move(filtered)
    };
}

}  // namespace copperfin::vfp
