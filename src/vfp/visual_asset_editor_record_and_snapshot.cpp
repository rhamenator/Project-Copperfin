// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "visual_asset_editor_support.h"

#include "dbf_table_raw_mutation.h"
#include "copperfin/vfp/dbf_text_encoding.h"

namespace copperfin::vfp {
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

VisualObjectDuplicateResult failed_visual_object_duplicate_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .parent_name = {}
    };
}

VisualObjectDuplicateBatchResult failed_visual_object_duplicate_batch_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .record_indexes = {},
        .duplicated_objects = {}
    };
}

VisualObjectDuplicateResult reject_identity_collision(
    const DbfTable& table,
    const std::string& field_name,
    const std::string& requested_value) {
    if (normalize_visual_object_name(requested_value).empty()) {
        return {
            .ok = true,
            .error = {},
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .parent_name = {}
        };
    }
    if (!find_field_index(table, field_name).has_value()) {
        return failed_visual_object_duplicate_result(
            visual_asset_text("VisualAssetEditor.Identity.ReplacementFieldMissing"));
    }
    if (!find_matching_record_indexes(table, field_name, normalize_visual_object_name(requested_value)).empty()) {
        return failed_visual_object_duplicate_result(
            visual_asset_text("VisualAssetEditor.Identity.ReplacementExists"));
    }
    return {
        .ok = true,
        .error = {},
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .parent_name = {}
    };
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Identity.FieldMissing")};
    }
    const auto matches = find_matching_record_indexes(table, field_name, normalized_value);
    const auto collision = std::find_if(matches.begin(), matches.end(), [&](std::size_t record_index) {
        return record_index != excluded_record_index;
    });
    if (collision != matches.end()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Identity.ValueExists")};
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

VisualObjectCreatedObject created_visual_object_from_record(const DbfRecord& record, std::size_t record_index) {
    const auto* unique_id = find_record_value(record, "UNIQUEID");
    const auto* parent_name = find_record_value(record, "PARENT");
    return {
        .record_index = record_index,
        .object_name = visual_object_record_name(record),
        .unique_id = unique_id == nullptr ? std::string{} : trim_both(unique_id->display_value),
        .parent_name = parent_name == nullptr ? std::string{} : trim_both(parent_name->display_value)
    };
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
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.RequiredFieldsMissing")};
        }
        const auto parsed_value = parse_visual_geometry_number(property_result.value);
        if (!parsed_value.has_value()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.ObjectGeometryNotNumeric")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.DistributionCoordinatesMissing")};
    }

    const auto parsed_value = parse_visual_geometry_number(property_result.value);
    if (!parsed_value.has_value()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Geometry.DistributionCoordinateNotNumeric")};
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
    const auto* methods = find_record_value(record, "METHODS");

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

    const std::size_t parsed_property_count = properties == nullptr
        ? 0U
        : parse_visual_property_blob(properties->display_value).size();
    const std::size_t parsed_method_count = methods == nullptr
        ? 0U
        : parse_visual_methods_blob(methods->display_value, methods->memo_block_number).size();

    return {
        .record_index = record.record_index,
        .deleted = record.deleted,
        .object_name = visual_object_record_name(record),
        .object_path = {},
        .object_depth = 0U,
        .unique_id = unique_id == nullptr ? std::string{} : trim_both(unique_id->display_value),
        .parent_name = parent_name == nullptr ? std::string{} : trim_both(parent_name->display_value),
        .parent_record_available = false,
        .parent_record_index = 0U,
        .ancestor_record_indexes = {},
        .sibling_index = 0U,
        .sibling_count = 0U,
        .child_count = 0U,
        .property_count = record.values.size() + parsed_property_count,
        .method_count = parsed_method_count,
        .class_name = class_name == nullptr ? std::string{} : trim_both(class_name->display_value),
        .baseclass_name = baseclass_name == nullptr ? std::string{} : trim_both(baseclass_name->display_value),
        .caption = caption
    };
}

const DbfRecord* find_visual_object_record_by_record_index(
    const DbfTable& table,
    std::size_t record_index) {
    const auto record = std::find_if(
        table.records.begin(),
        table.records.end(),
        [&](const DbfRecord& candidate) {
            return candidate.record_index == record_index;
        });
    return record == table.records.end() ? nullptr : &*record;
}

const DbfRecord* find_visual_object_record_by_name(
    const DbfTable& table,
    const std::string& object_name) {
    const std::string normalized_object_name = normalize_visual_object_name(object_name);
    if (normalized_object_name.empty()) {
        return nullptr;
    }
    const auto record = std::find_if(
        table.records.begin(),
        table.records.end(),
        [&](const DbfRecord& candidate) {
            return normalize_visual_object_name(visual_object_record_name(candidate)) == normalized_object_name;
        });
    return record == table.records.end() ? nullptr : &*record;
}

