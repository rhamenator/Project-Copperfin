// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/studio/designer_context.h"
#include "copperfin/studio/designer_dispatch.h"
#include "copperfin/studio/designer_invocation_admission.h"
#include "copperfin/studio/designer_launch_surfaces.h"
#include "test_environment_support.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
}

template <typename Descriptor>
bool has_id(const std::vector<Descriptor>& descriptors, std::string_view id) {
    for (const auto& descriptor : descriptors) {
        if (descriptor.id == id) {
            return true;
        }
    }
    return false;
}

void test_designer_context_default_catalog_refreshes_when_locale_changes() {
    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    locale_override.set("en-US");
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto english_result = copperfin::studio::plan_studio_builder_launch_for_selection({});
    locale_override.set("es-419");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto spanish_result = copperfin::studio::plan_studio_builder_launch_for_selection({});
    locale_override.set("qps-ploc");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto pseudo_result = copperfin::studio::plan_studio_builder_launch_for_selection({});
    constexpr std::string_view key = "Studio.SelectionBuilderLaunch.Error.BuilderIdRequired";
    expect(!english_result.ok && english_result.error == english_catalog.translate(key) &&
               !spanish_result.ok && spanish_result.error == spanish_catalog.translate(key) &&
               !pseudo_result.ok && pseudo_result.error == pseudo_catalog.translate(key),
           "#4369: designer-context diagnostics should refresh across locales");
}

bool has_action_launch_plan(
    const std::vector<copperfin::studio::StudioEditorActionLaunchPlanResult>& plans,
    std::string_view id) {
    for (const auto& plan : plans) {
        if (plan.ok && plan.plan.action.id == id) {
            return true;
        }
    }
    return false;
}

bool has_builder_launch_plan(
    const std::vector<copperfin::studio::StudioSelectionBuilderLaunchPlanResult>& plans,
    std::string_view id) {
    for (const auto& plan : plans) {
        if (plan.ok && plan.plan.builder.id == id) {
            return true;
        }
    }
    return false;
}

bool all_action_launch_plans_ok(
    const std::vector<copperfin::studio::StudioEditorActionLaunchPlanResult>& plans) {
    for (const auto& plan : plans) {
        if (!plan.ok) {
            return false;
        }
    }
    return true;
}

bool all_builder_launch_plans_ok(
    const std::vector<copperfin::studio::StudioSelectionBuilderLaunchPlanResult>& plans) {
    for (const auto& plan : plans) {
        if (!plan.ok) {
            return false;
        }
    }
    return true;
}

