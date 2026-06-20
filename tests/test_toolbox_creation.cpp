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
        {.name = "UNIQUEID", .type = 'C', .length = 64U},
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
            visual_plans[0].target_record_index > before_count &&
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
    expect(visual_plans[0].target_record_index > before_count &&
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
    expect(visual_textbox_plan != nullptr &&
            visual_textbox_plan->target_record_index > before_count &&
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
    expect(visual_textbox_plan != nullptr &&
            visual_textbox_plan->target_record_index > before_count &&
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
    test_toolbox_creation_selection_planner_resolves_contexts_without_mutation();
    test_toolbox_creation_selection_create_executes_context_resolved_creates();
    test_toolbox_creation_selection_dispatch_planner_resolves_contexts_without_mutation();
    test_toolbox_creation_selection_batch_planner_resolves_contexts_without_mutation();
    test_toolbox_creation_selection_batch_create_executes_context_resolved_batches();
    test_toolbox_creation_selection_batch_dispatch_planner_resolves_contexts_without_mutation();
    test_toolbox_creation_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_planner_rejects_invalid_palette_dispatches_without_mutation();
    test_toolbox_creation_create_from_dispatch_executes_admitted_dispatches();
    test_toolbox_creation_batch_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_batch_planner_rejects_invalid_palette_dispatches_without_mutation();
    test_toolbox_creation_batch_create_from_dispatch_executes_admitted_dispatches();
    test_toolbox_creation_dispatch_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_dispatch_planner_rejects_invalid_palette_dispatches_without_stale_arguments();
    test_toolbox_creation_batch_dispatch_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_batch_dispatch_planner_rejects_invalid_palette_dispatches_without_stale_arguments();
    test_toolbox_creation_catalog_plans_form_and_report_contexts_without_mutation();
    test_toolbox_creation_dispatch_catalog_plans_context_dispatches_without_mutation();
    test_toolbox_creation_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
    test_toolbox_creation_selection_dispatch_catalog_plans_context_dispatches_without_mutation();
    test_toolbox_creation_selection_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
    test_toolbox_creation_batch_plan_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_batch_plan_catalog_reports_planning_errors_without_stale_plans();
    test_toolbox_creation_selection_batch_plan_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_batch_dispatch_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_batch_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
    test_toolbox_creation_selection_batch_dispatch_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_selection_batch_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
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
