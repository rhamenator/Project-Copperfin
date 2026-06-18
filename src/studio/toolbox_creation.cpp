#include "copperfin/studio/toolbox_creation.h"

#include "copperfin/studio/toolbox_palette.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::studio {

namespace {

[[nodiscard]] vfp::VisualObjectCreateResult failed_create_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .parent_name = {}
    };
}

[[nodiscard]] vfp::VisualObjectCreateBatchResult failed_batch_create_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .record_indexes = {},
        .created_objects = {}
    };
}

[[nodiscard]] StudioToolboxObjectCreatePlanResult failed_plan_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .plan = {}
    };
}

[[nodiscard]] StudioToolboxObjectCreateBatchPlanResult failed_batch_plan_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .plan = {}
    };
}

[[nodiscard]] std::string trimmed_copy(std::string_view value) {
    std::size_t first = 0U;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])) != 0) {
        ++first;
    }

    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1U])) != 0) {
        --last;
    }

    return std::string(value.substr(first, last - first));
}

[[nodiscard]] std::string normalized_identity(std::string_view value) {
    std::string normalized = trimmed_copy(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return normalized;
}

[[nodiscard]] const vfp::DbfRecordValue* find_record_value(
    const vfp::DbfRecord& record,
    std::string_view field_name) {
    const std::string normalized_field_name = normalized_identity(field_name);
    const auto found = std::find_if(record.values.begin(), record.values.end(), [&](const vfp::DbfRecordValue& value) {
        return normalized_identity(value.field_name) == normalized_field_name;
    });
    return found == record.values.end() ? nullptr : &*found;
}

[[nodiscard]] bool table_has_identity(
    const vfp::DbfTable& table,
    std::string_view field_name,
    std::string_view candidate) {
    const std::string normalized_candidate = normalized_identity(candidate);
    if (normalized_candidate.empty()) {
        return false;
    }

    for (const auto& record : table.records) {
        const auto* value = find_record_value(record, field_name);
        if (value != nullptr && normalized_identity(value->display_value) == normalized_candidate) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool table_has_object_name(const vfp::DbfTable& table, std::string_view candidate) {
    return table_has_identity(table, "OBJNAME", candidate) || table_has_identity(table, "NAME", candidate);
}

[[nodiscard]] bool reserved_identity_has(
    const std::vector<std::string>& reserved_identities,
    std::string_view candidate) {
    const std::string normalized_candidate = normalized_identity(candidate);
    if (normalized_candidate.empty()) {
        return false;
    }
    return std::find_if(
        reserved_identities.begin(),
        reserved_identities.end(),
        [&](const std::string& reserved_identity) {
            return normalized_identity(reserved_identity) == normalized_candidate;
        }) != reserved_identities.end();
}

[[nodiscard]] bool table_or_reservations_have_object_name(
    const vfp::DbfTable& table,
    const std::vector<std::string>& reserved_object_names,
    std::string_view candidate) {
    return table_has_object_name(table, candidate) || reserved_identity_has(reserved_object_names, candidate);
}

[[nodiscard]] bool table_or_reservations_have_unique_id(
    const vfp::DbfTable& table,
    const std::vector<std::string>& reserved_unique_ids,
    std::string_view candidate) {
    return table_has_identity(table, "UNIQUEID", candidate) || reserved_identity_has(reserved_unique_ids, candidate);
}

[[nodiscard]] std::optional<StudioToolboxItemDescriptor> find_toolbox_item(std::string_view toolbox_item_id) {
    const auto& items = studio_toolbox_palette();
    const std::string normalized_item_id = normalized_identity(toolbox_item_id);
    const auto found = std::find_if(items.begin(), items.end(), [&](const StudioToolboxItemDescriptor& item) {
        return normalized_identity(item.id) == normalized_item_id;
    });
    if (found == items.end()) {
        return std::nullopt;
    }
    return *found;
}

[[nodiscard]] std::string generate_default_object_name(
    const StudioToolboxItemDescriptor& item,
    const vfp::DbfTable& table,
    const std::vector<std::string>& reserved_object_names = {}) {
    const std::string prefix = trimmed_copy(item.default_name_prefix);
    for (std::size_t ordinal = 1U; ordinal < std::numeric_limits<std::size_t>::max(); ++ordinal) {
        const std::string candidate = prefix + std::to_string(ordinal);
        if (!table_or_reservations_have_object_name(table, reserved_object_names, candidate)) {
            return candidate;
        }
    }
    return {};
}

[[nodiscard]] bool toolbox_item_supports_context(
    const StudioToolboxItemDescriptor& item,
    StudioToolboxContext context) {
    return std::find(item.contexts.begin(), item.contexts.end(), context) != item.contexts.end();
}

[[nodiscard]] StudioToolboxObjectCreatePlanResult build_plan_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request,
    const StudioToolboxItemDescriptor& item,
    const vfp::DbfTable& table,
    std::size_t target_record_index,
    const std::vector<std::string>& reserved_object_names,
    const std::vector<std::string>& reserved_unique_ids) {
    std::string object_name = trimmed_copy(request.object_name);
    if (object_name.empty()) {
        object_name = generate_default_object_name(item, table, reserved_object_names);
    }
    if (object_name.empty()) {
        return failed_plan_result("A unique object name could not be generated for the requested toolbox item.");
    }
    if (table_or_reservations_have_object_name(table, reserved_object_names, object_name)) {
        return failed_plan_result("The requested toolbox object identity already exists in the asset.");
    }

    const std::string unique_id = trimmed_copy(request.unique_id);
    if (!unique_id.empty() && table_or_reservations_have_unique_id(table, reserved_unique_ids, unique_id)) {
        return failed_plan_result("The requested toolbox object identity already exists in the asset.");
    }

    std::vector<vfp::VisualObjectPropertyChange> field_values{
        {.property_name = "OBJNAME", .property_value = object_name},
        {.property_name = "NAME", .property_value = object_name},
        {.property_name = "CLASS", .property_value = std::string(item.vfp_class)},
        {.property_name = "BASECLASS", .property_value = std::string(item.base_class)}
    };

    if (!unique_id.empty()) {
        field_values.push_back({.property_name = "UNIQUEID", .property_value = unique_id});
    }

    const std::string parent_name = trimmed_copy(request.parent_name);
    if (!parent_name.empty()) {
        field_values.push_back({.property_name = "PARENT", .property_value = parent_name});
    }

    field_values.insert(field_values.end(), request.field_values.begin(), request.field_values.end());

    return {
        .ok = true,
        .error = {},
        .plan = {
            .path = request.path,
            .toolbox_item = item,
            .toolbox_context_provided = request.toolbox_context_provided,
            .toolbox_context = request.toolbox_context,
            .target_record_index = target_record_index,
            .object_name = object_name,
            .unique_id = unique_id,
            .parent_name = parent_name,
            .field_values = std::move(field_values),
            .dry_run = true,
            .mutates_asset = false
        }
    };
}

}  // namespace

StudioToolboxObjectCreatePlanResult plan_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request) {
    if (request.path.empty()) {
        return failed_plan_result("No asset path was provided.");
    }

    const auto item = find_toolbox_item(request.toolbox_item_id);
    if (!item.has_value()) {
        return failed_plan_result("The requested toolbox item was not found.");
    }
    if (request.toolbox_context_provided && !toolbox_item_supports_context(*item, request.toolbox_context)) {
        return failed_plan_result("The requested toolbox item is not available in the requested designer context.");
    }

    const auto table_result = vfp::parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return failed_plan_result(table_result.error);
    }

    return build_plan_from_toolbox_item(
        request,
        *item,
        table_result.table,
        table_result.table.records.size(),
        {},
        {});
}

