#include "test_toolbox_creation_support.h"

namespace copperfin::toolbox_creation_tests
{
void test_toolbox_creation_selection_batch_dispatch_catalog_plans_context_batches_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_batch_dispatch_catalog_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto visual_catalog =
        copperfin::studio::plan_visual_object_batch_create_dispatch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
            .path = table_path.string(),
            .parent_name = "frmMain",
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Selection Batch Dispatch"}
            },
            .admit_create_operation = true
        });
    const auto* visual_label_plan = find_create_batch_plan(visual_catalog.dispatch.plan.plans, "label");
    const auto* visual_textbox_plan = find_create_batch_plan(visual_catalog.dispatch.plan.plans, "textbox");
    const auto* visual_command_plan = find_create_batch_plan(visual_catalog.dispatch.plan.plans, "commandbutton");

    expect(visual_catalog.ok &&
            visual_catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_catalog.launch_plan.ok &&
            visual_catalog.launch_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_catalog.item_count == visual_catalog.batch_plan.plan.item_count &&
            visual_catalog.batch_plan.ok &&
            visual_catalog.dispatch.ok &&
            visual_catalog.dispatch_count == 1U &&
            visual_catalog.error_count == 0U &&
            !visual_catalog.dry_run &&
            visual_catalog.mutates_asset,
        "#1298: admitted visual selection toolbox batch create dispatch catalogs should summarize form batches");
    expect(visual_label_plan != nullptr &&
            visual_label_plan->target_record_index == before_count &&
            visual_label_plan->object_name == "lbl1" &&
            visual_textbox_plan != nullptr &&
            visual_textbox_plan->target_record_index > visual_label_plan->target_record_index &&
            visual_textbox_plan->object_name == "txt2" &&
            visual_textbox_plan->parent_name == "frmMain" &&
            has_field_value(visual_textbox_plan->field_values, "CLASS", "TextBox") &&
            has_field_value(visual_textbox_plan->field_values, "CAPTION", "Selection Batch Dispatch") &&
            visual_command_plan != nullptr &&
            visual_command_plan->target_record_index > visual_textbox_plan->target_record_index &&
            visual_command_plan->object_name == "cmd1",
        "#1298: admitted visual selection toolbox batch create dispatch catalogs should preserve batch metadata");
    expect(visual_catalog.dispatch.plan.dispatch_admitted &&
            !visual_catalog.dispatch.plan.executed &&
            visual_catalog.dispatch.plan.mutates_asset &&
            has_argument(visual_catalog.dispatch.plan.dispatch_arguments, "--toolbox-create-batch") &&
            has_argument_pair(visual_catalog.dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(visual_catalog.dispatch.plan.dispatch_arguments, "--toolbox-item", "textbox") &&
            has_argument_pair(visual_catalog.dispatch.plan.dispatch_arguments, "--object-name", "txt2") &&
            has_argument_pair(visual_catalog.dispatch.plan.dispatch_arguments, "--toolbox-item", "commandbutton") &&
            has_argument_pair(visual_catalog.dispatch.plan.dispatch_arguments, "--object-name", "cmd1") &&
            has_argument_pair(visual_catalog.dispatch.plan.dispatch_arguments, "--field-value",
                "CAPTION=Selection Batch Dispatch"),
        "#1298: admitted visual selection toolbox batch create dispatch catalogs should materialize arguments");
    expect(object_count(table_path) == before_count,
        "#1298: admitted visual selection toolbox batch create dispatch catalogs should not mutate assets");

    const auto report_catalog =
        copperfin::studio::plan_visual_object_batch_create_dispatch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
            .path = table_path.string(),
            .parent_name = "DetailBand",
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Report Selection Batch Dispatch"}
            },
            .admit_create_operation = true
        });
    expect(report_catalog.ok &&
            report_catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_catalog.launch_plan.ok &&
            report_catalog.batch_plan.ok &&
            report_catalog.dispatch.ok &&
            report_catalog.dispatch.plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            has_argument_pair(report_catalog.dispatch.plan.dispatch_arguments, "--toolbox-context", "report") &&
            has_argument_pair(report_catalog.dispatch.plan.dispatch_arguments, "--toolbox-item", "label") &&
            !has_argument_pair(report_catalog.dispatch.plan.dispatch_arguments, "--toolbox-item", "textbox"),
        "#1298: admitted report selection toolbox batch create dispatch catalogs should resolve report-safe batches");
    expect(object_count(table_path) == before_count,
        "#1298: admitted report selection toolbox batch create dispatch catalogs should not mutate assets");

    const auto unsupported_catalog =
        copperfin::studio::plan_visual_object_batch_create_dispatch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::menu_item,
            .path = table_path.string(),
            .parent_name = "File",
            .field_values = {},
            .admit_create_operation = true
        });
    expect(!unsupported_catalog.ok &&
            unsupported_catalog.error ==
                "A selection-context toolbox object batch creation dispatch catalog request requires a toolbox palette." &&
            unsupported_catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item &&
            !unsupported_catalog.launch_plan.ok &&
            unsupported_catalog.launch_plan.error == "The selected Studio context does not expose a toolbox palette." &&
            unsupported_catalog.item_count == 0U &&
            unsupported_catalog.dispatch_count == 0U &&
            unsupported_catalog.error_count == 0U &&
            unsupported_catalog.dry_run &&
            !unsupported_catalog.mutates_asset,
        "#1298: unsupported selection toolbox batch create dispatch catalogs should reject without mutation");
    expect(object_count(table_path) == before_count,
        "#1298: selection toolbox batch create dispatch catalogs should not mutate assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_selection_batch_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_batch_dispatch_catalog_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto catalog =
        copperfin::studio::plan_visual_object_batch_create_dispatch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
            .path = table_path.string(),
            .parent_name = "frmMain",
            .field_values = {},
            .admit_create_operation = false
        });

    expect(catalog.ok &&
            catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            catalog.launch_plan.ok &&
            catalog.batch_plan.ok &&
            !catalog.dispatch.ok &&
            catalog.dispatch.error ==
                "A toolbox batch create dispatch request requires an admitted non-dry-run create operation." &&
            catalog.dispatch_count == 0U &&
            catalog.error_count == 1U &&
            catalog.dry_run &&
            !catalog.mutates_asset,
        "#1298: non-admitted selection toolbox batch create dispatch catalogs should report dispatch errors");
    expect(catalog.dispatch.plan.dispatch_arguments.empty(),
        "#1298: non-admitted selection toolbox batch create dispatch catalogs should not expose stale arguments");
    expect(object_count(table_path) == before_count,
        "#1298: non-admitted selection toolbox batch create dispatch catalogs should not mutate assets");

    fs::remove_all(temp_dir, ignored);
}


}
