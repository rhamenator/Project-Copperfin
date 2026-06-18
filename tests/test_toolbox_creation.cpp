#include "copperfin/studio/toolbox_creation.h"

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#define _getpid getpid
#endif
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

std::size_t object_count(const std::filesystem::path& table_path) {
    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    return list_result.ok ? list_result.objects.size() : 0U;
}

bool has_field_value(
    const std::vector<copperfin::vfp::VisualObjectPropertyChange>& changes,
    const std::string& property_name,
    const std::string& property_value) {
    for (const auto& change : changes) {
        if (change.property_name == property_name && change.property_value == property_value) {
            return true;
        }
    }
    return false;
}

bool has_argument_pair(const std::vector<std::string>& arguments, const std::string& key, const std::string& value) {
    for (std::size_t index = 0U; (index + 1U) < arguments.size(); ++index) {
        if (arguments[index] == key && arguments[index + 1U] == value) {
            return true;
        }
    }
    return false;
}

bool has_argument(const std::vector<std::string>& arguments, const std::string& value) {
    return std::find(arguments.begin(), arguments.end(), value) != arguments.end();
}

const copperfin::studio::StudioToolboxObjectCreatePlanCatalogEntry* find_create_plan_entry(
    const std::vector<copperfin::studio::StudioToolboxObjectCreatePlanCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.toolbox_item.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioToolboxObjectCreateDispatchCatalogEntry* find_create_dispatch_entry(
    const std::vector<copperfin::studio::StudioToolboxObjectCreateDispatchCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.toolbox_item.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioToolboxObjectCreatePlan* find_create_batch_plan(
    const std::vector<copperfin::studio::StudioToolboxObjectCreatePlan>& plans,
    std::string_view id) {
    for (const auto& plan : plans) {
        if (plan.toolbox_item.id == id) {
            return &plan;
        }
    }
    return nullptr;
}

std::filesystem::path create_toolbox_fixture(const std::filesystem::path& temp_dir) {
    const std::filesystem::path table_path = temp_dir / "toolbox_create.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "CAPTION", .type = 'C', .length = 32U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "frmMain", "form-guid", "", "Form", "Form", "Main", ""},
        {"txt1", "txt1", "existing-textbox-guid", "frmMain", "TextBox", "TextBox", "Existing", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#1017: toolbox creation fixture should be writable");
    return table_path;
}

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
    expect(textbox_plan != nullptr &&
            textbox_plan->target_record_index > before_count &&
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
            textbox_plan != nullptr &&
            textbox_plan->target_record_index > before_count &&
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

void test_toolbox_creation_maps_descriptors_and_defaults() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);

    const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Customer"},
            {.property_name = "PROPERTIES", .property_value = "ControlSource = \"customer.name\"\r\nLeft = 12\r\n"}
        }
    });

    expect(create_result.ok, "#1017: toolbox descriptor creates should succeed for known toolbox ids");
    expect(create_result.record_index == 2U,
        "#1017: toolbox descriptor creates should append the new object row");
    expect(create_result.object_name == "txt2" &&
            create_result.unique_id == "created-textbox-guid" &&
            create_result.parent_name == "frmMain",
        "#1017: toolbox descriptor creates should report generated identity metadata");

    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U,
        "#1017: toolbox descriptor creates should append exactly one object");
    if (list_result.ok && list_result.objects.size() == 3U) {
        const auto& created_object = list_result.objects[2];
        expect(created_object.object_name == "txt2",
            "#1017: created objects should carry generated object names");
        expect(created_object.unique_id == "created-textbox-guid",
            "#1017: created objects should carry unique ids");
        expect(created_object.parent_name == "frmMain",
            "#1017: created objects should carry parent names");
        expect(created_object.class_name == "TextBox",
            "#1017: created objects should carry descriptor class names");
        expect(created_object.baseclass_name == "TextBox",
            "#1017: created objects should carry descriptor baseclass names");
    }

    const auto caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "CAPTION"
    });
    expect(caption.ok && caption.exists && caption.value == "Customer",
        "#1017: toolbox descriptor creates should propagate caller-provided direct fields");

    const auto control_source = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(control_source.ok && control_source.exists && control_source.value == "\"customer.name\"",
        "#1017: toolbox descriptor creates should propagate caller-provided memo properties");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_respects_explicit_object_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_explicit_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);

    const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "commandbutton",
        .object_name = "cmdRun",
        .unique_id = "command-guid",
        .parent_name = "frmMain",
        .field_values = {}
    });

    expect(create_result.ok && create_result.object_name == "cmdRun",
        "#1017: explicit toolbox object names should take precedence over default name generation");

    const auto class_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "command-guid",
        .property_name = "CLASS"
    });
    expect(class_result.ok && class_result.exists && class_result.value == "CommandButton",
        "#1017: explicit-name creates should still map descriptor class metadata");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_rejects_unknown_toolbox_without_mutation() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_failure_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);
    const std::size_t before_count = object_count(table_path);

    const auto create_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "missing-toolbox-item",
        .object_name = {},
        .unique_id = "should-not-exist",
        .parent_name = "frmMain",
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Should Not Exist"}
        }
    });

    expect(!create_result.ok,
        "#1017: unknown toolbox ids should fail cleanly");
    expect(create_result.object_name.empty() &&
            create_result.unique_id.empty() &&
            create_result.parent_name.empty(),
        "#1017: failed toolbox descriptor creates should not report stale identity metadata");
    expect(object_count(table_path) == before_count,
        "#1017: unknown toolbox ids should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

void test_toolbox_creation_enforces_optional_context_filters() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_toolbox_creation_context_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = create_toolbox_fixture(temp_dir);

    const auto label_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "label",
        .object_name = {},
        .unique_id = "report-label-guid",
        .parent_name = "DetailBand",
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Total"}
        }
    });
    expect(label_result.ok && label_result.object_name == "lbl1",
        "#1019: report-compatible toolbox items should create when report context is requested");

    const std::size_t before_rejected_count = object_count(table_path);
    const auto textbox_result = copperfin::studio::create_visual_object_from_toolbox_item({
        .path = table_path.string(),
        .toolbox_item_id = "textbox",
        .object_name = {},
        .unique_id = "report-textbox-guid",
        .parent_name = "DetailBand",
        .toolbox_context_provided = true,
        .toolbox_context = copperfin::studio::StudioToolboxContext::report,
        .field_values = {
            {.property_name = "CAPTION", .property_value = "Should Not Exist"}
        }
    });
    expect(!textbox_result.ok,
        "#1019: report-incompatible toolbox items should fail when report context is requested");
    expect(textbox_result.object_name.empty() &&
            textbox_result.unique_id.empty() &&
            textbox_result.parent_name.empty(),
        "#1019: rejected context-filtered toolbox creates should not report stale identity metadata");
    expect(object_count(table_path) == before_rejected_count,
        "#1019: rejected context-filtered toolbox creates should not mutate the visual asset");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace

