// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/builder_invocation_admission.h"

#include "copperfin/localization/localization.h"

#include <string_view>
#include <mutex>
#include <utility>

namespace copperfin::studio {

namespace {

copperfin::localization::LocalizedCatalog builder_invocation_catalog() {
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

std::string builder_invocation_text(std::string_view key) {
    return builder_invocation_catalog().translate(key);
}

}  // namespace

StudioBuilderInvocationAdmissionResult plan_studio_builder_invocation_admission(
    const StudioBuilderInvocationAdmissionRequest& request) {
    if (request.launch_plan.builder.id.empty()) {
        return {
            .ok = false,
            .error = builder_invocation_text("Studio.BuilderInvocationAdmission.Error.ValidatedBuilderIdRequired"),
            .plan = {}
        };
    }
    if (request.launch_plan.entry_point.empty()) {
        return {
            .ok = false,
            .error = builder_invocation_text("Studio.BuilderInvocationAdmission.Error.EntryPointRequired"),
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .builder = request.launch_plan.builder,
            .context = request.launch_plan.context,
            .command_token = "studio.builder.invoke",
            .entry_point = request.launch_plan.entry_point,
            .asset_path = request.launch_plan.asset_path,
            .record_index = request.launch_plan.record_index,
            .object_name = request.launch_plan.object_name,
            .unique_id = request.launch_plan.unique_id,
            .ui_launch_admitted = request.admit_ui_launch,
            .dry_run = !request.admit_ui_launch,
            .mutates_asset = false
        }
    };
}

StudioBuilderInvocationAdmissionCatalogResult plan_studio_builder_invocation_admission_catalog(
    const StudioBuilderInvocationAdmissionCatalogRequest& request) {
    const auto builders = studio_builders_for_context(request.context);
    if (builders.empty()) {
        return {
            .ok = false,
            .error = builder_invocation_text("Studio.BuilderInvocationAdmission.Error.CatalogRequiresBuilder"),
            .context = request.context,
            .builder_count = 0U,
            .admission_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioBuilderInvocationAdmissionCatalogEntry> entries;
    entries.reserve(builders.size());
    std::size_t admission_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    for (const auto& builder : builders) {
        auto launch_plan = plan_studio_builder_launch({
            .context = request.context,
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
            .launch_plan = std::move(launch_plan),
            .invocation_admission = std::move(invocation_admission)
        });
    }

    return {
        .ok = true,
        .error = {},
        .context = request.context,
        .builder_count = builders.size(),
        .admission_count = admission_count,
        .error_count = error_count,
        .dry_run = dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
