#include "copperfin/studio/toolbox_creation.h"

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"

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
    test_toolbox_creation_catalog_plans_form_and_report_contexts_without_mutation();
    test_toolbox_creation_batch_planner_reserves_names_without_mutation();
    test_toolbox_creation_batch_planner_rejects_invalid_batches_without_mutation();
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
