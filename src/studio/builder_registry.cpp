#include "copperfin/studio/builder_registry.h"

#include <algorithm>

namespace copperfin::studio {

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
        case StudioBuilderContext::project:
            return "project";
        case StudioBuilderContext::data_environment:
            return "data_environment";
    }
    return "form";
}

const std::vector<StudioBuilderDescriptor>& studio_builder_registry() {
    static const std::vector<StudioBuilderDescriptor> builders = {
        {
            .id = "form-builder",
            .title = "Form Builder",
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::form,
            .vfp9_equivalent = "builder.app form builder",
            .copperfin_component = "cf_form_surface",
            .entry_point = "cf_builders.form_builder",
            .description = "Configure form-level data, layout, and generated method defaults."
        },
        {
            .id = "class-builder",
            .title = "Class Builder",
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::class_designer,
            .vfp9_equivalent = "builder.app class builder",
            .copperfin_component = "cf_class_surface",
            .entry_point = "cf_builders.class_builder",
            .description = "Configure visual class defaults, inheritance metadata, and reusable members."
        },
        {
            .id = "control-builder",
            .title = "Control Builder",
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::control,
            .vfp9_equivalent = "builder.app control builders",
            .copperfin_component = "cf_form_surface",
            .entry_point = "cf_builders.control_builder",
            .description = "Configure selected control bindings, captions, styles, and generated event hooks."
        },
        {
            .id = "grid-builder",
            .title = "Grid Builder",
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::control,
            .vfp9_equivalent = "builder.app grid builder",
            .copperfin_component = "cf_form_surface",
            .entry_point = "cf_builders.grid_builder",
            .description = "Configure grid columns, data bindings, headings, and display behavior."
        },
        {
            .id = "report-builder",
            .title = "Report Builder",
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::report,
            .vfp9_equivalent = "ReportBuilder.app",
            .copperfin_component = "cf_report_surface",
            .entry_point = "cf_builders.report_builder",
            .description = "Configure report data, grouping, bands, expressions, and preview defaults."
        },
        {
            .id = "label-wizard",
            .title = "Label Wizard",
            .kind = StudioBuilderKind::wizard,
            .context = StudioBuilderContext::label,
            .vfp9_equivalent = "Wizards label templates",
            .copperfin_component = "cf_wizards",
            .entry_point = "cf_wizards.label_wizard",
            .description = "Create label layouts from stock/template choices while preserving LBX/LBT semantics."
        },
        {
            .id = "application-wizard",
            .title = "Application Wizard",
            .kind = StudioBuilderKind::wizard,
            .context = StudioBuilderContext::project,
            .vfp9_equivalent = "Wizards application templates",
            .copperfin_component = "cf_wizards",
            .entry_point = "cf_wizards.application_wizard",
            .description = "Scaffold project assets, startup programs, and template forms using VFP-compatible metadata."
        },
        {
            .id = "data-environment-builder",
            .title = "Data Environment Builder",
            .kind = StudioBuilderKind::builder,
            .context = StudioBuilderContext::data_environment,
            .vfp9_equivalent = "data environment builder",
            .copperfin_component = "cf_data_explorer",
            .entry_point = "cf_builders.data_environment_builder",
            .description = "Configure table/cursor bindings and relation metadata for form and report data environments."
        }
    };

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

}  // namespace copperfin::studio
