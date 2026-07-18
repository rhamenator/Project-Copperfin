#include "test_toolbox_creation_support.h"

namespace copperfin::toolbox_creation_tests
{
void test_toolbox_creation_catalog_plans_form_and_report_contexts_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_catalog_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto form_catalog = copperfin::studio::plan_visual_object_catalog_from_toolbox_context({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .path = table_path.string(),
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Planned"}
        }
    });
    const auto* textbox_entry = find_create_plan_entry(form_catalog.entries, "textbox");
    const auto* command_entry = find_create_plan_entry(form_catalog.entries, "commandbutton");
    expect(form_catalog.ok &&
            form_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            form_catalog.item_count == form_catalog.entries.size() &&
            form_catalog.plan_count == form_catalog.item_count &&
            form_catalog.error_count == 0U &&
            form_catalog.dry_run &&
            !form_catalog.mutates_asset,
        "#1243: form toolbox creation catalogs should summarize all form-compatible plans");
    expect(textbox_entry != nullptr &&
            textbox_entry->create_plan.ok &&
            textbox_entry->create_plan.plan.target_record_index == before_count &&
            textbox_entry->create_plan.plan.object_name == "txt2" &&
            textbox_entry->create_plan.plan.parent_name == "frmMain" &&
            has_field_value(textbox_entry->create_plan.plan.field_values, "CLASS", "TextBox") &&
            has_field_value(textbox_entry->create_plan.plan.field_values, "CAPTION", "Planned"),
        "#1243: form toolbox creation catalogs should preserve textbox generated names and field values");
    expect(command_entry != nullptr &&
            command_entry->create_plan.ok &&
            command_entry->create_plan.plan.object_name == "cmd1" &&
            has_field_value(command_entry->create_plan.plan.field_values, "CLASS", "CommandButton"),
        "#1243: form toolbox creation catalogs should preserve command button generated names");
    expect(object_count(table_path) == before_count,
        "#1243: form toolbox creation catalogs should not mutate the visual asset");

    const auto report_catalog = copperfin::studio::plan_visual_object_catalog_from_toolbox_context({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .path = table_path.string(),
        .parent_name = "DetailBand",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Report Planned"}
        }
    });
    const auto* report_label_entry = find_create_plan_entry(report_catalog.entries, "label");
    expect(report_catalog.ok &&
            report_catalog.item_count == report_catalog.entries.size() &&
            report_catalog.plan_count == report_catalog.item_count &&
            report_catalog.error_count == 0U &&
            report_label_entry != nullptr &&
            report_label_entry->create_plan.ok &&
            report_label_entry->create_plan.plan.object_name == "lbl1" &&
            report_label_entry->create_plan.plan.parent_name == "DetailBand" &&
            has_field_value(report_label_entry->create_plan.plan.field_values, "CLASS", "Label"),
        "#1243: report toolbox creation catalogs should include report-compatible label plans");
    expect(find_create_plan_entry(report_catalog.entries, "textbox") == nullptr,
        "#1243: report toolbox creation catalogs should exclude form-only textbox plans");
    expect(object_count(table_path) == before_count,
        "#1243: report toolbox creation catalogs should not mutate the visual asset");

    const auto visual_selection_catalog =
        copperfin::studio::plan_visual_object_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
            .path = table_path.string(),
            .parent_name = "frmMain",
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Selection Planned"}
            }
        });
    const auto* selection_textbox_entry = find_create_plan_entry(visual_selection_catalog.entries, "textbox");
    expect(visual_selection_catalog.ok &&
            visual_selection_catalog.selection_context ==
                copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_selection_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_selection_catalog.launch_plan.ok &&
            visual_selection_catalog.launch_plan.plan.toolbox_context ==
                copperfin::studio::StudioToolboxContext::form &&
            visual_selection_catalog.item_count == form_catalog.item_count &&
            visual_selection_catalog.plan_count == visual_selection_catalog.item_count &&
            visual_selection_catalog.error_count == 0U &&
            visual_selection_catalog.dry_run &&
            !visual_selection_catalog.mutates_asset &&
            selection_textbox_entry != nullptr &&
            selection_textbox_entry->create_plan.ok &&
            selection_textbox_entry->create_plan.plan.object_name == "txt2" &&
            selection_textbox_entry->create_plan.plan.parent_name == "frmMain" &&
            has_field_value(selection_textbox_entry->create_plan.plan.field_values, "CLASS", "TextBox") &&
            has_field_value(selection_textbox_entry->create_plan.plan.field_values, "CAPTION", "Selection Planned"),
        "#1292: visual selection toolbox creation catalogs should resolve form create plans");

    const auto report_selection_catalog =
        copperfin::studio::plan_visual_object_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
            .path = table_path.string(),
            .parent_name = "DetailBand",
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Selection Report"}
            }
        });
    const auto* selection_report_label_entry =
        find_create_plan_entry(report_selection_catalog.entries, "label");
    expect(report_selection_catalog.ok &&
            report_selection_catalog.selection_context ==
                copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_selection_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_selection_catalog.launch_plan.ok &&
            report_selection_catalog.item_count == report_catalog.item_count &&
            report_selection_catalog.plan_count == report_selection_catalog.item_count &&
            report_selection_catalog.error_count == 0U &&
            report_selection_catalog.dry_run &&
            !report_selection_catalog.mutates_asset &&
            selection_report_label_entry != nullptr &&
            selection_report_label_entry->create_plan.ok &&
            selection_report_label_entry->create_plan.plan.object_name == "lbl1" &&
            selection_report_label_entry->create_plan.plan.parent_name == "DetailBand" &&
            has_field_value(selection_report_label_entry->create_plan.plan.field_values, "CLASS", "Label") &&
            find_create_plan_entry(report_selection_catalog.entries, "textbox") == nullptr,
        "#1292: report selection toolbox creation catalogs should resolve report-safe create plans");

    const auto unsupported_selection_catalog =
        copperfin::studio::plan_visual_object_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::menu_item,
            .path = table_path.string(),
            .parent_name = "File",
            .field_values = {}
        });
    expect(!unsupported_selection_catalog.ok &&
            unsupported_selection_catalog.error ==
                "A selection-context toolbox object creation catalog request requires a toolbox palette." &&
            unsupported_selection_catalog.selection_context ==
                copperfin::studio::StudioEditorSelectionContext::menu_item &&
            !unsupported_selection_catalog.launch_plan.ok &&
            unsupported_selection_catalog.launch_plan.error ==
                "The selected Studio context does not expose a toolbox palette." &&
            unsupported_selection_catalog.item_count == 0U &&
            unsupported_selection_catalog.plan_count == 0U &&
            unsupported_selection_catalog.error_count == 0U &&
            unsupported_selection_catalog.dry_run &&
            !unsupported_selection_catalog.mutates_asset &&
            unsupported_selection_catalog.entries.empty(),
        "#1292: unsupported selection toolbox creation catalogs should reject without mutation");

    expect(object_count(table_path) == before_count,
        "#1292: selection toolbox creation catalogs should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_dispatch_catalog_plans_context_dispatches_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_dispatch_catalog_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto form_catalog = copperfin::studio::plan_visual_object_create_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .path = table_path.string(),
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Dispatch Catalog"}
        },
        .admit_create_operation = true
    });
    const auto* textbox_entry = find_create_dispatch_entry(form_catalog.entries, "textbox");
    const auto* command_entry = find_create_dispatch_entry(form_catalog.entries, "commandbutton");

    expect(form_catalog.ok &&
            form_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            form_catalog.item_count == form_catalog.entries.size() &&
            form_catalog.dispatch_count == form_catalog.item_count &&
            form_catalog.error_count == 0U &&
            !form_catalog.dry_run &&
            form_catalog.mutates_asset,
        "#1253: admitted toolbox create dispatch catalogs should summarize form-compatible dispatches");
    expect(textbox_entry != nullptr &&
            textbox_entry->create_plan.ok &&
            textbox_entry->dispatch.ok &&
            textbox_entry->create_plan.plan.object_name == "txt2" &&
            textbox_entry->dispatch.plan.object_name == "txt2" &&
            textbox_entry->dispatch.plan.dispatch_admitted &&
            !textbox_entry->dispatch.plan.executed &&
            textbox_entry->dispatch.plan.mutates_asset &&
            has_argument_pair(textbox_entry->dispatch.plan.dispatch_arguments, "--toolbox-create", "textbox") &&
            has_argument_pair(textbox_entry->dispatch.plan.dispatch_arguments, "--object-name", "txt2") &&
            has_argument_pair(textbox_entry->dispatch.plan.dispatch_arguments, "--field-value",
                "CAPTION=Dispatch Catalog"),
        "#1253: admitted toolbox create dispatch catalogs should preserve textbox plans and arguments");
    expect(command_entry != nullptr &&
            command_entry->create_plan.ok &&
            command_entry->dispatch.ok &&
            command_entry->dispatch.plan.object_name == "cmd1" &&
            has_argument_pair(command_entry->dispatch.plan.dispatch_arguments, "--toolbox-create", "commandbutton") &&
            has_argument_pair(command_entry->dispatch.plan.dispatch_arguments, "--object-name", "cmd1"),
        "#1253: admitted toolbox create dispatch catalogs should preserve command button arguments");
    expect(object_count(table_path) == before_count,
        "#1253: toolbox create dispatch catalogs should not mutate the visual asset");

    const auto report_catalog = copperfin::studio::plan_visual_object_create_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .path = table_path.string(),
        .parent_name = "DetailBand",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Report Dispatch"}
        },
        .admit_create_operation = true
    });
    const auto* report_label_entry = find_create_dispatch_entry(report_catalog.entries, "label");
    expect(report_catalog.ok &&
            report_catalog.dispatch_count == report_catalog.item_count &&
            report_label_entry != nullptr &&
            report_label_entry->dispatch.ok &&
            report_label_entry->dispatch.plan.object_name == "lbl1" &&
            has_argument_pair(report_label_entry->dispatch.plan.dispatch_arguments, "--toolbox-create", "label") &&
            has_argument_pair(report_label_entry->dispatch.plan.dispatch_arguments, "--toolbox-context", "report"),
        "#1253: report toolbox create dispatch catalogs should include report-compatible dispatches");
    expect(find_create_dispatch_entry(report_catalog.entries, "textbox") == nullptr,
        "#1253: report toolbox create dispatch catalogs should exclude form-only textbox dispatches");
    expect(object_count(table_path) == before_count,
        "#1253: report toolbox create dispatch catalogs should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_dispatch_catalog_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto catalog = copperfin::studio::plan_visual_object_create_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .path = table_path.string(),
        .parent_name = "frmMain",
        .field_values = {},
        .admit_create_operation = false
    });
    const auto* textbox_entry = find_create_dispatch_entry(catalog.entries, "textbox");

    expect(catalog.ok &&
            catalog.item_count == catalog.entries.size() &&
            catalog.dispatch_count == 0U &&
            catalog.error_count == catalog.item_count &&
            catalog.dry_run &&
            !catalog.mutates_asset,
        "#1253: non-admitted toolbox create dispatch catalogs should summarize per-item errors");
    expect(textbox_entry != nullptr &&
            textbox_entry->create_plan.ok &&
            !textbox_entry->dispatch.ok &&
            textbox_entry->dispatch.error ==
                "A toolbox create dispatch request requires an admitted non-dry-run create operation." &&
            textbox_entry->dispatch.plan.dispatch_arguments.empty(),
        "#1253: non-admitted toolbox create dispatch entries should not expose stale arguments");
    expect(object_count(table_path) == before_count,
        "#1253: non-admitted toolbox create dispatch catalogs should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_selection_dispatch_catalog_plans_context_dispatches_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_dispatch_catalog_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto visual_catalog =
        copperfin::studio::plan_visual_object_create_dispatch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
            .path = table_path.string(),
            .parent_name = "frmMain",
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Selection Dispatch"}
            },
            .admit_create_operation = true
        });
    const auto* visual_textbox_entry = find_create_dispatch_entry(visual_catalog.entries, "textbox");
    expect(visual_catalog.ok &&
            visual_catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_catalog.launch_plan.ok &&
            visual_catalog.launch_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_catalog.item_count == visual_catalog.entries.size() &&
            visual_catalog.dispatch_count == visual_catalog.item_count &&
            visual_catalog.error_count == 0U &&
            !visual_catalog.dry_run &&
            visual_catalog.mutates_asset,
        "#1294: admitted visual selection toolbox create dispatch catalogs should summarize form dispatches");
    expect(visual_textbox_entry != nullptr &&
            visual_textbox_entry->create_plan.ok &&
            visual_textbox_entry->dispatch.ok &&
            visual_textbox_entry->create_plan.plan.object_name == "txt2" &&
            visual_textbox_entry->create_plan.plan.parent_name == "frmMain" &&
            visual_textbox_entry->dispatch.plan.object_name == "txt2" &&
            visual_textbox_entry->dispatch.plan.dispatch_admitted &&
            !visual_textbox_entry->dispatch.plan.executed &&
            visual_textbox_entry->dispatch.plan.mutates_asset &&
            has_argument_pair(visual_textbox_entry->dispatch.plan.dispatch_arguments, "--toolbox-create",
                "textbox") &&
            has_argument_pair(visual_textbox_entry->dispatch.plan.dispatch_arguments, "--field-value",
                "CAPTION=Selection Dispatch"),
        "#1294: admitted visual selection toolbox create dispatch catalogs should preserve textbox metadata");
    expect(object_count(table_path) == before_count,
        "#1294: admitted visual selection toolbox create dispatch catalogs should not mutate assets");

    const auto report_catalog =
        copperfin::studio::plan_visual_object_create_dispatch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
            .path = table_path.string(),
            .parent_name = "DetailBand",
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Report Selection Dispatch"}
            },
            .admit_create_operation = true
        });
    const auto* report_label_entry = find_create_dispatch_entry(report_catalog.entries, "label");
    expect(report_catalog.ok &&
            report_catalog.selection_context ==
                copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_catalog.launch_plan.ok &&
            report_catalog.dispatch_count == report_catalog.item_count &&
            report_label_entry != nullptr &&
            report_label_entry->dispatch.ok &&
            report_label_entry->dispatch.plan.object_name == "lbl1" &&
            has_argument_pair(report_label_entry->dispatch.plan.dispatch_arguments, "--toolbox-context",
                "report") &&
            find_create_dispatch_entry(report_catalog.entries, "textbox") == nullptr,
        "#1294: admitted report selection toolbox create dispatch catalogs should resolve report-safe dispatches");
    expect(object_count(table_path) == before_count,
        "#1294: admitted report selection toolbox create dispatch catalogs should not mutate assets");

    const auto unsupported_catalog =
        copperfin::studio::plan_visual_object_create_dispatch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::menu_item,
            .path = table_path.string(),
            .parent_name = "File",
            .field_values = {},
            .admit_create_operation = true
        });
    expect(!unsupported_catalog.ok &&
            unsupported_catalog.error ==
                "A selection-context toolbox object creation dispatch catalog request requires a toolbox palette." &&
            unsupported_catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item &&
            !unsupported_catalog.launch_plan.ok &&
            unsupported_catalog.launch_plan.error == "The selected Studio context does not expose a toolbox palette." &&
            unsupported_catalog.item_count == 0U &&
            unsupported_catalog.dispatch_count == 0U &&
            unsupported_catalog.error_count == 0U &&
            unsupported_catalog.dry_run &&
            !unsupported_catalog.mutates_asset &&
            unsupported_catalog.entries.empty(),
        "#1294: unsupported selection toolbox create dispatch catalogs should reject without mutation");

    expect(object_count(table_path) == before_count,
        "#1294: selection toolbox create dispatch catalogs should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_selection_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_dispatch_catalog_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto catalog =
        copperfin::studio::plan_visual_object_create_dispatch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
            .path = table_path.string(),
            .parent_name = "frmMain",
            .field_values = {},
            .admit_create_operation = false
        });
    const auto* textbox_entry = find_create_dispatch_entry(catalog.entries, "textbox");

    expect(catalog.ok &&
            catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            catalog.launch_plan.ok &&
            catalog.item_count == catalog.entries.size() &&
            catalog.dispatch_count == 0U &&
            catalog.error_count == catalog.item_count &&
            catalog.dry_run &&
            !catalog.mutates_asset,
        "#1294: non-admitted selection toolbox create dispatch catalogs should summarize per-item errors");
    expect(textbox_entry != nullptr &&
            textbox_entry->create_plan.ok &&
            !textbox_entry->dispatch.ok &&
            textbox_entry->dispatch.error ==
                "A toolbox create dispatch request requires an admitted non-dry-run create operation." &&
            textbox_entry->dispatch.plan.dispatch_arguments.empty(),
        "#1294: non-admitted selection toolbox create dispatch entries should not expose stale arguments");
    expect(object_count(table_path) == before_count,
        "#1294: non-admitted selection toolbox create dispatch catalogs should not mutate assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_plan_catalog_plans_context_batches_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_plan_catalog_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto form_catalog = copperfin::studio::plan_visual_object_batch_catalog_from_toolbox_context({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .path = table_path.string(),
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Batch Catalog"}
        }
    });
    const auto* label_plan = find_create_batch_plan(form_catalog.batch_plan.plan.plans, "label");
    const auto* textbox_plan = find_create_batch_plan(form_catalog.batch_plan.plan.plans, "textbox");
    const auto* command_plan = find_create_batch_plan(form_catalog.batch_plan.plan.plans, "commandbutton");

    expect(form_catalog.ok &&
            form_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            form_catalog.item_count == form_catalog.batch_plan.plan.item_count &&
            form_catalog.plan_count == 1U &&
            form_catalog.error_count == 0U &&
            form_catalog.batch_plan.ok &&
            form_catalog.dry_run &&
            !form_catalog.mutates_asset,
        "#1257: form toolbox batch create catalogs should summarize one context batch");
    expect(label_plan != nullptr &&
            label_plan->target_record_index == before_count &&
            label_plan->object_name == "lbl1" &&
            textbox_plan != nullptr &&
            textbox_plan->target_record_index > label_plan->target_record_index &&
            textbox_plan->object_name == "txt2" &&
            textbox_plan->parent_name == "frmMain" &&
            has_field_value(textbox_plan->field_values, "CLASS", "TextBox") &&
            has_field_value(textbox_plan->field_values, "CAPTION", "Batch Catalog"),
        "#1257: form toolbox batch create catalogs should preserve textbox generated plans");
    expect(command_plan != nullptr &&
            command_plan->target_record_index > textbox_plan->target_record_index &&
            command_plan->object_name == "cmd1" &&
            has_field_value(command_plan->field_values, "CLASS", "CommandButton"),
        "#1257: form toolbox batch create catalogs should reserve generated names across the batch");
    expect(object_count(table_path) == before_count,
        "#1257: form toolbox batch create catalogs should not mutate the visual asset");

    const auto report_catalog = copperfin::studio::plan_visual_object_batch_catalog_from_toolbox_context({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .path = table_path.string(),
        .parent_name = "DetailBand",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Report Batch Catalog"}
        }
    });
    const auto* report_label_plan = find_create_batch_plan(report_catalog.batch_plan.plan.plans, "label");

    expect(report_catalog.ok &&
            report_catalog.batch_plan.ok &&
            report_catalog.plan_count == 1U &&
            report_catalog.error_count == 0U &&
            report_label_plan != nullptr &&
            report_label_plan->object_name == "lbl1" &&
            report_label_plan->parent_name == "DetailBand" &&
            has_field_value(report_label_plan->field_values, "CLASS", "Label"),
        "#1257: report toolbox batch create catalogs should include report-compatible plans");
    expect(find_create_batch_plan(report_catalog.batch_plan.plan.plans, "textbox") == nullptr,
        "#1257: report toolbox batch create catalogs should exclude form-only textbox plans");
    expect(object_count(table_path) == before_count,
        "#1257: report toolbox batch create catalogs should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_plan_catalog_reports_planning_errors_without_stale_plans() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_plan_catalog_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto catalog = copperfin::studio::plan_visual_object_batch_catalog_from_toolbox_context({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .path = (temp_dir / "missing.scx").string(),
        .parent_name = "frmMain",
        .field_values = {}
    });

    expect(catalog.ok &&
            !catalog.batch_plan.ok &&
            !catalog.batch_plan.error.empty() &&
            catalog.plan_count == 0U &&
            catalog.error_count == 1U &&
            catalog.dry_run &&
            !catalog.mutates_asset &&
            catalog.batch_plan.plan.plans.empty(),
        "#1257: toolbox batch create catalogs should report planning errors without stale plans");
    expect(object_count(table_path) == before_count,
        "#1257: rejected toolbox batch create catalogs should not mutate unrelated visual assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_selection_batch_plan_catalog_plans_context_batches_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_batch_plan_catalog_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto visual_catalog =
        copperfin::studio::plan_visual_object_batch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
            .path = table_path.string(),
            .parent_name = "frmMain",
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Selection Batch Catalog"}
            }
        });
    const auto* visual_label_plan = find_create_batch_plan(visual_catalog.batch_plan.plan.plans, "label");
    const auto* visual_textbox_plan = find_create_batch_plan(visual_catalog.batch_plan.plan.plans, "textbox");
    const auto* visual_command_plan = find_create_batch_plan(visual_catalog.batch_plan.plan.plans, "commandbutton");

    expect(visual_catalog.ok &&
            visual_catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_catalog.launch_plan.ok &&
            visual_catalog.launch_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_catalog.item_count == visual_catalog.batch_plan.plan.item_count &&
            visual_catalog.plan_count == 1U &&
            visual_catalog.error_count == 0U &&
            visual_catalog.batch_plan.ok &&
            visual_catalog.dry_run &&
            !visual_catalog.mutates_asset,
        "#1296: visual selection toolbox batch create catalogs should summarize form batches");
    expect(visual_label_plan != nullptr &&
            visual_label_plan->target_record_index == before_count &&
            visual_label_plan->object_name == "lbl1" &&
            visual_textbox_plan != nullptr &&
            visual_textbox_plan->target_record_index > visual_label_plan->target_record_index &&
            visual_textbox_plan->object_name == "txt2" &&
            visual_textbox_plan->parent_name == "frmMain" &&
            has_field_value(visual_textbox_plan->field_values, "CLASS", "TextBox") &&
            has_field_value(visual_textbox_plan->field_values, "CAPTION", "Selection Batch Catalog"),
        "#1296: visual selection toolbox batch create catalogs should preserve textbox plans");
    expect(visual_command_plan != nullptr &&
            visual_command_plan->target_record_index > visual_textbox_plan->target_record_index &&
            visual_command_plan->object_name == "cmd1" &&
            has_field_value(visual_command_plan->field_values, "CLASS", "CommandButton"),
        "#1296: visual selection toolbox batch create catalogs should reserve generated names");
    expect(object_count(table_path) == before_count,
        "#1296: visual selection toolbox batch create catalogs should not mutate assets");

    const auto report_catalog =
        copperfin::studio::plan_visual_object_batch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
            .path = table_path.string(),
            .parent_name = "DetailBand",
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Report Selection Batch"}
            }
        });
    const auto* report_label_plan = find_create_batch_plan(report_catalog.batch_plan.plan.plans, "label");

    expect(report_catalog.ok &&
            report_catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_catalog.launch_plan.ok &&
            report_catalog.plan_count == 1U &&
            report_catalog.error_count == 0U &&
            report_catalog.batch_plan.ok &&
            report_label_plan != nullptr &&
            report_label_plan->object_name == "lbl1" &&
            report_label_plan->parent_name == "DetailBand" &&
            has_field_value(report_label_plan->field_values, "CLASS", "Label") &&
            has_field_value(report_label_plan->field_values, "CAPTION", "Report Selection Batch") &&
            find_create_batch_plan(report_catalog.batch_plan.plan.plans, "textbox") == nullptr,
        "#1296: report selection toolbox batch create catalogs should resolve report-safe batches");
    expect(object_count(table_path) == before_count,
        "#1296: report selection toolbox batch create catalogs should not mutate assets");

    const auto unsupported_catalog =
        copperfin::studio::plan_visual_object_batch_catalog_from_toolbox_selection({
            .selection_context = copperfin::studio::StudioEditorSelectionContext::menu_item,
            .path = table_path.string(),
            .parent_name = "File",
            .field_values = {}
        });
    expect(!unsupported_catalog.ok &&
            unsupported_catalog.error ==
                "A selection-context toolbox object batch creation catalog request requires a toolbox palette." &&
            unsupported_catalog.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item &&
            !unsupported_catalog.launch_plan.ok &&
            unsupported_catalog.launch_plan.error == "The selected Studio context does not expose a toolbox palette." &&
            unsupported_catalog.item_count == 0U &&
            unsupported_catalog.plan_count == 0U &&
            unsupported_catalog.error_count == 0U &&
            unsupported_catalog.dry_run &&
            !unsupported_catalog.mutates_asset &&
            unsupported_catalog.batch_plan.plan.plans.empty(),
        "#1296: unsupported selection toolbox batch create catalogs should reject without mutation");
    expect(object_count(table_path) == before_count,
        "#1296: selection toolbox batch create catalogs should not mutate assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_dispatch_catalog_plans_context_batches_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_dispatch_catalog_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto form_catalog = copperfin::studio::plan_visual_object_batch_create_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .path = table_path.string(),
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Batch Dispatch Catalog"}
        },
        .admit_create_operation = true
    });
    const auto* label_plan = find_create_batch_plan(form_catalog.dispatch.plan.plans, "label");
    const auto* textbox_plan = find_create_batch_plan(form_catalog.dispatch.plan.plans, "textbox");
    const auto* command_plan = find_create_batch_plan(form_catalog.dispatch.plan.plans, "commandbutton");

    expect(form_catalog.ok &&
            form_catalog.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            form_catalog.item_count == form_catalog.batch_plan.plan.item_count &&
            form_catalog.batch_plan.ok &&
            form_catalog.dispatch.ok &&
            form_catalog.dispatch_count == 1U &&
            form_catalog.error_count == 0U &&
            !form_catalog.dry_run &&
            form_catalog.mutates_asset,
        "#1255: admitted toolbox batch create dispatch catalogs should summarize one context batch");
    expect(form_catalog.dispatch.plan.item_count == form_catalog.item_count &&
            form_catalog.dispatch.plan.plans.size() == form_catalog.item_count &&
            label_plan != nullptr &&
            label_plan->target_record_index == before_count &&
            label_plan->object_name == "lbl1" &&
            textbox_plan != nullptr &&
            textbox_plan->target_record_index > label_plan->target_record_index &&
            textbox_plan->object_name == "txt2" &&
            command_plan != nullptr &&
            command_plan->target_record_index > textbox_plan->target_record_index &&
            command_plan->object_name == "cmd1" &&
            form_catalog.dispatch.plan.dispatch_admitted &&
            !form_catalog.dispatch.plan.executed &&
            form_catalog.dispatch.plan.mutates_asset,
        "#1255: admitted toolbox batch create dispatch catalogs should preserve batch plan metadata");
    expect(has_argument(form_catalog.dispatch.plan.dispatch_arguments, "--toolbox-create-batch") &&
            has_argument_pair(form_catalog.dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(form_catalog.dispatch.plan.dispatch_arguments, "--toolbox-item", "textbox") &&
            has_argument_pair(form_catalog.dispatch.plan.dispatch_arguments, "--object-name", "txt2") &&
            has_argument_pair(form_catalog.dispatch.plan.dispatch_arguments, "--toolbox-item", "commandbutton") &&
            has_argument_pair(form_catalog.dispatch.plan.dispatch_arguments, "--object-name", "cmd1") &&
            has_argument_pair(form_catalog.dispatch.plan.dispatch_arguments, "--field-value",
                "CAPTION=Batch Dispatch Catalog"),
        "#1255: admitted toolbox batch create dispatch catalogs should materialize batch host arguments");
    expect(object_count(table_path) == before_count,
        "#1255: toolbox batch create dispatch catalogs should not mutate the visual asset");

    const auto report_catalog = copperfin::studio::plan_visual_object_batch_create_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .path = table_path.string(),
        .parent_name = "DetailBand",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Report Batch Dispatch"}
        },
        .admit_create_operation = true
    });

    expect(report_catalog.ok &&
            report_catalog.batch_plan.ok &&
            report_catalog.dispatch.ok &&
            report_catalog.dispatch.plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            has_argument_pair(report_catalog.dispatch.plan.dispatch_arguments, "--toolbox-context", "report") &&
            has_argument_pair(report_catalog.dispatch.plan.dispatch_arguments, "--toolbox-item", "label") &&
            !has_argument_pair(report_catalog.dispatch.plan.dispatch_arguments, "--toolbox-item", "textbox"),
        "#1255: report toolbox batch create dispatch catalogs should include only report-compatible items");
    expect(object_count(table_path) == before_count,
        "#1255: report toolbox batch create dispatch catalogs should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_dispatch_catalog_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto catalog = copperfin::studio::plan_visual_object_batch_create_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .path = table_path.string(),
        .parent_name = "frmMain",
        .field_values = {},
        .admit_create_operation = false
    });

    expect(catalog.ok &&
            catalog.batch_plan.ok &&
            !catalog.dispatch.ok &&
            catalog.dispatch.error ==
                "A toolbox batch create dispatch request requires an admitted non-dry-run create operation." &&
            catalog.dispatch_count == 0U &&
            catalog.error_count == 1U &&
            catalog.dry_run &&
            !catalog.mutates_asset,
        "#1255: non-admitted toolbox batch create dispatch catalogs should report one batch dispatch error");
    expect(catalog.dispatch.plan.dispatch_arguments.empty(),
        "#1255: non-admitted toolbox batch create dispatch catalogs should not expose stale arguments");
    expect(object_count(table_path) == before_count,
        "#1255: non-admitted toolbox batch create dispatch catalogs should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}


}
