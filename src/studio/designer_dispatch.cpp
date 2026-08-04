// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/designer_dispatch.h"

#include "copperfin/localization/localization.h"

#include <string>
#include <string_view>
#include <mutex>
#include <utility>

namespace copperfin::studio {

namespace {

copperfin::localization::LocalizedCatalog designer_dispatch_catalog() {
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

std::string designer_dispatch_text(std::string_view key) {
    return designer_dispatch_catalog().translate(key);
}

std::string designer_dispatch_execution_readiness_error(
    const StudioDesignerDispatchResult& dispatch,
    bool admit_execution) {
    if (!dispatch.ok) {
        return dispatch.error;
    }
    if (!admit_execution) {
        return designer_dispatch_text("Studio.DesignerDispatch.CatalogEntry.Error.ExecutionAdmissionRequired");
    }

    const auto& dispatch_plan = dispatch.plan;
    if (dispatch_plan.dispatch_count == 0U) {
        return designer_dispatch_text("Studio.DesignerDispatch.CatalogEntry.Error.AdmittedDispatchRequired");
    }
    if (dispatch_plan.error_count != 0U) {
        return designer_dispatch_text("Studio.DesignerDispatch.CatalogEntry.Error.ErrorFreeDispatchRequired");
    }
    if (dispatch_plan.dry_run) {
        return designer_dispatch_text("Studio.DesignerDispatch.CatalogEntry.Error.NonDryRunDispatchRequired");
    }

    for (const auto& editor_dispatch : dispatch_plan.editor_action_dispatches) {
        if (editor_dispatch.ok && editor_dispatch.plan.executed) {
            return designer_dispatch_text("Studio.DesignerDispatch.CatalogEntry.Error.NonExecutedEditorDispatchesRequired");
        }
    }
    for (const auto& builder_dispatch : dispatch_plan.builder_dispatches) {
        if (builder_dispatch.ok && builder_dispatch.plan.executed) {
            return designer_dispatch_text("Studio.DesignerDispatch.CatalogEntry.Error.NonExecutedBuilderDispatchesRequired");
        }
    }
    if (dispatch_plan.toolbox_dispatch.ok && dispatch_plan.toolbox_dispatch.plan.executed) {
        return designer_dispatch_text("Studio.DesignerDispatch.CatalogEntry.Error.NonExecutedToolboxDispatchRequired");
    }

    return {};
}

}  // namespace

StudioDesignerDispatchResult plan_studio_designer_dispatch(
    const StudioDesignerDispatchRequest& request) {
    const auto& admission_plan = request.invocation_admission_plan;

    std::vector<StudioEditorActionDispatchResult> editor_dispatches;
    editor_dispatches.reserve(admission_plan.editor_action_invocations.size());
    for (const auto& admission : admission_plan.editor_action_invocations) {
        if (!admission.ok) {
            editor_dispatches.push_back({
                .ok = false,
                .error = admission.error,
                .plan = {}
            });
            continue;
        }
        editor_dispatches.push_back(plan_studio_editor_action_dispatch({
            .admission_plan = admission.plan
        }));
    }

    std::vector<StudioBuilderDispatchResult> builder_dispatches;
    builder_dispatches.reserve(admission_plan.builder_invocations.size());
    for (const auto& admission : admission_plan.builder_invocations) {
        if (!admission.ok) {
            builder_dispatches.push_back({
                .ok = false,
                .error = admission.error,
                .plan = {}
            });
            continue;
        }
        builder_dispatches.push_back(plan_studio_builder_dispatch({
            .admission_plan = admission.plan
        }));
    }

    StudioToolboxDispatchResult toolbox_dispatch{};
    if (admission_plan.toolbox_invocation.ok) {
        toolbox_dispatch = plan_studio_toolbox_dispatch({
            .admission_plan = admission_plan.toolbox_invocation.plan
        });
    } else {
        toolbox_dispatch = {
            .ok = false,
            .error = admission_plan.toolbox_invocation.error,
            .plan = {}
        };
    }

    const std::size_t input_surface_count = admission_plan.editor_action_invocations.size() +
                                            admission_plan.builder_invocations.size() +
                                            (admission_plan.toolbox_invocation.ok ? 1U : 0U);
    if (input_surface_count == 0U) {
        return {
            .ok = false,
            .error = designer_dispatch_text("Studio.DesignerDispatch.Error.InvocationAdmissionRequired"),
            .plan = {}
        };
    }

    std::size_t dispatch_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    auto summarize_dispatch = [&](const auto& dispatch) {
        if (dispatch.ok) {
            ++dispatch_count;
            dry_run = dry_run && dispatch.plan.dry_run;
            mutates_asset = mutates_asset || dispatch.plan.mutates_asset;
        } else {
            ++error_count;
        }
    };
    for (const auto& dispatch : editor_dispatches) {
        summarize_dispatch(dispatch);
    }
    for (const auto& dispatch : builder_dispatches) {
        summarize_dispatch(dispatch);
    }
    summarize_dispatch(toolbox_dispatch);

    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = admission_plan.selection_context,
            .asset_path = admission_plan.asset_path,
            .record_index = admission_plan.record_index,
            .object_name = admission_plan.object_name,
            .unique_id = admission_plan.unique_id,
            .symbol = admission_plan.symbol,
            .line = admission_plan.line,
            .column = admission_plan.column,
            .editor_action_dispatch_count = editor_dispatches.size(),
            .builder_dispatch_count = builder_dispatches.size(),
            .toolbox_dispatch_count = toolbox_dispatch.ok ? 1U : 0U,
            .dispatch_count = dispatch_count,
            .error_count = error_count,
            .editor_action_dispatches = std::move(editor_dispatches),
            .builder_dispatches = std::move(builder_dispatches),
            .toolbox_dispatch = std::move(toolbox_dispatch),
            .dry_run = dry_run,
            .mutates_asset = mutates_asset
        }
    };
}

