// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_toolbox_invocation_admission(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_invocation_admission_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1220: toolbox invocation-admission JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxInvocationAdmission\": {",
        "#1220: toolbox invocation-admission JSON should expose a plan object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1220: toolbox invocation-admission JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1220: visual-object toolbox invocation-admission JSON should resolve form toolbox contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1220: toolbox invocation-admission JSON should carry asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1220: toolbox invocation-admission JSON should carry record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1220: toolbox invocation-admission JSON should carry object-name selectors");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1220: toolbox invocation-admission JSON should carry unique-id selectors");
    expect_contains(visual_process.stdout_text, "\"id\": \"textbox\"",
        "#1220: visual-object toolbox invocation-admission JSON should include form-safe TextBox items");
    expect_contains(visual_process.stdout_text, "\"admissionReadyItemIds\": [\"label\", \"textbox\"",
        "#1388: toolbox invocation-admission JSON should summarize admission-ready items");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedItemIds\": []",
        "#1388: admitted toolbox invocation-admission JSON should summarize empty blocked item ids");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1388: admitted toolbox invocation-admission JSON should summarize empty admission errors");
    expect_contains(visual_process.stdout_text, "\"paletteInvocationAdmitted\": true",
        "#1220: admitted toolbox invocation-admission JSON should expose admitted state");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1220: admitted toolbox invocation-admission JSON should not be marked dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1220: toolbox invocation-admission JSON should remain non-mutating");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1220: toolbox invocation-admission JSON should default to dry-run admission");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1220: report toolbox invocation-admission JSON should resolve report toolbox contexts");
    expect_contains(report_process.stdout_text, "\"id\": \"label\"",
        "#1220: report toolbox invocation-admission JSON should include report-safe Label items");
    expect_not_contains(report_process.stdout_text, "\"id\": \"textbox\"",
        "#1220: report toolbox invocation-admission JSON should exclude form-only TextBox items");
    expect_contains(report_process.stdout_text, "\"admissionReadyItemIds\": [\"label\"",
        "#1388: dry-run toolbox invocation-admission JSON should preserve admission-ready items");
    expect_contains(report_process.stdout_text, "\"admissionBlockedItemIds\": []",
        "#1388: dry-run toolbox invocation-admission JSON should summarize empty blocked item ids");
    expect_contains(report_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1388: dry-run toolbox invocation-admission JSON should summarize empty admission errors");
    expect_contains(report_process.stdout_text, "\"paletteInvocationAdmitted\": false",
        "#1220: default toolbox invocation-admission JSON should not admit invocation");
    expect_contains(report_process.stdout_text, "\"dryRun\": true",
        "#1220: default toolbox invocation-admission JSON should expose dry-run state");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1220: toolbox invocation-admission JSON should reject unsupported selection contexts");
    expect_contains(unsupported_process.stdout_text, "\"toolboxInvocationAdmission\": null",
        "#1220: unsupported toolbox invocation-admission JSON should not expose a plan object");
    expect_contains(unsupported_process.stdout_text,
        "The selected Studio context does not expose a toolbox palette.",
        "#1220: unsupported toolbox invocation-admission JSON should report validation errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1220: toolbox invocation-admission JSON should reject invalid admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1220: invalid toolbox invocation-admission boolean JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission",
            "--selection-context", "visual_object",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1220: toolbox invocation-admission JSON should reject invalid record indexes");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1220: invalid toolbox invocation-admission record JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1220: toolbox invocation-admission JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1220: missing toolbox invocation-admission context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_toolbox_invocation_admission_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_invocation_admission_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto form_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission-catalog",
            "--toolbox-context", "form",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(form_process.exit_code == 0,
        "#1286: toolbox invocation-admission catalog JSON should accept admitted form contexts");
    expect_contains(form_process.stdout_text, "\"toolboxInvocationAdmissionCatalog\": {",
        "#1286: toolbox invocation-admission catalog JSON should expose a catalog object");
    expect_contains(form_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1286: toolbox invocation-admission catalog JSON should expose toolbox contexts");
    expect_contains(form_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1286: toolbox invocation-admission catalog JSON should expose command tokens");
    expect_contains(form_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1286: toolbox invocation-admission catalog JSON should carry asset paths");
    expect_contains(form_process.stdout_text, "\"recordIndex\": 1",
        "#1286: toolbox invocation-admission catalog JSON should carry record indexes");
    expect_contains(form_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1286: toolbox invocation-admission catalog JSON should carry object-name selectors");
    expect_contains(form_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1286: toolbox invocation-admission catalog JSON should carry unique-id selectors");
    expect_contains(form_process.stdout_text, "\"admissionCount\": 1",
        "#1286: admitted toolbox invocation-admission catalog JSON should expose admission counts");
    expect_contains(form_process.stdout_text, "\"errorCount\": 0",
        "#1286: admitted toolbox invocation-admission catalog JSON should expose zero errors");
    expect_contains(form_process.stdout_text, "\"dryRun\": false",
        "#1286: admitted toolbox invocation-admission catalog JSON should not be dry-run");
    expect_contains(form_process.stdout_text, "\"mutatesAsset\": false",
        "#1286: toolbox invocation-admission catalog JSON should remain non-mutating");
    expect_contains(form_process.stdout_text, "\"admissionReadyItemIds\": [\"label\", \"textbox\"",
        "#1369: toolbox invocation-admission catalog JSON should summarize admission-ready items");
    expect_contains(form_process.stdout_text, "\"admissionBlockedItemIds\": []",
        "#1369: admitted toolbox invocation-admission catalog JSON should summarize empty blocked item ids");
    expect_contains(form_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1369: admitted toolbox invocation-admission catalog JSON should summarize empty blocked admission errors");
    expect_contains(form_process.stdout_text, "\"id\": \"textbox\"",
        "#1286: form toolbox invocation-admission catalog JSON should include form-safe TextBox items");
    expect_contains(form_process.stdout_text, "\"invocationAdmissionOk\": true",
        "#1286: toolbox invocation-admission catalog JSON should expose admission results");
    expect_contains(form_process.stdout_text, "\"paletteInvocationAdmitted\": true",
        "#1286: toolbox invocation-admission catalog JSON should expose admitted palette state");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission-catalog",
            "--toolbox-context", "report",
            "--path", "reports/orders.frx",
            "--record", "3",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1286: toolbox invocation-admission catalog JSON should accept dry-run report contexts");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1286: report toolbox invocation-admission catalog JSON should expose report contexts");
    expect_contains(report_process.stdout_text, "\"id\": \"label\"",
        "#1286: report toolbox invocation-admission catalog JSON should include report-safe Label items");
    expect_not_contains(report_process.stdout_text, "\"id\": \"textbox\"",
        "#1286: report toolbox invocation-admission catalog JSON should exclude form-only TextBox items");
    expect_contains(report_process.stdout_text, "\"admissionCount\": 1",
        "#1286: dry-run report toolbox invocation-admission catalog JSON should expose admission counts");
    expect_contains(report_process.stdout_text, "\"paletteInvocationAdmitted\": false",
        "#1286: dry-run toolbox invocation-admission catalog JSON should expose unadmitted palette state");
    expect_contains(report_process.stdout_text, "\"dryRun\": true",
        "#1286: dry-run toolbox invocation-admission catalog JSON should preserve dry-run state");
    expect_contains(report_process.stdout_text, "\"admissionReadyItemIds\": [\"label\"",
        "#1369: dry-run toolbox invocation-admission catalog JSON should preserve admission-ready items");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission-catalog",
            "--toolbox-context", "form",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1286: toolbox invocation-admission catalog JSON should reject invalid admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1286: invalid toolbox invocation-admission catalog boolean JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission-catalog",
            "--toolbox-context", "form",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1286: toolbox invocation-admission catalog JSON should reject invalid record indexes");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1286: invalid toolbox invocation-admission catalog record JSON should report parser errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission-catalog",
            "--toolbox-context", "menu",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1286: toolbox invocation-admission catalog JSON should reject unknown toolbox contexts");
    expect_contains(unknown_context_process.stdout_text, "Unknown toolbox context token: menu",
        "#1286: unknown toolbox invocation-admission catalog context JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-invocation-admission-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1286: toolbox invocation-admission catalog JSON should reject missing toolbox contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1286: missing toolbox invocation-admission catalog context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selection_toolbox_invocation_admission_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_invocation_admission_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-invocation-admission-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1289: selection toolbox invocation-admission catalog JSON should accept admitted visual contexts");
    expect_contains(visual_process.stdout_text, "\"selectionToolboxInvocationAdmissionCatalog\": {",
        "#1289: selection toolbox invocation-admission catalog JSON should expose catalog objects");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1289: selection toolbox invocation-admission catalog JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1289: visual selection toolbox invocation-admission catalog JSON should resolve form contexts");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1289: selection toolbox invocation-admission catalog JSON should expose command tokens");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1289: selection toolbox invocation-admission catalog JSON should carry asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1289: selection toolbox invocation-admission catalog JSON should carry record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1289: selection toolbox invocation-admission catalog JSON should carry object-name selectors");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1289: selection toolbox invocation-admission catalog JSON should carry unique-id selectors");
    expect_contains(visual_process.stdout_text, "\"admissionCount\": 1",
        "#1289: admitted selection toolbox invocation-admission catalog JSON should expose admission counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1289: admitted selection toolbox invocation-admission catalog JSON should expose zero errors");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1289: admitted selection toolbox invocation-admission catalog JSON should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1289: selection toolbox invocation-admission catalog JSON should remain non-mutating");
    expect_contains(visual_process.stdout_text, "\"admissionReadyItemIds\": [\"label\", \"textbox\"",
        "#1373: selection toolbox invocation-admission catalog JSON should summarize admission-ready items");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedItemIds\": []",
        "#1373: admitted selection toolbox invocation-admission catalog JSON should summarize empty blocked item ids");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1373: admitted selection toolbox invocation-admission catalog JSON should summarize empty blocked admission errors");
    expect_contains(visual_process.stdout_text, "\"id\": \"textbox\"",
        "#1289: visual selection toolbox invocation-admission catalog JSON should include form-safe TextBox items");
    expect_contains(visual_process.stdout_text, "\"launchPlanOk\": true",
        "#1289: selection toolbox invocation-admission catalog JSON should expose launch-plan state");
    expect_contains(visual_process.stdout_text, "\"invocationAdmissionOk\": true",
        "#1289: selection toolbox invocation-admission catalog JSON should expose admission state");
    expect_contains(visual_process.stdout_text, "\"paletteInvocationAdmitted\": true",
        "#1289: selection toolbox invocation-admission catalog JSON should expose admitted palette state");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-invocation-admission-catalog",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "3",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1289: selection toolbox invocation-admission catalog JSON should accept dry-run report contexts");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1289: report selection toolbox invocation-admission catalog JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1289: report selection toolbox invocation-admission catalog JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"id\": \"label\"",
        "#1289: report selection toolbox invocation-admission catalog JSON should include report-safe Label items");
    expect_not_contains(report_process.stdout_text, "\"id\": \"textbox\"",
        "#1289: report selection toolbox invocation-admission catalog JSON should exclude form-only TextBox items");
    expect_contains(report_process.stdout_text, "\"paletteInvocationAdmitted\": false",
        "#1289: dry-run selection toolbox invocation-admission catalog JSON should expose unadmitted palette state");
    expect_contains(report_process.stdout_text, "\"dryRun\": true",
        "#1289: dry-run selection toolbox invocation-admission catalog JSON should preserve dry-run state");
    expect_contains(report_process.stdout_text, "\"admissionReadyItemIds\": [\"label\"",
        "#1373: dry-run selection toolbox invocation-admission catalog JSON should preserve admission-ready items");
    expect_contains(report_process.stdout_text, "\"admissionBlockedItemIds\": []",
        "#1373: dry-run selection toolbox invocation-admission catalog JSON should summarize empty blocked item ids");
    expect_contains(report_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1373: dry-run selection toolbox invocation-admission catalog JSON should summarize empty blocked admission errors");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-invocation-admission-catalog",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1289: selection toolbox invocation-admission catalog JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionToolboxInvocationAdmissionCatalog\": null",
        "#1289: unsupported selection toolbox invocation-admission catalog JSON should omit catalog objects");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox invocation admission catalog request requires a toolbox palette.",
        "#1289: unsupported selection toolbox invocation-admission catalog JSON should report planner errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-invocation-admission-catalog",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1289: selection toolbox invocation-admission catalog JSON should reject invalid booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1289: invalid selection toolbox invocation-admission catalog boolean JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-invocation-admission-catalog",
            "--selection-context", "visual_object",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1289: selection toolbox invocation-admission catalog JSON should reject invalid records");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1289: invalid selection toolbox invocation-admission catalog record JSON should report parser errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-invocation-admission-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1289: selection toolbox invocation-admission catalog JSON should reject unknown selections");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1289: unknown selection toolbox invocation-admission catalog JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-invocation-admission-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1289: selection toolbox invocation-admission catalog JSON should reject missing selections");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1289: missing selection toolbox invocation-admission catalog JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_toolbox_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_toolbox_dispatch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1234: toolbox dispatch JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxDispatch\": {",
        "#1234: toolbox dispatch JSON should expose a plan object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1234: toolbox dispatch JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1234: visual-object toolbox dispatch JSON should resolve form toolbox contexts");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1234: toolbox dispatch JSON should expose command tokens");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1234: toolbox dispatch JSON should carry asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1234: toolbox dispatch JSON should carry record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1234: toolbox dispatch JSON should carry object-name selectors");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1234: toolbox dispatch JSON should carry unique-id selectors");
    expect_contains(visual_process.stdout_text, "\"id\": \"textbox\"",
        "#1234: visual-object toolbox dispatch JSON should include form-safe TextBox items");
    expect_contains(visual_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\", \"textbox\"",
        "#1389: toolbox dispatch JSON should summarize dispatch-ready items");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1389: admitted toolbox dispatch JSON should summarize empty blocked item ids");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1389: admitted toolbox dispatch JSON should summarize empty dispatch errors");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [",
        "#1234: toolbox dispatch JSON should expose dispatch arguments");
    expect_contains(visual_process.stdout_text, "\"--toolbox-context\"",
        "#1234: toolbox dispatch JSON should expose toolbox-context arguments");
    expect_contains(visual_process.stdout_text, "\"form\"",
        "#1234: toolbox dispatch JSON should expose toolbox-context values");
    expect_contains(visual_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1234: admitted toolbox dispatch JSON should expose dispatch-admitted state");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1234: admitted toolbox dispatch JSON should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"executed\": false",
        "#1234: toolbox dispatch JSON should not execute toolbox UI");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1234: toolbox dispatch JSON should remain non-mutating");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "3",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1234: toolbox dispatch JSON should accept admitted report contexts");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1234: report toolbox dispatch JSON should resolve report toolbox contexts");
    expect_contains(report_process.stdout_text, "\"id\": \"label\"",
        "#1234: report toolbox dispatch JSON should include report-safe Label items");
    expect_not_contains(report_process.stdout_text, "\"id\": \"textbox\"",
        "#1234: report toolbox dispatch JSON should exclude form-only TextBox items");
    expect_contains(report_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#1389: report toolbox dispatch JSON should summarize dispatch-ready items");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1389: report toolbox dispatch JSON should summarize empty blocked item ids");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1389: report toolbox dispatch JSON should summarize empty dispatch errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 4,
        "#1234: toolbox dispatch JSON should reject dry-run dispatch requests");
    expect_contains(dry_run_process.stdout_text, "\"toolboxDispatch\": null",
        "#1234: dry-run toolbox dispatch JSON should not expose a plan object");
    expect_contains(dry_run_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1234: dry-run toolbox dispatch JSON should report dispatch admission errors");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch",
            "--selection-context", "menu_item",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1234: toolbox dispatch JSON should reject unsupported selection contexts");
    expect_contains(unsupported_process.stdout_text, "\"toolboxDispatch\": null",
        "#1234: unsupported toolbox dispatch JSON should not expose a plan object");
    expect_contains(unsupported_process.stdout_text,
        "The selected Studio context does not expose a toolbox palette.",
        "#1234: unsupported toolbox dispatch JSON should report validation errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1234: toolbox dispatch JSON should reject invalid admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1234: invalid toolbox dispatch boolean JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch",
            "--selection-context", "visual_object",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1234: toolbox dispatch JSON should reject invalid record indexes");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1234: invalid toolbox dispatch record JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1234: toolbox dispatch JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1234: missing toolbox dispatch context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_toolbox_execution(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_toolbox_execution_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1323: toolbox execution JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxExecution\": {",
        "#1323: toolbox execution JSON should expose an execution object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1323: toolbox execution JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1323: visual-object toolbox execution JSON should resolve form toolbox contexts");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1323: toolbox execution JSON should expose command tokens");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1323: toolbox execution JSON should carry asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1323: toolbox execution JSON should carry record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1323: toolbox execution JSON should carry object-name selectors");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1323: toolbox execution JSON should carry unique-id selectors");
    expect_contains(visual_process.stdout_text, "\"id\": \"textbox\"",
        "#1323: visual-object toolbox execution JSON should include form-safe TextBox items");
    expect_contains(visual_process.stdout_text, "\"executionReadyItemIds\": [\"label\", \"textbox\"",
        "#1390: toolbox execution JSON should summarize execution-ready items");
    expect_contains(visual_process.stdout_text, "\"executionBlockedItemIds\": []",
        "#1390: admitted toolbox execution JSON should summarize empty blocked item ids");
    expect_contains(visual_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1390: admitted toolbox execution JSON should summarize empty execution errors");
    expect_contains(visual_process.stdout_text, "\"launchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1323: toolbox execution JSON should expose launch commands");
    expect_contains(visual_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(COPPERFIN_TEST_SUCCESS_COMMAND, {}),
        "#1323: toolbox execution JSON should expose the shell command");
    expect_contains(visual_process.stdout_text, "\"observedExitCode\": 0",
        "#1323: successful toolbox execution JSON should expose zero exit status");
    expect_contains(visual_process.stdout_text, "\"executionAdmitted\": true",
        "#1323: toolbox execution JSON should expose execution admission");
    expect_contains(visual_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1323: toolbox execution JSON should expose dispatch admission");
    expect_contains(visual_process.stdout_text, "\"executed\": true",
        "#1323: toolbox execution JSON should mark execution complete");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1323: admitted toolbox execution JSON should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1323: toolbox execution JSON should remain non-mutating");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "3",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1323: toolbox execution JSON should accept admitted report contexts");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1323: report toolbox execution JSON should resolve report toolbox contexts");
    expect_contains(report_process.stdout_text, "\"id\": \"label\"",
        "#1323: report toolbox execution JSON should include report-safe Label items");
    expect_not_contains(report_process.stdout_text, "\"id\": \"textbox\"",
        "#1323: report toolbox execution JSON should exclude form-only TextBox items");
    expect_contains(report_process.stdout_text, "\"executionReadyItemIds\": [\"label\"",
        "#1390: report toolbox execution JSON should summarize execution-ready items");
    expect_contains(report_process.stdout_text, "\"executionBlockedItemIds\": []",
        "#1390: report toolbox execution JSON should summarize empty blocked item ids");
    expect_contains(report_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1390: report toolbox execution JSON should summarize empty execution errors");
    expect_contains(report_process.stdout_text, "\"executed\": true",
        "#1323: report toolbox execution JSON should mark execution complete");

    const auto missing_command_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--json"
        },
        temp_root);
    expect(missing_command_process.exit_code == 2,
        "#1323: toolbox execution JSON should reject missing launch commands");
    expect_contains(missing_command_process.stdout_text, "No toolbox launch command was provided.",
        "#1323: missing toolbox launch commands should report parser errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "false",
            "--admit-toolbox-execution", "true",
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 4,
        "#1323: toolbox execution JSON should reject dry-run dispatch requests");
    expect_contains(dry_run_process.stdout_text, "\"toolboxExecution\": null",
        "#1323: dry-run toolbox execution JSON should not expose a result object");
    expect_contains(dry_run_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1323: dry-run toolbox execution JSON should report dispatch admission errors");

    const auto unadmitted_execution_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "false",
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(unadmitted_execution_process.exit_code == 4,
        "#1323: toolbox execution JSON should require explicit execution admission");
    expect_contains(unadmitted_execution_process.stdout_text,
        "A toolbox dispatch execution request requires explicit execution admission.",
        "#1323: unadmitted toolbox execution JSON should report execution admission errors");
    expect_contains(unadmitted_execution_process.stdout_text, "\"executed\": false",
        "#1323: unadmitted toolbox execution JSON should not mark execution complete");

    const auto failed_command_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--toolbox-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(failed_command_process.exit_code == 4,
        "#1323: toolbox execution JSON should report nonzero process exits");
    expect_contains(failed_command_process.stdout_text,
        "Toolbox launch command returned a non-zero exit code.",
        "#1323: failed toolbox execution JSON should report process errors");
    expect_contains(failed_command_process.stdout_text, "\"observedExitCode\": 1",
        "#1347: failed toolbox execution JSON should report normalized child exit codes");
    expect_contains(failed_command_process.stdout_text, "\"executed\": false",
        "#1323: failed toolbox execution JSON should not mark execution complete");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "menu_item",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1323: toolbox execution JSON should reject unsupported selection contexts");
    expect_contains(unsupported_process.stdout_text, "\"toolboxExecution\": null",
        "#1323: unsupported toolbox execution JSON should not expose a result object");
    expect_contains(unsupported_process.stdout_text,
        "The selected Studio context does not expose a toolbox palette.",
        "#1323: unsupported toolbox execution JSON should report validation errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_toolbox_dispatch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_toolbox_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto form_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-catalog",
            "--toolbox-context", "form",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(form_process.exit_code == 0,
        "#1236: toolbox dispatch catalog JSON should accept admitted form contexts");
    expect_contains(form_process.stdout_text, "\"toolboxDispatchCatalog\": {",
        "#1236: toolbox dispatch catalog JSON should expose a catalog object");
    expect_contains(form_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1236: toolbox dispatch catalog JSON should expose toolbox contexts");
    expect_contains(form_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1236: toolbox dispatch catalog JSON should expose command tokens");
    expect_contains(form_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1236: toolbox dispatch catalog JSON should carry asset paths");
    expect_contains(form_process.stdout_text, "\"recordIndex\": 1",
        "#1236: toolbox dispatch catalog JSON should carry record indexes");
    expect_contains(form_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1236: toolbox dispatch catalog JSON should carry object-name selectors");
    expect_contains(form_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1236: toolbox dispatch catalog JSON should carry unique-id selectors");
    expect_contains(form_process.stdout_text, "\"id\": \"textbox\"",
        "#1236: form toolbox dispatch catalog JSON should include form-safe TextBox items");
    expect_contains(form_process.stdout_text, "\"dispatchCount\": 1",
        "#1236: admitted toolbox dispatch catalog JSON should expose dispatch counts");
    expect_contains(form_process.stdout_text, "\"errorCount\": 0",
        "#1236: admitted toolbox dispatch catalog JSON should expose zero error counts");
    expect_contains(form_process.stdout_text, "\"dryRun\": false",
        "#1236: admitted toolbox dispatch catalog JSON should not be dry-run");
    expect_contains(form_process.stdout_text, "\"mutatesAsset\": false",
        "#1236: toolbox dispatch catalog JSON should remain non-mutating");
    expect_contains(form_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\", \"textbox\"",
        "#1370: toolbox dispatch catalog JSON should summarize dispatch-ready items");
    expect_contains(form_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1370: admitted toolbox dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(form_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1370: admitted toolbox dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(form_process.stdout_text, "\"invocationAdmissionOk\": true",
        "#1236: toolbox dispatch catalog JSON should expose admission results");
    expect_contains(form_process.stdout_text, "\"paletteInvocationAdmitted\": true",
        "#1236: toolbox dispatch catalog JSON should expose palette admission state");
    expect_contains(form_process.stdout_text, "\"dispatchOk\": true",
        "#1236: toolbox dispatch catalog JSON should expose dispatch status");
    expect_contains(form_process.stdout_text, "\"dispatchArguments\": [",
        "#1236: toolbox dispatch catalog JSON should expose dispatch arguments");
    expect_contains(form_process.stdout_text, "\"--toolbox-context\"",
        "#1236: toolbox dispatch catalog JSON should expose toolbox-context arguments");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-catalog",
            "--toolbox-context", "report",
            "--path", "reports/orders.frx",
            "--record", "3",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1236: toolbox dispatch catalog JSON should accept admitted report contexts");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1236: report toolbox dispatch catalog JSON should expose report contexts");
    expect_contains(report_process.stdout_text, "\"id\": \"label\"",
        "#1236: report toolbox dispatch catalog JSON should include report-safe Label items");
    expect_not_contains(report_process.stdout_text, "\"id\": \"textbox\"",
        "#1236: report toolbox dispatch catalog JSON should exclude form-only TextBox items");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-catalog",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1236: dry-run toolbox dispatch catalog JSON should report aggregate errors without parse failure");
    expect_contains(dry_run_process.stdout_text, "\"dispatchCount\": 0",
        "#1236: dry-run toolbox dispatch catalog JSON should expose zero dispatch counts");
    expect_contains(dry_run_process.stdout_text, "\"errorCount\": 1",
        "#1236: dry-run toolbox dispatch catalog JSON should expose dispatch error counts");
    expect_contains(dry_run_process.stdout_text, "\"dryRun\": true",
        "#1236: dry-run toolbox dispatch catalog JSON should preserve dry-run state");
    expect_contains(dry_run_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1370: dry-run toolbox dispatch catalog JSON should summarize empty dispatch-ready items");
    expect_contains(dry_run_process.stdout_text, "\"dispatchBlockedItemIds\": [\"label\", \"textbox\"",
        "#1370: dry-run toolbox dispatch catalog JSON should summarize blocked item ids");
    expect_contains(dry_run_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox dispatch request requires an admitted non-dry-run invocation.\"",
        "#1370: dry-run toolbox dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_process.stdout_text, "\"paletteInvocationAdmitted\": false",
        "#1236: dry-run toolbox dispatch catalog JSON should expose unadmitted palette state");
    expect_contains(dry_run_process.stdout_text, "\"dispatchOk\": false",
        "#1236: dry-run toolbox dispatch catalog JSON should expose dispatch rejection");
    expect_contains(dry_run_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1236: dry-run toolbox dispatch catalog JSON should report dispatch errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-catalog",
            "--toolbox-context", "form",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1236: toolbox dispatch catalog JSON should reject invalid admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1236: invalid toolbox dispatch catalog boolean JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-catalog",
            "--toolbox-context", "form",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1236: toolbox dispatch catalog JSON should reject invalid record indexes");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1236: invalid toolbox dispatch catalog record JSON should report parser errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-catalog",
            "--toolbox-context", "menu",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1236: toolbox dispatch catalog JSON should reject unknown toolbox contexts");
    expect_contains(unknown_context_process.stdout_text, "Unknown toolbox context token: menu",
        "#1236: unknown toolbox dispatch catalog context JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1236: toolbox dispatch catalog JSON should reject missing toolbox contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1236: missing toolbox dispatch catalog context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_toolbox_dispatch_execution_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_dispatch_execution_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto admitted_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-execution-catalog",
            "--toolbox-context", "form",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--json"
        },
        temp_root);
    expect(admitted_process.exit_code == 0,
        "#1331: toolbox dispatch execution catalog JSON should accept admitted form catalogs");
    expect_contains(admitted_process.stdout_text, "\"toolboxDispatchExecutionCatalog\": {",
        "#1331: toolbox dispatch execution catalog JSON should expose a catalog object");
    expect_contains(admitted_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1331: toolbox dispatch execution catalog JSON should expose toolbox contexts");
    expect_contains(admitted_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1331: toolbox dispatch execution catalog JSON should expose command tokens");
    expect_contains(admitted_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1331: toolbox dispatch execution catalog JSON should carry asset paths");
    expect_contains(admitted_process.stdout_text, "\"recordIndex\": 1",
        "#1331: toolbox dispatch execution catalog JSON should carry record indexes");
    expect_contains(admitted_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1331: toolbox dispatch execution catalog JSON should carry object-name selectors");
    expect_contains(admitted_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1331: toolbox dispatch execution catalog JSON should carry unique-id selectors");
    expect_contains(admitted_process.stdout_text, "\"itemCount\": ",
        "#1331: toolbox dispatch execution catalog JSON should expose item counts");
    expect_contains(admitted_process.stdout_text, "\"executionReadyCount\": ",
        "#1331: toolbox dispatch execution catalog JSON should expose readiness counts");
    expect_contains(admitted_process.stdout_text, "\"errorCount\": 0",
        "#1331: admitted toolbox dispatch execution catalog JSON should expose zero errors");
    expect_contains(admitted_process.stdout_text, "\"dryRun\": false",
        "#1331: admitted toolbox dispatch execution catalog JSON should not be dry-run");
    expect_contains(admitted_process.stdout_text, "\"executionReadyItemIds\": [\"label\", \"textbox\"",
        "#1371: admitted toolbox dispatch execution catalog JSON should summarize ready item ids");
    expect_contains(admitted_process.stdout_text, "\"executionBlockedItemIds\": []",
        "#1371: admitted toolbox dispatch execution catalog JSON should summarize empty blocked item ids");
    expect_contains(admitted_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1371: admitted toolbox dispatch execution catalog JSON should summarize empty blocked errors");
    expect_contains(admitted_process.stdout_text, "\"id\": \"textbox\"",
        "#1331: toolbox dispatch execution catalog JSON should include per-item metadata");
    expect_contains(admitted_process.stdout_text, "\"entries\": [",
        "#1331: toolbox dispatch execution catalog JSON should expose per-item readiness entries");
    expect_contains(admitted_process.stdout_text, "\"executionAdmitted\": true",
        "#1331: toolbox dispatch execution catalog JSON should expose execution admission state");
    expect_contains(admitted_process.stdout_text, "\"executionReady\": true",
        "#1331: toolbox dispatch execution catalog JSON should expose execution readiness");
    expect_contains(admitted_process.stdout_text, "\"executionError\": \"\"",
        "#1331: admitted toolbox dispatch execution catalog JSON should expose empty execution errors");
    expect_contains(admitted_process.stdout_text, "\"paletteInvocationAdmitted\": true",
        "#1331: toolbox dispatch execution catalog JSON should expose palette admission state");
    expect_contains(admitted_process.stdout_text, "\"dispatchOk\": true",
        "#1331: toolbox dispatch execution catalog JSON should expose dispatch readiness");
    expect_contains(admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1331: toolbox dispatch execution catalog JSON should expose dispatch arguments");

    const auto unadmitted_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-execution-catalog",
            "--toolbox-context", "form",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "false",
            "--json"
        },
        temp_root);
    expect(unadmitted_process.exit_code == 0,
        "#1331: toolbox dispatch execution catalog JSON should report unadmitted execution as item errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionReadyCount\": 0",
        "#1331: unadmitted toolbox dispatch execution catalog JSON should expose zero readiness");
    expect_contains(unadmitted_process.stdout_text, "\"executionReadyItemIds\": []",
        "#1371: unadmitted toolbox dispatch execution catalog JSON should summarize empty ready item ids");
    expect_contains(unadmitted_process.stdout_text, "\"executionBlockedItemIds\": [\"label\", \"textbox\"",
        "#1371: unadmitted toolbox dispatch execution catalog JSON should summarize blocked item ids");
    expect_contains(unadmitted_process.stdout_text,
        "\"executionBlockedErrors\": [\"A toolbox dispatch execution catalog entry requires explicit execution admission.\"",
        "#1371: unadmitted toolbox dispatch execution catalog JSON should summarize blocked errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionAdmitted\": false",
        "#1331: unadmitted toolbox dispatch execution catalog JSON should expose admission false");
    expect_contains(unadmitted_process.stdout_text,
        "A toolbox dispatch execution catalog entry requires explicit execution admission.",
        "#1331: unadmitted toolbox dispatch execution catalog JSON should expose execution errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-execution-catalog",
            "--toolbox-context", "form",
            "--admit-toolbox-execution", "true",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1331: toolbox dispatch execution catalog JSON should report dry-run dispatch failures");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyCount\": 0",
        "#1331: dry-run toolbox dispatch execution catalog JSON should expose zero readiness");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyItemIds\": []",
        "#1371: dry-run toolbox dispatch execution catalog JSON should summarize empty ready item ids");
    expect_contains(dry_run_process.stdout_text, "\"executionBlockedItemIds\": [\"label\", \"textbox\"",
        "#1371: dry-run toolbox dispatch execution catalog JSON should summarize blocked item ids");
    expect_contains(dry_run_process.stdout_text,
        "\"executionBlockedErrors\": [\"A toolbox dispatch request requires an admitted non-dry-run invocation.\"",
        "#1371: dry-run toolbox dispatch execution catalog JSON should summarize dispatch-blocked errors");
    expect_contains(dry_run_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1331: dry-run toolbox dispatch execution catalog JSON should expose dispatch errors");

    const auto invalid_execution_bool_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-execution-catalog",
            "--toolbox-context", "form",
            "--admit-toolbox-execution", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_execution_bool_process.exit_code == 2,
        "#1331: toolbox dispatch execution catalog JSON should reject invalid execution booleans");
    expect_contains(invalid_execution_bool_process.stdout_text,
        "The --admit-toolbox-execution value must be true or false.",
        "#1331: invalid toolbox execution catalog admission JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch-execution-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1331: toolbox dispatch execution catalog JSON should reject missing toolbox contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1331: missing toolbox dispatch execution catalog context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selection_toolbox_dispatch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1291: selection toolbox dispatch catalog JSON should accept admitted visual contexts");
    expect_contains(visual_process.stdout_text, "\"selectionToolboxDispatchCatalog\": {",
        "#1291: selection toolbox dispatch catalog JSON should expose catalog objects");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1291: selection toolbox dispatch catalog JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1291: visual selection toolbox dispatch catalog JSON should resolve form contexts");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1291: selection toolbox dispatch catalog JSON should expose command tokens");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1291: selection toolbox dispatch catalog JSON should carry asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1291: selection toolbox dispatch catalog JSON should carry record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1291: selection toolbox dispatch catalog JSON should carry object-name selectors");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1291: selection toolbox dispatch catalog JSON should carry unique-id selectors");
    expect_contains(visual_process.stdout_text, "\"id\": \"textbox\"",
        "#1291: visual selection toolbox dispatch catalog JSON should include form-safe TextBox items");
    expect_contains(visual_process.stdout_text, "\"launchPlanOk\": true",
        "#1291: selection toolbox dispatch catalog JSON should expose launch-plan state");
    expect_contains(visual_process.stdout_text, "\"invocationAdmissionOk\": true",
        "#1291: selection toolbox dispatch catalog JSON should expose admission state");
    expect_contains(visual_process.stdout_text, "\"paletteInvocationAdmitted\": true",
        "#1291: admitted selection toolbox dispatch catalog JSON should expose palette admission state");
    expect_contains(visual_process.stdout_text, "\"dispatchCount\": 1",
        "#1291: admitted selection toolbox dispatch catalog JSON should expose dispatch counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1291: admitted selection toolbox dispatch catalog JSON should expose zero errors");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1291: admitted selection toolbox dispatch catalog JSON should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1291: selection toolbox dispatch catalog JSON should remain non-mutating");
    expect_contains(visual_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\", \"textbox\"",
        "#1372: selection toolbox dispatch catalog JSON should summarize dispatch-ready items");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1372: admitted selection toolbox dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1372: admitted selection toolbox dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(visual_process.stdout_text, "\"dispatchOk\": true",
        "#1291: admitted selection toolbox dispatch catalog JSON should expose dispatch status");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [",
        "#1291: selection toolbox dispatch catalog JSON should expose dispatch arguments");
    expect_contains(visual_process.stdout_text, "\"--selection-context\"",
        "#1291: selection toolbox dispatch catalog JSON should expose selection-context arguments");
    expect_contains(visual_process.stdout_text, "\"visual_object\"",
        "#1291: selection toolbox dispatch catalog JSON should expose selection-context values");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-catalog",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "3",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1291: selection toolbox dispatch catalog JSON should accept admitted report contexts");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1291: report selection toolbox dispatch catalog JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1291: report selection toolbox dispatch catalog JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"id\": \"label\"",
        "#1291: report selection toolbox dispatch catalog JSON should include report-safe Label items");
    expect_not_contains(report_process.stdout_text, "\"id\": \"textbox\"",
        "#1291: report selection toolbox dispatch catalog JSON should exclude form-only TextBox items");
    expect_contains(report_process.stdout_text, "\"dispatchCount\": 1",
        "#1291: report selection toolbox dispatch catalog JSON should expose dispatch counts");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-catalog",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1291: dry-run selection toolbox dispatch catalog JSON should report aggregate errors");
    expect_contains(dry_run_process.stdout_text, "\"dispatchCount\": 0",
        "#1291: dry-run selection toolbox dispatch catalog JSON should expose zero dispatch counts");
    expect_contains(dry_run_process.stdout_text, "\"errorCount\": 1",
        "#1291: dry-run selection toolbox dispatch catalog JSON should expose dispatch error counts");
    expect_contains(dry_run_process.stdout_text, "\"dryRun\": true",
        "#1291: dry-run selection toolbox dispatch catalog JSON should preserve dry-run state");
    expect_contains(dry_run_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1372: dry-run selection toolbox dispatch catalog JSON should summarize empty dispatch-ready items");
    expect_contains(dry_run_process.stdout_text, "\"dispatchBlockedItemIds\": [\"label\", \"textbox\"",
        "#1372: dry-run selection toolbox dispatch catalog JSON should summarize blocked item ids");
    expect_contains(dry_run_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox dispatch request requires an admitted non-dry-run invocation.\"",
        "#1372: dry-run selection toolbox dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_process.stdout_text, "\"paletteInvocationAdmitted\": false",
        "#1291: dry-run selection toolbox dispatch catalog JSON should expose unadmitted palette state");
    expect_contains(dry_run_process.stdout_text, "\"dispatchOk\": false",
        "#1291: dry-run selection toolbox dispatch catalog JSON should expose dispatch rejection");
    expect_contains(dry_run_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1291: dry-run selection toolbox dispatch catalog JSON should report dispatch errors");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-catalog",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1291: selection toolbox dispatch catalog JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionToolboxDispatchCatalog\": null",
        "#1291: unsupported selection toolbox dispatch catalog JSON should omit catalog objects");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox dispatch catalog request requires a toolbox palette.",
        "#1291: unsupported selection toolbox dispatch catalog JSON should report planner errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-catalog",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1291: selection toolbox dispatch catalog JSON should reject invalid booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1291: invalid selection toolbox dispatch catalog boolean JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-catalog",
            "--selection-context", "visual_object",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1291: selection toolbox dispatch catalog JSON should reject invalid records");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1291: invalid selection toolbox dispatch catalog record JSON should report parser errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1291: selection toolbox dispatch catalog JSON should reject unknown selections");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1291: unknown selection toolbox dispatch catalog JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1291: selection toolbox dispatch catalog JSON should reject missing selections");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1291: missing selection toolbox dispatch catalog JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selection_toolbox_dispatch_execution_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_dispatch_execution_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-execution-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1408: selection toolbox dispatch execution catalog JSON should accept admitted visual contexts");
    expect_contains(visual_process.stdout_text, "\"selectionToolboxDispatchExecutionCatalog\": {",
        "#1408: selection toolbox dispatch execution catalog JSON should expose catalog objects");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1408: selection toolbox dispatch execution catalog JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1408: visual selection toolbox dispatch execution catalog JSON should resolve form contexts");
    expect_contains(visual_process.stdout_text, "\"executionReadyCount\": 14",
        "#1408: admitted visual selection toolbox dispatch execution catalog JSON should expose ready counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1408: admitted visual selection toolbox dispatch execution catalog JSON should expose zero errors");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1408: admitted visual selection toolbox dispatch execution catalog JSON should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1408: selection toolbox dispatch execution catalog JSON should remain non-mutating");
    expect_contains(visual_process.stdout_text, "\"executionReadyItemIds\": [\"label\", \"textbox\"",
        "#1408: selection toolbox dispatch execution catalog JSON should summarize execution-ready items");
    expect_contains(visual_process.stdout_text, "\"executionBlockedItemIds\": []",
        "#1408: admitted selection toolbox dispatch execution catalog JSON should summarize empty blocked ids");
    expect_contains(visual_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1408: admitted selection toolbox dispatch execution catalog JSON should summarize empty blocked errors");
    expect_contains(visual_process.stdout_text, "\"executionAdmitted\": true",
        "#1408: selection toolbox dispatch execution catalog entries should expose execution admission");
    expect_contains(visual_process.stdout_text, "\"executionReady\": true",
        "#1408: selection toolbox dispatch execution catalog entries should expose execution readiness");
    expect_contains(visual_process.stdout_text, "\"dispatchOk\": true",
        "#1408: selection toolbox dispatch execution catalog JSON should expose dispatch status");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [",
        "#1408: selection toolbox dispatch execution catalog JSON should preserve dispatch arguments");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-execution-catalog",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1408: dry-run selection toolbox dispatch execution catalog JSON should report aggregate errors");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyCount\": 0",
        "#1408: dry-run selection toolbox dispatch execution catalog JSON should expose zero ready count");
    expect_contains(dry_run_process.stdout_text, "\"errorCount\": 14",
        "#1408: dry-run selection toolbox dispatch execution catalog JSON should expose item error counts");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyItemIds\": []",
        "#1408: dry-run selection toolbox dispatch execution catalog JSON should summarize empty ready items");
    expect_contains(dry_run_process.stdout_text, "\"executionBlockedItemIds\": [\"label\", \"textbox\"",
        "#1408: dry-run selection toolbox dispatch execution catalog JSON should summarize blocked item ids");
    expect_contains(dry_run_process.stdout_text,
        "\"executionBlockedErrors\": [\"A toolbox dispatch request requires an admitted non-dry-run invocation.\"",
        "#1408: dry-run selection toolbox dispatch execution catalog JSON should summarize blocked errors");
    expect_contains(dry_run_process.stdout_text, "\"dispatchOk\": false",
        "#1408: dry-run selection toolbox dispatch execution catalog JSON should preserve dispatch rejection");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-dispatch-execution-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1408: selection toolbox dispatch execution catalog JSON should reject missing selections");
    expect_contains(missing_context_process.stdout_text, "\"selectionToolboxDispatchExecutionCatalog\": null",
        "#1408: missing selection toolbox dispatch execution catalog JSON should omit catalog objects");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1408: missing selection toolbox dispatch execution catalog JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