void enrich_visual_object_hierarchy_snapshot(
    VisualObjectSnapshot& snapshot,
    const DbfTable& table) {
    const std::string normalized_parent_name = normalize_visual_object_name(snapshot.parent_name);
    snapshot.parent_record_available = false;
    snapshot.parent_record_index = 0U;
    if (!normalized_parent_name.empty()) {
        const auto* parent = find_visual_object_record_by_name(table, snapshot.parent_name);
        if (parent != nullptr) {
            snapshot.parent_record_available = true;
            snapshot.parent_record_index = parent->record_index;
        }
    }

    snapshot.ancestor_record_indexes.clear();
    if (snapshot.parent_record_available) {
        std::size_t current_parent_record_index = snapshot.parent_record_index;
        while (snapshot.ancestor_record_indexes.size() < table.records.size()) {
            const auto* parent = find_visual_object_record_by_record_index(table, current_parent_record_index);
            if (parent == nullptr) {
                break;
            }
            snapshot.ancestor_record_indexes.push_back(parent->record_index);
            const auto* next_parent_name = find_record_value(*parent, "PARENT");
            if (next_parent_name == nullptr || trim_both(next_parent_name->display_value).empty()) {
                break;
            }
            const auto* next_parent = find_visual_object_record_by_name(table, next_parent_name->display_value);
            if (next_parent == nullptr) {
                break;
            }
            current_parent_record_index = next_parent->record_index;
        }
        std::reverse(snapshot.ancestor_record_indexes.begin(), snapshot.ancestor_record_indexes.end());
    }
    snapshot.object_depth = snapshot.ancestor_record_indexes.size();

    snapshot.object_path.clear();
    if (!snapshot.object_name.empty()) {
        for (const auto ancestor_record_index : snapshot.ancestor_record_indexes) {
            const auto* ancestor = find_visual_object_record_by_record_index(table, ancestor_record_index);
            if (ancestor == nullptr) {
                continue;
            }
            const std::string ancestor_name = visual_object_record_name(*ancestor);
            if (ancestor_name.empty()) {
                continue;
            }
            if (!snapshot.object_path.empty()) {
                snapshot.object_path += ".";
            }
            snapshot.object_path += ancestor_name;
        }
        if (!snapshot.object_path.empty()) {
            snapshot.object_path += ".";
        }
        snapshot.object_path += snapshot.object_name;
    }

    snapshot.sibling_index = 0U;
    snapshot.sibling_count = 0U;
    for (const auto& candidate : table.records) {
        const auto* candidate_parent_value = find_record_value(candidate, "PARENT");
        const std::string candidate_parent_name =
            candidate_parent_value == nullptr ? std::string{} : trim_both(candidate_parent_value->display_value);
        const auto* candidate_parent = find_visual_object_record_by_name(table, candidate_parent_name);
        const bool candidate_parent_available = candidate_parent != nullptr;
        const bool same_resolved_parent = snapshot.parent_record_available &&
            candidate_parent_available &&
            candidate_parent->record_index == snapshot.parent_record_index;
        const bool same_unresolved_parent = !snapshot.parent_record_available &&
            !candidate_parent_available &&
            normalize_visual_object_name(candidate_parent_name) == normalize_visual_object_name(snapshot.parent_name);
        const bool same_parent = same_resolved_parent || same_unresolved_parent;
        if (!same_parent) {
            continue;
        }
        if (candidate.record_index == snapshot.record_index) {
            snapshot.sibling_index = snapshot.sibling_count;
        }
        ++snapshot.sibling_count;
    }

    const std::string normalized_object_name = normalize_visual_object_name(snapshot.object_name);
    snapshot.child_count = normalized_object_name.empty()
        ? 0U
        : static_cast<std::size_t>(std::count_if(
              table.records.begin(),
              table.records.end(),
              [&](const DbfRecord& candidate) {
                  const auto* parent = find_record_value(candidate, "PARENT");
                  return parent != nullptr &&
                      normalize_visual_object_name(parent->display_value) == normalized_object_name;
              }));
}

VisualObjectSnapshot build_visual_object_snapshot(const DbfRecord& record, const DbfTable& table) {
    VisualObjectSnapshot snapshot = build_visual_object_snapshot(record);
    enrich_visual_object_hierarchy_snapshot(snapshot, table);
    return snapshot;
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
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.UniqueIdNotFound")};
        }
        if (matches.size() > 1U) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.UniqueIdAmbiguous")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.NameRequired")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.NameNotFound")};
    }
    if (matches.size() > 1U) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.NameAmbiguous")};
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
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.UniqueIdNotFound")};
        }
        if (matches.size() > 1U) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.UniqueIdAmbiguous")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.NameRequired")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.NameNotFound")};
    }
    if (matches.size() > 1U) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.NameAmbiguous")};
    }

    record_index = matches.front();
    return {.ok = true, .error = {}};
}

