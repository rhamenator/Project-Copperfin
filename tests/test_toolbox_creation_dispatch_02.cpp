#include "test_toolbox_creation_support.h"

namespace copperfin::toolbox_creation_tests
{
void test_toolbox_creation_batch_create_from_dispatch_executes_admitted_dispatches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_dispatch_source_create_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto dispatch_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "frmMain",
        .unique_id = "form-guid",
        .admit_palette_invocation = true
    });
    expect(dispatch_catalog.ok && dispatch_catalog.dispatch.ok,
        "#1313: toolbox batch create-from-dispatch execution fixture should produce admitted dispatches");

    const auto visual_batch = copperfin::studio::create_visual_objects_from_toolbox_dispatch({
        .dispatch_plan = dispatch_catalog.dispatch.plan,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "dispatch-batch-created-first-guid",
                .parent_name = {},
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "First Dispatch Batch"}
                }
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "cmdDispatchBatchCreate",
                .unique_id = "dispatch-batch-created-command-guid",
                .parent_name = "cntToolbar",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Run Dispatch Batch"}
                }
            },
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "dispatch-batch-created-second-guid",
                .parent_name = {},
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Second Dispatch Batch"}
                }
            }
        }
    });
    const auto& visual_plans = visual_batch.batch_plan.plan.plans;
    expect(visual_batch.ok &&
            visual_batch.batch_plan.ok &&
            visual_batch.create_result.ok &&
            visual_batch.batch_plan.plan.toolbox_context_provided &&
            visual_batch.batch_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_batch.batch_plan.plan.item_count == 3U &&
            visual_batch.create_result.record_indexes.size() == 3U &&
            visual_batch.create_result.record_indexes[0] == before_count &&
            visual_batch.create_result.record_indexes[1] == before_count + 1U &&
            visual_batch.create_result.record_indexes[2] == before_count + 2U &&
            !visual_batch.dry_run &&
            visual_batch.mutates_asset,
        "#1313: toolbox batch create-from-dispatch execution should append ordered form objects");
    expect(visual_plans.size() == 3U &&
            visual_plans[0].toolbox_item.id == "textbox" &&
            visual_plans[0].object_name == "txt2" &&
            visual_plans[0].unique_id == "dispatch-batch-created-first-guid" &&
            visual_plans[0].parent_name == "frmMain" &&
            has_field_value(visual_plans[0].field_values, "CAPTION", "First Dispatch Batch") &&
            visual_plans[1].toolbox_item.id == "commandbutton" &&
            visual_plans[1].object_name == "cmdDispatchBatchCreate" &&
            visual_plans[1].unique_id == "dispatch-batch-created-command-guid" &&
            visual_plans[1].parent_name == "cntToolbar" &&
            has_field_value(visual_plans[1].field_values, "CAPTION", "Run Dispatch Batch") &&
            visual_plans[2].toolbox_item.id == "textbox" &&
            visual_plans[2].object_name == "txt3" &&
            visual_plans[2].unique_id == "dispatch-batch-created-second-guid" &&
            visual_plans[2].parent_name == "frmMain" &&
            has_field_value(visual_plans[2].field_values, "CAPTION", "Second Dispatch Batch"),
        "#1313: toolbox batch create-from-dispatch execution should preserve ordered planned metadata");
    expect(visual_batch.create_result.created_objects.size() == 3U &&
            visual_batch.create_result.created_objects[0].object_name == "txt2" &&
            visual_batch.create_result.created_objects[0].unique_id == "dispatch-batch-created-first-guid" &&
            visual_batch.create_result.created_objects[0].parent_name == "frmMain" &&
            visual_batch.create_result.created_objects[1].object_name == "cmdDispatchBatchCreate" &&
            visual_batch.create_result.created_objects[1].unique_id == "dispatch-batch-created-command-guid" &&
            visual_batch.create_result.created_objects[1].parent_name == "cntToolbar" &&
            visual_batch.create_result.created_objects[2].object_name == "txt3" &&
            visual_batch.create_result.created_objects[2].unique_id == "dispatch-batch-created-second-guid" &&
            visual_batch.create_result.created_objects[2].parent_name == "frmMain",
        "#1313: toolbox batch create-from-dispatch execution should report created identity metadata");
    const auto first_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dispatch-batch-created-first-guid",
        .property_name = "CAPTION"
    });
    const auto command_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dispatch-batch-created-command-guid",
        .property_name = "CAPTION"
    });
    const auto second_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dispatch-batch-created-second-guid",
        .property_name = "CAPTION"
    });
    expect(first_caption.ok && first_caption.exists && first_caption.value == "First Dispatch Batch" &&
            command_caption.ok && command_caption.exists && command_caption.value == "Run Dispatch Batch" &&
            second_caption.ok && second_caption.exists && second_caption.value == "Second Dispatch Batch",
        "#1313: toolbox batch create-from-dispatch execution should persist caller fields");

    const auto report_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "DetailBand",
        .unique_id = "report-guid",
        .admit_palette_invocation = true
    });
    expect(report_catalog.ok && report_catalog.dispatch.ok,
        "#1313: report toolbox batch create-from-dispatch execution fixture should produce admitted dispatches");
    const auto report_batch = copperfin::studio::create_visual_objects_from_toolbox_dispatch({
        .dispatch_plan = report_catalog.dispatch.plan,
        .items = {
            {
                .toolbox_item_id = "label",
                .object_name = {},
                .unique_id = "dispatch-batch-created-report-label-guid",
                .parent_name = {},
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Report Dispatch Batch"}
                }
            }
        }
    });
    expect(report_batch.ok &&
            report_batch.batch_plan.ok &&
            report_batch.create_result.ok &&
            report_batch.batch_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_batch.batch_plan.plan.plans.size() == 1U &&
            report_batch.batch_plan.plan.plans[0].object_name == "lbl1" &&
            report_batch.batch_plan.plan.plans[0].parent_name == "DetailBand" &&
            report_batch.create_result.record_indexes.size() == 1U &&
            report_batch.create_result.record_indexes[0] == before_count + 3U &&
            report_batch.create_result.created_objects.size() == 1U &&
            report_batch.create_result.created_objects[0].object_name == "lbl1" &&
            report_batch.create_result.created_objects[0].unique_id == "dispatch-batch-created-report-label-guid" &&
            report_batch.create_result.created_objects[0].parent_name == "DetailBand",
        "#1313: toolbox batch create-from-dispatch execution should append report-safe objects");

    const std::size_t committed_count = object_count(table_path);
    const auto dry_run_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "frmMain",
        .unique_id = "form-guid",
        .admit_palette_invocation = false
    });
    expect(dry_run_catalog.ok && !dry_run_catalog.dispatch.ok,
        "#1313: dry-run toolbox batch create-from-dispatch execution fixture should preserve rejected dispatch state");
    auto dry_run_plan = dry_run_catalog.invocation_admission.plan;
    const auto dry_run_batch = copperfin::studio::create_visual_objects_from_toolbox_dispatch({
        .dispatch_plan = {
            .selection_context = dry_run_plan.selection_context,
            .toolbox_context = dry_run_plan.toolbox_context,
            .command_token = dry_run_plan.command_token,
            .asset_path = dry_run_plan.asset_path,
            .record_index = dry_run_plan.record_index,
            .object_name = dry_run_plan.object_name,
            .unique_id = dry_run_plan.unique_id,
            .item_count = dry_run_plan.item_count,
            .items = dry_run_plan.items,
            .dispatch_arguments = {},
            .dispatch_admitted = false,
            .dry_run = true,
            .executed = false,
            .mutates_asset = false
        },
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "dry-run-dispatch-batch-guid",
                .parent_name = {},
                .field_values = {}
            }
        }
    });
    expect(!dry_run_batch.ok &&
            dry_run_batch.error ==
                "A toolbox batch create-from-dispatch request requires an admitted non-executed toolbox dispatch." &&
            !dry_run_batch.batch_plan.ok &&
            !dry_run_batch.create_result.ok &&
            dry_run_batch.create_result.record_indexes.empty() &&
            dry_run_batch.create_result.created_objects.empty() &&
            dry_run_batch.dry_run &&
            !dry_run_batch.mutates_asset,
        "#1313: toolbox batch create-from-dispatch execution should reject dry-run dispatches cleanly");
    expect(object_count(table_path) == committed_count,
        "#1313: rejected dry-run batch create-from-dispatch execution should not mutate assets");

    const auto unavailable_batch = copperfin::studio::create_visual_objects_from_toolbox_dispatch({
        .dispatch_plan = report_catalog.dispatch.plan,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "unavailable-dispatch-batch-guid",
                .parent_name = {},
                .field_values = {}
            }
        }
    });
    expect(!unavailable_batch.ok &&
            unavailable_batch.error == "The requested toolbox item is not available in the admitted toolbox dispatch." &&
            !unavailable_batch.batch_plan.ok &&
            !unavailable_batch.create_result.ok &&
            unavailable_batch.create_result.record_indexes.empty() &&
            unavailable_batch.create_result.created_objects.empty() &&
            unavailable_batch.dry_run &&
            !unavailable_batch.mutates_asset,
        "#1313: toolbox batch create-from-dispatch execution should reject unavailable dispatch items");
    expect(object_count(table_path) == committed_count,
        "#1313: unavailable batch create-from-dispatch execution should not mutate assets");

    const auto duplicate_batch = copperfin::studio::create_visual_objects_from_toolbox_dispatch({
        .dispatch_plan = dispatch_catalog.dispatch.plan,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = "dupDispatchBatch",
                .unique_id = "dup-dispatch-batch-guid-1",
                .parent_name = {},
                .field_values = {}
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "dupDispatchBatch",
                .unique_id = "dup-dispatch-batch-guid-2",
                .parent_name = {},
                .field_values = {}
            }
        }
    });
    expect(!duplicate_batch.ok &&
            duplicate_batch.error == "The requested toolbox object identity already exists in the asset." &&
            !duplicate_batch.batch_plan.ok &&
            duplicate_batch.create_result.record_indexes.empty() &&
            duplicate_batch.create_result.created_objects.empty() &&
            duplicate_batch.dry_run &&
            !duplicate_batch.mutates_asset,
        "#1313: toolbox batch create-from-dispatch execution should reject duplicate planned identities");
    expect(object_count(table_path) == committed_count,
        "#1313: duplicate batch create-from-dispatch execution should not mutate assets");

    const auto invalid_field_batch = copperfin::studio::create_visual_objects_from_toolbox_dispatch({
        .dispatch_plan = dispatch_catalog.dispatch.plan,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "valid-before-invalid-dispatch-batch-guid",
                .parent_name = {},
                .field_values = {}
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = {},
                .unique_id = "invalid-field-dispatch-batch-guid",
                .parent_name = {},
                .field_values = {
                    {.property_name = "UNKNOWN", .property_value = "value"}
                }
            }
        }
    });
    expect(!invalid_field_batch.ok &&
            invalid_field_batch.error == "The requested field was not found in the asset." &&
            invalid_field_batch.batch_plan.ok &&
            invalid_field_batch.create_result.record_indexes.empty() &&
            invalid_field_batch.create_result.created_objects.empty() &&
            !invalid_field_batch.dry_run &&
            !invalid_field_batch.mutates_asset,
        "#1313: toolbox batch create-from-dispatch execution should surface lower-layer failures cleanly");
    expect(object_count(table_path) == committed_count,
        "#1313: lower-layer batch create-from-dispatch failures should not partially mutate assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_dispatch_planner_uses_admitted_palette_dispatch_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_dispatch_from_dispatch_source_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto dispatch_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "frmMain",
        .unique_id = "form-guid",
        .admit_palette_invocation = true
    });
    expect(dispatch_catalog.ok && dispatch_catalog.dispatch.ok,
        "#1264: toolbox create-dispatch-from-dispatch fixture should produce admitted dispatches");

    const auto dispatch = copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = dispatch_catalog.dispatch.plan,
            .toolbox_item_id = "textbox",
            .object_name = {},
            .unique_id = "dispatch-source-textbox-guid",
            .parent_name = {},
            .field_values = {
                {.property_name = "CAPTION", .property_value = "Dispatch Source"}
            }
        },
        .admit_create_operation = true
    });

    expect(dispatch.ok &&
            dispatch.plan.path == table_path.string() &&
            dispatch.plan.toolbox_context_provided &&
            dispatch.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            dispatch.plan.target_record_index == before_count &&
            dispatch.plan.object_name == "txt2" &&
            dispatch.plan.unique_id == "dispatch-source-textbox-guid" &&
            dispatch.plan.parent_name == "frmMain" &&
            dispatch.plan.dispatch_admitted &&
            !dispatch.plan.dry_run &&
            !dispatch.plan.executed &&
            dispatch.plan.mutates_asset,
        "#1264: toolbox create-dispatch-from-dispatch planning should preserve metadata and mutation intent");
    expect(has_argument_pair(dispatch.plan.dispatch_arguments, "--path", table_path.string()) &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-create", "textbox") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--object-name", "txt2") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--unique-id", "dispatch-source-textbox-guid") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--parent-name", "frmMain") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "OBJNAME=txt2") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "CLASS=TextBox") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "CAPTION=Dispatch Source"),
        "#1264: toolbox create-dispatch-from-dispatch planning should materialize deterministic host arguments");
    expect(object_count(table_path) == before_count,
        "#1264: toolbox create-dispatch-from-dispatch planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_dispatch_planner_rejects_invalid_palette_dispatches_without_stale_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_dispatch_from_dispatch_source_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto dry_run_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "frmMain",
        .unique_id = "form-guid",
        .admit_palette_invocation = false
    });
    expect(dry_run_catalog.ok && !dry_run_catalog.dispatch.ok,
        "#1264: dry-run toolbox create-dispatch-from-dispatch fixture should preserve rejected dispatch state");

    auto dry_run_plan = dry_run_catalog.invocation_admission.plan;
    auto dispatch = copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = {
                .selection_context = dry_run_plan.selection_context,
                .toolbox_context = dry_run_plan.toolbox_context,
                .command_token = dry_run_plan.command_token,
                .asset_path = dry_run_plan.asset_path,
                .record_index = dry_run_plan.record_index,
                .object_name = dry_run_plan.object_name,
                .unique_id = dry_run_plan.unique_id,
                .item_count = dry_run_plan.item_count,
                .items = dry_run_plan.items,
                .dispatch_arguments = {},
                .dispatch_admitted = false,
                .dry_run = true,
                .executed = false,
                .mutates_asset = false
            },
            .toolbox_item_id = "textbox",
            .object_name = {},
            .unique_id = {},
            .parent_name = {},
            .field_values = {}
        },
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error ==
                "A toolbox create-from-dispatch request requires an admitted non-executed toolbox dispatch." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1264: toolbox create-dispatch-from-dispatch planning should reject dry-run dispatch plans");

    const auto form_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "frmMain",
        .unique_id = "form-guid",
        .admit_palette_invocation = true
    });
    expect(form_catalog.ok && form_catalog.dispatch.ok,
        "#1264: admitted toolbox create-dispatch-from-dispatch fixture should produce dispatches");

    dispatch = copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = form_catalog.dispatch.plan,
            .toolbox_item_id = "textbox",
            .object_name = {},
            .unique_id = "non-admitted-create-guid",
            .parent_name = {},
            .field_values = {}
        },
        .admit_create_operation = false
    });
    expect(!dispatch.ok &&
            dispatch.error == "A toolbox create dispatch request requires an admitted non-dry-run create operation." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1264: toolbox create-dispatch-from-dispatch planning should reject non-admitted create operations");

    const auto report_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "DetailBand",
        .unique_id = "report-guid",
        .admit_palette_invocation = true
    });
    expect(report_catalog.ok && report_catalog.dispatch.ok,
        "#1264: report toolbox create-dispatch-from-dispatch fixture should produce admitted dispatches");

    dispatch = copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = report_catalog.dispatch.plan,
            .toolbox_item_id = "textbox",
            .object_name = {},
            .unique_id = {},
            .parent_name = {},
            .field_values = {}
        },
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "The requested toolbox item is not available in the admitted toolbox dispatch." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1264: toolbox create-dispatch-from-dispatch planning should reject unavailable dispatch items");

    dispatch = copperfin::studio::plan_visual_object_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = form_catalog.dispatch.plan,
            .toolbox_item_id = "textbox",
            .object_name = "txt1",
            .unique_id = "duplicate-name-guid",
            .parent_name = {},
            .field_values = {}
        },
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "The requested toolbox object identity already exists in the asset." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1264: toolbox create-dispatch-from-dispatch planning should reject invalid create plans");

    expect(object_count(table_path) == before_count,
        "#1264: rejected toolbox create-dispatch-from-dispatch plans should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_dispatch_planner_uses_admitted_palette_dispatch_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_dispatch_from_dispatch_source_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto dispatch_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "frmMain",
        .unique_id = "form-guid",
        .admit_palette_invocation = true
    });
    expect(dispatch_catalog.ok && dispatch_catalog.dispatch.ok,
        "#1266: toolbox batch create-dispatch-from-dispatch fixture should produce admitted dispatches");

    const auto dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = dispatch_catalog.dispatch.plan,
            .items = {
                {
                    .toolbox_item_id = "textbox",
                    .object_name = {},
                    .unique_id = "first-batch-dispatch-source-textbox-guid",
                    .parent_name = {},
                    .field_values = {
                        {.property_name = "CAPTION", .property_value = "First Dispatch Source"}
                    }
                },
                {
                    .toolbox_item_id = "commandbutton",
                    .object_name = "cmdDispatchSource",
                    .unique_id = "batch-dispatch-source-command-guid",
                    .parent_name = "cntToolbar",
                    .field_values = {
                        {.property_name = "CAPTION", .property_value = "Run Dispatch Source"}
                    }
                },
                {
                    .toolbox_item_id = "textbox",
                    .object_name = {},
                    .unique_id = "second-batch-dispatch-source-textbox-guid",
                    .parent_name = {},
                    .field_values = {
                        {.property_name = "CAPTION", .property_value = "Second Dispatch Source"}
                    }
                }
            }
        },
        .admit_create_operation = true
    });

    expect(dispatch.ok &&
            dispatch.plan.path == table_path.string() &&
            dispatch.plan.toolbox_context_provided &&
            dispatch.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            dispatch.plan.item_count == 3U &&
            dispatch.plan.plans.size() == 3U &&
            dispatch.plan.plans[0].target_record_index == before_count &&
            dispatch.plan.plans[0].object_name == "txt2" &&
            dispatch.plan.plans[0].parent_name == "frmMain" &&
            dispatch.plan.plans[1].target_record_index == before_count + 1U &&
            dispatch.plan.plans[1].object_name == "cmdDispatchSource" &&
            dispatch.plan.plans[1].parent_name == "cntToolbar" &&
            dispatch.plan.plans[2].target_record_index == before_count + 2U &&
            dispatch.plan.plans[2].object_name == "txt3" &&
            dispatch.plan.plans[2].parent_name == "frmMain" &&
            dispatch.plan.dispatch_admitted &&
            !dispatch.plan.dry_run &&
            !dispatch.plan.executed &&
            dispatch.plan.mutates_asset,
        "#1266: toolbox batch create-dispatch-from-dispatch planning should preserve batch metadata and mutation intent");
    expect(has_argument(dispatch.plan.dispatch_arguments, "--toolbox-create-batch") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--path", table_path.string()) &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-item", "textbox") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--object-name", "txt2") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--unique-id",
                "first-batch-dispatch-source-textbox-guid") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--parent-name", "frmMain") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "CAPTION=First Dispatch Source") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-item", "commandbutton") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--object-name", "cmdDispatchSource") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--unique-id",
                "batch-dispatch-source-command-guid") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--parent-name", "cntToolbar") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "CAPTION=Run Dispatch Source") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--object-name", "txt3") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--unique-id",
                "second-batch-dispatch-source-textbox-guid") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "CAPTION=Second Dispatch Source"),
        "#1266: toolbox batch create-dispatch-from-dispatch planning should materialize deterministic host arguments");
    expect(object_count(table_path) == before_count,
        "#1266: toolbox batch create-dispatch-from-dispatch planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_dispatch_planner_rejects_invalid_palette_dispatches_without_stale_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_dispatch_from_dispatch_source_rejection_tests_" +
            std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto dry_run_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "frmMain",
        .unique_id = "form-guid",
        .admit_palette_invocation = false
    });
    expect(dry_run_catalog.ok && !dry_run_catalog.dispatch.ok,
        "#1266: dry-run toolbox batch create-dispatch-from-dispatch fixture should preserve rejected dispatch state");

    auto dry_run_plan = dry_run_catalog.invocation_admission.plan;
    auto dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = {
                .selection_context = dry_run_plan.selection_context,
                .toolbox_context = dry_run_plan.toolbox_context,
                .command_token = dry_run_plan.command_token,
                .asset_path = dry_run_plan.asset_path,
                .record_index = dry_run_plan.record_index,
                .object_name = dry_run_plan.object_name,
                .unique_id = dry_run_plan.unique_id,
                .item_count = dry_run_plan.item_count,
                .items = dry_run_plan.items,
                .dispatch_arguments = {},
                .dispatch_admitted = false,
                .dry_run = true,
                .executed = false,
                .mutates_asset = false
            },
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
    expect(!dispatch.ok &&
            dispatch.error ==
                "A toolbox batch create-from-dispatch request requires an admitted non-executed toolbox dispatch." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1266: toolbox batch create-dispatch-from-dispatch planning should reject dry-run dispatch plans");

    const auto form_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "frmMain",
        .unique_id = "form-guid",
        .admit_palette_invocation = true
    });
    expect(form_catalog.ok && form_catalog.dispatch.ok,
        "#1266: admitted toolbox batch create-dispatch-from-dispatch fixture should produce dispatches");

    dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = form_catalog.dispatch.plan,
            .items = {
                {
                    .toolbox_item_id = "textbox",
                    .object_name = {},
                    .unique_id = "non-admitted-batch-create-guid",
                    .parent_name = {},
                    .field_values = {}
                }
            }
        },
        .admit_create_operation = false
    });
    expect(!dispatch.ok &&
            dispatch.error ==
                "A toolbox batch create dispatch request requires an admitted non-dry-run create operation." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1266: toolbox batch create-dispatch-from-dispatch planning should reject non-admitted create operations");

    const auto report_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "DetailBand",
        .unique_id = "report-guid",
        .admit_palette_invocation = true
    });
    expect(report_catalog.ok && report_catalog.dispatch.ok,
        "#1266: report toolbox batch create-dispatch-from-dispatch fixture should produce admitted dispatches");

    dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = report_catalog.dispatch.plan,
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
    expect(!dispatch.ok &&
            dispatch.error == "The requested toolbox item is not available in the admitted toolbox dispatch." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1266: toolbox batch create-dispatch-from-dispatch planning should reject unavailable dispatch items");

    dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = form_catalog.dispatch.plan,
            .items = {}
        },
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "No toolbox object creates were provided." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1266: toolbox batch create-dispatch-from-dispatch planning should reject empty batches");

    dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch_from_toolbox_dispatch({
        .create_request = {
            .dispatch_plan = form_catalog.dispatch.plan,
            .items = {
                {
                    .toolbox_item_id = "textbox",
                    .object_name = "dupDispatchName",
                    .unique_id = "first-duplicate-batch-dispatch-guid",
                    .parent_name = {},
                    .field_values = {}
                },
                {
                    .toolbox_item_id = "commandbutton",
                    .object_name = "dupDispatchName",
                    .unique_id = "second-duplicate-batch-dispatch-guid",
                    .parent_name = {},
                    .field_values = {}
                }
            }
        },
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "The requested toolbox object identity already exists in the asset." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1266: toolbox batch create-dispatch-from-dispatch planning should reject invalid batch create plans");

    expect(object_count(table_path) == before_count,
        "#1266: rejected toolbox batch create-dispatch-from-dispatch plans should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}


}
