#include "copperfin/studio/builder_registry.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

bool has_builder(const std::vector<copperfin::studio::StudioBuilderDescriptor>& builders, std::string_view id) {
    for (const auto& builder : builders) {
        if (builder.id == id) {
            return true;
        }
    }
    return false;
}

}  // namespace

int main() {
    using copperfin::studio::StudioBuilderContext;
    using copperfin::studio::StudioBuilderKind;

    const auto& builders = copperfin::studio::studio_builder_registry();
    expect(builders.size() >= 9U, "#956: builder registry should cover major VFP-compatible designer actions");
    expect(std::string(copperfin::studio::studio_builder_kind_name(StudioBuilderKind::builder)) == "builder",
           "#956: builder kind token should be stable");
    expect(std::string(copperfin::studio::studio_builder_kind_name(StudioBuilderKind::wizard)) == "wizard",
           "#956: wizard kind token should be stable");
    expect(std::string(copperfin::studio::studio_builder_context_name(StudioBuilderContext::data_environment)) ==
               "data_environment",
           "#956: data-environment context token should be stable");
    expect(std::string(copperfin::studio::studio_builder_context_name(StudioBuilderContext::menu)) == "menu",
           "#1013: menu builder context token should be stable");

    bool found_builder = false;
    bool found_wizard = false;
    bool found_vfp_equivalent = false;

    for (const auto& builder : builders) {
        expect(!std::string(builder.id).empty(), "#956: each builder descriptor should have an id");
        expect(!std::string(builder.title).empty(), "#956: each builder descriptor should have a title");
        expect(!std::string(builder.vfp9_equivalent).empty(),
               "#956: each builder descriptor should name the VFP 9 equivalent");
        expect(!std::string(builder.copperfin_component).empty(),
               "#956: each builder descriptor should name the Copperfin component");
        expect(!std::string(builder.entry_point).empty(), "#956: each builder descriptor should name an entry point");
        expect(!std::string(builder.description).empty(), "#956: each builder descriptor should describe the action");
        if (builder.kind == StudioBuilderKind::builder) {
            found_builder = true;
        }
        if (builder.kind == StudioBuilderKind::wizard) {
            found_wizard = true;
        }
        if (builder.vfp9_equivalent.find("builder.app") != std::string_view::npos ||
            builder.vfp9_equivalent.find("ReportBuilder.app") != std::string_view::npos ||
            builder.vfp9_equivalent.find("Wizards") != std::string_view::npos) {
            found_vfp_equivalent = true;
        }
    }

    expect(found_builder, "#956: registry should include builder actions");
    expect(found_wizard, "#956: registry should include wizard actions");
    expect(found_vfp_equivalent, "#956: registry should preserve VFP builder/wizard equivalent names");
    expect(has_builder(builders, "form-builder"), "#956: registry should include the form builder");
    expect(has_builder(builders, "control-builder"), "#956: registry should include the control builder");
    expect(has_builder(builders, "grid-builder"), "#956: registry should include the grid builder");
    expect(has_builder(builders, "report-builder"), "#956: registry should include the report builder");
    expect(has_builder(builders, "menu-designer"), "#1013: registry should include the menu designer builder");
    expect(has_builder(builders, "application-wizard"), "#956: registry should include the application wizard");

    const auto control_builders = copperfin::studio::studio_builders_for_context(StudioBuilderContext::control);
    expect(control_builders.size() >= 2U, "#956: control context should expose multiple control builders");
    expect(has_builder(control_builders, "control-builder"), "#956: control context should include control builder");
    expect(has_builder(control_builders, "grid-builder"), "#956: control context should include grid builder");
    expect(!has_builder(control_builders, "report-builder"), "#956: control context should not include report builders");

    const auto report_builders = copperfin::studio::studio_builders_for_context(StudioBuilderContext::report);
    expect(report_builders.size() == 1U, "#956: report context should expose only report actions for now");
    expect(has_builder(report_builders, "report-builder"), "#956: report context should include report builder");

    const auto menu_builders = copperfin::studio::studio_builders_for_context(StudioBuilderContext::menu);
    expect(menu_builders.size() == 1U, "#1013: menu context should expose only menu designer actions for now");
    expect(has_builder(menu_builders, "menu-designer"), "#1013: menu context should include menu designer builder");
    expect(!has_builder(menu_builders, "form-builder"), "#1013: menu context should exclude form builders");

    const auto project_builders = copperfin::studio::studio_builders_for_context(StudioBuilderContext::project);
    expect(project_builders.size() == 1U, "#956: project context should expose application wizard");
    expect(has_builder(project_builders, "application-wizard"), "#956: project context should include application wizard");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
