#include "test_toolbox_creation_support.h"

namespace copperfin::toolbox_creation_tests
{
void test_toolbox_creation_batch_planner_reserves_names_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_plan_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto batch_plan = copperfin::studio::plan_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "first-textbox-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "First"}
                }
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "cmdRun",
                .unique_id = "run-command-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Run"}
                }
            },
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "second-textbox-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Second"}
                }
            }
        }
    });

    expect(batch_plan.ok &&
            batch_plan.plan.path == table_path.string() &&
            batch_plan.plan.toolbox_context_provided &&
            batch_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            batch_plan.plan.item_count == 3U &&
            batch_plan.plan.plans.size() == 3U &&
            batch_plan.plan.dry_run &&
            !batch_plan.plan.mutates_asset,
        "#1245: toolbox batch creation planning should preserve batch metadata without mutation");
    if (batch_plan.ok && batch_plan.plan.plans.size() == 3U) {
        expect(batch_plan.plan.plans[0].target_record_index == before_count &&
                batch_plan.plan.plans[0].object_name == "txt2" &&
                batch_plan.plan.plans[0].unique_id == "first-textbox-guid" &&
                batch_plan.plan.plans[0].parent_name == "frmMain" &&
                std::string(batch_plan.plan.plans[0].toolbox_item.id) == "textbox" &&
                has_field_value(batch_plan.plan.plans[0].field_values, "CLASS", "TextBox") &&
                has_field_value(batch_plan.plan.plans[0].field_values, "CAPTION", "First"),
            "#1245: first toolbox batch plan should preserve descriptor metadata and generated names");
        expect(batch_plan.plan.plans[1].target_record_index == before_count + 1U &&
                batch_plan.plan.plans[1].object_name == "cmdRun" &&
                batch_plan.plan.plans[1].unique_id == "run-command-guid" &&
                std::string(batch_plan.plan.plans[1].toolbox_item.id) == "commandbutton" &&
                has_field_value(batch_plan.plan.plans[1].field_values, "CLASS", "CommandButton") &&
                has_field_value(batch_plan.plan.plans[1].field_values, "CAPTION", "Run"),
            "#1245: explicit toolbox batch plan names should preserve append-order metadata");
        expect(batch_plan.plan.plans[2].target_record_index == before_count + 2U &&
                batch_plan.plan.plans[2].object_name == "txt3" &&
                batch_plan.plan.plans[2].unique_id == "second-textbox-guid" &&
                std::string(batch_plan.plan.plans[2].toolbox_item.id) == "textbox" &&
                has_field_value(batch_plan.plan.plans[2].field_values, "CLASS", "TextBox") &&
                has_field_value(batch_plan.plan.plans[2].field_values, "CAPTION", "Second"),
            "#1245: toolbox batch planning should reserve generated names across earlier planned items");
    }
    expect(object_count(table_path) == before_count,
        "#1245: toolbox batch creation planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_planner_rejects_invalid_batches_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_plan_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    auto rejected_plan = copperfin::studio::plan_visual_objects_from_toolbox_items({
        .path = {},
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
    expect(!rejected_plan.ok && rejected_plan.error == "No asset path was provided.",
        "#1245: toolbox batch planning should reject missing asset paths");

    rejected_plan = copperfin::studio::plan_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .items = {}
    });
    expect(!rejected_plan.ok && rejected_plan.error == "No toolbox object creates were provided.",
        "#1245: toolbox batch planning should reject empty item lists");

    rejected_plan = copperfin::studio::plan_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = "dupName",
                .unique_id = "dup-guid-1",
                .parent_name = "frmMain",
                .field_values = {}
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "dupName",
                .unique_id = "dup-guid-2",
                .parent_name = "frmMain",
                .field_values = {}
            }
        }
    });
    expect(!rejected_plan.ok &&
            rejected_plan.error == "The requested toolbox object identity already exists in the asset.",
        "#1245: toolbox batch planning should reject duplicate explicit object names in the same batch");

    rejected_plan = copperfin::studio::plan_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = "txtCustomer",
                .unique_id = "existing-textbox-guid",
                .parent_name = "frmMain",
                .field_values = {}
            }
        }
    });
    expect(!rejected_plan.ok &&
            rejected_plan.error == "The requested toolbox object identity already exists in the asset.",
        "#1245: toolbox batch planning should reject unique-id collisions with existing objects");

    rejected_plan = copperfin::studio::plan_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "report-textbox-guid",
                .parent_name = "DetailBand",
                .field_values = {}
            }
        }
    });
    expect(!rejected_plan.ok &&
            rejected_plan.error ==
                "The requested toolbox item is not available in the requested designer context.",
        "#1245: toolbox batch planning should reject context-incompatible toolbox items");

    expect(object_count(table_path) == before_count,
        "#1245: rejected toolbox batch plans should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_create_maps_descriptors_and_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_create_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto create_result = copperfin::studio::create_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "first-batch-textbox-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "First Batch"}
                }
            },
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "second-batch-textbox-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Second Batch"}
                }
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "cmdBatchRun",
                .unique_id = "batch-command-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Run Batch"}
                }
            }
        }
    });

    expect(create_result.ok &&
            create_result.record_indexes.size() == 3U &&
            create_result.record_indexes[0] == before_count &&
            create_result.record_indexes[1] == before_count + 1U &&
            create_result.record_indexes[2] == before_count + 2U,
        "#1247: toolbox batch creates should append every planned object in order");
    expect(create_result.created_objects.size() == 3U &&
            create_result.created_objects[0].object_name == "txt2" &&
            create_result.created_objects[0].unique_id == "first-batch-textbox-guid" &&
            create_result.created_objects[0].parent_name == "frmMain" &&
            create_result.created_objects[1].object_name == "txt3" &&
            create_result.created_objects[1].unique_id == "second-batch-textbox-guid" &&
            create_result.created_objects[1].parent_name == "frmMain" &&
            create_result.created_objects[2].object_name == "cmdBatchRun" &&
            create_result.created_objects[2].unique_id == "batch-command-guid" &&
            create_result.created_objects[2].parent_name == "frmMain",
        "#1247: toolbox batch creates should report created object identity metadata in append order");

    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == before_count + 3U,
        "#1247: toolbox batch creates should persist every created object");
    if (list_result.ok && list_result.objects.size() == before_count + 3U) {
        expect(list_result.objects[2].object_name == "txt2" &&
                list_result.objects[2].class_name == "TextBox" &&
                list_result.objects[2].baseclass_name == "TextBox",
            "#1247: first toolbox batch-created object should map textbox descriptors");
        expect(list_result.objects[3].object_name == "txt3" &&
                list_result.objects[3].class_name == "TextBox",
            "#1247: second toolbox batch-created object should reserve generated names");
        expect(list_result.objects[4].object_name == "cmdBatchRun" &&
                list_result.objects[4].class_name == "CommandButton" &&
                list_result.objects[4].baseclass_name == "CommandButton",
            "#1247: explicit toolbox batch-created objects should map command descriptors");
    }
    const auto first_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "first-batch-textbox-guid",
        .property_name = "CAPTION"
    });
    const auto second_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "second-batch-textbox-guid",
        .property_name = "CAPTION"
    });
    const auto command_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "batch-command-guid",
        .property_name = "CAPTION"
    });
    expect(first_caption.ok && first_caption.exists && first_caption.value == "First Batch" &&
            second_caption.ok && second_caption.exists && second_caption.value == "Second Batch" &&
            command_caption.ok && command_caption.exists && command_caption.value == "Run Batch",
        "#1247: toolbox batch creates should persist caller-provided direct fields");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_create_rejects_invalid_batches_without_partial_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_create_failure_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    auto create_result = copperfin::studio::create_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "report-textbox-guid",
                .parent_name = "DetailBand",
                .field_values = {}
            }
        }
    });
    expect(!create_result.ok &&
            create_result.error == "The requested toolbox item is not available in the requested designer context." &&
            create_result.record_indexes.empty() &&
            create_result.created_objects.empty(),
        "#1247: toolbox batch creates should reject context-incompatible items before mutation");
    expect(object_count(table_path) == before_count,
        "#1247: context-rejected toolbox batch creates should not mutate the visual asset");

    create_result = copperfin::studio::create_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = "dupBatchName",
                .unique_id = "dup-batch-guid-1",
                .parent_name = "frmMain",
                .field_values = {}
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "dupBatchName",
                .unique_id = "dup-batch-guid-2",
                .parent_name = "frmMain",
                .field_values = {}
            }
        }
    });
    expect(!create_result.ok &&
            create_result.error == "The requested toolbox object identity already exists in the asset." &&
            create_result.record_indexes.empty() &&
            create_result.created_objects.empty(),
        "#1247: toolbox batch creates should reject duplicate explicit identities before mutation");
    expect(object_count(table_path) == before_count,
        "#1247: duplicate-rejected toolbox batch creates should not mutate the visual asset");

    create_result = copperfin::studio::create_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "valid-before-invalid-guid",
                .parent_name = "frmMain",
                .field_values = {}
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = {},
                .unique_id = "invalid-field-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "UNKNOWN", .property_value = "value"}
                }
            }
        }
    });
    expect(!create_result.ok &&
            create_result.error == "The requested field was not found in the asset." &&
            create_result.record_indexes.empty() &&
            create_result.created_objects.empty(),
        "#1247: toolbox batch creates should reject lower-layer invalid fields without stale metadata");
    expect(object_count(table_path) == before_count,
        "#1247: lower-layer toolbox batch create failures should not partially mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_dispatch_plans_host_arguments_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_dispatch_plan_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto create_plan = copperfin::studio::plan_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "dispatch-textbox-guid",
        .parent_name = "frmMain",
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Dispatch Planned"}
        }
    });
    expect(create_plan.ok,
        "#1249: toolbox create dispatch fixture should produce a valid create plan");

    const auto dispatch = copperfin::studio::plan_visual_object_create_dispatch({
        .create_plan = create_plan.plan,
        .admit_create_operation = true
    });

    expect(dispatch.ok &&
            dispatch.plan.path == table_path.string() &&
            std::string(dispatch.plan.toolbox_item.id) == "textbox" &&
            dispatch.plan.toolbox_context_provided &&
            dispatch.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            dispatch.plan.target_record_index == before_count &&
            dispatch.plan.object_name == "txt2" &&
            dispatch.plan.unique_id == "dispatch-textbox-guid" &&
            dispatch.plan.parent_name == "frmMain" &&
            dispatch.plan.dispatch_admitted &&
            !dispatch.plan.dry_run &&
            !dispatch.plan.executed &&
            dispatch.plan.mutates_asset,
        "#1249: toolbox create dispatch planning should preserve planned metadata and mutation intent");
    expect(has_argument_pair(dispatch.plan.dispatch_arguments, "--path", table_path.string()) &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-create", "textbox") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--object-name", "txt2") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--unique-id", "dispatch-textbox-guid") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--parent-name", "frmMain") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "OBJNAME=txt2") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "CLASS=TextBox") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "CAPTION=Dispatch Planned"),
        "#1249: toolbox create dispatch planning should materialize deterministic host arguments");
    expect(object_count(table_path) == before_count,
        "#1249: toolbox create dispatch planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_dispatch_rejects_invalid_plans_without_stale_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_dispatch_plan_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto create_plan = copperfin::studio::plan_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "dispatch-reject-guid",
        .parent_name = "frmMain",
        .field_values = {}
    });
    expect(create_plan.ok,
        "#1249: toolbox create dispatch rejection fixture should produce a valid create plan");

    auto dispatch = copperfin::studio::plan_visual_object_create_dispatch({
        .create_plan = create_plan.plan,
        .admit_create_operation = false
    });
    expect(!dispatch.ok &&
            dispatch.error == "A toolbox create dispatch request requires an admitted non-dry-run create operation." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1249: toolbox create dispatch planning should reject non-admitted create operations");

    dispatch = copperfin::studio::plan_visual_object_create_dispatch({
        .create_plan = {},
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "A toolbox create dispatch request requires validated toolbox item metadata." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1249: toolbox create dispatch planning should reject missing toolbox metadata");

    auto missing_path_plan = create_plan.plan;
    missing_path_plan.path.clear();
    dispatch = copperfin::studio::plan_visual_object_create_dispatch({
        .create_plan = missing_path_plan,
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "A toolbox create dispatch request requires an asset path." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1249: toolbox create dispatch planning should reject missing asset paths");

    auto missing_descriptor_fields_plan = create_plan.plan;
    missing_descriptor_fields_plan.field_values.clear();
    dispatch = copperfin::studio::plan_visual_object_create_dispatch({
        .create_plan = missing_descriptor_fields_plan,
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "A toolbox create dispatch request requires descriptor field values." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1249: toolbox create dispatch planning should reject incomplete descriptor field values");

    expect(object_count(table_path) == before_count,
        "#1249: rejected toolbox create dispatch plans should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_dispatch_plans_host_arguments_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_dispatch_plan_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto batch_plan = copperfin::studio::plan_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "batch-dispatch-textbox-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Batch Dispatch Text"}
                }
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "cmdBatchDispatch",
                .unique_id = "batch-dispatch-command-guid",
                .parent_name = "frmMain",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Batch Dispatch Command"}
                }
            }
        }
    });
    expect(batch_plan.ok,
        "#1251: toolbox batch create dispatch fixture should produce a valid batch plan");

    const auto dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch({
        .batch_plan = batch_plan.plan,
        .admit_create_operation = true
    });

    expect(dispatch.ok &&
            dispatch.plan.path == table_path.string() &&
            dispatch.plan.toolbox_context_provided &&
            dispatch.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            dispatch.plan.item_count == 2U &&
            dispatch.plan.plans.size() == 2U &&
            dispatch.plan.plans[0].target_record_index == before_count &&
            dispatch.plan.plans[0].object_name == "txt2" &&
            dispatch.plan.plans[1].target_record_index == before_count + 1U &&
            dispatch.plan.plans[1].object_name == "cmdBatchDispatch" &&
            dispatch.plan.dispatch_admitted &&
            !dispatch.plan.dry_run &&
            !dispatch.plan.executed &&
            dispatch.plan.mutates_asset,
        "#1251: toolbox batch create dispatch planning should preserve batch metadata and mutation intent");
    expect(has_argument(dispatch.plan.dispatch_arguments, "--toolbox-create-batch") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--path", table_path.string()) &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-context", "form") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-item", "textbox") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--object-name", "txt2") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--unique-id", "batch-dispatch-textbox-guid") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "CAPTION=Batch Dispatch Text") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--toolbox-item", "commandbutton") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--object-name", "cmdBatchDispatch") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--unique-id", "batch-dispatch-command-guid") &&
            has_argument_pair(dispatch.plan.dispatch_arguments, "--field-value", "CAPTION=Batch Dispatch Command"),
        "#1251: toolbox batch create dispatch planning should materialize deterministic per-item host arguments");
    expect(object_count(table_path) == before_count,
        "#1251: toolbox batch create dispatch planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_dispatch_rejects_invalid_plans_without_stale_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_dispatch_plan_rejection_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto batch_plan = copperfin::studio::plan_visual_objects_from_toolbox_items({
        .path = table_path.string(),
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "batch-dispatch-reject-guid",
                .parent_name = "frmMain",
                .field_values = {}
            }
        }
    });
    expect(batch_plan.ok,
        "#1251: toolbox batch create dispatch rejection fixture should produce a valid batch plan");

    auto dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch({
        .batch_plan = batch_plan.plan,
        .admit_create_operation = false
    });
    expect(!dispatch.ok &&
            dispatch.error ==
                "A toolbox batch create dispatch request requires an admitted non-dry-run create operation." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1251: toolbox batch create dispatch planning should reject non-admitted create operations");

    dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch({
        .batch_plan = {},
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "A toolbox batch create dispatch request requires an asset path." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1251: toolbox batch create dispatch planning should reject missing asset paths");

    auto empty_batch_plan = batch_plan.plan;
    empty_batch_plan.plans.clear();
    empty_batch_plan.item_count = 0U;
    dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch({
        .batch_plan = empty_batch_plan,
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "A toolbox batch create dispatch request requires planned toolbox creates." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1251: toolbox batch create dispatch planning should reject empty batch plans");

    auto incomplete_plan = batch_plan.plan;
    incomplete_plan.plans[0].field_values.clear();
    dispatch = copperfin::studio::plan_visual_object_batch_create_dispatch({
        .batch_plan = incomplete_plan,
        .admit_create_operation = true
    });
    expect(!dispatch.ok &&
            dispatch.error == "A toolbox batch create dispatch request requires descriptor field values." &&
            dispatch.plan.dispatch_arguments.empty(),
        "#1251: toolbox batch create dispatch planning should reject incomplete per-item field values");

    expect(object_count(table_path) == before_count,
        "#1251: rejected toolbox batch create dispatch plans should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}


}