VisualAssetEditResult apply_visual_object_reorder_to_records(
    std::vector<DbfRecord>& records,
    const VisualObjectReorderBatchItem& request) {
    const std::string placement = normalize_visual_property_name(request.placement);
    if (placement != "front" && placement != "back" && placement != "before" && placement != "after") {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.PlacementUnsupported")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.RecordUnavailable")};
    }

    std::size_t target_record_index = 0U;
    if (placement == "before" || placement == "after") {
        if (trim_both(request.target_object_name).empty() && trim_both(request.target_unique_id).empty()) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TargetSelectorRequired")};
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
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TargetRecordUnavailable")};
        }
        if (target_record_index == source_record_index) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.ReorderRelativeToSelf")};
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
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Object.TargetRecordUnavailable")};
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

namespace {

VisualAssetEditResult recover_visual_asset_before_raw_record_mutation(
    const std::string& path) {
    return recover_visual_asset_storage_transaction(path);
}

VisualAssetEditResult commit_raw_record_mutation(
    const std::string& path,
    DbfRawRecordMutationResult mutation) {
    if (!mutation.ok) {
        return {.ok = false, .error = std::move(mutation.error)};
    }

    const std::vector<std::uint8_t> original_table_bytes = read_binary_file(path);
    if (original_table_bytes.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed")};
    }

    if (!mutation.has_memo_sidecar) {
        if (mutation.table_bytes == original_table_bytes) {
            return {.ok = true, .error = {}};
        }
        return write_visual_asset_table_transaction(path, mutation.table_bytes);
    }

    const std::vector<std::uint8_t> original_memo_bytes = read_binary_file(mutation.memo_path);
    if (original_memo_bytes.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.MemoSidecarOpenFailed")};
    }
    if (mutation.table_bytes == original_table_bytes &&
        mutation.memo_bytes == original_memo_bytes) {
        return {.ok = true, .error = {}};
    }
    return write_visual_asset_file_transaction(
        path,
        mutation.table_bytes,
        mutation.memo_path,
        mutation.memo_bytes);
}

}  // namespace

VisualAssetEditResult append_visual_asset_records_preserving_raw(
    const std::string& path,
    const std::vector<VisualAssetRawRecordAppend>& appends) {
    const auto recovery_result = recover_visual_asset_before_raw_record_mutation(path);
    if (!recovery_result.ok) {
        return recovery_result;
    }

    std::vector<DbfRawRecordAppend> raw_appends;
    raw_appends.reserve(appends.size());
    for (const auto& append : appends) {
        DbfRawRecordAppend raw_append{
            .source_record_index = append.source_record_index,
            .field_values = {}
        };
        raw_append.field_values.reserve(append.field_values.size());
        for (const auto& field_value : append.field_values) {
            raw_append.field_values.emplace_back(
                field_value.property_name,
                field_value.property_value);
        }
        raw_appends.push_back(std::move(raw_append));
    }
    return commit_raw_record_mutation(
        path,
        stage_dbf_raw_record_appends(path, raw_appends));
}

VisualAssetEditResult reorder_visual_asset_records_preserving_raw(
    const std::string& path,
    const std::vector<std::size_t>& record_order) {
    const auto recovery_result = recover_visual_asset_before_raw_record_mutation(path);
    if (!recovery_result.ok) {
        return recovery_result;
    }
    return commit_raw_record_mutation(
        path,
        stage_dbf_raw_record_reorder(path, record_order));
}

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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed")};
    }

    const auto header_result = parse_dbf_header(table_bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    if (record_index >= header_result.header.record_count) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.RecordIndexOutOfRange")};
    }

    const std::size_t record_offset = header_result.header.header_length +
                                      (record_index * header_result.header.record_length);
    const std::size_t field_offset = record_offset + field.offset;
    if ((field_offset + field.length) > table_bytes.size()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.RecordDataTruncated")};
    }

    std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset), field.length, static_cast<std::uint8_t>(' '));

    switch (field.type) {
        case 'C': {
            const DbfTextConversionResult encoded = encode_dbf_text(
                header_result.header.code_page_mark,
                trim_right(new_value));
            if (!encoded.ok) {
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TextEncodingConversionFailed")};
            }
            if (encoded.text.size() > field.length) {
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.CharacterValueTooLarge")};
            }
            std::copy(encoded.text.begin(),
                      encoded.text.end(),
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
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.NumericValueTooLarge")};
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
                return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.LogicalValueRequired")};
            }
            table_bytes[field_offset] = static_cast<std::uint8_t>(*logical_value);
            break;
        }
        default:
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.DirectFieldUpdateUnsupported")};
    }

    if (!write_binary_file(table_path, table_bytes)) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableWriteFailed")};
    }

    return {.ok = true, .error = {}};
}

