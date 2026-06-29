#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_plans_toolbox_object_creation(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan", "textbox",
            "--toolbox-context", "form",
            "--unique-id", "planned-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Customer",
            "--field-value", "PROPERTIES=ControlSource = \"customer.name\"",
            "--json"
        },
        temp_root);

    expect(plan_process.exit_code == 0,
        "#1242: toolbox-create-plan JSON command should exit successfully");
    expect_contains(plan_process.stdout_text, "\"status\": \"ok\"",
        "#1242: successful toolbox-create-plan JSON should report ok status");
    expect_contains(plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#1242: toolbox-create-plan JSON should expose a stable result object");
    expect_contains(plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1242: toolbox-create-plan JSON should expose toolbox item ids");
    expect_contains(plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1242: toolbox-create-plan JSON should expose descriptor class names");
    expect_contains(plan_process.stdout_text, "\"baseClassName\": \"TextBox\"",
        "#1242: toolbox-create-plan JSON should expose descriptor base class names");
    expect_contains(plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1242: toolbox-create-plan JSON should expose requested toolbox context state");
    expect_contains(plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1242: toolbox-create-plan JSON should expose requested toolbox contexts");
    expect_contains(plan_process.stdout_text, "\"planReadyItemIds\": [\"textbox\"]",
        "#1405: toolbox-create-plan JSON should summarize plan-ready item ids");
    expect_contains(plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1405: toolbox-create-plan JSON should expose empty blocked item ids");
    expect_contains(plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#1405: toolbox-create-plan JSON should expose empty blocked plan errors");
    expect_contains(plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1242: toolbox-create-plan JSON should expose target record indexes");
    expect_contains(plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1242: toolbox-create-plan JSON should expose generated object names");
    expect_contains(plan_process.stdout_text, "\"uniqueId\": \"planned-textbox-guid\"",
        "#1242: toolbox-create-plan JSON should expose planned unique ids");
    expect_contains(plan_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1242: toolbox-create-plan JSON should expose planned parent names");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"OBJNAME\"",
        "#1242: toolbox-create-plan JSON should expose generated field values");
    expect_contains(plan_process.stdout_text, "\"propertyValue\": \"txt2\"",
        "#1242: toolbox-create-plan JSON should expose generated object-name values");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1242: toolbox-create-plan JSON should expose caller direct fields");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"PROPERTIES\"",
        "#1242: toolbox-create-plan JSON should expose caller memo fields");
    expect_contains(plan_process.stdout_text, "\"dryRun\": true",
        "#1242: toolbox-create-plan JSON should expose dry-run state");
    expect_contains(plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1242: toolbox-create-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1242: toolbox-create-plan host command should not mutate the visual asset");

    const auto report_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan", "label",
            "--toolbox-context", "report",
            "--unique-id", "direct-report-plan-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Direct Report Plan",
            "--json"
        },
        temp_root);
    expect(report_plan_process.exit_code == 0,
        "#2102: report toolbox-create-plan JSON command should exit successfully");
    expect_contains(report_plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#2102: report toolbox-create-plan JSON should expose create plans");
    expect_contains(report_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2102: report toolbox-create-plan JSON should expose label plans");
    expect_contains(report_plan_process.stdout_text, "\"className\": \"Label\"",
        "#2102: report toolbox-create-plan JSON should expose label descriptor metadata");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2102: report toolbox-create-plan JSON should preserve report contexts");
    expect_contains(report_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2102: report toolbox-create-plan JSON should summarize plan-ready report item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2102: report toolbox-create-plan JSON should summarize empty blocked item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2102: report toolbox-create-plan JSON should summarize empty plan errors");
    expect_contains(report_plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#2102: report toolbox-create-plan JSON should expose target record indexes");
    expect_contains(report_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2102: report toolbox-create-plan JSON should expose generated label names");
    expect_contains(report_plan_process.stdout_text, "\"uniqueId\": \"direct-report-plan-guid\"",
        "#2102: report toolbox-create-plan JSON should preserve label unique ids");
    expect_contains(report_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2102: report toolbox-create-plan JSON should preserve label parent overrides");
    expect_contains(report_plan_process.stdout_text, "\"propertyValue\": \"Direct Report Plan\"",
        "#2102: report toolbox-create-plan JSON should preserve label field values");
    expect_contains(report_plan_process.stdout_text, "\"dryRun\": true",
        "#2102: report toolbox-create-plan JSON should remain a dry-run plan");
    expect_contains(report_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2102: report toolbox-create-plan JSON should remain non-mutating");
    expect_not_contains(report_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2102: report toolbox-create-plan JSON should exclude form-only TextBox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2102: report toolbox-create-plan host command should not mutate assets");

    const auto unknown_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan", "missing-toolbox-item",
            "--json"
        },
        temp_root);
    expect(unknown_process.exit_code == 4,
        "#1242: unknown toolbox-create-plan ids should return command failure");
    expect_contains(unknown_process.stdout_text, "\"toolboxCreatePlan\": null",
        "#1242: failed toolbox-create-plan JSON should not expose stale plans");
    expect_contains(unknown_process.stdout_text, "The requested toolbox item was not found.",
        "#1242: failed toolbox-create-plan JSON should expose clean errors");
    expect(visual_object_count(form_path) == before_count,
        "#1242: failed toolbox-create-plan host commands should not mutate the visual asset");

    const auto invalid_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan", "textbox",
            "--toolbox-context", "missing",
            "--json"
        },
        temp_root);
    expect(invalid_context_process.exit_code == 2,
        "#1242: toolbox-create-plan JSON should reject invalid toolbox contexts");
    expect_contains(invalid_context_process.stdout_text, "Unknown toolbox context token: missing",
        "#1242: invalid toolbox-create-plan context JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1242: invalid toolbox-create-plan host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "visual_object",
            "--unique-id", "selected-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selected Customer",
            "--field-value", "PROPERTIES=ControlSource = \"customer.name\"",
            "--json"
        },
        temp_root);

    expect(plan_process.exit_code == 0,
        "#1301: selection-toolbox-create-plan JSON command should exit successfully");
    expect_contains(plan_process.stdout_text, "\"status\": \"ok\"",
        "#1301: successful selection-toolbox-create-plan JSON should report ok status");
    expect_contains(plan_process.stdout_text, "\"selectionToolboxCreatePlan\": {",
        "#1301: selection-toolbox-create-plan JSON should expose a stable result object");
    expect_contains(plan_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1301: selection-toolbox-create-plan JSON should expose selected contexts");
    expect_contains(plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1301: selection-toolbox-create-plan JSON should expose resolved toolbox contexts");
    expect_contains(plan_process.stdout_text, "\"launchPlanOk\": true",
        "#1301: selection-toolbox-create-plan JSON should expose launch plan status");
    expect_contains(plan_process.stdout_text, "\"createPlanOk\": true",
        "#1301: selection-toolbox-create-plan JSON should expose create plan status");
    expect_contains(plan_process.stdout_text, "\"createPlan\": {",
        "#1301: selection-toolbox-create-plan JSON should expose nested create plans");
    expect_contains(plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1301: selection-toolbox-create-plan JSON should expose toolbox item ids");
    expect_contains(plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1301: selection-toolbox-create-plan JSON should expose descriptor class names");
    expect_contains(plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1301: selection-toolbox-create-plan JSON should expose context-filtered planning");
    expect_contains(plan_process.stdout_text, "\"planReadyItemIds\": [\"textbox\"]",
        "#1405: selection-toolbox-create-plan JSON should summarize plan-ready item ids");
    expect_contains(plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1405: selection-toolbox-create-plan JSON should expose empty blocked item ids");
    expect_contains(plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#1405: selection-toolbox-create-plan JSON should expose empty blocked plan errors");
    expect_contains(plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1301: selection-toolbox-create-plan JSON should expose target record indexes");
    expect_contains(plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1301: selection-toolbox-create-plan JSON should expose generated object names");
    expect_contains(plan_process.stdout_text, "\"uniqueId\": \"selected-textbox-guid\"",
        "#1301: selection-toolbox-create-plan JSON should expose planned unique ids");
    expect_contains(plan_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1301: selection-toolbox-create-plan JSON should expose planned parent names");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1301: selection-toolbox-create-plan JSON should expose caller direct fields");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"PROPERTIES\"",
        "#1301: selection-toolbox-create-plan JSON should expose caller memo fields");
    expect_contains(plan_process.stdout_text, "\"dryRun\": true",
        "#1301: selection-toolbox-create-plan JSON should expose dry-run state");
    expect_contains(plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1301: selection-toolbox-create-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1301: selection-toolbox-create-plan host command should not mutate the visual asset");

    const auto report_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "label",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--json"
        },
        temp_root);
    expect(report_plan_process.exit_code == 0,
        "#1301: report selection-toolbox-create-plan JSON command should exit successfully");
    expect_contains(report_plan_process.stdout_text, "\"selectionToolboxCreatePlan\": {",
        "#2119: report selection-toolbox-create-plan JSON should expose a stable result object");
    expect_contains(report_plan_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1301: report selection-toolbox-create-plan JSON should expose report selections");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1301: report selection-toolbox-create-plan JSON should expose report contexts");
    expect_contains(report_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#2119: report selection-toolbox-create-plan JSON should expose launch plan status");
    expect_contains(report_plan_process.stdout_text, "\"createPlanOk\": true",
        "#2119: report selection-toolbox-create-plan JSON should expose create plan status");
    expect_contains(report_plan_process.stdout_text, "\"createPlan\": {",
        "#2119: report selection-toolbox-create-plan JSON should expose nested create plans");
    expect_contains(report_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1301: report selection-toolbox-create-plan JSON should expose label plans");
    expect_contains(report_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1301: report selection-toolbox-create-plan JSON should expose generated label names");
    expect_contains(report_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2119: report selection-toolbox-create-plan JSON should preserve report parent payloads");
    expect_contains(report_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2119: report selection-toolbox-create-plan JSON should summarize plan-ready report item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2119: report selection-toolbox-create-plan JSON should expose empty blocked item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2119: report selection-toolbox-create-plan JSON should expose empty blocked plan errors");
    expect_contains(report_plan_process.stdout_text, "\"dryRun\": true",
        "#2119: report selection-toolbox-create-plan JSON should expose dry-run state");
    expect_contains(report_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2119: report selection-toolbox-create-plan JSON should remain non-mutating");
    expect_not_contains(report_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1301: report selection-toolbox-create-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2119: report selection-toolbox-create-plan host command should not mutate assets");

    const auto label_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "label",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--json"
        },
        temp_root);
    expect(label_plan_process.exit_code == 0,
        "#2081: label selection-toolbox-create-plan JSON command should exit successfully");
    expect_contains(label_plan_process.stdout_text, "\"selectionToolboxCreatePlan\": {",
        "#2121: label selection-toolbox-create-plan JSON should expose a stable result object");
    expect_contains(label_plan_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2081: label selection-toolbox-create-plan JSON should expose label selections");
    expect_contains(label_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2081: label selection-toolbox-create-plan JSON should expose report contexts");
    expect_contains(label_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#2121: label selection-toolbox-create-plan JSON should expose launch plan status");
    expect_contains(label_plan_process.stdout_text, "\"createPlanOk\": true",
        "#2121: label selection-toolbox-create-plan JSON should expose create plan status");
    expect_contains(label_plan_process.stdout_text, "\"createPlan\": {",
        "#2121: label selection-toolbox-create-plan JSON should expose nested create plans");
    expect_contains(label_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2081: label selection-toolbox-create-plan JSON should expose label plans");
    expect_contains(label_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2081: label selection-toolbox-create-plan JSON should expose generated label names");
    expect_contains(label_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2121: label selection-toolbox-create-plan JSON should preserve label parent payloads");
    expect_contains(label_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2121: label selection-toolbox-create-plan JSON should summarize plan-ready label item ids");
    expect_contains(label_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2121: label selection-toolbox-create-plan JSON should expose empty blocked item ids");
    expect_contains(label_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2121: label selection-toolbox-create-plan JSON should expose empty blocked plan errors");
    expect_contains(label_plan_process.stdout_text, "\"dryRun\": true",
        "#2121: label selection-toolbox-create-plan JSON should expose dry-run state");
    expect_contains(label_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2121: label selection-toolbox-create-plan JSON should remain non-mutating");
    expect_not_contains(label_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2081: label selection-toolbox-create-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2081: label selection-toolbox-create-plan host command should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "report_expression",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1301: selection-toolbox-create-plan JSON should reject unavailable context items");
    expect_contains(unavailable_process.stdout_text, "\"selectionToolboxCreatePlan\": null",
        "#1301: unavailable selection-toolbox-create-plan JSON should suppress stale plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1301: unavailable selection-toolbox-create-plan JSON should report planner errors");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1301: selection-toolbox-create-plan JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionToolboxCreatePlan\": null",
        "#1301: unsupported selection-toolbox-create-plan JSON should suppress stale plans");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object creation plan request requires a toolbox palette.",
        "#1301: unsupported selection-toolbox-create-plan JSON should report palette errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1301: missing path selection-toolbox-create-plan JSON should report parser errors");

    const auto missing_item_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-context", "visual_object",
            "--selection-toolbox-create-plan",
            "--json"
        },
        temp_root);
    expect(missing_item_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject missing item ids");
    expect_contains(missing_item_process.stdout_text, "Missing value for --selection-toolbox-create-plan.",
        "#1301: missing item selection-toolbox-create-plan JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1301: missing selection selection-toolbox-create-plan JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1301: unknown selection selection-toolbox-create-plan JSON should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1301: malformed field selection-toolbox-create-plan JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-plan option: --toolbox-context",
        "#1301: unknown option selection-toolbox-create-plan JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1301: rejected selection-toolbox-create-plan host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_dispatch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-plan", "textbox",
            "--toolbox-context", "form",
            "--unique-id", "dispatch-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(dispatch_process.exit_code == 0,
        "#1250: toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(dispatch_process.stdout_text, "\"toolboxCreateDispatchPlan\": {",
        "#1250: toolbox-create-dispatch-plan JSON should expose a dispatch result object");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose toolbox item ids");
    expect_contains(dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose descriptor class names");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1250: toolbox-create-dispatch-plan JSON should expose requested context state");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose requested contexts");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1250: toolbox-create-dispatch-plan JSON should expose planned target record indexes");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose generated object names");
    expect_contains(dispatch_process.stdout_text, "\"uniqueId\": \"dispatch-textbox-guid\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose planned unique ids");
    expect_contains(dispatch_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose planned parent names");
    expect_contains(dispatch_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose planned field values");
    expect_contains(dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1250: toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-create\", \"textbox\"",
        "#1250: toolbox-create-dispatch-plan JSON should dispatch to toolbox-create");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-context\", \"form\"",
        "#1250: toolbox-create-dispatch-plan JSON should preserve toolbox context arguments");
    expect_contains(dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1250: toolbox-create-dispatch-plan JSON should preserve object-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch\"",
        "#1250: toolbox-create-dispatch-plan JSON should preserve caller field-value arguments");
    expect_contains(dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"textbox\"]",
        "#1386: toolbox-create-dispatch-plan JSON should summarize dispatch-ready item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1386: toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1386: successful toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1250: toolbox-create-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(dispatch_process.stdout_text, "\"dryRun\": false",
        "#1250: toolbox-create-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"executed\": false",
        "#1250: toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_contains(dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1250: toolbox-create-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1250: toolbox-create-dispatch-plan host command should not mutate the visual asset");

    const auto report_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-plan", "label",
            "--toolbox-context", "report",
            "--unique-id", "direct-report-dispatch-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Direct Report Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_dispatch_process.exit_code == 0,
        "#2103: report toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxCreateDispatchPlan\": {",
        "#2103: report toolbox-create-dispatch-plan JSON should expose dispatch plans");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2103: report toolbox-create-dispatch-plan JSON should expose label plans");
    expect_contains(report_dispatch_process.stdout_text, "\"className\": \"Label\"",
        "#2103: report toolbox-create-dispatch-plan JSON should expose label descriptor metadata");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve report contexts");
    expect_contains(report_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2103: report toolbox-create-dispatch-plan JSON should expose generated label names");
    expect_contains(report_dispatch_process.stdout_text, "\"uniqueId\": \"direct-report-dispatch-guid\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label unique ids");
    expect_contains(report_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label parent overrides");
    expect_contains(report_dispatch_process.stdout_text, "\"propertyValue\": \"Direct Report Dispatch\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label field values");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2103: report toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-create\", \"label\"",
        "#2103: report toolbox-create-dispatch-plan JSON should dispatch label creates");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve report context arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve generated label arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--unique-id\", \"direct-report-dispatch-guid\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Direct Report Dispatch\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label field arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#2103: report toolbox-create-dispatch-plan JSON should summarize dispatch-ready report item ids");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2103: report toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2103: report toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(report_dispatch_process.stdout_text, "\"executed\": false",
        "#2103: report toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_not_contains(report_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2103: report toolbox-create-dispatch-plan JSON should exclude form-only TextBox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2103: report toolbox-create-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-plan", "textbox",
            "--admit-create-operation", "false",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1250: toolbox-create-dispatch-plan JSON should reject non-admitted create operations");
    expect_contains(non_admitted_process.stdout_text, "\"toolboxCreateDispatchPlan\": null",
        "#1250: non-admitted toolbox-create-dispatch-plan JSON should not expose stale dispatch plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox create dispatch request requires an admitted non-dry-run create operation.",
        "#1250: non-admitted toolbox-create-dispatch-plan JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1386: non-admitted toolbox-create-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1386: non-admitted toolbox-create-dispatch-plan JSON should summarize aggregate blocked state");
    expect_contains(non_admitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1386: non-admitted toolbox-create-dispatch-plan JSON should summarize dispatch errors");
    expect_not_contains(non_admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1250: failed toolbox-create-dispatch-plan JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1250: non-admitted toolbox-create-dispatch-plan commands should not mutate the visual asset");

    const auto unknown_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-plan", "missing-toolbox-item",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unknown_process.exit_code == 4,
        "#1250: toolbox-create-dispatch-plan JSON should reject invalid create plans");
    expect_contains(unknown_process.stdout_text, "The requested toolbox item was not found.",
        "#1250: invalid toolbox-create-dispatch-plan create plans should report planning errors");
    expect_contains(unknown_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1386: invalid toolbox-create-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(unknown_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The requested toolbox item was not found.\"",
        "#1386: invalid toolbox-create-dispatch-plan JSON should summarize planning errors");
    expect_not_contains(unknown_process.stdout_text, "\"dispatchArguments\": [",
        "#1250: invalid toolbox-create-dispatch-plan create plans should not expose stale arguments");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-plan", "textbox",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1250: toolbox-create-dispatch-plan JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1250: invalid toolbox-create-dispatch-plan admission tokens should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_dispatch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--unique-id", "selection-dispatch-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(dispatch_process.exit_code == 0,
        "#1303: selection-toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(dispatch_process.stdout_text, "\"selectionToolboxCreateDispatchPlan\": {",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose a stable result object");
    expect_contains(dispatch_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose selected Studio contexts");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose resolved toolbox contexts");
    expect_contains(dispatch_process.stdout_text, "\"launchPlanOk\": true",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose launch state");
    expect_contains(dispatch_process.stdout_text, "\"createPlanOk\": true",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose create-plan state");
    expect_contains(dispatch_process.stdout_text, "\"dispatchOk\": true",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"dispatchCount\": 1",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose dispatch counts");
    expect_contains(dispatch_process.stdout_text, "\"errorCount\": 0",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose zero errors");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose toolbox item ids");
    expect_contains(dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose descriptor class names");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose planned target records");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose generated names");
    expect_contains(dispatch_process.stdout_text, "\"uniqueId\": \"selection-dispatch-textbox-guid\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose caller unique ids");
    expect_contains(dispatch_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose parent names");
    expect_contains(dispatch_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose caller field names");
    expect_contains(dispatch_process.stdout_text, "\"propertyValue\": \"Selection Dispatch\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose caller field values");
    expect_contains(dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-create\", \"textbox\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should dispatch to toolbox-create");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-context\", \"form\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve resolved toolbox contexts");
    expect_contains(dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve object-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--unique-id\", \"selection-dispatch-textbox-guid\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve unique-id arguments");
    expect_contains(dispatch_process.stdout_text, "\"--parent-name\", \"frmCustomer\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve parent-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Selection Dispatch\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve field-value arguments");
    expect_contains(dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"textbox\"]",
        "#1386: selection-toolbox-create-dispatch-plan JSON should summarize dispatch-ready item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1386: selection-toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1386: successful selection-toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(dispatch_process.stdout_text, "\"dryRun\": false",
        "#1303: admitted selection-toolbox-create-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(dispatch_process.stdout_text, "\"executed\": false",
        "#1303: selection-toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_contains(dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1303: admitted selection-toolbox-create-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1303: admitted selection-toolbox-create-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--admit-create-operation", "false",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject non-admitted creates");
    expect_contains(non_admitted_process.stdout_text, "\"createPlanOk\": true",
        "#1303: non-admitted selection-toolbox-create-dispatch-plan JSON should preserve create-plan state");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchOk\": false",
        "#1303: non-admitted selection-toolbox-create-dispatch-plan JSON should expose dispatch failures");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox create dispatch request requires an admitted non-dry-run create operation.",
        "#1303: non-admitted selection-toolbox-create-dispatch-plan JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1386: non-admitted selection-toolbox-create-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1386: non-admitted selection-toolbox-create-dispatch-plan JSON should summarize aggregate blocked state");
    expect_contains(non_admitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1386: non-admitted selection-toolbox-create-dispatch-plan JSON should summarize dispatch errors");
    expect_not_contains(non_admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1303: non-admitted selection-toolbox-create-dispatch-plan JSON should omit stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1303: non-admitted selection-toolbox-create-dispatch-plan host command should not mutate assets");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "label",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1303: report selection-toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"selectionToolboxCreateDispatchPlan\": {",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"launchPlanOk\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose launch state");
    expect_contains(report_process.stdout_text, "\"createPlanOk\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose create-plan state");
    expect_contains(report_process.stdout_text, "\"dispatchOk\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose dispatch state");
    expect_contains(report_process.stdout_text, "\"dispatchCount\": 1",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose dispatch counts");
    expect_contains(report_process.stdout_text, "\"errorCount\": 0",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose zero errors");
    expect_contains(report_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should expose label dispatches");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should expose generated label names");
    expect_contains(report_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should preserve report parent payloads");
    expect_contains(report_process.stdout_text, "\"dispatchArguments\": [",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_process.stdout_text, "\"--toolbox-create\", \"label\"",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should dispatch to toolbox-create");
    expect_contains(report_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should preserve report context arguments");
    expect_contains(report_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should preserve report parent arguments");
    expect_contains(report_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#1386: report selection-toolbox-create-dispatch-plan JSON should summarize dispatch-ready report item ids");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1386: report selection-toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(report_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(report_process.stdout_text, "\"executed\": false",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2117: report selection-toolbox-create-dispatch-plan host command should not mutate assets");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "label",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2082: label selection-toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"selectionToolboxCreateDispatchPlan\": {",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should expose label selections");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"launchPlanOk\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose launch state");
    expect_contains(label_process.stdout_text, "\"createPlanOk\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose create-plan state");
    expect_contains(label_process.stdout_text, "\"dispatchOk\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose dispatch state");
    expect_contains(label_process.stdout_text, "\"dispatchCount\": 1",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose dispatch counts");
    expect_contains(label_process.stdout_text, "\"errorCount\": 0",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose zero errors");
    expect_contains(label_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should expose label dispatches");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should expose generated label names");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should preserve label parent payloads");
    expect_contains(label_process.stdout_text, "\"dispatchArguments\": [",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(label_process.stdout_text, "\"--toolbox-create\", \"label\"",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should dispatch to toolbox-create");
    expect_contains(label_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should preserve report context arguments");
    expect_contains(label_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(label_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should summarize dispatch-ready label item ids");
    expect_contains(label_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(label_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(label_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(label_process.stdout_text, "\"executed\": false",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2123: label selection-toolbox-create-dispatch-plan host command should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "report_expression",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject unavailable selected-context items");
    expect_contains(unavailable_process.stdout_text, "\"createPlanOk\": false",
        "#1303: unavailable selection-toolbox-create-dispatch-plan JSON should expose failed create-plan state");
    expect_contains(unavailable_process.stdout_text, "\"dispatchOk\": false",
        "#1303: unavailable selection-toolbox-create-dispatch-plan JSON should avoid stale dispatches");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1303: unavailable selection-toolbox-create-dispatch-plan JSON should report planner errors");
    expect_contains(unavailable_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1386: unavailable selection-toolbox-create-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(unavailable_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1386: unavailable selection-toolbox-create-dispatch-plan JSON should summarize planner errors");
    expect_not_contains(unavailable_process.stdout_text, "\"dispatchArguments\": [",
        "#1303: unavailable selection-toolbox-create-dispatch-plan JSON should omit stale arguments");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "menu_item",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1303: unsupported selection-toolbox-create-dispatch-plan JSON should preserve selected contexts");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object creation plan request requires a toolbox palette.",
        "#1303: unsupported selection-toolbox-create-dispatch-plan JSON should report palette errors");
    expect_not_contains(unsupported_process.stdout_text, "\"dispatchArguments\": [",
        "#1303: unsupported selection-toolbox-create-dispatch-plan JSON should omit stale arguments");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1303: missing path selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto missing_item_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-context", "visual_object",
            "--selection-toolbox-create-dispatch-plan",
            "--json"
        },
        temp_root);
    expect(missing_item_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject missing item ids");
    expect_contains(missing_item_process.stdout_text,
        "Missing value for --selection-toolbox-create-dispatch-plan.",
        "#1303: missing item selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1303: missing selection selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1303: unknown selection selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1303: invalid selection-toolbox-create-dispatch-plan admission tokens should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1303: malformed field selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-dispatch-plan option: --toolbox-context",
        "#1303: unknown option selection-toolbox-create-dispatch-plan JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1303: rejected selection-toolbox-create-dispatch-plan host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_plan_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Planned",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1244: toolbox-create-plan-catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"toolboxCreatePlanCatalog\": {",
        "#1244: toolbox-create-plan-catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1244: toolbox-create-plan-catalog JSON should expose toolbox contexts");
    expect_contains(catalog_process.stdout_text, "\"itemCount\": ",
        "#1244: toolbox-create-plan-catalog JSON should expose item counts");
    expect_contains(catalog_process.stdout_text, "\"planCount\": ",
        "#1244: toolbox-create-plan-catalog JSON should expose plan counts");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1244: toolbox-create-plan-catalog JSON should expose zero error counts");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1244: form toolbox-create-plan-catalog JSON should include textbox plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1244: form toolbox-create-plan-catalog JSON should include command button plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1244: form toolbox-create-plan-catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1244: form toolbox-create-plan-catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1244: toolbox-create-plan-catalog JSON should expose planned parent names");
    expect_contains(catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1244: toolbox-create-plan-catalog JSON should expose caller field values");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": true",
        "#1244: toolbox-create-plan-catalog JSON should expose dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1244: toolbox-create-plan-catalog JSON should remain non-mutating");
    expect_contains(catalog_process.stdout_text,
        "\"planReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1374: toolbox-create-plan-catalog JSON should summarize plan-ready form items");
    expect_contains(catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1374: toolbox-create-plan-catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1374: toolbox-create-plan-catalog JSON should summarize empty blocked plan errors");
    expect(visual_object_count(form_path) == before_count,
        "#1244: toolbox-create-plan-catalog host command should not mutate the visual asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan-catalog",
            "--toolbox-context", "report",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Plan Catalog",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1244: report toolbox-create-plan-catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxCreatePlanCatalog\": {",
        "#2107: report toolbox-create-plan-catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1244: report toolbox-create-plan-catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"planCount\": ",
        "#2107: report toolbox-create-plan-catalog JSON should expose report plan counts");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2107: report toolbox-create-plan-catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1244: report toolbox-create-plan-catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#1374: report toolbox-create-plan-catalog JSON should summarize plan-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1374: report toolbox-create-plan-catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1374: report toolbox-create-plan-catalog JSON should summarize empty blocked plan errors");
    expect_contains(report_catalog_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1244: report toolbox-create-plan-catalog JSON should expose generated label names");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2107: report toolbox-create-plan-catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2143: report toolbox-create-plan-catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Plan Catalog\"",
        "#2143: report toolbox-create-plan-catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": true",
        "#2107: report toolbox-create-plan-catalog JSON should remain dry-run");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2107: report toolbox-create-plan-catalog JSON should remain non-mutating");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1244: report toolbox-create-plan-catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#1244: report toolbox-create-plan-catalog host command should not mutate the visual asset");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1244: toolbox-create-plan-catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1244: missing toolbox-create-plan-catalog context JSON should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan-catalog",
            "--toolbox-context", "form",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1244: toolbox-create-plan-catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1244: malformed toolbox-create-plan-catalog field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1244: rejected toolbox-create-plan-catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_plan_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto visual_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Planned",
            "--json"
        },
        temp_root);
    expect(visual_catalog_process.exit_code == 0,
        "#1293: selection toolbox create-plan catalog JSON command should exit successfully");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionToolboxCreatePlanCatalog\": {",
        "#1293: selection toolbox create-plan catalog JSON should expose a catalog object");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1293: selection toolbox create-plan catalog JSON should expose selected Studio contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1293: visual selection toolbox create-plan catalog JSON should resolve form contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#1293: selection toolbox create-plan catalog JSON should expose launch state");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1293: visual selection toolbox create-plan catalog JSON should include textbox plans");
    expect_contains(visual_catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1293: visual selection toolbox create-plan catalog JSON should expose generated names");
    expect_contains(visual_catalog_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1293: selection toolbox create-plan catalog JSON should expose planned parents");
    expect_contains(visual_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1293: selection toolbox create-plan catalog JSON should expose caller field values");
    expect_contains(visual_catalog_process.stdout_text, "\"dryRun\": true",
        "#1293: selection toolbox create-plan catalog JSON should expose dry-run state");
    expect_contains(visual_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1293: selection toolbox create-plan catalog JSON should remain non-mutating");
    expect_contains(visual_catalog_process.stdout_text,
        "\"planReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1375: selection toolbox create-plan catalog JSON should summarize plan-ready visual items");
    expect_contains(visual_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1375: selection toolbox create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(visual_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1375: selection toolbox create-plan catalog JSON should summarize empty blocked plan errors");
    expect(visual_object_count(form_path) == before_count,
        "#1293: visual selection toolbox create-plan catalog host command should not mutate assets");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Plan Catalog",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1293: report selection toolbox create-plan catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"selectionToolboxCreatePlanCatalog\": {",
        "#2111: report selection toolbox create-plan catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1293: report selection toolbox create-plan catalog JSON should expose report selections");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1293: report selection toolbox create-plan catalog JSON should resolve report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2111: report selection toolbox create-plan catalog JSON should expose launch state");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1293: report selection toolbox create-plan catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#1375: report selection toolbox create-plan catalog JSON should summarize plan-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1375: report selection toolbox create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1375: report selection toolbox create-plan catalog JSON should summarize empty blocked plan errors");
    expect_contains(report_catalog_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1293: report selection toolbox create-plan catalog JSON should expose generated labels");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2111: report selection toolbox create-plan catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2148: report selection toolbox create-plan catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Selection Plan Catalog\"",
        "#2148: report selection toolbox create-plan catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": true",
        "#2111: report selection toolbox create-plan catalog JSON should remain dry-run");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2111: report selection toolbox create-plan catalog JSON should remain non-mutating");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1293: report selection toolbox create-plan catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#1293: report selection toolbox create-plan catalog host command should not mutate assets");

    const auto label_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Plan Catalog",
            "--json"
        },
        temp_root);
    expect(label_catalog_process.exit_code == 0,
        "#2080: label selection toolbox create-plan catalog JSON command should exit successfully");
    expect_contains(label_catalog_process.stdout_text, "\"selectionToolboxCreatePlanCatalog\": {",
        "#2125: label selection toolbox create-plan catalog JSON should expose a catalog object");
    expect_contains(label_catalog_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2080: label selection toolbox create-plan catalog JSON should expose label selections");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2080: label selection toolbox create-plan catalog JSON should resolve report contexts");
    expect_contains(label_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2125: label selection toolbox create-plan catalog JSON should expose launch state");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2080: label selection toolbox create-plan catalog JSON should include label plans");
    expect_contains(label_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#2080: label selection toolbox create-plan catalog JSON should summarize plan-ready label items");
    expect_contains(label_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2080: label selection toolbox create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(label_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#2080: label selection toolbox create-plan catalog JSON should summarize empty blocked plan errors");
    expect_contains(label_catalog_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2080: label selection toolbox create-plan catalog JSON should expose generated labels");
    expect_contains(label_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2125: label selection toolbox create-plan catalog JSON should preserve label parent payloads");
    expect_contains(label_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2150: label selection toolbox create-plan catalog JSON should expose caller field names");
    expect_contains(label_catalog_process.stdout_text, "\"propertyValue\": \"Label Selection Plan Catalog\"",
        "#2150: label selection toolbox create-plan catalog JSON should expose caller field values");
    expect_contains(label_catalog_process.stdout_text, "\"dryRun\": true",
        "#2125: label selection toolbox create-plan catalog JSON should remain dry-run");
    expect_contains(label_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2125: label selection toolbox create-plan catalog JSON should remain non-mutating");
    expect_not_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#2080: label selection toolbox create-plan catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2125: label selection toolbox create-plan catalog host command should not mutate assets");

    const auto unsupported_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_catalog_process.exit_code == 4,
        "#1293: selection toolbox create-plan catalog JSON should reject unsupported selections");
    expect_contains(unsupported_catalog_process.stdout_text, "\"selectionToolboxCreatePlanCatalog\": null",
        "#1293: unsupported selection toolbox create-plan catalog JSON should omit catalog objects");
    expect_contains(unsupported_catalog_process.stdout_text,
        "A selection-context toolbox object creation catalog request requires a toolbox palette.",
        "#1293: unsupported selection toolbox create-plan catalog JSON should report planner errors");
    expect(visual_object_count(form_path) == before_count,
        "#1293: unsupported selection toolbox create-plan catalog host command should not mutate assets");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1293: selection toolbox create-plan catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1293: missing selection toolbox create-plan catalog JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1293: selection toolbox create-plan catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1293: unknown selection toolbox create-plan catalog JSON should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1293: selection toolbox create-plan catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1293: malformed selection toolbox create-plan catalog field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1293: selection toolbox create-plan catalog JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-plan-catalog option: --toolbox-context",
        "#1293: unknown selection toolbox create-plan catalog options should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1293: rejected selection toolbox create-plan catalog host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_dispatch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1254: toolbox-create-dispatch-catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"toolboxCreateDispatchCatalog\": {",
        "#1254: toolbox-create-dispatch-catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1254: toolbox-create-dispatch-catalog JSON should expose requested contexts");
    expect_contains(catalog_process.stdout_text, "\"itemCount\": ",
        "#1254: toolbox-create-dispatch-catalog JSON should expose item counts");
    expect_contains(catalog_process.stdout_text, "\"dispatchCount\": ",
        "#1254: toolbox-create-dispatch-catalog JSON should expose dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1254: admitted toolbox-create-dispatch-catalog JSON should expose zero errors");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1254: admitted toolbox-create-dispatch-catalog JSON should expose non-dry-run dispatch state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#1254: admitted toolbox-create-dispatch-catalog JSON should expose mutation intent");
    expect_contains(catalog_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1376: toolbox-create-dispatch-catalog JSON should summarize dispatch-ready form items");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1376: admitted toolbox-create-dispatch-catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1376: admitted toolbox-create-dispatch-catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1254: toolbox-create-dispatch-catalog JSON should include textbox entries");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1254: toolbox-create-dispatch-catalog JSON should include command button entries");
    expect_contains(catalog_process.stdout_text, "\"createPlanOk\": true",
        "#1254: toolbox-create-dispatch-catalog JSON should expose create plan state");
    expect_contains(catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1254: toolbox-create-dispatch-catalog JSON should expose dispatch state");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1254: toolbox-create-dispatch-catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1254: toolbox-create-dispatch-catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [",
        "#1254: toolbox-create-dispatch-catalog JSON should expose dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-create\", \"textbox\"",
        "#1254: toolbox-create-dispatch-catalog JSON should emit textbox dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-create\", \"commandbutton\"",
        "#1254: toolbox-create-dispatch-catalog JSON should emit command dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Catalog\"",
        "#1254: toolbox-create-dispatch-catalog JSON should preserve shared field values");
    expect_contains(catalog_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1254: toolbox-create-dispatch-catalog JSON should expose dispatch admission state");
    expect(visual_object_count(form_path) == before_count,
        "#1254: toolbox-create-dispatch-catalog host command should not mutate the visual asset");

    const auto dry_run_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(dry_run_catalog_process.exit_code == 0,
        "#1254: non-admitted toolbox-create-dispatch-catalog JSON should return a catalog");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchCount\": 0",
        "#1254: non-admitted toolbox-create-dispatch-catalog JSON should expose zero dispatches");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1376: non-admitted toolbox-create-dispatch-catalog JSON should summarize empty ready item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1376: non-admitted toolbox-create-dispatch-catalog JSON should summarize blocked form item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1376: non-admitted toolbox-create-dispatch-catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_catalog_process.stdout_text,
        "A toolbox create dispatch request requires an admitted non-dry-run create operation.",
        "#1254: non-admitted toolbox-create-dispatch-catalog JSON should expose per-item errors");
    expect_not_contains(dry_run_catalog_process.stdout_text, "\"--toolbox-create\"",
        "#1254: non-admitted toolbox-create-dispatch-catalog JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1254: non-admitted toolbox-create-dispatch-catalog host command should not mutate the asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "report",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1254: report toolbox-create-dispatch-catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxCreateDispatchCatalog\": {",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1254: report toolbox-create-dispatch-catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchCount\": ",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose report dispatch counts");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1254: report toolbox-create-dispatch-catalog JSON should include label dispatches");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#1376: report toolbox-create-dispatch-catalog JSON should summarize dispatch-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1376: report toolbox-create-dispatch-catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1376: report toolbox-create-dispatch-catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2108: report toolbox-create-dispatch-catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2145: report toolbox-create-dispatch-catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Dispatch Catalog\"",
        "#2145: report toolbox-create-dispatch-catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1254: report toolbox-create-dispatch-catalog JSON should preserve report dispatch context");
    expect_contains(report_catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Report Dispatch Catalog\"",
        "#2145: report toolbox-create-dispatch-catalog JSON should preserve report dispatch field arguments");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": false",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose non-dry-run dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose mutation intent");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1254: report toolbox-create-dispatch-catalog JSON should exclude form-only textbox dispatches");
    expect(visual_object_count(form_path) == before_count,
        "#2108: report toolbox-create-dispatch-catalog host command should not mutate assets");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1254: toolbox-create-dispatch-catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1254: missing toolbox-create-dispatch-catalog context JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "form",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1254: toolbox-create-dispatch-catalog JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1254: invalid toolbox-create-dispatch-catalog admission tokens should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "form",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1254: toolbox-create-dispatch-catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1254: malformed toolbox-create-dispatch-catalog field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1254: rejected toolbox-create-dispatch-catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation_dispatch_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto visual_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(visual_catalog_process.exit_code == 0,
        "#1295: selection toolbox create-dispatch catalog JSON command should exit successfully");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionToolboxCreateDispatchCatalog\": {",
        "#1295: selection toolbox create-dispatch catalog JSON should expose catalog objects");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1295: selection toolbox create-dispatch catalog JSON should expose selected Studio contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1295: visual selection toolbox create-dispatch catalog JSON should resolve form contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#1295: selection toolbox create-dispatch catalog JSON should expose launch state");
    expect_contains(visual_catalog_process.stdout_text, "\"dispatchCount\": ",
        "#1295: selection toolbox create-dispatch catalog JSON should expose dispatch counts");
    expect_contains(visual_catalog_process.stdout_text, "\"errorCount\": 0",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose zero errors");
    expect_contains(visual_catalog_process.stdout_text, "\"dryRun\": false",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose non-dry-run state");
    expect_contains(visual_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose mutation intent");
    expect_contains(visual_catalog_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1377: selection toolbox create-dispatch catalog JSON should summarize dispatch-ready visual items");
    expect_contains(visual_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1377: admitted selection toolbox create-dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(visual_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1377: admitted selection toolbox create-dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1295: visual selection toolbox create-dispatch catalog JSON should include textbox entries");
    expect_contains(visual_catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1295: visual selection toolbox create-dispatch catalog JSON should expose generated names");
    expect_contains(visual_catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose dispatch state");
    expect_contains(visual_catalog_process.stdout_text, "\"--toolbox-create\", \"textbox\"",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose create arguments");
    expect_contains(visual_catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Selection Dispatch Catalog\"",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should preserve field values");
    expect(visual_object_count(form_path) == before_count,
        "#1295: admitted selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto dry_run_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(dry_run_catalog_process.exit_code == 0,
        "#1295: non-admitted selection toolbox create-dispatch catalog JSON should return catalogs");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchCount\": 0",
        "#1295: non-admitted selection toolbox create-dispatch catalog JSON should expose zero dispatches");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1377: non-admitted selection toolbox create-dispatch catalog JSON should summarize empty ready item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1377: non-admitted selection toolbox create-dispatch catalog JSON should summarize blocked visual item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1377: non-admitted selection toolbox create-dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_catalog_process.stdout_text,
        "A toolbox create dispatch request requires an admitted non-dry-run create operation.",
        "#1295: non-admitted selection toolbox create-dispatch catalog JSON should expose dispatch errors");
    expect_not_contains(dry_run_catalog_process.stdout_text, "\"--toolbox-create\"",
        "#1295: non-admitted selection toolbox create-dispatch catalog JSON should omit stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1295: non-admitted selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1295: report selection toolbox create-dispatch catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"selectionToolboxCreateDispatchCatalog\": {",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should expose report selections");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should resolve report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose launch state");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchCount\": ",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose dispatch counts");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should include label dispatches");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#1377: report selection toolbox create-dispatch catalog JSON should summarize dispatch-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1377: report selection toolbox create-dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1377: report selection toolbox create-dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2112: report selection toolbox create-dispatch catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2149: report selection toolbox create-dispatch catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Selection Dispatch Catalog\"",
        "#2149: report selection toolbox create-dispatch catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should preserve report contexts");
    expect_contains(report_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Report Selection Dispatch Catalog\"",
        "#2149: report selection toolbox create-dispatch catalog JSON should preserve report dispatch field arguments");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": false",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose non-dry-run dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose mutation intent");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should exclude form-only textbox entries");
    expect(visual_object_count(form_path) == before_count,
        "#1295: report selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto label_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_catalog_process.exit_code == 0,
        "#2083: label selection toolbox create-dispatch catalog JSON command should exit successfully");
    expect_contains(label_catalog_process.stdout_text, "\"selectionToolboxCreateDispatchCatalog\": {",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose a catalog object");
    expect_contains(label_catalog_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should expose label selections");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should resolve report contexts");
    expect_contains(label_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose launch state");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchCount\": ",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose dispatch counts");
    expect_contains(label_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose zero catalog errors");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should include label dispatches");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should summarize dispatch-ready label items");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2083: label selection toolbox create-dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2083: label selection toolbox create-dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(label_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2126: label selection toolbox create-dispatch catalog JSON should preserve label parent payloads");
    expect_contains(label_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2151: label selection toolbox create-dispatch catalog JSON should expose caller field names");
    expect_contains(label_catalog_process.stdout_text, "\"propertyValue\": \"Label Selection Dispatch Catalog\"",
        "#2151: label selection toolbox create-dispatch catalog JSON should expose caller field values");
    expect_contains(label_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should preserve report contexts");
    expect_contains(label_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Label Selection Dispatch Catalog\"",
        "#2151: label selection toolbox create-dispatch catalog JSON should preserve label dispatch field arguments");
    expect_contains(label_catalog_process.stdout_text, "\"dryRun\": false",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose non-dry-run dispatch state");
    expect_contains(label_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose mutation intent");
    expect_not_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should exclude form-only textbox entries");
    expect(visual_object_count(form_path) == before_count,
        "#2126: label selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto unsupported_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "menu_item",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unsupported_catalog_process.exit_code == 4,
        "#1295: selection toolbox create-dispatch catalog JSON should reject unsupported selections");
    expect_contains(unsupported_catalog_process.stdout_text, "\"selectionToolboxCreateDispatchCatalog\": null",
        "#1295: unsupported selection toolbox create-dispatch catalog JSON should omit catalog objects");
    expect_contains(unsupported_catalog_process.stdout_text,
        "A selection-context toolbox object creation dispatch catalog request requires a toolbox palette.",
        "#1295: unsupported selection toolbox create-dispatch catalog JSON should report planner errors");
    expect(visual_object_count(form_path) == before_count,
        "#1295: unsupported selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1295: missing selection toolbox create-dispatch catalog JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1295: unknown selection toolbox create-dispatch catalog JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1295: invalid selection toolbox create-dispatch catalog admission tokens should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1295: malformed selection toolbox create-dispatch catalog field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-dispatch-catalog option: --toolbox-context",
        "#1295: unknown selection toolbox create-dispatch catalog options should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1295: rejected selection toolbox create-dispatch catalog host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_batch_plan_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_plan_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Batch Plan Catalog",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1258: toolbox-create-batch-plan-catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"toolboxCreateBatchPlanCatalog\": {",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose requested contexts");
    expect_contains(catalog_process.stdout_text, "\"planCount\": 1",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose one batch plan");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose zero errors");
    expect_contains(catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose batch plan state");
    expect_contains(catalog_process.stdout_text, "\"batchPlan\": {",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose nested batch plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should include textbox plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should include command button plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"propertyValue\": \"Batch Plan Catalog\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should preserve shared field values");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": true",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1258: toolbox-create-batch-plan-catalog JSON should remain non-mutating");
    expect_contains(catalog_process.stdout_text,
        "\"planReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1378: toolbox-create-batch-plan-catalog JSON should summarize plan-ready form items");
    expect_contains(catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1378: toolbox-create-batch-plan-catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1378: toolbox-create-batch-plan-catalog JSON should summarize empty blocked plan errors");
    expect(visual_object_count(form_path) == before_count,
        "#1258: toolbox-create-batch-plan-catalog host command should not mutate the visual asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan-catalog",
            "--toolbox-context", "report",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Batch Plan Catalog",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1258: report toolbox-create-batch-plan-catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxCreateBatchPlanCatalog\": {",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1258: report toolbox-create-batch-plan-catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"planCount\": 1",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose one report batch plan");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose nested batch plans");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1258: report toolbox-create-batch-plan-catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#1378: report toolbox-create-batch-plan-catalog JSON should summarize plan-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1378: report toolbox-create-batch-plan-catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1378: report toolbox-create-batch-plan-catalog JSON should summarize empty blocked plan errors");
    expect_contains(report_catalog_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose generated label names");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2109: report toolbox-create-batch-plan-catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2146: report toolbox-create-batch-plan-catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Batch Plan Catalog\"",
        "#2146: report toolbox-create-batch-plan-catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": true",
        "#2109: report toolbox-create-batch-plan-catalog JSON should remain dry-run");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2109: report toolbox-create-batch-plan-catalog JSON should remain non-mutating");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1258: report toolbox-create-batch-plan-catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2109: report toolbox-create-batch-plan-catalog host command should not mutate assets");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1258: toolbox-create-batch-plan-catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1258: missing toolbox-create-batch-plan-catalog context JSON should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan-catalog",
            "--toolbox-context", "form",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1258: toolbox-create-batch-plan-catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1258: malformed toolbox-create-batch-plan-catalog field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1258: rejected toolbox-create-batch-plan-catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation_batch_plan_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_batch_plan_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto visual_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Batch Plan",
            "--json"
        },
        temp_root);
    expect(visual_catalog_process.exit_code == 0,
        "#1297: selection toolbox batch create-plan catalog JSON command should exit successfully");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionToolboxCreateBatchPlanCatalog\": {",
        "#1297: selection toolbox batch create-plan catalog JSON should expose catalog objects");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1297: selection toolbox batch create-plan catalog JSON should expose selected Studio contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should resolve form contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#1297: selection toolbox batch create-plan catalog JSON should expose launch state");
    expect_contains(visual_catalog_process.stdout_text, "\"planCount\": 1",
        "#1297: selection toolbox batch create-plan catalog JSON should expose one batch plan");
    expect_contains(visual_catalog_process.stdout_text, "\"errorCount\": 0",
        "#1297: selection toolbox batch create-plan catalog JSON should expose zero errors");
    expect_contains(visual_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1297: selection toolbox batch create-plan catalog JSON should expose batch plan state");
    expect_contains(visual_catalog_process.stdout_text, "\"batchPlan\": {",
        "#1297: selection toolbox batch create-plan catalog JSON should expose nested batch plans");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should include textbox plans");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should include command button plans");
    expect_contains(visual_catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should expose generated textbox names");
    expect_contains(visual_catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should expose generated command names");
    expect_contains(visual_catalog_process.stdout_text, "\"propertyValue\": \"Selection Batch Plan\"",
        "#1297: selection toolbox batch create-plan catalog JSON should preserve shared field values");
    expect_contains(visual_catalog_process.stdout_text, "\"dryRun\": true",
        "#1297: selection toolbox batch create-plan catalog JSON should expose dry-run state");
    expect_contains(visual_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1297: selection toolbox batch create-plan catalog JSON should remain non-mutating");
    expect_contains(visual_catalog_process.stdout_text,
        "\"planReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1379: selection toolbox batch create-plan catalog JSON should summarize plan-ready visual items");
    expect_contains(visual_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1379: selection toolbox batch create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(visual_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1379: selection toolbox batch create-plan catalog JSON should summarize empty blocked plan errors");
    expect(visual_object_count(form_path) == before_count,
        "#1297: visual selection toolbox batch create-plan catalog host command should not mutate assets");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Batch Plan Catalog",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1297: report selection toolbox batch create-plan catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"selectionToolboxCreateBatchPlanCatalog\": {",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose catalog objects");
    expect_contains(report_catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1297: report selection toolbox batch create-plan catalog JSON should expose report selections");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1297: report selection toolbox batch create-plan catalog JSON should resolve report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose launch state");
    expect_contains(report_catalog_process.stdout_text, "\"planCount\": 1",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose one report batch plan");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose nested batch plans");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1297: report selection toolbox batch create-plan catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#1379: report selection toolbox batch create-plan catalog JSON should summarize plan-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1379: report selection toolbox batch create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1379: report selection toolbox batch create-plan catalog JSON should summarize empty blocked plan errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2113: report selection toolbox batch create-plan catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2152: report selection toolbox batch create-plan catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Selection Batch Plan Catalog\"",
        "#2152: report selection toolbox batch create-plan catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": true",
        "#2113: report selection toolbox batch create-plan catalog JSON should remain dry-run");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2113: report selection toolbox batch create-plan catalog JSON should remain non-mutating");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1297: report selection toolbox batch create-plan catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#1297: report selection toolbox batch create-plan catalog host command should not mutate assets");

    const auto label_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Batch Plan Catalog",
            "--json"
        },
        temp_root);
    expect(label_catalog_process.exit_code == 0,
        "#2089: label selection toolbox batch create-plan catalog JSON command should exit successfully");
    expect_contains(label_catalog_process.stdout_text, "\"selectionToolboxCreateBatchPlanCatalog\": {",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose catalog objects");
    expect_contains(label_catalog_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should expose label selections");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should resolve report contexts");
    expect_contains(label_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose launch state");
    expect_contains(label_catalog_process.stdout_text, "\"planCount\": 1",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose one label batch plan");
    expect_contains(label_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose zero catalog errors");
    expect_contains(label_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose nested batch plans");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should include label plans");
    expect_contains(label_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should summarize plan-ready label items");
    expect_contains(label_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2089: label selection toolbox batch create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(label_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#2089: label selection toolbox batch create-plan catalog JSON should summarize empty blocked plan errors");
    expect_contains(label_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2127: label selection toolbox batch create-plan catalog JSON should preserve label parent payloads");
    expect_contains(label_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2154: label selection toolbox batch create-plan catalog JSON should expose caller field names");
    expect_contains(label_catalog_process.stdout_text, "\"propertyValue\": \"Label Selection Batch Plan Catalog\"",
        "#2154: label selection toolbox batch create-plan catalog JSON should expose caller field values");
    expect_contains(label_catalog_process.stdout_text, "\"dryRun\": true",
        "#2127: label selection toolbox batch create-plan catalog JSON should remain dry-run");
    expect_contains(label_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2127: label selection toolbox batch create-plan catalog JSON should remain non-mutating");
    expect_not_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2127: label selection toolbox batch create-plan catalog host command should not mutate assets");

    const auto unsupported_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_catalog_process.exit_code == 4,
        "#1297: selection toolbox batch create-plan catalog JSON should reject unsupported selections");
    expect_contains(unsupported_catalog_process.stdout_text, "\"selectionToolboxCreateBatchPlanCatalog\": null",
        "#1297: unsupported selection toolbox batch create-plan catalog JSON should omit catalog objects");
    expect_contains(unsupported_catalog_process.stdout_text,
        "A selection-context toolbox object batch creation catalog request requires a toolbox palette.",
        "#1297: unsupported selection toolbox batch create-plan catalog JSON should report planner errors");
    expect(visual_object_count(form_path) == before_count,
        "#1297: unsupported selection toolbox batch create-plan catalog host command should not mutate assets");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1297: selection toolbox batch create-plan catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1297: missing selection toolbox batch create-plan catalog JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1297: selection toolbox batch create-plan catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1297: unknown selection toolbox batch create-plan catalog JSON should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1297: selection toolbox batch create-plan catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1297: malformed selection toolbox batch create-plan catalog field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1297: selection toolbox batch create-plan catalog JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch-plan-catalog option: --toolbox-context",
        "#1297: unknown selection toolbox batch create-plan catalog options should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1297: rejected selection toolbox batch create-plan catalog host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_batch_dispatch_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Batch Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1256: toolbox-create-batch-dispatch-catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"toolboxCreateBatchDispatchCatalog\": {",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose requested contexts");
    expect_contains(catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#1256: admitted toolbox-create-batch-dispatch-catalog JSON should expose one batch dispatch");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1256: admitted toolbox-create-batch-dispatch-catalog JSON should expose zero errors");
    expect_contains(catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose batch plan state");
    expect_contains(catalog_process.stdout_text, "\"batchPlan\": {",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose nested batch plans");
    expect_contains(catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose dispatch state");
    expect_contains(catalog_process.stdout_text, "\"dispatch\": {",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose nested dispatch plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should include textbox plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should include command button plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should dispatch to toolbox-create-batch");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-item\", \"textbox\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should include textbox dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-item\", \"commandbutton\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should include command dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Batch Dispatch Catalog\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should preserve shared field values");
    expect_contains(catalog_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose dispatch admission state");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1256: admitted toolbox-create-batch-dispatch-catalog JSON should expose non-dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#1256: admitted toolbox-create-batch-dispatch-catalog JSON should expose mutation intent");
    expect_contains(catalog_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1380: toolbox-create-batch-dispatch-catalog JSON should summarize dispatch-ready form items");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1380: admitted toolbox-create-batch-dispatch-catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1380: admitted toolbox-create-batch-dispatch-catalog JSON should summarize empty blocked dispatch errors");
    expect(visual_object_count(form_path) == before_count,
        "#1256: toolbox-create-batch-dispatch-catalog host command should not mutate the visual asset");

    const auto dry_run_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(dry_run_catalog_process.exit_code == 0,
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should return a catalog");
    expect_contains(dry_run_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should preserve batch plans");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchCount\": 0",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should expose zero dispatches");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1380: non-admitted toolbox-create-batch-dispatch-catalog JSON should summarize empty ready item ids");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1380: non-admitted toolbox-create-batch-dispatch-catalog JSON should summarize aggregate blocked state without fabricated item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox batch create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1380: non-admitted toolbox-create-batch-dispatch-catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatch\": null",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should not expose stale dispatch plans");
    expect_contains(dry_run_catalog_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should expose dispatch errors");
    expect_not_contains(dry_run_catalog_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should not expose stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog host command should not mutate the asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "report",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Batch Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1256: report toolbox-create-batch-dispatch-catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxCreateBatchDispatchCatalog\": {",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1256: report toolbox-create-batch-dispatch-catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose one report batch dispatch");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose nested batch plans");
    expect_contains(report_catalog_process.stdout_text, "\"dispatch\": {",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose nested dispatch plans");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1256: report toolbox-create-batch-dispatch-catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#1380: report toolbox-create-batch-dispatch-catalog JSON should summarize dispatch-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1380: report toolbox-create-batch-dispatch-catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1380: report toolbox-create-batch-dispatch-catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2147: report toolbox-create-batch-dispatch-catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Batch Dispatch Catalog\"",
        "#2147: report toolbox-create-batch-dispatch-catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1256: report toolbox-create-batch-dispatch-catalog JSON should preserve report dispatch context");
    expect_contains(report_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Report Batch Dispatch Catalog\"",
        "#2147: report toolbox-create-batch-dispatch-catalog JSON should preserve report dispatch field arguments");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": false",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose non-dry-run dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose mutation intent");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1256: report toolbox-create-batch-dispatch-catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2110: report toolbox-create-batch-dispatch-catalog host command should not mutate assets");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1256: toolbox-create-batch-dispatch-catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1256: missing toolbox-create-batch-dispatch-catalog context JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "form",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1256: toolbox-create-batch-dispatch-catalog JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1256: invalid toolbox-create-batch-dispatch-catalog admission tokens should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "form",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1256: toolbox-create-batch-dispatch-catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1256: malformed toolbox-create-batch-dispatch-catalog field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1256: rejected toolbox-create-batch-dispatch-catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation_batch_dispatch_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_selection_toolbox_create_batch_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Batch Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1299: selection toolbox batch dispatch catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"selectionToolboxCreateBatchDispatchCatalog\": {",
        "#1299: selection toolbox batch dispatch catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1299: selection toolbox batch dispatch catalog JSON should expose selected contexts");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1299: selection toolbox batch dispatch catalog JSON should expose resolved toolbox contexts");
    expect_contains(catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#1299: selection toolbox batch dispatch catalog JSON should expose launch metadata");
    expect_contains(catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#1299: admitted selection toolbox batch dispatch catalog JSON should expose one dispatch");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1299: admitted selection toolbox batch dispatch catalog JSON should expose zero errors");
    expect_contains(catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1299: selection toolbox batch dispatch catalog JSON should expose batch plan state");
    expect_contains(catalog_process.stdout_text, "\"batchPlan\": {",
        "#1299: selection toolbox batch dispatch catalog JSON should expose nested batch plans");
    expect_contains(catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1299: selection toolbox batch dispatch catalog JSON should expose dispatch state");
    expect_contains(catalog_process.stdout_text, "\"dispatch\": {",
        "#1299: selection toolbox batch dispatch catalog JSON should expose nested dispatch plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1299: selection toolbox batch dispatch catalog JSON should include textbox plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1299: selection toolbox batch dispatch catalog JSON should include command button plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1299: selection toolbox batch dispatch catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1299: selection toolbox batch dispatch catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [",
        "#1299: selection toolbox batch dispatch catalog JSON should expose dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1299: selection toolbox batch dispatch catalog JSON should dispatch to toolbox-create-batch");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-item\", \"textbox\"",
        "#1299: selection toolbox batch dispatch catalog JSON should include textbox dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-item\", \"commandbutton\"",
        "#1299: selection toolbox batch dispatch catalog JSON should include command dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Selection Batch Dispatch\"",
        "#1299: selection toolbox batch dispatch catalog JSON should preserve shared field values");
    expect_contains(catalog_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1299: selection toolbox batch dispatch catalog JSON should expose dispatch admission state");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1299: admitted selection toolbox batch dispatch catalog JSON should expose non-dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#1299: admitted selection toolbox batch dispatch catalog JSON should expose mutation intent");
    expect_contains(catalog_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1381: selection toolbox batch dispatch catalog JSON should summarize dispatch-ready visual items");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1381: admitted selection toolbox batch dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1381: admitted selection toolbox batch dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect(visual_object_count(form_path) == before_count,
        "#1299: selection toolbox batch dispatch catalog host command should not mutate the visual asset");

    const auto dry_run_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(dry_run_catalog_process.exit_code == 0,
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should return a catalog");
    expect_contains(dry_run_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should preserve batch plans");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchOk\": false",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should expose dispatch failure");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchCount\": 0",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should expose zero dispatches");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1381: non-admitted selection toolbox batch dispatch catalog JSON should summarize empty ready item ids");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1381: non-admitted selection toolbox batch dispatch catalog JSON should summarize aggregate blocked state without fabricated item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox batch create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1381: non-admitted selection toolbox batch dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatch\": null",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should not expose stale dispatch plans");
    expect_contains(dry_run_catalog_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should expose dispatch errors");
    expect_not_contains(dry_run_catalog_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should not expose stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1299: non-admitted selection toolbox batch dispatch catalog host command should not mutate the asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Batch Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1299: report selection toolbox batch dispatch catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"selectionToolboxCreateBatchDispatchCatalog\": {",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose catalog objects");
    expect_contains(report_catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should expose report selections");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose launch state");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose one report batch dispatch");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose batch plan state");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose nested batch plans");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchOk\": true",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"dispatch\": {",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose nested dispatch plans");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#1381: report selection toolbox batch dispatch catalog JSON should summarize dispatch-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1381: report selection toolbox batch dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1381: report selection toolbox batch dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2114: report selection toolbox batch dispatch catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2153: report selection toolbox batch dispatch catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Selection Batch Dispatch Catalog\"",
        "#2153: report selection toolbox batch dispatch catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should preserve report dispatch context");
    expect_contains(report_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Report Selection Batch Dispatch Catalog\"",
        "#2153: report selection toolbox batch dispatch catalog JSON should preserve report dispatch field arguments");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": false",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose non-dry-run dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose mutation intent");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#1299: report selection toolbox batch dispatch catalog host command should not mutate the asset");

    const auto label_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Batch Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_catalog_process.exit_code == 0,
        "#2088: label selection toolbox batch dispatch catalog JSON command should exit successfully");
    expect_contains(label_catalog_process.stdout_text, "\"selectionToolboxCreateBatchDispatchCatalog\": {",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose catalog objects");
    expect_contains(label_catalog_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should expose label selections");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should expose report contexts");
    expect_contains(label_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose launch state");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose one label batch dispatch");
    expect_contains(label_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose zero catalog errors");
    expect_contains(label_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose batch plan state");
    expect_contains(label_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose nested batch plans");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchOk\": true",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose dispatch state");
    expect_contains(label_catalog_process.stdout_text, "\"dispatch\": {",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose nested dispatch plans");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should include label plans");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should summarize dispatch-ready label items");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2088: label selection toolbox batch dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2088: label selection toolbox batch dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(label_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2128: label selection toolbox batch dispatch catalog JSON should preserve label parent payloads");
    expect_contains(label_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2155: label selection toolbox batch dispatch catalog JSON should expose caller field names");
    expect_contains(label_catalog_process.stdout_text, "\"propertyValue\": \"Label Selection Batch Dispatch Catalog\"",
        "#2155: label selection toolbox batch dispatch catalog JSON should expose caller field values");
    expect_contains(label_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should preserve report dispatch context");
    expect_contains(label_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Label Selection Batch Dispatch Catalog\"",
        "#2155: label selection toolbox batch dispatch catalog JSON should preserve label dispatch field arguments");
    expect_contains(label_catalog_process.stdout_text, "\"dryRun\": false",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose non-dry-run dispatch state");
    expect_contains(label_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose mutation intent");
    expect_not_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2128: label selection toolbox batch dispatch catalog host command should not mutate the asset");

    const auto unsupported_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "menu_item",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unsupported_selection_process.exit_code == 4,
        "#1299: selection toolbox batch dispatch catalog JSON should reject unsupported selections");
    expect_contains(unsupported_selection_process.stdout_text,
        "\"selectionToolboxCreateBatchDispatchCatalog\": null",
        "#1299: unsupported selection toolbox batch dispatch catalog JSON should suppress stale payloads");
    expect_contains(unsupported_selection_process.stdout_text,
        "A selection-context toolbox object batch creation dispatch catalog request requires a toolbox palette.",
        "#1299: unsupported selection toolbox batch dispatch catalog JSON should report palette errors");
    expect(visual_object_count(form_path) == before_count,
        "#1299: unsupported selection toolbox batch dispatch catalog host command should not mutate the asset");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1299: missing path selection toolbox batch dispatch catalog JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1299: missing selection toolbox batch dispatch catalog JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1299: unknown selection toolbox batch dispatch catalog JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1299: invalid selection toolbox batch dispatch catalog admission tokens should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1299: malformed selection toolbox batch dispatch catalog field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch-dispatch-catalog option: --toolbox-context",
        "#1299: unknown selection toolbox batch dispatch catalog options should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1299: rejected selection toolbox batch dispatch catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto batch_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--toolbox-context", "form",
            "--toolbox-item", "textbox",
            "--unique-id", "first-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First",
            "--toolbox-item", "textbox",
            "--unique-id", "second-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Second",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdRun",
            "--unique-id", "command-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Run",
            "--json"
        },
        temp_root);
    expect(batch_plan_process.exit_code == 0,
        "#1246: toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxCreateBatchPlan\": {",
        "#1246: toolbox-create-batch-plan JSON should expose a batch plan object");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1246: toolbox-create-batch-plan JSON should expose requested context state");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1246: toolbox-create-batch-plan JSON should expose requested contexts");
    expect_contains(batch_plan_process.stdout_text, "\"itemCount\": 3",
        "#1246: toolbox-create-batch-plan JSON should expose batch item counts");
    expect_contains(batch_plan_process.stdout_text,
        "\"planReadyItemIds\": [\"textbox\", \"textbox\", \"commandbutton\"]",
        "#1404: toolbox-create-batch-plan JSON should summarize plan-ready item ids");
    expect_contains(batch_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1404: toolbox-create-batch-plan JSON should expose empty blocked item ids");
    expect_contains(batch_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#1404: toolbox-create-batch-plan JSON should expose empty blocked plan errors");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1246: toolbox-create-batch-plan JSON should expose textbox descriptors");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1246: toolbox-create-batch-plan JSON should expose command button descriptors");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1246: toolbox-create-batch-plan JSON should expose first append target indexes");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1246: toolbox-create-batch-plan JSON should expose later append target indexes");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1246: toolbox-create-batch-plan JSON should expose first generated names");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1246: toolbox-create-batch-plan JSON should reserve generated names across the batch");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"cmdRun\"",
        "#1246: toolbox-create-batch-plan JSON should preserve explicit names");
    expect_contains(batch_plan_process.stdout_text, "\"uniqueId\": \"first-textbox-guid\"",
        "#1246: toolbox-create-batch-plan JSON should expose per-item unique ids");
    expect_contains(batch_plan_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1246: toolbox-create-batch-plan JSON should expose per-item parents");
    expect_contains(batch_plan_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1246: toolbox-create-batch-plan JSON should expose per-item field values");
    expect_contains(batch_plan_process.stdout_text, "\"dryRun\": true",
        "#1246: toolbox-create-batch-plan JSON should expose dry-run state");
    expect_contains(batch_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1246: toolbox-create-batch-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1246: toolbox-create-batch-plan host command should not mutate the visual asset");

    const auto report_batch_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--toolbox-context", "report",
            "--toolbox-item", "label",
            "--unique-id", "direct-report-batch-plan-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Direct Report Plan",
            "--json"
        },
        temp_root);
    expect(report_batch_plan_process.exit_code == 0,
        "#2101: report toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxCreateBatchPlan\": {",
        "#2101: report toolbox-create-batch-plan JSON should expose batch plans");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2101: report toolbox-create-batch-plan JSON should preserve report contexts");
    expect_contains(report_batch_plan_process.stdout_text, "\"itemCount\": 1",
        "#2101: report toolbox-create-batch-plan JSON should expose report item counts");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2101: report toolbox-create-batch-plan JSON should expose label plans");
    expect_contains(report_batch_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2101: report toolbox-create-batch-plan JSON should expose generated label names");
    expect_contains(report_batch_plan_process.stdout_text, "\"uniqueId\": \"direct-report-batch-plan-guid\"",
        "#2101: report toolbox-create-batch-plan JSON should preserve label unique ids");
    expect_contains(report_batch_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2101: report toolbox-create-batch-plan JSON should preserve label parent overrides");
    expect_contains(report_batch_plan_process.stdout_text, "\"propertyValue\": \"Direct Report Plan\"",
        "#2101: report toolbox-create-batch-plan JSON should preserve label field values");
    expect_contains(report_batch_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2101: report toolbox-create-batch-plan JSON should summarize plan-ready report item ids");
    expect_contains(report_batch_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2101: report toolbox-create-batch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_batch_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2101: report toolbox-create-batch-plan JSON should summarize empty plan errors");
    expect_contains(report_batch_plan_process.stdout_text, "\"dryRun\": true",
        "#2101: report toolbox-create-batch-plan JSON should remain a dry-run plan");
    expect_contains(report_batch_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2101: report toolbox-create-batch-plan JSON should remain non-mutating");
    expect_not_contains(report_batch_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2101: report toolbox-create-batch-plan JSON should exclude form-only TextBox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2101: report toolbox-create-batch-plan host command should not mutate assets");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1246: toolbox-create-batch-plan JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1246: empty toolbox-create-batch-plan item lists should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1246: toolbox-create-batch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1246: orphan toolbox-create-batch-plan item options should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1246: toolbox-create-batch-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1246: malformed toolbox-create-batch-plan field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1246: rejected toolbox-create-batch-plan host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_batch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto batch_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-first-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First Selection",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdSelection",
            "--unique-id", "selection-command-guid",
            "--parent-name", "cntToolbar",
            "--field-value", "CAPTION=Run Selection",
            "--toolbox-item", "textbox",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Second Selection",
            "--json"
        },
        temp_root);
    expect(batch_plan_process.exit_code == 0,
        "#1305: selection-toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(batch_plan_process.stdout_text, "\"selectionToolboxCreateBatchPlan\": {",
        "#1305: selection-toolbox-create-batch-plan JSON should expose a stable result object");
    expect_contains(batch_plan_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose selected Studio contexts");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose resolved toolbox contexts");
    expect_contains(batch_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#1305: selection-toolbox-create-batch-plan JSON should expose launch state");
    expect_contains(batch_plan_process.stdout_text, "\"itemCount\": 3",
        "#1305: selection-toolbox-create-batch-plan JSON should expose item counts");
    expect_contains(batch_plan_process.stdout_text, "\"planCount\": 1",
        "#1305: selection-toolbox-create-batch-plan JSON should expose plan counts");
    expect_contains(batch_plan_process.stdout_text, "\"errorCount\": 0",
        "#1305: selection-toolbox-create-batch-plan JSON should expose zero errors");
    expect_contains(batch_plan_process.stdout_text,
        "\"planReadyItemIds\": [\"textbox\", \"commandbutton\", \"textbox\"]",
        "#1404: selection-toolbox-create-batch-plan JSON should summarize plan-ready item ids");
    expect_contains(batch_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1404: selection-toolbox-create-batch-plan JSON should expose empty blocked item ids");
    expect_contains(batch_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#1404: selection-toolbox-create-batch-plan JSON should expose empty blocked plan errors");
    expect_contains(batch_plan_process.stdout_text, "\"batchPlanOk\": true",
        "#1305: selection-toolbox-create-batch-plan JSON should expose nested batch-plan state");
    expect_contains(batch_plan_process.stdout_text, "\"batchPlan\": {",
        "#1305: selection-toolbox-create-batch-plan JSON should expose nested batch plans");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose textbox descriptors");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose command button descriptors");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1305: selection-toolbox-create-batch-plan JSON should expose first target records");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1305: selection-toolbox-create-batch-plan JSON should expose later target records");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose first generated names");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1305: selection-toolbox-create-batch-plan JSON should reserve generated names across the batch");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"cmdSelection\"",
        "#1305: selection-toolbox-create-batch-plan JSON should preserve explicit object names");
    expect_contains(batch_plan_process.stdout_text, "\"uniqueId\": \"selection-first-textbox-guid\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose per-item unique ids");
    expect_contains(batch_plan_process.stdout_text, "\"parentName\": \"cntToolbar\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose per-item parent names");
    expect_contains(batch_plan_process.stdout_text, "\"propertyValue\": \"Second Selection\"",
        "#1305: selection-toolbox-create-batch-plan JSON should preserve per-item field values");
    expect_contains(batch_plan_process.stdout_text, "\"dryRun\": true",
        "#1305: selection-toolbox-create-batch-plan JSON should expose dry-run state");
    expect_contains(batch_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1305: selection-toolbox-create-batch-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1305: selection-toolbox-create-batch-plan host command should not mutate assets");

    const auto report_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "report_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-report-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection",
            "--json"
        },
        temp_root);
    expect(report_plan_process.exit_code == 0,
        "#1305: report selection-toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(report_plan_process.stdout_text, "\"selectionToolboxCreateBatchPlan\": {",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose a stable result object");
    expect_contains(report_plan_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should expose report selections");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should resolve report contexts");
    expect_contains(report_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose launch state");
    expect_contains(report_plan_process.stdout_text, "\"itemCount\": 1",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose report item counts");
    expect_contains(report_plan_process.stdout_text, "\"planCount\": 1",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose plan counts");
    expect_contains(report_plan_process.stdout_text, "\"errorCount\": 0",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose zero errors");
    expect_contains(report_plan_process.stdout_text, "\"batchPlanOk\": true",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose batch-plan state");
    expect_contains(report_plan_process.stdout_text, "\"batchPlan\": {",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose nested batch plans");
    expect_contains(report_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should expose label plans");
    expect_contains(report_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should expose generated label names");
    expect_contains(report_plan_process.stdout_text, "\"uniqueId\": \"selection-report-label-guid\"",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose caller unique ids");
    expect_contains(report_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2120: report selection-toolbox-create-batch-plan JSON should preserve report parent payloads");
    expect_contains(report_plan_process.stdout_text, "\"propertyValue\": \"Report Selection\"",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose caller report fields");
    expect_contains(report_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2120: report selection-toolbox-create-batch-plan JSON should summarize plan-ready report item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose empty blocked item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose empty blocked plan errors");
    expect_contains(report_plan_process.stdout_text, "\"dryRun\": true",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose dry-run state");
    expect_contains(report_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2120: report selection-toolbox-create-batch-plan JSON should remain non-mutating");
    expect_not_contains(report_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2120: report selection-toolbox-create-batch-plan host command should not mutate assets");

    const auto label_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "label_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-label-batch-plan-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection",
            "--json"
        },
        temp_root);
    expect(label_plan_process.exit_code == 0,
        "#2087: label selection-toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(label_plan_process.stdout_text, "\"selectionToolboxCreateBatchPlan\": {",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose a stable result object");
    expect_contains(label_plan_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should expose label selections");
    expect_contains(label_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should resolve report contexts");
    expect_contains(label_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose launch state");
    expect_contains(label_plan_process.stdout_text, "\"itemCount\": 1",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose label item counts");
    expect_contains(label_plan_process.stdout_text, "\"planCount\": 1",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose plan counts");
    expect_contains(label_plan_process.stdout_text, "\"errorCount\": 0",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose zero errors");
    expect_contains(label_plan_process.stdout_text, "\"batchPlanOk\": true",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose batch-plan state");
    expect_contains(label_plan_process.stdout_text, "\"batchPlan\": {",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose nested batch plans");
    expect_contains(label_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should expose label plans");
    expect_contains(label_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should expose generated label names");
    expect_contains(label_plan_process.stdout_text, "\"uniqueId\": \"selection-label-batch-plan-guid\"",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose caller unique ids");
    expect_contains(label_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2122: label selection-toolbox-create-batch-plan JSON should preserve label parent payloads");
    expect_contains(label_plan_process.stdout_text, "\"propertyValue\": \"Label Selection\"",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose caller label fields");
    expect_contains(label_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2122: label selection-toolbox-create-batch-plan JSON should summarize plan-ready label item ids");
    expect_contains(label_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose empty blocked item ids");
    expect_contains(label_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose empty blocked plan errors");
    expect_contains(label_plan_process.stdout_text, "\"dryRun\": true",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose dry-run state");
    expect_contains(label_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2122: label selection-toolbox-create-batch-plan JSON should remain non-mutating");
    expect_not_contains(label_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2087: label selection-toolbox-create-batch-plan host command should not mutate assets");

    const auto unavailable_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "report_expression",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(unavailable_plan_process.exit_code == 4,
        "#1305: selection-toolbox-create-batch-plan JSON should reject unavailable selected-context items");
    expect_contains(unavailable_plan_process.stdout_text, "\"batchPlanOk\": false",
        "#1305: unavailable selection-toolbox-create-batch-plan JSON should expose failed batch-plan state");
    expect_contains(unavailable_plan_process.stdout_text, "\"planReadyItemIds\": []",
        "#1404: unavailable selection-toolbox-create-batch-plan JSON should expose empty ready item ids");
    expect_contains(unavailable_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1404: unavailable selection-toolbox-create-batch-plan JSON should not fabricate blocked item ids");
    expect_contains(unavailable_plan_process.stdout_text,
        "\"planBlockedErrors\": [\"The requested toolbox item is not available in the requested designer context.\"]",
        "#1404: unavailable selection-toolbox-create-batch-plan JSON should summarize blocked plan errors");
    expect_contains(unavailable_plan_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1305: unavailable selection-toolbox-create-batch-plan JSON should report planner errors");
    expect_not_contains(unavailable_plan_process.stdout_text, "\"plans\": [",
        "#1305: unavailable selection-toolbox-create-batch-plan JSON should omit stale plans");

    const auto unsupported_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "menu_item",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(unsupported_plan_process.exit_code == 4,
        "#1305: selection-toolbox-create-batch-plan JSON should reject unsupported selections");
    expect_contains(unsupported_plan_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1305: unsupported selection-toolbox-create-batch-plan JSON should preserve selected contexts");
    expect_contains(unsupported_plan_process.stdout_text,
        "A selection-context toolbox object batch creation plan request requires a toolbox palette.",
        "#1305: unsupported selection-toolbox-create-batch-plan JSON should report palette errors");
    expect_not_contains(unsupported_plan_process.stdout_text, "\"plans\": [",
        "#1305: unsupported selection-toolbox-create-batch-plan JSON should omit stale plans");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1305: missing selection selection-toolbox-create-batch-plan JSON should report parser errors");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1305: empty selection-toolbox-create-batch-plan item lists should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Selection toolbox batch item options require a preceding --toolbox-item.",
        "#1305: orphan selection-toolbox-create-batch-plan item options should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1305: malformed selection-toolbox-create-batch-plan field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch-plan option: --toolbox-context",
        "#1305: unknown option selection-toolbox-create-batch-plan JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1305: rejected selection-toolbox-create-batch-plan host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_batch_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_dispatch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-context", "form",
            "--toolbox-item", "textbox",
            "--unique-id", "first-dispatch-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First Dispatch",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdDispatch",
            "--unique-id", "dispatch-command-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Dispatch Command",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(dispatch_process.exit_code == 0,
        "#1252: toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose a dispatch result object");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose requested context state");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose requested contexts");
    expect_contains(dispatch_process.stdout_text, "\"itemCount\": 2",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose item counts");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose first toolbox item metadata");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose later toolbox item metadata");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose first target indexes");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 3",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose later target indexes");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose generated names");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"cmdDispatch\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should preserve explicit names");
    expect_contains(dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should dispatch to toolbox-create-batch");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-item\", \"textbox\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include first toolbox item arguments");
    expect_contains(dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include generated object-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=First Dispatch\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include first field-value arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-item\", \"commandbutton\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include later toolbox item arguments");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Command\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include later field-value arguments");
    expect_contains(dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"textbox\", \"commandbutton\"]",
        "#1387: toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1387: toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1387: successful toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(dispatch_process.stdout_text, "\"dryRun\": false",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"executed\": false",
        "#1252: toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_contains(dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1252: toolbox-create-batch-dispatch-plan host command should not mutate the visual asset");

    const auto report_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-context", "report",
            "--toolbox-item", "label",
            "--unique-id", "direct-report-batch-dispatch-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Direct Report Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_dispatch_process.exit_code == 0,
        "#2100: report toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose dispatch plans");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve report contexts");
    expect_contains(report_dispatch_process.stdout_text, "\"itemCount\": 1",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose report item counts");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose label plans");
    expect_contains(report_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose generated label names");
    expect_contains(report_dispatch_process.stdout_text, "\"uniqueId\": \"direct-report-batch-dispatch-guid\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label unique ids");
    expect_contains(report_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label parent overrides");
    expect_contains(report_dispatch_process.stdout_text, "\"propertyValue\": \"Direct Report Dispatch\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label field values");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should dispatch to toolbox-create-batch");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve report context arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-item\", \"label\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label item arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve generated label arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--unique-id\", \"direct-report-batch-dispatch-guid\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Direct Report Dispatch\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label field arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready report item ids");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(report_dispatch_process.stdout_text, "\"executed\": false",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_not_contains(report_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should exclude form-only TextBox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2100: report toolbox-create-batch-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "false",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1252: toolbox-create-batch-dispatch-plan JSON should reject non-admitted create operations");
    expect_contains(non_admitted_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": null",
        "#1252: non-admitted toolbox-create-batch-dispatch-plan JSON should not expose stale dispatch plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1252: non-admitted toolbox-create-batch-dispatch-plan JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1387: non-admitted toolbox-create-batch-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1387: non-admitted toolbox-create-batch-dispatch-plan JSON should summarize aggregate blocked state");
    expect_contains(non_admitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox batch create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1387: non-admitted toolbox-create-batch-dispatch-plan JSON should summarize dispatch errors");
    expect_not_contains(non_admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1252: failed toolbox-create-batch-dispatch-plan JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1252: non-admitted toolbox-create-batch-dispatch-plan commands should not mutate the asset");

    const auto invalid_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-context", "report",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(invalid_plan_process.exit_code == 4,
        "#1252: toolbox-create-batch-dispatch-plan JSON should reject invalid batch plans");
    expect_contains(invalid_plan_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1252: invalid toolbox-create-batch-dispatch-plan batch plans should report planning errors");
    expect_contains(invalid_plan_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1387: invalid toolbox-create-batch-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(invalid_plan_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1387: invalid toolbox-create-batch-dispatch-plan JSON should summarize planning errors");
    expect_not_contains(invalid_plan_process.stdout_text, "\"dispatchArguments\": [",
        "#1252: invalid toolbox-create-batch-dispatch-plan batch plans should not expose stale arguments");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1252: toolbox-create-batch-dispatch-plan JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1252: invalid toolbox-create-batch-dispatch-plan admission tokens should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1252: toolbox-create-batch-dispatch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1252: orphan toolbox-create-batch-dispatch-plan item options should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation_batch_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_batch_dispatch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-dispatch-first-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First Selection Dispatch",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdSelectionDispatch",
            "--unique-id", "selection-dispatch-command-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Run Selection Dispatch",
            "--toolbox-item", "textbox",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Second Selection Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(dispatch_process.exit_code == 0,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(dispatch_process.stdout_text, "\"selectionToolboxCreateBatchDispatchPlan\": {",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose a stable result object");
    expect_contains(dispatch_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose selected contexts");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose resolved toolbox contexts");
    expect_contains(dispatch_process.stdout_text, "\"launchPlanOk\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose launch state");
    expect_contains(dispatch_process.stdout_text, "\"itemCount\": 3",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose item counts");
    expect_contains(dispatch_process.stdout_text, "\"dispatchCount\": 1",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch counts");
    expect_contains(dispatch_process.stdout_text, "\"errorCount\": 0",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose zero errors");
    expect_contains(dispatch_process.stdout_text, "\"batchPlanOk\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose nested batch-plan state");
    expect_contains(dispatch_process.stdout_text, "\"dispatchOk\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose nested dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"batchPlan\": {",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose nested batch plans");
    expect_contains(dispatch_process.stdout_text, "\"dispatch\": {",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose nested dispatch plans");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose textbox descriptors");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose command button descriptors");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose first target records");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose later target records");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose first generated names");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reserve generated names across the batch");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"cmdSelectionDispatch\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should preserve explicit object names");
    expect_contains(dispatch_process.stdout_text, "\"uniqueId\": \"selection-dispatch-first-textbox-guid\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose per-item unique ids");
    expect_contains(dispatch_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose per-item parent names");
    expect_contains(dispatch_process.stdout_text, "\"propertyValue\": \"Second Selection Dispatch\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should preserve per-item field values");
    expect_contains(dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should dispatch to batch creation");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-context\", \"form\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should include resolved contexts");
    expect_contains(dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should include generated object-name args");
    expect_contains(dispatch_process.stdout_text, "\"--unique-id\", \"selection-dispatch-first-textbox-guid\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should include unique id args");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Run Selection Dispatch\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should include field-value args");
    expect_contains(dispatch_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"textbox\", \"commandbutton\", \"textbox\"]",
        "#1387: selection-toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1387: selection-toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1387: successful selection-toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose admission state");
    expect_contains(dispatch_process.stdout_text, "\"dryRun\": false",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"executed\": false",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_contains(dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1307: selection-toolbox-create-batch-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "false",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject non-admitted operations");
    expect_contains(non_admitted_process.stdout_text, "\"batchPlanOk\": true",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should keep batch-plan evidence");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchOk\": false",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch failures");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1387: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1387: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should summarize aggregate blocked state");
    expect_contains(non_admitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox batch create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1387: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should summarize dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatch\": null",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should omit stale dispatch plans");
    expect_not_contains(non_admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should omit stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan commands should not mutate assets");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "report_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-report-dispatch-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"selectionToolboxCreateBatchDispatchPlan\": {",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"launchPlanOk\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose launch state");
    expect_contains(report_process.stdout_text, "\"itemCount\": 1",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose report item counts");
    expect_contains(report_process.stdout_text, "\"dispatchCount\": 1",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch counts");
    expect_contains(report_process.stdout_text, "\"errorCount\": 0",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose zero errors");
    expect_contains(report_process.stdout_text, "\"batchPlanOk\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose batch-plan state");
    expect_contains(report_process.stdout_text, "\"dispatchOk\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch state");
    expect_contains(report_process.stdout_text, "\"batchPlan\": {",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose nested batch plans");
    expect_contains(report_process.stdout_text, "\"dispatch\": {",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose nested dispatch plans");
    expect_contains(report_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should expose label plans");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should expose generated label names");
    expect_contains(report_process.stdout_text, "\"uniqueId\": \"selection-report-dispatch-label-guid\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose caller unique ids");
    expect_contains(report_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should preserve report parent payloads");
    expect_contains(report_process.stdout_text, "\"propertyValue\": \"Report Selection Dispatch\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose caller report fields");
    expect_contains(report_process.stdout_text, "\"dispatchArguments\": [",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should dispatch to batch creation");
    expect_contains(report_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should dispatch resolved report contexts");
    expect_contains(report_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should preserve report parent arguments");
    expect_contains(report_process.stdout_text, "\"--field-value\", \"CAPTION=Report Selection Dispatch\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should preserve report field arguments");
    expect_contains(report_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#1387: report selection-toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready report item ids");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1387: report selection-toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(report_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose admission state");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(report_process.stdout_text, "\"executed\": false",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should exclude form-only textboxes");
    expect(visual_object_count(form_path) == before_count,
        "#2118: report selection-toolbox-create-batch-dispatch-plan host command should not mutate assets");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "label_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-batch-dispatch-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"selectionToolboxCreateBatchDispatchPlan\": {",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should expose label selections");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"launchPlanOk\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose launch state");
    expect_contains(label_process.stdout_text, "\"itemCount\": 1",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose label item counts");
    expect_contains(label_process.stdout_text, "\"dispatchCount\": 1",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch counts");
    expect_contains(label_process.stdout_text, "\"errorCount\": 0",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose zero errors");
    expect_contains(label_process.stdout_text, "\"batchPlanOk\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose batch-plan state");
    expect_contains(label_process.stdout_text, "\"dispatchOk\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch state");
    expect_contains(label_process.stdout_text, "\"batchPlan\": {",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose nested batch plans");
    expect_contains(label_process.stdout_text, "\"dispatch\": {",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose nested dispatch plans");
    expect_contains(label_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should expose label plans");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should expose generated label names");
    expect_contains(label_process.stdout_text, "\"uniqueId\": \"selection-batch-dispatch-guid\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose caller unique ids");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should preserve label parent payloads");
    expect_contains(label_process.stdout_text, "\"propertyValue\": \"Label Selection Dispatch\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose caller label fields");
    expect_contains(label_process.stdout_text, "\"dispatchArguments\": [",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(label_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should dispatch to batch creation");
    expect_contains(label_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should dispatch resolved report contexts");
    expect_contains(label_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(label_process.stdout_text, "\"--field-value\", \"CAPTION=Label Selection Dispatch\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should preserve label field arguments");
    expect_contains(label_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready label item ids");
    expect_contains(label_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(label_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(label_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose admission state");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(label_process.stdout_text, "\"executed\": false",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should exclude form-only textboxes");
    expect(visual_object_count(form_path) == before_count,
        "#2124: label selection-toolbox-create-batch-dispatch-plan host command should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "report_expression",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject unavailable selected-context items");
    expect_contains(unavailable_process.stdout_text, "\"batchPlanOk\": false",
        "#1307: unavailable selection-toolbox-create-batch-dispatch-plan JSON should expose failed batch plans");
    expect_contains(unavailable_process.stdout_text, "\"dispatchOk\": false",
        "#1307: unavailable selection-toolbox-create-batch-dispatch-plan JSON should expose no dispatch");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1307: unavailable selection-toolbox-create-batch-dispatch-plan JSON should report planner errors");
    expect_contains(unavailable_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1387: unavailable selection-toolbox-create-batch-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(unavailable_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1387: unavailable selection-toolbox-create-batch-dispatch-plan JSON should summarize planner errors");
    expect_not_contains(unavailable_process.stdout_text, "\"dispatchArguments\": [",
        "#1307: unavailable selection-toolbox-create-batch-dispatch-plan JSON should omit stale dispatch args");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "menu_item",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1307: unsupported selection-toolbox-create-batch-dispatch-plan JSON should preserve selections");
    expect_contains(unsupported_process.stdout_text, "\"launchPlanOk\": false",
        "#1307: unsupported selection-toolbox-create-batch-dispatch-plan JSON should expose launch failures");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object batch creation plan request requires a toolbox palette.",
        "#1307: unsupported selection-toolbox-create-batch-dispatch-plan JSON should report palette errors");
    expect_not_contains(unsupported_process.stdout_text, "\"dispatchArguments\": [",
        "#1307: unsupported selection-toolbox-create-batch-dispatch-plan JSON should omit stale dispatch args");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1307: missing path selection-toolbox-create-batch-dispatch-plan JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1307: missing selection selection-toolbox-create-batch-dispatch-plan JSON should report parser errors");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1307: empty selection-toolbox-create-batch-dispatch-plan item lists should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Selection toolbox batch dispatch item options require a preceding --toolbox-item.",
        "#1307: orphan selection-toolbox-create-batch-dispatch-plan item options should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1307: malformed selection-toolbox-create-batch-dispatch-plan field values should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject invalid admission booleans");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1307: invalid selection-toolbox-create-batch-dispatch-plan admission values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch-dispatch-plan option: --toolbox-context",
        "#1307: unknown option selection-toolbox-create-batch-dispatch-plan JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1307: rejected selection-toolbox-create-batch-dispatch-plan host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
