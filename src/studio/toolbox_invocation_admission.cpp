// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/toolbox_invocation_admission.h"

#include "copperfin/localization/localization.h"

#include <string>
#include <string_view>
#include <mutex>
#include <utility>

namespace copperfin::studio {

namespace {

copperfin::localization::LocalizedCatalog toolbox_invocation_admission_catalog() {
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

std::string toolbox_invocation_admission_text(std::string_view key) {
    return toolbox_invocation_admission_catalog().translate(key);
}

}  // namespace

StudioToolboxInvocationAdmissionResult plan_studio_toolbox_invocation_admission(
    const StudioToolboxInvocationAdmissionRequest& request) {
    if (request.launch_plan.items.empty() || request.launch_plan.item_count == 0U) {
        return {
            .ok = false,
            .error = toolbox_invocation_admission_text(
                "Studio.ToolboxInvocationAdmission.Error.ValidatedItemMetadataRequired"),
            .plan = {}
        };
    }
    if (request.launch_plan.item_count != request.launch_plan.items.size()) {
        return {
            .ok = false,
            .error = toolbox_invocation_admission_text(
                "Studio.ToolboxInvocationAdmission.Error.ConsistentItemMetadataRequired"),
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = request.launch_plan.selection_context,
            .toolbox_context = request.launch_plan.toolbox_context,
            .command_token = "studio.toolbox.palette.invoke",
            .asset_path = request.launch_plan.asset_path,
            .record_index = request.launch_plan.record_index,
            .object_name = request.launch_plan.object_name,
            .unique_id = request.launch_plan.unique_id,
            .item_count = request.launch_plan.item_count,
            .items = request.launch_plan.items,
            .palette_invocation_admitted = request.admit_palette_invocation,
            .dry_run = !request.admit_palette_invocation,
            .mutates_asset = false
        }
    };
}

StudioToolboxInvocationAdmissionCatalogResult plan_studio_toolbox_invocation_admission_catalog(
    const StudioToolboxInvocationAdmissionCatalogRequest& request) {
    auto items = studio_toolbox_items_for_context(request.toolbox_context);
    if (items.empty()) {
        return {
            .ok = false,
            .error = toolbox_invocation_admission_text(
                "Studio.ToolboxInvocationAdmission.Error.CatalogRequiresItemMetadata"),
            .selection_context = request.selection_context,
            .toolbox_context = request.toolbox_context,
            .command_token = {},
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = 0U,
            .items = {},
            .invocation_admission = {},
            .admission_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto invocation_admission = plan_studio_toolbox_invocation_admission({
        .launch_plan = {
            .selection_context = request.selection_context,
            .toolbox_context = request.toolbox_context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = items.size(),
            .items = items
        },
        .admit_palette_invocation = request.admit_palette_invocation
    });

    const std::size_t admission_count = invocation_admission.ok ? 1U : 0U;
    const std::size_t error_count = invocation_admission.ok ? 0U : 1U;

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = request.toolbox_context,
        .command_token = invocation_admission.ok ? invocation_admission.plan.command_token : std::string{},
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .item_count = items.size(),
        .items = std::move(items),
        .invocation_admission = std::move(invocation_admission),
        .admission_count = admission_count,
        .error_count = error_count,
        .dry_run = admission_count == 0U ? true : invocation_admission.plan.dry_run,
        .mutates_asset = admission_count == 0U ? false : invocation_admission.plan.mutates_asset
    };
}

StudioSelectionToolboxInvocationAdmissionCatalogResult
plan_studio_toolbox_invocation_admission_catalog_for_selection(
    const StudioSelectionToolboxInvocationAdmissionCatalogRequest& request) {
    auto launch_plan = plan_studio_toolbox_palette_launch({
        .selection_context = request.selection_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id
    });
    if (!launch_plan.ok) {
        return {
            .ok = false,
            .error = toolbox_invocation_admission_text(
                "Studio.ToolboxInvocationAdmission.Error.SelectionCatalogRequiresPalette"),
            .selection_context = request.selection_context,
            .toolbox_context = StudioToolboxContext::form,
            .command_token = {},
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = 0U,
            .items = {},
            .launch_plan = std::move(launch_plan),
            .invocation_admission = {},
            .admission_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto invocation_admission = plan_studio_toolbox_invocation_admission({
        .launch_plan = launch_plan.plan,
        .admit_palette_invocation = request.admit_palette_invocation
    });

    const std::size_t admission_count = invocation_admission.ok ? 1U : 0U;
    const std::size_t error_count = invocation_admission.ok ? 0U : 1U;
    const bool dry_run = admission_count == 0U ? true : invocation_admission.plan.dry_run;
    const bool mutates_asset = invocation_admission.ok ? invocation_admission.plan.mutates_asset : false;

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = launch_plan.plan.toolbox_context,
        .command_token = invocation_admission.ok ? invocation_admission.plan.command_token : std::string{},
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .item_count = launch_plan.plan.item_count,
        .items = launch_plan.plan.items,
        .launch_plan = std::move(launch_plan),
        .invocation_admission = std::move(invocation_admission),
        .admission_count = admission_count,
        .error_count = error_count,
        .dry_run = dry_run,
        .mutates_asset = mutates_asset
    };
}

}  // namespace copperfin::studio
