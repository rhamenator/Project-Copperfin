// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_editor_action_launch_plans(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_editor_action_launch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto method_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-plan", "edit-visual-method",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "cmdSave",
            "--unique-id", "button-guid",
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--json"
        },
        temp_root);
    expect(method_process.exit_code == 0,
        "#1208: editor action launch-plan JSON should accept context-valid method editor actions");
    expect_contains(method_process.stdout_text, "\"editorActionLaunchPlan\": {",
        "#1208: editor action launch-plan JSON should expose a plan object");
    expect_contains(method_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1208: editor action launch-plan JSON should expose action ids");
    expect_contains(method_process.stdout_text, "\"kind\": \"source_editor\"",
        "#1208: editor action launch-plan JSON should expose action kind metadata");
    expect_contains(method_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1208: editor action launch-plan JSON should expose selected Studio contexts");
    expect_contains(method_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1208: editor action launch-plan JSON should expose command tokens");
    expect_contains(method_process.stdout_text, "\"targetSurface\": \"method-editor\"",
        "#1208: editor action launch-plan JSON should expose target surfaces");
    expect_contains(method_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1208: editor action launch-plan JSON should carry asset paths");
    expect_contains(method_process.stdout_text, "\"recordIndex\": 4",
        "#1208: editor action launch-plan JSON should carry record indexes");
    expect_contains(method_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1208: editor action launch-plan JSON should carry object-name selectors");
    expect_contains(method_process.stdout_text, "\"uniqueId\": \"button-guid\"",
        "#1208: editor action launch-plan JSON should carry unique-id selectors");
    expect_contains(method_process.stdout_text, "\"symbol\": \"cmdSave.Click\"",
        "#1208: editor action launch-plan JSON should carry launch symbols");
    expect_contains(method_process.stdout_text, "\"line\": 42",
        "#1208: editor action launch-plan JSON should carry line metadata");
    expect_contains(method_process.stdout_text, "\"column\": 7",
        "#1208: editor action launch-plan JSON should carry column metadata");
    expect_contains(method_process.stdout_text, "\"launchReadyActionIds\": [\"edit-visual-method\"]",
        "#1395: editor action launch-plan JSON should summarize launch-ready action ids");
    expect_contains(method_process.stdout_text, "\"launchBlockedActionIds\": []",
        "#1395: editor action launch-plan JSON should summarize empty blocked action ids");
    expect_contains(method_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1395: editor action launch-plan JSON should summarize empty launch errors");

    const auto expression_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-plan", "edit-report-expression",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "2",
            "--symbol", "Expr1.Expression",
            "--json"
        },
        temp_root);
    expect(expression_process.exit_code == 0,
        "#1208: editor action launch-plan JSON should accept report expression actions");
    expect_contains(expression_process.stdout_text, "\"kind\": \"expression_editor\"",
        "#1208: expression editor launch-plan JSON should expose expression-editor metadata");
    expect_contains(expression_process.stdout_text, "\"targetSurface\": \"expression-editor\"",
        "#1208: expression editor launch-plan JSON should expose expression editor target surfaces");
    expect_contains(expression_process.stdout_text, "\"launchReadyActionIds\": [\"edit-report-expression\"]",
        "#1395: expression editor launch-plan JSON should summarize launch-ready action ids");
    expect_contains(expression_process.stdout_text, "\"launchBlockedActionIds\": []",
        "#1395: expression editor launch-plan JSON should summarize empty blocked action ids");
    expect_contains(expression_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1395: expression editor launch-plan JSON should summarize empty launch errors");

    const auto data_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-plan", "edit-data-environment",
            "--selection-context", "data_environment",
            "--path", "forms/customer.scx",
            "--record", "0",
            "--object-name", "Dataenvironment",
            "--symbol", "Dataenvironment.OpenTables",
            "--json"
        },
        temp_root);
    expect(data_process.exit_code == 0,
        "#1208: editor action launch-plan JSON should accept data-environment actions");
    expect_contains(data_process.stdout_text, "\"commandToken\": \"studio.data_environment.open\"",
        "#1208: data-environment editor launch-plan JSON should expose command tokens");

    const auto project_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-plan", "navigate-project-item",
            "--selection-context", "project_item",
            "--path", "apps/customer.pjx",
            "--record", "5",
            "--symbol", "forms/customer.scx",
            "--json"
        },
        temp_root);
    expect(project_process.exit_code == 0,
        "#1208: editor action launch-plan JSON should accept project navigation actions");
    expect_contains(project_process.stdout_text, "\"kind\": \"navigator\"",
        "#1208: project navigation launch-plan JSON should expose navigator metadata");

    const auto wrong_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-plan", "edit-report-expression",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(wrong_context_process.exit_code == 4,
        "#1208: editor action launch-plan JSON should reject wrong-context actions");
    expect_contains(wrong_context_process.stdout_text,
        "The requested editor action is not available for the selected Studio context.",
        "#1208: wrong-context editor action launch-plan JSON should report validation errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-plan", "show-property-grid",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1208: editor action launch-plan JSON should reject unknown selection tokens");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1208: unknown-context editor action launch-plan JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-plan", "show-property-grid",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1208: editor action launch-plan JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1208: missing-context editor action launch-plan JSON should report parser errors");

    const auto missing_id_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-plan",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_id_process.exit_code == 2,
        "#1208: editor action launch-plan JSON should reject missing action ids");
    expect_contains(missing_id_process.stdout_text, "Missing value for --editor-action-launch-plan.",
        "#1208: missing-id editor action launch-plan JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-plan", "show-property-grid",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1208: editor action launch-plan JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1208: invalid-line editor action launch-plan JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_editor_action_launch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_editor_action_launch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "cmdSave",
            "--unique-id", "button-guid",
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1280: editor action launch catalog JSON should accept visual-object catalogs");
    expect_contains(visual_process.stdout_text, "\"editorActionLaunchCatalog\": {",
        "#1280: editor action launch catalog JSON should expose a catalog object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1280: editor action launch catalog JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"actionCount\": 5",
        "#1280: visual-object launch catalog JSON should expose action counts");
    expect_contains(visual_process.stdout_text, "\"launchPlanCount\": 5",
        "#1280: visual-object launch catalog JSON should expose launch-plan counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1280: visual-object launch catalog JSON should expose error counts");
    expect_contains(visual_process.stdout_text, "\"dryRun\": true",
        "#1280: editor action launch catalog JSON should stay dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1280: editor action launch catalog JSON should remain non-mutating");
    expect_contains(visual_process.stdout_text, "\"launchReadyActionIds\": [\"show-property-grid\"",
        "#1364: editor action launch catalog JSON should summarize launch-ready actions");
    expect_contains(visual_process.stdout_text, "\"launchBlockedActionIds\": []",
        "#1364: editor action launch catalog JSON should summarize empty blocked action ids");
    expect_contains(visual_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1364: editor action launch catalog JSON should summarize empty blocked launch errors");
    expect_contains(visual_process.stdout_text, "\"entries\": [",
        "#1280: editor action launch catalog JSON should expose per-action entries");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1280: editor action launch catalog JSON should include method actions");
    expect_contains(visual_process.stdout_text, "\"kind\": \"source_editor\"",
        "#1280: editor action launch catalog JSON should expose action kind metadata");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1280: editor action launch catalog JSON should expose command tokens");
    expect_contains(visual_process.stdout_text, "\"targetSurface\": \"method-editor\"",
        "#1280: editor action launch catalog JSON should expose target surfaces");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1280: editor action launch catalog JSON should carry asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 4",
        "#1280: editor action launch catalog JSON should carry record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1280: editor action launch catalog JSON should carry object-name selectors");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"button-guid\"",
        "#1280: editor action launch catalog JSON should carry unique-id selectors");
    expect_contains(visual_process.stdout_text, "\"symbol\": \"cmdSave.Click\"",
        "#1280: editor action launch catalog JSON should carry launch symbols");
    expect_contains(visual_process.stdout_text, "\"line\": 42",
        "#1280: editor action launch catalog JSON should carry line metadata");
    expect_contains(visual_process.stdout_text, "\"column\": 7",
        "#1280: editor action launch catalog JSON should carry column metadata");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "2",
            "--symbol", "Expr1.Expression",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1280: editor action launch catalog JSON should accept report-expression catalogs");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1280: report-expression launch catalog JSON should expose selected contexts");
    expect_contains(report_process.stdout_text, "\"actionId\": \"edit-report-expression\"",
        "#1280: report-expression launch catalog JSON should include expression editor actions");
    expect_contains(report_process.stdout_text, "\"kind\": \"expression_editor\"",
        "#1280: report-expression launch catalog JSON should expose expression-editor metadata");
    expect_contains(report_process.stdout_text, "\"targetSurface\": \"expression-editor\"",
        "#1280: report-expression launch catalog JSON should expose expression editor target surfaces");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1280: editor action launch catalog JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "\"editorActionLaunchCatalog\": null",
        "#1280: invalid editor action launch catalog JSON should not expose catalog objects");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1280: invalid editor action launch catalog contexts should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1280: editor action launch catalog JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1280: missing-context editor action launch catalog JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1280: editor action launch catalog JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1280: invalid-line editor action launch catalog JSON should report parser errors");

    const auto invalid_column_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--selection-context", "visual_object",
            "--column", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_column_process.exit_code == 2,
        "#1280: editor action launch catalog JSON should reject invalid column values");
    expect_contains(invalid_column_process.stdout_text, "The --column value must be a non-negative integer.",
        "#1280: invalid-column editor action launch catalog JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_editor_action_invocation_admission(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_editor_action_invocation_admission_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto method_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "edit-visual-method",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "cmdSave",
            "--unique-id", "button-guid",
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);
    expect(method_process.exit_code == 0,
        "#1218: editor action invocation-admission JSON should accept admitted method editor actions");
    expect_contains(method_process.stdout_text, "\"editorActionInvocationAdmission\": {",
        "#1218: editor action invocation-admission JSON should expose a plan object");
    expect_contains(method_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1218: editor action invocation-admission JSON should expose action ids");
    expect_contains(method_process.stdout_text, "\"kind\": \"source_editor\"",
        "#1218: editor action invocation-admission JSON should expose action kind metadata");
    expect_contains(method_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1218: editor action invocation-admission JSON should expose selected Studio contexts");
    expect_contains(method_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1218: editor action invocation-admission JSON should expose command tokens");
    expect_contains(method_process.stdout_text, "\"targetSurface\": \"method-editor\"",
        "#1218: editor action invocation-admission JSON should expose target surfaces");
    expect_contains(method_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1218: editor action invocation-admission JSON should carry asset paths");
    expect_contains(method_process.stdout_text, "\"recordIndex\": 4",
        "#1218: editor action invocation-admission JSON should carry record indexes");
    expect_contains(method_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1218: editor action invocation-admission JSON should carry object-name selectors");
    expect_contains(method_process.stdout_text, "\"uniqueId\": \"button-guid\"",
        "#1218: editor action invocation-admission JSON should carry unique-id selectors");
    expect_contains(method_process.stdout_text, "\"symbol\": \"cmdSave.Click\"",
        "#1218: editor action invocation-admission JSON should carry launch symbols");
    expect_contains(method_process.stdout_text, "\"line\": 42",
        "#1218: editor action invocation-admission JSON should carry line metadata");
    expect_contains(method_process.stdout_text, "\"column\": 7",
        "#1218: editor action invocation-admission JSON should carry column metadata");
    expect_contains(method_process.stdout_text, "\"admissionReadyActionIds\": [\"edit-visual-method\"]",
        "#1396: editor action invocation-admission JSON should summarize admission-ready action ids");
    expect_contains(method_process.stdout_text, "\"admissionBlockedActionIds\": []",
        "#1396: editor action invocation-admission JSON should summarize empty blocked action ids");
    expect_contains(method_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1396: editor action invocation-admission JSON should summarize empty admission errors");
    expect_contains(method_process.stdout_text, "\"editorInvocationAdmitted\": true",
        "#1218: admitted editor action invocation-admission JSON should expose admitted state");
    expect_contains(method_process.stdout_text, "\"dryRun\": false",
        "#1218: admitted editor action invocation-admission JSON should not be marked dry-run");
    expect_contains(method_process.stdout_text, "\"mutatesAsset\": false",
        "#1218: editor action invocation-admission JSON should remain non-mutating");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "show-property-grid",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1218: editor action invocation-admission JSON should default to dry-run admission");
    expect_contains(dry_run_process.stdout_text, "\"actionId\": \"show-property-grid\"",
        "#1218: dry-run editor action invocation-admission JSON should expose action ids");
    expect_contains(dry_run_process.stdout_text, "\"admissionReadyActionIds\": [\"show-property-grid\"]",
        "#1396: dry-run editor action invocation-admission JSON should summarize admission-ready action ids");
    expect_contains(dry_run_process.stdout_text, "\"admissionBlockedActionIds\": []",
        "#1396: dry-run editor action invocation-admission JSON should summarize empty blocked action ids");
    expect_contains(dry_run_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1396: dry-run editor action invocation-admission JSON should summarize empty admission errors");
    expect_contains(dry_run_process.stdout_text, "\"editorInvocationAdmitted\": false",
        "#1218: default editor action invocation-admission JSON should not admit invocation");
    expect_contains(dry_run_process.stdout_text, "\"dryRun\": true",
        "#1218: default editor action invocation-admission JSON should expose dry-run state");

    const auto wrong_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "edit-report-expression",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(wrong_context_process.exit_code == 4,
        "#1218: editor action invocation-admission JSON should reject wrong-context actions");
    expect_contains(wrong_context_process.stdout_text, "\"editorActionInvocationAdmission\": null",
        "#1218: wrong-context editor action invocation-admission JSON should expose null plans");
    expect_contains(wrong_context_process.stdout_text,
        "The requested editor action is not available for the selected Studio context.",
        "#1218: wrong-context editor action invocation-admission JSON should report validation errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1218: editor action invocation-admission JSON should reject invalid admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-editor-invocation value must be true or false.",
        "#1218: invalid editor action invocation-admission boolean JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "edit-visual-method",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1218: editor action invocation-admission JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1218: invalid-line editor action invocation-admission JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "edit-visual-method",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1218: editor action invocation-admission JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1218: missing-context editor action invocation-admission JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_editor_action_invocation_admission_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_editor_action_invocation_admission_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "cmdSave",
            "--unique-id", "button-guid",
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1282: editor action invocation-admission catalog JSON should accept admitted visual-object catalogs");
    expect_contains(visual_process.stdout_text, "\"editorActionInvocationAdmissionCatalog\": {",
        "#1282: editor action invocation-admission catalog JSON should expose a catalog object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1282: editor action invocation-admission catalog JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"actionCount\": 5",
        "#1282: visual-object invocation-admission catalog JSON should expose action counts");
    expect_contains(visual_process.stdout_text, "\"admissionCount\": 5",
        "#1282: admitted visual-object invocation-admission catalog JSON should expose admission counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1282: admitted visual-object invocation-admission catalog JSON should expose error counts");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1282: admitted visual-object invocation-admission catalog JSON should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1282: editor action invocation-admission catalog JSON should remain non-mutating");
    expect_contains(visual_process.stdout_text, "\"admissionReadyActionIds\": [\"show-property-grid\"",
        "#1365: editor action invocation-admission catalog JSON should summarize admission-ready actions");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedActionIds\": []",
        "#1365: editor action invocation-admission catalog JSON should summarize empty blocked action ids");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1365: editor action invocation-admission catalog JSON should summarize empty blocked admission errors");
    expect_contains(visual_process.stdout_text, "\"entries\": [",
        "#1282: editor action invocation-admission catalog JSON should expose per-action entries");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1282: editor action invocation-admission catalog JSON should include method actions");
    expect_contains(visual_process.stdout_text, "\"kind\": \"source_editor\"",
        "#1282: editor action invocation-admission catalog JSON should expose action kind metadata");
    expect_contains(visual_process.stdout_text, "\"launchOk\": true",
        "#1282: editor action invocation-admission catalog JSON should expose launch status");
    expect_contains(visual_process.stdout_text, "\"invocationAdmissionOk\": true",
        "#1282: editor action invocation-admission catalog JSON should expose admission status");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1282: editor action invocation-admission catalog JSON should expose command tokens");
    expect_contains(visual_process.stdout_text, "\"targetSurface\": \"method-editor\"",
        "#1282: editor action invocation-admission catalog JSON should expose target surfaces");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1282: editor action invocation-admission catalog JSON should carry asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 4",
        "#1282: editor action invocation-admission catalog JSON should carry record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1282: editor action invocation-admission catalog JSON should carry object-name selectors");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"button-guid\"",
        "#1282: editor action invocation-admission catalog JSON should carry unique-id selectors");
    expect_contains(visual_process.stdout_text, "\"symbol\": \"cmdSave.Click\"",
        "#1282: editor action invocation-admission catalog JSON should carry launch symbols");
    expect_contains(visual_process.stdout_text, "\"line\": 42",
        "#1282: editor action invocation-admission catalog JSON should carry line metadata");
    expect_contains(visual_process.stdout_text, "\"column\": 7",
        "#1282: editor action invocation-admission catalog JSON should carry column metadata");
    expect_contains(visual_process.stdout_text, "\"editorInvocationAdmitted\": true",
        "#1282: editor action invocation-admission catalog JSON should expose admitted state");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission-catalog",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "2",
            "--symbol", "Expr1.Expression",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1282: editor action invocation-admission catalog JSON should accept report-expression catalogs");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1282: report-expression invocation-admission catalog JSON should expose selected contexts");
    expect_contains(report_process.stdout_text, "\"actionId\": \"edit-report-expression\"",
        "#1282: report-expression invocation-admission catalog JSON should include expression editor actions");
    expect_contains(report_process.stdout_text, "\"kind\": \"expression_editor\"",
        "#1282: report-expression invocation-admission catalog JSON should expose expression-editor metadata");
    expect_contains(report_process.stdout_text, "\"targetSurface\": \"expression-editor\"",
        "#1282: report-expression invocation-admission catalog JSON should expose expression editor target surfaces");
    expect_contains(report_process.stdout_text, "\"editorInvocationAdmitted\": false",
        "#1282: report-expression invocation-admission catalog JSON should default to dry-run admission");
    expect_contains(report_process.stdout_text, "\"dryRun\": true",
        "#1282: report-expression invocation-admission catalog JSON should expose dry-run state");
    expect_contains(report_process.stdout_text, "\"admissionReadyActionIds\": [\"show-property-grid\"",
        "#1365: dry-run editor action invocation-admission catalog JSON should preserve admission-ready actions");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1282: editor action invocation-admission catalog JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "\"editorActionInvocationAdmissionCatalog\": null",
        "#1282: invalid editor action invocation-admission catalog JSON should not expose catalog objects");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1282: invalid editor action invocation-admission catalog contexts should report parser errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission-catalog",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1282: editor action invocation-admission catalog JSON should reject invalid admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-editor-invocation value must be true or false.",
        "#1282: invalid editor action invocation-admission catalog booleans should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1282: editor action invocation-admission catalog JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1282: missing-context editor action invocation-admission catalog JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission-catalog",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1282: editor action invocation-admission catalog JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1282: invalid-line editor action invocation-admission catalog JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_editor_action_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_editor_action_dispatch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto method_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch", "edit-visual-method",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "cmdSave",
            "--unique-id", "button-guid",
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);
    expect(method_process.exit_code == 0,
        "#1226: editor action dispatch JSON should accept admitted method editor actions");
    expect_contains(method_process.stdout_text, "\"editorActionDispatch\": {",
        "#1226: editor action dispatch JSON should expose a dispatch object");
    expect_contains(method_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1226: editor action dispatch JSON should expose action ids");
    expect_contains(method_process.stdout_text, "\"kind\": \"source_editor\"",
        "#1226: editor action dispatch JSON should expose action kind metadata");
    expect_contains(method_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1226: editor action dispatch JSON should expose selected Studio contexts");
    expect_contains(method_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1226: editor action dispatch JSON should expose command tokens");
    expect_contains(method_process.stdout_text, "\"targetSurface\": \"method-editor\"",
        "#1226: editor action dispatch JSON should expose target surfaces");
    expect_contains(method_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1226: editor action dispatch JSON should carry asset paths");
    expect_contains(method_process.stdout_text, "\"recordIndex\": 4",
        "#1226: editor action dispatch JSON should carry record indexes");
    expect_contains(method_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1226: editor action dispatch JSON should carry object-name selectors");
    expect_contains(method_process.stdout_text, "\"uniqueId\": \"button-guid\"",
        "#1226: editor action dispatch JSON should carry unique-id selectors");
    expect_contains(method_process.stdout_text, "\"symbol\": \"cmdSave.Click\"",
        "#1226: editor action dispatch JSON should carry launch symbols");
    expect_contains(method_process.stdout_text, "\"line\": 42",
        "#1226: editor action dispatch JSON should carry line metadata");
    expect_contains(method_process.stdout_text, "\"column\": 7",
        "#1226: editor action dispatch JSON should carry column metadata");
    expect_contains(method_process.stdout_text, "\"dispatchReadyActionIds\": [\"edit-visual-method\"]",
        "#1397: editor action dispatch JSON should summarize dispatch-ready action ids");
    expect_contains(method_process.stdout_text, "\"dispatchBlockedActionIds\": []",
        "#1397: editor action dispatch JSON should summarize empty blocked action ids");
    expect_contains(method_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1397: editor action dispatch JSON should summarize empty dispatch errors");
    expect_contains(method_process.stdout_text, "\"dispatchArguments\": [",
        "#1226: editor action dispatch JSON should expose dispatch arguments");
    expect_contains(method_process.stdout_text, "\"--command-token\"",
        "#1226: editor action dispatch JSON should expose command-token arguments");
    expect_contains(method_process.stdout_text, "\"studio.method_editor.open\"",
        "#1226: editor action dispatch JSON should expose command-token values");
    expect_contains(method_process.stdout_text, "\"--action-id\"",
        "#1226: editor action dispatch JSON should expose action-id arguments");
    expect_contains(method_process.stdout_text, "\"edit-visual-method\"",
        "#1226: editor action dispatch JSON should expose action-id values");
    expect_contains(method_process.stdout_text, "\"--selection-context\"",
        "#1226: editor action dispatch JSON should expose selection-context arguments");
    expect_contains(method_process.stdout_text, "\"visual_object\"",
        "#1226: editor action dispatch JSON should expose selection-context values");
    expect_contains(method_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1226: editor action dispatch JSON should expose admitted dispatch state");
    expect_contains(method_process.stdout_text, "\"dryRun\": false",
        "#1226: editor action dispatch JSON should not be dry-run when admitted");
    expect_contains(method_process.stdout_text, "\"executed\": false",
        "#1226: editor action dispatch JSON should not execute editor processes");
    expect_contains(method_process.stdout_text, "\"mutatesAsset\": false",
        "#1226: editor action dispatch JSON should remain non-mutating");

    const auto expression_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch", "edit-report-expression",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "2",
            "--symbol", "Expr1.Expression",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);
    expect(expression_process.exit_code == 0,
        "#1226: editor action dispatch JSON should accept admitted report expression actions");
    expect_contains(expression_process.stdout_text, "\"kind\": \"expression_editor\"",
        "#1226: expression editor dispatch JSON should expose expression-editor metadata");
    expect_contains(expression_process.stdout_text, "\"targetSurface\": \"expression-editor\"",
        "#1226: expression editor dispatch JSON should expose expression editor target surfaces");
    expect_contains(expression_process.stdout_text, "\"dispatchReadyActionIds\": [\"edit-report-expression\"]",
        "#1397: expression editor dispatch JSON should summarize dispatch-ready action ids");
    expect_contains(expression_process.stdout_text, "\"dispatchBlockedActionIds\": []",
        "#1397: expression editor dispatch JSON should summarize empty blocked action ids");
    expect_contains(expression_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1397: expression editor dispatch JSON should summarize empty dispatch errors");
    expect_contains(expression_process.stdout_text, "\"--selection-context\"",
        "#1226: expression editor dispatch JSON should expose selection-context arguments");
    expect_contains(expression_process.stdout_text, "\"report_expression\"",
        "#1226: expression editor dispatch JSON should expose selection-context values");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch", "show-property-grid",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 4,
        "#1226: editor action dispatch JSON should reject unadmitted dry-run dispatch requests");
    expect_contains(dry_run_process.stdout_text, "\"editorActionDispatch\": null",
        "#1226: dry-run editor action dispatch JSON should expose null plans");
    expect_contains(dry_run_process.stdout_text,
        "An editor action dispatch request requires an admitted non-dry-run invocation.",
        "#1226: dry-run editor action dispatch JSON should report admission errors");

    const auto wrong_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch", "edit-report-expression",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);
    expect(wrong_context_process.exit_code == 4,
        "#1226: editor action dispatch JSON should reject wrong-context actions");
    expect_contains(wrong_context_process.stdout_text, "\"editorActionDispatch\": null",
        "#1226: wrong-context editor action dispatch JSON should expose null plans");
    expect_contains(wrong_context_process.stdout_text,
        "The requested editor action is not available for the selected Studio context.",
        "#1226: wrong-context editor action dispatch JSON should report validation errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1226: editor action dispatch JSON should reject invalid admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-editor-invocation value must be true or false.",
        "#1226: invalid editor action dispatch boolean JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch", "edit-visual-method",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1226: editor action dispatch JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1226: missing-context editor action dispatch JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_editor_action_execution(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_editor_action_execution_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto method_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-execute", "edit-visual-method",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "cmdSave",
            "--unique-id", "button-guid",
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--admit-editor-invocation", "true",
            "--admit-editor-action-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(method_process.exit_code == 0,
        "#1321: editor action execution JSON should accept admitted method editor actions");
    expect_contains(method_process.stdout_text, "\"editorActionExecution\": {",
        "#1321: editor action execution JSON should expose an execution object");
    expect_contains(method_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1321: editor action execution JSON should expose action ids");
    expect_contains(method_process.stdout_text, "\"kind\": \"source_editor\"",
        "#1321: editor action execution JSON should expose action kind metadata");
    expect_contains(method_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1321: editor action execution JSON should expose selected Studio contexts");
    expect_contains(method_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1321: editor action execution JSON should expose command tokens");
    expect_contains(method_process.stdout_text, "\"targetSurface\": \"method-editor\"",
        "#1321: editor action execution JSON should expose target surfaces");
    expect_contains(method_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1321: editor action execution JSON should carry asset paths");
    expect_contains(method_process.stdout_text, "\"recordIndex\": 4",
        "#1321: editor action execution JSON should carry record indexes");
    expect_contains(method_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1321: editor action execution JSON should carry object-name selectors");
    expect_contains(method_process.stdout_text, "\"uniqueId\": \"button-guid\"",
        "#1321: editor action execution JSON should carry unique-id selectors");
    expect_contains(method_process.stdout_text, "\"symbol\": \"cmdSave.Click\"",
        "#1321: editor action execution JSON should carry launch symbols");
    expect_contains(method_process.stdout_text, "\"line\": 42",
        "#1321: editor action execution JSON should carry line metadata");
    expect_contains(method_process.stdout_text, "\"column\": 7",
        "#1321: editor action execution JSON should carry column metadata");
    expect_contains(method_process.stdout_text, "\"executionReadyActionIds\": [\"edit-visual-method\"]",
        "#1398: editor action execution JSON should summarize execution-ready action ids");
    expect_contains(method_process.stdout_text, "\"executionBlockedActionIds\": []",
        "#1398: editor action execution JSON should summarize empty blocked action ids");
    expect_contains(method_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1398: editor action execution JSON should summarize empty execution errors");
    expect_contains(method_process.stdout_text, "\"launchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1321: editor action execution JSON should expose launch commands");
    expect_contains(method_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(COPPERFIN_TEST_SUCCESS_COMMAND, {}),
        "#1321: editor action execution JSON should expose the shell command");
    expect_contains(method_process.stdout_text, "\"observedExitCode\": 0",
        "#1321: successful editor action execution JSON should expose zero exit status");
    expect_contains(method_process.stdout_text, "\"executionAdmitted\": true",
        "#1321: editor action execution JSON should expose execution admission");
    expect_contains(method_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1321: editor action execution JSON should expose dispatch admission");
    expect_contains(method_process.stdout_text, "\"executed\": true",
        "#1321: editor action execution JSON should mark execution complete");
    expect_contains(method_process.stdout_text, "\"dryRun\": false",
        "#1321: admitted editor action execution JSON should not be dry-run");
    expect_contains(method_process.stdout_text, "\"mutatesAsset\": false",
        "#1321: editor action execution JSON should remain non-mutating");

    const auto expression_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-execute", "edit-report-expression",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "2",
            "--symbol", "Expr1.Expression",
            "--admit-editor-invocation", "true",
            "--admit-editor-action-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(expression_process.exit_code == 0,
        "#1321: editor action execution JSON should accept admitted report expression actions");
    expect_contains(expression_process.stdout_text, "\"kind\": \"expression_editor\"",
        "#1321: expression editor execution JSON should expose expression-editor metadata");
    expect_contains(expression_process.stdout_text, "\"targetSurface\": \"expression-editor\"",
        "#1321: expression editor execution JSON should expose expression editor target surfaces");
    expect_contains(expression_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1321: expression editor execution JSON should expose selection contexts");
    expect_contains(expression_process.stdout_text, "\"executionReadyActionIds\": [\"edit-report-expression\"]",
        "#1398: expression editor execution JSON should summarize execution-ready action ids");
    expect_contains(expression_process.stdout_text, "\"executionBlockedActionIds\": []",
        "#1398: expression editor execution JSON should summarize empty blocked action ids");
    expect_contains(expression_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1398: expression editor execution JSON should summarize empty execution errors");
    expect_contains(expression_process.stdout_text, "\"executed\": true",
        "#1321: expression editor execution JSON should mark execution complete");

    const auto missing_command_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-execute", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--admit-editor-action-execution", "true",
            "--json"
        },
        temp_root);
    expect(missing_command_process.exit_code == 2,
        "#1321: editor action execution JSON should reject missing launch commands");
    expect_contains(missing_command_process.stdout_text, "No editor action launch command was provided.",
        "#1321: missing editor action launch commands should report parser errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-execute", "show-property-grid",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "false",
            "--admit-editor-action-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 4,
        "#1321: editor action execution JSON should reject dry-run dispatch requests");
    expect_contains(dry_run_process.stdout_text, "\"editorActionExecution\": null",
        "#1321: dry-run editor action execution JSON should not expose a result object");
    expect_contains(dry_run_process.stdout_text,
        "An editor action dispatch request requires an admitted non-dry-run invocation.",
        "#1321: dry-run editor action execution JSON should report dispatch admission errors");

    const auto unadmitted_execution_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-execute", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--admit-editor-action-execution", "false",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(unadmitted_execution_process.exit_code == 4,
        "#1321: editor action execution JSON should require explicit execution admission");
    expect_contains(unadmitted_execution_process.stdout_text,
        "An editor action dispatch execution request requires explicit execution admission.",
        "#1321: unadmitted editor action execution JSON should report execution admission errors");
    expect_contains(unadmitted_execution_process.stdout_text, "\"executed\": false",
        "#1321: unadmitted editor action execution JSON should not mark execution complete");

    const auto failed_command_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-execute", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--admit-editor-action-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(failed_command_process.exit_code == 4,
        "#1321: editor action execution JSON should report nonzero process exits");
    expect_contains(failed_command_process.stdout_text,
        "Editor action launch command returned a non-zero exit code.",
        "#1321: failed editor action execution JSON should report process errors");
    expect_contains(failed_command_process.stdout_text, "\"observedExitCode\": 1",
        "#1347: failed editor action execution JSON should report normalized child exit codes");
    expect_contains(failed_command_process.stdout_text, "\"executed\": false",
        "#1321: failed editor action execution JSON should not mark execution complete");

    const auto invalid_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-execute", "edit-visual-method",
            "--selection-context", "not-a-context",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(invalid_context_process.exit_code == 2,
        "#1321: editor action execution JSON should reject unknown selection contexts");
    expect_contains(invalid_context_process.stdout_text, "Unknown selection context token: not-a-context",
        "#1321: invalid editor action execution context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_editor_action_dispatch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_editor_action_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "cmdSave",
            "--unique-id", "button-guid",
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1228: editor action dispatch catalog JSON should accept admitted visual-object catalogs");
    expect_contains(visual_process.stdout_text, "\"editorActionDispatchCatalog\": {",
        "#1228: editor action dispatch catalog JSON should expose a catalog object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1228: editor action dispatch catalog JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"actionCount\": 5",
        "#1228: visual-object dispatch catalog JSON should expose action counts");
    expect_contains(visual_process.stdout_text, "\"dispatchCount\": 5",
        "#1228: admitted visual-object dispatch catalog JSON should expose dispatch counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1228: admitted visual-object dispatch catalog JSON should expose error counts");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1228: admitted visual-object dispatch catalog JSON should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1228: editor action dispatch catalog JSON should remain non-mutating");
    expect_contains(visual_process.stdout_text, "\"dispatchReadyActionIds\": [\"show-property-grid\"",
        "#1366: editor action dispatch catalog JSON should summarize dispatch-ready actions");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedActionIds\": []",
        "#1366: admitted editor action dispatch catalog JSON should summarize empty blocked action ids");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1366: admitted editor action dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(visual_process.stdout_text, "\"entries\": [",
        "#1228: editor action dispatch catalog JSON should expose per-action entries");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1228: editor action dispatch catalog JSON should include method actions");
    expect_contains(visual_process.stdout_text, "\"kind\": \"source_editor\"",
        "#1228: editor action dispatch catalog JSON should expose action kind metadata");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1228: editor action dispatch catalog JSON should expose command tokens");
    expect_contains(visual_process.stdout_text, "\"targetSurface\": \"method-editor\"",
        "#1228: editor action dispatch catalog JSON should expose target surfaces");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1228: editor action dispatch catalog JSON should carry asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 4",
        "#1228: editor action dispatch catalog JSON should carry record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1228: editor action dispatch catalog JSON should carry object-name selectors");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"button-guid\"",
        "#1228: editor action dispatch catalog JSON should carry unique-id selectors");
    expect_contains(visual_process.stdout_text, "\"symbol\": \"cmdSave.Click\"",
        "#1228: editor action dispatch catalog JSON should carry launch symbols");
    expect_contains(visual_process.stdout_text, "\"line\": 42",
        "#1228: editor action dispatch catalog JSON should carry line metadata");
    expect_contains(visual_process.stdout_text, "\"column\": 7",
        "#1228: editor action dispatch catalog JSON should carry column metadata");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [",
        "#1228: editor action dispatch catalog JSON should expose dispatch arguments");
    expect_contains(visual_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1228: editor action dispatch catalog JSON should expose admitted dispatch state");
    expect_contains(visual_process.stdout_text, "\"executed\": false",
        "#1228: editor action dispatch catalog JSON should not execute editor processes");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-catalog",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1228: editor action dispatch catalog JSON should report dry-run per-action rejections");
    expect_contains(dry_run_process.stdout_text, "\"dispatchCount\": 0",
        "#1228: dry-run editor action dispatch catalog JSON should expose zero dispatch count");
    expect_contains(dry_run_process.stdout_text, "\"errorCount\": 5",
        "#1228: dry-run editor action dispatch catalog JSON should expose per-action error counts");
    expect_contains(dry_run_process.stdout_text, "\"dryRun\": true",
        "#1228: dry-run editor action dispatch catalog JSON should expose aggregate dry-run state");
    expect_contains(dry_run_process.stdout_text, "\"dispatchReadyActionIds\": []",
        "#1366: dry-run editor action dispatch catalog JSON should summarize empty dispatch-ready actions");
    expect_contains(dry_run_process.stdout_text, "\"dispatchBlockedActionIds\": [\"show-property-grid\"",
        "#1366: dry-run editor action dispatch catalog JSON should summarize blocked actions");
    expect_contains(dry_run_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"An editor action dispatch request requires an admitted non-dry-run invocation.\"",
        "#1366: dry-run editor action dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_process.stdout_text,
        "An editor action dispatch request requires an admitted non-dry-run invocation.",
        "#1228: dry-run editor action dispatch catalog JSON should expose dispatch errors");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-catalog",
            "--selection-context", "report_expression",
            "--path", "reports/orders.frx",
            "--record", "2",
            "--symbol", "Expr1.Expression",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1228: editor action dispatch catalog JSON should accept admitted report-expression catalogs");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1228: report-expression dispatch catalog JSON should expose selected contexts");
    expect_contains(report_process.stdout_text, "\"actionId\": \"edit-report-expression\"",
        "#1228: report-expression dispatch catalog JSON should include expression editor actions");
    expect_contains(report_process.stdout_text, "\"kind\": \"expression_editor\"",
        "#1228: report-expression dispatch catalog JSON should expose expression-editor metadata");
    expect_contains(report_process.stdout_text, "\"targetSurface\": \"expression-editor\"",
        "#1228: report-expression dispatch catalog JSON should expose expression editor target surfaces");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-catalog",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1228: editor action dispatch catalog JSON should reject invalid admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-editor-invocation value must be true or false.",
        "#1228: invalid editor action dispatch catalog boolean JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1228: editor action dispatch catalog JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1228: missing-context editor action dispatch catalog JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_editor_action_dispatch_execution_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_editor_action_dispatch_execution_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto admitted_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-execution-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "cmdSave",
            "--unique-id", "button-guid",
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--admit-editor-invocation", "true",
            "--admit-editor-action-execution", "true",
            "--json"
        },
        temp_root);
    expect(admitted_process.exit_code == 0,
        "#1329: editor action dispatch execution catalog JSON should accept admitted visual-object catalogs");
    expect_contains(admitted_process.stdout_text, "\"editorActionDispatchExecutionCatalog\": {",
        "#1329: editor action dispatch execution catalog JSON should expose a catalog object");
    expect_contains(admitted_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1329: editor action dispatch execution catalog JSON should expose selected contexts");
    expect_contains(admitted_process.stdout_text, "\"actionCount\": 5",
        "#1329: editor action dispatch execution catalog JSON should expose action counts");
    expect_contains(admitted_process.stdout_text, "\"executionReadyCount\": 5",
        "#1329: admitted editor action dispatch execution catalog JSON should expose readiness counts");
    expect_contains(admitted_process.stdout_text, "\"errorCount\": 0",
        "#1329: admitted editor action dispatch execution catalog JSON should expose zero errors");
    expect_contains(admitted_process.stdout_text, "\"dryRun\": false",
        "#1329: admitted editor action dispatch execution catalog JSON should not be dry-run");
    expect_contains(admitted_process.stdout_text, "\"executionReadyActionIds\": [\"show-property-grid\"",
        "#1367: admitted editor action dispatch execution catalog JSON should summarize execution-ready actions");
    expect_contains(admitted_process.stdout_text, "\"executionBlockedActionIds\": []",
        "#1367: admitted editor action dispatch execution catalog JSON should summarize empty blocked action ids");
    expect_contains(admitted_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1367: admitted editor action dispatch execution catalog JSON should summarize empty blocked execution errors");
    expect_contains(admitted_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1329: editor action dispatch execution catalog JSON should expose method actions");
    expect_contains(admitted_process.stdout_text, "\"launchOk\": true",
        "#1329: editor action dispatch execution catalog JSON should expose launch readiness");
    expect_contains(admitted_process.stdout_text, "\"admissionOk\": true",
        "#1329: editor action dispatch execution catalog JSON should expose invocation admission readiness");
    expect_contains(admitted_process.stdout_text, "\"dispatchOk\": true",
        "#1329: editor action dispatch execution catalog JSON should expose dispatch readiness");
    expect_contains(admitted_process.stdout_text, "\"executionAdmitted\": true",
        "#1329: editor action dispatch execution catalog JSON should expose execution admission state");
    expect_contains(admitted_process.stdout_text, "\"executionReady\": true",
        "#1329: editor action dispatch execution catalog JSON should expose execution readiness");
    expect_contains(admitted_process.stdout_text, "\"executionError\": \"\"",
        "#1329: admitted editor action dispatch execution catalog JSON should expose empty execution errors");
    expect_contains(admitted_process.stdout_text, "\"targetSurface\": \"method-editor\"",
        "#1329: editor action dispatch execution catalog JSON should preserve target surfaces");
    expect_contains(admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1329: editor action dispatch execution catalog JSON should expose dispatch arguments");
    expect_contains(admitted_process.stdout_text, "\"executed\": false",
        "#1329: editor action dispatch execution catalog JSON should not launch editors");

    const auto unadmitted_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-execution-catalog",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--admit-editor-action-execution", "false",
            "--json"
        },
        temp_root);
    expect(unadmitted_process.exit_code == 0,
        "#1329: editor action dispatch execution catalog JSON should report unadmitted execution as catalog errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionReadyCount\": 0",
        "#1329: unadmitted editor action dispatch execution catalog JSON should expose zero readiness");
    expect_contains(unadmitted_process.stdout_text, "\"errorCount\": 5",
        "#1329: unadmitted editor action dispatch execution catalog JSON should expose per-action errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionAdmitted\": false",
        "#1329: unadmitted editor action dispatch execution catalog JSON should expose admission false");
    expect_contains(unadmitted_process.stdout_text, "\"executionReadyActionIds\": []",
        "#1367: unadmitted editor action dispatch execution catalog JSON should summarize empty ready action ids");
    expect_contains(unadmitted_process.stdout_text, "\"executionBlockedActionIds\": [\"show-property-grid\"",
        "#1367: unadmitted editor action dispatch execution catalog JSON should summarize blocked action ids");
    expect_contains(unadmitted_process.stdout_text,
        "\"executionBlockedErrors\": [\"An editor action dispatch execution catalog entry requires explicit execution admission.\"",
        "#1367: unadmitted editor action dispatch execution catalog JSON should summarize blocked execution errors");
    expect_contains(unadmitted_process.stdout_text,
        "An editor action dispatch execution catalog entry requires explicit execution admission.",
        "#1329: unadmitted editor action dispatch execution catalog JSON should expose execution errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-execution-catalog",
            "--selection-context", "visual_object",
            "--admit-editor-action-execution", "true",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1329: editor action dispatch execution catalog JSON should report dry-run dispatch failures");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyCount\": 0",
        "#1329: dry-run editor action dispatch execution catalog JSON should expose zero readiness");
    expect_contains(dry_run_process.stdout_text, "\"errorCount\": 5",
        "#1329: dry-run editor action dispatch execution catalog JSON should expose per-action errors");
    expect_contains(dry_run_process.stdout_text, "\"executionBlockedActionIds\": [\"show-property-grid\"",
        "#1367: dry-run editor action dispatch execution catalog JSON should summarize dispatch-blocked action ids");
    expect_contains(dry_run_process.stdout_text,
        "\"executionBlockedErrors\": [\"An editor action dispatch request requires an admitted non-dry-run invocation.\"",
        "#1367: dry-run editor action dispatch execution catalog JSON should summarize dispatch-blocked errors");
    expect_contains(dry_run_process.stdout_text,
        "An editor action dispatch request requires an admitted non-dry-run invocation.",
        "#1329: dry-run editor action dispatch execution catalog JSON should expose dispatch errors");

    const auto invalid_execution_bool_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-execution-catalog",
            "--selection-context", "visual_object",
            "--admit-editor-action-execution", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_execution_bool_process.exit_code == 2,
        "#1329: editor action dispatch execution catalog JSON should reject invalid execution booleans");
    expect_contains(invalid_execution_bool_process.stdout_text,
        "The --admit-editor-action-execution value must be true or false.",
        "#1329: invalid editor execution catalog admission JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-dispatch-execution-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1329: editor action dispatch execution catalog JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1329: missing-context editor action dispatch execution catalog JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