int main() {
    test_toolbox_creation_planner_maps_descriptors_without_mutation();
    test_toolbox_creation_planner_respects_explicit_names_and_rejections();
    test_toolbox_creation_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_planner_rejects_invalid_palette_dispatches_without_mutation();
    test_toolbox_creation_batch_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_batch_planner_rejects_invalid_palette_dispatches_without_mutation();
    test_toolbox_creation_catalog_plans_form_and_report_contexts_without_mutation();
    test_toolbox_creation_dispatch_catalog_plans_context_dispatches_without_mutation();
    test_toolbox_creation_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
    test_toolbox_creation_batch_plan_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_batch_plan_catalog_reports_planning_errors_without_stale_plans();
    test_toolbox_creation_batch_dispatch_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_batch_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
    test_toolbox_creation_batch_planner_reserves_names_without_mutation();
    test_toolbox_creation_batch_planner_rejects_invalid_batches_without_mutation();
    test_toolbox_creation_batch_create_maps_descriptors_and_metadata();
    test_toolbox_creation_batch_create_rejects_invalid_batches_without_partial_mutation();
    test_toolbox_creation_dispatch_plans_host_arguments_without_mutation();
    test_toolbox_creation_dispatch_rejects_invalid_plans_without_stale_arguments();
    test_toolbox_creation_batch_dispatch_plans_host_arguments_without_mutation();
    test_toolbox_creation_batch_dispatch_rejects_invalid_plans_without_stale_arguments();
    test_toolbox_creation_maps_descriptors_and_defaults();
    test_toolbox_creation_respects_explicit_object_name();
    test_toolbox_creation_rejects_unknown_toolbox_without_mutation();
    test_toolbox_creation_enforces_optional_context_filters();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
