// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/studio/toolbox_dispatch.h"

#include "copperfin/localization/localization.h"

#include <string>
#include <string_view>
#include <mutex>
#include <utility>

namespace copperfin::studio {

namespace {

void append_argument(std::vector<std::string>& arguments, std::string key, std::string value) {
    arguments.push_back(std::move(key));
    arguments.push_back(std::move(value));
}

copperfin::localization::LocalizedCatalog toolbox_dispatch_catalog() {
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

std::string toolbox_dispatch_text(std::string_view key) {
    return toolbox_dispatch_catalog().translate(key);
}

std::string toolbox_dispatch_execution_readiness_error(
    const StudioToolboxDispatchResult& dispatch,
    bool admit_execution) {
    if (!dispatch.ok) {
        return dispatch.error;
    }
    if (!admit_execution) {
        return toolbox_dispatch_text("Studio.ToolboxDispatch.CatalogEntry.Error.ExecutionAdmissionRequired");
    }
    const auto& dispatch_plan = dispatch.plan;
    if (dispatch_plan.command_token.empty()) {
        return toolbox_dispatch_text("Studio.ToolboxDispatch.CatalogEntry.Error.CommandTokenRequired");
    }
    if (!dispatch_plan.dispatch_admitted || dispatch_plan.dry_run) {
        return toolbox_dispatch_text("Studio.ToolboxDispatch.CatalogEntry.Error.AdmittedDispatchRequired");
    }
    if (dispatch_plan.executed) {
        return toolbox_dispatch_text("Studio.ToolboxDispatch.CatalogEntry.Error.NonExecutedDispatchRequired");
    }
    if (dispatch_plan.items.empty() || dispatch_plan.item_count == 0U) {
        return toolbox_dispatch_text("Studio.ToolboxDispatch.CatalogEntry.Error.ValidatedItemMetadataRequired");
    }
    if (dispatch_plan.item_count != dispatch_plan.items.size()) {
        return toolbox_dispatch_text("Studio.ToolboxDispatch.CatalogEntry.Error.ConsistentItemMetadataRequired");
    }
    if (dispatch_plan.dispatch_arguments.empty()) {
        return toolbox_dispatch_text("Studio.ToolboxDispatch.CatalogEntry.Error.DispatchArgumentsRequired");
    }

    return {};
}

}  // namespace

StudioToolboxDispatchResult plan_studio_toolbox_dispatch(
    const StudioToolboxDispatchRequest& request) {
    if (request.admission_plan.command_token.empty()) {
        return {
            .ok = false,
            .error = toolbox_dispatch_text("Studio.ToolboxDispatch.Error.CommandTokenRequired"),
            .plan = {}
        };
    }
    if (!request.admission_plan.palette_invocation_admitted || request.admission_plan.dry_run) {
        return {
            .ok = false,
            .error = toolbox_dispatch_text("Studio.ToolboxDispatch.Error.AdmittedInvocationRequired"),
            .plan = {}
        };
    }
    if (request.admission_plan.items.empty() || request.admission_plan.item_count == 0U) {
        return {
            .ok = false,
            .error = toolbox_dispatch_text("Studio.ToolboxDispatch.Error.ValidatedItemMetadataRequired"),
            .plan = {}
        };
    }
    if (request.admission_plan.item_count != request.admission_plan.items.size()) {
        return {
            .ok = false,
            .error = toolbox_dispatch_text("Studio.ToolboxDispatch.Error.ConsistentItemMetadataRequired"),
            .plan = {}
        };
    }

    std::vector<std::string> arguments;
    append_argument(arguments, "--command-token", request.admission_plan.command_token);
    append_argument(arguments, "--selection-context",
        studio_editor_selection_context_name(request.admission_plan.selection_context));
    append_argument(arguments, "--toolbox-context", studio_toolbox_context_name(request.admission_plan.toolbox_context));
    append_argument(arguments, "--path", request.admission_plan.asset_path);
    append_argument(arguments, "--record", std::to_string(request.admission_plan.record_index));
    append_argument(arguments, "--object-name", request.admission_plan.object_name);
    append_argument(arguments, "--unique-id", request.admission_plan.unique_id);
    append_argument(arguments, "--item-count", std::to_string(request.admission_plan.item_count));

    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = request.admission_plan.selection_context,
            .toolbox_context = request.admission_plan.toolbox_context,
            .command_token = request.admission_plan.command_token,
            .asset_path = request.admission_plan.asset_path,
            .record_index = request.admission_plan.record_index,
            .object_name = request.admission_plan.object_name,
            .unique_id = request.admission_plan.unique_id,
            .item_count = request.admission_plan.item_count,
            .items = request.admission_plan.items,
            .dispatch_arguments = std::move(arguments),
            .dispatch_admitted = true,
            .dry_run = false,
            .executed = false,
            .mutates_asset = false
        }
    };
}

