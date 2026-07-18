#include "test_toolbox_creation_support.h"

namespace copperfin::toolbox_creation_tests
{
void test_toolbox_creation_planner_uses_admitted_palette_dispatch_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_dispatch_source_tests_" + std::to_string(_getpid()));
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
        "#1260: toolbox create-from-dispatch fixture should produce an admitted dispatch");

    const auto plan_result = copperfin::studio::plan_visual_object_from_toolbox_dispatch({
        .dispatch_plan = dispatch_catalog.dispatch.plan,
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "dispatch-textbox-guid",
        .parent_name = {},
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Dispatch Text"}
        }
    });

    expect(plan_result.ok &&
            plan_result.plan.path == table_path.string() &&
            plan_result.plan.toolbox_context_provided &&
            plan_result.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            plan_result.plan.object_name == "txt2" &&
            plan_result.plan.unique_id == "dispatch-textbox-guid" &&
            plan_result.plan.parent_name == "frmMain" &&
            plan_result.plan.dry_run &&
            !plan_result.plan.mutates_asset,
        "#1260: toolbox create-from-dispatch planning should preserve dispatch context and selected parent");
    expect(has_field_value(plan_result.plan.field_values, "CLASS", "TextBox") &&
            has_field_value(plan_result.plan.field_values, "PARENT", "frmMain") &&
            has_field_value(plan_result.plan.field_values, "CAPTION", "Dispatch Text"),
        "#1260: toolbox create-from-dispatch planning should reuse descriptor field mapping");
    expect(object_count(table_path) == before_count,
        "#1260: toolbox create-from-dispatch planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_planner_rejects_invalid_palette_dispatches_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_dispatch_source_rejection_tests_" + std::to_string(_getpid()));
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
        "#1260: dry-run toolbox dispatch fixture should preserve rejected dispatch state");

    auto dry_run_plan = dry_run_catalog.invocation_admission.plan;
    const auto dry_run_result = copperfin::studio::plan_visual_object_from_toolbox_dispatch({
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
    });
    expect(!dry_run_result.ok &&
            dry_run_result.error ==
                "A toolbox create-from-dispatch request requires an admitted non-executed toolbox dispatch.",
        "#1260: toolbox create-from-dispatch planning should reject dry-run dispatch plans");

    const auto report_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "DetailBand",
        .unique_id = "report-guid",
        .admit_palette_invocation = true
    });
    expect(report_catalog.ok && report_catalog.dispatch.ok,
        "#1260: report toolbox dispatch fixture should produce admitted dispatches");

    const auto unavailable_result = copperfin::studio::plan_visual_object_from_toolbox_dispatch({
        .dispatch_plan = report_catalog.dispatch.plan,
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = {},
        .parent_name = {},
        .field_values = {}
    });
    expect(!unavailable_result.ok &&
            unavailable_result.error ==
                "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1260: toolbox create-from-dispatch planning should reject unavailable dispatch items");
    expect(object_count(table_path) == before_count,
        "#1260: rejected toolbox create-from-dispatch planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_create_from_dispatch_executes_admitted_dispatches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_dispatch_source_create_tests_" + std::to_string(_getpid()));
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
        "#1312: toolbox create-from-dispatch execution fixture should produce admitted dispatches");

    const auto visual_create = copperfin::studio::create_visual_object_from_toolbox_dispatch({
        .dispatch_plan = dispatch_catalog.dispatch.plan,
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "dispatch-created-textbox-guid",
        .parent_name = {},
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Dispatch Created"}
        }
    });
    expect(visual_create.ok &&
            visual_create.create_plan.ok &&
            visual_create.create_result.ok &&
            visual_create.create_plan.plan.toolbox_context_provided &&
            visual_create.create_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::form &&
            visual_create.create_plan.plan.target_record_index == before_count &&
            visual_create.create_plan.plan.object_name == "txt2" &&
            visual_create.create_plan.plan.unique_id == "dispatch-created-textbox-guid" &&
            visual_create.create_plan.plan.parent_name == "frmMain" &&
            visual_create.create_result.record_index == before_count &&
            visual_create.create_result.object_name == "txt2" &&
            visual_create.create_result.unique_id == "dispatch-created-textbox-guid" &&
            visual_create.create_result.parent_name == "frmMain" &&
            !visual_create.dry_run &&
            visual_create.mutates_asset,
        "#1312: toolbox create-from-dispatch execution should append dispatch-sourced form objects");
    expect(has_field_value(visual_create.create_plan.plan.field_values, "CLASS", "TextBox") &&
            has_field_value(visual_create.create_plan.plan.field_values, "PARENT", "frmMain") &&
            has_field_value(visual_create.create_plan.plan.field_values, "CAPTION", "Dispatch Created"),
        "#1312: toolbox create-from-dispatch execution should preserve planned descriptor fields");
    const auto visual_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dispatch-created-textbox-guid",
        .property_name = "CAPTION"
    });
    expect(visual_caption.ok && visual_caption.exists && visual_caption.value == "Dispatch Created",
        "#1312: toolbox create-from-dispatch execution should persist caller fields");

    const auto report_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "DetailBand",
        .unique_id = "report-guid",
        .admit_palette_invocation = true
    });
    expect(report_catalog.ok && report_catalog.dispatch.ok,
        "#1312: report toolbox create-from-dispatch execution fixture should produce admitted dispatches");
    const auto report_create = copperfin::studio::create_visual_object_from_toolbox_dispatch({
        .dispatch_plan = report_catalog.dispatch.plan,
        .toolbox_item_id = "label",
        .object_name = {},
        .unique_id = "dispatch-created-report-label-guid",
        .parent_name = {},
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Report Dispatch Created"}
        }
    });
    expect(report_create.ok &&
            report_create.create_plan.ok &&
            report_create.create_result.ok &&
            report_create.create_plan.plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
            report_create.create_plan.plan.object_name == "lbl1" &&
            report_create.create_plan.plan.parent_name == "DetailBand" &&
            report_create.create_result.record_index == before_count + 1U &&
            report_create.create_result.object_name == "lbl1" &&
            report_create.create_result.unique_id == "dispatch-created-report-label-guid" &&
            report_create.create_result.parent_name == "DetailBand",
        "#1312: toolbox create-from-dispatch execution should append report-safe objects");

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
        "#1312: dry-run toolbox create-from-dispatch execution fixture should preserve rejected dispatch state");
    auto dry_run_plan = dry_run_catalog.invocation_admission.plan;
    const auto dry_run_create = copperfin::studio::create_visual_object_from_toolbox_dispatch({
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
        .unique_id = "dry-run-dispatch-create-guid",
        .parent_name = {},
        .field_values = {}
    });
    expect(!dry_run_create.ok &&
            dry_run_create.error ==
                "A toolbox create-from-dispatch request requires an admitted non-executed toolbox dispatch." &&
            !dry_run_create.create_plan.ok &&
            !dry_run_create.create_result.ok &&
            dry_run_create.create_result.object_name.empty() &&
            dry_run_create.create_result.unique_id.empty() &&
            dry_run_create.create_result.parent_name.empty() &&
            dry_run_create.dry_run &&
            !dry_run_create.mutates_asset,
        "#1312: toolbox create-from-dispatch execution should reject dry-run dispatches without stale metadata");
    expect(object_count(table_path) == committed_count,
        "#1312: rejected dry-run create-from-dispatch execution should not mutate assets");

    const auto unavailable_create = copperfin::studio::create_visual_object_from_toolbox_dispatch({
        .dispatch_plan = report_catalog.dispatch.plan,
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "unavailable-dispatch-create-guid",
        .parent_name = {},
        .field_values = {}
    });
    expect(!unavailable_create.ok &&
            unavailable_create.error == "The requested toolbox item is not available in the admitted toolbox dispatch." &&
            !unavailable_create.create_plan.ok &&
            !unavailable_create.create_result.ok &&
            unavailable_create.create_result.object_name.empty() &&
            unavailable_create.create_result.unique_id.empty() &&
            unavailable_create.create_result.parent_name.empty() &&
            unavailable_create.dry_run &&
            !unavailable_create.mutates_asset,
        "#1312: toolbox create-from-dispatch execution should reject unavailable dispatch items");
    expect(object_count(table_path) == committed_count,
        "#1312: unavailable create-from-dispatch execution should not mutate assets");

    const auto invalid_field_create = copperfin::studio::create_visual_object_from_toolbox_dispatch({
        .dispatch_plan = dispatch_catalog.dispatch.plan,
        .toolbox_item_id = "commandbutton",
        .object_name = {},
        .unique_id = "invalid-field-dispatch-create-guid",
        .parent_name = {},
        .field_values = {
            {.property_name = "UNKNOWN", .property_value = "value"}
        }
    });
    expect(!invalid_field_create.ok &&
            invalid_field_create.error == "The requested field was not found in the asset." &&
            invalid_field_create.create_plan.ok &&
            !invalid_field_create.create_result.ok &&
            invalid_field_create.create_result.object_name.empty() &&
            invalid_field_create.create_result.unique_id.empty() &&
            invalid_field_create.create_result.parent_name.empty() &&
            !invalid_field_create.dry_run &&
            !invalid_field_create.mutates_asset,
        "#1312: toolbox create-from-dispatch execution should surface lower-layer create failures cleanly");
    expect(object_count(table_path) == committed_count,
        "#1312: lower-layer create-from-dispatch failures should not partially mutate assets");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_planner_uses_admitted_palette_dispatch_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_dispatch_source_tests_" + std::to_string(_getpid()));
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
        "#1262: toolbox batch create-from-dispatch fixture should produce an admitted dispatch");

    const auto batch_plan = copperfin::studio::plan_visual_objects_from_toolbox_dispatch({
        .dispatch_plan = dispatch_catalog.dispatch.plan,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "first-dispatch-textbox-guid",
                .parent_name = {},
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "First Dispatch"}
                }
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "cmdDispatch",
                .unique_id = "dispatch-command-guid",
                .parent_name = "cntToolbar",
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Run Dispatch"}
                }
            },
            {
                .toolbox_item_id = "textbox",
                .object_name = {},
                .unique_id = "second-dispatch-textbox-guid",
                .parent_name = {},
                .field_values = {
                    {.property_name = "CAPTION", .property_value = "Second Dispatch"}
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
        "#1262: toolbox batch create-from-dispatch planning should preserve dispatch metadata");
    if (batch_plan.ok && batch_plan.plan.plans.size() == 3U) {
        expect(batch_plan.plan.plans[0].target_record_index == before_count &&
                batch_plan.plan.plans[0].object_name == "txt2" &&
                batch_plan.plan.plans[0].unique_id == "first-dispatch-textbox-guid" &&
                batch_plan.plan.plans[0].parent_name == "frmMain" &&
                std::string(batch_plan.plan.plans[0].toolbox_item.id) == "textbox" &&
                has_field_value(batch_plan.plan.plans[0].field_values, "CLASS", "TextBox") &&
                has_field_value(batch_plan.plan.plans[0].field_values, "PARENT", "frmMain") &&
                has_field_value(batch_plan.plan.plans[0].field_values, "CAPTION", "First Dispatch"),
            "#1262: first dispatch-sourced batch plan should default parent and map descriptors");
        expect(batch_plan.plan.plans[1].target_record_index == before_count + 1U &&
                batch_plan.plan.plans[1].object_name == "cmdDispatch" &&
                batch_plan.plan.plans[1].unique_id == "dispatch-command-guid" &&
                batch_plan.plan.plans[1].parent_name == "cntToolbar" &&
                std::string(batch_plan.plan.plans[1].toolbox_item.id) == "commandbutton" &&
                has_field_value(batch_plan.plan.plans[1].field_values, "CLASS", "CommandButton") &&
                has_field_value(batch_plan.plan.plans[1].field_values, "PARENT", "cntToolbar") &&
                has_field_value(batch_plan.plan.plans[1].field_values, "CAPTION", "Run Dispatch"),
            "#1262: explicit dispatch-sourced batch overrides should preserve order and parent overrides");
        expect(batch_plan.plan.plans[2].target_record_index == before_count + 2U &&
                batch_plan.plan.plans[2].object_name == "txt3" &&
                batch_plan.plan.plans[2].unique_id == "second-dispatch-textbox-guid" &&
                batch_plan.plan.plans[2].parent_name == "frmMain" &&
                std::string(batch_plan.plan.plans[2].toolbox_item.id) == "textbox" &&
                has_field_value(batch_plan.plan.plans[2].field_values, "CLASS", "TextBox") &&
                has_field_value(batch_plan.plan.plans[2].field_values, "PARENT", "frmMain") &&
                has_field_value(batch_plan.plan.plans[2].field_values, "CAPTION", "Second Dispatch"),
            "#1262: dispatch-sourced batch planning should reserve generated names across planned items");
    }
    expect(object_count(table_path) == before_count,
        "#1262: toolbox batch create-from-dispatch planning should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_batch_planner_rejects_invalid_palette_dispatches_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_batch_dispatch_source_rejection_tests_" + std::to_string(_getpid()));
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
        "#1262: dry-run toolbox batch dispatch fixture should preserve rejected dispatch state");

    auto dry_run_plan = dry_run_catalog.invocation_admission.plan;
    auto rejected_plan = copperfin::studio::plan_visual_objects_from_toolbox_dispatch({
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
    });
    expect(!rejected_plan.ok &&
            rejected_plan.error ==
                "A toolbox batch create-from-dispatch request requires an admitted non-executed toolbox dispatch.",
        "#1262: toolbox batch create-from-dispatch planning should reject dry-run dispatch plans");

    const auto form_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::form,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "frmMain",
        .unique_id = "form-guid",
        .admit_palette_invocation = true
    });
    expect(form_catalog.ok && form_catalog.dispatch.ok,
        "#1262: admitted toolbox batch dispatch fixture should produce dispatches");

    rejected_plan = copperfin::studio::plan_visual_objects_from_toolbox_dispatch({
        .dispatch_plan = form_catalog.dispatch.plan,
        .items = {}
    });
    expect(!rejected_plan.ok && rejected_plan.error == "No toolbox object creates were provided.",
        "#1262: toolbox batch create-from-dispatch planning should reject empty batches");

    const auto report_catalog = copperfin::studio::plan_studio_toolbox_dispatch_catalog({
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .asset_path = table_path.string(),
        .record_index = 0U,
        .object_name = "DetailBand",
        .unique_id = "report-guid",
        .admit_palette_invocation = true
    });
    expect(report_catalog.ok && report_catalog.dispatch.ok,
        "#1262: report toolbox batch dispatch fixture should produce admitted dispatches");

    rejected_plan = copperfin::studio::plan_visual_objects_from_toolbox_dispatch({
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
    });
    expect(!rejected_plan.ok &&
            rejected_plan.error ==
                "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1262: toolbox batch create-from-dispatch planning should reject unavailable dispatch items");

    rejected_plan = copperfin::studio::plan_visual_objects_from_toolbox_dispatch({
        .dispatch_plan = form_catalog.dispatch.plan,
        .items = {
            {
                .toolbox_item_id = "textbox",
                .object_name = "dupDispatchName",
                .unique_id = "first-dispatch-dup-guid",
                .parent_name = {},
                .field_values = {}
            },
            {
                .toolbox_item_id = "commandbutton",
                .object_name = "dupDispatchName",
                .unique_id = "second-dispatch-dup-guid",
                .parent_name = {},
                .field_values = {}
            }
        }
    });
    expect(!rejected_plan.ok &&
            rejected_plan.error == "The requested toolbox object identity already exists in the asset.",
        "#1262: toolbox batch create-from-dispatch planning should reuse batch identity validation");

    expect(object_count(table_path) == before_count,
        "#1262: rejected toolbox batch create-from-dispatch plans should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}


}
