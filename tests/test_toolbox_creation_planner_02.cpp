#include "test_toolbox_creation_support.h"

namespace copperfin::toolbox_creation_tests
{
void test_toolbox_creation_selection_batch_create_executes_context_resolved_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_batch_create_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto visual_batch = copperfin::studio::create_visual_objects_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
        .path = table_path.string(),
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "selection-batch-created-textbox-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "First Selection Batch Created"}
                }
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "cmdSelectionBatchCreate",
                .unique_id = "selection-batch-created-command-guid",
                .parent_name = "cntToolbar",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Run Selection Batch Created"}
                }
            },
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "selection-batch-created-second-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Second Selection Batch Created"}
                }
            }
        }
    });
    const auto& visual_plans = visual_batch.batch_plan.batch_plan.plan.plans;

    expect(visual_batch.ok &&
            visual_batch.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_batch.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_batch.launch_plan.ok &&
            visual_batch.item_count == 3U &&
            visual_batch.batch_plan.ok &&
            visual_batch.batch_plan.batch_plan.ok &&
            visual_batch.batch_plan.plan_count == 1U &&
            visual_batch.batch_plan.error_count == 0U &&
            visual_batch.create_result.ok &&
            visual_batch.create_result.record_indexes.size() == 3U &&
            visual_batch.create_result.record_indexes[0] == before_count &&
            visual_batch.create_result.record_indexes[1] == before_count + 1U &&
            visual_batch.create_result.record_indexes[2] == before_count + 2U &&
            !visual_batch.dry_run &&
            visual_batch.mutates_asset,
        "#1310: visual selection toolbox batch creates should resolve form context and append ordered objects");
    expect(visual_plans.size() == 3U &&
            visual_plans[0].toolbox_item.id == "textbox" &&
            visual_plans[0].object_name == "txt2" &&
            visual_plans[0].unique_id == "selection-batch-created-textbox-guid" &&
            visual_plans[0].parent_name == "frmMain" &&
            has_field_value(visual_plans[0].field_values, "CLASS", "TextBox") &&
            has_field_value(visual_plans[0].field_values, "CAPTION", "First Selection Batch Created") &&
            visual_plans[1].toolbox_item.id == "commandbutton" &&
            visual_plans[1].object_name == "cmdSelectionBatchCreate" &&
            visual_plans[1].unique_id == "selection-batch-created-command-guid" &&
            visual_plans[1].parent_name == "cntToolbar" &&
            has_field_value(visual_plans[1].field_values, "CLASS", "CommandButton") &&
            has_field_value(visual_plans[1].field_values, "CAPTION", "Run Selection Batch Created") &&
            visual_plans[2].toolbox_item.id == "textbox" &&
            visual_plans[2].object_name == "txt3" &&
            visual_plans[2].unique_id == "selection-batch-created-second-guid" &&
            visual_plans[2].parent_name == "frmMain" &&
            has_field_value(visual_plans[2].field_values, "CLASS", "TextBox") &&
            has_field_value(visual_plans[2].field_values, "CAPTION", "Second Selection Batch Created"),
        "#1310: visual selection toolbox batch creates should preserve ordered planning metadata");
    expect(visual_batch.create_result.created_objects.size() == 3U &&
            visual_batch.create_result.created_objects[0].object_name == "txt2" &&
            visual_batch.create_result.created_objects[0].unique_id == "selection-batch-created-textbox-guid" &&
            visual_batch.create_result.created_objects[0].parent_name == "frmMain" &&
            visual_batch.create_result.created_objects[1].object_name == "cmdSelectionBatchCreate" &&
            visual_batch.create_result.created_objects[1].unique_id == "selection-batch-created-command-guid" &&
            visual_batch.create_result.created_objects[1].parent_name == "cntToolbar" &&
            visual_batch.create_result.created_objects[2].object_name == "txt3" &&
            visual_batch.create_result.created_objects[2].unique_id == "selection-batch-created-second-guid" &&
            visual_batch.create_result.created_objects[2].parent_name == "frmMain",
        "#1310: visual selection toolbox batch creates should report created identity metadata in order");

    const auto first_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-created-textbox-guid",
        .property_name = "CAPTION"
    });
    const auto command_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-created-command-guid",
        .property_name = "CAPTION"
    });
    const auto second_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-created-second-guid",
        .property_name = "CAPTION"
    });
    expect(first_caption.ok && first_caption.exists && first_caption.value == "First Selection Batch Created" &&
            command_caption.ok && command_caption.exists && command_caption.value == "Run Selection Batch Created" &&
            second_caption.ok && second_caption.exists && second_caption.value == "Second Selection Batch Created",
        "#1310: visual selection toolbox batch creates should persist caller-provided fields");
    expect(object_count(table_path) == before_count + 3U,
        "#1310: visual selection toolbox batch creates should mutate once per accepted item");

    const std::size_t before_report_count = object_count(table_path);
    const auto report_batch = copperfin::studio::create_visual_objects_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
        .path = table_path.string(),
        .items = {
            {
                .toolbox_item_id = "label",
                .object_name = {},
                .unique_id = "report-selection-batch-created-label-guid",
                .parent_name = "DetailBand",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Report Selection Batch Created"}
                }
            }
        }
    });
    expect(report_batch.ok &&
            report_batch.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_batch.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_batch.launch_plan.ok &&
            report_batch.item_count == 1U &&
            report_batch.batch_plan.ok &&
            report_batch.create_result.ok &&
            report_batch.create_result.record_indexes.size() == 1U &&
            report_batch.create_result.record_indexes[0] == before_report_count &&
            report_batch.create_result.created_objects.size() == 1U &&
            report_batch.create_result.created_objects[0].object_name == "lbl1" &&
            report_batch.create_result.created_objects[0].unique_id == "report-selection-batch-created-label-guid" &&
            report_batch.create_result.created_objects[0].parent_name == "DetailBand" &&
            !report_batch.dry_run &&
            report_batch.mutates_asset,
        "#1310: report selection toolbox batch creates should resolve report context and append labels");
    const auto report_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "report-selection-batch-created-label-guid",
        .property_name = "CAPTION"
    });
    expect(report_caption.ok && report_caption.exists && report_caption.value == "Report Selection Batch Created",
        "#1310: report selection toolbox batch creates should persist caller-provided fields");

    const std::size_t before_rejections_count = object_count(table_path);
    const auto unavailable_batch = copperfin::studio::create_visual_objects_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
        .path = table_path.string(),
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "report-selection-batch-rejected-textbox-guid",
                .parent_name = "DetailBand",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Should Not Exist"}
                }
            }
        }
    });
    expect(!unavailable_batch.ok &&
            unavailable_batch.error ==
                "The requested toolbox item is not available in the requested designer context." &&
            unavailable_batch.selection_context ==
                copperfin::studio::StudioEditorSelectionContext::report_expression &&
            unavailable_batch.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            unavailable_batch.launch_plan.ok &&
            unavailable_batch.item_count == 1U &&
            !unavailable_batch.batch_plan.ok &&
            !unavailable_batch.create_result.ok &&
            unavailable_batch.create_result.record_indexes.empty() &&
            unavailable_batch.create_result.created_objects.empty() &&
            unavailable_batch.dry_run &&
            !unavailable_batch.mutates_asset,
        "#1310: unavailable selection toolbox batch creates should reject without stale create metadata");
    expect(object_count(table_path) == before_rejections_count,
        "#1310: unavailable selection toolbox batch creates should not mutate assets");

    const auto unsupported_batch = copperfin::studio::create_visual_objects_from_toolbox_selection({
        .selection_context = copperfin::studio::StudioEditorSelectionContext::menu_item,
        .path = table_path.string(),
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "unsupported-selection-batch-rejected-guid",
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
            !unsupported_batch.batch_plan.ok &&
            !unsupported_batch.create_result.ok &&
            unsupported_batch.create_result.record_indexes.empty() &&
            unsupported_batch.create_result.created_objects.empty() &&
            unsupported_batch.dry_run &&
            !unsupported_batch.mutates_asset,
        "#1310: unsupported selection toolbox batch creates should reject without stale create metadata");
    expect(object_count(table_path) == before_rejections_count,
        "#1310: unsupported selection toolbox batch creates should not mutate assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_selection_batch_dispatch_planner_resolves_contexts_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_selection_batch_dispatch_plan_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto visual_dispatch =
        copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_selection({
            .batch_request = {
                .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
                .path = table_path.string(),
                .items = {
                    {
                        .toolbox_item_id = "textbox",
                        .object_name = {},
                        .unique_id = "selection-batch-dispatch-textbox-guid",
                        .parent_name = "frmMain",
                        .field_values = {
                            {.property_name = "CAPTION", .property_value = "First Selection Dispatch"}
                        }
                    },
                    {
                        .toolbox_item_id = "commandbutton",
                        .object_name = "cmdSelectionDispatch",
                        .unique_id = "selection-batch-dispatch-command-guid",
                        .parent_name = "cntToolbar",
                        .field_values = {
                            {.property_name = "CAPTION", .property_value = "Run Selection Dispatch"}
                        }
                    },
                    {
                        .toolbox_item_id = "textbox",
                        .object_name = {},
                        .unique_id = {},
                        .parent_name = "frmMain",
                        .field_values = {
                            {.property_name = "CAPTION", .property_value = "Second Selection Dispatch"}
                        }
                    }
                }
            },
            .admit_create_operation = true
        });
    const auto& visual_plans = visual_dispatch.dispatch.plan.plans;

    expect(visual_dispatch.ok &&
            visual_dispatch.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object &&
            visual_dispatch.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_dispatch.launch_plan.ok &&
            visual_dispatch.item_count == 3U &&
            visual_dispatch.batch_plan.ok &&
            visual_dispatch.batch_plan.batch_plan.ok &&
            visual_dispatch.dispatch.ok &&
            visual_dispatch.dispatch_count == 1U &&
            visual_dispatch.error_count == 0U &&
            !visual_dispatch.dry_run &&
            visual_dispatch.mutates_asset &&
            visual_dispatch.dispatch.plan.dispatch_admitted &&
            !visual_dispatch.dispatch.plan.executed &&
            visual_dispatch.dispatch.plan.mutates_asset,
        "#1306: admitted visual selection toolbox batch create dispatch planning should summarize form batches");
    expect(visual_plans.size() == 3U &&
            visual_plans[0].toolbox_item.id == "textbox" &&
            visual_plans[0].object_name == "txt2" &&
            visual_plans[0].unique_id == "selection-batch-dispatch-textbox-guid" &&
            visual_plans[0].parent_name == "frmMain" &&
            has_field_value(visual_plans[0].field_values, "CAPTION", "First Selection Dispatch") &&
            visual_plans[1].toolbox_item.id == "commandbutton" &&
            visual_plans[1].object_name == "cmdSelectionDispatch" &&
            visual_plans[1].unique_id == "selection-batch-dispatch-command-guid" &&
            visual_plans[1].parent_name == "cntToolbar" &&
            has_field_value(visual_plans[1].field_values, "CAPTION", "Run Selection Dispatch") &&
            visual_plans[2].toolbox_item.id == "textbox" &&
            visual_plans[2].object_name == "txt3" &&
            visual_plans[2].parent_name == "frmMain" &&
            has_field_value(visual_plans[2].field_values, "CAPTION", "Second Selection Dispatch"),
        "#1306: admitted visual selection toolbox batch create dispatch planning should preserve ordered metadata");
    expect(visual_plans[0].target_record_index == before_count &&
            visual_plans[1].target_record_index > visual_plans[0].target_record_index &&
            visual_plans[2].target_record_index > visual_plans[1].target_record_index,
        "#1306: admitted visual selection toolbox batch create dispatch planning should reserve target records");
    expect(has_argument(visual_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-create-batch") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--path", table_path.string()) &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-item", "textbox") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--object-name", "txt2") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--unique-id",
                "selection-batch-dispatch-textbox-guid") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--parent-name", "cntToolbar") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--field-value",
                "CAPTION=Run Selection Dispatch") &&
            has_argument_pair(visual_dispatch.dispatch.plan.dispatch_arguments, "--object-name", "txt3"),
        "#1306: admitted visual selection toolbox batch create dispatch planning should materialize arguments");
    expect(object_count(table_path) == before_count,
        "#1306: admitted visual selection toolbox batch create dispatch planning should not mutate assets");

    const auto non_admitted_dispatch =
        copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_selection({
            .batch_request = {
                .selection_context = copperfin::studio::StudioEditorSelectionContext::visual_object,
                .path = table_path.string(),
                .items = {
                    {
                        .toolbox_item_id = "textbox",
                        .object_name = {},
                        .unique_id = {},
                        .parent_name = "frmMain",
                        .field_values = {}
                    }
                }
            },
            .admit_create_operation = false
        });
    expect(!non_admitted_dispatch.ok &&
            non_admitted_dispatch.batch_plan.ok &&
            non_admitted_dispatch.batch_plan.batch_plan.ok &&
            !non_admitted_dispatch.dispatch.ok &&
            non_admitted_dispatch.dispatch.error ==
                "A toolbox batch create dispatch request requires an admitted non-dry-run create operation." &&
            non_admitted_dispatch.dispatch.plan.dispatch_arguments.empty() &&
            non_admitted_dispatch.dispatch_count == 0U &&
            non_admitted_dispatch.error_count == 1U &&
            non_admitted_dispatch.dry_run &&
            !non_admitted_dispatch.mutates_asset,
        "#1306: non-admitted selection toolbox batch create dispatch planning should reject without stale args");

    const auto report_dispatch =
        copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_selection({
            .batch_request = {
                .selection_context = copperfin::studio::StudioEditorSelectionContext::report_expression,
                .path = table_path.string(),
                .items = {
                    {
                        .toolbox_item_id = "label",
                        .object_name = {},
                        .unique_id = "report-selection-batch-dispatch-label-guid",
                        .parent_name = "DetailBand",
                        .field_values = {
                            {.property_name = "CAPTION", .property_value = "Report Selection Dispatch"}
                        }
                    }
                }
            },
            .admit_create_operation = true
        });
    expect(report_dispatch.ok &&
            report_dispatch.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression &&
            report_dispatch.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_dispatch.launch_plan.ok &&
            report_dispatch.dispatch.ok &&
            report_dispatch.dispatch.plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_dispatch.dispatch.plan.plans.size() == 1U &&
            report_dispatch.dispatch.plan.plans[0].toolbox_item.id == "label" &&
            report_dispatch.dispatch.plan.plans[0].object_name == "lbl1" &&
            report_dispatch.dispatch.plan.plans[0].unique_id == "report-selection-batch-dispatch-label-guid" &&
            has_argument_pair(report_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-context", "report") &&
            has_argument_pair(report_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-item", "label") &&
            !has_argument_pair(report_dispatch.dispatch.plan.dispatch_arguments, "--toolbox-item", "textbox"),
        "#1306: admitted report selection toolbox batch create dispatch planning should resolve report-safe batches");
    expect(object_count(table_path) == before_count,
        "#1306: admitted report selection toolbox batch create dispatch planning should not mutate assets");

    const auto unavailable_dispatch =
        copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_selection({
            .batch_request = {
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
            },
            .admit_create_operation = true
        });
    expect(!unavailable_dispatch.ok &&
            unavailable_dispatch.error ==
                "The requested toolbox item is not available in the requested designer context." &&
            unavailable_dispatch.selection_context ==
                copperfin::studio::StudioEditorSelectionContext::report_expression &&
            unavailable_dispatch.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            unavailable_dispatch.launch_plan.ok &&
            unavailable_dispatch.item_count == 1U &&
            !unavailable_dispatch.batch_plan.ok &&
            !unavailable_dispatch.dispatch.ok &&
            unavailable_dispatch.dispatch.plan.dispatch_arguments.empty() &&
            unavailable_dispatch.dispatch_count == 0U &&
            unavailable_dispatch.error_count == 1U,
        "#1306: unavailable selection toolbox batch create dispatch planning should reject without stale dispatches");

    const auto unsupported_dispatch =
        copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_selection({
            .batch_request = {
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
            },
            .admit_create_operation = true
        });
    expect(!unsupported_dispatch.ok &&
            unsupported_dispatch.error ==
                "A selection-context toolbox object batch creation plan request requires a toolbox palette." &&
            unsupported_dispatch.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item &&
            !unsupported_dispatch.launch_plan.ok &&
            unsupported_dispatch.item_count == 0U &&
            !unsupported_dispatch.batch_plan.ok &&
            !unsupported_dispatch.dispatch.ok &&
            unsupported_dispatch.dispatch.plan.dispatch_arguments.empty() &&
            unsupported_dispatch.dispatch_count == 0U &&
            unsupported_dispatch.error_count == 0U,
        "#1306: unsupported selection toolbox batch create dispatch planning should reject without stale dispatches");

    expect(object_count(table_path) == before_count,
        "#1306: selection toolbox batch create dispatch planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}


}
