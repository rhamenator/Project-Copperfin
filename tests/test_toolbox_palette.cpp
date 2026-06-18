#include "copperfin/studio/toolbox_palette.h"
#include "copperfin/studio/toolbox_dispatch.h"
#include "copperfin/studio/toolbox_invocation_admission.h"

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

bool has_toolbox_item(
    const std::vector<copperfin::studio::StudioToolboxItemDescriptor>& items,
    std::string_view id) {
    for (const auto& item : items) {
        if (item.id == id) {
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

}  // namespace

int main() {
    using copperfin::studio::StudioEditorSelectionContext;
    using copperfin::studio::StudioToolboxContext;

    const auto& items = copperfin::studio::studio_toolbox_palette();
    expect(items.size() >= 12U, "#957: toolbox palette should expose common VFP visual controls");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::form)) == "form",
           "#957: form toolbox context token should be stable");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::class_designer)) ==
               "class_designer",
           "#957: class-designer toolbox context token should be stable");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::container)) ==
               "container",
           "#957: container toolbox context token should be stable");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::report)) == "report",
           "#957: report toolbox context token should be stable");

    bool found_container = false;
    bool found_report_item = false;
    bool found_data_control = false;

    for (const auto& item : items) {
        expect(!std::string(item.id).empty(), "#957: each toolbox item should have an id");
        expect(!std::string(item.title).empty(), "#957: each toolbox item should have a title");
        expect(!std::string(item.category).empty(), "#957: each toolbox item should have a category");
        expect(!std::string(item.vfp_class).empty(), "#957: each toolbox item should name the VFP class");
        expect(!std::string(item.base_class).empty(), "#957: each toolbox item should name the VFP base class");
        expect(!std::string(item.default_name_prefix).empty(),
               "#957: each toolbox item should provide a default object-name prefix");
        expect(!item.contexts.empty(), "#957: each toolbox item should name at least one target context");
        expect(!std::string(item.description).empty(), "#957: each toolbox item should describe its creation action");
        if (item.container) {
            found_container = true;
        }
        if (item.category == "Data Controls") {
            found_data_control = true;
        }
        for (const auto context : item.contexts) {
            if (context == StudioToolboxContext::report) {
                found_report_item = true;
            }
        }
    }

    expect(found_container, "#957: toolbox palette should identify container controls");
    expect(found_report_item, "#957: toolbox palette should include report-compatible items");
    expect(found_data_control, "#957: toolbox palette should include data controls");
    expect(has_toolbox_item(items, "label"), "#957: toolbox palette should include Label");
    expect(has_toolbox_item(items, "textbox"), "#957: toolbox palette should include TextBox");
    expect(has_toolbox_item(items, "commandbutton"), "#957: toolbox palette should include CommandButton");
    expect(has_toolbox_item(items, "combobox"), "#957: toolbox palette should include ComboBox");
    expect(has_toolbox_item(items, "grid"), "#957: toolbox palette should include Grid");
    expect(has_toolbox_item(items, "container"), "#957: toolbox palette should include Container");
    expect(has_toolbox_item(items, "pageframe"), "#957: toolbox palette should include PageFrame");

    const auto form_items = copperfin::studio::studio_toolbox_items_for_context(StudioToolboxContext::form);
    expect(has_toolbox_item(form_items, "textbox"), "#957: form context should expose TextBox");
    expect(has_toolbox_item(form_items, "pageframe"), "#957: form context should expose PageFrame");
    expect(has_toolbox_item(form_items, "olecontrol"), "#957: form context should expose OLEControl");

    const auto container_items = copperfin::studio::studio_toolbox_items_for_context(StudioToolboxContext::container);
    expect(has_toolbox_item(container_items, "checkbox"), "#957: container context should expose CheckBox");
    expect(has_toolbox_item(container_items, "grid"), "#957: container context should expose Grid");

    const auto report_items = copperfin::studio::studio_toolbox_items_for_context(StudioToolboxContext::report);
    expect(has_toolbox_item(report_items, "label"), "#957: report context should expose Label");
    expect(has_toolbox_item(report_items, "line"), "#957: report context should expose Line");
    expect(!has_toolbox_item(report_items, "textbox"), "#957: report context should exclude form-only TextBox");
    expect(!has_toolbox_item(report_items, "pageframe"), "#957: report context should exclude form-only PageFrame");

    const auto visual_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "frmCustomer",
        .unique_id = "form-guid"
    });
    expect(visual_plan.ok,
        "#1209: visual-object selection contexts should plan toolbox palettes");
    expect(visual_plan.plan.selection_context == StudioEditorSelectionContext::visual_object &&
            visual_plan.plan.toolbox_context == StudioToolboxContext::form &&
            visual_plan.plan.asset_path == "forms/customer.scx" &&
            visual_plan.plan.record_index == 1U &&
            visual_plan.plan.object_name == "frmCustomer" &&
            visual_plan.plan.unique_id == "form-guid" &&
            visual_plan.plan.item_count == visual_plan.plan.items.size() &&
            has_toolbox_item(visual_plan.plan.items, "textbox") &&
            has_toolbox_item(visual_plan.plan.items, "pageframe"),
        "#1209: toolbox palette plans should preserve visual selection metadata and form items");

    const auto container_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::container_object,
        .asset_path = "forms/customer.scx",
        .record_index = 2U,
        .object_name = "pgAddress",
        .unique_id = "page-guid"
    });
    expect(container_plan.ok &&
            container_plan.plan.toolbox_context == StudioToolboxContext::container &&
            has_toolbox_item(container_plan.plan.items, "checkbox") &&
            has_toolbox_item(container_plan.plan.items, "grid"),
        "#1209: container selection contexts should plan container-safe toolbox palettes");

    const auto class_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::class_designer,
        .asset_path = "classes/controls.vcx",
        .record_index = 0U,
        .object_name = "txtBase",
        .unique_id = "class-guid"
    });
    expect(class_plan.ok &&
            class_plan.plan.toolbox_context == StudioToolboxContext::class_designer &&
            has_toolbox_item(class_plan.plan.items, "textbox"),
        "#1209: class-designer selection contexts should plan class-safe toolbox palettes");

    const auto report_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::report_expression,
        .asset_path = "reports/orders.frx",
        .record_index = 3U,
        .object_name = "Field1",
        .unique_id = "field-guid"
    });
    expect(report_plan.ok &&
            report_plan.plan.toolbox_context == StudioToolboxContext::report &&
            has_toolbox_item(report_plan.plan.items, "label") &&
            !has_toolbox_item(report_plan.plan.items, "textbox"),
        "#1209: report selection contexts should plan report-safe toolbox palettes");

    const auto label_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::label_expression,
        .asset_path = "labels/mailing.lbx",
        .record_index = 0U,
        .object_name = "Label1",
        .unique_id = "label-guid"
    });
    expect(label_plan.ok &&
            label_plan.plan.toolbox_context == StudioToolboxContext::report &&
            has_toolbox_item(label_plan.plan.items, "label") &&
            !has_toolbox_item(label_plan.plan.items, "pageframe"),
        "#1209: label selection contexts should reuse report-safe toolbox palettes");

    const auto menu_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::menu_item,
        .asset_path = "menus/main.mnx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(!menu_plan.ok,
        "#1209: menu selection contexts should reject toolbox palette launch planning");

    const auto project_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::project_item,
        .asset_path = "apps/customer.pjx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(!project_plan.ok,
        "#1209: project selection contexts should reject toolbox palette launch planning");

    const auto data_plan = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::data_environment,
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = "Dataenvironment",
        .unique_id = "de-guid"
    });
    expect(!data_plan.ok,
        "#1209: data-environment selection contexts should reject toolbox palette launch planning");

    const auto admitted_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = visual_plan.plan,
        .admit_palette_invocation = true
    });
    expect(admitted_invocation.ok,
        "#1219: toolbox invocation admission should accept validated launch plans");
    expect(admitted_invocation.plan.selection_context == StudioEditorSelectionContext::visual_object &&
            admitted_invocation.plan.toolbox_context == StudioToolboxContext::form &&
            admitted_invocation.plan.command_token == "studio.toolbox.palette.invoke" &&
            admitted_invocation.plan.asset_path == "forms/customer.scx" &&
            admitted_invocation.plan.record_index == 1U &&
            admitted_invocation.plan.object_name == "frmCustomer" &&
            admitted_invocation.plan.unique_id == "form-guid" &&
            admitted_invocation.plan.item_count == visual_plan.plan.item_count &&
            admitted_invocation.plan.items.size() == visual_plan.plan.items.size() &&
            admitted_invocation.plan.palette_invocation_admitted &&
            !admitted_invocation.plan.dry_run &&
            !admitted_invocation.plan.mutates_asset &&
            has_toolbox_item(admitted_invocation.plan.items, "textbox"),
        "#1219: toolbox invocation admission should preserve palette metadata and admitted state");

    const auto toolbox_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = admitted_invocation.plan
    });
    expect(toolbox_dispatch.ok,
        "#1233: toolbox dispatch should accept admitted toolbox palette invocations");
    expect(toolbox_dispatch.plan.selection_context == StudioEditorSelectionContext::visual_object &&
            toolbox_dispatch.plan.toolbox_context == StudioToolboxContext::form &&
            toolbox_dispatch.plan.command_token == "studio.toolbox.palette.invoke" &&
            toolbox_dispatch.plan.asset_path == "forms/customer.scx" &&
            toolbox_dispatch.plan.record_index == 1U &&
            toolbox_dispatch.plan.object_name == "frmCustomer" &&
            toolbox_dispatch.plan.unique_id == "form-guid" &&
            toolbox_dispatch.plan.item_count == visual_plan.plan.item_count &&
            toolbox_dispatch.plan.items.size() == visual_plan.plan.items.size() &&
            toolbox_dispatch.plan.dispatch_admitted &&
            !toolbox_dispatch.plan.dry_run &&
            !toolbox_dispatch.plan.executed &&
            !toolbox_dispatch.plan.mutates_asset &&
            has_toolbox_item(toolbox_dispatch.plan.items, "textbox"),
        "#1233: toolbox dispatch should preserve palette admission metadata without executing");
    expect(has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--command-token", "studio.toolbox.palette.invoke") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--selection-context", "visual_object") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--path", "forms/customer.scx") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--record", "1") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--object-name", "frmCustomer") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--unique-id", "form-guid") &&
            has_argument_pair(toolbox_dispatch.plan.dispatch_arguments, "--item-count",
                std::to_string(visual_plan.plan.item_count)),
        "#1233: toolbox dispatch should materialize a deterministic argument contract");

    const auto dry_run_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = report_plan.plan,
        .admit_palette_invocation = false
    });
    expect(dry_run_invocation.ok &&
            dry_run_invocation.plan.selection_context == StudioEditorSelectionContext::report_expression &&
            dry_run_invocation.plan.toolbox_context == StudioToolboxContext::report &&
            !dry_run_invocation.plan.palette_invocation_admitted &&
            dry_run_invocation.plan.dry_run &&
            !dry_run_invocation.plan.mutates_asset &&
            has_toolbox_item(dry_run_invocation.plan.items, "label") &&
            !has_toolbox_item(dry_run_invocation.plan.items, "textbox"),
        "#1219: toolbox invocation admission should default to dry-run and preserve filtered report items");

    const auto dry_run_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = dry_run_invocation.plan
    });
    expect(!dry_run_dispatch.ok &&
            dry_run_dispatch.error == "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1233: toolbox dispatch should reject dry-run admission plans");

    auto missing_command_plan = admitted_invocation.plan;
    missing_command_plan.command_token = {};
    const auto missing_command_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = missing_command_plan
    });
    expect(!missing_command_dispatch.ok &&
            missing_command_dispatch.error == "A toolbox dispatch request requires a command token.",
        "#1233: toolbox dispatch should reject admitted plans without command tokens");

    auto missing_items_plan = admitted_invocation.plan;
    missing_items_plan.items.clear();
    missing_items_plan.item_count = 0U;
    const auto missing_items_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = missing_items_plan
    });
    expect(!missing_items_dispatch.ok &&
            missing_items_dispatch.error == "A toolbox dispatch request requires validated toolbox item metadata.",
        "#1233: toolbox dispatch should reject admitted plans without item metadata");

    auto inconsistent_dispatch_plan = admitted_invocation.plan;
    inconsistent_dispatch_plan.item_count += 1U;
    const auto inconsistent_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = inconsistent_dispatch_plan
    });
    expect(!inconsistent_dispatch.ok &&
            inconsistent_dispatch.error == "A toolbox dispatch request requires consistent toolbox item metadata.",
        "#1233: toolbox dispatch should reject admitted plans with inconsistent item metadata");

    const auto missing_items_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = {},
        .admit_palette_invocation = true
    });
    expect(!missing_items_invocation.ok &&
            missing_items_invocation.error ==
                "A toolbox invocation admission request requires validated toolbox item metadata.",
        "#1219: toolbox invocation admission should reject missing item metadata");

    auto inconsistent_plan = visual_plan.plan;
    inconsistent_plan.item_count += 1U;
    const auto inconsistent_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = inconsistent_plan,
        .admit_palette_invocation = true
    });
    expect(!inconsistent_invocation.ok &&
            inconsistent_invocation.error ==
                "A toolbox invocation admission request requires consistent toolbox item metadata.",
        "#1219: toolbox invocation admission should reject inconsistent item metadata");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