VisualAssetEditResult replace_field_value(
    const std::string& table_path,
    std::size_t record_index,
    const RawFieldDescriptor& field,
    const std::string& new_value,
    bool raw_memo_value) {
    if (field.type == 'M') {
        return replace_memo_field_value(table_path, record_index, field.name, new_value, raw_memo_value);
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

VisualAssetEditResult replace_memo_field_value(
    const std::string& table_path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& new_value,
    bool raw_value) {
    const SidecarPathResolution memo_resolution = infer_memo_sidecar_path(table_path);
    if (memo_resolution.ambiguous) {
        return {.ok = false, .error = ambiguous_memo_sidecar_error(memo_resolution)};
    }
    const std::string memo_path = selected_memo_sidecar_path(memo_resolution);
    if (memo_path.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.MemoSidecarPathMissing")};
    }

    const auto recovery_result = recover_visual_asset_file_transaction(table_path, memo_path);
    if (!recovery_result.ok) {
        return recovery_result;
    }

    auto table_bytes = read_binary_file(table_path);
    if (table_bytes.empty()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TableOpenFailed")};
    }

    const auto header_result = parse_dbf_header(table_bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    std::vector<std::uint8_t> encoded_bytes;
    if (raw_value) {
        encoded_bytes.assign(new_value.begin(), new_value.end());
    } else {
        const DbfTextConversionResult encoded = encode_dbf_text(
            header_result.header.code_page_mark,
            new_value);
        if (!encoded.ok) {
            return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TextEncodingConversionFailed")};
        }
        encoded_bytes.assign(encoded.text.begin(), encoded.text.end());
    }

    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(table_bytes);
    const std::string normalized_field_name = normalize_visual_property_name(field_name);
    const auto field_it = std::find_if(fields.begin(), fields.end(), [&](const RawFieldDescriptor& field) {
        return normalize_visual_property_name(field.name) == normalized_field_name;
    });
    if (field_it == fields.end()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Field.TargetNotFound")};
    }
    if (field_it->type != 'M') {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.TargetFieldMemoRequired")};
    }

    if (record_index >= header_result.header.record_count) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.RecordIndexOutOfRange")};
    }

    const std::size_t record_offset = header_result.header.header_length +
                                      (record_index * header_result.header.record_length);
    const std::size_t field_offset = record_offset + field_it->offset;
    if ((field_offset + 4U) > table_bytes.size()) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.RecordDataTruncated")};
    }

    auto memo_bytes = read_binary_file(memo_path);
    if (memo_bytes.size() < 8U) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.MemoSidecarOpenFailed")};
    }

    const std::uint16_t block_size = read_be_u16(memo_bytes, 6U);
    if (block_size == 0U) {
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.MemoSidecarBlockSizeInvalid")};
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
        return {.ok = false, .error = visual_asset_text("VisualAssetEditor.Storage.MemoSidecarNextFreeBlockInvalid")};
    }

    const auto required_bytes = static_cast<std::size_t>(8U + encoded_bytes.size());
    const auto required_blocks = static_cast<std::uint32_t>((required_bytes + block_size - 1U) / block_size);
    const std::size_t new_block_offset = static_cast<std::size_t>(next_free_block) * block_size;
    const std::size_t new_total_size = new_block_offset + (static_cast<std::size_t>(required_blocks) * block_size);
    if (memo_bytes.size() < new_total_size) {
        memo_bytes.resize(new_total_size, 0U);
    }

    for (std::size_t index = 0; index < original_block_header.size(); ++index) {
        memo_bytes[new_block_offset + index] = original_block_header[index];
    }
    write_be_u32(memo_bytes, new_block_offset + 4U, static_cast<std::uint32_t>(encoded_bytes.size()));
    std::fill(
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(new_block_offset + 8U),
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(new_total_size),
        static_cast<std::uint8_t>(0U));
    std::copy(
        encoded_bytes.begin(),
        encoded_bytes.end(),
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(new_block_offset + 8U));

    write_be_u32(memo_bytes, 0U, next_free_block + required_blocks);
    write_le_u32(table_bytes, field_offset, next_free_block);

    return write_visual_asset_file_transaction(
        table_path,
        table_bytes,
        memo_path,
        memo_bytes);
}

}  // namespace copperfin::vfp
