// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/builder_dispatch.h"

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

copperfin::localization::LocalizedCatalog builder_dispatch_catalog() {
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

std::string builder_dispatch_text(std::string_view key) {
    return builder_dispatch_catalog().translate(key);
}

std::string builder_dispatch_execution_readiness_error(
    const StudioBuilderDispatchResult& dispatch,
    bool admit_execution) {
    if (!dispatch.ok) {
        return dispatch.error;
    }
    if (!admit_execution) {
        return builder_dispatch_text("Studio.BuilderDispatch.CatalogEntry.Error.ExecutionAdmissionRequired");
    }
    const auto& dispatch_plan = dispatch.plan;
    if (dispatch_plan.builder.id.empty()) {
        return builder_dispatch_text("Studio.BuilderDispatch.CatalogEntry.Error.ValidatedBuilderIdRequired");
    }
    if (dispatch_plan.command_token.empty()) {
        return builder_dispatch_text("Studio.BuilderDispatch.CatalogEntry.Error.CommandTokenRequired");
    }
    if (dispatch_plan.entry_point.empty()) {
        return builder_dispatch_text("Studio.BuilderDispatch.CatalogEntry.Error.EntryPointRequired");
    }
    if (!dispatch_plan.dispatch_admitted || dispatch_plan.dry_run) {
        return builder_dispatch_text("Studio.BuilderDispatch.CatalogEntry.Error.AdmittedDispatchRequired");
    }
    if (dispatch_plan.executed) {
        return builder_dispatch_text("Studio.BuilderDispatch.CatalogEntry.Error.NonExecutedDispatchRequired");
    }
    if (dispatch_plan.dispatch_arguments.empty()) {
        return builder_dispatch_text("Studio.BuilderDispatch.CatalogEntry.Error.DispatchArgumentsRequired");
    }

    return {};
}

}  // namespace

StudioBuilderDispatchResult plan_studio_builder_dispatch(
    const StudioBuilderDispatchRequest& request) {
    if (request.admission_plan.builder.id.empty()) {
        return {
            .ok = false,
            .error = builder_dispatch_text("Studio.BuilderDispatch.Error.ValidatedBuilderIdRequired"),
            .plan = {}
        };
    }
    if (request.admission_plan.command_token.empty()) {
        return {
            .ok = false,
            .error = builder_dispatch_text("Studio.BuilderDispatch.Error.CommandTokenRequired"),
            .plan = {}
        };
    }
    if (request.admission_plan.entry_point.empty()) {
        return {
            .ok = false,
            .error = builder_dispatch_text("Studio.BuilderDispatch.Error.EntryPointRequired"),
            .plan = {}
        };
    }
    if (!request.admission_plan.ui_launch_admitted || request.admission_plan.dry_run) {
        return {
            .ok = false,
            .error = builder_dispatch_text("Studio.BuilderDispatch.Error.AdmittedInvocationRequired"),
            .plan = {}
        };
    }

    std::vector<std::string> arguments;
    append_argument(arguments, "--command-token", request.admission_plan.command_token);
    append_argument(arguments, "--builder-id", std::string(request.admission_plan.builder.id));
    append_argument(arguments, "--builder-context", studio_builder_context_name(request.admission_plan.context));
    append_argument(arguments, "--entry-point", request.admission_plan.entry_point);
    append_argument(arguments, "--path", request.admission_plan.asset_path);
    append_argument(arguments, "--record", std::to_string(request.admission_plan.record_index));
    append_argument(arguments, "--object-name", request.admission_plan.object_name);
    append_argument(arguments, "--unique-id", request.admission_plan.unique_id);

    return {
        .ok = true,
        .error = {},
        .plan = {
            .builder = request.admission_plan.builder,
            .context = request.admission_plan.context,
            .command_token = request.admission_plan.command_token,
            .entry_point = request.admission_plan.entry_point,
            .asset_path = request.admission_plan.asset_path,
            .record_index = request.admission_plan.record_index,
            .object_name = request.admission_plan.object_name,
            .unique_id = request.admission_plan.unique_id,
            .dispatch_arguments = std::move(arguments),
            .dispatch_admitted = true,
            .dry_run = false,
            .executed = false,
            .mutates_asset = false
        }
    };
}

StudioBuilderDispatchCatalogResult plan_studio_builder_dispatch_catalog(
    const StudioBuilderDispatchCatalogRequest& request) {
    auto admission_catalog = plan_studio_builder_invocation_admission_catalog({
        .context = request.context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .admit_ui_launches = request.admit_ui_launches
    });
    if (!admission_catalog.ok) {
        return {
            .ok = false,
            .error = builder_dispatch_text("Studio.BuilderDispatch.Error.DispatchCatalogRequiresBuilder"),
            .context = request.context,
            .builder_count = 0U,
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioBuilderDispatchCatalogEntry> entries;
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
            .launch_plan = std::move(admission_entry.launch_plan),
            .invocation_admission = std::move(admission_entry.invocation_admission),
            .dispatch = std::move(dispatch)
        });
    }

    return {
        .ok = true,
        .error = {},
        .context = request.context,
        .builder_count = admission_catalog.builder_count,
        .dispatch_count = dispatch_count,
        .error_count = error_count,
        .dry_run = dispatch_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

StudioBuilderDispatchExecutionResult execute_studio_builder_dispatch(
    const StudioBuilderDispatchExecutionRequest& request) {
    auto failed = [&](std::string error) {
        return StudioBuilderDispatchExecutionResult{
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
        return failed(builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.ExecutionAdmissionRequired"));
    }
    if (!request.executor) {
        return failed(builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.ExecutorRequired"));
    }
    if (dispatch_plan.builder.id.empty()) {
        return failed(builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.ValidatedBuilderIdRequired"));
    }
    if (dispatch_plan.command_token.empty()) {
        return failed(builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.CommandTokenRequired"));
    }
    if (dispatch_plan.entry_point.empty()) {
        return failed(builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.EntryPointRequired"));
    }
    if (!dispatch_plan.dispatch_admitted || dispatch_plan.dry_run) {
        return failed(builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.AdmittedDispatchRequired"));
    }
    if (dispatch_plan.executed) {
        return failed(builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.NonExecutedDispatchRequired"));
    }
    if (dispatch_plan.dispatch_arguments.empty()) {
        return failed(builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.DispatchArgumentsRequired"));
    }

    auto observation = request.executor(dispatch_plan);
    if (!observation.launched) {
        return {
            .ok = false,
            .error = observation.error.empty()
                ? builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.ExecutorDidNotLaunch")
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
                ? builder_dispatch_text("Studio.BuilderDispatch.Execution.Error.ExecutorNonZeroExit")
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

StudioBuilderDispatchExecutionCatalogResult plan_studio_builder_dispatch_execution_catalog(
    const StudioBuilderDispatchExecutionCatalogRequest& request) {
    auto dispatch_catalog = plan_studio_builder_dispatch_catalog({
        .context = request.context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .admit_ui_launches = request.admit_ui_launches
    });
    if (!dispatch_catalog.ok) {
        return {
            .ok = false,
            .error = builder_dispatch_text("Studio.BuilderDispatch.Error.ExecutionCatalogRequiresBuilder"),
            .context = request.context,
            .builder_count = 0U,
            .execution_ready_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioBuilderDispatchExecutionCatalogEntry> entries;
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
        .context = request.context,
        .builder_count = dispatch_catalog.builder_count,
        .execution_ready_count = execution_ready_count,
        .error_count = error_count,
        .dry_run = execution_ready_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