StudioDesignerDispatchCatalogResult plan_studio_designer_dispatch_catalog(
    const StudioDesignerDispatchCatalogRequest& request) {
    auto admission_catalog = plan_studio_designer_invocation_admission_catalog({
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .symbol = request.symbol,
        .line = request.line,
        .column = request.column,
        .admit_editor_invocations = request.admit_editor_invocations,
        .admit_builder_invocations = request.admit_builder_invocations,
        .admit_toolbox_invocation = request.admit_toolbox_invocation
    });
    if (!admission_catalog.ok) {
        return {
            .ok = false,
            .error = admission_catalog.error,
            .context_count = 0U,
            .contexts = {}
        };
    }

    std::vector<StudioDesignerDispatchCatalogEntry> entries;
    entries.reserve(admission_catalog.contexts.size());
    for (auto& admission_entry : admission_catalog.contexts) {
        StudioDesignerDispatchResult dispatch{};
        if (admission_entry.invocation_admission.ok) {
            dispatch = plan_studio_designer_dispatch({
                .invocation_admission_plan = admission_entry.invocation_admission.plan
            });
        } else {
            dispatch = {
                .ok = false,
                .error = admission_entry.invocation_admission.error,
                .plan = {}
            };
        }

        const auto& plan = dispatch.plan;
        entries.push_back({
            .selection_context = admission_entry.selection_context,
            .editor_action_dispatch_count = dispatch.ok ? plan.editor_action_dispatch_count : 0U,
            .builder_dispatch_count = dispatch.ok ? plan.builder_dispatch_count : 0U,
            .toolbox_dispatch_count = dispatch.ok ? plan.toolbox_dispatch_count : 0U,
            .dispatch_count = dispatch.ok ? plan.dispatch_count : 0U,
            .error_count = dispatch.ok ? plan.error_count : 1U,
            .dry_run = dispatch.ok ? plan.dry_run : true,
            .mutates_asset = dispatch.ok ? plan.mutates_asset : false,
            .dispatch = std::move(dispatch)
        });
    }

    return {
        .ok = true,
        .error = {},
        .context_count = admission_catalog.context_count,
        .contexts = std::move(entries)
    };
}

