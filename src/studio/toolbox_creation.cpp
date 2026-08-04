// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/toolbox_creation.h"

#include "copperfin/localization/localization.h"
#include "copperfin/studio/toolbox_palette.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace copperfin::studio {

namespace {

copperfin::localization::LocalizedCatalog toolbox_creation_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        copperfin::localization::LocalizedCatalog catalog;
    };
    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        copperfin::localization::load_catalogs(
            copperfin::localization::resolve_catalog_root(),
            copperfin::localization::default_locale)};
    const auto locale_root = copperfin::localization::resolve_catalog_root();
    const auto locale = copperfin::localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = copperfin::localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

std::string toolbox_creation_text(std::string_view key) {
    return toolbox_creation_catalog().translate(key);
}

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

[[nodiscard]] StudioToolboxObjectCreateDispatchResult failed_dispatch_result(std::string error) {
    return {
        .ok = false,
        .error = std::move(error),
        .plan = {}
    };
}

[[nodiscard]] StudioToolboxObjectCreateBatchDispatchResult failed_batch_dispatch_result(std::string error) {
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

[[nodiscard]] bool has_field_value_named(
    const std::vector<vfp::VisualObjectPropertyChange>& field_values,
    std::string_view property_name) {
    const std::string normalized_property_name = normalized_identity(property_name);
    return std::find_if(
        field_values.begin(),
        field_values.end(),
        [&](const vfp::VisualObjectPropertyChange& field_value) {
            return normalized_identity(field_value.property_name) == normalized_property_name;
        }) != field_values.end();
}

void append_argument(std::vector<std::string>& arguments, std::string key, std::string value) {
    arguments.push_back(std::move(key));
    arguments.push_back(std::move(value));
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
        return failed_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Error.UniqueObjectNameUnavailable"));
    }
    if (table_or_reservations_have_object_name(table, reserved_object_names, object_name)) {
        return failed_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Error.ObjectIdentityExists"));
    }

    const std::string unique_id = trimmed_copy(request.unique_id);
    if (!unique_id.empty() && table_or_reservations_have_unique_id(table, reserved_unique_ids, unique_id)) {
        return failed_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Error.ObjectIdentityExists"));
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
        return failed_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Error.AssetPathRequired"));
    }

    const auto item = find_toolbox_item(request.toolbox_item_id);
    if (!item.has_value()) {
        return failed_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Error.ItemNotFound"));
    }
    if (request.toolbox_context_provided && !toolbox_item_supports_context(*item, request.toolbox_context)) {
        return failed_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Error.ItemUnavailableForContext"));
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

StudioSelectionToolboxObjectCreatePlanResult plan_visual_object_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreatePlanRequest& request) {
    auto launch_plan = plan_studio_toolbox_palette_launch({
        .selection_context = request.selection_context,
        .asset_path = request.path,
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    if (!launch_plan.ok) {
        return {
            .ok = false,
            .error = toolbox_creation_text("Studio.ToolboxCreation.SelectionPlan.Error.PaletteRequired"),
            .selection_context = request.selection_context,
            .toolbox_context = StudioToolboxContext::form,
            .launch_plan = std::move(launch_plan),
            .create_plan = {},
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto create_plan = plan_visual_object_from_toolbox_item({
        .path = request.path,
        .toolbox_item_id = request.toolbox_item_id,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .parent_name = request.parent_name,
        .toolbox_context_provided = true,
        .toolbox_context = launch_plan.plan.toolbox_context,
        .field_values = request.field_values
    });
    if (!create_plan.ok) {
        return {
            .ok = false,
            .error = create_plan.error,
            .selection_context = request.selection_context,
            .toolbox_context = launch_plan.plan.toolbox_context,
            .launch_plan = std::move(launch_plan),
            .create_plan = std::move(create_plan),
            .dry_run = true,
            .mutates_asset = false
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = create_plan.plan.toolbox_context,
        .launch_plan = std::move(launch_plan),
        .create_plan = std::move(create_plan),
        .dry_run = true,
        .mutates_asset = false
    };
}

StudioToolboxObjectCreatePlanResult plan_visual_object_from_toolbox_dispatch(
    const StudioToolboxObjectCreateFromPaletteDispatchRequest& request) {
    const auto& dispatch_plan = request.dispatch_plan;
    if (!dispatch_plan.dispatch_admitted || dispatch_plan.dry_run || dispatch_plan.executed) {
        return failed_plan_result(
            toolbox_creation_text("Studio.ToolboxCreation.FromDispatch.Error.AdmittedDispatchRequired"));
    }
    if (dispatch_plan.asset_path.empty()) {
        return failed_plan_result(toolbox_creation_text("Studio.ToolboxCreation.FromDispatch.Error.AssetPathRequired"));
    }
    if (dispatch_plan.items.empty() || dispatch_plan.item_count == 0U) {
        return failed_plan_result(
            toolbox_creation_text("Studio.ToolboxCreation.FromDispatch.Error.ValidatedItemMetadataRequired"));
    }
    if (dispatch_plan.item_count != dispatch_plan.items.size()) {
        return failed_plan_result(
            toolbox_creation_text("Studio.ToolboxCreation.FromDispatch.Error.ConsistentItemMetadataRequired"));
    }

    const std::string requested_item_id = normalized_identity(request.toolbox_item_id);
    const auto item_found = std::find_if(
        dispatch_plan.items.begin(),
        dispatch_plan.items.end(),
        [&](const StudioToolboxItemDescriptor& item) {
            return normalized_identity(item.id) == requested_item_id;
        });
    if (item_found == dispatch_plan.items.end()) {
        return failed_plan_result(
            toolbox_creation_text("Studio.ToolboxCreation.Error.ItemUnavailableForAdmittedDispatch"));
    }

    const std::string parent_name = trimmed_copy(request.parent_name).empty()
        ? dispatch_plan.object_name
        : request.parent_name;
    return plan_visual_object_from_toolbox_item({
        .path = dispatch_plan.asset_path,
        .toolbox_item_id = request.toolbox_item_id,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .parent_name = parent_name,
        .toolbox_context_provided = true,
        .toolbox_context = dispatch_plan.toolbox_context,
        .field_values = request.field_values
    });
}

StudioToolboxObjectCreateBatchPlanResult plan_visual_objects_from_toolbox_items(
    const StudioToolboxObjectCreateBatchPlanRequest& request) {
    if (request.path.empty()) {
        return failed_batch_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Error.AssetPathRequired"));
    }
    if (request.items.empty()) {
        return failed_batch_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Batch.Error.NoCreatesProvided"));
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
            return failed_batch_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Error.ItemNotFound"));
        }
        if (request.toolbox_context_provided && !toolbox_item_supports_context(*item, request.toolbox_context)) {
            return failed_batch_plan_result(
                toolbox_creation_text("Studio.ToolboxCreation.Error.ItemUnavailableForContext"));
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

StudioSelectionToolboxObjectCreateBatchPlanResult plan_visual_objects_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchPlanRequest& request) {
    auto launch_plan = plan_studio_toolbox_palette_launch({
        .selection_context = request.selection_context,
        .asset_path = request.path,
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    if (!launch_plan.ok) {
        return {
            .ok = false,
            .error = toolbox_creation_text("Studio.ToolboxCreation.SelectionBatchPlan.Error.PaletteRequired"),
            .selection_context = request.selection_context,
            .toolbox_context = StudioToolboxContext::form,
            .launch_plan = std::move(launch_plan),
            .item_count = 0U,
            .plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .batch_plan = {}
        };
    }

    auto batch_plan = plan_visual_objects_from_toolbox_items({
        .path = request.path,
        .toolbox_context_provided = true,
        .toolbox_context = launch_plan.plan.toolbox_context,
        .items = request.items
    });
    if (!batch_plan.ok) {
        return {
            .ok = false,
            .error = batch_plan.error,
            .selection_context = request.selection_context,
            .toolbox_context = launch_plan.plan.toolbox_context,
            .launch_plan = std::move(launch_plan),
            .item_count = request.items.size(),
            .plan_count = 0U,
            .error_count = 1U,
            .dry_run = true,
            .mutates_asset = false,
            .batch_plan = std::move(batch_plan)
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = batch_plan.plan.toolbox_context,
        .launch_plan = std::move(launch_plan),
        .item_count = batch_plan.plan.item_count,
        .plan_count = 1U,
        .error_count = 0U,
        .dry_run = batch_plan.plan.dry_run,
        .mutates_asset = batch_plan.plan.mutates_asset,
        .batch_plan = std::move(batch_plan)
    };
}

StudioToolboxObjectCreateBatchPlanResult plan_visual_objects_from_toolbox_dispatch(
    const StudioToolboxObjectCreateBatchFromPaletteDispatchRequest& request) {
    const auto& dispatch_plan = request.dispatch_plan;
    if (!dispatch_plan.dispatch_admitted || dispatch_plan.dry_run || dispatch_plan.executed) {
        return failed_batch_plan_result(
            toolbox_creation_text("Studio.ToolboxCreation.BatchFromDispatch.Error.AdmittedDispatchRequired"));
    }
    if (dispatch_plan.asset_path.empty()) {
        return failed_batch_plan_result(
            toolbox_creation_text("Studio.ToolboxCreation.BatchFromDispatch.Error.AssetPathRequired"));
    }
    if (dispatch_plan.items.empty() || dispatch_plan.item_count == 0U) {
        return failed_batch_plan_result(
            toolbox_creation_text("Studio.ToolboxCreation.BatchFromDispatch.Error.ValidatedItemMetadataRequired"));
    }
    if (dispatch_plan.item_count != dispatch_plan.items.size()) {
        return failed_batch_plan_result(
            toolbox_creation_text("Studio.ToolboxCreation.BatchFromDispatch.Error.ConsistentItemMetadataRequired"));
    }
    if (request.items.empty()) {
        return failed_batch_plan_result(toolbox_creation_text("Studio.ToolboxCreation.Batch.Error.NoCreatesProvided"));
    }

    std::vector<StudioToolboxObjectCreateBatchItem> batch_items;
    batch_items.reserve(request.items.size());
    for (const auto& item_request : request.items) {
        const std::string requested_item_id = normalized_identity(item_request.toolbox_item_id);
        const auto item_found = std::find_if(
            dispatch_plan.items.begin(),
            dispatch_plan.items.end(),
            [&](const StudioToolboxItemDescriptor& item) {
                return normalized_identity(item.id) == requested_item_id;
            });
        if (item_found == dispatch_plan.items.end()) {
            return failed_batch_plan_result(
                toolbox_creation_text("Studio.ToolboxCreation.Error.ItemUnavailableForAdmittedDispatch"));
        }

        const std::string parent_name = trimmed_copy(item_request.parent_name).empty()
            ? dispatch_plan.object_name
            : item_request.parent_name;
        batch_items.push_back({
            .toolbox_item_id = item_request.toolbox_item_id,
            .object_name = item_request.object_name,
            .unique_id = item_request.unique_id,
            .parent_name = parent_name,
            .field_values = item_request.field_values
        });
    }

    return plan_visual_objects_from_toolbox_items({
        .path = dispatch_plan.asset_path,
        .toolbox_context_provided = true,
        .toolbox_context = dispatch_plan.toolbox_context,
        .items = std::move(batch_items)
    });
}

StudioToolboxObjectCreateDispatchResult plan_visual_object_create_dispatch(
    const StudioToolboxObjectCreateDispatchRequest& request) {
    const auto& create_plan = request.create_plan;
    if (create_plan.toolbox_item.id.empty() ||
            create_plan.toolbox_item.vfp_class.empty() ||
            create_plan.toolbox_item.base_class.empty()) {
        return failed_dispatch_result(
            toolbox_creation_text("Studio.ToolboxCreation.Dispatch.Error.ValidatedItemMetadataRequired"));
    }
    if (create_plan.path.empty()) {
        return failed_dispatch_result(toolbox_creation_text("Studio.ToolboxCreation.Dispatch.Error.AssetPathRequired"));
    }
    if (trimmed_copy(create_plan.object_name).empty()) {
        return failed_dispatch_result(
            toolbox_creation_text("Studio.ToolboxCreation.Dispatch.Error.PlannedObjectNameRequired"));
    }
    if (create_plan.field_values.empty() ||
        !has_field_value_named(create_plan.field_values, "OBJNAME") ||
        !has_field_value_named(create_plan.field_values, "NAME") ||
        !has_field_value_named(create_plan.field_values, "CLASS") ||
        !has_field_value_named(create_plan.field_values, "BASECLASS")) {
        return failed_dispatch_result(
            toolbox_creation_text("Studio.ToolboxCreation.Dispatch.Error.DescriptorFieldValuesRequired"));
    }
    if (!request.admit_create_operation) {
        return failed_dispatch_result(
            toolbox_creation_text("Studio.ToolboxCreation.Dispatch.Error.AdmittedCreateOperationRequired"));
    }

    std::vector<std::string> arguments;
    append_argument(arguments, "--path", create_plan.path);
    append_argument(arguments, "--toolbox-create", std::string(create_plan.toolbox_item.id));
    if (create_plan.toolbox_context_provided) {
        append_argument(arguments, "--toolbox-context", studio_toolbox_context_name(create_plan.toolbox_context));
    }
    append_argument(arguments, "--object-name", create_plan.object_name);
    if (!create_plan.unique_id.empty()) {
        append_argument(arguments, "--unique-id", create_plan.unique_id);
    }
    if (!create_plan.parent_name.empty()) {
        append_argument(arguments, "--parent-name", create_plan.parent_name);
    }
    for (const auto& field_value : create_plan.field_values) {
        append_argument(arguments, "--field-value", field_value.property_name + "=" + field_value.property_value);
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .path = create_plan.path,
            .toolbox_item = create_plan.toolbox_item,
            .toolbox_context_provided = create_plan.toolbox_context_provided,
            .toolbox_context = create_plan.toolbox_context,
            .target_record_index = create_plan.target_record_index,
            .object_name = create_plan.object_name,
            .unique_id = create_plan.unique_id,
            .parent_name = create_plan.parent_name,
            .field_values = create_plan.field_values,
            .dispatch_arguments = std::move(arguments),
            .dispatch_admitted = true,
            .dry_run = false,
            .executed = false,
            .mutates_asset = true
        }
    };
}

StudioSelectionToolboxObjectCreateDispatchResult plan_visual_object_create_dispatch_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateDispatchRequest& request) {
    auto create_plan = plan_visual_object_from_toolbox_selection(request.create_request);
    if (!create_plan.ok) {
        return {
            .ok = false,
            .error = create_plan.error,
            .selection_context = create_plan.selection_context,
            .toolbox_context = create_plan.toolbox_context,
            .launch_plan = create_plan.launch_plan,
            .create_plan = create_plan,
            .dispatch = {},
            .dispatch_count = 0U,
            .error_count = 1U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto dispatch = plan_visual_object_create_dispatch({
        .create_plan = create_plan.create_plan.plan,
        .admit_create_operation = request.admit_create_operation
    });
    if (!dispatch.ok) {
        return {
            .ok = false,
            .error = dispatch.error,
            .selection_context = create_plan.selection_context,
            .toolbox_context = create_plan.toolbox_context,
            .launch_plan = create_plan.launch_plan,
            .create_plan = create_plan,
            .dispatch = dispatch,
            .dispatch_count = 0U,
            .error_count = 1U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = create_plan.selection_context,
        .toolbox_context = create_plan.toolbox_context,
        .launch_plan = create_plan.launch_plan,
        .create_plan = create_plan,
        .dispatch = dispatch,
        .dispatch_count = 1U,
        .error_count = 0U,
        .dry_run = dispatch.plan.dry_run,
        .mutates_asset = dispatch.plan.mutates_asset
    };
}

StudioToolboxObjectCreateDispatchResult plan_visual_object_create_dispatch_from_toolbox_dispatch(
    const StudioToolboxObjectCreateDispatchFromPaletteDispatchRequest& request) {
    const auto create_plan = plan_visual_object_from_toolbox_dispatch(request.create_request);
    if (!create_plan.ok) {
        return failed_dispatch_result(create_plan.error);
    }
    return plan_visual_object_create_dispatch({
        .create_plan = create_plan.plan,
        .admit_create_operation = request.admit_create_operation
    });
}

StudioToolboxObjectCreateBatchDispatchResult plan_visual_object_batch_create_dispatch(
    const StudioToolboxObjectCreateBatchDispatchRequest& request) {
    const auto& batch_plan = request.batch_plan;
    if (batch_plan.path.empty()) {
        return failed_batch_dispatch_result(
            toolbox_creation_text("Studio.ToolboxCreation.BatchDispatch.Error.AssetPathRequired"));
    }
    if (batch_plan.plans.empty() || batch_plan.item_count == 0U) {
        return failed_batch_dispatch_result(
            toolbox_creation_text("Studio.ToolboxCreation.BatchDispatch.Error.PlannedCreatesRequired"));
    }
    if (batch_plan.item_count != batch_plan.plans.size()) {
        return failed_batch_dispatch_result(
            toolbox_creation_text("Studio.ToolboxCreation.BatchDispatch.Error.ConsistentPlannedCreatesRequired"));
    }
    if (!request.admit_create_operation) {
        return failed_batch_dispatch_result(
            toolbox_creation_text("Studio.ToolboxCreation.BatchDispatch.Error.AdmittedCreateOperationRequired"));
    }

    std::vector<std::string> arguments;
    append_argument(arguments, "--path", batch_plan.path);
    arguments.push_back("--toolbox-create-batch");
    if (batch_plan.toolbox_context_provided) {
        append_argument(arguments, "--toolbox-context", studio_toolbox_context_name(batch_plan.toolbox_context));
    }

    for (const auto& create_plan : batch_plan.plans) {
        if (create_plan.toolbox_item.id.empty() ||
            create_plan.toolbox_item.vfp_class.empty() ||
            create_plan.toolbox_item.base_class.empty()) {
            return failed_batch_dispatch_result(
                toolbox_creation_text("Studio.ToolboxCreation.BatchDispatch.Error.ValidatedItemMetadataRequired"));
        }
        if (trimmed_copy(create_plan.object_name).empty()) {
            return failed_batch_dispatch_result(
                toolbox_creation_text("Studio.ToolboxCreation.BatchDispatch.Error.PlannedObjectNamesRequired"));
        }
        if (create_plan.field_values.empty() ||
            !has_field_value_named(create_plan.field_values, "OBJNAME") ||
            !has_field_value_named(create_plan.field_values, "NAME") ||
            !has_field_value_named(create_plan.field_values, "CLASS") ||
            !has_field_value_named(create_plan.field_values, "BASECLASS")) {
            return failed_batch_dispatch_result(
                toolbox_creation_text("Studio.ToolboxCreation.BatchDispatch.Error.DescriptorFieldValuesRequired"));
        }

        append_argument(arguments, "--toolbox-item", std::string(create_plan.toolbox_item.id));
        append_argument(arguments, "--object-name", create_plan.object_name);
        if (!create_plan.unique_id.empty()) {
            append_argument(arguments, "--unique-id", create_plan.unique_id);
        }
        if (!create_plan.parent_name.empty()) {
            append_argument(arguments, "--parent-name", create_plan.parent_name);
        }
        for (const auto& field_value : create_plan.field_values) {
            append_argument(arguments, "--field-value", field_value.property_name + "=" + field_value.property_value);
        }
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .path = batch_plan.path,
            .toolbox_context_provided = batch_plan.toolbox_context_provided,
            .toolbox_context = batch_plan.toolbox_context,
            .item_count = batch_plan.item_count,
            .plans = batch_plan.plans,
            .dispatch_arguments = std::move(arguments),
            .dispatch_admitted = true,
            .dry_run = false,
            .executed = false,
            .mutates_asset = true
        }
    };
}

StudioSelectionToolboxObjectCreateBatchDispatchResult
plan_visual_object_batch_create_dispatch_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchDispatchRequest& request) {
    auto batch_plan = plan_visual_objects_from_toolbox_selection(request.batch_request);
    if (!batch_plan.ok) {
        return {
            .ok = false,
            .error = batch_plan.error,
            .selection_context = batch_plan.selection_context,
            .toolbox_context = batch_plan.toolbox_context,
            .launch_plan = batch_plan.launch_plan,
            .item_count = batch_plan.item_count,
            .batch_plan = batch_plan,
            .dispatch = {},
            .dispatch_count = 0U,
            .error_count = batch_plan.error_count,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto dispatch = plan_visual_object_batch_create_dispatch({
        .batch_plan = batch_plan.batch_plan.plan,
        .admit_create_operation = request.admit_create_operation
    });
    if (!dispatch.ok) {
        return {
            .ok = false,
            .error = dispatch.error,
            .selection_context = batch_plan.selection_context,
            .toolbox_context = batch_plan.toolbox_context,
            .launch_plan = batch_plan.launch_plan,
            .item_count = batch_plan.item_count,
            .batch_plan = batch_plan,
            .dispatch = dispatch,
            .dispatch_count = 0U,
            .error_count = 1U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = batch_plan.selection_context,
        .toolbox_context = batch_plan.toolbox_context,
        .launch_plan = batch_plan.launch_plan,
        .item_count = batch_plan.item_count,
        .batch_plan = batch_plan,
        .dispatch = dispatch,
        .dispatch_count = 1U,
        .error_count = 0U,
        .dry_run = dispatch.plan.dry_run,
        .mutates_asset = dispatch.plan.mutates_asset
    };
}

StudioToolboxObjectCreateBatchDispatchResult plan_visual_object_batch_create_dispatch_from_toolbox_dispatch(
    const StudioToolboxObjectCreateBatchDispatchFromPaletteDispatchRequest& request) {
    const auto batch_plan = plan_visual_objects_from_toolbox_dispatch(request.create_request);
    if (!batch_plan.ok) {
        return failed_batch_dispatch_result(batch_plan.error);
    }
    return plan_visual_object_batch_create_dispatch({
        .batch_plan = batch_plan.plan,
        .admit_create_operation = request.admit_create_operation
    });
}

StudioToolboxObjectCreatePlanCatalogResult plan_visual_object_catalog_from_toolbox_context(
    const StudioToolboxObjectCreatePlanCatalogRequest& request) {
    const auto items = studio_toolbox_items_for_context(request.toolbox_context);
    if (items.empty()) {
        return {
            .ok = false,
            .error = toolbox_creation_text("Studio.ToolboxCreation.Catalog.Error.ValidatedItemMetadataRequired"),
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

StudioSelectionToolboxObjectCreatePlanCatalogResult plan_visual_object_catalog_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreatePlanCatalogRequest& request) {
    auto launch_plan = plan_studio_toolbox_palette_launch({
        .selection_context = request.selection_context,
        .asset_path = request.path,
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    if (!launch_plan.ok) {
        return {
            .ok = false,
            .error = toolbox_creation_text("Studio.ToolboxCreation.SelectionCatalog.Error.PaletteRequired"),
            .selection_context = request.selection_context,
            .toolbox_context = StudioToolboxContext::form,
            .launch_plan = std::move(launch_plan),
            .item_count = 0U,
            .plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    auto create_catalog = plan_visual_object_catalog_from_toolbox_context({
        .toolbox_context = launch_plan.plan.toolbox_context,
        .path = request.path,
        .parent_name = request.parent_name,
        .field_values = request.field_values
    });
    if (!create_catalog.ok) {
        return {
            .ok = false,
            .error = create_catalog.error,
            .selection_context = request.selection_context,
            .toolbox_context = launch_plan.plan.toolbox_context,
            .launch_plan = std::move(launch_plan),
            .item_count = 0U,
            .plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = create_catalog.toolbox_context,
        .launch_plan = std::move(launch_plan),
        .item_count = create_catalog.item_count,
        .plan_count = create_catalog.plan_count,
        .error_count = create_catalog.error_count,
        .dry_run = create_catalog.dry_run,
        .mutates_asset = create_catalog.mutates_asset,
        .entries = std::move(create_catalog.entries)
    };
}

StudioToolboxObjectCreateDispatchCatalogResult plan_visual_object_create_dispatch_catalog(
    const StudioToolboxObjectCreateDispatchCatalogRequest& request) {
    const auto create_catalog = plan_visual_object_catalog_from_toolbox_context({
        .toolbox_context = request.toolbox_context,
        .path = request.path,
        .parent_name = request.parent_name,
        .field_values = request.field_values
    });
    if (!create_catalog.ok) {
        return {
            .ok = false,
            .error = create_catalog.error,
            .toolbox_context = request.toolbox_context,
            .item_count = 0U,
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioToolboxObjectCreateDispatchCatalogEntry> entries;
    entries.reserve(create_catalog.entries.size());
    std::size_t dispatch_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;
    for (const auto& entry : create_catalog.entries) {
        StudioToolboxObjectCreateDispatchResult dispatch{};
        if (entry.create_plan.ok) {
            dispatch = plan_visual_object_create_dispatch({
                .create_plan = entry.create_plan.plan,
                .admit_create_operation = request.admit_create_operation
            });
        } else {
            dispatch = {
                .ok = false,
                .error = entry.create_plan.error,
                .plan = {}
            };
        }

        if (dispatch.ok) {
            ++dispatch_count;
            dry_run = dry_run && dispatch.plan.dry_run;
            mutates_asset = mutates_asset || dispatch.plan.mutates_asset;
        } else {
            ++error_count;
        }

        entries.push_back({
            .toolbox_item = entry.toolbox_item,
            .create_plan = entry.create_plan,
            .dispatch = std::move(dispatch)
        });
    }

    return {
        .ok = true,
        .error = {},
        .toolbox_context = request.toolbox_context,
        .item_count = create_catalog.item_count,
        .dispatch_count = dispatch_count,
        .error_count = error_count,
        .dry_run = dispatch_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

StudioSelectionToolboxObjectCreateDispatchCatalogResult
plan_visual_object_create_dispatch_catalog_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateDispatchCatalogRequest& request) {
    auto launch_plan = plan_studio_toolbox_palette_launch({
        .selection_context = request.selection_context,
        .asset_path = request.path,
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    if (!launch_plan.ok) {
        return {
            .ok = false,
            .error = toolbox_creation_text("Studio.ToolboxCreation.SelectionDispatchCatalog.Error.PaletteRequired"),
            .selection_context = request.selection_context,
            .toolbox_context = StudioToolboxContext::form,
            .launch_plan = std::move(launch_plan),
            .item_count = 0U,
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    auto dispatch_catalog = plan_visual_object_create_dispatch_catalog({
        .toolbox_context = launch_plan.plan.toolbox_context,
        .path = request.path,
        .parent_name = request.parent_name,
        .field_values = request.field_values,
        .admit_create_operation = request.admit_create_operation
    });
    if (!dispatch_catalog.ok) {
        return {
            .ok = false,
            .error = dispatch_catalog.error,
            .selection_context = request.selection_context,
            .toolbox_context = launch_plan.plan.toolbox_context,
            .launch_plan = std::move(launch_plan),
            .item_count = 0U,
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = dispatch_catalog.toolbox_context,
        .launch_plan = std::move(launch_plan),
        .item_count = dispatch_catalog.item_count,
        .dispatch_count = dispatch_catalog.dispatch_count,
        .error_count = dispatch_catalog.error_count,
        .dry_run = dispatch_catalog.dry_run,
        .mutates_asset = dispatch_catalog.mutates_asset,
        .entries = std::move(dispatch_catalog.entries)
    };
}

StudioToolboxObjectCreateBatchPlanCatalogResult plan_visual_object_batch_catalog_from_toolbox_context(
    const StudioToolboxObjectCreateBatchPlanCatalogRequest& request) {
    const auto items = studio_toolbox_items_for_context(request.toolbox_context);
    if (items.empty()) {
        return {
            .ok = false,
            .error = toolbox_creation_text("Studio.ToolboxCreation.BatchCatalog.Error.ValidatedItemMetadataRequired"),
            .toolbox_context = request.toolbox_context,
            .item_count = 0U,
            .plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .batch_plan = {}
        };
    }

    std::vector<StudioToolboxObjectCreateBatchItem> batch_items;
    batch_items.reserve(items.size());
    for (const auto& item : items) {
        batch_items.push_back({
            .toolbox_item_id = std::string(item.id),
            .object_name = {},
            .unique_id = {},
            .parent_name = request.parent_name,
            .field_values = request.field_values
        });
    }

    auto batch_plan = plan_visual_objects_from_toolbox_items({
        .path = request.path,
        .toolbox_context_provided = true,
        .toolbox_context = request.toolbox_context,
        .items = std::move(batch_items)
    });

    const bool plan_ok = batch_plan.ok;
    return {
        .ok = true,
        .error = {},
        .toolbox_context = request.toolbox_context,
        .item_count = items.size(),
        .plan_count = plan_ok ? 1U : 0U,
        .error_count = plan_ok ? 0U : 1U,
        .dry_run = plan_ok ? batch_plan.plan.dry_run : true,
        .mutates_asset = plan_ok ? batch_plan.plan.mutates_asset : false,
        .batch_plan = std::move(batch_plan)
    };
}

StudioSelectionToolboxObjectCreateBatchPlanCatalogResult
plan_visual_object_batch_catalog_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchPlanCatalogRequest& request) {
    auto launch_plan = plan_studio_toolbox_palette_launch({
        .selection_context = request.selection_context,
        .asset_path = request.path,
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    if (!launch_plan.ok) {
        return {
            .ok = false,
            .error = toolbox_creation_text("Studio.ToolboxCreation.SelectionBatchCatalog.Error.PaletteRequired"),
            .selection_context = request.selection_context,
            .toolbox_context = StudioToolboxContext::form,
            .launch_plan = std::move(launch_plan),
            .item_count = 0U,
            .plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .batch_plan = {}
        };
    }

    auto batch_catalog = plan_visual_object_batch_catalog_from_toolbox_context({
        .toolbox_context = launch_plan.plan.toolbox_context,
        .path = request.path,
        .parent_name = request.parent_name,
        .field_values = request.field_values
    });
    if (!batch_catalog.ok) {
        return {
            .ok = false,
            .error = batch_catalog.error,
            .selection_context = request.selection_context,
            .toolbox_context = launch_plan.plan.toolbox_context,
            .launch_plan = std::move(launch_plan),
            .item_count = 0U,
            .plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .batch_plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = batch_catalog.toolbox_context,
        .launch_plan = std::move(launch_plan),
        .item_count = batch_catalog.item_count,
        .plan_count = batch_catalog.plan_count,
        .error_count = batch_catalog.error_count,
        .dry_run = batch_catalog.dry_run,
        .mutates_asset = batch_catalog.mutates_asset,
        .batch_plan = std::move(batch_catalog.batch_plan)
    };
}

StudioToolboxObjectCreateBatchDispatchCatalogResult plan_visual_object_batch_create_dispatch_catalog(
    const StudioToolboxObjectCreateBatchDispatchCatalogRequest& request) {
    const auto items = studio_toolbox_items_for_context(request.toolbox_context);
    if (items.empty()) {
        return {
            .ok = false,
            .error = toolbox_creation_text(
                "Studio.ToolboxCreation.BatchDispatchCatalog.Error.ValidatedItemMetadataRequired"),
            .toolbox_context = request.toolbox_context,
            .item_count = 0U,
            .batch_plan = {},
            .dispatch = {},
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    std::vector<StudioToolboxObjectCreateBatchItem> batch_items;
    batch_items.reserve(items.size());
    for (const auto& item : items) {
        batch_items.push_back({
            .toolbox_item_id = std::string(item.id),
            .object_name = {},
            .unique_id = {},
            .parent_name = request.parent_name,
            .field_values = request.field_values
        });
    }

    auto batch_plan = plan_visual_objects_from_toolbox_items({
        .path = request.path,
        .toolbox_context_provided = true,
        .toolbox_context = request.toolbox_context,
        .items = std::move(batch_items)
    });

    StudioToolboxObjectCreateBatchDispatchResult dispatch{};
    if (batch_plan.ok) {
        dispatch = plan_visual_object_batch_create_dispatch({
            .batch_plan = batch_plan.plan,
            .admit_create_operation = request.admit_create_operation
        });
    } else {
        dispatch = {
            .ok = false,
            .error = batch_plan.error,
            .plan = {}
        };
    }

    const bool dispatch_ok = dispatch.ok;
    const bool dry_run = dispatch_ok ? dispatch.plan.dry_run : true;
    const bool mutates_asset = dispatch_ok ? dispatch.plan.mutates_asset : false;

    return {
        .ok = true,
        .error = {},
        .toolbox_context = request.toolbox_context,
        .item_count = items.size(),
        .batch_plan = std::move(batch_plan),
        .dispatch = std::move(dispatch),
        .dispatch_count = dispatch_ok ? 1U : 0U,
        .error_count = dispatch_ok ? 0U : 1U,
        .dry_run = dry_run,
        .mutates_asset = mutates_asset
    };
}

StudioSelectionToolboxObjectCreateBatchDispatchCatalogResult
plan_visual_object_batch_create_dispatch_catalog_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchDispatchCatalogRequest& request) {
    auto launch_plan = plan_studio_toolbox_palette_launch({
        .selection_context = request.selection_context,
        .asset_path = request.path,
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    if (!launch_plan.ok) {
        return {
            .ok = false,
            .error =
                toolbox_creation_text("Studio.ToolboxCreation.SelectionBatchDispatchCatalog.Error.PaletteRequired"),
            .selection_context = request.selection_context,
            .toolbox_context = StudioToolboxContext::form,
            .launch_plan = std::move(launch_plan),
            .item_count = 0U,
            .batch_plan = {},
            .dispatch = {},
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto dispatch_catalog = plan_visual_object_batch_create_dispatch_catalog({
        .toolbox_context = launch_plan.plan.toolbox_context,
        .path = request.path,
        .parent_name = request.parent_name,
        .field_values = request.field_values,
        .admit_create_operation = request.admit_create_operation
    });
    if (!dispatch_catalog.ok) {
        return {
            .ok = false,
            .error = dispatch_catalog.error,
            .selection_context = request.selection_context,
            .toolbox_context = launch_plan.plan.toolbox_context,
            .launch_plan = std::move(launch_plan),
            .item_count = 0U,
            .batch_plan = {},
            .dispatch = {},
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = dispatch_catalog.toolbox_context,
        .launch_plan = std::move(launch_plan),
        .item_count = dispatch_catalog.item_count,
        .batch_plan = std::move(dispatch_catalog.batch_plan),
        .dispatch = std::move(dispatch_catalog.dispatch),
        .dispatch_count = dispatch_catalog.dispatch_count,
        .error_count = dispatch_catalog.error_count,
        .dry_run = dispatch_catalog.dry_run,
        .mutates_asset = dispatch_catalog.mutates_asset
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

StudioSelectionToolboxObjectCreateResult create_visual_object_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreatePlanRequest& request) {
    auto plan_result = plan_visual_object_from_toolbox_selection(request);
    if (!plan_result.ok) {
        return {
            .ok = false,
            .error = plan_result.error,
            .selection_context = plan_result.selection_context,
            .toolbox_context = plan_result.toolbox_context,
            .launch_plan = plan_result.launch_plan,
            .create_plan = plan_result,
            .create_result = failed_create_result(plan_result.error),
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto create_result = vfp::create_visual_object({
        .path = plan_result.create_plan.plan.path,
        .field_values = plan_result.create_plan.plan.field_values
    });
    if (!create_result.ok) {
        return {
            .ok = false,
            .error = create_result.error,
            .selection_context = plan_result.selection_context,
            .toolbox_context = plan_result.toolbox_context,
            .launch_plan = plan_result.launch_plan,
            .create_plan = plan_result,
            .create_result = create_result,
            .dry_run = false,
            .mutates_asset = false
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = plan_result.selection_context,
        .toolbox_context = plan_result.toolbox_context,
        .launch_plan = plan_result.launch_plan,
        .create_plan = plan_result,
        .create_result = create_result,
        .dry_run = false,
        .mutates_asset = true
    };
}

StudioToolboxObjectCreateFromDispatchResult create_visual_object_from_toolbox_dispatch(
    const StudioToolboxObjectCreateFromPaletteDispatchRequest& request) {
    auto plan_result = plan_visual_object_from_toolbox_dispatch(request);
    if (!plan_result.ok) {
        return {
            .ok = false,
            .error = plan_result.error,
            .create_plan = plan_result,
            .create_result = failed_create_result(plan_result.error),
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto create_result = vfp::create_visual_object({
        .path = plan_result.plan.path,
        .field_values = plan_result.plan.field_values
    });
    if (!create_result.ok) {
        return {
            .ok = false,
            .error = create_result.error,
            .create_plan = plan_result,
            .create_result = create_result,
            .dry_run = false,
            .mutates_asset = false
        };
    }

    return {
        .ok = true,
        .error = {},
        .create_plan = plan_result,
        .create_result = create_result,
        .dry_run = false,
        .mutates_asset = true
    };
}

StudioSelectionToolboxObjectCreateBatchResult create_visual_objects_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchPlanRequest& request) {
    auto plan_result = plan_visual_objects_from_toolbox_selection(request);
    if (!plan_result.ok) {
        return {
            .ok = false,
            .error = plan_result.error,
            .selection_context = plan_result.selection_context,
            .toolbox_context = plan_result.toolbox_context,
            .launch_plan = plan_result.launch_plan,
            .item_count = plan_result.item_count,
            .batch_plan = plan_result,
            .create_result = failed_batch_create_result(plan_result.error),
            .dry_run = true,
            .mutates_asset = false
        };
    }

    std::vector<vfp::VisualObjectCreateBatchItem> objects;
    objects.reserve(plan_result.batch_plan.plan.plans.size());
    for (const auto& plan : plan_result.batch_plan.plan.plans) {
        objects.push_back({
            .field_values = plan.field_values
        });
    }

    auto create_result = vfp::create_visual_objects({
        .path = plan_result.batch_plan.plan.path,
        .objects = std::move(objects)
    });
    if (!create_result.ok) {
        return {
            .ok = false,
            .error = create_result.error,
            .selection_context = plan_result.selection_context,
            .toolbox_context = plan_result.toolbox_context,
            .launch_plan = plan_result.launch_plan,
            .item_count = plan_result.item_count,
            .batch_plan = plan_result,
            .create_result = create_result,
            .dry_run = false,
            .mutates_asset = false
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = plan_result.selection_context,
        .toolbox_context = plan_result.toolbox_context,
        .launch_plan = plan_result.launch_plan,
        .item_count = plan_result.item_count,
        .batch_plan = plan_result,
        .create_result = create_result,
        .dry_run = false,
        .mutates_asset = true
    };
}

StudioToolboxObjectCreateBatchFromDispatchResult create_visual_objects_from_toolbox_dispatch(
    const StudioToolboxObjectCreateBatchFromPaletteDispatchRequest& request) {
    auto plan_result = plan_visual_objects_from_toolbox_dispatch(request);
    if (!plan_result.ok) {
        return {
            .ok = false,
            .error = plan_result.error,
            .batch_plan = plan_result,
            .create_result = failed_batch_create_result(plan_result.error),
            .dry_run = true,
            .mutates_asset = false
        };
    }

    std::vector<vfp::VisualObjectCreateBatchItem> objects;
    objects.reserve(plan_result.plan.plans.size());
    for (const auto& plan : plan_result.plan.plans) {
        objects.push_back({
            .field_values = plan.field_values
        });
    }

    auto create_result = vfp::create_visual_objects({
        .path = plan_result.plan.path,
        .objects = std::move(objects)
    });
    if (!create_result.ok) {
        return {
            .ok = false,
            .error = create_result.error,
            .batch_plan = plan_result,
            .create_result = create_result,
            .dry_run = false,
            .mutates_asset = false
        };
    }

    return {
        .ok = true,
        .error = {},
        .batch_plan = plan_result,
        .create_result = create_result,
        .dry_run = false,
        .mutates_asset = true
    };
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
