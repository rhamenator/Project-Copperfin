// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/builder_registry.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <iterator>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

namespace copperfin::studio {

namespace {

copperfin::localization::LocalizedCatalog builder_registry_catalog() {
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
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root();
    const std::string locale = copperfin::localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = copperfin::localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

std::string builder_registry_text(std::string_view key) {
    return builder_registry_catalog().translate(key);
}

const std::vector<std::string>& builder_registry_display_text(
    const copperfin::localization::LocalizedCatalog& catalog) {
    static const std::array<std::string_view, 27U> translation_keys = {
        "Studio.Builder.FormBuilder.Title",
        "Studio.Builder.FormBuilder.Description",
        "Studio.Builder.ClassBuilder.Title",
        "Studio.Builder.ClassBuilder.Description",
        "Studio.Builder.ControlBuilder.Title",
        "Studio.Builder.ControlBuilder.Description",
        "Studio.Builder.GridBuilder.Title",
        "Studio.Builder.GridBuilder.Description",
        "Studio.Builder.ReportBuilder.Title",
        "Studio.Builder.ReportBuilder.Description",
        "Studio.Builder.LabelWizard.Title",
        "Studio.Builder.LabelWizard.Description",
        "Studio.Builder.MenuDesigner.Title",
        "Studio.Builder.MenuDesigner.Description",
        "Studio.Builder.ApplicationWizard.Title",
        "Studio.Builder.ApplicationWizard.Description",
        "Studio.Builder.DataEnvironmentBuilder.Title",
        "Studio.Builder.DataEnvironmentBuilder.Description",
        "Studio.Builder.FormBuilder.Vfp9Equivalent",
        "Studio.Builder.ClassBuilder.Vfp9Equivalent",
        "Studio.Builder.ControlBuilder.Vfp9Equivalent",
        "Studio.Builder.GridBuilder.Vfp9Equivalent",
        "Studio.Builder.ReportBuilder.Vfp9Equivalent",
        "Studio.Builder.LabelWizard.Vfp9Equivalent",
        "Studio.Builder.MenuDesigner.Vfp9Equivalent",
        "Studio.Builder.ApplicationWizard.Vfp9Equivalent",
        "Studio.Builder.DataEnvironmentBuilder.Vfp9Equivalent"
    };
    static std::map<std::string, std::vector<std::string>> text_by_locale;
    static std::mutex text_cache_mutex;
    std::string cache_key = catalog.requested_locale.empty()
        ? std::string(copperfin::localization::default_locale)
        : catalog.requested_locale;
    for (const auto key : translation_keys) {
        cache_key.push_back('\0');
        cache_key += catalog.translate(key);
    }
    std::lock_guard<std::mutex> lock(text_cache_mutex);
    const auto [entry, inserted] = text_by_locale.emplace(cache_key, std::vector<std::string>{});
    if (inserted) {
        entry->second.reserve(translation_keys.size());
        for (const auto key : translation_keys) {
            entry->second.push_back(catalog.translate(key));
        }
    }
    return entry->second;
}

}  // namespace

const char* studio_builder_kind_name(StudioBuilderKind kind) {
    switch (kind) {
        case StudioBuilderKind::builder:
            return "builder";
        case StudioBuilderKind::wizard:
            return "wizard";
    }
    return "builder";
}

const char* studio_builder_context_name(StudioBuilderContext context) {
    switch (context) {
        case StudioBuilderContext::form:
            return "form";
        case StudioBuilderContext::class_designer:
            return "class_designer";
        case StudioBuilderContext::control:
            return "control";
        case StudioBuilderContext::report:
            return "report";
        case StudioBuilderContext::label:
            return "label";
        case StudioBuilderContext::menu:
            return "menu";
        case StudioBuilderContext::project:
            return "project";
        case StudioBuilderContext::data_environment:
            return "data_environment";
    }
    return "form";
}

std::vector<StudioBuilderDescriptor> studio_builder_registry_for_catalog(
    const copperfin::localization::LocalizedCatalog& catalog) {
    const auto& text = builder_registry_display_text(catalog);
    return {
        {
            .id = "form-builder",
            .title = text[0],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::form,
            .vfp9_equivalent = "builder.app form builder",
            .copperfin_component = "cf_form_surface",
            .entry_point = "cf_builders.form_builder",
            .description = text[1],
            .vfp9_equivalent_display = text[18]
        },
        {
            .id = "class-builder",
            .title = text[2],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::class_designer,
            .vfp9_equivalent = "builder.app class builder",
            .copperfin_component = "cf_class_surface",
            .entry_point = "cf_builders.class_builder",
            .description = text[3],
            .vfp9_equivalent_display = text[19]
        },
        {
            .id = "control-builder",
            .title = text[4],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::control,
            .vfp9_equivalent = "builder.app control builders",
            .copperfin_component = "cf_form_surface",
            .entry_point = "cf_builders.control_builder",
            .description = text[5],
            .vfp9_equivalent_display = text[20]
        },
        {
            .id = "grid-builder",
            .title = text[6],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::control,
            .vfp9_equivalent = "builder.app grid builder",
            .copperfin_component = "cf_form_surface",
            .entry_point = "cf_builders.grid_builder",
            .description = text[7],
            .vfp9_equivalent_display = text[21]
        },
        {
            .id = "report-builder",
            .title = text[8],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::report,
            .vfp9_equivalent = "ReportBuilder.app",
            .copperfin_component = "cf_report_surface",
            .entry_point = "cf_builders.report_builder",
            .description = text[9],
            .vfp9_equivalent_display = text[22]
        },
        {
            .id = "label-wizard",
            .title = text[10],
            .kind = StudioBuilderKind::wizard,
            .context = StudioBuilderContext::label,
            .vfp9_equivalent = "Wizards label templates",
            .copperfin_component = "cf_wizards",
            .entry_point = "cf_wizards.label_wizard",
            .description = text[11],
            .vfp9_equivalent_display = text[23]
        },
        {
            .id = "menu-designer",
            .title = text[12],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::menu,
            .vfp9_equivalent = "Menu Designer",
            .copperfin_component = "cf_menu_surface",
            .entry_point = "cf_builders.menu_designer",
            .description = text[13],
            .vfp9_equivalent_display = text[24]
        },
        {
            .id = "application-wizard",
            .title = text[14],
            .kind = StudioBuilderKind::wizard,
            .context = StudioBuilderContext::project,
            .vfp9_equivalent = "Wizards application templates",
            .copperfin_component = "cf_wizards",
            .entry_point = "cf_wizards.application_wizard",
            .description = text[15],
            .vfp9_equivalent_display = text[25]
        },
        {
            .id = "data-environment-builder",
            .title = text[16],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::data_environment,
            .vfp9_equivalent = "data environment builder",
            .copperfin_component = "cf_data_explorer",
            .entry_point = "cf_builders.data_environment_builder",
            .description = text[17],
            .vfp9_equivalent_display = text[26]
        }
    };
}

std::vector<StudioBuilderDescriptor> studio_builder_registry() {
    struct BuilderCache {
        std::filesystem::path locale_root;
        std::string locale;
        std::vector<StudioBuilderDescriptor> builders;
    };

    const auto catalog = builder_registry_catalog();
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root();
    const std::string locale = copperfin::localization::select_locale();
    static std::mutex cache_mutex;
    static BuilderCache cache{};
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.builders = studio_builder_registry_for_catalog(catalog);
    }
    return cache.builders;
}

std::vector<StudioBuilderDescriptor> studio_builders_for_context(StudioBuilderContext context) {
    std::vector<StudioBuilderDescriptor> filtered;
    const auto& builders = studio_builder_registry();
    std::copy_if(
        builders.begin(),
        builders.end(),
        std::back_inserter(filtered),
        [&](const StudioBuilderDescriptor& builder) {
            return builder.context == context;
        });
    return filtered;
}

StudioBuilderLaunchPlanResult plan_studio_builder_launch(const StudioBuilderLaunchRequest& request) {
    if (request.builder_id.empty()) {
        return {
            .ok = false,
            .error = builder_registry_text("Studio.BuilderRegistry.Error.BuilderIdRequired"),
            .plan = {}
        };
    }

    const auto builders = studio_builders_for_context(request.context);
    const auto builder = std::find_if(
        builders.begin(),
        builders.end(),
        [&](const StudioBuilderDescriptor& candidate) {
            return candidate.id == request.builder_id;
        });

    if (builder == builders.end()) {
        return {
            .ok = false,
            .error = builder_registry_text("Studio.BuilderRegistry.Error.BuilderUnavailableForContext"),
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .builder = *builder,
            .context = request.context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .entry_point = std::string(builder->entry_point)
        }
    };
}

StudioBuilderLaunchCatalogResult plan_studio_builder_launch_catalog(
    const StudioBuilderLaunchCatalogRequest& request) {
    const auto builders = studio_builders_for_context(request.context);
    if (builders.empty()) {
        return {
            .ok = false,
            .error = builder_registry_text("Studio.BuilderRegistry.Error.LaunchCatalogRequiresBuilder"),
            .context = request.context,
            .builder_count = 0U,
            .launch_plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioBuilderLaunchCatalogEntry> entries;
    entries.reserve(builders.size());
    std::size_t launch_plan_count = 0U;
    std::size_t error_count = 0U;

    for (const auto& builder : builders) {
        auto launch_plan = plan_studio_builder_launch({
            .context = request.context,
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
            .launch_plan = std::move(launch_plan)
        });
    }

    return {
        .ok = true,
        .error = {},
        .context = request.context,
        .builder_count = builders.size(),
        .launch_plan_count = launch_plan_count,
        .error_count = error_count,
        .dry_run = true,
        .mutates_asset = false,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