StudioToolboxObjectCreateBatchPlanResult plan_visual_objects_from_toolbox_items(
    const StudioToolboxObjectCreateBatchPlanRequest& request) {
    if (request.path.empty()) {
        return failed_batch_plan_result("No asset path was provided.");
    }
    if (request.items.empty()) {
        return failed_batch_plan_result("No toolbox object creates were provided.");
    }

    const auto table_result = vfp::parse_dbf_table_from_file(request.path, std::numeric_limits<std::size_t>::max());
    if (!table_result.ok) {
        return failed_batch_plan_result(table_result.error);
    }

    std::vector<std::string> reserved_object_names;
    std::vector<std::string> reserved_unique_ids;
    std::vector<StudioToolboxObjectCreatePlan> plans;
    plans.reserve(request.items.size());

    const auto& table = table_result.table;
    const std::size_t first_target_record_index = table.records.size();
    for (const auto& item_request : request.items) {
        const auto item = find_toolbox_item(item_request.toolbox_item_id);
        if (!item.has_value()) {
            return failed_batch_plan_result("The requested toolbox item was not found.");
        }
        if (request.toolbox_context_provided && !toolbox_item_supports_context(*item, request.toolbox_context)) {
            return failed_batch_plan_result(
                "The requested toolbox item is not available in the requested designer context.");
        }

        const auto plan_result = build_plan_from_toolbox_item(
            {
                .path = request.path,
                .toolbox_item_id = item_request.toolbox_item_id,
                .object_name = item_request.object_name,
                .unique_id = item_request.unique_id,
                .parent_name = item_request.parent_name,
                .toolbox_context_provided = request.toolbox_context_provided,
                .toolbox_context = request.toolbox_context,
                .field_values = item_request.field_values
            },
            *item,
            table,
            first_target_record_index + plans.size(),
            reserved_object_names,
            reserved_unique_ids);
        if (!plan_result.ok) {
            return failed_batch_plan_result(plan_result.error);
        }

        reserved_object_names.push_back(plan_result.plan.object_name);
        if (!plan_result.plan.unique_id.empty()) {
            reserved_unique_ids.push_back(plan_result.plan.unique_id);
        }
        plans.push_back(plan_result.plan);
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .path = request.path,
            .toolbox_context_provided = request.toolbox_context_provided,
            .toolbox_context = request.toolbox_context,
            .item_count = plans.size(),
            .plans = std::move(plans),
            .dry_run = true,
            .mutates_asset = false
        }
    };
}

