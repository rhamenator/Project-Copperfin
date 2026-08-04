// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/studio/designer_invocation_admission.h"

#include "copperfin/localization/localization.h"

#include <mutex>
#include <string_view>
#include <utility>

namespace copperfin::studio {

namespace {

copperfin::localization::LocalizedCatalog designer_invocation_catalog() {
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

std::string designer_invocation_text(std::string_view key) {
    return designer_invocation_catalog().translate(key);
}

}  // namespace

StudioDesignerInvocationAdmissionResult plan_studio_designer_invocation_admission(
    const StudioDesignerInvocationAdmissionRequest& request) {
    std::vector<StudioEditorActionInvocationAdmissionResult> editor_action_invocations;
    editor_action_invocations.reserve(request.launch_surface_plan.editor_action_launch_plans.size());
    for (const auto& launch_result : request.launch_surface_plan.editor_action_launch_plans) {
        if (!launch_result.ok) {
            editor_action_invocations.push_back({
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            });
            continue;
        }
        editor_action_invocations.push_back(plan_studio_editor_action_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_editor_invocation = request.admit_editor_invocations
        }));
    }

    std::vector<StudioBuilderInvocationAdmissionResult> builder_invocations;
    builder_invocations.reserve(request.launch_surface_plan.builder_launch_plans.size());
    for (const auto& launch_result : request.launch_surface_plan.builder_launch_plans) {
        if (!launch_result.ok) {
            builder_invocations.push_back({
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            });
            continue;
        }
        builder_invocations.push_back(plan_studio_builder_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_ui_launch = request.admit_builder_invocations
        }));
    }

    StudioToolboxInvocationAdmissionResult toolbox_invocation{};
    if (request.launch_surface_plan.toolbox_palette_launch_plan.ok) {
        toolbox_invocation = plan_studio_toolbox_invocation_admission({
            .launch_plan = request.launch_surface_plan.toolbox_palette_launch_plan.plan,
            .admit_palette_invocation = request.admit_toolbox_invocation
        });
    } else {
        toolbox_invocation = {
            .ok = false,
            .error = request.launch_surface_plan.toolbox_palette_launch_plan.error,
            .plan = {}
        };
    }

    std::size_t valid_surface_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;
    for (const auto& admission : editor_action_invocations) {
        if (admission.ok) {
            ++valid_surface_count;
            dry_run = dry_run && admission.plan.dry_run;
            mutates_asset = mutates_asset || admission.plan.mutates_asset;
        }
    }
    for (const auto& admission : builder_invocations) {
        if (admission.ok) {
            ++valid_surface_count;
            dry_run = dry_run && admission.plan.dry_run;
            mutates_asset = mutates_asset || admission.plan.mutates_asset;
        }
    }
    if (toolbox_invocation.ok) {
        ++valid_surface_count;
        dry_run = dry_run && toolbox_invocation.plan.dry_run;
        mutates_asset = mutates_asset || toolbox_invocation.plan.mutates_asset;
    }

    if (valid_surface_count == 0U) {
        return {
            .ok = false,
            .error = designer_invocation_text("Studio.DesignerInvocationAdmission.Error.ValidatedLaunchSurfaceRequired"),
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = request.launch_surface_plan.selection_context,
            .asset_path = request.launch_surface_plan.asset_path,
            .record_index = request.launch_surface_plan.record_index,
            .object_name = request.launch_surface_plan.object_name,
            .unique_id = request.launch_surface_plan.unique_id,
            .symbol = request.launch_surface_plan.symbol,
            .line = request.launch_surface_plan.line,
            .column = request.launch_surface_plan.column,
            .editor_action_invocation_count = editor_action_invocations.size(),
            .builder_invocation_count = builder_invocations.size(),
            .toolbox_available = toolbox_invocation.ok,
            .toolbox_item_count = toolbox_invocation.ok ? toolbox_invocation.plan.item_count : 0U,
            .toolbox_error = toolbox_invocation.ok ? std::string{} : toolbox_invocation.error,
            .editor_action_invocations = std::move(editor_action_invocations),
            .builder_invocations = std::move(builder_invocations),
            .toolbox_invocation = std::move(toolbox_invocation),
            .dry_run = dry_run,
            .mutates_asset = mutates_asset
        }
    };
}

StudioDesignerInvocationAdmissionCatalogResult plan_studio_designer_invocation_admission_catalog(
    const StudioDesignerInvocationAdmissionCatalogRequest& request) {
    const auto launch_surface_catalog = plan_studio_designer_launch_surface_catalog({
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .symbol = request.symbol,
        .line = request.line,
        .column = request.column
    });
    if (!launch_surface_catalog.ok) {
        return {
            .ok = false,
            .error = launch_surface_catalog.error,
            .context_count = 0U,
            .contexts = {}
        };
    }

    std::vector<StudioDesignerInvocationAdmissionCatalogEntry> entries;
    entries.reserve(launch_surface_catalog.contexts.size());
    for (const auto& context : launch_surface_catalog.contexts) {
        auto invocation_admission = plan_studio_designer_invocation_admission({
            .launch_surface_plan = context.launch_surface_plan.plan,
            .admit_editor_invocations = request.admit_editor_invocations,
            .admit_builder_invocations = request.admit_builder_invocations,
            .admit_toolbox_invocation = request.admit_toolbox_invocation
        });
        const auto& plan = invocation_admission.plan;
        entries.push_back({
            .selection_context = context.selection_context,
            .editor_action_invocation_count = invocation_admission.ok ? plan.editor_action_invocation_count : 0U,
            .builder_invocation_count = invocation_admission.ok ? plan.builder_invocation_count : 0U,
            .toolbox_available = invocation_admission.ok && plan.toolbox_available,
            .toolbox_item_count = invocation_admission.ok ? plan.toolbox_item_count : 0U,
            .toolbox_error = invocation_admission.ok ? plan.toolbox_error : invocation_admission.error,
            .dry_run = invocation_admission.ok ? plan.dry_run : true,
            .mutates_asset = invocation_admission.ok ? plan.mutates_asset : false,
            .invocation_admission = std::move(invocation_admission)
        });
    }

    const auto context_count = entries.size();
    return {
        .ok = true,
        .error = {},
        .context_count = context_count,
        .contexts = std::move(entries)
    };
}

}  // namespace copperfin::studio