StudioToolboxDispatchCatalogResult plan_studio_toolbox_dispatch_catalog(
    const StudioToolboxDispatchCatalogRequest& request) {
    auto invocation_catalog = plan_studio_toolbox_invocation_admission_catalog({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .toolbox_context = request.toolbox_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .admit_palette_invocation = request.admit_palette_invocation
    });
    if (!invocation_catalog.ok) {
        return {
            .ok = false,
            .error = invocation_catalog.error,
            .toolbox_context = request.toolbox_context,
            .command_token = {},
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = 0U,
            .items = {},
            .invocation_admission = {},
            .dispatch = {},
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    StudioToolboxDispatchResult dispatch{};
    if (invocation_catalog.invocation_admission.ok) {
        dispatch = plan_studio_toolbox_dispatch({
            .admission_plan = invocation_catalog.invocation_admission.plan
        });
    } else {
        dispatch = {
            .ok = false,
            .error = invocation_catalog.invocation_admission.error,
            .plan = {}
        };
    }

    const std::size_t dispatch_count = dispatch.ok ? 1U : 0U;
    const std::size_t error_count = dispatch.ok ? 0U : 1U;
    const bool dry_run = dispatch_count == 0U ? true : dispatch.plan.dry_run;
    const bool mutates_asset = dispatch.ok ? dispatch.plan.mutates_asset : false;

    return {
        .ok = true,
        .error = {},
        .toolbox_context = request.toolbox_context,
        .command_token = invocation_catalog.command_token,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .item_count = invocation_catalog.item_count,
        .items = std::move(invocation_catalog.items),
        .invocation_admission = std::move(invocation_catalog.invocation_admission),
        .dispatch = std::move(dispatch),
        .dispatch_count = dispatch_count,
        .error_count = error_count,
        .dry_run = dry_run,
        .mutates_asset = mutates_asset
    };
}

StudioSelectionToolboxDispatchCatalogResult plan_studio_toolbox_dispatch_catalog_for_selection(
    const StudioSelectionToolboxDispatchCatalogRequest& request) {
    auto invocation_catalog = plan_studio_toolbox_invocation_admission_catalog_for_selection({
        .selection_context = request.selection_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .admit_palette_invocation = request.admit_palette_invocation
    });
    if (!invocation_catalog.ok) {
        return {
            .ok = false,
            .error = toolbox_dispatch_text("Studio.ToolboxDispatch.Error.SelectionCatalogRequiresPalette"),
            .selection_context = request.selection_context,
            .toolbox_context = StudioToolboxContext::form,
            .command_token = {},
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = 0U,
            .items = {},
            .launch_plan = std::move(invocation_catalog.launch_plan),
            .invocation_admission = {},
            .dispatch = {},
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    StudioToolboxDispatchResult dispatch{};
    if (invocation_catalog.invocation_admission.ok) {
        dispatch = plan_studio_toolbox_dispatch({
            .admission_plan = invocation_catalog.invocation_admission.plan
        });
    } else {
        dispatch = {
            .ok = false,
            .error = invocation_catalog.invocation_admission.error,
            .plan = {}
        };
    }

    const std::size_t dispatch_count = dispatch.ok ? 1U : 0U;
    const std::size_t error_count = dispatch.ok ? 0U : 1U;
    const bool dry_run = dispatch_count == 0U ? true : dispatch.plan.dry_run;
    const bool mutates_asset = dispatch.ok ? dispatch.plan.mutates_asset : false;

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = invocation_catalog.toolbox_context,
        .command_token = invocation_catalog.command_token,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .item_count = invocation_catalog.item_count,
        .items = std::move(invocation_catalog.items),
        .launch_plan = std::move(invocation_catalog.launch_plan),
        .invocation_admission = std::move(invocation_catalog.invocation_admission),
        .dispatch = std::move(dispatch),
        .dispatch_count = dispatch_count,
        .error_count = error_count,
        .dry_run = dry_run,
        .mutates_asset = mutates_asset
    };
}

StudioToolboxDispatchExecutionResult execute_studio_toolbox_dispatch(
    const StudioToolboxDispatchExecutionRequest& request) {
    auto failed = [&](std::string error) {
        return StudioToolboxDispatchExecutionResult{
            .ok = false,
            .error = std::move(error),
            .dispatch_plan = {},
            .observation = {},
            .execution_admitted = request.admit_execution,
            .executed = false,
            .dry_run = true,
            .mutates_asset = false
        };
    };

    const auto& dispatch_plan = request.dispatch_plan;
    if (!request.admit_execution) {
        return failed(toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.ExecutionAdmissionRequired"));
    }
    if (!request.executor) {
        return failed(toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.ExecutorRequired"));
    }
    if (dispatch_plan.command_token.empty()) {
        return failed(toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.CommandTokenRequired"));
    }
    if (!dispatch_plan.dispatch_admitted || dispatch_plan.dry_run) {
        return failed(toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.AdmittedDispatchRequired"));
    }
    if (dispatch_plan.executed) {
        return failed(toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.NonExecutedDispatchRequired"));
    }
    if (dispatch_plan.items.empty() || dispatch_plan.item_count == 0U) {
        return failed(toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.ValidatedItemMetadataRequired"));
    }
    if (dispatch_plan.item_count != dispatch_plan.items.size()) {
        return failed(toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.ConsistentItemMetadataRequired"));
    }
    if (dispatch_plan.dispatch_arguments.empty()) {
        return failed(toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.DispatchArgumentsRequired"));
    }

    auto observation = request.executor(dispatch_plan);
    if (!observation.launched) {
        return {
            .ok = false,
            .error = observation.error.empty()
                ? toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.ExecutorDidNotLaunch")
                : observation.error,
            .dispatch_plan = {},
            .observation = std::move(observation),
            .execution_admitted = true,
            .executed = false,
            .dry_run = true,
            .mutates_asset = false
        };
    }
    if (observation.exit_code != 0) {
        return {
            .ok = false,
            .error = observation.error.empty()
                ? toolbox_dispatch_text("Studio.ToolboxDispatch.Execution.Error.ExecutorNonZeroExit")
                : observation.error,
            .dispatch_plan = {},
            .observation = std::move(observation),
            .execution_admitted = true,
            .executed = false,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto executed_plan = dispatch_plan;
    executed_plan.executed = true;
    const bool mutates_asset = observation.mutates_asset;
    return {
        .ok = true,
        .error = {},
        .dispatch_plan = std::move(executed_plan),
        .observation = std::move(observation),
        .execution_admitted = true,
        .executed = true,
        .dry_run = false,
        .mutates_asset = mutates_asset
    };
}

StudioToolboxDispatchExecutionCatalogResult plan_studio_toolbox_dispatch_execution_catalog(
    const StudioToolboxDispatchExecutionCatalogRequest& request) {
    auto dispatch_catalog = plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = request.toolbox_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .admit_palette_invocation = request.admit_palette_invocation
    });
    if (!dispatch_catalog.ok) {
        return {
            .ok = false,
            .error = dispatch_catalog.error,
            .toolbox_context = request.toolbox_context,
            .command_token = {},
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = 0U,
            .items = {},
            .invocation_admission = {},
            .dispatch = {},
            .execution_ready_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    const auto execution_error = toolbox_dispatch_execution_readiness_error(
        dispatch_catalog.dispatch, request.admit_execution);
    const bool execution_ready = execution_error.empty();
    std::vector<StudioToolboxDispatchExecutionCatalogEntry> entries;
    entries.reserve(dispatch_catalog.items.size());
    for (const auto& item : dispatch_catalog.items) {
        entries.push_back({
            .item = item,
            .execution_admitted = request.admit_execution,
            .execution_ready = execution_ready,
            .execution_error = execution_error
        });
    }

    return {
        .ok = true,
        .error = {},
        .toolbox_context = request.toolbox_context,
        .command_token = dispatch_catalog.command_token,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .item_count = dispatch_catalog.item_count,
        .items = std::move(dispatch_catalog.items),
        .invocation_admission = std::move(dispatch_catalog.invocation_admission),
        .dispatch = std::move(dispatch_catalog.dispatch),
        .execution_ready_count = execution_ready ? dispatch_catalog.item_count : 0U,
        .error_count = execution_ready ? 0U : dispatch_catalog.item_count,
        .dry_run = execution_ready ? dispatch_catalog.dispatch.plan.dry_run : true,
        .mutates_asset = execution_ready ? dispatch_catalog.dispatch.plan.mutates_asset : false,
        .entries = std::move(entries)
    };
}

StudioSelectionToolboxDispatchExecutionCatalogResult
plan_studio_toolbox_dispatch_execution_catalog_for_selection(
    const StudioSelectionToolboxDispatchExecutionCatalogRequest& request) {
    auto dispatch_catalog = plan_studio_toolbox_dispatch_catalog_for_selection({
        .selection_context = request.selection_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .admit_palette_invocation = request.admit_palette_invocation
    });
    if (!dispatch_catalog.ok) {
        return {
            .ok = false,
            .error = dispatch_catalog.error,
            .selection_context = request.selection_context,
            .toolbox_context = dispatch_catalog.toolbox_context,
            .command_token = {},
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = 0U,
            .items = {},
            .launch_plan = std::move(dispatch_catalog.launch_plan),
            .invocation_admission = {},
            .dispatch = {},
            .execution_ready_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    const auto execution_error = toolbox_dispatch_execution_readiness_error(
        dispatch_catalog.dispatch, request.admit_execution);
    const bool execution_ready = execution_error.empty();
    std::vector<StudioToolboxDispatchExecutionCatalogEntry> entries;
    entries.reserve(dispatch_catalog.items.size());
    for (const auto& item : dispatch_catalog.items) {
        entries.push_back({
            .item = item,
            .execution_admitted = request.admit_execution,
            .execution_ready = execution_ready,
            .execution_error = execution_error
        });
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = dispatch_catalog.toolbox_context,
        .command_token = dispatch_catalog.command_token,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .item_count = dispatch_catalog.item_count,
        .items = std::move(dispatch_catalog.items),
        .launch_plan = std::move(dispatch_catalog.launch_plan),
        .invocation_admission = std::move(dispatch_catalog.invocation_admission),
        .dispatch = std::move(dispatch_catalog.dispatch),
        .execution_ready_count = execution_ready ? dispatch_catalog.item_count : 0U,
        .error_count = execution_ready ? 0U : dispatch_catalog.item_count,
        .dry_run = execution_ready ? dispatch_catalog.dispatch.plan.dry_run : true,
        .mutates_asset = execution_ready ? dispatch_catalog.dispatch.plan.mutates_asset : false,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
