#include "copperfin/studio/builder_dispatch.h"
#include "copperfin/studio/builder_invocation_admission.h"
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

bool has_argument_pair(const std::vector<std::string>& arguments, const std::string& key, const std::string& value) {
    for (std::size_t index = 0U; (index + 1U) < arguments.size(); index += 2U) {
        if (arguments[index] == key && arguments[index + 1U] == value) {
            return true;
        }
    }
    return false;
}

const copperfin::studio::StudioBuilderDispatchCatalogEntry* find_dispatch_catalog_entry(
    const std::vector<copperfin::studio::StudioBuilderDispatchCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.builder.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioBuilderLaunchCatalogEntry* find_launch_catalog_entry(
    const std::vector<copperfin::studio::StudioBuilderLaunchCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.builder.id == id) {
            return &entry;
        }
    }
    return nullptr;
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

    const auto control_launch_catalog = copperfin::studio::plan_studio_builder_launch_catalog({
        .context = StudioBuilderContext::control,
        .asset_path = "forms/customer.scx",
        .record_index = 4U,
        .object_name = "grdOrders",
        .unique_id = "grid-guid"
    });
    expect(control_launch_catalog.ok &&
               control_launch_catalog.context == StudioBuilderContext::control &&
               control_launch_catalog.builder_count == control_builders.size() &&
               control_launch_catalog.launch_plan_count == control_builders.size() &&
               control_launch_catalog.error_count == 0U &&
               control_launch_catalog.dry_run &&
               !control_launch_catalog.mutates_asset,
           "#1268: builder launch catalogs should plan every context builder without mutation");
    const auto* catalog_grid_launch = find_launch_catalog_entry(control_launch_catalog.entries, "grid-builder");
    expect(catalog_grid_launch != nullptr &&
               catalog_grid_launch->launch_plan.ok &&
               std::string(catalog_grid_launch->launch_plan.plan.builder.id) == "grid-builder" &&
               catalog_grid_launch->launch_plan.plan.builder.kind == StudioBuilderKind::builder &&
               catalog_grid_launch->launch_plan.plan.context == StudioBuilderContext::control &&
               catalog_grid_launch->launch_plan.plan.asset_path == "forms/customer.scx" &&
               catalog_grid_launch->launch_plan.plan.record_index == 4U &&
               catalog_grid_launch->launch_plan.plan.object_name == "grdOrders" &&
               catalog_grid_launch->launch_plan.plan.unique_id == "grid-guid" &&
               catalog_grid_launch->launch_plan.plan.entry_point == "cf_builders.grid_builder",
           "#1268: builder launch catalog entries should preserve builder and target metadata");

    const auto missing_launch_catalog = copperfin::studio::plan_studio_builder_launch_catalog({
        .context = static_cast<StudioBuilderContext>(999),
        .asset_path = "forms/customer.scx",
        .record_index = 4U,
        .object_name = "grdOrders",
        .unique_id = "grid-guid"
    });
    expect(!missing_launch_catalog.ok &&
               missing_launch_catalog.error ==
                   "A builder launch catalog request requires at least one context builder." &&
               missing_launch_catalog.builder_count == 0U &&
               missing_launch_catalog.launch_plan_count == 0U &&
               missing_launch_catalog.error_count == 0U &&
               missing_launch_catalog.dry_run &&
               !missing_launch_catalog.mutates_asset,
           "#1268: builder launch catalogs should reject empty builder contexts without mutation");

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

    const auto control_launch = copperfin::studio::plan_studio_builder_launch({
        .context = StudioBuilderContext::control,
        .builder_id = "grid-builder",
        .asset_path = "forms/customer.scx",
        .record_index = 4U,
        .object_name = "grdOrders",
        .unique_id = "grid-guid"
    });
    expect(control_launch.ok, "#1203: builder launch plans should accept context-valid builders");
    expect(std::string(control_launch.plan.builder.id) == "grid-builder" &&
               control_launch.plan.context == StudioBuilderContext::control &&
               control_launch.plan.asset_path == "forms/customer.scx" &&
               control_launch.plan.record_index == 4U &&
               control_launch.plan.object_name == "grdOrders" &&
               control_launch.plan.unique_id == "grid-guid",
           "#1203: builder launch plans should preserve target asset and selector metadata");
    expect(std::string(control_launch.plan.builder.vfp9_equivalent) == "builder.app grid builder" &&
               std::string(control_launch.plan.builder.copperfin_component) == "cf_form_surface" &&
               control_launch.plan.entry_point == "cf_builders.grid_builder",
           "#1203: builder launch plans should preserve stable builder descriptor metadata");

    const auto label_launch = copperfin::studio::plan_studio_builder_launch({
        .context = StudioBuilderContext::label,
        .builder_id = "label-wizard",
        .asset_path = "labels/mailing.lbx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(label_launch.ok, "#1203: builder launch plans should accept context-valid wizards");
    expect(label_launch.plan.builder.kind == StudioBuilderKind::wizard &&
               std::string(label_launch.plan.builder.vfp9_equivalent) == "Wizards label templates" &&
               label_launch.plan.entry_point == "cf_wizards.label_wizard",
           "#1203: builder launch plans should preserve wizard metadata distinctly from builders");

    const auto admitted_control_invocation = copperfin::studio::plan_studio_builder_invocation_admission({
        .launch_plan = control_launch.plan,
        .admit_ui_launch = true
    });
    expect(admitted_control_invocation.ok,
           "#1215: builder invocation admission should accept validated launch plans");
    expect(std::string(admitted_control_invocation.plan.builder.id) == "grid-builder" &&
               admitted_control_invocation.plan.context == StudioBuilderContext::control &&
               admitted_control_invocation.plan.command_token == "studio.builder.invoke" &&
               admitted_control_invocation.plan.entry_point == "cf_builders.grid_builder" &&
               admitted_control_invocation.plan.asset_path == "forms/customer.scx" &&
               admitted_control_invocation.plan.record_index == 4U &&
               admitted_control_invocation.plan.object_name == "grdOrders" &&
               admitted_control_invocation.plan.unique_id == "grid-guid",
           "#1215: builder invocation admission should preserve launch metadata and emit a stable command token");
    expect(admitted_control_invocation.plan.ui_launch_admitted &&
               !admitted_control_invocation.plan.dry_run &&
               !admitted_control_invocation.plan.mutates_asset,
           "#1215: admitted builder invocation plans should allow UI launch while remaining non-mutating");

    const auto dry_run_label_invocation = copperfin::studio::plan_studio_builder_invocation_admission({
        .launch_plan = label_launch.plan,
        .admit_ui_launch = false
    });
    expect(dry_run_label_invocation.ok &&
               std::string(dry_run_label_invocation.plan.builder.id) == "label-wizard" &&
               dry_run_label_invocation.plan.builder.kind == StudioBuilderKind::wizard &&
               !dry_run_label_invocation.plan.ui_launch_admitted &&
               dry_run_label_invocation.plan.dry_run &&
               !dry_run_label_invocation.plan.mutates_asset,
           "#1215: non-admitted builder invocation plans should remain deterministic dry runs");

    const auto control_dispatch = copperfin::studio::plan_studio_builder_dispatch({
        .admission_plan = admitted_control_invocation.plan
    });
    expect(control_dispatch.ok,
           "#1229: builder dispatch should accept admitted builder invocations");
    expect(std::string(control_dispatch.plan.builder.id) == "grid-builder" &&
               control_dispatch.plan.builder.kind == StudioBuilderKind::builder &&
               control_dispatch.plan.context == StudioBuilderContext::control &&
               control_dispatch.plan.command_token == "studio.builder.invoke" &&
               control_dispatch.plan.entry_point == "cf_builders.grid_builder" &&
               control_dispatch.plan.asset_path == "forms/customer.scx" &&
               control_dispatch.plan.record_index == 4U &&
               control_dispatch.plan.object_name == "grdOrders" &&
               control_dispatch.plan.unique_id == "grid-guid" &&
               control_dispatch.plan.dispatch_admitted &&
               !control_dispatch.plan.dry_run &&
               !control_dispatch.plan.executed &&
               !control_dispatch.plan.mutates_asset,
           "#1229: builder dispatch should preserve admission metadata without executing");
    expect(has_argument_pair(control_dispatch.plan.dispatch_arguments, "--command-token", "studio.builder.invoke") &&
               has_argument_pair(control_dispatch.plan.dispatch_arguments, "--builder-id", "grid-builder") &&
               has_argument_pair(control_dispatch.plan.dispatch_arguments, "--builder-context", "control") &&
               has_argument_pair(control_dispatch.plan.dispatch_arguments, "--entry-point", "cf_builders.grid_builder") &&
               has_argument_pair(control_dispatch.plan.dispatch_arguments, "--path", "forms/customer.scx") &&
               has_argument_pair(control_dispatch.plan.dispatch_arguments, "--record", "4") &&
               has_argument_pair(control_dispatch.plan.dispatch_arguments, "--object-name", "grdOrders") &&
               has_argument_pair(control_dispatch.plan.dispatch_arguments, "--unique-id", "grid-guid"),
           "#1229: builder dispatch should materialize a deterministic argument contract");

    const auto dry_run_builder_dispatch = copperfin::studio::plan_studio_builder_dispatch({
        .admission_plan = dry_run_label_invocation.plan
    });
    expect(!dry_run_builder_dispatch.ok &&
               dry_run_builder_dispatch.error ==
                   "A builder dispatch request requires an admitted non-dry-run invocation.",
           "#1229: builder dispatch should reject dry-run admission plans");

    auto missing_dispatch_command_plan = admitted_control_invocation.plan;
    missing_dispatch_command_plan.command_token = {};
    const auto missing_dispatch_command = copperfin::studio::plan_studio_builder_dispatch({
        .admission_plan = missing_dispatch_command_plan
    });
    expect(!missing_dispatch_command.ok &&
               missing_dispatch_command.error == "A builder dispatch request requires a command token.",
           "#1229: builder dispatch should reject admitted plans without command tokens");

    auto missing_dispatch_entry_plan = admitted_control_invocation.plan;
    missing_dispatch_entry_plan.entry_point = {};
    const auto missing_dispatch_entry = copperfin::studio::plan_studio_builder_dispatch({
        .admission_plan = missing_dispatch_entry_plan
    });
    expect(!missing_dispatch_entry.ok &&
               missing_dispatch_entry.error == "A builder dispatch request requires a launch entry point.",
           "#1229: builder dispatch should reject admitted plans without entry points");

    auto missing_dispatch_builder_plan = admitted_control_invocation.plan;
    missing_dispatch_builder_plan.builder = {};
    const auto missing_dispatch_builder = copperfin::studio::plan_studio_builder_dispatch({
        .admission_plan = missing_dispatch_builder_plan
    });
    expect(!missing_dispatch_builder.ok &&
               missing_dispatch_builder.error == "A builder dispatch request requires a validated builder id.",
           "#1229: builder dispatch should reject admitted plans without builder ids");

    const auto admitted_control_dispatch_catalog = copperfin::studio::plan_studio_builder_dispatch_catalog({
        .context = StudioBuilderContext::control,
        .asset_path = "forms/customer.scx",
        .record_index = 4U,
        .object_name = "grdOrders",
        .unique_id = "grid-guid",
        .admit_ui_launches = true
    });
    expect(admitted_control_dispatch_catalog.ok &&
               admitted_control_dispatch_catalog.context == StudioBuilderContext::control &&
               admitted_control_dispatch_catalog.builder_count == control_builders.size() &&
               admitted_control_dispatch_catalog.dispatch_count == control_builders.size() &&
               admitted_control_dispatch_catalog.error_count == 0U &&
               !admitted_control_dispatch_catalog.dry_run &&
               !admitted_control_dispatch_catalog.mutates_asset,
           "#1231: admitted builder dispatch catalogs should dispatch every context builder without mutation");
    const auto* catalog_grid_dispatch = find_dispatch_catalog_entry(
        admitted_control_dispatch_catalog.entries, "grid-builder");
    expect(catalog_grid_dispatch != nullptr &&
               catalog_grid_dispatch->launch_plan.ok &&
               catalog_grid_dispatch->invocation_admission.ok &&
               catalog_grid_dispatch->dispatch.ok &&
               std::string(catalog_grid_dispatch->dispatch.plan.builder.id) == "grid-builder" &&
               catalog_grid_dispatch->dispatch.plan.builder.kind == StudioBuilderKind::builder &&
               catalog_grid_dispatch->dispatch.plan.context == StudioBuilderContext::control &&
               catalog_grid_dispatch->dispatch.plan.asset_path == "forms/customer.scx" &&
               catalog_grid_dispatch->dispatch.plan.record_index == 4U &&
               catalog_grid_dispatch->dispatch.plan.object_name == "grdOrders" &&
               catalog_grid_dispatch->dispatch.plan.unique_id == "grid-guid" &&
               has_argument_pair(
                   catalog_grid_dispatch->dispatch.plan.dispatch_arguments,
                   "--builder-id",
                   "grid-builder") &&
               has_argument_pair(
                   catalog_grid_dispatch->dispatch.plan.dispatch_arguments,
                   "--builder-context",
                   "control"),
           "#1231: builder dispatch catalog entries should preserve builder and target metadata");

    const auto dry_run_control_dispatch_catalog = copperfin::studio::plan_studio_builder_dispatch_catalog({
        .context = StudioBuilderContext::control,
        .asset_path = "forms/customer.scx",
        .record_index = 4U,
        .object_name = "grdOrders",
        .unique_id = "grid-guid",
        .admit_ui_launches = false
    });
    expect(dry_run_control_dispatch_catalog.ok &&
               dry_run_control_dispatch_catalog.builder_count == control_builders.size() &&
               dry_run_control_dispatch_catalog.dispatch_count == 0U &&
               dry_run_control_dispatch_catalog.error_count == control_builders.size() &&
               dry_run_control_dispatch_catalog.dry_run &&
               !dry_run_control_dispatch_catalog.mutates_asset,
           "#1231: dry-run builder dispatch catalogs should report per-builder dispatch rejections");
    const auto* dry_run_grid_dispatch = find_dispatch_catalog_entry(
        dry_run_control_dispatch_catalog.entries, "grid-builder");
    expect(dry_run_grid_dispatch != nullptr &&
               dry_run_grid_dispatch->launch_plan.ok &&
               dry_run_grid_dispatch->invocation_admission.ok &&
               !dry_run_grid_dispatch->invocation_admission.plan.ui_launch_admitted &&
               !dry_run_grid_dispatch->dispatch.ok &&
               dry_run_grid_dispatch->dispatch.error ==
                   "A builder dispatch request requires an admitted non-dry-run invocation.",
           "#1231: dry-run builder dispatch catalog entries should preserve admission failures");

    const auto label_dispatch_catalog = copperfin::studio::plan_studio_builder_dispatch_catalog({
        .context = StudioBuilderContext::label,
        .asset_path = "labels/mailing.lbx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .admit_ui_launches = true
    });
    const auto label_builders = copperfin::studio::studio_builders_for_context(StudioBuilderContext::label);
    const auto* label_wizard_dispatch = find_dispatch_catalog_entry(
        label_dispatch_catalog.entries, "label-wizard");
    expect(label_dispatch_catalog.ok &&
               label_dispatch_catalog.builder_count == label_builders.size() &&
               label_dispatch_catalog.dispatch_count == label_builders.size() &&
               label_dispatch_catalog.error_count == 0U &&
               label_wizard_dispatch != nullptr &&
               label_wizard_dispatch->dispatch.ok &&
               label_wizard_dispatch->dispatch.plan.builder.kind == StudioBuilderKind::wizard &&
               label_wizard_dispatch->dispatch.plan.context == StudioBuilderContext::label &&
               label_wizard_dispatch->dispatch.plan.entry_point == "cf_wizards.label_wizard" &&
               label_wizard_dispatch->dispatch.plan.asset_path == "labels/mailing.lbx" &&
               has_argument_pair(
                   label_wizard_dispatch->dispatch.plan.dispatch_arguments,
                   "--builder-id",
                   "label-wizard") &&
               has_argument_pair(
                   label_wizard_dispatch->dispatch.plan.dispatch_arguments,
                   "--builder-context",
                   "label"),
           "#1231: label dispatch catalogs should include wizard dispatch metadata");

    auto missing_entry_plan = control_launch.plan;
    missing_entry_plan.entry_point = {};
    const auto missing_entry_invocation = copperfin::studio::plan_studio_builder_invocation_admission({
        .launch_plan = missing_entry_plan,
        .admit_ui_launch = true
    });
    expect(!missing_entry_invocation.ok,
           "#1215: builder invocation admission should reject launch plans without entry points");

    auto missing_builder_plan = control_launch.plan;
    missing_builder_plan.builder = {};
    const auto missing_builder_invocation = copperfin::studio::plan_studio_builder_invocation_admission({
        .launch_plan = missing_builder_plan,
        .admit_ui_launch = true
    });
    expect(!missing_builder_invocation.ok,
           "#1215: builder invocation admission should reject launch plans without builder ids");

    const auto wrong_context_launch = copperfin::studio::plan_studio_builder_launch({
        .context = StudioBuilderContext::report,
        .builder_id = "grid-builder",
        .asset_path = "reports/orders.frx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(!wrong_context_launch.ok,
           "#1203: builder launch plans should reject builders outside the selected context");

    const auto unknown_launch = copperfin::studio::plan_studio_builder_launch({
        .context = StudioBuilderContext::form,
        .builder_id = "unknown-builder",
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(!unknown_launch.ok, "#1203: builder launch plans should reject unknown builder ids");

    const auto missing_id_launch = copperfin::studio::plan_studio_builder_launch({
        .context = StudioBuilderContext::form,
        .builder_id = {},
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(!missing_id_launch.ok, "#1203: builder launch plans should reject missing builder ids");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