StudioDesignerDispatchExecutionResult execute_studio_designer_dispatch(
    const StudioDesignerDispatchExecutionRequest& request) {
    auto failed = [&](std::string error) {
        return StudioDesignerDispatchExecutionResult{
            .ok = false,
            .error = std::move(error),
            .dispatch_plan = {},
            .editor_action_executions = {},
            .builder_executions = {},
            .toolbox_execution = {},
            .execution_count = 0U,
            .error_count = 0U,
            .execution_admitted = request.admit_execution,
            .executed = false,
            .dry_run = true,
            .mutates_asset = false
        };
    };

    const auto& dispatch_plan = request.dispatch_plan;
    if (!request.admit_execution) {
        return failed(designer_dispatch_text("Studio.DesignerDispatch.Execution.Error.ExecutionAdmissionRequired"));
    }
    if (dispatch_plan.dispatch_count == 0U) {
        return failed(designer_dispatch_text("Studio.DesignerDispatch.Execution.Error.AdmittedDispatchRequired"));
    }
    if (dispatch_plan.error_count != 0U) {
        return failed(designer_dispatch_text("Studio.DesignerDispatch.Execution.Error.ErrorFreeDispatchRequired"));
    }
    if (dispatch_plan.dry_run) {
        return failed(designer_dispatch_text("Studio.DesignerDispatch.Execution.Error.NonDryRunDispatchRequired"));
    }

    bool needs_editor_executor = false;
    for (const auto& dispatch : dispatch_plan.editor_action_dispatches) {
        if (dispatch.ok) {
            needs_editor_executor = true;
            break;
        }
    }
    bool needs_builder_executor = false;
    for (const auto& dispatch : dispatch_plan.builder_dispatches) {
        if (dispatch.ok) {
            needs_builder_executor = true;
            break;
        }
    }
    const bool needs_toolbox_executor = dispatch_plan.toolbox_dispatch.ok;
    if (needs_editor_executor && !request.editor_action_executor) {
        return failed(designer_dispatch_text("Studio.DesignerDispatch.Execution.Error.EditorExecutorRequired"));
    }
    if (needs_builder_executor && !request.builder_executor) {
        return failed(designer_dispatch_text("Studio.DesignerDispatch.Execution.Error.BuilderExecutorRequired"));
    }
    if (needs_toolbox_executor && !request.toolbox_executor) {
        return failed(designer_dispatch_text("Studio.DesignerDispatch.Execution.Error.ToolboxExecutorRequired"));
    }

    std::vector<StudioEditorActionDispatchExecutionResult> editor_executions;
    editor_executions.reserve(dispatch_plan.editor_action_dispatches.size());
    std::vector<StudioBuilderDispatchExecutionResult> builder_executions;
    builder_executions.reserve(dispatch_plan.builder_dispatches.size());
    StudioToolboxDispatchExecutionResult toolbox_execution{};

    std::size_t execution_count = 0U;
    std::size_t error_count = 0U;
    bool mutates_asset = false;

    for (const auto& dispatch : dispatch_plan.editor_action_dispatches) {
        if (!dispatch.ok) {
            ++error_count;
            editor_executions.push_back({
                .ok = false,
                .error = dispatch.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = true,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            });
            continue;
        }
        auto execution = execute_studio_editor_action_dispatch({
            .dispatch_plan = dispatch.plan,
            .admit_execution = true,
            .executor = request.editor_action_executor
        });
        if (execution.ok) {
            ++execution_count;
            mutates_asset = mutates_asset || execution.mutates_asset;
        } else {
            ++error_count;
        }
        editor_executions.push_back(std::move(execution));
    }

    for (const auto& dispatch : dispatch_plan.builder_dispatches) {
        if (!dispatch.ok) {
            ++error_count;
            builder_executions.push_back({
                .ok = false,
                .error = dispatch.error,
                .dispatch_plan = {},
                .observation = {},
                .execution_admitted = true,
                .executed = false,
                .dry_run = true,
                .mutates_asset = false
            });
            continue;
        }
        auto execution = execute_studio_builder_dispatch({
            .dispatch_plan = dispatch.plan,
            .admit_execution = true,
            .executor = request.builder_executor
        });
        if (execution.ok) {
            ++execution_count;
            mutates_asset = mutates_asset || execution.mutates_asset;
        } else {
            ++error_count;
        }
        builder_executions.push_back(std::move(execution));
    }

    if (dispatch_plan.toolbox_dispatch.ok) {
        toolbox_execution = execute_studio_toolbox_dispatch({
            .dispatch_plan = dispatch_plan.toolbox_dispatch.plan,
            .admit_execution = true,
            .executor = request.toolbox_executor
        });
        if (toolbox_execution.ok) {
            ++execution_count;
            mutates_asset = mutates_asset || toolbox_execution.mutates_asset;
        } else {
            ++error_count;
        }
    }

    if (error_count != 0U) {
        return {
            .ok = true,
            .error = {},
            .dispatch_plan = {},
            .editor_action_executions = std::move(editor_executions),
            .builder_executions = std::move(builder_executions),
            .toolbox_execution = std::move(toolbox_execution),
            .execution_count = execution_count,
            .error_count = error_count,
            .execution_admitted = true,
            .executed = false,
            .dry_run = true,
            .mutates_asset = mutates_asset
        };
    }

    auto executed_plan = dispatch_plan;
    for (auto& dispatch : executed_plan.editor_action_dispatches) {
        if (dispatch.ok) {
            dispatch.plan.executed = true;
        }
    }
    for (auto& dispatch : executed_plan.builder_dispatches) {
        if (dispatch.ok) {
            dispatch.plan.executed = true;
        }
    }
    if (executed_plan.toolbox_dispatch.ok) {
        executed_plan.toolbox_dispatch.plan.executed = true;
    }
    executed_plan.dry_run = false;
    executed_plan.mutates_asset = mutates_asset;

    return {
        .ok = true,
        .error = {},
        .dispatch_plan = std::move(executed_plan),
        .editor_action_executions = std::move(editor_executions),
        .builder_executions = std::move(builder_executions),
        .toolbox_execution = std::move(toolbox_execution),
        .execution_count = execution_count,
        .error_count = 0U,
        .execution_admitted = true,
        .executed = true,
        .dry_run = false,
        .mutates_asset = mutates_asset
    };
}

