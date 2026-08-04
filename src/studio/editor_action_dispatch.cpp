// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/editor_action_dispatch.h"

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

copperfin::localization::LocalizedCatalog editor_action_dispatch_catalog() {
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

std::string editor_action_dispatch_text(std::string_view key) {
    return editor_action_dispatch_catalog().translate(key);
}

std::string editor_action_dispatch_execution_readiness_error(
    const StudioEditorActionDispatchResult& dispatch,
    bool admit_execution) {
    if (!dispatch.ok) {
        return dispatch.error;
    }
    if (!admit_execution) {
        return editor_action_dispatch_text(
            "Studio.EditorActionDispatch.CatalogEntry.Error.ExecutionAdmissionRequired");
    }
    const auto& dispatch_plan = dispatch.plan;
    if (dispatch_plan.action.id.empty()) {
        return editor_action_dispatch_text(
            "Studio.EditorActionDispatch.CatalogEntry.Error.ValidatedActionIdRequired");
    }
    if (dispatch_plan.command_token.empty()) {
        return editor_action_dispatch_text(
            "Studio.EditorActionDispatch.CatalogEntry.Error.CommandTokenRequired");
    }
    if (!dispatch_plan.dispatch_admitted || dispatch_plan.dry_run) {
        return editor_action_dispatch_text(
            "Studio.EditorActionDispatch.CatalogEntry.Error.AdmittedDispatchRequired");
    }
    if (dispatch_plan.executed) {
        return editor_action_dispatch_text(
            "Studio.EditorActionDispatch.CatalogEntry.Error.NonExecutedDispatchRequired");
    }
    if (dispatch_plan.dispatch_arguments.empty()) {
        return editor_action_dispatch_text(
            "Studio.EditorActionDispatch.CatalogEntry.Error.DispatchArgumentsRequired");
    }

    return {};
}

}  // namespace

StudioEditorActionDispatchResult plan_studio_editor_action_dispatch(
    const StudioEditorActionDispatchRequest& request) {
    if (request.admission_plan.action.id.empty()) {
        return {
            .ok = false,
            .error = editor_action_dispatch_text(
                "Studio.EditorActionDispatch.Error.ValidatedActionIdRequired"),
            .plan = {}
        };
    }
    if (request.admission_plan.command_token.empty()) {
        return {
            .ok = false,
            .error = editor_action_dispatch_text(
                "Studio.EditorActionDispatch.Error.CommandTokenRequired"),
            .plan = {}
        };
    }
    if (!request.admission_plan.editor_invocation_admitted || request.admission_plan.dry_run) {
        return {
            .ok = false,
            .error = editor_action_dispatch_text(
                "Studio.EditorActionDispatch.Error.AdmittedInvocationRequired"),
            .plan = {}
        };
    }

    std::vector<std::string> arguments;
    append_argument(arguments, "--command-token", request.admission_plan.command_token);
    append_argument(arguments, "--action-id", std::string(request.admission_plan.action.id));
    append_argument(arguments, "--selection-context",
        studio_editor_selection_context_name(request.admission_plan.selection_context));
    append_argument(arguments, "--target-surface", request.admission_plan.target_surface);
    append_argument(arguments, "--path", request.admission_plan.asset_path);
    append_argument(arguments, "--record", std::to_string(request.admission_plan.record_index));
    append_argument(arguments, "--object-name", request.admission_plan.object_name);
    append_argument(arguments, "--unique-id", request.admission_plan.unique_id);
    append_argument(arguments, "--symbol", request.admission_plan.symbol);
    append_argument(arguments, "--line", std::to_string(request.admission_plan.line));
    append_argument(arguments, "--column", std::to_string(request.admission_plan.column));

    return {
        .ok = true,
        .error = {},
        .plan = {
            .action = request.admission_plan.action,
            .selection_context = request.admission_plan.selection_context,
            .command_token = request.admission_plan.command_token,
            .target_surface = request.admission_plan.target_surface,
            .asset_path = request.admission_plan.asset_path,
            .record_index = request.admission_plan.record_index,
            .object_name = request.admission_plan.object_name,
            .unique_id = request.admission_plan.unique_id,
            .symbol = request.admission_plan.symbol,
            .line = request.admission_plan.line,
            .column = request.admission_plan.column,
            .dispatch_arguments = std::move(arguments),
            .dispatch_admitted = true,
            .dry_run = false,
            .executed = false,
            .mutates_asset = false
        }
    };
}