StudioToolboxObjectCreatePlanCatalogResult plan_visual_object_catalog_from_toolbox_context(
    const StudioToolboxObjectCreatePlanCatalogRequest& request) {
    const auto items = studio_toolbox_items_for_context(request.toolbox_context);
    if (items.empty()) {
        return {
            .ok = false,
            .error = "A toolbox object creation catalog request requires validated toolbox item metadata.",
            .toolbox_context = request.toolbox_context,
            .item_count = 0U,
            .plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioToolboxObjectCreatePlanCatalogEntry> entries;
    entries.reserve(items.size());
    std::size_t plan_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;
    for (const auto& item : items) {
        auto create_plan = plan_visual_object_from_toolbox_item({
            .path = request.path,
            .toolbox_item_id = std::string(item.id),
            .object_name = {},
            .unique_id = {},
            .parent_name = request.parent_name,
            .toolbox_context_provided = true,
            .toolbox_context = request.toolbox_context,
            .field_values = request.field_values
        });
        if (create_plan.ok) {
            ++plan_count;
            dry_run = dry_run && create_plan.plan.dry_run;
            mutates_asset = mutates_asset || create_plan.plan.mutates_asset;
        } else {
            ++error_count;
        }
        entries.push_back({
            .toolbox_item = item,
            .create_plan = std::move(create_plan)
        });
    }

    return {
        .ok = true,
        .error = {},
        .toolbox_context = request.toolbox_context,
        .item_count = items.size(),
        .plan_count = plan_count,
        .error_count = error_count,
        .dry_run = dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

vfp::VisualObjectCreateResult create_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request) {
    const auto plan_result = plan_visual_object_from_toolbox_item(request);
    if (!plan_result.ok) {
        return failed_create_result(plan_result.error);
    }

    return vfp::create_visual_object({
        .path = plan_result.plan.path,
        .field_values = plan_result.plan.field_values
    });
}

vfp::VisualObjectCreateBatchResult create_visual_objects_from_toolbox_items(
    const StudioToolboxObjectCreateBatchPlanRequest& request) {
    const auto plan_result = plan_visual_objects_from_toolbox_items(request);
    if (!plan_result.ok) {
        return failed_batch_create_result(plan_result.error);
    }

    std::vector<vfp::VisualObjectCreateBatchItem> objects;
    objects.reserve(plan_result.plan.plans.size());
    for (const auto& plan : plan_result.plan.plans) {
        objects.push_back({
            .field_values = plan.field_values
        });
    }

    return vfp::create_visual_objects({
        .path = plan_result.plan.path,
        .objects = std::move(objects)
    });
}

}  // namespace copperfin::studio