StudioDesignerDispatchExecutionCatalogResult plan_studio_designer_dispatch_execution_catalog(
    const StudioDesignerDispatchExecutionCatalogRequest& request) {
    auto dispatch_catalog = plan_studio_designer_dispatch_catalog({
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .symbol = request.symbol,
        .line = request.line,
        .column = request.column,
        .admit_editor_invocations = request.admit_editor_invocations,
        .admit_builder_invocations = request.admit_builder_invocations,
        .admit_toolbox_invocation = request.admit_toolbox_invocation
    });
    if (!dispatch_catalog.ok) {
        return {
            .ok = false,
            .error = dispatch_catalog.error,
            .context_count = 0U,
            .execution_ready_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .contexts = {}
        };
    }

    std::vector<StudioDesignerDispatchExecutionCatalogEntry> entries;
    entries.reserve(dispatch_catalog.contexts.size());
    std::size_t execution_ready_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    for (auto& dispatch_entry : dispatch_catalog.contexts) {
        auto execution_error = designer_dispatch_execution_readiness_error(
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
            .selection_context = dispatch_entry.selection_context,
            .editor_action_dispatch_count = dispatch_entry.editor_action_dispatch_count,
            .builder_dispatch_count = dispatch_entry.builder_dispatch_count,
            .toolbox_dispatch_count = dispatch_entry.toolbox_dispatch_count,
            .dispatch_count = dispatch_entry.dispatch_count,
            .dispatch_error_count = dispatch_entry.error_count,
            .dispatch_dry_run = dispatch_entry.dry_run,
            .dispatch_mutates_asset = dispatch_entry.mutates_asset,
            .dispatch = std::move(dispatch_entry.dispatch),
            .execution_admitted = request.admit_execution,
            .execution_ready = execution_ready,
            .execution_error = std::move(execution_error)
        });
    }

    return {
        .ok = true,
        .error = {},
        .context_count = dispatch_catalog.context_count,
        .execution_ready_count = execution_ready_count,
        .error_count = error_count,
        .dry_run = execution_ready_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .contexts = std::move(entries)
    };
}

}  // namespace copperfin::studio
