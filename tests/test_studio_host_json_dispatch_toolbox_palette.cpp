// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_toolbox_palette_launch_plans(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_toolbox_palette_launch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-plan",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1210: toolbox palette launch-plan JSON should accept visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxPaletteLaunchPlan\": {",
        "#1210: toolbox palette launch-plan JSON should expose a plan object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1210: toolbox palette launch-plan JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1210: visual-object toolbox palette launch-plan JSON should resolve form toolbox contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1210: toolbox palette launch-plan JSON should carry asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1210: toolbox palette launch-plan JSON should carry record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1210: toolbox palette launch-plan JSON should carry object-name selectors");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1210: toolbox palette launch-plan JSON should carry unique-id selectors");
    expect_contains(visual_process.stdout_text, "\"itemCount\": ",
        "#1210: toolbox palette launch-plan JSON should expose item counts");
    expect_contains(visual_process.stdout_text, "\"launchReadySelectionContexts\": [\"visual_object\"]",
        "#1403: toolbox palette launch-plan JSON should summarize launch-ready selected contexts");
    expect_contains(visual_process.stdout_text, "\"launchBlockedSelectionContexts\": []",
        "#1403: toolbox palette launch-plan JSON should expose empty blocked selected contexts");
    expect_contains(visual_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1403: toolbox palette launch-plan JSON should expose empty blocked launch errors");
    expect_contains(visual_process.stdout_text, "\"id\": \"textbox\"",
        "#1210: visual-object toolbox palette launch-plan JSON should include form-safe TextBox items");
    expect_contains(visual_process.stdout_text, "\"id\": \"pageframe\"",
        "#1210: visual-object toolbox palette launch-plan JSON should include form-safe PageFrame items");

    const auto container_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-plan",
            "--selection-context", "container_object",
            "--path", "forms/customer.scx",
            "--record", "2",
            "--object-name", "pgAddress",
            "--unique-id", "page-guid",
            "--json"
        },
        temp_root);
    expect(container_process.exit_code == 0,
        "#1210: toolbox palette launch-plan JSON should accept container contexts");
    expect_contains(container_process.stdout_text, "\"toolboxContext\": \"container\"",
        "#1210: container toolbox palette launch-plan JSON should resolve container toolbox contexts");
    expect_contains(container_process.stdout_text, "\"id\": \"checkbox\"",
        "#1210: container toolbox palette launch-plan JSON should include container-safe CheckBox items");
    expect_contains(container_process.stdout_text, "\"id\": \"grid\"",
        "#1210: container toolbox palette launch-plan JSON should include container-safe Grid items");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-plan",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1210: toolbox palette launch-plan JSON should accept report contexts");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1210: report toolbox palette launch-plan JSON should resolve report toolbox contexts");
    expect_contains(report_process.stdout_text, "\"id\": \"label\"",
        "#1210: report toolbox palette launch-plan JSON should include report-safe Label items");
    expect_not_contains(report_process.stdout_text, "\"id\": \"textbox\"",
        "#1210: report toolbox palette launch-plan JSON should exclude form-only TextBox items");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-plan",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1210: toolbox palette launch-plan JSON should reject unsupported selection contexts");
    expect_contains(unsupported_process.stdout_text, "\"toolboxPaletteLaunchPlan\": null",
        "#1210: unsupported toolbox palette launch-plan JSON should not expose a plan object");
    expect_contains(unsupported_process.stdout_text,
        "The selected Studio context does not expose a toolbox palette.",
        "#1210: unsupported toolbox palette launch-plan JSON should report validation errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-plan",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1210: toolbox palette launch-plan JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1210: unknown toolbox palette context JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-plan",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1210: toolbox palette launch-plan JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1210: missing toolbox palette context JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-plan",
            "--selection-context", "visual_object",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1210: toolbox palette launch-plan JSON should reject invalid record indexes");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1210: invalid toolbox palette record JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_toolbox_palette_launch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_palette_launch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-catalog",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "cmdSave",
            "--unique-id", "button-guid",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1317: toolbox palette launch catalog JSON should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"toolboxPaletteLaunchCatalog\": {",
        "#1317: toolbox palette launch catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"contextCount\": 9",
        "#1317: toolbox palette launch catalog JSON should expose context counts");
    expect_contains(catalog_process.stdout_text, "\"launchPlanCount\": 6",
        "#1317: toolbox palette launch catalog JSON should expose launch-plan counts");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 3",
        "#1317: toolbox palette launch catalog JSON should expose unsupported-context counts");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": true",
        "#1317: toolbox palette launch catalog JSON should remain dry-run");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1317: toolbox palette launch catalog JSON should remain non-mutating");
    expect_contains(catalog_process.stdout_text, "\"launchReadySelectionContexts\": [\"visual_object\"",
        "#1368: toolbox palette launch catalog JSON should summarize launch-ready contexts");
    expect_contains(catalog_process.stdout_text, "\"launchBlockedSelectionContexts\": [\"menu_item\"",
        "#1368: toolbox palette launch catalog JSON should summarize launch-blocked contexts");
    expect_contains(catalog_process.stdout_text,
        "\"launchBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"",
        "#1368: toolbox palette launch catalog JSON should summarize launch-blocked errors");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1317: toolbox palette launch catalog JSON should include visual-object entries");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1317: visual-object toolbox palette launch catalog JSON should resolve form palettes");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"container_object\"",
        "#1317: toolbox palette launch catalog JSON should include container entries");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"container\"",
        "#1317: container toolbox palette launch catalog JSON should resolve container palettes");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1317: toolbox palette launch catalog JSON should include report entries");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1317: report toolbox palette launch catalog JSON should resolve report palettes");
    expect_contains(catalog_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1317: toolbox palette launch catalog JSON should preserve asset paths");
    expect_contains(catalog_process.stdout_text, "\"recordIndex\": 4",
        "#1317: toolbox palette launch catalog JSON should preserve record indexes");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1317: toolbox palette launch catalog JSON should preserve object names");
    expect_contains(catalog_process.stdout_text, "\"uniqueId\": \"button-guid\"",
        "#1317: toolbox palette launch catalog JSON should preserve unique ids");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1317: toolbox palette launch catalog JSON should include unsupported menu entries");
    expect_contains(catalog_process.stdout_text, "\"toolboxAvailable\": false",
        "#1317: unsupported toolbox palette launch catalog JSON entries should report unavailable state");
    expect_contains(catalog_process.stdout_text,
        "The selected Studio context does not expose a toolbox palette.",
        "#1317: unsupported toolbox palette launch catalog JSON entries should report deterministic errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-catalog",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1317: toolbox palette launch catalog JSON should reject invalid record indexes");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1317: invalid toolbox palette launch catalog record JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-launch-catalog",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1317: toolbox palette launch catalog JSON should reject selection-context options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown toolbox-palette-launch-catalog option: --selection-context",
        "#1317: unknown toolbox palette launch catalog options should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_toolbox_palette_query_filters(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_palette_query_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto filtered_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-query",
            "--toolbox-context", "form",
            "--toolbox-search", "textbox",
            "--toolbox-category", "Standard Controls",
            "--json"
        },
        temp_root);
    expect(filtered_process.exit_code == 0,
        "#1414: toolbox palette query JSON should exit successfully for matching filters");
    expect_contains(filtered_process.stdout_text, "\"toolboxPaletteQuery\": {",
        "#1414: toolbox palette query JSON should expose a query object");
    expect_contains(filtered_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1414: toolbox palette query JSON should expose toolbox contexts");
    expect_contains(filtered_process.stdout_text, "\"searchText\": \"textbox\"",
        "#1414: toolbox palette query JSON should expose search filters");
    expect_contains(filtered_process.stdout_text, "\"category\": \"Standard Controls\"",
        "#1414: toolbox palette query JSON should expose category filters");
    expect_contains(filtered_process.stdout_text, "\"itemCount\": 1",
        "#1414: toolbox palette query JSON should expose filtered item counts");
    expect_contains(filtered_process.stdout_text, "\"dryRun\": true",
        "#1414: toolbox palette query JSON should remain dry-run");
    expect_contains(filtered_process.stdout_text, "\"mutatesAsset\": false",
        "#1414: toolbox palette query JSON should remain non-mutating");
    expect_contains(filtered_process.stdout_text, "\"id\": \"textbox\"",
        "#1414: toolbox palette query JSON should include matching descriptors");
    expect_not_contains(filtered_process.stdout_text, "\"id\": \"pageframe\"",
        "#1414: toolbox palette query JSON should exclude non-matching descriptors");

    const auto empty_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-query",
            "--toolbox-context", "report",
            "--toolbox-search", "textbox",
            "--json"
        },
        temp_root);
    expect(empty_process.exit_code == 0,
        "#1414: toolbox palette query JSON should succeed for empty results");
    expect_contains(empty_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1414: empty toolbox palette query JSON should preserve requested context");
    expect_contains(empty_process.stdout_text, "\"itemCount\": 0",
        "#1414: empty toolbox palette query JSON should report zero matches");
    expect_contains(empty_process.stdout_text, "\"items\": [\n    ]",
        "#1414: empty toolbox palette query JSON should expose an empty item list");

    const auto invalid_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-palette-query",
            "--toolbox-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(invalid_context_process.exit_code == 2,
        "#1414: toolbox palette query JSON should reject invalid toolbox contexts");
    expect_contains(invalid_context_process.stdout_text, "\"toolboxPaletteQuery\": null",
        "#1414: invalid toolbox palette query JSON should not expose a query object");
    expect_contains(invalid_context_process.stdout_text, "Unknown toolbox context token: menu_item",
        "#1414: invalid toolbox palette query JSON should report parser errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--toolbox-palette-query --toolbox-context <token>",
        "#1414: usage text should expose toolbox palette query commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_from_palette_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_from_dispatch_plan_json_tests";
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
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--record", "0",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--create-unique-id", "dispatch-textbox-guid",
            "--field-value", "CAPTION=Dispatch Plan",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(plan_process.exit_code == 0,
        "#1261: toolbox-create-from-dispatch-plan JSON command should exit successfully");
    expect_contains(plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose create plans");
    expect_contains(plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose selected toolbox items");
    expect_contains(plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose descriptor class names");
    expect_contains(plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1261: toolbox-create-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should resolve visual-object form contexts");
    expect_contains(plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose planned target indexes");
    expect_contains(plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose generated object names");
    expect_contains(plan_process.stdout_text, "\"uniqueId\": \"dispatch-textbox-guid\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose create unique-id overrides");
    expect_contains(plan_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should default parents from selected objects");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose caller field values");
    expect_contains(plan_process.stdout_text, "\"propertyValue\": \"Dispatch Plan\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose caller field-value payloads");
    expect_contains(plan_process.stdout_text, "\"dryRun\": true",
        "#1261: toolbox-create-from-dispatch-plan JSON should remain a create plan");
    expect_contains(plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1261: toolbox-create-from-dispatch-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1261: toolbox-create-from-dispatch-plan host command should not mutate the visual asset");

    const auto override_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "commandbutton",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--create-object-name", "cmdLaunch",
            "--create-parent-name", "cntToolbar",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(override_process.exit_code == 0,
        "#1261: toolbox-create-from-dispatch-plan JSON should accept create override fields");
    expect_contains(override_process.stdout_text, "\"objectName\": \"cmdLaunch\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose create object-name overrides");
    expect_contains(override_process.stdout_text, "\"parentName\": \"cntToolbar\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose create parent overrides");
    expect(visual_object_count(form_path) == before_count,
        "#1261: toolbox-create-from-dispatch-plan override commands should not mutate the visual asset");

    const auto report_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "label",
            "--selection-context", "report_expression",
            "--create-unique-id", "dispatch-report-plan-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Report Plan",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_plan_process.exit_code == 0,
        "#2142: report toolbox-create-from-dispatch-plan JSON command should exit successfully");
    expect_contains(report_plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose stable create plans");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2142: report toolbox-create-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose label plans");
    expect_contains(report_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose generated label names");
    expect_contains(report_plan_process.stdout_text, "\"uniqueId\": \"dispatch-report-plan-guid\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose report label unique ids");
    expect_contains(report_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose report label parent overrides");
    expect_contains(report_plan_process.stdout_text, "\"propertyValue\": \"Dispatch Report Plan\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose report label field values");
    expect_contains(report_plan_process.stdout_text, "\"dryRun\": true",
        "#2142: report toolbox-create-from-dispatch-plan JSON should remain a create plan");
    expect_contains(report_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2142: report toolbox-create-from-dispatch-plan JSON should remain non-mutating");
    expect_not_contains(report_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2142: report toolbox-create-from-dispatch-plan host command should not mutate assets");

    const auto label_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "label",
            "--selection-context", "label_expression",
            "--create-unique-id", "dispatch-label-plan-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Label Plan",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(label_plan_process.exit_code == 0,
        "#2139: label toolbox-create-from-dispatch-plan JSON command should exit successfully");
    expect_contains(label_plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose stable create plans");
    expect_contains(label_plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2139: label toolbox-create-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(label_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose label plans");
    expect_contains(label_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose generated label names");
    expect_contains(label_plan_process.stdout_text, "\"uniqueId\": \"dispatch-label-plan-guid\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(label_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(label_plan_process.stdout_text, "\"propertyValue\": \"Dispatch Label Plan\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose label field values");
    expect_contains(label_plan_process.stdout_text, "\"dryRun\": true",
        "#2139: label toolbox-create-from-dispatch-plan JSON should remain a create plan");
    expect_contains(label_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2139: label toolbox-create-from-dispatch-plan JSON should remain non-mutating");
    expect_not_contains(label_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2139: label toolbox-create-from-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1261: toolbox-create-from-dispatch-plan JSON should reject non-admitted palette dispatches");
    expect_contains(non_admitted_process.stdout_text, "\"toolboxCreatePlan\": null",
        "#1261: failed toolbox-create-from-dispatch-plan JSON should not expose stale plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1261: non-admitted toolbox-create-from-dispatch-plan JSON should report dispatch errors");
    expect(visual_object_count(form_path) == before_count,
        "#1261: non-admitted toolbox-create-from-dispatch-plan commands should not mutate the visual asset");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--selection-context", "report_expression",
            "--object-name", "rptCustomer",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1261: toolbox-create-from-dispatch-plan JSON should reject unavailable dispatch items");
    expect_contains(unavailable_process.stdout_text, "\"toolboxCreatePlan\": null",
        "#1261: unavailable toolbox-create-from-dispatch-plan JSON should not expose stale plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1261: unavailable toolbox-create-from-dispatch-plan JSON should report availability errors");
    expect(visual_object_count(form_path) == before_count,
        "#1261: unavailable toolbox-create-from-dispatch-plan commands should not mutate the visual asset");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1261: toolbox-create-from-dispatch-plan JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1261: missing-context toolbox-create-from-dispatch-plan JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1261: toolbox-create-from-dispatch-plan JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1261: invalid-admission toolbox-create-from-dispatch-plan JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_creates_toolbox_object_from_palette_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_from_dispatch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "textbox",
            "--selection-context", "visual_object",
            "--record", "0",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--create-unique-id", "dispatch-host-created-textbox-guid",
            "--field-value", "CAPTION=Dispatch Host Created",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    if (create_process.exit_code != 0) {
        std::cerr << "studio host toolbox-create-from-dispatch stdout:\n"
                  << create_process.stdout_text << "\n";
        std::cerr << "studio host toolbox-create-from-dispatch stderr:\n"
                  << create_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }
    expect(create_process.exit_code == 0,
        "#1314: toolbox-create-from-dispatch JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
        "#1314: successful toolbox-create-from-dispatch JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"toolboxCreateFromDispatch\": {",
        "#1314: toolbox-create-from-dispatch JSON should expose a stable result object");
    expect_contains(create_process.stdout_text, "\"createPlanOk\": true",
        "#1314: toolbox-create-from-dispatch JSON should expose create-plan state");
    expect_contains(create_process.stdout_text, "\"createResult\": {",
        "#1314: toolbox-create-from-dispatch JSON should expose lower-level create results");
    expect_contains(create_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1314: toolbox-create-from-dispatch JSON should expose selected toolbox items");
    expect_contains(create_process.stdout_text, "\"className\": \"TextBox\"",
        "#1314: toolbox-create-from-dispatch JSON should expose descriptor metadata");
    expect_contains(create_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1314: toolbox-create-from-dispatch JSON should use dispatch toolbox contexts");
    expect_contains(create_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1314: toolbox-create-from-dispatch JSON should resolve visual-object form contexts");
    expect_contains(create_process.stdout_text, "\"recordIndex\": 2",
        "#1314: toolbox-create-from-dispatch JSON should expose appended record index");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1314: toolbox-create-from-dispatch JSON should expose generated object names");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"dispatch-host-created-textbox-guid\"",
        "#1314: toolbox-create-from-dispatch JSON should expose created unique ids");
    expect_contains(create_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1314: toolbox-create-from-dispatch JSON should expose created parents");
    expect_contains(create_process.stdout_text, "\"createdObjectNames\": [\"txt2\"]",
        "#1384: toolbox-create-from-dispatch JSON should summarize created object names");
    expect_contains(create_process.stdout_text, "\"createdUniqueIds\": [\"dispatch-host-created-textbox-guid\"]",
        "#1384: toolbox-create-from-dispatch JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
        "#1384: successful toolbox-create-from-dispatch JSON should summarize empty create errors");
    expect_contains(create_process.stdout_text, "\"propertyValue\": \"Dispatch Host Created\"",
        "#1314: toolbox-create-from-dispatch JSON should expose caller field values");
    expect_contains(create_process.stdout_text, "\"dryRun\": false",
        "#1314: toolbox-create-from-dispatch JSON should expose execution state");
    expect_contains(create_process.stdout_text, "\"mutatesAsset\": true",
        "#1314: toolbox-create-from-dispatch JSON should expose mutation state");
    expect(visual_object_count(form_path) == before_count + 1U,
        "#1314: toolbox-create-from-dispatch host command should mutate the asset exactly once");
    expect(visual_object_property(form_path, "dispatch-host-created-textbox-guid", "CAPTION") ==
            "Dispatch Host Created",
        "#1314: toolbox-create-from-dispatch host command should persist caller fields");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "label",
            "--selection-context", "report_expression",
            "--object-name", "rptCustomer",
            "--create-unique-id", "dispatch-host-report-label-guid",
            "--field-value", "CAPTION=Dispatch Report",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#2137: report toolbox-create-from-dispatch JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"toolboxCreateFromDispatch\": {",
        "#2137: report toolbox-create-from-dispatch JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"createPlanOk\": true",
        "#2137: report toolbox-create-from-dispatch JSON should expose create-plan state");
    expect_contains(report_process.stdout_text, "\"createResult\": {",
        "#2137: report toolbox-create-from-dispatch JSON should expose lower-level create results");
    expect_contains(report_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2137: report toolbox-create-from-dispatch JSON should use dispatch toolbox contexts");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2137: report toolbox-create-from-dispatch JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2137: report toolbox-create-from-dispatch JSON should expose generated label names");
    expect_contains(report_process.stdout_text, "\"uniqueId\": \"dispatch-host-report-label-guid\"",
        "#2137: report toolbox-create-from-dispatch JSON should expose report label unique ids");
    expect_contains(report_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
        "#2137: report toolbox-create-from-dispatch JSON should summarize created report object names");
    expect_contains(report_process.stdout_text, "\"createdUniqueIds\": [\"dispatch-host-report-label-guid\"]",
        "#2137: report toolbox-create-from-dispatch JSON should summarize created report unique ids");
    expect_contains(report_process.stdout_text, "\"createErrors\": []",
        "#2137: report toolbox-create-from-dispatch JSON should summarize empty create errors");
    expect_contains(report_process.stdout_text, "\"propertyValue\": \"Dispatch Report\"",
        "#2137: report toolbox-create-from-dispatch JSON should preserve report label field values");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2137: report toolbox-create-from-dispatch JSON should expose execution state");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2137: report toolbox-create-from-dispatch JSON should expose mutation state");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#2137: report toolbox-create-from-dispatch JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 2U,
        "#2137: report toolbox-create-from-dispatch host command should mutate the asset exactly once");
    expect(visual_object_property(form_path, "dispatch-host-report-label-guid", "CAPTION") ==
            "Dispatch Report",
        "#2137: report toolbox-create-from-dispatch host command should persist caller fields");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "label",
            "--selection-context", "label_expression",
            "--create-unique-id", "dispatch-host-label-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Label",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2091: label toolbox-create-from-dispatch JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"toolboxCreateFromDispatch\": {",
        "#2131: label toolbox-create-from-dispatch JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"createPlanOk\": true",
        "#2131: label toolbox-create-from-dispatch JSON should expose create-plan state");
    expect_contains(label_process.stdout_text, "\"createResult\": {",
        "#2131: label toolbox-create-from-dispatch JSON should expose lower-level create results");
    expect_contains(label_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2131: label toolbox-create-from-dispatch JSON should use dispatch toolbox contexts");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2091: label toolbox-create-from-dispatch JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl2\"",
        "#2091: label toolbox-create-from-dispatch JSON should expose generated label names");
    expect_contains(label_process.stdout_text, "\"uniqueId\": \"dispatch-host-label-guid\"",
        "#2091: label toolbox-create-from-dispatch JSON should expose label unique ids");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2091: label toolbox-create-from-dispatch JSON should expose label parent overrides");
    expect_contains(label_process.stdout_text, "\"createdObjectNames\": [\"lbl2\"]",
        "#2091: label toolbox-create-from-dispatch JSON should summarize created label object names");
    expect_contains(label_process.stdout_text, "\"createdUniqueIds\": [\"dispatch-host-label-guid\"]",
        "#2091: label toolbox-create-from-dispatch JSON should summarize created label unique ids");
    expect_contains(label_process.stdout_text, "\"createErrors\": []",
        "#2091: label toolbox-create-from-dispatch JSON should summarize empty create errors");
    expect_contains(label_process.stdout_text, "\"propertyValue\": \"Dispatch Label\"",
        "#2091: label toolbox-create-from-dispatch JSON should expose label field values");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2131: label toolbox-create-from-dispatch JSON should expose execution state");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2131: label toolbox-create-from-dispatch JSON should expose mutation state");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2091: label toolbox-create-from-dispatch JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 3U,
        "#2131: label toolbox-create-from-dispatch host command should mutate the asset exactly once");
    expect(visual_object_property(form_path, "dispatch-host-label-guid", "CAPTION") == "Dispatch Label",
        "#2131: label toolbox-create-from-dispatch host command should persist caller fields");

    const std::size_t committed_count = visual_object_count(form_path);
    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "textbox",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1314: toolbox-create-from-dispatch JSON should reject non-admitted palette dispatches");
    expect_contains(non_admitted_process.stdout_text, "\"createPlan\": null",
        "#1314: non-admitted toolbox-create-from-dispatch JSON should not expose stale create plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1314: non-admitted toolbox-create-from-dispatch JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"createdObjectNames\": []",
        "#1384: non-admitted toolbox-create-from-dispatch JSON should summarize no created object names");
    expect_contains(non_admitted_process.stdout_text, "\"createdUniqueIds\": []",
        "#1384: non-admitted toolbox-create-from-dispatch JSON should summarize no created unique ids");
    expect_contains(non_admitted_process.stdout_text,
        "\"createErrors\": [\"A toolbox dispatch request requires an admitted non-dry-run invocation.\"",
        "#1384: non-admitted toolbox-create-from-dispatch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1314: non-admitted toolbox-create-from-dispatch commands should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "textbox",
            "--selection-context", "report_expression",
            "--object-name", "rptCustomer",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1314: toolbox-create-from-dispatch JSON should reject unavailable dispatch items");
    expect_contains(unavailable_process.stdout_text, "\"createPlan\": null",
        "#1314: unavailable toolbox-create-from-dispatch JSON should not expose stale create plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1314: unavailable toolbox-create-from-dispatch JSON should report availability errors");
    expect_contains(unavailable_process.stdout_text, "\"objectName\": \"\"",
        "#1314: unavailable toolbox-create-from-dispatch JSON should avoid stale object names");
    expect_contains(unavailable_process.stdout_text, "\"createdObjectNames\": []",
        "#1384: unavailable toolbox-create-from-dispatch JSON should summarize no created object names");
    expect_contains(unavailable_process.stdout_text, "\"createdUniqueIds\": []",
        "#1384: unavailable toolbox-create-from-dispatch JSON should summarize no created unique ids");
    expect_contains(unavailable_process.stdout_text,
        "\"createErrors\": [\"The requested toolbox item is not available in the admitted toolbox dispatch.\"",
        "#1384: unavailable toolbox-create-from-dispatch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1314: unavailable toolbox-create-from-dispatch commands should not mutate assets");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "commandbutton",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--create-unique-id", "dispatch-host-invalid-field-guid",
            "--field-value", "UNKNOWN=Invalid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 4,
        "#1314: toolbox-create-from-dispatch JSON should reject invalid create fields");
    expect_contains(invalid_field_process.stdout_text, "\"createPlanOk\": true",
        "#1314: invalid-field toolbox-create-from-dispatch JSON should expose successful dispatch planning");
    expect_contains(invalid_field_process.stdout_text,
        "The requested field was not found in the asset.",
        "#1314: invalid-field toolbox-create-from-dispatch JSON should report lower-layer failures");
    expect(visual_object_count(form_path) == committed_count,
        "#1314: invalid-field toolbox-create-from-dispatch commands should not partially mutate assets");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "textbox",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1314: toolbox-create-from-dispatch JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1314: missing-context toolbox-create-from-dispatch JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "textbox",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1314: toolbox-create-from-dispatch JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1314: invalid-admission toolbox-create-from-dispatch JSON should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "textbox",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1314: toolbox-create-from-dispatch JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#1314: malformed-field toolbox-create-from-dispatch JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch", "textbox",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1314: toolbox-create-from-dispatch JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown toolbox-create-from-dispatch option: --toolbox-context",
        "#1314: unknown-option toolbox-create-from-dispatch JSON should report parser errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1314: rejected toolbox-create-from-dispatch commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_dispatches_from_palette_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_dispatch_from_dispatch_plan_json_tests";
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
            "--toolbox-create-dispatch-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--record", "0",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--create-unique-id", "dispatch-source-textbox-guid",
            "--field-value", "CAPTION=Dispatch Source",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(dispatch_process.exit_code == 0,
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(dispatch_process.stdout_text, "\"toolboxCreateDispatchPlan\": {",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose dispatch plans");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose selected toolbox items");
    expect_contains(dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose descriptor class names");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should resolve visual-object form contexts");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose planned target indexes");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose generated object names");
    expect_contains(dispatch_process.stdout_text, "\"uniqueId\": \"dispatch-source-textbox-guid\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose create unique-id overrides");
    expect_contains(dispatch_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should default parents from selected objects");
    expect_contains(dispatch_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose caller field values");
    expect_contains(dispatch_process.stdout_text, "\"propertyValue\": \"Dispatch Source\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose caller field-value payloads");
    expect_contains(dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-create\", \"textbox\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should dispatch to toolbox-create");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-context\", \"form\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should preserve toolbox context arguments");
    expect_contains(dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should preserve object-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--unique-id\", \"dispatch-source-textbox-guid\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should preserve unique-id arguments");
    expect_contains(dispatch_process.stdout_text, "\"--parent-name\", \"frmCustomer\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should preserve parent-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Source\"",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should preserve field-value arguments");
    expect_contains(dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(dispatch_process.stdout_text, "\"dryRun\": false",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"executed\": false",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should remain non-executing");
    expect_contains(dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1265: toolbox-create-dispatch-from-dispatch-plan host command should not mutate the visual asset");

    const auto report_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-from-dispatch-plan", "label",
            "--selection-context", "report_expression",
            "--create-unique-id", "dispatch-report-dispatch-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Report Plan",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_dispatch_process.exit_code == 0,
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxCreateDispatchPlan\": {",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose a stable result object");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose label dispatch plans");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose generated label names");
    expect_contains(report_dispatch_process.stdout_text, "\"uniqueId\": \"dispatch-report-dispatch-guid\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(report_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(report_dispatch_process.stdout_text, "\"propertyValue\": \"Dispatch Report Plan\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose label field values");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-create\", \"label\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should dispatch label creates");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should preserve report context arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should preserve label object-name arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--unique-id\", \"dispatch-report-dispatch-guid\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Report Plan\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should preserve label field arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(report_dispatch_process.stdout_text, "\"dryRun\": false",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(report_dispatch_process.stdout_text, "\"executed\": false",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should remain non-executing");
    expect_contains(report_dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(report_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2135: report toolbox-create-dispatch-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2135: report toolbox-create-dispatch-from-dispatch-plan host command should not mutate assets");

    const auto label_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-from-dispatch-plan", "label",
            "--selection-context", "label_expression",
            "--create-unique-id", "dispatch-label-dispatch-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Label Plan",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_dispatch_process.exit_code == 0,
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(label_dispatch_process.stdout_text, "\"toolboxCreateDispatchPlan\": {",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose a stable result object");
    expect_contains(label_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose label dispatch plans");
    expect_contains(label_dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(label_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose generated label names");
    expect_contains(label_dispatch_process.stdout_text, "\"uniqueId\": \"dispatch-label-dispatch-guid\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(label_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(label_dispatch_process.stdout_text, "\"propertyValue\": \"Dispatch Label Plan\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose label field values");
    expect_contains(label_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(label_dispatch_process.stdout_text, "\"--toolbox-create\", \"label\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should dispatch label creates");
    expect_contains(label_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should preserve report context arguments");
    expect_contains(label_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should preserve label object-name arguments");
    expect_contains(label_dispatch_process.stdout_text, "\"--unique-id\", \"dispatch-label-dispatch-guid\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(label_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(label_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Label Plan\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should preserve label field arguments");
    expect_contains(label_dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(label_dispatch_process.stdout_text, "\"dryRun\": false",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(label_dispatch_process.stdout_text, "\"executed\": false",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should remain non-executing");
    expect_contains(label_dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(label_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2133: label toolbox-create-dispatch-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2133: label toolbox-create-dispatch-from-dispatch-plan host command should not mutate assets");

    const auto non_admitted_palette_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(non_admitted_palette_process.exit_code == 4,
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should reject non-admitted palette dispatches");
    expect_contains(non_admitted_palette_process.stdout_text, "\"toolboxCreateDispatchPlan\": null",
        "#1265: non-admitted palette dispatch JSON should not expose stale dispatch plans");
    expect_contains(non_admitted_palette_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1265: non-admitted palette dispatch JSON should report dispatch errors");
    expect_not_contains(non_admitted_palette_process.stdout_text, "\"--toolbox-create\"",
        "#1265: non-admitted palette dispatch JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1265: non-admitted palette dispatch commands should not mutate the visual asset");

    const auto non_admitted_create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(non_admitted_create_process.exit_code == 4,
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should reject non-admitted create operations");
    expect_contains(non_admitted_create_process.stdout_text, "\"toolboxCreateDispatchPlan\": null",
        "#1265: non-admitted create-operation JSON should not expose stale dispatch plans");
    expect_contains(non_admitted_create_process.stdout_text,
        "A toolbox create dispatch request requires an admitted non-dry-run create operation.",
        "#1265: non-admitted create-operation JSON should report dispatch errors");
    expect_not_contains(non_admitted_create_process.stdout_text, "\"--toolbox-create\"",
        "#1265: non-admitted create-operation JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1265: non-admitted create-operation commands should not mutate the visual asset");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-from-dispatch-plan", "textbox",
            "--selection-context", "report_expression",
            "--object-name", "rptCustomer",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should reject unavailable dispatch items");
    expect_contains(unavailable_process.stdout_text, "\"toolboxCreateDispatchPlan\": null",
        "#1265: unavailable dispatch item JSON should not expose stale dispatch plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1265: unavailable dispatch item JSON should report availability errors");
    expect_not_contains(unavailable_process.stdout_text, "\"--toolbox-create\"",
        "#1265: unavailable dispatch item JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1265: unavailable dispatch item commands should not mutate the visual asset");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-from-dispatch-plan", "textbox",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1265: missing-context toolbox-create-dispatch-from-dispatch-plan JSON should report parser errors");

    const auto invalid_create_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_create_admission_process.exit_code == 2,
        "#1265: toolbox-create-dispatch-from-dispatch-plan JSON should reject invalid create admission tokens");
    expect_contains(invalid_create_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1265: invalid create admission token JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_batches_from_palette_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_from_dispatch_plan_json_tests";
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
            "--toolbox-create-batch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--record", "0",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--toolbox-item", "textbox",
            "--create-unique-id", "first-dispatch-textbox-guid",
            "--field-value", "CAPTION=First Dispatch",
            "--toolbox-item", "commandbutton",
            "--create-object-name", "cmdDispatch",
            "--create-unique-id", "dispatch-command-guid",
            "--create-parent-name", "cntToolbar",
            "--field-value", "CAPTION=Run Dispatch",
            "--toolbox-item", "textbox",
            "--create-unique-id", "second-dispatch-textbox-guid",
            "--field-value", "CAPTION=Second Dispatch",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(batch_plan_process.exit_code == 0,
        "#1263: toolbox-create-batch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxCreateBatchPlan\": {",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should expose batch plans");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should resolve form dispatch contexts");
    expect_contains(batch_plan_process.stdout_text, "\"itemCount\": 3",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should expose batch item counts");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should expose textbox plans");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should expose command-button plans");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should expose first target indexes");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should expose later target indexes");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should expose generated names");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should reserve generated names");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"cmdDispatch\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should preserve create object-name overrides");
    expect_contains(batch_plan_process.stdout_text, "\"uniqueId\": \"first-dispatch-textbox-guid\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should expose per-item unique ids");
    expect_contains(batch_plan_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should default parents from selected objects");
    expect_contains(batch_plan_process.stdout_text, "\"parentName\": \"cntToolbar\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should preserve create parent overrides");
    expect_contains(batch_plan_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should expose per-item field values");
    expect_contains(batch_plan_process.stdout_text, "\"dryRun\": true",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should remain a dry-run plan");
    expect_contains(batch_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1263: toolbox-create-batch-from-dispatch-plan host command should not mutate the visual asset");

    const auto report_batch_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch-plan",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-report-batch-label-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Report Batch",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_batch_plan_process.exit_code == 0,
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxCreateBatchPlan\": {",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should expose stable batch plans");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_batch_plan_process.stdout_text, "\"itemCount\": 1",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should expose report batch item counts");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should expose label batch plans");
    expect_contains(report_batch_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should expose generated label names");
    expect_contains(report_batch_plan_process.stdout_text, "\"uniqueId\": \"dispatch-report-batch-label-guid\"",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(report_batch_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(report_batch_plan_process.stdout_text, "\"propertyValue\": \"Dispatch Report Batch\"",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should expose label field values");
    expect_contains(report_batch_plan_process.stdout_text, "\"dryRun\": true",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should remain a dry-run plan");
    expect_contains(report_batch_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should remain non-mutating");
    expect_not_contains(report_batch_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2141: report toolbox-create-batch-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2141: report toolbox-create-batch-from-dispatch-plan host command should not mutate assets");

    const auto label_batch_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch-plan",
            "--selection-context", "label_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-label-batch-label-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Label Batch",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(label_batch_plan_process.exit_code == 0,
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(label_batch_plan_process.stdout_text, "\"toolboxCreateBatchPlan\": {",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should expose stable batch plans");
    expect_contains(label_batch_plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(label_batch_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_batch_plan_process.stdout_text, "\"itemCount\": 1",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should expose label batch item counts");
    expect_contains(label_batch_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should expose label batch plans");
    expect_contains(label_batch_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should expose generated label names");
    expect_contains(label_batch_plan_process.stdout_text, "\"uniqueId\": \"dispatch-label-batch-label-guid\"",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(label_batch_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(label_batch_plan_process.stdout_text, "\"propertyValue\": \"Dispatch Label Batch\"",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should expose label field values");
    expect_contains(label_batch_plan_process.stdout_text, "\"dryRun\": true",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should remain a dry-run plan");
    expect_contains(label_batch_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should remain non-mutating");
    expect_not_contains(label_batch_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2140: label toolbox-create-batch-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2140: label toolbox-create-batch-from-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should reject non-admitted dispatches");
    expect_contains(non_admitted_process.stdout_text, "\"toolboxCreateBatchPlan\": null",
        "#1263: non-admitted toolbox-create-batch-from-dispatch-plan JSON should not expose stale plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1263: non-admitted toolbox-create-batch-from-dispatch-plan JSON should report dispatch errors");
    expect(visual_object_count(form_path) == before_count,
        "#1263: non-admitted toolbox-create-batch-from-dispatch-plan commands should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch-plan",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should reject unavailable dispatch items");
    expect_contains(unavailable_process.stdout_text, "\"toolboxCreateBatchPlan\": null",
        "#1263: unavailable toolbox-create-batch-from-dispatch-plan JSON should not expose stale plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1263: unavailable toolbox-create-batch-from-dispatch-plan JSON should report availability errors");
    expect(visual_object_count(form_path) == before_count,
        "#1263: unavailable toolbox-create-batch-from-dispatch-plan commands should not mutate assets");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1263: empty toolbox-create-batch-from-dispatch-plan JSON should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--create-parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1263: orphan toolbox-create-batch-from-dispatch-plan item options should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1263: toolbox-create-batch-from-dispatch-plan JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1263: invalid toolbox-create-batch-from-dispatch-plan admission tokens should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_creates_toolbox_object_batches_from_palette_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_from_dispatch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--record", "0",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--toolbox-item", "textbox",
            "--create-unique-id", "first-dispatch-host-batch-guid",
            "--field-value", "CAPTION=First Dispatch Host Batch",
            "--toolbox-item", "commandbutton",
            "--create-object-name", "cmdDispatchHostBatch",
            "--create-unique-id", "dispatch-host-batch-command-guid",
            "--create-parent-name", "cntToolbar",
            "--field-value", "CAPTION=Run Dispatch Host Batch",
            "--toolbox-item", "textbox",
            "--create-unique-id", "second-dispatch-host-batch-guid",
            "--field-value", "CAPTION=Second Dispatch Host Batch",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    if (create_process.exit_code != 0) {
        std::cerr << "studio host toolbox-create-batch-from-dispatch stdout:\n"
                  << create_process.stdout_text << "\n";
        std::cerr << "studio host toolbox-create-batch-from-dispatch stderr:\n"
                  << create_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }
    expect(create_process.exit_code == 0,
        "#1315: toolbox-create-batch-from-dispatch JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
        "#1315: successful toolbox-create-batch-from-dispatch JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"toolboxCreateBatchFromDispatch\": {",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose a stable result object");
    expect_contains(create_process.stdout_text, "\"batchPlanOk\": true",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose batch-plan state");
    expect_contains(create_process.stdout_text, "\"createResult\": {",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose lower-level create results");
    expect_contains(create_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1315: toolbox-create-batch-from-dispatch JSON should use dispatch toolbox contexts");
    expect_contains(create_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should resolve form contexts");
    expect_contains(create_process.stdout_text, "\"itemCount\": 3",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose item counts");
    expect_contains(create_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose textbox plans");
    expect_contains(create_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose command-button plans");
    expect_contains(create_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose first target indexes");
    expect_contains(create_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose later target indexes");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose generated names");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should reserve generated names");
    expect_contains(create_process.stdout_text, "\"objectName\": \"cmdDispatchHostBatch\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should preserve explicit names");
    expect_contains(create_process.stdout_text, "\"recordIndexes\": [2, 3, 4]",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose created record indexes");
    expect_contains(create_process.stdout_text, "\"createdObjects\": [",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose created object metadata");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"first-dispatch-host-batch-guid\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose created unique ids");
    expect_contains(create_process.stdout_text, "\"parentName\": \"cntToolbar\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should preserve parent overrides");
    expect_contains(create_process.stdout_text,
        "\"createdObjectNames\": [\"txt2\", \"cmdDispatchHostBatch\", \"txt3\"]",
        "#1385: toolbox-create-batch-from-dispatch JSON should summarize created object names");
    expect_contains(create_process.stdout_text,
        "\"createdUniqueIds\": [\"first-dispatch-host-batch-guid\", \"dispatch-host-batch-command-guid\", \"second-dispatch-host-batch-guid\"]",
        "#1385: toolbox-create-batch-from-dispatch JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
        "#1385: successful toolbox-create-batch-from-dispatch JSON should summarize empty create errors");
    expect_contains(create_process.stdout_text, "\"dryRun\": false",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose execution state");
    expect_contains(create_process.stdout_text, "\"mutatesAsset\": true",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose mutation state");
    expect(visual_object_count(form_path) == before_count + 3U,
        "#1315: toolbox-create-batch-from-dispatch host command should append all batch objects");
    expect(visual_object_property(form_path, "first-dispatch-host-batch-guid", "CAPTION") ==
            "First Dispatch Host Batch" &&
            visual_object_property(form_path, "dispatch-host-batch-command-guid", "CAPTION") ==
            "Run Dispatch Host Batch" &&
            visual_object_property(form_path, "second-dispatch-host-batch-guid", "CAPTION") ==
            "Second Dispatch Host Batch",
        "#1315: toolbox-create-batch-from-dispatch host command should persist per-item fields");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-host-batch-report-label-guid",
            "--field-value", "CAPTION=Batch Report Label",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#2138: report toolbox-create-batch-from-dispatch JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"toolboxCreateBatchFromDispatch\": {",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"batchPlanOk\": true",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose batch-plan state");
    expect_contains(report_process.stdout_text, "\"createResult\": {",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose lower-level create results");
    expect_contains(report_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2138: report toolbox-create-batch-from-dispatch JSON should use dispatch toolbox contexts");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"itemCount\": 1",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose report batch item counts");
    expect_contains(report_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose label batch plans");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose generated label names");
    expect_contains(report_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
        "#2138: report toolbox-create-batch-from-dispatch JSON should summarize created report object names");
    expect_contains(report_process.stdout_text, "\"createdUniqueIds\": [\"dispatch-host-batch-report-label-guid\"]",
        "#2138: report toolbox-create-batch-from-dispatch JSON should summarize created report unique ids");
    expect_contains(report_process.stdout_text, "\"createErrors\": []",
        "#2138: report toolbox-create-batch-from-dispatch JSON should summarize empty create errors");
    expect_contains(report_process.stdout_text, "\"propertyValue\": \"Batch Report Label\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should preserve report label field values");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose execution state");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose mutation state");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 4U,
        "#2138: report toolbox-create-batch-from-dispatch host command should append one object");
    expect(visual_object_property(form_path, "dispatch-host-batch-report-label-guid", "CAPTION") ==
            "Batch Report Label",
        "#2138: report toolbox-create-batch-from-dispatch host command should persist caller fields");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "label_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-host-batch-label-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Batch Label",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2132: label toolbox-create-batch-from-dispatch JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"toolboxCreateBatchFromDispatch\": {",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"batchPlanOk\": true",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose batch-plan state");
    expect_contains(label_process.stdout_text, "\"createResult\": {",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose lower-level create results");
    expect_contains(label_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2132: label toolbox-create-batch-from-dispatch JSON should use dispatch toolbox contexts");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"itemCount\": 1",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose label batch item counts");
    expect_contains(label_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose label batch plans");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl2\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose generated label names");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should preserve label parent overrides");
    expect_contains(label_process.stdout_text, "\"propertyValue\": \"Batch Label\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should preserve label field values");
    expect_contains(label_process.stdout_text, "\"createdObjectNames\": [\"lbl2\"]",
        "#2132: label toolbox-create-batch-from-dispatch JSON should summarize created label object names");
    expect_contains(label_process.stdout_text, "\"createdUniqueIds\": [\"dispatch-host-batch-label-guid\"]",
        "#2132: label toolbox-create-batch-from-dispatch JSON should summarize created label unique ids");
    expect_contains(label_process.stdout_text, "\"createErrors\": []",
        "#2132: label toolbox-create-batch-from-dispatch JSON should summarize empty create errors");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose execution state");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose mutation state");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 5U,
        "#2132: label toolbox-create-batch-from-dispatch host command should append one object");
    expect(visual_object_property(form_path, "dispatch-host-batch-label-guid", "CAPTION") == "Batch Label",
        "#2132: label toolbox-create-batch-from-dispatch host command should persist caller fields");

    const std::size_t committed_count = visual_object_count(form_path);
    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject non-admitted dispatches");
    expect_contains(non_admitted_process.stdout_text, "\"batchPlan\": null",
        "#1315: non-admitted toolbox-create-batch-from-dispatch JSON should not expose stale batch plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1315: non-admitted toolbox-create-batch-from-dispatch JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"createdObjectNames\": []",
        "#1385: non-admitted toolbox-create-batch-from-dispatch JSON should summarize no created object names");
    expect_contains(non_admitted_process.stdout_text, "\"createdUniqueIds\": []",
        "#1385: non-admitted toolbox-create-batch-from-dispatch JSON should summarize no created unique ids");
    expect_contains(non_admitted_process.stdout_text,
        "\"createErrors\": [\"A toolbox dispatch request requires an admitted non-dry-run invocation.\"",
        "#1385: non-admitted toolbox-create-batch-from-dispatch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: non-admitted toolbox-create-batch-from-dispatch commands should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject unavailable dispatch items");
    expect_contains(unavailable_process.stdout_text, "\"batchPlan\": null",
        "#1315: unavailable toolbox-create-batch-from-dispatch JSON should not expose stale batch plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1315: unavailable toolbox-create-batch-from-dispatch JSON should report availability errors");
    expect_contains(unavailable_process.stdout_text, "\"createdObjectNames\": []",
        "#1385: unavailable toolbox-create-batch-from-dispatch JSON should summarize no created object names");
    expect_contains(unavailable_process.stdout_text, "\"createdUniqueIds\": []",
        "#1385: unavailable toolbox-create-batch-from-dispatch JSON should summarize no created unique ids");
    expect_contains(unavailable_process.stdout_text,
        "\"createErrors\": [\"The requested toolbox item is not available in the admitted toolbox dispatch.\"",
        "#1385: unavailable toolbox-create-batch-from-dispatch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: unavailable toolbox-create-batch-from-dispatch commands should not mutate assets");

    const auto duplicate_identity_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--create-unique-id", "duplicate-dispatch-host-batch-guid",
            "--toolbox-item", "commandbutton",
            "--create-unique-id", "duplicate-dispatch-host-batch-guid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(duplicate_identity_process.exit_code == 4,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject duplicate planned identities");
    expect_contains(duplicate_identity_process.stdout_text, "\"batchPlanOk\": false",
        "#1315: duplicate toolbox-create-batch-from-dispatch JSON should report failed batch planning");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: duplicate toolbox-create-batch-from-dispatch commands should not mutate assets");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--create-unique-id", "invalid-field-dispatch-host-batch-guid",
            "--field-value", "UNKNOWN=Invalid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 4,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject lower-layer invalid fields");
    expect_contains(invalid_field_process.stdout_text, "\"batchPlanOk\": true",
        "#1315: invalid-field toolbox-create-batch-from-dispatch JSON should preserve successful planning");
    expect_contains(invalid_field_process.stdout_text,
        "The requested field was not found in the asset.",
        "#1315: invalid-field toolbox-create-batch-from-dispatch JSON should report lower-layer failures");
    expect_contains(invalid_field_process.stdout_text, "\"createdObjectNames\": []",
        "#1385: invalid-field toolbox-create-batch-from-dispatch JSON should summarize no created object names");
    expect_contains(invalid_field_process.stdout_text,
        "\"createErrors\": [\"The requested field was not found in the asset.\"",
        "#1385: invalid-field toolbox-create-batch-from-dispatch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: invalid-field toolbox-create-batch-from-dispatch commands should not partially mutate assets");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1315: empty toolbox-create-batch-from-dispatch JSON should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--create-parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1315: orphan toolbox-create-batch-from-dispatch item options should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1315: invalid toolbox-create-batch-from-dispatch admission tokens should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#1315: malformed toolbox-create-batch-from-dispatch field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown toolbox-create-batch-from-dispatch option: --toolbox-context",
        "#1315: unknown toolbox-create-batch-from-dispatch options should report parser errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: rejected toolbox-create-batch-from-dispatch commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_batch_dispatches_from_palette_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_dispatch_from_dispatch_plan_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto batch_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--record", "0",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--toolbox-item", "textbox",
            "--create-unique-id", "first-batch-dispatch-source-guid",
            "--field-value", "CAPTION=First Batch Dispatch",
            "--toolbox-item", "commandbutton",
            "--create-object-name", "cmdBatchDispatchSource",
            "--create-unique-id", "batch-dispatch-source-command-guid",
            "--create-parent-name", "cntToolbar",
            "--field-value", "CAPTION=Run Batch Dispatch",
            "--toolbox-item", "textbox",
            "--create-unique-id", "second-batch-dispatch-source-guid",
            "--field-value", "CAPTION=Second Batch Dispatch",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(batch_dispatch_process.exit_code == 0,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose batch dispatch plans");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should resolve form contexts");
    expect_contains(batch_dispatch_process.stdout_text, "\"itemCount\": 3",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose item counts");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose textbox plans");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose command-button plans");
    expect_contains(batch_dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose first target indexes");
    expect_contains(batch_dispatch_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose later target indexes");
    expect_contains(batch_dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose generated names");
    expect_contains(batch_dispatch_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reserve generated names");
    expect_contains(batch_dispatch_process.stdout_text, "\"objectName\": \"cmdBatchDispatchSource\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve explicit names");
    expect_contains(batch_dispatch_process.stdout_text, "\"uniqueId\": \"first-batch-dispatch-source-guid\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose unique ids");
    expect_contains(batch_dispatch_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should default selected parents");
    expect_contains(batch_dispatch_process.stdout_text, "\"parentName\": \"cntToolbar\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve parent overrides");
    expect_contains(batch_dispatch_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose field values");
    expect_contains(batch_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(batch_dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should dispatch to batch create");
    expect_contains(batch_dispatch_process.stdout_text, "\"--toolbox-context\", \"form\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve toolbox contexts");
    expect_contains(batch_dispatch_process.stdout_text, "\"--toolbox-item\", \"textbox\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve textbox items");
    expect_contains(batch_dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve generated names");
    expect_contains(batch_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=First Batch Dispatch\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve first field values");
    expect_contains(batch_dispatch_process.stdout_text, "\"--toolbox-item\", \"commandbutton\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve command items");
    expect_contains(batch_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Run Batch Dispatch\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve command field values");
    expect_contains(batch_dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(batch_dispatch_process.stdout_text, "\"dryRun\": false",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(batch_dispatch_process.stdout_text, "\"executed\": false",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should remain non-executing");
    expect_contains(batch_dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan host command should not mutate the visual asset");

    const auto report_batch_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-report-batch-dispatch-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Report Batch Plan",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_batch_dispatch_process.exit_code == 0,
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose stable batch dispatch plans");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"itemCount\": 1",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose report batch item counts");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label batch dispatch plans");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose generated label names");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"uniqueId\": \"dispatch-report-batch-dispatch-guid\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"propertyValue\": \"Dispatch Report Batch Plan\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label field values");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should dispatch to batch create");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve report context arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--toolbox-item\", \"label\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label item arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label object-name arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--unique-id\", \"dispatch-report-batch-dispatch-guid\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Report Batch Plan\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label field arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"dryRun\": false",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"executed\": false",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should remain non-executing");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(report_batch_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan host command should not mutate assets");

    const auto label_batch_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "label_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-label-batch-dispatch-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Label Batch Plan",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_batch_dispatch_process.exit_code == 0,
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose stable batch dispatch plans");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"itemCount\": 1",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label batch item counts");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label batch dispatch plans");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose generated label names");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"uniqueId\": \"dispatch-label-batch-dispatch-guid\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"propertyValue\": \"Dispatch Label Batch Plan\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label field values");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should dispatch to batch create");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve report context arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--toolbox-item\", \"label\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label item arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label object-name arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--unique-id\", \"dispatch-label-batch-dispatch-guid\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Label Batch Plan\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label field arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"dryRun\": false",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"executed\": false",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should remain non-executing");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(label_batch_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan host command should not mutate assets");

    const auto non_admitted_palette_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(non_admitted_palette_process.exit_code == 4,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject non-admitted dispatches");
    expect_contains(non_admitted_palette_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": null",
        "#1267: non-admitted batch dispatch JSON should not expose stale plans");
    expect_contains(non_admitted_palette_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1267: non-admitted batch dispatch JSON should report dispatch errors");
    expect_not_contains(non_admitted_palette_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1267: non-admitted batch dispatch JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1267: non-admitted batch dispatch commands should not mutate assets");

    const auto non_admitted_create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(non_admitted_create_process.exit_code == 4,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject non-admitted create operations");
    expect_contains(non_admitted_create_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": null",
        "#1267: non-admitted create batch dispatch JSON should not expose stale plans");
    expect_contains(non_admitted_create_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1267: non-admitted create batch dispatch JSON should report dispatch errors");
    expect_not_contains(non_admitted_create_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1267: non-admitted create batch dispatch JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1267: non-admitted create batch dispatch commands should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject unavailable dispatch items");
    expect_contains(unavailable_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": null",
        "#1267: unavailable batch dispatch JSON should not expose stale plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1267: unavailable batch dispatch JSON should report availability errors");
    expect_not_contains(unavailable_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1267: unavailable batch dispatch JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1267: unavailable batch dispatch commands should not mutate assets");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1267: empty toolbox-create-batch-dispatch-from-dispatch-plan JSON should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--create-parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1267: orphan toolbox-create-batch-dispatch-from-dispatch-plan item options should report parser errors");

    const auto invalid_create_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_create_admission_process.exit_code == 2,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject invalid create admission tokens");
    expect_contains(invalid_create_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1267: invalid create admission JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
