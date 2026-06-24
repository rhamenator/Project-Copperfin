#include "copperfin/studio/builder_registry.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <utility>

namespace copperfin::studio {

namespace {

const copperfin::localization::LocalizedCatalog& builder_registry_catalog() {
    static const copperfin::localization::LocalizedCatalog catalog =
        copperfin::localization::load_catalogs(
            copperfin::localization::resolve_catalog_root(),
            copperfin::localization::select_locale());
    return catalog;
}

std::string builder_registry_text(std::string_view key) {
    return builder_registry_catalog().translate(key);
}

const std::vector<std::string>& builder_registry_display_text(
    const copperfin::localization::LocalizedCatalog& catalog) {
    static std::map<std::string, std::vector<std::string>> text_by_locale;
    const auto cache_key = catalog.requested_locale.empty()
        ? std::string(copperfin::localization::default_locale)
        : catalog.requested_locale;
    const auto [entry, inserted] = text_by_locale.emplace(cache_key, std::vector<std::string>{});
    if (inserted) {
        entry->second = {
            catalog.translate("Studio.Builder.FormBuilder.Title"),
            catalog.translate("Studio.Builder.FormBuilder.Description"),
            catalog.translate("Studio.Builder.ClassBuilder.Title"),
            catalog.translate("Studio.Builder.ClassBuilder.Description"),
            catalog.translate("Studio.Builder.ControlBuilder.Title"),
            catalog.translate("Studio.Builder.ControlBuilder.Description"),
            catalog.translate("Studio.Builder.GridBuilder.Title"),
            catalog.translate("Studio.Builder.GridBuilder.Description"),
            catalog.translate("Studio.Builder.ReportBuilder.Title"),
            catalog.translate("Studio.Builder.ReportBuilder.Description"),
            catalog.translate("Studio.Builder.LabelWizard.Title"),
            catalog.translate("Studio.Builder.LabelWizard.Description"),
            catalog.translate("Studio.Builder.MenuDesigner.Title"),
            catalog.translate("Studio.Builder.MenuDesigner.Description"),
            catalog.translate("Studio.Builder.ApplicationWizard.Title"),
            catalog.translate("Studio.Builder.ApplicationWizard.Description"),
            catalog.translate("Studio.Builder.DataEnvironmentBuilder.Title"),
            catalog.translate("Studio.Builder.DataEnvironmentBuilder.Description")
        };
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
            .description = text[1]
        },
        {
            .id = "class-builder",
            .title = text[2],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::class_designer,
            .vfp9_equivalent = "builder.app class builder",
            .copperfin_component = "cf_class_surface",
            .entry_point = "cf_builders.class_builder",
            .description = text[3]
        },
        {
            .id = "control-builder",
            .title = text[4],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::control,
            .vfp9_equivalent = "builder.app control builders",
            .copperfin_component = "cf_form_surface",
            .entry_point = "cf_builders.control_builder",
            .description = text[5]
        },
        {
            .id = "grid-builder",
            .title = text[6],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::control,
            .vfp9_equivalent = "builder.app grid builder",
            .copperfin_component = "cf_form_surface",
            .entry_point = "cf_builders.grid_builder",
            .description = text[7]
        },
        {
            .id = "report-builder",
            .title = text[8],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::report,
            .vfp9_equivalent = "ReportBuilder.app",
            .copperfin_component = "cf_report_surface",
            .entry_point = "cf_builders.report_builder",
            .description = text[9]
        },
        {
            .id = "label-wizard",
            .title = text[10],
            .kind = StudioBuilderKind::wizard,
            .context = StudioBuilderContext::label,
            .vfp9_equivalent = "Wizards label templates",
            .copperfin_component = "cf_wizards",
            .entry_point = "cf_wizards.label_wizard",
            .description = text[11]
        },
        {
            .id = "menu-designer",
            .title = text[12],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::menu,
            .vfp9_equivalent = "Menu Designer",
            .copperfin_component = "cf_menu_surface",
            .entry_point = "cf_builders.menu_designer",
            .description = text[13]
        },
        {
            .id = "application-wizard",
            .title = text[14],
            .kind = StudioBuilderKind::wizard,
            .context = StudioBuilderContext::project,
            .vfp9_equivalent = "Wizards application templates",
            .copperfin_component = "cf_wizards",
            .entry_point = "cf_wizards.application_wizard",
            .description = text[15]
        },
        {
            .id = "data-environment-builder",
            .title = text[16],
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::data_environment,
            .vfp9_equivalent = "data environment builder",
            .copperfin_component = "cf_data_explorer",
            .entry_point = "cf_builders.data_environment_builder",
            .description = text[17]
        }
    };
}

const std::vector<StudioBuilderDescriptor>& studio_builder_registry() {
    static const std::vector<StudioBuilderDescriptor> builders =
        studio_builder_registry_for_catalog(builder_registry_catalog());
    return builders;
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