const copperfin::studio::StudioSelectionBuilderLaunchCatalogEntry*
find_selection_builder_launch_entry(
    const std::vector<copperfin::studio::StudioSelectionBuilderLaunchCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.builder.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

bool has_editor_invocation_admission(
    const std::vector<copperfin::studio::StudioEditorActionInvocationAdmissionResult>& admissions,
    std::string_view id,
    bool admitted) {
    for (const auto& admission : admissions) {
        if (admission.ok &&
            admission.plan.action.id == id &&
            admission.plan.editor_invocation_admitted == admitted) {
            return true;
        }
    }
    return false;
}

bool has_builder_invocation_admission(
    const std::vector<copperfin::studio::StudioBuilderInvocationAdmissionResult>& admissions,
    std::string_view id,
    bool admitted) {
    for (const auto& admission : admissions) {
        if (admission.ok &&
            admission.plan.builder.id == id &&
            admission.plan.ui_launch_admitted == admitted) {
            return true;
        }
    }
    return false;
}

bool has_editor_dispatch(
    const std::vector<copperfin::studio::StudioEditorActionDispatchResult>& dispatches,
    std::string_view id,
    bool admitted) {
    for (const auto& dispatch : dispatches) {
        if (dispatch.ok &&
            dispatch.plan.action.id == id &&
            dispatch.plan.dispatch_admitted == admitted) {
            return true;
        }
    }
    return false;
}

bool has_builder_dispatch(
    const std::vector<copperfin::studio::StudioBuilderDispatchResult>& dispatches,
    std::string_view id,
    bool admitted) {
    for (const auto& dispatch : dispatches) {
        if (dispatch.ok &&
            dispatch.plan.builder.id == id &&
            dispatch.plan.dispatch_admitted == admitted) {
            return true;
        }
    }
    return false;
}

const copperfin::studio::StudioDesignerLaunchSurfaceCatalogEntry* find_catalog_entry(
    const std::vector<copperfin::studio::StudioDesignerLaunchSurfaceCatalogEntry>& entries,
    copperfin::studio::StudioEditorSelectionContext context) {
    for (const auto& entry : entries) {
        if (entry.selection_context == context) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioDesignerInvocationAdmissionCatalogEntry* find_invocation_catalog_entry(
    const std::vector<copperfin::studio::StudioDesignerInvocationAdmissionCatalogEntry>& entries,
    copperfin::studio::StudioEditorSelectionContext context) {
    for (const auto& entry : entries) {
        if (entry.selection_context == context) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogEntry*
find_selection_builder_admission_entry(
    const std::vector<copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.builder.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioSelectionBuilderDispatchCatalogEntry*
find_selection_builder_dispatch_entry(
    const std::vector<copperfin::studio::StudioSelectionBuilderDispatchCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.builder.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioDesignerDispatchCatalogEntry* find_dispatch_catalog_entry(
    const std::vector<copperfin::studio::StudioDesignerDispatchCatalogEntry>& entries,
    copperfin::studio::StudioEditorSelectionContext context) {
    for (const auto& entry : entries) {
        if (entry.selection_context == context) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioDesignerDispatchExecutionCatalogEntry*
find_dispatch_execution_catalog_entry(
    const std::vector<copperfin::studio::StudioDesignerDispatchExecutionCatalogEntry>& entries,
    copperfin::studio::StudioEditorSelectionContext context) {
    for (const auto& entry : entries) {
        if (entry.selection_context == context) {
            return &entry;
        }
    }
    return nullptr;
}

}  // namespace

int main() {
    test_designer_context_default_catalog_refreshes_when_locale_changes();
    using copperfin::studio::StudioDesignerContextRequest;
    using copperfin::studio::StudioEditorSelectionContext;

    const auto visual_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::visual_object
    });
    expect(visual_context.selection_context == StudioEditorSelectionContext::visual_object,
           "#959: visual context result should preserve the requested selection context");
    expect(has_id(visual_context.editor_actions, "show-property-grid"),
           "#959: visual context should include property-grid action");
    expect(has_id(visual_context.editor_actions, "edit-visual-method"),
           "#959: visual context should include method-editor action");
    expect(visual_context.editor_action_count == visual_context.editor_actions.size(),
           "#1009: visual context should report editor-action count metadata");
    expect(has_id(visual_context.builders, "control-builder"),
           "#959: visual context should include control builder");
    expect(has_id(visual_context.builders, "grid-builder"), "#959: visual context should include grid builder");
    expect(visual_context.builder_count == visual_context.builders.size(),
           "#1009: visual context should report builder count metadata");
    expect(visual_context.builder_count == 3U,
           "#1010: visual context should expose form plus control builders");
    expect(has_id(visual_context.builders, "form-builder"),
           "#1010: visual context should include form builder");
    expect(has_id(visual_context.toolbox_items, "textbox"), "#959: visual context should include TextBox toolbox item");
    expect(has_id(visual_context.toolbox_items, "pageframe"),
           "#959: visual context should include PageFrame toolbox item");
    expect(visual_context.toolbox_item_count == visual_context.toolbox_items.size(),
           "#1009: visual context should report toolbox-item count metadata");

    const auto container_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::container_object
    });
    expect(container_context.selection_context == StudioEditorSelectionContext::container_object,
           "#1014: container context result should preserve the requested selection context");
    expect(has_id(container_context.editor_actions, "show-property-grid"),
           "#1014: container context should include property-grid action");
    expect(has_id(container_context.editor_actions, "edit-visual-method"),
           "#1014: container context should include method-editor action");
    expect(has_id(container_context.builders, "control-builder"),
           "#1014: container context should include control builder");
    expect(has_id(container_context.builders, "grid-builder"),
           "#1014: container context should include grid builder");
    expect(!has_id(container_context.builders, "form-builder"),
           "#1014: container context should not inherit form builder availability");
    expect(!has_id(container_context.builders, "class-builder"),
           "#1014: container context should not inherit class builder availability");
    expect(has_id(container_context.toolbox_items, "checkbox"),
           "#1014: container context should include container-safe CheckBox toolbox item");
    expect(has_id(container_context.toolbox_items, "grid"),
           "#1014: container context should include container-safe Grid toolbox item");
    expect(container_context.builder_count == 2U &&
               container_context.editor_action_count == container_context.editor_actions.size() &&
               container_context.toolbox_item_count == container_context.toolbox_items.size(),
           "#1014: container context should report filtered descriptor counts");

    const auto class_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::class_designer
    });
    expect(class_context.selection_context == StudioEditorSelectionContext::class_designer,
           "#1012: class context result should preserve the requested selection context");
    expect(has_id(class_context.editor_actions, "show-property-grid"),
           "#1012: class context should include property-grid action");
    expect(has_id(class_context.editor_actions, "edit-visual-method"),
           "#1012: class context should include method-editor action");
    expect(has_id(class_context.builders, "class-builder"),
           "#1012: class context should include class builder");
    expect(!has_id(class_context.builders, "form-builder"),
           "#1012: class context should not inherit form builder availability");
    expect(!has_id(class_context.builders, "control-builder"),
           "#1012: class context should not inherit control builder availability");
    expect(has_id(class_context.toolbox_items, "textbox"),
           "#1012: class context should include class-safe TextBox toolbox item");
    expect(class_context.builder_count == 1U &&
               class_context.editor_action_count == class_context.editor_actions.size() &&
               class_context.toolbox_item_count == class_context.toolbox_items.size(),
           "#1012: class context should report filtered descriptor counts");

    const auto report_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::report_expression
    });
    expect(has_id(report_context.editor_actions, "edit-report-expression"),
           "#959: report context should include expression editor action");
    expect(!has_id(report_context.editor_actions, "edit-visual-method"),
           "#959: report context should exclude visual method editor action");
    expect(has_id(report_context.builders, "report-builder"),
           "#959: report context should include report builder");
    expect(has_id(report_context.toolbox_items, "label"), "#959: report context should include Label toolbox item");
    expect(!has_id(report_context.toolbox_items, "textbox"),
           "#959: report context should exclude form-only TextBox toolbox item");
    expect(report_context.editor_action_count == report_context.editor_actions.size() &&
               report_context.builder_count == report_context.builders.size() &&
               report_context.toolbox_item_count == report_context.toolbox_items.size(),
           "#1009: report context should report counts from filtered descriptor vectors");

    const auto label_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::label_expression
    });
    expect(label_context.selection_context == StudioEditorSelectionContext::label_expression,
           "#1011: label context result should preserve the requested selection context");
    expect(has_id(label_context.editor_actions, "edit-report-expression"),
           "#1011: label context should include shared expression editor action");
    expect(has_id(label_context.builders, "label-wizard"),
           "#1011: label context should include label wizard");
    expect(!has_id(label_context.builders, "report-builder"),
           "#1011: label context should not reuse report builder metadata");
    expect(has_id(label_context.toolbox_items, "label"),
           "#1011: label context should include report-safe Label toolbox item");
    expect(label_context.builder_count == 1U &&
               label_context.editor_action_count == label_context.editor_actions.size() &&
               label_context.toolbox_item_count == label_context.toolbox_items.size(),
           "#1011: label context should report filtered descriptor counts");

    const auto menu_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::menu_item
    });
    expect(menu_context.selection_context == StudioEditorSelectionContext::menu_item,
           "#1013: menu context result should preserve the requested selection context");
    expect(has_id(menu_context.editor_actions, "show-property-grid"),
           "#1013: menu context should include property-grid action");
    expect(has_id(menu_context.editor_actions, "edit-menu-command"),
           "#1413: menu context should include menu command editor action");
    expect(has_id(menu_context.editor_actions, "open-builder"),
           "#1013: menu context should include builder action");
    expect(has_id(menu_context.builders, "menu-designer"),
           "#1013: menu context should include menu designer builder");
    expect(!has_id(menu_context.builders, "form-builder"),
           "#1013: menu context should not inherit form builder availability");
    expect(!has_id(menu_context.builders, "report-builder"),
           "#1013: menu context should not inherit report builder availability");
    expect(menu_context.toolbox_items.empty(),
           "#1013: menu context should not expose toolbox items");
    expect(menu_context.builder_count == 1U &&
               menu_context.editor_action_count == menu_context.editor_actions.size() &&
               menu_context.toolbox_item_count == 0U,
           "#1013: menu context should report filtered descriptor counts");

    const auto visual_method_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::visual_method
    });
    expect(has_id(visual_method_context.builders, "control-builder"),
           "#1010: visual-method context should preserve control builder availability");
    expect(!has_id(visual_method_context.builders, "form-builder"),
           "#1010: visual-method context should not inherit form builder availability");

    const auto project_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::project_item
    });
    expect(has_id(project_context.editor_actions, "navigate-project-item"),
           "#959: project context should include project navigation action");
    expect(has_id(project_context.builders, "application-wizard"),
           "#959: project context should include application wizard");
    expect(project_context.toolbox_items.empty(), "#959: project context should not expose toolbox items");
    expect(project_context.toolbox_item_count == 0U,
           "#1009: project context should report zero toolbox-item count");

    const auto data_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::data_environment
    });
    expect(has_id(data_context.editor_actions, "show-property-grid"),
           "#1410: data-environment context should include property-grid action");
    expect(has_id(data_context.editor_actions, "edit-data-environment"),
           "#959: data-environment context should include data-environment editor action");
    expect(has_id(data_context.builders, "data-environment-builder"),
           "#959: data-environment context should include data-environment builder");
    expect(data_context.toolbox_items.empty(), "#959: data-environment context should not expose toolbox items");
    expect(data_context.editor_action_count == data_context.editor_actions.size() &&
               data_context.builder_count == data_context.builders.size() &&
               data_context.toolbox_item_count == 0U,
           "#1009: data-environment context should report filtered descriptor counts");

    const auto visual_form_plan = copperfin::studio::plan_studio_builder_launch_for_selection({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .builder_id = "form-builder",
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid"
    });
    expect(visual_form_plan.ok,
           "#1205: visual-object selection contexts should plan form builders");
    expect(visual_form_plan.selection_context == StudioEditorSelectionContext::visual_object &&
               std::string(visual_form_plan.plan.builder.id) == "form-builder" &&
               visual_form_plan.plan.context == copperfin::studio::StudioBuilderContext::form &&
               visual_form_plan.plan.asset_path == "forms/customer.scx" &&
               visual_form_plan.plan.record_index == 1U &&
               visual_form_plan.plan.object_name == "frmCustomer" &&
               visual_form_plan.plan.unique_id == "form-guid",
           "#1205: selection-context builder launch plans should preserve visual selection metadata");
    expect(visual_form_plan.plan.entry_point == "cf_builders.form_builder",
           "#1205: selection-context builder launch plans should preserve entry-point metadata");

    const auto visual_grid_plan = copperfin::studio::plan_studio_builder_launch_for_selection({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .builder_id = "grid-builder",
        .asset_path = "forms/customer.scx",
        .record_index = 4U,
        .object_name = "grdOrders",
        .unique_id = "grid-guid"
    });
    expect(visual_grid_plan.ok &&
               std::string(visual_grid_plan.plan.builder.id) == "grid-builder" &&
               visual_grid_plan.plan.context == copperfin::studio::StudioBuilderContext::control,
           "#1205: visual-object selection contexts should also plan control builders");

    const auto container_form_plan = copperfin::studio::plan_studio_builder_launch_for_selection({
        .selection_context = StudioEditorSelectionContext::container_object,
        .builder_id = "form-builder",
        .asset_path = "forms/customer.scx",
        .record_index = 2U,
        .object_name = "pgAddress",
        .unique_id = "page-guid"
    });
    expect(!container_form_plan.ok,
           "#1205: container selection contexts should reject form-only builders");

    const auto label_plan = copperfin::studio::plan_studio_builder_launch_for_selection({
        .selection_context = StudioEditorSelectionContext::label_expression,
        .builder_id = "label-wizard",
        .asset_path = "labels/mailing.lbx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(label_plan.ok &&
               std::string(label_plan.plan.builder.id) == "label-wizard" &&
               label_plan.plan.builder.kind == copperfin::studio::StudioBuilderKind::wizard &&
               label_plan.plan.entry_point == "cf_wizards.label_wizard",
           "#1205: label selection contexts should plan label wizards");

    const auto data_builder_plan = copperfin::studio::plan_studio_builder_launch_for_selection({
        .selection_context = StudioEditorSelectionContext::data_environment,
        .builder_id = "data-environment-builder",
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = "Dataenvironment",
        .unique_id = "de-guid"
    });
    expect(data_builder_plan.ok &&
               std::string(data_builder_plan.plan.builder.id) == "data-environment-builder" &&
               data_builder_plan.plan.context == copperfin::studio::StudioBuilderContext::data_environment,
           "#1205: data-environment selection contexts should plan data-environment builders");

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> selection_builder_error_keys = {
        "Studio.SelectionBuilderDispatch.Error.DispatchCatalogRequiresBuilder",
        "Studio.SelectionBuilderDispatch.Error.ExecutionCatalogRequiresBuilder",
        "Studio.SelectionBuilderInvocationAdmission.Error.CatalogRequiresBuilder",
        "Studio.SelectionBuilderLaunch.Error.BuilderIdRequired",
        "Studio.SelectionBuilderLaunch.Error.BuilderUnavailableForContext",
        "Studio.SelectionBuilderLaunch.Error.CatalogRequiresBuilder"};
    expect(english_catalog.translate("Studio.SelectionBuilderLaunch.Error.BuilderIdRequired") ==
               "A selection-context builder launch request requires a builder id." &&
               english_catalog.translate("Studio.SelectionBuilderLaunch.Error.CatalogRequiresBuilder") ==
                   "A selection-context builder launch catalog request requires at least one builder." &&
               english_catalog.translate("Studio.SelectionBuilderInvocationAdmission.Error.CatalogRequiresBuilder") ==
                   "A selection-context builder invocation admission catalog request requires at least one builder." &&
               english_catalog.translate("Studio.SelectionBuilderDispatch.Error.DispatchCatalogRequiresBuilder") ==
                   "A selection-context builder dispatch catalog request requires at least one builder." &&
               english_catalog.translate("Studio.SelectionBuilderDispatch.Error.ExecutionCatalogRequiresBuilder") ==
                   "A selection-context builder dispatch execution catalog request requires at least one builder." &&
               pseudo_catalog.translate("Studio.SelectionBuilderLaunch.Error.BuilderUnavailableForContext").starts_with("[!! ") &&
               pseudo_catalog.translate("Studio.BuilderDispatch.CatalogEntry.Error.ExecutionAdmissionRequired").starts_with("[!! "),
           "#2369: selection-context builder error prose should resolve through localizable catalog keys");
    expect(
        spanish_catalog.translate("Studio.SelectionBuilderLaunch.Error.BuilderIdRequired") ==
            "Una solicitud de inicio de builder con contexto de seleccion requiere un id de builder.",
        "#2649: es-419 selection-builder launch-id error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.SelectionBuilderDispatch.Error.ExecutionCatalogRequiresBuilder") ==
            "Una solicitud de catalogo de ejecucion de dispatch de builder con contexto de seleccion requiere al menos un builder.",
        "#2649: es-419 selection-builder dispatch execution-catalog error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.SelectionBuilderInvocationAdmission.Error.CatalogRequiresBuilder") ==
            "Uma solicitacao de catalogo de admissao de invocacao de builder com contexto de selecao exige pelo menos um builder.",
        "#2649: pt-BR selection-builder invocation-admission catalog error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.SelectionBuilderLaunch.Error.BuilderUnavailableForContext") ==
            "O builder solicitado nao esta disponivel para o contexto selecionado do Studio.",
        "#2649: pt-BR selection-builder unavailable-for-context error should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.SelectionBuilderLaunch.Error.CatalogRequiresBuilder") ==
            copperfin::localization::pseudo_localize(
                "A selection-context builder launch catalog request requires at least one builder."),
        "#2649: qps-ploc selection-builder launch-catalog error should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", selection_builder_error_keys) == 0U,
        "#2649: es-419 should define every remaining Studio.SelectionBuilder localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", selection_builder_error_keys) == 0U,
        "#2649: pt-BR should define every remaining Studio.SelectionBuilder localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", selection_builder_error_keys) == 0U,
        "#2649: qps-ploc should define every remaining Studio.SelectionBuilder localization key");

    const auto missing_builder_plan = copperfin::studio::plan_studio_builder_launch_for_selection({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .builder_id = {},
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(!missing_builder_plan.ok,
           "#1205: selection-context builder launch plans should reject missing builder ids");

    const auto unknown_builder_plan = copperfin::studio::plan_studio_builder_launch_for_selection({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .builder_id = "unknown-builder",
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(!unknown_builder_plan.ok,
           "#1205: selection-context builder launch plans should reject unknown builders");

    const auto visual_builder_launch_catalog =
        copperfin::studio::plan_studio_builder_launch_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid"
        });
    const auto* visual_form_launch = find_selection_builder_launch_entry(
        visual_builder_launch_catalog.entries, "form-builder");
    const auto* visual_grid_launch = find_selection_builder_launch_entry(
        visual_builder_launch_catalog.entries, "grid-builder");
    expect(visual_builder_launch_catalog.ok &&
               visual_builder_launch_catalog.selection_context == StudioEditorSelectionContext::visual_object &&
               visual_builder_launch_catalog.builder_count == visual_context.builder_count &&
               visual_builder_launch_catalog.launch_plan_count == visual_context.builder_count &&
               visual_builder_launch_catalog.error_count == 0U &&
               visual_builder_launch_catalog.dry_run &&
               !visual_builder_launch_catalog.mutates_asset,
           "#1277: visual selection builder launch catalogs should plan every visual builder");
    expect(visual_form_launch != nullptr &&
               visual_form_launch->selection_context == StudioEditorSelectionContext::visual_object &&
               visual_form_launch->launch_plan.ok &&
               visual_form_launch->launch_plan.plan.context == copperfin::studio::StudioBuilderContext::form &&
               visual_form_launch->launch_plan.plan.entry_point == "cf_builders.form_builder" &&
               visual_form_launch->launch_plan.plan.asset_path == "forms/customer.scx" &&
               visual_form_launch->launch_plan.plan.record_index == 1U &&
               visual_form_launch->launch_plan.plan.object_name == "frmCustomer" &&
               visual_form_launch->launch_plan.plan.unique_id == "form-guid",
           "#1277: visual selection launch catalogs should preserve form builder metadata");
    expect(visual_grid_launch != nullptr &&
               visual_grid_launch->launch_plan.ok &&
               visual_grid_launch->launch_plan.plan.context == copperfin::studio::StudioBuilderContext::control,
           "#1277: visual selection launch catalogs should include control builders beside form builders");

    const auto menu_builder_launch_catalog =
        copperfin::studio::plan_studio_builder_launch_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::menu_item,
            .asset_path = "menus/main.mnx",
            .record_index = 0U,
            .object_name = "mnuMain",
            .unique_id = "menu-guid"
        });
    const auto* menu_builder_launch = find_selection_builder_launch_entry(
        menu_builder_launch_catalog.entries, "menu-designer");
    expect(menu_builder_launch_catalog.ok &&
               menu_builder_launch_catalog.builder_count == menu_context.builder_count &&
               menu_builder_launch != nullptr &&
               menu_builder_launch->launch_plan.ok &&
               menu_builder_launch->launch_plan.plan.context == copperfin::studio::StudioBuilderContext::menu &&
               menu_builder_launch->launch_plan.plan.entry_point == "cf_builders.menu_designer" &&
               menu_builder_launch->launch_plan.plan.asset_path == "menus/main.mnx",
           "#1277: menu selection builder launch catalogs should preserve menu designer metadata");

    const auto empty_selection_launch_catalog =
        copperfin::studio::plan_studio_builder_launch_catalog_for_selection({
            .selection_context = static_cast<StudioEditorSelectionContext>(999),
            .asset_path = "forms/customer.scx",
            .record_index = 0U,
            .object_name = {},
            .unique_id = {}
        });
    expect(!empty_selection_launch_catalog.ok &&
               empty_selection_launch_catalog.error ==
                   "A selection-context builder launch catalog request requires at least one builder." &&
               empty_selection_launch_catalog.builder_count == 0U &&
               empty_selection_launch_catalog.launch_plan_count == 0U &&
               empty_selection_launch_catalog.error_count == 0U &&
               empty_selection_launch_catalog.dry_run &&
               !empty_selection_launch_catalog.mutates_asset,
           "#1277: selection builder launch catalogs should reject empty builder contexts without mutation");

    const auto visual_builder_admission_catalog =
        copperfin::studio::plan_studio_builder_invocation_admission_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid",
            .admit_ui_launches = true
        });
    expect(visual_builder_admission_catalog.ok &&
               visual_builder_admission_catalog.selection_context == StudioEditorSelectionContext::visual_object &&
               visual_builder_admission_catalog.builder_count == visual_context.builder_count &&
               visual_builder_admission_catalog.admission_count == visual_context.builder_count &&
               visual_builder_admission_catalog.error_count == 0U &&
               !visual_builder_admission_catalog.dry_run &&
               !visual_builder_admission_catalog.mutates_asset,
           "#1273: visual selection builder admission catalogs should admit every visual builder");
    const auto* visual_form_admission = find_selection_builder_admission_entry(
        visual_builder_admission_catalog.entries, "form-builder");
    const auto* visual_grid_admission = find_selection_builder_admission_entry(
        visual_builder_admission_catalog.entries, "grid-builder");
    expect(visual_form_admission != nullptr &&
               visual_form_admission->selection_context == StudioEditorSelectionContext::visual_object &&
               visual_form_admission->launch_plan.ok &&
               visual_form_admission->invocation_admission.ok &&
               visual_form_admission->invocation_admission.plan.context ==
                   copperfin::studio::StudioBuilderContext::form &&
               visual_form_admission->invocation_admission.plan.command_token == "studio.builder.invoke" &&
               visual_form_admission->invocation_admission.plan.entry_point == "cf_builders.form_builder" &&
               visual_form_admission->invocation_admission.plan.asset_path == "forms/customer.scx" &&
               visual_form_admission->invocation_admission.plan.record_index == 1U &&
               visual_form_admission->invocation_admission.plan.object_name == "frmCustomer" &&
               visual_form_admission->invocation_admission.plan.unique_id == "form-guid" &&
               visual_form_admission->invocation_admission.plan.ui_launch_admitted &&
               !visual_form_admission->invocation_admission.plan.mutates_asset,
           "#1273: visual selection admission catalogs should preserve form builder metadata");
    expect(visual_grid_admission != nullptr &&
               visual_grid_admission->invocation_admission.ok &&
               visual_grid_admission->invocation_admission.plan.context ==
                   copperfin::studio::StudioBuilderContext::control &&
               visual_grid_admission->invocation_admission.plan.ui_launch_admitted,
           "#1273: visual selection admission catalogs should include control builders beside form builders");

    const auto dry_run_visual_admission_catalog =
        copperfin::studio::plan_studio_builder_invocation_admission_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid",
            .admit_ui_launches = false
        });
    const auto* dry_run_visual_form_admission = find_selection_builder_admission_entry(
        dry_run_visual_admission_catalog.entries, "form-builder");
    expect(dry_run_visual_admission_catalog.ok &&
               dry_run_visual_admission_catalog.admission_count == visual_context.builder_count &&
               dry_run_visual_admission_catalog.error_count == 0U &&
               dry_run_visual_admission_catalog.dry_run &&
               !dry_run_visual_admission_catalog.mutates_asset &&
               dry_run_visual_form_admission != nullptr &&
               dry_run_visual_form_admission->invocation_admission.ok &&
               !dry_run_visual_form_admission->invocation_admission.plan.ui_launch_admitted &&
               dry_run_visual_form_admission->invocation_admission.plan.dry_run,
           "#1273: dry-run selection builder admission catalogs should preserve non-admitted state");

    const auto menu_builder_admission_catalog =
        copperfin::studio::plan_studio_builder_invocation_admission_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::menu_item,
            .asset_path = "menus/main.mnx",
            .record_index = 0U,
            .object_name = "mnuMain",
            .unique_id = "menu-guid",
            .admit_ui_launches = true
        });
    const auto* menu_builder_admission = find_selection_builder_admission_entry(
        menu_builder_admission_catalog.entries, "menu-designer");
    expect(menu_builder_admission_catalog.ok &&
               menu_builder_admission_catalog.builder_count == menu_context.builder_count &&
               menu_builder_admission != nullptr &&
               menu_builder_admission->invocation_admission.ok &&
               menu_builder_admission->invocation_admission.plan.context ==
                   copperfin::studio::StudioBuilderContext::menu &&
               menu_builder_admission->invocation_admission.plan.entry_point == "cf_builders.menu_designer" &&
               menu_builder_admission->invocation_admission.plan.asset_path == "menus/main.mnx",
           "#1273: menu selection builder admission catalogs should preserve menu designer metadata");

    const auto empty_selection_admission_catalog =
        copperfin::studio::plan_studio_builder_invocation_admission_catalog_for_selection({
            .selection_context = static_cast<StudioEditorSelectionContext>(999),
            .asset_path = "forms/customer.scx",
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .admit_ui_launches = true
        });
    expect(!empty_selection_admission_catalog.ok &&
               empty_selection_admission_catalog.error ==
                   "A selection-context builder invocation admission catalog request requires at least one builder." &&
               empty_selection_admission_catalog.builder_count == 0U &&
               empty_selection_admission_catalog.admission_count == 0U &&
               empty_selection_admission_catalog.error_count == 0U &&
               empty_selection_admission_catalog.dry_run &&
               !empty_selection_admission_catalog.mutates_asset,
           "#1273: selection builder admission catalogs should reject empty builder contexts without mutation");

    const auto visual_builder_dispatch_catalog =
        copperfin::studio::plan_studio_builder_dispatch_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid",
            .admit_ui_launches = true
        });
    const auto* visual_form_dispatch = find_selection_builder_dispatch_entry(
        visual_builder_dispatch_catalog.entries, "form-builder");
    const auto* visual_grid_dispatch = find_selection_builder_dispatch_entry(
        visual_builder_dispatch_catalog.entries, "grid-builder");
    expect(visual_builder_dispatch_catalog.ok &&
               visual_builder_dispatch_catalog.selection_context == StudioEditorSelectionContext::visual_object &&
               visual_builder_dispatch_catalog.builder_count == visual_context.builder_count &&
               visual_builder_dispatch_catalog.dispatch_count == visual_context.builder_count &&
               visual_builder_dispatch_catalog.error_count == 0U &&
               !visual_builder_dispatch_catalog.dry_run &&
               !visual_builder_dispatch_catalog.mutates_asset,
           "#1275: visual selection builder dispatch catalogs should dispatch every admitted visual builder");
    expect(visual_form_dispatch != nullptr &&
               visual_form_dispatch->selection_context == StudioEditorSelectionContext::visual_object &&
               visual_form_dispatch->launch_plan.ok &&
               visual_form_dispatch->invocation_admission.ok &&
               visual_form_dispatch->dispatch.ok &&
               visual_form_dispatch->dispatch.plan.context == copperfin::studio::StudioBuilderContext::form &&
               visual_form_dispatch->dispatch.plan.command_token == "studio.builder.invoke" &&
               visual_form_dispatch->dispatch.plan.entry_point == "cf_builders.form_builder" &&
               visual_form_dispatch->dispatch.plan.asset_path == "forms/customer.scx" &&
               visual_form_dispatch->dispatch.plan.record_index == 1U &&
               visual_form_dispatch->dispatch.plan.object_name == "frmCustomer" &&
               visual_form_dispatch->dispatch.plan.unique_id == "form-guid" &&
               visual_form_dispatch->dispatch.plan.dispatch_admitted &&
               !visual_form_dispatch->dispatch.plan.executed &&
               !visual_form_dispatch->dispatch.plan.mutates_asset,
           "#1275: visual selection dispatch catalogs should preserve form builder dispatch metadata");
    expect(visual_grid_dispatch != nullptr &&
               visual_grid_dispatch->dispatch.ok &&
               visual_grid_dispatch->dispatch.plan.context == copperfin::studio::StudioBuilderContext::control &&
               visual_grid_dispatch->dispatch.plan.dispatch_admitted,
           "#1275: visual selection dispatch catalogs should include control builders beside form builders");

    const auto dry_run_visual_dispatch_catalog =
        copperfin::studio::plan_studio_builder_dispatch_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid",
            .admit_ui_launches = false
        });
    const auto* dry_run_visual_form_dispatch = find_selection_builder_dispatch_entry(
        dry_run_visual_dispatch_catalog.entries, "form-builder");
    expect(dry_run_visual_dispatch_catalog.ok &&
               dry_run_visual_dispatch_catalog.builder_count == visual_context.builder_count &&
               dry_run_visual_dispatch_catalog.dispatch_count == 0U &&
               dry_run_visual_dispatch_catalog.error_count == visual_context.builder_count &&
               dry_run_visual_dispatch_catalog.dry_run &&
               !dry_run_visual_dispatch_catalog.mutates_asset &&
               dry_run_visual_form_dispatch != nullptr &&
               dry_run_visual_form_dispatch->invocation_admission.ok &&
               !dry_run_visual_form_dispatch->dispatch.ok &&
               dry_run_visual_form_dispatch->dispatch.error ==
                   "A builder dispatch request requires an admitted non-dry-run invocation.",
           "#1275: dry-run selection builder dispatch catalogs should reject non-admitted dispatches");

    const auto menu_builder_dispatch_catalog =
        copperfin::studio::plan_studio_builder_dispatch_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::menu_item,
            .asset_path = "menus/main.mnx",
            .record_index = 0U,
            .object_name = "mnuMain",
            .unique_id = "menu-guid",
            .admit_ui_launches = true
        });
    const auto* menu_builder_dispatch = find_selection_builder_dispatch_entry(
        menu_builder_dispatch_catalog.entries, "menu-designer");
    expect(menu_builder_dispatch_catalog.ok &&
               menu_builder_dispatch_catalog.builder_count == menu_context.builder_count &&
               menu_builder_dispatch != nullptr &&
               menu_builder_dispatch->dispatch.ok &&
               menu_builder_dispatch->dispatch.plan.context == copperfin::studio::StudioBuilderContext::menu &&
               menu_builder_dispatch->dispatch.plan.entry_point == "cf_builders.menu_designer" &&
               menu_builder_dispatch->dispatch.plan.asset_path == "menus/main.mnx",
           "#1275: menu selection builder dispatch catalogs should preserve menu designer dispatch metadata");

    const auto empty_selection_dispatch_catalog =
        copperfin::studio::plan_studio_builder_dispatch_catalog_for_selection({
            .selection_context = static_cast<StudioEditorSelectionContext>(999),
            .asset_path = "forms/customer.scx",
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .admit_ui_launches = true
        });
    expect(!empty_selection_dispatch_catalog.ok &&
               empty_selection_dispatch_catalog.error ==
                   "A selection-context builder dispatch catalog request requires at least one builder." &&
               empty_selection_dispatch_catalog.builder_count == 0U &&
               empty_selection_dispatch_catalog.dispatch_count == 0U &&
               empty_selection_dispatch_catalog.error_count == 0U &&
               empty_selection_dispatch_catalog.dry_run &&
               !empty_selection_dispatch_catalog.mutates_asset,
           "#1275: selection builder dispatch catalogs should reject empty builder contexts without mutation");

    const auto visual_launch_surfaces = copperfin::studio::plan_studio_designer_launch_surfaces({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .symbol = "Click",
        .line = 12U,
        .column = 4U
    });
    expect(visual_launch_surfaces.ok,
           "#1211: visual-object launch-surface planning should succeed without executing launch surfaces");
    expect(visual_launch_surfaces.plan.selection_context == StudioEditorSelectionContext::visual_object &&
               visual_launch_surfaces.plan.asset_path == "forms/customer.scx" &&
               visual_launch_surfaces.plan.record_index == 1U &&
               visual_launch_surfaces.plan.object_name == "frmCustomer" &&
               visual_launch_surfaces.plan.unique_id == "form-guid" &&
               visual_launch_surfaces.plan.symbol == "Click" &&
               visual_launch_surfaces.plan.line == 12U &&
               visual_launch_surfaces.plan.column == 4U,
           "#1211: combined launch-surface plans should preserve selected object and editor metadata");
    expect(visual_launch_surfaces.plan.editor_action_launch_plan_count ==
                   visual_launch_surfaces.plan.editor_action_launch_plans.size() &&
               visual_launch_surfaces.plan.builder_launch_plan_count ==
                   visual_launch_surfaces.plan.builder_launch_plans.size(),
           "#1211: combined launch-surface plans should report action and builder launch-plan counts");
    expect(all_action_launch_plans_ok(visual_launch_surfaces.plan.editor_action_launch_plans) &&
               has_action_launch_plan(visual_launch_surfaces.plan.editor_action_launch_plans, "show-property-grid") &&
               has_action_launch_plan(visual_launch_surfaces.plan.editor_action_launch_plans, "edit-visual-method") &&
               has_action_launch_plan(visual_launch_surfaces.plan.editor_action_launch_plans, "show-toolbox"),
           "#1211: visual-object launch surfaces should include valid editor action launch plans");
    expect(all_builder_launch_plans_ok(visual_launch_surfaces.plan.builder_launch_plans) &&
               has_builder_launch_plan(visual_launch_surfaces.plan.builder_launch_plans, "form-builder") &&
               has_builder_launch_plan(visual_launch_surfaces.plan.builder_launch_plans, "control-builder") &&
               has_builder_launch_plan(visual_launch_surfaces.plan.builder_launch_plans, "grid-builder"),
           "#1211: visual-object launch surfaces should include form and control builder launch plans");
    expect(visual_launch_surfaces.plan.toolbox_available &&
               visual_launch_surfaces.plan.toolbox_palette_launch_plan.ok &&
               visual_launch_surfaces.plan.toolbox_item_count ==
                   visual_launch_surfaces.plan.toolbox_palette_launch_plan.plan.items.size() &&
               has_id(visual_launch_surfaces.plan.toolbox_palette_launch_plan.plan.items, "textbox") &&
               has_id(visual_launch_surfaces.plan.toolbox_palette_launch_plan.plan.items, "pageframe"),
           "#1211: visual-object launch surfaces should include form toolbox palette launch data");

    const auto container_launch_surfaces = copperfin::studio::plan_studio_designer_launch_surfaces({
        .selection_context = StudioEditorSelectionContext::container_object,
        .asset_path = "forms/customer.scx",
        .record_index = 3U,
        .object_name = "pgAddress",
        .unique_id = "page-guid",
        .symbol = {},
        .line = 0U,
        .column = 0U
    });
    expect(container_launch_surfaces.ok &&
               has_builder_launch_plan(container_launch_surfaces.plan.builder_launch_plans, "control-builder") &&
               !has_builder_launch_plan(container_launch_surfaces.plan.builder_launch_plans, "form-builder") &&
               container_launch_surfaces.plan.toolbox_available &&
               container_launch_surfaces.plan.toolbox_palette_launch_plan.plan.toolbox_context ==
                   copperfin::studio::StudioToolboxContext::container &&
               has_id(container_launch_surfaces.plan.toolbox_palette_launch_plan.plan.items, "checkbox") &&
               has_id(container_launch_surfaces.plan.toolbox_palette_launch_plan.plan.items, "grid"),
           "#1211: container launch surfaces should preserve container builder and toolbox filtering");

    const auto report_launch_surfaces = copperfin::studio::plan_studio_designer_launch_surfaces({
        .selection_context = StudioEditorSelectionContext::report_expression,
        .asset_path = "reports/invoice.frx",
        .record_index = 7U,
        .object_name = "exprTotal",
        .unique_id = "expr-guid",
        .symbol = {},
        .line = 0U,
        .column = 0U
    });
    expect(report_launch_surfaces.ok &&
               has_action_launch_plan(report_launch_surfaces.plan.editor_action_launch_plans, "edit-report-expression") &&
               has_builder_launch_plan(report_launch_surfaces.plan.builder_launch_plans, "report-builder") &&
               report_launch_surfaces.plan.toolbox_available &&
               report_launch_surfaces.plan.toolbox_palette_launch_plan.plan.toolbox_context ==
                   copperfin::studio::StudioToolboxContext::report &&
               has_id(report_launch_surfaces.plan.toolbox_palette_launch_plan.plan.items, "label") &&
               !has_id(report_launch_surfaces.plan.toolbox_palette_launch_plan.plan.items, "textbox"),
           "#1211: report launch surfaces should preserve expression editor, report builder, and report toolbox filtering");

    const auto method_launch_surfaces = copperfin::studio::plan_studio_designer_launch_surfaces({
        .selection_context = StudioEditorSelectionContext::visual_method,
        .asset_path = "forms/customer.scx",
        .record_index = 2U,
        .object_name = "cmdSave",
        .unique_id = "button-guid",
        .symbol = "Valid",
        .line = 40U,
        .column = 9U
    });
    expect(method_launch_surfaces.ok &&
               method_launch_surfaces.plan.editor_action_launch_plan_count == 1U &&
               has_action_launch_plan(method_launch_surfaces.plan.editor_action_launch_plans, "edit-visual-method") &&
               method_launch_surfaces.plan.editor_action_launch_plans.front().plan.symbol == "Valid" &&
               method_launch_surfaces.plan.editor_action_launch_plans.front().plan.line == 40U &&
               method_launch_surfaces.plan.editor_action_launch_plans.front().plan.column == 9U &&
               has_builder_launch_plan(method_launch_surfaces.plan.builder_launch_plans, "control-builder") &&
               !has_builder_launch_plan(method_launch_surfaces.plan.builder_launch_plans, "form-builder"),
           "#1211: visual-method launch surfaces should preserve code editor metadata and control builder filtering");

    const auto menu_launch_surfaces = copperfin::studio::plan_studio_designer_launch_surfaces({
        .selection_context = StudioEditorSelectionContext::menu_item,
        .asset_path = "menus/main.mnx",
        .record_index = 5U,
        .object_name = "FileExit",
        .unique_id = "menu-guid",
        .symbol = {},
        .line = 0U,
        .column = 0U
    });
    expect(menu_launch_surfaces.ok &&
               has_action_launch_plan(menu_launch_surfaces.plan.editor_action_launch_plans, "show-property-grid") &&
               has_action_launch_plan(menu_launch_surfaces.plan.editor_action_launch_plans, "edit-menu-command") &&
               has_builder_launch_plan(menu_launch_surfaces.plan.builder_launch_plans, "menu-designer") &&
               !menu_launch_surfaces.plan.toolbox_available &&
               menu_launch_surfaces.plan.toolbox_item_count == 0U &&
               !menu_launch_surfaces.plan.toolbox_palette_launch_plan.ok &&
               menu_launch_surfaces.plan.toolbox_error ==
                   "The selected Studio context does not expose a toolbox palette.",
           "#1211: menu launch surfaces should preserve supported actions/builders and explicit toolbox rejection");

    const auto visual_invocation_admission = copperfin::studio::plan_studio_designer_invocation_admission({
        .launch_surface_plan = visual_launch_surfaces.plan,
        .admit_editor_invocations = true,
        .admit_builder_invocations = true,
        .admit_toolbox_invocation = true
    });
    expect(visual_invocation_admission.ok,
           "#1221: aggregate designer invocation admission should accept validated visual launch surfaces");
    expect(visual_invocation_admission.plan.selection_context == StudioEditorSelectionContext::visual_object &&
               visual_invocation_admission.plan.asset_path == "forms/customer.scx" &&
               visual_invocation_admission.plan.record_index == 1U &&
               visual_invocation_admission.plan.object_name == "frmCustomer" &&
               visual_invocation_admission.plan.unique_id == "form-guid" &&
               visual_invocation_admission.plan.symbol == "Click" &&
               visual_invocation_admission.plan.line == 12U &&
               visual_invocation_admission.plan.column == 4U &&
               visual_invocation_admission.plan.editor_action_invocation_count ==
                   visual_launch_surfaces.plan.editor_action_launch_plan_count &&
               visual_invocation_admission.plan.builder_invocation_count ==
                   visual_launch_surfaces.plan.builder_launch_plan_count &&
               visual_invocation_admission.plan.toolbox_available &&
               visual_invocation_admission.plan.toolbox_item_count ==
                   visual_launch_surfaces.plan.toolbox_item_count &&
               !visual_invocation_admission.plan.dry_run &&
               !visual_invocation_admission.plan.mutates_asset,
           "#1221: aggregate designer invocation admission should preserve metadata, counts, and admitted state");
    expect(has_editor_invocation_admission(
               visual_invocation_admission.plan.editor_action_invocations, "edit-visual-method", true) &&
               has_builder_invocation_admission(
                   visual_invocation_admission.plan.builder_invocations, "form-builder", true) &&
               visual_invocation_admission.plan.toolbox_invocation.ok &&
               visual_invocation_admission.plan.toolbox_invocation.plan.palette_invocation_admitted &&
               has_id(visual_invocation_admission.plan.toolbox_invocation.plan.items, "textbox"),
           "#1221: aggregate designer invocation admission should route editor, builder, and toolbox planners");

    const auto menu_invocation_admission = copperfin::studio::plan_studio_designer_invocation_admission({
        .launch_surface_plan = menu_launch_surfaces.plan,
        .admit_editor_invocations = false,
        .admit_builder_invocations = false,
        .admit_toolbox_invocation = false
    });
    expect(menu_invocation_admission.ok &&
               menu_invocation_admission.plan.selection_context == StudioEditorSelectionContext::menu_item &&
               menu_invocation_admission.plan.toolbox_available == false &&
               menu_invocation_admission.plan.toolbox_item_count == 0U &&
               menu_invocation_admission.plan.toolbox_error ==
                   "The selected Studio context does not expose a toolbox palette." &&
               menu_invocation_admission.plan.dry_run &&
               !menu_invocation_admission.plan.mutates_asset &&
               has_editor_invocation_admission(
                   menu_invocation_admission.plan.editor_action_invocations, "show-property-grid", false) &&
               has_editor_invocation_admission(
                   menu_invocation_admission.plan.editor_action_invocations, "edit-menu-command", false) &&
               has_builder_invocation_admission(
                   menu_invocation_admission.plan.builder_invocations, "menu-designer", false),
           "#1221: aggregate designer invocation admission should preserve unsupported-toolbox contexts as dry-runs");

    const auto empty_invocation_admission = copperfin::studio::plan_studio_designer_invocation_admission({
        .launch_surface_plan = {},
        .admit_editor_invocations = true,
        .admit_builder_invocations = true,
        .admit_toolbox_invocation = true
    });
    expect(!empty_invocation_admission.ok &&
               empty_invocation_admission.error ==
                   "A designer invocation admission request requires at least one validated launch surface.",
           "#1221: aggregate designer invocation admission should reject empty launch-surface plans");
    expect(english_catalog.translate("Studio.DesignerInvocationAdmission.Error.ValidatedLaunchSurfaceRequired") ==
               "A designer invocation admission request requires at least one validated launch surface." &&
               pseudo_catalog.translate("Studio.DesignerInvocationAdmission.Error.ValidatedLaunchSurfaceRequired").starts_with("[!! "),
           "#2370: designer invocation admission error prose should resolve through localizable catalog keys");

    const auto visual_dispatch = copperfin::studio::plan_studio_designer_dispatch({
        .invocation_admission_plan = visual_invocation_admission.plan
    });
    const auto expected_visual_dispatch_count =
        visual_invocation_admission.plan.editor_action_invocations.size() +
        visual_invocation_admission.plan.builder_invocations.size() + 1U;
    expect(visual_dispatch.ok &&
               visual_dispatch.plan.selection_context == StudioEditorSelectionContext::visual_object &&
               visual_dispatch.plan.asset_path == "forms/customer.scx" &&
               visual_dispatch.plan.record_index == 1U &&
               visual_dispatch.plan.object_name == "frmCustomer" &&
               visual_dispatch.plan.unique_id == "form-guid" &&
               visual_dispatch.plan.symbol == "Click" &&
               visual_dispatch.plan.line == 12U &&
               visual_dispatch.plan.column == 4U &&
               visual_dispatch.plan.editor_action_dispatch_count ==
                   visual_invocation_admission.plan.editor_action_invocations.size() &&
               visual_dispatch.plan.builder_dispatch_count ==
                   visual_invocation_admission.plan.builder_invocations.size() &&
               visual_dispatch.plan.toolbox_dispatch_count == 1U &&
               visual_dispatch.plan.dispatch_count == expected_visual_dispatch_count &&
               visual_dispatch.plan.error_count == 0U &&
               !visual_dispatch.plan.dry_run &&
               !visual_dispatch.plan.mutates_asset,
           "#1237: aggregate designer dispatch should preserve metadata and admitted dispatch counts");
    expect(has_editor_dispatch(visual_dispatch.plan.editor_action_dispatches, "edit-visual-method", true) &&
               has_builder_dispatch(visual_dispatch.plan.builder_dispatches, "form-builder", true) &&
               visual_dispatch.plan.toolbox_dispatch.ok &&
               visual_dispatch.plan.toolbox_dispatch.plan.dispatch_admitted &&
               has_id(visual_dispatch.plan.toolbox_dispatch.plan.items, "textbox"),
           "#1237: aggregate designer dispatch should route editor, builder, and toolbox dispatch planners");

    const auto menu_dispatch = copperfin::studio::plan_studio_designer_dispatch({
        .invocation_admission_plan = menu_invocation_admission.plan
    });
    expect(menu_dispatch.ok &&
               menu_dispatch.plan.selection_context == StudioEditorSelectionContext::menu_item &&
               menu_dispatch.plan.editor_action_dispatch_count ==
                   menu_invocation_admission.plan.editor_action_invocations.size() &&
               menu_dispatch.plan.builder_dispatch_count ==
                   menu_invocation_admission.plan.builder_invocations.size() &&
               menu_dispatch.plan.toolbox_dispatch_count == 0U &&
               menu_dispatch.plan.dispatch_count == 0U &&
               menu_dispatch.plan.error_count ==
                   menu_dispatch.plan.editor_action_dispatch_count + menu_dispatch.plan.builder_dispatch_count + 1U &&
               menu_dispatch.plan.dry_run &&
               !menu_dispatch.plan.mutates_asset &&
               !menu_dispatch.plan.toolbox_dispatch.ok &&
               menu_dispatch.plan.toolbox_dispatch.error ==
                   "The selected Studio context does not expose a toolbox palette.",
           "#1237: aggregate designer dispatch should preserve dry-run rejections and unsupported toolbox errors");

    const auto empty_dispatch = copperfin::studio::plan_studio_designer_dispatch({
        .invocation_admission_plan = {}
    });
    expect(!empty_dispatch.ok &&
               empty_dispatch.error == "A designer dispatch request requires at least one invocation admission.",
           "#1237: aggregate designer dispatch should reject empty invocation inputs");
    expect(english_catalog.translate("Studio.DesignerDispatch.Error.InvocationAdmissionRequired") ==
               "A designer dispatch request requires at least one invocation admission." &&
               english_catalog.translate("Studio.DesignerDispatch.Execution.Error.ExecutionAdmissionRequired") ==
                   "A designer dispatch execution request requires explicit execution admission." &&
               english_catalog.translate("Studio.DesignerDispatch.Execution.Error.BuilderExecutorRequired") ==
                   "A designer dispatch execution request requires a builder executor." &&
               english_catalog.translate("Studio.DesignerDispatch.CatalogEntry.Error.ErrorFreeDispatchRequired") ==
                   "A designer dispatch execution catalog entry requires an error-free dispatch plan." &&
               english_catalog.translate("Studio.DesignerDispatch.CatalogEntry.Error.AdmittedDispatchRequired") ==
                   "A designer dispatch execution catalog entry requires at least one admitted dispatch." &&
               pseudo_catalog.translate("Studio.DesignerDispatch.Error.InvocationAdmissionRequired").starts_with("[!! ") &&
               pseudo_catalog.translate("Studio.DesignerDispatch.CatalogEntry.Error.ExecutionAdmissionRequired").starts_with("[!! "),
           "#2371: designer dispatch error prose should resolve through localizable catalog keys");
    const std::vector<std::string_view> designer_gate_error_keys = {
        "Studio.DesignerInvocationAdmission.Error.ValidatedLaunchSurfaceRequired",
        "Studio.DesignerDispatch.Error.InvocationAdmissionRequired"};
    expect(
        spanish_catalog.translate("Studio.DesignerInvocationAdmission.Error.ValidatedLaunchSurfaceRequired") ==
            "Una solicitud de admision de invocacion de disenador requiere al menos una superficie de lanzamiento validada.",
        "#2644: es-419 designer invocation-admission gate error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.DesignerDispatch.Error.InvocationAdmissionRequired") ==
            "Una solicitud de dispatch de disenador requiere al menos una admision de invocacion.",
        "#2644: es-419 designer dispatch gate error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.DesignerInvocationAdmission.Error.ValidatedLaunchSurfaceRequired") ==
            "Uma solicitacao de admissao de invocacao de designer exige pelo menos uma superficie de inicializacao validada.",
        "#2644: pt-BR designer invocation-admission gate error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.DesignerDispatch.Error.InvocationAdmissionRequired") ==
            "Uma solicitacao de dispatch de designer exige pelo menos uma admissao de invocacao.",
        "#2644: pt-BR designer dispatch gate error should localize through the catalog");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", designer_gate_error_keys) == 0U,
        "#2644: es-419 should define every remaining designer gate-error localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", designer_gate_error_keys) == 0U,
        "#2644: pt-BR should define every remaining designer gate-error localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", designer_gate_error_keys) == 0U,
        "#2644: qps-ploc should define every remaining designer gate-error localization key");
    const std::vector<std::string_view> catalog_entry_keys = {
        "Studio.DesignerDispatch.CatalogEntry.Error.AdmittedDispatchRequired",
        "Studio.DesignerDispatch.CatalogEntry.Error.ErrorFreeDispatchRequired",
        "Studio.DesignerDispatch.CatalogEntry.Error.ExecutionAdmissionRequired",
        "Studio.DesignerDispatch.CatalogEntry.Error.NonDryRunDispatchRequired",
        "Studio.DesignerDispatch.CatalogEntry.Error.NonExecutedBuilderDispatchesRequired",
        "Studio.DesignerDispatch.CatalogEntry.Error.NonExecutedEditorDispatchesRequired",
        "Studio.DesignerDispatch.CatalogEntry.Error.NonExecutedToolboxDispatchRequired"};
    expect(
        spanish_catalog.translate("Studio.DesignerDispatch.CatalogEntry.Error.ExecutionAdmissionRequired") ==
            "Una entrada de catalogo de ejecucion de dispatch de disenador requiere admision explicita de ejecucion.",
        "#2616: es-419 designer dispatch catalog-entry execution-admission error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.DesignerDispatch.CatalogEntry.Error.NonExecutedToolboxDispatchRequired") ==
            "Una entrada de catalogo de ejecucion de dispatch de disenador requiere un dispatch de caja de herramientas no ejecutado.",
        "#2616: es-419 designer dispatch catalog-entry toolbox-dispatch error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.DesignerDispatch.CatalogEntry.Error.ErrorFreeDispatchRequired") ==
            "Uma entrada de catalogo de execucao de dispatch de designer exige um plano de dispatch sem erros.",
        "#2616: pt-BR designer dispatch catalog-entry error-free-dispatch error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.DesignerDispatch.CatalogEntry.Error.NonExecutedBuilderDispatchesRequired") ==
            "Uma entrada de catalogo de execucao de dispatch de designer exige dispatches de builder nao executados.",
        "#2616: pt-BR designer dispatch catalog-entry builder-dispatch error should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.DesignerDispatch.CatalogEntry.Error.NonDryRunDispatchRequired") ==
            copperfin::localization::pseudo_localize(
                "A designer dispatch execution catalog entry requires a non-dry-run dispatch plan."),
        "#2616: qps-ploc designer dispatch catalog-entry non-dry-run error should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", catalog_entry_keys) == 0U,
        "#2616: es-419 should define every remaining Studio.DesignerDispatch.CatalogEntry localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", catalog_entry_keys) == 0U,
        "#2616: pt-BR should define every remaining Studio.DesignerDispatch.CatalogEntry localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", catalog_entry_keys) == 0U,
        "#2616: qps-ploc should define every remaining Studio.DesignerDispatch.CatalogEntry localization key");
    const std::vector<std::string_view> execution_keys = {
        "Studio.DesignerDispatch.Execution.Error.AdmittedDispatchRequired",
        "Studio.DesignerDispatch.Execution.Error.BuilderExecutorRequired",
        "Studio.DesignerDispatch.Execution.Error.EditorExecutorRequired",
        "Studio.DesignerDispatch.Execution.Error.ErrorFreeDispatchRequired",
        "Studio.DesignerDispatch.Execution.Error.ExecutionAdmissionRequired",
        "Studio.DesignerDispatch.Execution.Error.NonDryRunDispatchRequired",
        "Studio.DesignerDispatch.Execution.Error.ToolboxExecutorRequired"};
    expect(
        spanish_catalog.translate("Studio.DesignerDispatch.Execution.Error.ExecutionAdmissionRequired") ==
            "Una solicitud de ejecucion de dispatch de disenador requiere admision explicita de ejecucion.",
        "#2617: es-419 designer dispatch execution-admission error should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.DesignerDispatch.Execution.Error.ToolboxExecutorRequired") ==
            "Una solicitud de ejecucion de dispatch de disenador requiere un ejecutor de caja de herramientas.",
        "#2617: es-419 designer dispatch toolbox-executor error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.DesignerDispatch.Execution.Error.ErrorFreeDispatchRequired") ==
            "Uma solicitacao de execucao de dispatch de designer exige um plano de dispatch sem erros.",
        "#2617: pt-BR designer dispatch error-free-dispatch error should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.DesignerDispatch.Execution.Error.EditorExecutorRequired") ==
            "Uma solicitacao de execucao de dispatch de designer exige um executor de acoes do editor.",
        "#2617: pt-BR designer dispatch editor-executor error should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.DesignerDispatch.Execution.Error.NonDryRunDispatchRequired") ==
            copperfin::localization::pseudo_localize(
                "A designer dispatch execution request requires a non-dry-run dispatch plan."),
        "#2617: qps-ploc designer dispatch non-dry-run execution error should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", execution_keys) == 0U,
        "#2617: es-419 should define every remaining Studio.DesignerDispatch.Execution localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", execution_keys) == 0U,
        "#2617: pt-BR should define every remaining Studio.DesignerDispatch.Execution localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", execution_keys) == 0U,
        "#2617: qps-ploc should define every remaining Studio.DesignerDispatch.Execution localization key");

    std::size_t editor_execution_calls = 0U;
    std::size_t builder_execution_calls = 0U;
    std::size_t toolbox_execution_calls = 0U;
    const auto executed_visual_dispatch = copperfin::studio::execute_studio_designer_dispatch({
        .dispatch_plan = visual_dispatch.plan,
        .admit_execution = true,
        .editor_action_executor = [&](const copperfin::studio::StudioEditorActionDispatchPlan& plan) {
            ++editor_execution_calls;
            expect(plan.selection_context == StudioEditorSelectionContext::visual_object &&
                       plan.dispatch_admitted &&
                       !plan.dry_run &&
                       !plan.executed,
                   "#1324: designer dispatch execution should invoke editor executors with admitted dispatches");
            return copperfin::studio::StudioEditorActionDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = "editor action executed",
                .error = {},
                .mutates_asset = false
            };
        },
        .builder_executor = [&](const copperfin::studio::StudioBuilderDispatchPlan& plan) {
            ++builder_execution_calls;
            expect(plan.dispatch_admitted &&
                       !plan.dry_run &&
                       !plan.executed &&
                       plan.command_token == "studio.builder.invoke",
                   "#1324: designer dispatch execution should invoke builder executors with admitted dispatches");
            return copperfin::studio::StudioBuilderDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = "builder executed",
                .error = {},
                .mutates_asset = true
            };
        },
        .toolbox_executor = [&](const copperfin::studio::StudioToolboxDispatchPlan& plan) {
            ++toolbox_execution_calls;
            expect(plan.dispatch_admitted &&
                       !plan.dry_run &&
                       !plan.executed &&
                       plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
                       has_id(plan.items, "textbox"),
                   "#1324: designer dispatch execution should invoke toolbox executors with admitted dispatches");
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = "toolbox executed",
                .error = {},
                .mutates_asset = false
            };
        }
    });
    expect(executed_visual_dispatch.ok &&
               executed_visual_dispatch.execution_admitted &&
               executed_visual_dispatch.executed &&
               !executed_visual_dispatch.dry_run &&
               executed_visual_dispatch.mutates_asset &&
               executed_visual_dispatch.execution_count == expected_visual_dispatch_count &&
               executed_visual_dispatch.error_count == 0U &&
               editor_execution_calls == visual_dispatch.plan.editor_action_dispatch_count &&
               builder_execution_calls == visual_dispatch.plan.builder_dispatch_count &&
               toolbox_execution_calls == 1U &&
               executed_visual_dispatch.editor_action_executions.size() ==
                   visual_dispatch.plan.editor_action_dispatch_count &&
               executed_visual_dispatch.builder_executions.size() ==
                   visual_dispatch.plan.builder_dispatch_count &&
               executed_visual_dispatch.toolbox_execution.ok &&
               executed_visual_dispatch.dispatch_plan.dispatch_count == visual_dispatch.plan.dispatch_count &&
               executed_visual_dispatch.dispatch_plan.editor_action_dispatches.front().plan.executed &&
               executed_visual_dispatch.dispatch_plan.builder_dispatches.front().plan.executed &&
               executed_visual_dispatch.dispatch_plan.toolbox_dispatch.plan.executed,
           "#1324: designer dispatch execution should preserve aggregate metadata and executed child state");

    editor_execution_calls = 0U;
    builder_execution_calls = 0U;
    toolbox_execution_calls = 0U;
    const auto unadmitted_designer_execution = copperfin::studio::execute_studio_designer_dispatch({
        .dispatch_plan = visual_dispatch.plan,
        .admit_execution = false,
        .editor_action_executor = [&](const copperfin::studio::StudioEditorActionDispatchPlan&) {
            ++editor_execution_calls;
            return copperfin::studio::StudioEditorActionDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        },
        .builder_executor = [&](const copperfin::studio::StudioBuilderDispatchPlan&) {
            ++builder_execution_calls;
            return copperfin::studio::StudioBuilderDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        },
        .toolbox_executor = [&](const copperfin::studio::StudioToolboxDispatchPlan&) {
            ++toolbox_execution_calls;
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        }
    });
    expect(editor_execution_calls == 0U &&
               builder_execution_calls == 0U &&
               toolbox_execution_calls == 0U &&
               !unadmitted_designer_execution.ok &&
               unadmitted_designer_execution.error ==
                   "A designer dispatch execution request requires explicit execution admission.",
           "#1324: designer dispatch execution should reject unadmitted execution before child executors");

    auto errorful_dispatch_plan = visual_dispatch.plan;
    errorful_dispatch_plan.error_count = 1U;
    const auto errorful_execution = copperfin::studio::execute_studio_designer_dispatch({
        .dispatch_plan = errorful_dispatch_plan,
        .admit_execution = true,
        .editor_action_executor = [&](const copperfin::studio::StudioEditorActionDispatchPlan&) {
            ++editor_execution_calls;
            return copperfin::studio::StudioEditorActionDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        },
        .builder_executor = [&](const copperfin::studio::StudioBuilderDispatchPlan&) {
            ++builder_execution_calls;
            return copperfin::studio::StudioBuilderDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        },
        .toolbox_executor = [&](const copperfin::studio::StudioToolboxDispatchPlan&) {
            ++toolbox_execution_calls;
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        }
    });
    expect(editor_execution_calls == 0U &&
               builder_execution_calls == 0U &&
               toolbox_execution_calls == 0U &&
               !errorful_execution.ok &&
               errorful_execution.error == "A designer dispatch execution request requires an error-free dispatch plan.",
           "#1324: designer dispatch execution should reject aggregate dispatch errors before partial execution");

    const auto missing_builder_executor_execution = copperfin::studio::execute_studio_designer_dispatch({
        .dispatch_plan = visual_dispatch.plan,
        .admit_execution = true,
        .editor_action_executor = [](const copperfin::studio::StudioEditorActionDispatchPlan&) {
            return copperfin::studio::StudioEditorActionDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        },
        .builder_executor = {},
        .toolbox_executor = [](const copperfin::studio::StudioToolboxDispatchPlan&) {
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        }
    });
    expect(!missing_builder_executor_execution.ok &&
               missing_builder_executor_execution.error ==
                   "A designer dispatch execution request requires a builder executor.",
           "#1324: designer dispatch execution should preflight required child executors before launch");

    builder_execution_calls = 0U;
    const auto child_failure_execution = copperfin::studio::execute_studio_designer_dispatch({
        .dispatch_plan = visual_dispatch.plan,
        .admit_execution = true,
        .editor_action_executor = [](const copperfin::studio::StudioEditorActionDispatchPlan&) {
            return copperfin::studio::StudioEditorActionDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        },
        .builder_executor = [&](const copperfin::studio::StudioBuilderDispatchPlan&) {
            ++builder_execution_calls;
            return copperfin::studio::StudioBuilderDispatchExecutionObservation{
                .launched = true,
                .exit_code = 9,
                .output = {},
                .error = "builder failed",
                .mutates_asset = false
            };
        },
        .toolbox_executor = [](const copperfin::studio::StudioToolboxDispatchPlan&) {
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        }
    });
    expect(child_failure_execution.ok &&
               !child_failure_execution.executed &&
               child_failure_execution.dry_run &&
               child_failure_execution.dispatch_plan.dispatch_count == 0U &&
               child_failure_execution.error_count == builder_execution_calls &&
               child_failure_execution.execution_count ==
                   expected_visual_dispatch_count - builder_execution_calls &&
               !child_failure_execution.builder_executions.empty() &&
               !child_failure_execution.builder_executions.front().ok &&
               child_failure_execution.builder_executions.front().error == "builder failed",
           "#1324: designer dispatch execution should summarize child failures without stale aggregate metadata");

    builder_execution_calls = 0U;
    const auto partial_mutation_before_failure_execution = copperfin::studio::execute_studio_designer_dispatch({
        .dispatch_plan = visual_dispatch.plan,
        .admit_execution = true,
        .editor_action_executor = [](const copperfin::studio::StudioEditorActionDispatchPlan&) {
            return copperfin::studio::StudioEditorActionDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = true
            };
        },
        .builder_executor = [&](const copperfin::studio::StudioBuilderDispatchPlan&) {
            ++builder_execution_calls;
            return copperfin::studio::StudioBuilderDispatchExecutionObservation{
                .launched = true,
                .exit_code = 9,
                .output = {},
                .error = "builder failed",
                .mutates_asset = false
            };
        },
        .toolbox_executor = [](const copperfin::studio::StudioToolboxDispatchPlan&) {
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        }
    });
    expect(partial_mutation_before_failure_execution.ok &&
               !partial_mutation_before_failure_execution.executed &&
               partial_mutation_before_failure_execution.mutates_asset,
           "designer dispatch execution should report real mutations from steps that succeeded before a later step failed");

    const auto invocation_catalog = copperfin::studio::plan_studio_designer_invocation_admission_catalog({
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .symbol = "Click",
        .line = 12U,
        .column = 4U,
        .admit_editor_invocations = true,
        .admit_builder_invocations = false,
        .admit_toolbox_invocation = true
    });
    expect(invocation_catalog.ok && invocation_catalog.context_count == invocation_catalog.contexts.size(),
           "#1223: designer invocation admission catalog should include every known Studio context");

    const auto* visual_invocation_entry = find_invocation_catalog_entry(
        invocation_catalog.contexts, StudioEditorSelectionContext::visual_object);
    expect(visual_invocation_entry != nullptr &&
               visual_invocation_entry->invocation_admission.ok &&
               visual_invocation_entry->editor_action_invocation_count > 0U &&
               visual_invocation_entry->builder_invocation_count > 0U &&
               visual_invocation_entry->toolbox_available &&
               visual_invocation_entry->toolbox_item_count > 0U &&
               !visual_invocation_entry->dry_run &&
               !visual_invocation_entry->mutates_asset &&
               has_editor_invocation_admission(
                   visual_invocation_entry->invocation_admission.plan.editor_action_invocations,
                   "edit-visual-method",
                   true) &&
               has_builder_invocation_admission(
                   visual_invocation_entry->invocation_admission.plan.builder_invocations,
                   "form-builder",
                   false) &&
               visual_invocation_entry->invocation_admission.plan.toolbox_invocation.ok &&
               visual_invocation_entry->invocation_admission.plan.toolbox_invocation.plan.palette_invocation_admitted,
           "#1223: designer invocation admission catalog should summarize visual admissions and requested policies");

    const auto* report_invocation_entry = find_invocation_catalog_entry(
        invocation_catalog.contexts, StudioEditorSelectionContext::report_expression);
    expect(report_invocation_entry != nullptr &&
               report_invocation_entry->invocation_admission.ok &&
               report_invocation_entry->toolbox_available &&
               report_invocation_entry->invocation_admission.plan.toolbox_invocation.ok &&
               report_invocation_entry->invocation_admission.plan.toolbox_invocation.plan.toolbox_context ==
                   copperfin::studio::StudioToolboxContext::report &&
               has_editor_invocation_admission(
                   report_invocation_entry->invocation_admission.plan.editor_action_invocations,
                   "edit-report-expression",
                   true) &&
               has_builder_invocation_admission(
                   report_invocation_entry->invocation_admission.plan.builder_invocations,
                   "report-builder",
                   false),
           "#1223: designer invocation admission catalog should preserve report editor, builder, and toolbox admissions");

    const auto* menu_invocation_entry = find_invocation_catalog_entry(
        invocation_catalog.contexts, StudioEditorSelectionContext::menu_item);
    expect(menu_invocation_entry != nullptr &&
               menu_invocation_entry->invocation_admission.ok &&
               !menu_invocation_entry->toolbox_available &&
               menu_invocation_entry->toolbox_item_count == 0U &&
               menu_invocation_entry->toolbox_error ==
                   "The selected Studio context does not expose a toolbox palette." &&
               !menu_invocation_entry->invocation_admission.plan.toolbox_invocation.ok &&
               has_editor_invocation_admission(
                   menu_invocation_entry->invocation_admission.plan.editor_action_invocations,
                   "show-property-grid",
                   true) &&
               has_editor_invocation_admission(
                   menu_invocation_entry->invocation_admission.plan.editor_action_invocations,
                   "edit-menu-command",
                   true) &&
               has_builder_invocation_admission(
                   menu_invocation_entry->invocation_admission.plan.builder_invocations,
                   "menu-designer",
                   false),
           "#1223: designer invocation admission catalog should preserve unsupported-toolbox menu contexts");

    const auto* data_invocation_entry = find_invocation_catalog_entry(
        invocation_catalog.contexts, StudioEditorSelectionContext::data_environment);
    expect(data_invocation_entry != nullptr &&
               data_invocation_entry->invocation_admission.ok &&
               !data_invocation_entry->toolbox_available &&
               has_editor_invocation_admission(
                   data_invocation_entry->invocation_admission.plan.editor_action_invocations,
                   "show-property-grid",
                   true) &&
               has_editor_invocation_admission(
                   data_invocation_entry->invocation_admission.plan.editor_action_invocations,
                   "edit-data-environment",
                   true),
           "#1410: designer invocation admission catalog should include data-environment property/editor admissions");

    const auto dispatch_catalog = copperfin::studio::plan_studio_designer_dispatch_catalog({
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .symbol = "Click",
        .line = 12U,
        .column = 4U,
        .admit_editor_invocations = true,
        .admit_builder_invocations = false,
        .admit_toolbox_invocation = true
    });
    expect(dispatch_catalog.ok && dispatch_catalog.context_count == dispatch_catalog.contexts.size(),
           "#1239: designer dispatch catalog should include every known Studio context");

    const auto* visual_dispatch_entry = find_dispatch_catalog_entry(
        dispatch_catalog.contexts, StudioEditorSelectionContext::visual_object);
    expect(visual_dispatch_entry != nullptr &&
               visual_dispatch_entry->dispatch.ok &&
               visual_dispatch_entry->editor_action_dispatch_count > 0U &&
               visual_dispatch_entry->builder_dispatch_count > 0U &&
               visual_dispatch_entry->toolbox_dispatch_count == 1U &&
               visual_dispatch_entry->dispatch_count ==
                   visual_dispatch_entry->editor_action_dispatch_count + visual_dispatch_entry->toolbox_dispatch_count &&
               visual_dispatch_entry->error_count == visual_dispatch_entry->builder_dispatch_count &&
               !visual_dispatch_entry->dry_run &&
               !visual_dispatch_entry->mutates_asset &&
               has_editor_dispatch(
                   visual_dispatch_entry->dispatch.plan.editor_action_dispatches, "edit-visual-method", true) &&
               visual_dispatch_entry->dispatch.plan.toolbox_dispatch.ok &&
               visual_dispatch_entry->dispatch.plan.toolbox_dispatch.plan.dispatch_admitted,
           "#1239: designer dispatch catalog should summarize visual dispatches and requested policies");
    expect(visual_invocation_entry != nullptr &&
               visual_dispatch_entry != nullptr &&
               visual_dispatch_entry->dispatch.ok &&
               visual_dispatch_entry->dispatch.plan.selection_context ==
                   visual_invocation_entry->invocation_admission.plan.selection_context &&
               visual_dispatch_entry->dispatch.plan.editor_action_dispatch_count ==
                   visual_invocation_entry->editor_action_invocation_count &&
               visual_dispatch_entry->dispatch.plan.builder_dispatch_count ==
                   visual_invocation_entry->builder_invocation_count &&
               visual_dispatch_entry->dispatch.plan.toolbox_dispatch_count ==
                   (visual_invocation_entry->toolbox_available ? 1U : 0U) &&
               visual_dispatch_entry->dispatch.plan.asset_path ==
                   visual_invocation_entry->invocation_admission.plan.asset_path &&
               visual_dispatch_entry->dispatch.plan.object_name ==
                   visual_invocation_entry->invocation_admission.plan.object_name &&
               visual_dispatch_entry->dispatch.plan.symbol ==
                   visual_invocation_entry->invocation_admission.plan.symbol,
           "#1284: designer dispatch catalogs should preserve shared invocation-admission catalog metadata");

    const auto* report_dispatch_entry = find_dispatch_catalog_entry(
        dispatch_catalog.contexts, StudioEditorSelectionContext::report_expression);
    expect(report_dispatch_entry != nullptr &&
               report_dispatch_entry->dispatch.ok &&
               report_dispatch_entry->toolbox_dispatch_count == 1U &&
               report_dispatch_entry->dispatch.plan.toolbox_dispatch.ok &&
               report_dispatch_entry->dispatch.plan.toolbox_dispatch.plan.toolbox_context ==
                   copperfin::studio::StudioToolboxContext::report &&
               has_editor_dispatch(
                   report_dispatch_entry->dispatch.plan.editor_action_dispatches,
                   "edit-report-expression",
                   true),
           "#1239: designer dispatch catalog should preserve report editor and toolbox dispatches");

    const auto* menu_dispatch_entry = find_dispatch_catalog_entry(
        dispatch_catalog.contexts, StudioEditorSelectionContext::menu_item);
    expect(menu_dispatch_entry != nullptr &&
               menu_dispatch_entry->dispatch.ok &&
               menu_dispatch_entry->toolbox_dispatch_count == 0U &&
               menu_dispatch_entry->dispatch_count == menu_dispatch_entry->editor_action_dispatch_count &&
               menu_dispatch_entry->error_count == menu_dispatch_entry->builder_dispatch_count + 1U &&
               !menu_dispatch_entry->dry_run &&
               !menu_dispatch_entry->mutates_asset &&
               !menu_dispatch_entry->dispatch.plan.toolbox_dispatch.ok &&
               menu_dispatch_entry->dispatch.plan.toolbox_dispatch.error ==
                   "The selected Studio context does not expose a toolbox palette." &&
               has_editor_dispatch(
                   menu_dispatch_entry->dispatch.plan.editor_action_dispatches, "show-property-grid", true) &&
               has_editor_dispatch(
                   menu_dispatch_entry->dispatch.plan.editor_action_dispatches, "edit-menu-command", true),
           "#1239: designer dispatch catalog should summarize menu dispatches and unsupported toolbox errors");
    expect(menu_invocation_entry != nullptr &&
               menu_dispatch_entry != nullptr &&
               menu_dispatch_entry->dispatch.ok &&
               menu_dispatch_entry->dispatch.plan.editor_action_dispatch_count ==
                   menu_invocation_entry->editor_action_invocation_count &&
               menu_dispatch_entry->dispatch.plan.builder_dispatch_count ==
                   menu_invocation_entry->builder_invocation_count &&
               menu_dispatch_entry->dispatch.plan.toolbox_dispatch_count == 0U &&
               !menu_dispatch_entry->dispatch.plan.toolbox_dispatch.ok &&
               menu_dispatch_entry->dispatch.plan.toolbox_dispatch.error ==
                   menu_invocation_entry->toolbox_error,
           "#1284: designer dispatch catalogs should retain unsupported-toolbox admission metadata");

    const auto execution_catalog = copperfin::studio::plan_studio_designer_dispatch_execution_catalog({
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .symbol = "Click",
        .line = 12U,
        .column = 4U,
        .admit_editor_invocations = true,
        .admit_builder_invocations = true,
        .admit_toolbox_invocation = true,
        .admit_execution = true
    });
    const auto* visual_execution_entry = find_dispatch_execution_catalog_entry(
        execution_catalog.contexts, StudioEditorSelectionContext::visual_object);
    const auto* menu_execution_entry = find_dispatch_execution_catalog_entry(
        execution_catalog.contexts, StudioEditorSelectionContext::menu_item);
    expect(execution_catalog.ok &&
               execution_catalog.context_count == execution_catalog.contexts.size() &&
               execution_catalog.execution_ready_count > 0U &&
               execution_catalog.error_count > 0U &&
               !execution_catalog.dry_run &&
               !execution_catalog.mutates_asset,
           "#1332: designer dispatch execution catalogs should summarize non-mutating aggregate readiness");
    expect(visual_execution_entry != nullptr &&
               visual_execution_entry->dispatch.ok &&
               visual_execution_entry->execution_admitted &&
               visual_execution_entry->execution_ready &&
               visual_execution_entry->execution_error.empty() &&
               visual_execution_entry->editor_action_dispatch_count > 0U &&
               visual_execution_entry->builder_dispatch_count > 0U &&
               visual_execution_entry->toolbox_dispatch_count == 1U &&
               visual_execution_entry->dispatch_count ==
                   visual_execution_entry->editor_action_dispatch_count +
                       visual_execution_entry->builder_dispatch_count +
                       visual_execution_entry->toolbox_dispatch_count &&
               visual_execution_entry->dispatch_error_count == 0U &&
               !visual_execution_entry->dispatch_dry_run &&
               !visual_execution_entry->dispatch_mutates_asset &&
               has_editor_dispatch(
                   visual_execution_entry->dispatch.plan.editor_action_dispatches,
                   "edit-visual-method",
                   true) &&
               has_builder_dispatch(visual_execution_entry->dispatch.plan.builder_dispatches, "form-builder", true) &&
               visual_execution_entry->dispatch.plan.toolbox_dispatch.ok &&
               visual_execution_entry->dispatch.plan.toolbox_dispatch.plan.dispatch_admitted &&
               !visual_execution_entry->dispatch.plan.toolbox_dispatch.plan.executed,
           "#1332: designer dispatch execution catalogs should preserve visual aggregate dispatch metadata");
    expect(menu_execution_entry != nullptr &&
               !menu_execution_entry->execution_ready &&
               menu_execution_entry->execution_error ==
                   "A designer dispatch execution catalog entry requires an error-free dispatch plan." &&
               menu_execution_entry->dispatch_error_count > 0U &&
               menu_execution_entry->dispatch.ok &&
               !menu_execution_entry->dispatch.plan.toolbox_dispatch.ok,
           "#1332: designer dispatch execution catalogs should propagate aggregate dispatch errors");

    const auto unadmitted_execution_catalog =
        copperfin::studio::plan_studio_designer_dispatch_execution_catalog({
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid",
            .symbol = "Click",
            .line = 12U,
            .column = 4U,
            .admit_editor_invocations = true,
            .admit_builder_invocations = true,
            .admit_toolbox_invocation = true,
            .admit_execution = false
        });
    const auto* unadmitted_visual_execution_entry = find_dispatch_execution_catalog_entry(
        unadmitted_execution_catalog.contexts, StudioEditorSelectionContext::visual_object);
    expect(unadmitted_execution_catalog.ok &&
               unadmitted_execution_catalog.execution_ready_count == 0U &&
               unadmitted_execution_catalog.error_count == unadmitted_execution_catalog.context_count &&
               unadmitted_execution_catalog.dry_run &&
               unadmitted_visual_execution_entry != nullptr &&
               !unadmitted_visual_execution_entry->execution_admitted &&
               !unadmitted_visual_execution_entry->execution_ready &&
               unadmitted_visual_execution_entry->execution_error ==
                   "A designer dispatch execution catalog entry requires explicit execution admission.",
           "#1332: designer dispatch execution catalogs should require explicit aggregate execution admission");

    const auto dry_run_execution_catalog =
        copperfin::studio::plan_studio_designer_dispatch_execution_catalog({
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "frmCustomer",
            .unique_id = "form-guid",
            .symbol = "Click",
            .line = 12U,
            .column = 4U,
            .admit_editor_invocations = false,
            .admit_builder_invocations = false,
            .admit_toolbox_invocation = false,
            .admit_execution = true
        });
    const auto* dry_run_visual_execution_entry = find_dispatch_execution_catalog_entry(
        dry_run_execution_catalog.contexts, StudioEditorSelectionContext::visual_object);
    expect(dry_run_execution_catalog.ok &&
               dry_run_execution_catalog.execution_ready_count == 0U &&
               dry_run_execution_catalog.error_count == dry_run_execution_catalog.context_count &&
               dry_run_execution_catalog.dry_run &&
               dry_run_visual_execution_entry != nullptr &&
               !dry_run_visual_execution_entry->execution_ready &&
               dry_run_visual_execution_entry->execution_error ==
                   "A designer dispatch execution catalog entry requires at least one admitted dispatch.",
           "#1332: designer dispatch execution catalogs should report dry-run aggregate preflight failures");

    const auto launch_surface_catalog = copperfin::studio::plan_studio_designer_launch_surface_catalog({
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid",
        .symbol = "Click",
        .line = 12U,
        .column = 4U
    });
    expect(launch_surface_catalog.ok && launch_surface_catalog.context_count == 9U &&
               launch_surface_catalog.context_count == launch_surface_catalog.contexts.size(),
           "#1213: launch-surface catalog should include every supported Studio selection context");

    const auto* visual_catalog = find_catalog_entry(
        launch_surface_catalog.contexts,
        StudioEditorSelectionContext::visual_object);
    expect(visual_catalog != nullptr && visual_catalog->launch_surface_plan.ok &&
               visual_catalog->editor_action_launch_plan_count >= 3U &&
               visual_catalog->builder_launch_plan_count == 3U &&
               visual_catalog->toolbox_available &&
               visual_catalog->toolbox_item_count ==
                   visual_catalog->launch_surface_plan.plan.toolbox_palette_launch_plan.plan.items.size() &&
               visual_catalog->launch_surface_plan.plan.asset_path == "forms/customer.scx" &&
               visual_catalog->launch_surface_plan.plan.object_name == "frmCustomer" &&
               visual_catalog->launch_surface_plan.plan.symbol == "Click",
           "#1213: visual-object catalog entries should summarize and preserve nested launch-surface metadata");

    const auto* container_catalog = find_catalog_entry(
        launch_surface_catalog.contexts,
        StudioEditorSelectionContext::container_object);
    expect(container_catalog != nullptr && container_catalog->launch_surface_plan.ok &&
               container_catalog->builder_launch_plan_count == 2U &&
               container_catalog->toolbox_available &&
               container_catalog->launch_surface_plan.plan.toolbox_palette_launch_plan.plan.toolbox_context ==
                   copperfin::studio::StudioToolboxContext::container,
           "#1213: container catalog entries should summarize container builder and toolbox availability");

    const auto* report_catalog = find_catalog_entry(
        launch_surface_catalog.contexts,
        StudioEditorSelectionContext::report_expression);
    expect(report_catalog != nullptr && report_catalog->launch_surface_plan.ok &&
               report_catalog->editor_action_launch_plan_count >= 2U &&
               report_catalog->builder_launch_plan_count == 1U &&
               report_catalog->toolbox_available &&
               report_catalog->launch_surface_plan.plan.toolbox_palette_launch_plan.plan.toolbox_context ==
                   copperfin::studio::StudioToolboxContext::report,
           "#1213: report catalog entries should summarize report action, builder, and toolbox availability");

    const auto* menu_catalog = find_catalog_entry(
        launch_surface_catalog.contexts,
        StudioEditorSelectionContext::menu_item);
    expect(menu_catalog != nullptr && menu_catalog->launch_surface_plan.ok &&
               menu_catalog->editor_action_launch_plan_count == 3U &&
               menu_catalog->builder_launch_plan_count == 1U &&
               !menu_catalog->toolbox_available &&
               menu_catalog->toolbox_item_count == 0U &&
               menu_catalog->toolbox_error ==
                   "The selected Studio context does not expose a toolbox palette.",
           "#1213: menu catalog entries should preserve unsupported-toolbox reasons");

    const auto* project_catalog = find_catalog_entry(
        launch_surface_catalog.contexts,
        StudioEditorSelectionContext::project_item);
    expect(project_catalog != nullptr && project_catalog->launch_surface_plan.ok &&
               project_catalog->editor_action_launch_plan_count == 3U &&
               project_catalog->builder_launch_plan_count == 1U &&
               !project_catalog->toolbox_available &&
               project_catalog->toolbox_item_count == 0U,
           "#1213: project catalog entries should summarize project actions/builders without toolbox availability");

    const auto* data_catalog = find_catalog_entry(
        launch_surface_catalog.contexts,
        StudioEditorSelectionContext::data_environment);
    expect(data_catalog != nullptr && data_catalog->launch_surface_plan.ok &&
               data_catalog->editor_action_launch_plan_count == 3U &&
               data_catalog->builder_launch_plan_count == 1U &&
               !data_catalog->toolbox_available &&
               data_catalog->toolbox_item_count == 0U,
           "#1410: data-environment catalog entries should summarize property-grid and data-environment actions without toolbox availability");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
