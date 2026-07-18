#include "test_toolbox_creation_support.h"

namespace copperfin::toolbox_creation_tests
{
void test_toolbox_creation_planner_maps_descriptors_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_plan_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto plan_result = copperfin::studio::plan_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "planned-textbox-guid",
        .parent_name = "frmMain",
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Planned Customer"},
            {.property_name = "PROPERTIES", .property_value = "ControlSource = \"customer.name\"\r\n"}
        }
    });

    expect(plan_result.ok,
        "#1241: toolbox creation planning should accept known toolbox items");
    expect(plan_result.plan.path == table_path.string() &&
            std::string(plan_result.plan.toolbox_item.id) == "textbox" &&
            plan_result.plan.toolbox_context_provided &&
            plan_result.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            plan_result.plan.target_record_index == before_count &&
            plan_result.plan.object_name == "txt2" &&
            plan_result.plan.unique_id == "planned-textbox-guid" &&
            plan_result.plan.parent_name == "frmMain" &&
            plan_result.plan.dry_run &&
            !plan_result.plan.mutates_asset,
        "#1241: toolbox creation planning should preserve descriptor, target, identity, and dry-run metadata");
    expect(has_field_value(plan_result.plan.field_values, "OBJNAME", "txt2") &&
            has_field_value(plan_result.plan.field_values, "NAME", "txt2") &&
            has_field_value(plan_result.plan.field_values, "CLASS", "TextBox") &&
            has_field_value(plan_result.plan.field_values, "BASECLASS", "TextBox") &&
            has_field_value(plan_result.plan.field_values, "UNIQUEID", "planned-textbox-guid") &&
            has_field_value(plan_result.plan.field_values, "PARENT", "frmMain") &&
            has_field_value(plan_result.plan.field_values, "CAPTION", "Planned Customer") &&
            has_field_value(plan_result.plan.field_values, "PROPERTIES", "ControlSource = \"customer.name\"\r\n"),
        "#1241: toolbox creation planning should preserve the field values used by mutating creates");
    expect(object_count(table_path) == before_count,
        "#1241: toolbox creation planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_planner_respects_explicit_names_and_rejections() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_plan_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto explicit_plan = copperfin::studio::plan_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "commandbutton",
        .object_name = "cmdRun",
        .unique_id = "command-guid",
        .parent_name = "frmMain",
        .field_values = {}
    });
    expect(explicit_plan.ok &&
            explicit_plan.plan.object_name == "cmdRun" &&
            explicit_plan.plan.target_record_index == before_count &&
            has_field_value(explicit_plan.plan.field_values, "CLASS", "CommandButton"),
        "#1241: toolbox creation planning should preserve explicit object names and descriptor metadata");
    expect(object_count(table_path) == before_count,
        "#1241: explicit-name toolbox creation planning should not mutate the visual asset");

    const auto unknown_plan = copperfin::studio::plan_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "missing-toolbox-item",
        .object_name = {},
        .unique_id = "should-not-exist",
        .parent_name = "frmMain",
        .field_values = {}
    });
    expect(!unknown_plan.ok && unknown_plan.error == "The requested toolbox item was not found.",
        "#1241: toolbox creation planning should reject unknown toolbox ids");
    expect(object_count(table_path) == before_count,
        "#1241: unknown toolbox creation planning should not mutate the visual asset");

    const auto rejected_context_plan = copperfin::studio::plan_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "report-textbox-guid",
        .parent_name = "DetailBand",
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .field_values = {}
    });
    expect(!rejected_context_plan.ok &&
            rejected_context_plan.error ==
                "The requested toolbox item is not available in the requested designer context.",
        "#1241: toolbox creation planning should reject incompatible toolbox contexts");
    expect(object_count(table_path) == before_count,
        "#1241: rejected context toolbox creation planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_selection_planner_resolves_contexts_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_plan_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto visual_plan = copperfin::studio::plan_visual_object_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = {},
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Selection Planned"}
        }
    });
    expect(visual_plan.ok &&
            visual_plan.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_plan.launch_plan.ok &&
            visual_plan.create_plan.ok &&
            visual_plan.create_plan.plan.toolbox_context_provided &&
            visual_plan.create_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_plan.create_plan.plan.target_record_index == before_count &&
            visual_plan.create_plan.plan.object_name == "txt2" &&
            visual_plan.create_plan.plan.parent_name == "frmMain" &&
            visual_plan.dry_run &&
            !visual_plan.mutates_asset,
        "#1300: selection toolbox creation planning should resolve visual selections to form create plans");
    expect(has_field_value(visual_plan.create_plan.plan.field_values, "CLASS", "TextBox") &&
            has_field_value(visual_plan.create_plan.plan.field_values, "CAPTION", "Selection Planned"),
        "#1300: selection toolbox creation planning should preserve descriptor and caller field values");
    expect(object_count(table_path) == before_count,
        "#1300: selection toolbox creation planning should not mutate visual assets");

    const auto explicit_plan = copperfin::studio::plan_visual_object_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
        .path = table_path.string(),
        .toolbox_item_id = "commandbutton",
        .object_name = "cmdSave",
        .unique_id = "command-guid",
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Save"}
        }
    });
    expect(explicit_plan.ok &&
            explicit_plan.create_plan.ok &&
            explicit_plan.create_plan.plan.object_name == "cmdSave" &&
            explicit_plan.create_plan.plan.unique_id == "command-guid" &&
            explicit_plan.create_plan.plan.parent_name == "frmMain" &&
            has_field_value(explicit_plan.create_plan.plan.field_values, "UNIQUEID", "command-guid") &&
            has_field_value(explicit_plan.create_plan.plan.field_values, "PARENT", "frmMain") &&
            has_field_value(explicit_plan.create_plan.plan.field_values, "CAPTION", "Save"),
        "#1300: selection toolbox creation planning should preserve explicit identity and parent metadata");

    const auto report_plan = copperfin::studio::plan_visual_object_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
        .path = table_path.string(),
        .toolbox_item_id = "label",
        .object_name = {},
        .unique_id = {},
        .parent_name = "DetailBand",
        .field_values = {}
    });
    expect(report_plan.ok &&
            report_plan.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_plan.launch_plan.ok &&
            report_plan.create_plan.ok &&
            report_plan.create_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_plan.create_plan.plan.object_name == "lbl1" &&
            report_plan.create_plan.plan.parent_name == "DetailBand" &&
            has_field_value(report_plan.create_plan.plan.field_values, "CLASS", "Label"),
        "#1300: selection toolbox creation planning should resolve report selections to report-safe create plans");

    const auto unavailable_plan = copperfin::studio::plan_visual_object_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = {},
        .parent_name = "DetailBand",
        .field_values = {}
    });
    expect(!unavailable_plan.ok &&
            unavailable_plan.error == "The requested toolbox item is not available in the requested designer context." &&
            unavailable_plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            unavailable_plan.launch_plan.ok &&
            !unavailable_plan.create_plan.ok &&
            unavailable_plan.create_plan.plan.object_name.empty(),
        "#1300: selection toolbox creation planning should reject unavailable selected-context items");

    const auto unsupported_plan = copperfin::studio::plan_visual_object_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::menu_item,
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = {},
        .parent_name = {},
        .field_values = {}
    });
    expect(!unsupported_plan.ok &&
            unsupported_plan.error ==
                "A selection-context toolbox object creation plan request requires a toolbox palette." &&
            unsupported_plan.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item &&
            !unsupported_plan.launch_plan.ok &&
            !unsupported_plan.create_plan.ok &&
            unsupported_plan.create_plan.plan.object_name.empty() &&
            unsupported_plan.dry_run &&
            !unsupported_plan.mutates_asset,
        "#1300: selection toolbox creation planning should reject unsupported selections without stale plans");
    expect(object_count(table_path) == before_count,
        "#1300: rejected selection toolbox creation plans should not mutate visual assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_selection_create_executes_context_resolved_creates() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_create_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto visual_create = copperfin::studio::create_visual_object_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "selection-created-textbox-guid",
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Selection Created"}
        }
    });
    expect(visual_create.ok &&
            visual_create.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_create.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_create.launch_plan.ok &&
            visual_create.create_plan.ok &&
            visual_create.create_plan.create_plan.ok &&
            visual_create.create_result.ok &&
            visual_create.create_result.record_index == before_count &&
            visual_create.create_result.object_name == "txt2" &&
            visual_create.create_result.unique_id == "selection-created-textbox-guid" &&
            visual_create.create_result.parent_name == "frmMain" &&
            !visual_create.dry_run &&
            visual_create.mutates_asset,
        "#1308: visual selection toolbox creates should resolve form context and append one object");
    expect(visual_create.create_plan.create_plan.plan.object_name == "txt2" &&
            visual_create.create_plan.create_plan.plan.toolbox_context_provided &&
            visual_create.create_plan.create_plan.plan.toolbox_context ==
                copperfin::studio::StudioToolboxContext::form &&
            has_field_value(visual_create.create_plan.create_plan.plan.field_values, "CLASS", "TextBox") &&
            has_field_value(visual_create.create_plan.create_plan.plan.field_values, "CAPTION", "Selection Created"),
        "#1308: visual selection toolbox creates should preserve planning metadata and field values");
    const auto visual_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-created-textbox-guid",
        .property_name = "CAPTION"
    });
    expect(visual_caption.ok && visual_caption.exists && visual_caption.value == "Selection Created",
        "#1308: visual selection toolbox creates should persist caller-provided fields");
    expect(object_count(table_path) == before_count + 1U,
        "#1308: visual selection toolbox creates should mutate exactly once");

    const auto report_create = copperfin::studio::create_visual_object_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
        .path = table_path.string(),
        .toolbox_item_id = "label",
        .object_name = {},
        .unique_id = "selection-created-report-label-guid",
        .parent_name = "DetailBand",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Report Created"}
        }
    });
    expect(report_create.ok &&
            report_create.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_create.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_create.launch_plan.ok &&
            report_create.create_plan.ok &&
            report_create.create_result.ok &&
            report_create.create_result.record_index == before_count + 1U &&
            report_create.create_result.object_name == "lbl1" &&
            report_create.create_result.unique_id == "selection-created-report-label-guid" &&
            report_create.create_result.parent_name == "DetailBand" &&
            has_field_value(report_create.create_plan.create_plan.plan.field_values, "CLASS", "Label"),
        "#1308: report selection toolbox creates should resolve report context and append labels");
    const auto report_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-created-report-label-guid",
        .property_name = "CAPTION"
    });
    expect(report_caption.ok && report_caption.exists && report_caption.value == "Report Created",
        "#1308: report selection toolbox creates should persist caller-provided fields");
    expect(object_count(table_path) == before_count + 2U,
        "#1308: report selection toolbox creates should mutate exactly once");

    const std::size_t before_rejections_count = object_count(table_path);
    const auto unavailable_create = copperfin::studio::create_visual_object_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "selection-created-rejected-textbox-guid",
        .parent_name = "DetailBand",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Should Not Exist"}
        }
    });
    expect(!unavailable_create.ok &&
            unavailable_create.error ==
                "The requested toolbox item is not available in the requested designer context." &&
            unavailable_create.selection_context ==
                copperfin::studio::StudioEditorSelectionContext::report_expression &&
            unavailable_create.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            unavailable_create.launch_plan.ok &&
            !unavailable_create.create_plan.ok &&
            !unavailable_create.create_result.ok &&
            unavailable_create.create_result.object_name.empty() &&
            unavailable_create.create_result.unique_id.empty() &&
            unavailable_create.create_result.parent_name.empty() &&
            unavailable_create.dry_run &&
            !unavailable_create.mutates_asset,
        "#1308: unavailable selection toolbox creates should reject without stale identity metadata");
    expect(object_count(table_path) == before_rejections_count,
        "#1308: unavailable selection toolbox creates should not mutate assets");

    const auto unsupported_create = copperfin::studio::create_visual_object_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::menu_item,
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "selection-created-unsupported-guid",
        .parent_name = {},
        .field_values = {}
    });
    expect(!unsupported_create.ok &&
            unsupported_create.error ==
                "A selection-context toolbox object creation plan request requires a toolbox palette." &&
            unsupported_create.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item &&
            !unsupported_create.launch_plan.ok &&
            !unsupported_create.create_plan.ok &&
            !unsupported_create.create_result.ok &&
            unsupported_create.create_result.object_name.empty() &&
            unsupported_create.create_result.unique_id.empty() &&
            unsupported_create.create_result.parent_name.empty() &&
            unsupported_create.dry_run &&
            !unsupported_create.mutates_asset,
        "#1308: unsupported selection toolbox creates should reject without stale identity metadata");
    expect(object_count(table_path) == before_rejections_count,
        "#1308: unsupported selection toolbox creates should not mutate assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_selection_dispatch_planner_resolves_contexts_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_dispatch_plan_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto visual_dispatch = copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_selection({
        .create_request = {
            .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
            .path = table_path.string(),
            .toolbox_item_id = "textbox",
            .object_name = {},
            .unique_id = "selection-dispatch-textbox-guid",
            .parent_name = "frmMain",
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Selection Dispatch"}
            }
        },
        .admit_create_operation = true
    });
    expect(visual_dispatch.ok &&
            visual_dispatch.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_dispatch.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_dispatch.launch_plan.ok &&
            visual_dispatch.create_plan.ok &&
            visual_dispatch.dispatch.ok &&
            visual_dispatch.dispatch_count == 1U &&
            visual_dispatch.error_count == 0U &&
            !visual_dispatch.dry_run &&
            visual_dispatch.mutates_asset &&
            visual_dispatch.create_plan.create_plan.plan.object_name == "txt2" &&
            visual_dispatch.dispatch.plan.object_name == "txt2" &&
            visual_dispatch.dispatch.plan.unique_id == "selection-dispatch-textbox-guid" &&
            visual_dispatch.dispatch.plan.parent_name == "frmMain",
        "#1302: selection toolbox create-dispatch planning should resolve visual selections to admitted form dispatch plans");
    expect(has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--path", table_path.string()) &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-create", "textbox") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--object-name", "txt2") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--unique-id",
                "selection-dispatch-textbox-guid") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--parent-name", "frmMain") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--field-value",
                "CAPTION=Selection Dispatch"),
        "#1302: selection toolbox create-dispatch planning should preserve deterministic dispatch arguments");
    expect(object_count(table_path) == before_count,
        "#1302: selection toolbox create-dispatch planning should not mutate visual assets");

    const auto non_admitted_dispatch =
        copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_selection({
            .create_request = {
                .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
                .path = table_path.string(),
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = {},
                .parent_name = "frmMain",
                .field_values = {}
            },
            .admit_create_operation = false
        });
    expect(!non_admitted_dispatch.ok &&
            non_admitted_dispatch.error ==
                "A toolbox create dispatch request requires an admitted non-dry-run create operation." &&
            non_admitted_dispatch.create_plan.ok &&
            !non_admitted_dispatch.dispatch.ok &&
            non_admitted_dispatch.dispatch.plan.dispatch_arguments.empty() &&
            non_admitted_dispatch.dispatch_count == 0U &&
            non_admitted_dispatch.error_count == 1U &&
            non_admitted_dispatch.dry_run &&
            !non_admitted_dispatch.mutates_asset,
        "#1302: selection toolbox create-dispatch planning should reject non-admitted creates without stale arguments");

    const auto report_dispatch = copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_selection({
        .create_request = {
            .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
            .path = table_path.string(),
            .toolbox_item_id = "label",
            .object_name = {},
            .unique_id = {},
            .parent_name = "DetailBand",
            .field_values = {}
        },
        .admit_create_operation = true
    });
    expect(report_dispatch.ok &&
            report_dispatch.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_dispatch.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_dispatch.create_plan.ok &&
            report_dispatch.dispatch.ok &&
            report_dispatch.dispatch.plan.object_name == "lbl1" &&
            report_dispatch.dispatch.plan.parent_name == "DetailBand" &&
            has_argument_pair(report_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-create", "label") &&
            has_argument_pair(report_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-context", "report"),
        "#1302: selection toolbox create-dispatch planning should resolve report selections to report-safe dispatches");

    const auto unavailable_dispatch =
        copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_selection({
            .create_request = {
                .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
                .path = table_path.string(),
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = {},
                .parent_name = "DetailBand",
                .field_values = {}
            },
            .admit_create_operation = true
        });
    expect(!unavailable_dispatch.ok &&
            unavailable_dispatch.error ==
                "The requested toolbox item is not available in the requested designer context." &&
            unavailable_dispatch.launch_plan.ok &&
            !unavailable_dispatch.create_plan.ok &&
            !unavailable_dispatch.dispatch.ok &&
            unavailable_dispatch.dispatch.plan.dispatch_arguments.empty() &&
            unavailable_dispatch.dispatch_count == 0U &&
            unavailable_dispatch.error_count == 1U,
        "#1302: selection toolbox create-dispatch planning should reject unavailable selected-context items");

    const auto unsupported_dispatch =
        copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_selection({
            .create_request = {
                .selection_context = copperfin::studio::StudioEditorSelectionContext::menu_item,
                .path = table_path.string(),
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            },
            .admit_create_operation = true
        });
    expect(!unsupported_dispatch.ok &&
            unsupported_dispatch.error ==
                "A selection-context toolbox object creation plan request requires a toolbox palette." &&
            unsupported_dispatch.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item &&
            !unsupported_dispatch.launch_plan.ok &&
            !unsupported_dispatch.create_plan.ok &&
            !unsupported_dispatch.dispatch.ok &&
            unsupported_dispatch.dispatch.plan.dispatch_arguments.empty() &&
            unsupported_dispatch.dispatch_count == 0U &&
            unsupported_dispatch.error_count == 1U &&
            unsupported_dispatch.dry_run &&
            !unsupported_dispatch.mutates_asset,
        "#1302: selection toolbox create-dispatch planning should reject unsupported selections without stale plans");
    expect(object_count(table_path) == before_count,
        "#1302: rejected selection toolbox create-dispatch plans should not mutate visual assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_selection_batch_planner_resolves_contexts_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_batch_plan_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto visual_batch = copperfin::studio::plan_visual_objects_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
        .path = table_path.string(),
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "selection-batch-textbox-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "First Selection Batch"}
                }
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "cmdSave",
                .unique_id = "selection-batch-command-guid",
                .parent_name = "cntToolbar",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Run Selection Batch"}
                }
            },
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = {},
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Second Selection Batch"}
                }
            }
        }
    });
    const auto* visual_textbox_plan = find_create_batch_plan(visual_batch.batch_plan.plan.plans, "textbox");
    const auto* visual_command_plan = find_create_batch_plan(visual_batch.batch_plan.plan.plans, "commandbutton");
    const auto& visual_plans = visual_batch.batch_plan.plan.plans;

    expect(visual_batch.ok &&
            visual_batch.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_batch.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_batch.launch_plan.ok &&
            visual_batch.launch_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_batch.item_count == 3U &&
            visual_batch.plan_count == 1U &&
            visual_batch.error_count == 0U &&
            visual_batch.batch_plan.ok &&
            visual_batch.batch_plan.plan.toolbox_context_provided &&
            visual_batch.batch_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_batch.batch_plan.plan.item_count == 3U &&
            visual_batch.dry_run &&
            !visual_batch.mutates_asset,
        "#1304: visual selection toolbox batch create planning should summarize ordered form batches");
    expect(visual_plans.size() == 3U &&
            visual_plans[0].toolbox_item.id == "textbox" &&
            visual_plans[0].object_name == "txt2" &&
            visual_plans[0].unique_id == "selection-batch-textbox-guid" &&
            visual_plans[0].parent_name == "frmMain" &&
            has_field_value(visual_plans[0].field_values, "CLASS", "TextBox") &&
            has_field_value(visual_plans[0].field_values, "CAPTION", "First Selection Batch") &&
            visual_plans[1].toolbox_item.id == "commandbutton" &&
            visual_plans[1].object_name == "cmdSave" &&
            visual_plans[1].unique_id == "selection-batch-command-guid" &&
            visual_plans[1].parent_name == "cntToolbar" &&
            has_field_value(visual_plans[1].field_values, "CLASS", "CommandButton") &&
            has_field_value(visual_plans[1].field_values, "CAPTION", "Run Selection Batch") &&
            visual_plans[2].toolbox_item.id == "textbox" &&
            visual_plans[2].object_name == "txt3" &&
            visual_plans[2].parent_name == "frmMain" &&
            has_field_value(visual_plans[2].field_values, "CLASS", "TextBox") &&
            has_field_value(visual_plans[2].field_values, "CAPTION", "Second Selection Batch"),
        "#1304: visual selection toolbox batch create planning should preserve ordered identity and field metadata");
    expect(visual_textbox_plan != nullptr &&
            visual_command_plan != nullptr &&
            visual_plans[0].target_record_index == before_count &&
            visual_plans[1].target_record_index > visual_plans[0].target_record_index &&
            visual_plans[2].target_record_index > visual_plans[1].target_record_index,
        "#1304: visual selection toolbox batch create planning should reserve names and target records in order");
    expect(object_count(table_path) == before_count,
        "#1304: visual selection toolbox batch create planning should not mutate assets");

    const auto report_batch = copperfin::studio::plan_visual_objects_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
        .path = table_path.string(),
        .items = {
            {
                .toolbox_item_id = "label",
                .object_name = {},
                .unique_id = "report-selection-batch-label-guid",
                .parent_name = "DetailBand",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Report Selection Batch"}
                }
            }
        }
    });
    expect(report_batch.ok &&
            report_batch.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_batch.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_batch.launch_plan.ok &&
            report_batch.item_count == 1U &&
            report_batch.plan_count == 1U &&
            report_batch.error_count == 0U &&
            report_batch.batch_plan.ok &&
            report_batch.batch_plan.plan.plans.size() == 1U &&
            report_batch.batch_plan.plan.plans[0].toolbox_item.id == "label" &&
            report_batch.batch_plan.plan.plans[0].object_name == "lbl1" &&
            report_batch.batch_plan.plan.plans[0].unique_id == "report-selection-batch-label-guid" &&
            report_batch.batch_plan.plan.plans[0].parent_name == "DetailBand" &&
            has_field_value(report_batch.batch_plan.plan.plans[0].field_values, "CLASS", "Label") &&
            has_field_value(report_batch.batch_plan.plan.plans[0].field_values, "CAPTION", "Report Selection Batch"),
        "#1304: report selection toolbox batch create planning should resolve report-safe batches");
    expect(object_count(table_path) == before_count,
        "#1304: report selection toolbox batch create planning should not mutate assets");

    const auto unavailable_batch = copperfin::studio::plan_visual_objects_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
        .path = table_path.string(),
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = {},
                .parent_name = "DetailBand",
                .field_values = {}
            }
        }
    });
    expect(!unavailable_batch.ok &&
            unavailable_batch.error ==
                "The requested toolbox item is not available in the requested designer context." &&
            unavailable_batch.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression &&
            unavailable_batch.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            unavailable_batch.launch_plan.ok &&
            unavailable_batch.item_count == 1U &&
            unavailable_batch.plan_count == 0U &&
            unavailable_batch.error_count == 1U &&
            unavailable_batch.dry_run &&
            !unavailable_batch.mutates_asset &&
            !unavailable_batch.batch_plan.ok &&
            unavailable_batch.batch_plan.plan.plans.empty(),
        "#1304: unavailable selection toolbox batch create planning should reject without stale plans");

    const auto unsupported_batch = copperfin::studio::plan_visual_objects_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::menu_item,
        .path = table_path.string(),
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = {},
                .parent_name = {},
                .field_values = {}
            }
        }
    });
    expect(!unsupported_batch.ok &&
            unsupported_batch.error ==
                "A selection-context toolbox object batch creation plan request requires a toolbox palette." &&
            unsupported_batch.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item &&
            !unsupported_batch.launch_plan.ok &&
            unsupported_batch.launch_plan.error == "The selected Studio context does not expose a toolbox palette." &&
            unsupported_batch.item_count == 0U &&
            unsupported_batch.plan_count == 0U &&
            unsupported_batch.error_count == 0U &&
            unsupported_batch.dry_run &&
            !unsupported_batch.mutates_asset &&
            unsupported_batch.batch_plan.plan.plans.empty(),
        "#1304: unsupported selection toolbox batch create planning should reject without stale plans");

    expect(object_count(table_path) == before_count,
        "#1304: selection toolbox batch create planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}


}
