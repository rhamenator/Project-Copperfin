#include "copperfin/studio/builder_dispatch.h"

#include <string>
#include <utility>

namespace copperfin::studio {

namespace {

void append_argument(std::vector<std::string>& arguments, std::string key, std::string value) {
    arguments.push_back(std::move(key));
    arguments.push_back(std::move(value));
}

}  // namespace

StudioBuilderDispatchResult plan_studio_builder_dispatch(
    const StudioBuilderDispatchRequest& request) {
    if (request.admission_plan.builder.id.empty()) {
        return {
            .ok = false,
            .error = "A builder dispatch request requires a validated builder id.",
            .plan = {}
        };
    }
    if (request.admission_plan.command_token.empty()) {
        return {
            .ok = false,
            .error = "A builder dispatch request requires a command token.",
            .plan = {}
        };
    }
    if (request.admission_plan.entry_point.empty()) {
        return {
            .ok = false,
            .error = "A builder dispatch request requires a launch entry point.",
            .plan = {}
        };
    }
    if (!request.admission_plan.ui_launch_admitted || request.admission_plan.dry_run) {
        return {
            .ok = false,
            .error = "A builder dispatch request requires an admitted non-dry-run invocation.",
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
            .error = "A builder dispatch catalog request requires at least one context builder.",
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

}  // namespace copperfin::studio
