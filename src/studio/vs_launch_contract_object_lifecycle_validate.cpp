// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

std::optional<LaunchParseResult> validate_object_lifecycle(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog) {
if (result.request.rename_object &&
        result.request.new_object_name.empty() &&
        result.request.new_name.empty() &&
        result.request.new_unique_id.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_command_requires_options(
            catalog,
            localized_object_command_rename(catalog),
            localized_object_rename_required_options(catalog))};
    }

if (result.request.reparent_object &&
        !result.request.clear_parent &&
        result.request.parent_name.empty() &&
        result.request.parent_unique_id.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_command_requires_options(
            catalog,
            localized_object_command_reparent(catalog),
            localized_object_reparent_required_options(catalog))};
    }

if (result.request.reorder_object && result.request.placement.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_command_requires_options(
            catalog,
            localized_object_command_reorder(catalog),
            "--placement")};
    }

if (result.request.group_object && result.request.field_values.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_group_requires_field_value(catalog)};
    }

if (result.request.group_object && result.request.group_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_group_requires_grouped_child_selector(catalog)};
    }

if (!result.request.group_object && !result.request.field_values.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_field_value_only_with_group_object(catalog)};
    }

if (!result.request.group_object && !result.request.group_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_grouped_child_selectors_only_with_group_object(catalog)};
    }

if (result.request.deleted_states && result.request.deleted_state_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_request_requires_selector(
            catalog,
            catalog.translate("StudioHost.LaunchParse.Request.DeletedStates"),
            catalog.translate("StudioHost.LaunchParse.Selector.Target"))};
    }

if (result.request.deleted_states) {
        for (const auto& object : result.request.deleted_state_objects) {
            if (!object.deleted_available) {
                return LaunchParseResult{.ok = false, .error = localized_request_item_requires_option_after_target(
                    catalog,
                    catalog.translate("StudioHost.LaunchParse.Request.DeletedStates"),
                    catalog.translate("StudioHost.LaunchParse.Selector.Target"),
                    "--deleted-state")};
            }
        }
    }

if (!result.request.deleted_states && !result.request.deleted_state_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_request_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.Request.DeletedStateTargetTitle"),
            "--deleted-states")};
    }

if (result.request.subtree_deleted_state && !result.request.subtree_deleted_available) {
        return LaunchParseResult{.ok = false, .error = localized_request_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.Request.SubtreeDeletedState"),
            "--subtree-deleted")};
    }

if (result.request.subtree_deleted_state &&
        result.request.object_name.empty() &&
        result.request.unique_id.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_request_requires_selector(
            catalog,
            catalog.translate("StudioHost.LaunchParse.Request.SubtreeDeletedState"),
            catalog.translate("StudioHost.LaunchParse.Selector.Root"))};
    }

if (!result.request.subtree_deleted_state && result.request.subtree_deleted_available) {
        return LaunchParseResult{.ok = false, .error = localized_request_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.Request.SubtreeDeletedStateTitle"),
            "--subtree-deleted-state")};
    }
    return std::nullopt;
}

}  // namespace copperfin::studio
