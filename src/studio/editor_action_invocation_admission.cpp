// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/editor_action_invocation_admission.h"

#include "copperfin/localization/localization.h"

#include <string>
#include <string_view>
#include <mutex>
#include <utility>

namespace copperfin::studio {

namespace {

copperfin::localization::LocalizedCatalog editor_action_invocation_admission_catalog() {
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

std::string editor_action_invocation_admission_text(std::string_view key) {
    return editor_action_invocation_admission_catalog().translate(key);
}

}  // namespace

StudioEditorActionInvocationAdmissionResult plan_studio_editor_action_invocation_admission(
    const StudioEditorActionInvocationAdmissionRequest& request) {
    if (request.launch_plan.action.id.empty()) {
        return {
            .ok = false,
            .error = editor_action_invocation_admission_text(
                "Studio.EditorActionInvocationAdmission.Error.ValidatedActionIdRequired"),
            .plan = {}
        };
    }
    if (request.launch_plan.command_token.empty()) {
        return {
            .ok = false,
            .error = editor_action_invocation_admission_text(
                "Studio.EditorActionInvocationAdmission.Error.CommandTokenRequired"),
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .action = request.launch_plan.action,
            .selection_context = request.launch_plan.selection_context,
            .command_token = request.launch_plan.command_token,
            .target_surface = request.launch_plan.target_surface,
            .asset_path = request.launch_plan.asset_path,
            .record_index = request.launch_plan.record_index,
            .object_name = request.launch_plan.object_name,
            .unique_id = request.launch_plan.unique_id,
            .symbol = request.launch_plan.symbol,
            .line = request.launch_plan.line,
            .column = request.launch_plan.column,
            .editor_invocation_admitted = request.admit_editor_invocation,
            .dry_run = !request.admit_editor_invocation,
            .mutates_asset = false
        }
    };
}

StudioEditorActionInvocationAdmissionCatalogResult
plan_studio_editor_action_invocation_admission_catalog(
    const StudioEditorActionInvocationAdmissionCatalogRequest& request) {
    const auto actions = studio_editor_actions_for_context(request.selection_context);
    if (actions.empty()) {
        return {
            .ok = false,
            .error = editor_action_invocation_admission_text(
                "Studio.EditorActionInvocationAdmission.Error.CatalogRequiresAction"),
            .selection_context = request.selection_context,
            .action_count = 0U,
            .admission_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioEditorActionInvocationAdmissionCatalogEntry> entries;
    entries.reserve(actions.size());
    std::size_t admission_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    for (const auto& action : actions) {
        auto launch_plan = plan_studio_editor_action_launch({
            .selection_context = request.selection_context,
            .action_id = std::string(action.id),
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .symbol = request.symbol,
            .line = request.line,
            .column = request.column
        });

        StudioEditorActionInvocationAdmissionResult invocation_admission{};
        if (launch_plan.ok) {
            invocation_admission = plan_studio_editor_action_invocation_admission({
                .launch_plan = launch_plan.plan,
                .admit_editor_invocation = request.admit_editor_invocations
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
            .action = action,
            .launch_plan = std::move(launch_plan),
            .invocation_admission = std::move(invocation_admission)
        });
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .action_count = entries.size(),
        .admission_count = admission_count,
        .error_count = error_count,
        .dry_run = admission_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