StudioEditorActionDispatchCatalogResult plan_studio_editor_action_dispatch_catalog(
    const StudioEditorActionDispatchCatalogRequest& request) {
    auto admission_catalog = plan_studio_editor_action_invocation_admission_catalog({
        .selection_context = request.selection_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .symbol = request.symbol,
        .line = request.line,
        .column = request.column,
        .admit_editor_invocations = request.admit_editor_invocations
    });
    if (!admission_catalog.ok) {
        return {
            .ok = false,
            .error = editor_action_dispatch_text(
                "Studio.EditorActionDispatch.Error.DispatchCatalogRequiresAction"),
            .selection_context = request.selection_context,
            .action_count = 0U,
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioEditorActionDispatchCatalogEntry> entries;
    entries.reserve(admission_catalog.entries.size());
    std::size_t dispatch_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    for (auto& admission_entry : admission_catalog.entries) {
        StudioEditorActionDispatchResult dispatch{};

        if (admission_entry.invocation_admission.ok) {
            dispatch = plan_studio_editor_action_dispatch({
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
            .action = admission_entry.action,
            .launch_plan = std::move(admission_entry.launch_plan),
            .invocation_admission = std::move(admission_entry.invocation_admission),
            .dispatch = std::move(dispatch)
        });
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .action_count = admission_catalog.action_count,
        .dispatch_count = dispatch_count,
        .error_count = error_count,
        .dry_run = dispatch_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

StudioEditorActionDispatchExecutionResult execute_studio_editor_action_dispatch(
    const StudioEditorActionDispatchExecutionRequest& request) {
    auto failed = [&](std::string error) {
        return StudioEditorActionDispatchExecutionResult{
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
        return failed(editor_action_dispatch_text(
            "Studio.EditorActionDispatch.Execution.Error.ExecutionAdmissionRequired"));
    }
    if (!request.executor) {
        return failed(editor_action_dispatch_text(
            "Studio.EditorActionDispatch.Execution.Error.ExecutorRequired"));
    }
    if (dispatch_plan.action.id.empty()) {
        return failed(editor_action_dispatch_text(
            "Studio.EditorActionDispatch.Execution.Error.ValidatedActionIdRequired"));
    }
    if (dispatch_plan.command_token.empty()) {
        return failed(editor_action_dispatch_text(
            "Studio.EditorActionDispatch.Execution.Error.CommandTokenRequired"));
    }
    if (!dispatch_plan.dispatch_admitted || dispatch_plan.dry_run) {
        return failed(editor_action_dispatch_text(
            "Studio.EditorActionDispatch.Execution.Error.AdmittedDispatchRequired"));
    }
    if (dispatch_plan.executed) {
        return failed(editor_action_dispatch_text(
            "Studio.EditorActionDispatch.Execution.Error.NonExecutedDispatchRequired"));
    }
    if (dispatch_plan.dispatch_arguments.empty()) {
        return failed(editor_action_dispatch_text(
            "Studio.EditorActionDispatch.Execution.Error.DispatchArgumentsRequired"));
    }

    auto observation = request.executor(dispatch_plan);
    if (!observation.launched) {
        return {
            .ok = false,
            .error = observation.error.empty()
                ? editor_action_dispatch_text(
                    "Studio.EditorActionDispatch.Execution.Error.ExecutorDidNotLaunch")
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
                ? editor_action_dispatch_text(
                    "Studio.EditorActionDispatch.Execution.Error.ExecutorNonZeroExit")
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

StudioEditorActionDispatchExecutionCatalogResult plan_studio_editor_action_dispatch_execution_catalog(
    const StudioEditorActionDispatchExecutionCatalogRequest& request) {
    auto dispatch_catalog = plan_studio_editor_action_dispatch_catalog({
        .selection_context = request.selection_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .symbol = request.symbol,
        .line = request.line,
        .column = request.column,
        .admit_editor_invocations = request.admit_editor_invocations
    });
    if (!dispatch_catalog.ok) {
        return {
            .ok = false,
            .error = editor_action_dispatch_text(
                "Studio.EditorActionDispatch.Error.ExecutionCatalogRequiresAction"),
            .selection_context = request.selection_context,
            .action_count = 0U,
            .execution_ready_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioEditorActionDispatchExecutionCatalogEntry> entries;
    entries.reserve(dispatch_catalog.entries.size());
    std::size_t execution_ready_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    for (auto& dispatch_entry : dispatch_catalog.entries) {
        auto execution_error = editor_action_dispatch_execution_readiness_error(
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
            .action = dispatch_entry.action,
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
        .action_count = dispatch_catalog.action_count,
        .execution_ready_count = execution_ready_count,
        .error_count = error_count,
        .dry_run = execution_ready_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
