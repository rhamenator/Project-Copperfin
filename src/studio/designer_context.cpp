// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/designer_context.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

namespace copperfin::studio {

namespace {

std::vector<StudioBuilderDescriptor> builders_for_selection_context(StudioEditorSelectionContext context) {
    switch (context) {
        case StudioEditorSelectionContext::visual_object:
        {
            auto builders = studio_builders_for_context(StudioBuilderContext::form);
            auto control_builders = studio_builders_for_context(StudioBuilderContext::control);
            builders.insert(builders.end(), control_builders.begin(), control_builders.end());
            return builders;
        }
        case StudioEditorSelectionContext::visual_method:
        case StudioEditorSelectionContext::container_object:
            return studio_builders_for_context(StudioBuilderContext::control);
        case StudioEditorSelectionContext::class_designer:
            return studio_builders_for_context(StudioBuilderContext::class_designer);
        case StudioEditorSelectionContext::report_expression:
            return studio_builders_for_context(StudioBuilderContext::report);
        case StudioEditorSelectionContext::label_expression:
            return studio_builders_for_context(StudioBuilderContext::label);
        case StudioEditorSelectionContext::menu_item:
            return studio_builders_for_context(StudioBuilderContext::menu);
        case StudioEditorSelectionContext::project_item:
            return studio_builders_for_context(StudioBuilderContext::project);
        case StudioEditorSelectionContext::data_environment:
            return studio_builders_for_context(StudioBuilderContext::data_environment);
    }
    return {};
}

std::vector<StudioToolboxItemDescriptor> toolbox_items_for_selection_context(StudioEditorSelectionContext context) {
    switch (context) {
        case StudioEditorSelectionContext::visual_object:
        case StudioEditorSelectionContext::visual_method:
            return studio_toolbox_items_for_context(StudioToolboxContext::form);
        case StudioEditorSelectionContext::container_object:
            return studio_toolbox_items_for_context(StudioToolboxContext::container);
        case StudioEditorSelectionContext::class_designer:
            return studio_toolbox_items_for_context(StudioToolboxContext::class_designer);
        case StudioEditorSelectionContext::report_expression:
        case StudioEditorSelectionContext::label_expression:
            return studio_toolbox_items_for_context(StudioToolboxContext::report);
        case StudioEditorSelectionContext::menu_item:
            return {};
        case StudioEditorSelectionContext::project_item:
        case StudioEditorSelectionContext::data_environment:
            return {};
    }
    return {};
}

copperfin::localization::LocalizedCatalog designer_context_catalog() {
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

std::string designer_context_text(std::string_view key) {
    return designer_context_catalog().translate(key);
}

std::string builder_dispatch_execution_readiness_error(
    const StudioBuilderDispatchResult& dispatch,
    bool admit_execution) {
    if (!dispatch.ok) {
        return dispatch.error;
    }
    if (!admit_execution) {
        return designer_context_text("Studio.BuilderDispatch.CatalogEntry.Error.ExecutionAdmissionRequired");
    }
    const auto& dispatch_plan = dispatch.plan;
    if (dispatch_plan.builder.id.empty()) {
        return designer_context_text("Studio.BuilderDispatch.CatalogEntry.Error.ValidatedBuilderIdRequired");
    }
    if (dispatch_plan.command_token.empty()) {
        return designer_context_text("Studio.BuilderDispatch.CatalogEntry.Error.CommandTokenRequired");
    }
    if (dispatch_plan.entry_point.empty()) {
        return designer_context_text("Studio.BuilderDispatch.CatalogEntry.Error.EntryPointRequired");
    }
    if (!dispatch_plan.dispatch_admitted || dispatch_plan.dry_run) {
        return designer_context_text("Studio.BuilderDispatch.CatalogEntry.Error.AdmittedDispatchRequired");
    }
    if (dispatch_plan.executed) {
        return designer_context_text("Studio.BuilderDispatch.CatalogEntry.Error.NonExecutedDispatchRequired");
    }
    if (dispatch_plan.dispatch_arguments.empty()) {
        return designer_context_text("Studio.BuilderDispatch.CatalogEntry.Error.DispatchArgumentsRequired");
    }

    return {};
}

}  // namespace

StudioDesignerContextResult studio_designer_context_for_selection(const StudioDesignerContextRequest& request) {
    auto editor_actions = studio_editor_actions_for_context(request.selection_context);
    auto builders = builders_for_selection_context(request.selection_context);
    auto toolbox_items = toolbox_items_for_selection_context(request.selection_context);

    return {
        .selection_context = request.selection_context,
        .editor_action_count = editor_actions.size(),
        .builder_count = builders.size(),
        .toolbox_item_count = toolbox_items.size(),
        .editor_actions = std::move(editor_actions),
        .builders = std::move(builders),
        .toolbox_items = std::move(toolbox_items)
    };
}

StudioSelectionBuilderLaunchPlanResult plan_studio_builder_launch_for_selection(
    const StudioSelectionBuilderLaunchRequest& request) {
    if (request.builder_id.empty()) {
        return {
            .ok = false,
            .error = designer_context_text("Studio.SelectionBuilderLaunch.Error.BuilderIdRequired"),
            .selection_context = request.selection_context,
            .plan = {}
        };
    }

    const auto designer_context = studio_designer_context_for_selection({
        .selection_context = request.selection_context
    });
    const auto builder = std::find_if(
        designer_context.builders.begin(),
        designer_context.builders.end(),
        [&](const StudioBuilderDescriptor& candidate) {
            return candidate.id == request.builder_id;
        });

    if (builder == designer_context.builders.end()) {
        return {
            .ok = false,
            .error = designer_context_text("Studio.SelectionBuilderLaunch.Error.BuilderUnavailableForContext"),
            .selection_context = request.selection_context,
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .plan = {
            .builder = *builder,
            .context = builder->context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .entry_point = std::string(builder->entry_point)
        }
    };
}

StudioSelectionBuilderLaunchCatalogResult
plan_studio_builder_launch_catalog_for_selection(
    const StudioSelectionBuilderLaunchCatalogRequest& request) {
    const auto designer_context = studio_designer_context_for_selection({
        .selection_context = request.selection_context
    });
    if (designer_context.builders.empty()) {
        return {
            .ok = false,
            .error = designer_context_text("Studio.SelectionBuilderLaunch.Error.CatalogRequiresBuilder"),
            .selection_context = request.selection_context,
            .builder_count = 0U,
            .launch_plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioSelectionBuilderLaunchCatalogEntry> entries;
    entries.reserve(designer_context.builders.size());
    std::size_t launch_plan_count = 0U;
    std::size_t error_count = 0U;

    for (const auto& builder : designer_context.builders) {
        auto launch_plan = plan_studio_builder_launch_for_selection({
            .selection_context = request.selection_context,
            .builder_id = std::string(builder.id),
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id
        });

        if (launch_plan.ok) {
            ++launch_plan_count;
        } else {
            ++error_count;
        }

        entries.push_back({
            .builder = builder,
            .selection_context = request.selection_context,
            .launch_plan = std::move(launch_plan)
        });
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .builder_count = designer_context.builders.size(),
        .launch_plan_count = launch_plan_count,
        .error_count = error_count,
        .dry_run = true,
        .mutates_asset = false,
        .entries = std::move(entries)
    };
}

StudioSelectionBuilderInvocationAdmissionCatalogResult
plan_studio_builder_invocation_admission_catalog_for_selection(
    const StudioSelectionBuilderInvocationAdmissionCatalogRequest& request) {
    const auto designer_context = studio_designer_context_for_selection({
        .selection_context = request.selection_context
    });
    if (designer_context.builders.empty()) {
        return {
            .ok = false,
            .error = designer_context_text("Studio.SelectionBuilderInvocationAdmission.Error.CatalogRequiresBuilder"),
            .selection_context = request.selection_context,
            .builder_count = 0U,
            .admission_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioSelectionBuilderInvocationAdmissionCatalogEntry> entries;
    entries.reserve(designer_context.builders.size());
    std::size_t admission_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    for (const auto& builder : designer_context.builders) {
        auto launch_plan = plan_studio_builder_launch_for_selection({
            .selection_context = request.selection_context,
            .builder_id = std::string(builder.id),
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id
        });

        StudioBuilderInvocationAdmissionResult invocation_admission{};
        if (launch_plan.ok) {
            invocation_admission = plan_studio_builder_invocation_admission({
                .launch_plan = launch_plan.plan,
                .admit_ui_launch = request.admit_ui_launches
            });
        } else {
            invocation_admission = {
                .ok = false,
                .error = launch_plan.error,
                .plan = {}
            };
        }

        if (invocation_admission.ok) {
            ++admission_count;
            dry_run = dry_run && invocation_admission.plan.dry_run;
            mutates_asset = mutates_asset || invocation_admission.plan.mutates_asset;
        } else {
            ++error_count;
        }

        entries.push_back({
            .builder = builder,
            .selection_context = request.selection_context,
            .launch_plan = std::move(launch_plan),
            .invocation_admission = std::move(invocation_admission)
        });
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .builder_count = designer_context.builders.size(),
        .admission_count = admission_count,
        .error_count = error_count,
        .dry_run = admission_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

StudioSelectionBuilderDispatchCatalogResult
plan_studio_builder_dispatch_catalog_for_selection(
    const StudioSelectionBuilderDispatchCatalogRequest& request) {
    auto admission_catalog = plan_studio_builder_invocation_admission_catalog_for_selection({
        .selection_context = request.selection_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .admit_ui_launches = request.admit_ui_launches
    });
    if (!admission_catalog.ok) {
        return {
            .ok = false,
            .error = designer_context_text("Studio.SelectionBuilderDispatch.Error.DispatchCatalogRequiresBuilder"),
            .selection_context = request.selection_context,
            .builder_count = 0U,
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioSelectionBuilderDispatchCatalogEntry> entries;
    entries.reserve(admission_catalog.entries.size());
    std::size_t dispatch_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    for (auto& admission_entry : admission_catalog.entries) {
        StudioBuilderDispatchResult dispatch{};

        if (admission_entry.invocation_admission.ok) {
            dispatch = plan_studio_builder_dispatch({
                .admission_plan = admission_entry.invocation_admission.plan
            });
        } else {
            dispatch = {
                .ok = false,
                .error = admission_entry.invocation_admission.error,
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
            .builder = admission_entry.builder,
            .selection_context = request.selection_context,
            .launch_plan = std::move(admission_entry.launch_plan),
            .invocation_admission = std::move(admission_entry.invocation_admission),
            .dispatch = std::move(dispatch)
        });
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .builder_count = admission_catalog.builder_count,
        .dispatch_count = dispatch_count,
        .error_count = error_count,
        .dry_run = dispatch_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

StudioSelectionBuilderDispatchExecutionCatalogResult
plan_studio_builder_dispatch_execution_catalog_for_selection(
    const StudioSelectionBuilderDispatchExecutionCatalogRequest& request) {
    auto dispatch_catalog = plan_studio_builder_dispatch_catalog_for_selection({
        .selection_context = request.selection_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .admit_ui_launches = request.admit_ui_launches
    });
    if (!dispatch_catalog.ok) {
        return {
            .ok = false,
            .error = designer_context_text("Studio.SelectionBuilderDispatch.Error.ExecutionCatalogRequiresBuilder"),
            .selection_context = request.selection_context,
            .builder_count = 0U,
            .execution_ready_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioSelectionBuilderDispatchExecutionCatalogEntry> entries;
    entries.reserve(dispatch_catalog.entries.size());
    std::size_t execution_ready_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    for (auto& dispatch_entry : dispatch_catalog.entries) {
        auto execution_error = builder_dispatch_execution_readiness_error(
            dispatch_entry.dispatch, request.admit_execution);
        const bool execution_ready = execution_error.empty();

        if (execution_ready) {
            ++execution_ready_count;
            dry_run = dry_run && dispatch_entry.dispatch.plan.dry_run;
            mutates_asset = mutates_asset || dispatch_entry.dispatch.plan.mutates_asset;
        } else {
            ++error_count;
        }

        entries.push_back({
            .builder = dispatch_entry.builder,
            .selection_context = request.selection_context,
            .launch_plan = std::move(dispatch_entry.launch_plan),
            .invocation_admission = std::move(dispatch_entry.invocation_admission),
            .dispatch = std::move(dispatch_entry.dispatch),
            .execution_admitted = request.admit_execution,
            .execution_ready = execution_ready,
            .execution_error = std::move(execution_error)
        });
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .builder_count = dispatch_catalog.builder_count,
        .execution_ready_count = execution_ready_count,
        .error_count = error_count,
        .dry_run = execution_ready_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
