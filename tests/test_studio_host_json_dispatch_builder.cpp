// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_builder_launch_plans(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_builder_launch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto valid_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "control",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "grdOrders",
            "--unique-id", "grid-guid",
            "--json"
        },
        temp_root);
    expect(valid_process.exit_code == 0,
        "#1204: builder launch-plan JSON requests should exit successfully for context-valid builders");
    expect_contains(valid_process.stdout_text, "\"builderLaunchPlan\": {",
        "#1204: builder launch-plan JSON should expose a plan object");
    expect_contains(valid_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1204: builder launch-plan JSON should expose builder ids");
    expect_contains(valid_process.stdout_text, "\"kind\": \"builder\"",
        "#1204: builder launch-plan JSON should expose builder kind metadata");
    expect_contains(valid_process.stdout_text, "\"context\": \"control\"",
        "#1204: builder launch-plan JSON should expose selected builder contexts");
    expect_contains(valid_process.stdout_text, "\"vfp9Equivalent\": \"builder.app grid builder\"",
        "#1204: builder launch-plan JSON should preserve VFP 9 equivalent metadata");
    expect_contains(valid_process.stdout_text, "\"vfp9EquivalentDisplay\": \"builder.app grid builder\"",
        "#4303: builder launch-plan JSON should expose localized VFP equivalent display metadata");
    expect_contains(valid_process.stdout_text, "\"copperfinComponent\": \"cf_form_surface\"",
        "#1204: builder launch-plan JSON should preserve Copperfin component metadata");
    expect_contains(valid_process.stdout_text, "\"entryPoint\": \"cf_builders.grid_builder\"",
        "#1204: builder launch-plan JSON should expose entry-point metadata");
    expect_contains(valid_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1204: builder launch-plan JSON should carry asset paths without opening or mutating assets");
    expect_contains(valid_process.stdout_text, "\"recordIndex\": 4",
        "#1204: builder launch-plan JSON should carry selected record indexes");
    expect_contains(valid_process.stdout_text, "\"objectName\": \"grdOrders\"",
        "#1204: builder launch-plan JSON should carry object-name selectors");
    expect_contains(valid_process.stdout_text, "\"uniqueId\": \"grid-guid\"",
        "#1204: builder launch-plan JSON should carry unique-id selectors");
    expect_contains(valid_process.stdout_text, "\"launchReadyBuilderIds\": [\"grid-builder\"]",
        "#1391: builder launch-plan JSON should summarize launch-ready builder ids");
    expect_contains(valid_process.stdout_text, "\"launchBlockedBuilderIds\": []",
        "#1391: builder launch-plan JSON should summarize empty blocked builder ids");
    expect_contains(valid_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1391: builder launch-plan JSON should summarize empty launch errors");

    const auto wizard_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "label-wizard",
            "--builder-context", "label",
            "--path", "labels/mailing.lbx",
            "--json"
        },
        temp_root);
    expect(wizard_process.exit_code == 0,
        "#1204: builder launch-plan JSON requests should accept context-valid wizards");
    expect_contains(wizard_process.stdout_text, "\"kind\": \"wizard\"",
        "#1204: builder launch-plan JSON should preserve wizard kind metadata");
    expect_contains(wizard_process.stdout_text, "\"entryPoint\": \"cf_wizards.label_wizard\"",
        "#1204: builder launch-plan JSON should expose wizard entry points");

    const auto selection_visual_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "form-builder",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(selection_visual_process.exit_code == 0,
        "#1206: selection-context builder launch-plan JSON should accept visual-object form builders");
    expect_contains(selection_visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1206: selection-context builder launch-plan JSON should expose Studio selection contexts");
    expect_contains(selection_visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1206: selection-context builder launch-plan JSON should expose resolved form builders");
    expect_contains(selection_visual_process.stdout_text, "\"context\": \"form\"",
        "#1206: selection-context builder launch-plan JSON should expose resolved builder contexts");
    expect_contains(selection_visual_process.stdout_text, "\"launchReadyBuilderIds\": [\"form-builder\"]",
        "#1391: selection-context builder launch-plan JSON should summarize launch-ready builder ids");
    expect_contains(selection_visual_process.stdout_text, "\"launchBlockedBuilderIds\": []",
        "#1391: selection-context builder launch-plan JSON should summarize empty blocked builder ids");
    expect_contains(selection_visual_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1391: selection-context builder launch-plan JSON should summarize empty launch errors");

    const auto selection_grid_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "grdOrders",
            "--unique-id", "grid-guid",
            "--json"
        },
        temp_root);
    expect(selection_grid_process.exit_code == 0,
        "#1206: visual-object selection-context builder launch-plan JSON should also accept control builders");
    expect_contains(selection_grid_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1206: visual-object control-builder launch plans should expose Studio selection contexts");
    expect_contains(selection_grid_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1206: visual-object control-builder launch plans should expose resolved builder ids");

    const auto selection_label_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "label-wizard",
            "--selection-context", "label_expression",
            "--path", "labels/mailing.lbx",
            "--json"
        },
        temp_root);
    expect(selection_label_process.exit_code == 0,
        "#1206: label selection-context builder launch-plan JSON should accept label wizards");
    expect_contains(selection_label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#1206: label selection-context builder launch-plan JSON should expose Studio selection contexts");
    expect_contains(selection_label_process.stdout_text, "\"kind\": \"wizard\"",
        "#1206: label selection-context builder launch-plan JSON should preserve wizard metadata");

    const auto selection_wrong_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "form-builder",
            "--selection-context", "container_object",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);
    expect(selection_wrong_context_process.exit_code == 4,
        "#1206: selection-context builder launch-plan JSON should reject unavailable builders");
    expect_contains(selection_wrong_context_process.stdout_text,
        "The requested builder is not available for the selected Studio context.",
        "#1206: wrong selection-context builder launch-plan JSON should report validation errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1206: selection-context builder launch-plan JSON should reject unknown selection tokens");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1206: unknown selection-context builder launch-plan JSON should report parser errors");

    const auto ambiguous_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(ambiguous_context_process.exit_code == 2,
        "#1206: builder launch-plan JSON should reject simultaneous builder and selection contexts");
    expect_contains(ambiguous_context_process.stdout_text,
        "Builder launch-plan requests cannot provide both --builder-context and --selection-context.",
        "#1206: ambiguous builder launch-plan JSON should report parser errors");

    const auto wrong_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "report",
            "--path", "reports/orders.frx",
            "--json"
        },
        temp_root);
    expect(wrong_context_process.exit_code == 4,
        "#1204: builder launch-plan JSON should reject wrong-context builders");
    expect_contains(wrong_context_process.stdout_text, "\"builderLaunchPlan\": null",
        "#1204: wrong-context builder launch-plan JSON should not expose a plan object");
    expect_contains(wrong_context_process.stdout_text,
        "The requested builder is not available for the selected designer context.",
        "#1204: wrong-context builder launch-plan JSON should report validation errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1204: builder launch-plan JSON should reject unknown context tokens during parsing");
    expect_contains(unknown_context_process.stdout_text, "\"builderLaunchPlan\": null",
        "#1204: invalid builder context JSON should not expose a plan object");
    expect_contains(unknown_context_process.stdout_text, "Unknown builder context token: unknown",
        "#1204: invalid builder context JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "form-builder",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1204: builder launch-plan JSON should reject missing context tokens");
    expect_contains(missing_context_process.stdout_text, "No builder or selection context was provided.",
        "#1204: missing builder context JSON should report parser errors");

    const auto missing_id_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan",
            "--builder-context", "form",
            "--json"
        },
        temp_root);
    expect(missing_id_process.exit_code == 2,
        "#1204: builder launch-plan JSON should reject missing builder ids");
    expect_contains(missing_id_process.stdout_text, "Missing value for --builder-launch-plan.",
        "#1204: missing builder id JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "form-builder",
            "--builder-context", "form",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1204: builder launch-plan JSON should reject invalid record indexes");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1204: invalid builder launch-plan record JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_builder_launch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_builder_launch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto control_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-catalog",
            "--builder-context", "control",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "grdOrders",
            "--unique-id", "grid-guid",
            "--json"
        },
        temp_root);
    expect(control_process.exit_code == 0,
        "#1269: builder launch catalog JSON should accept control contexts");
    expect_contains(control_process.stdout_text, "\"builderLaunchCatalog\": {",
        "#1269: builder launch catalog JSON should expose a catalog object");
    expect_contains(control_process.stdout_text, "\"context\": \"control\"",
        "#1269: builder launch catalog JSON should expose builder contexts");
    expect_contains(control_process.stdout_text, "\"builderCount\": 2",
        "#1269: control builder launch catalog JSON should expose builder counts");
    expect_contains(control_process.stdout_text, "\"launchPlanCount\": 2",
        "#1269: control builder launch catalog JSON should expose launch-plan counts");
    expect_contains(control_process.stdout_text, "\"errorCount\": 0",
        "#1269: control builder launch catalog JSON should expose error counts");
    expect_contains(control_process.stdout_text, "\"dryRun\": true",
        "#1269: builder launch catalog JSON should remain a dry-run surface");
    expect_contains(control_process.stdout_text, "\"mutatesAsset\": false",
        "#1269: builder launch catalog JSON should remain non-mutating");
    expect_contains(control_process.stdout_text, "\"launchReadyBuilderIds\": [\"control-builder\", \"grid-builder\"]",
        "#1360: builder launch catalog JSON should summarize launch-ready builders");
    expect_contains(control_process.stdout_text, "\"launchBlockedBuilderIds\": []",
        "#1360: builder launch catalog JSON should summarize empty blocked builder ids");
    expect_contains(control_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1360: builder launch catalog JSON should summarize empty blocked launch errors");
    expect_contains(control_process.stdout_text, "\"entries\": [",
        "#1269: builder launch catalog JSON should expose per-builder entries");
    expect_contains(control_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1269: builder launch catalog JSON should include grid builders");
    expect_contains(control_process.stdout_text, "\"kind\": \"builder\"",
        "#1269: builder launch catalog JSON should expose builder kind metadata");
    expect_contains(control_process.stdout_text, "\"launchOk\": true",
        "#1269: builder launch catalog JSON should expose launch validation state");
    expect_contains(control_process.stdout_text, "\"vfp9Equivalent\": \"builder.app grid builder\"",
        "#1269: builder launch catalog JSON should preserve VFP 9 equivalent metadata");
    expect_contains(control_process.stdout_text, "\"vfp9EquivalentDisplay\": \"builder.app grid builder\"",
        "#4303: builder launch catalog JSON should expose localized VFP equivalent display metadata");
    expect_contains(control_process.stdout_text, "\"copperfinComponent\": \"cf_form_surface\"",
        "#1269: builder launch catalog JSON should preserve Copperfin component metadata");
    expect_contains(control_process.stdout_text, "\"entryPoint\": \"cf_builders.grid_builder\"",
        "#1269: builder launch catalog JSON should expose entry points");
    expect_contains(control_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1269: builder launch catalog JSON should preserve asset paths");
    expect_contains(control_process.stdout_text, "\"recordIndex\": 4",
        "#1269: builder launch catalog JSON should preserve record indexes");
    expect_contains(control_process.stdout_text, "\"objectName\": \"grdOrders\"",
        "#1269: builder launch catalog JSON should preserve object names");
    expect_contains(control_process.stdout_text, "\"uniqueId\": \"grid-guid\"",
        "#1269: builder launch catalog JSON should preserve unique ids");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-catalog",
            "--builder-context", "label",
            "--path", "labels/mailing.lbx",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#1269: builder launch catalog JSON should accept label contexts");
    expect_contains(label_process.stdout_text, "\"builderId\": \"label-wizard\"",
        "#1269: label builder launch catalog JSON should expose label wizards");
    expect_contains(label_process.stdout_text, "\"kind\": \"wizard\"",
        "#1269: label builder launch catalog JSON should preserve wizard kind metadata");
    expect_contains(label_process.stdout_text, "\"entryPoint\": \"cf_wizards.label_wizard\"",
        "#1269: label builder launch catalog JSON should expose wizard entry points");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-catalog",
            "--builder-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1269: builder launch catalog JSON should reject unknown context tokens");
    expect_contains(unknown_context_process.stdout_text, "\"builderLaunchCatalog\": null",
        "#1269: invalid builder launch catalog JSON should not expose catalog objects");
    expect_contains(unknown_context_process.stdout_text, "Unknown builder context token: unknown",
        "#1269: invalid builder launch catalog JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1269: builder launch catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No builder context was provided.",
        "#1269: missing builder launch catalog context JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-catalog",
            "--builder-context", "control",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1269: builder launch catalog JSON should reject invalid record indexes");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1269: invalid builder launch catalog record JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-catalog",
            "--builder-context", "control",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1269: builder launch catalog JSON should reject invocation-only options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown builder-launch-catalog option: --admit-ui-launch",
        "#1269: unknown builder launch catalog options should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selection_builder_launch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_builder_launch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-launch-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1278: selection builder launch catalog JSON should accept visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"selectionBuilderLaunchCatalog\": {",
        "#1278: selection builder launch catalog JSON should expose catalog objects");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1278: selection builder launch catalog JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"builderCount\": 3",
        "#1278: visual-object selection builder launch catalogs should expose mixed builder counts");
    expect_contains(visual_process.stdout_text, "\"launchPlanCount\": 3",
        "#1278: visual-object selection builder launch catalogs should expose launch-plan counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1278: visual-object selection builder launch catalogs should expose error counts");
    expect_contains(visual_process.stdout_text, "\"dryRun\": true",
        "#1278: selection builder launch catalogs should remain dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1278: selection builder launch catalogs should remain non-mutating");
    expect_contains(visual_process.stdout_text,
        "\"launchReadyBuilderIds\": [\"form-builder\", \"control-builder\", \"grid-builder\"]",
        "#1406: selection builder launch catalogs should summarize launch-ready builders");
    expect_contains(visual_process.stdout_text, "\"launchBlockedBuilderIds\": []",
        "#1406: selection builder launch catalogs should summarize empty blocked builder ids");
    expect_contains(visual_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1406: selection builder launch catalogs should summarize empty blocked launch errors");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1278: visual-object selection builder launch catalogs should include form builders");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1278: visual-object selection builder launch catalogs should include control builders");
    expect_contains(visual_process.stdout_text, "\"context\": \"form\"",
        "#1278: selection builder launch catalogs should expose resolved form contexts");
    expect_contains(visual_process.stdout_text, "\"context\": \"control\"",
        "#1278: selection builder launch catalogs should expose resolved control contexts");
    expect_contains(visual_process.stdout_text, "\"entryPoint\": \"cf_builders.form_builder\"",
        "#1278: selection builder launch catalogs should expose form builder entry points");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1278: selection builder launch catalogs should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1278: selection builder launch catalogs should preserve record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1278: selection builder launch catalogs should preserve object names");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1278: selection builder launch catalogs should preserve unique ids");

    const auto menu_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-launch-catalog",
            "--selection-context", "menu_item",
            "--path", "menus/main.mnx",
            "--object-name", "mnuMain",
            "--unique-id", "menu-guid",
            "--json"
        },
        temp_root);
    expect(menu_process.exit_code == 0,
        "#1278: selection builder launch catalog JSON should accept menu contexts");
    expect_contains(menu_process.stdout_text, "\"builderId\": \"menu-designer\"",
        "#1278: menu selection builder launch catalogs should expose menu designers");
    expect_contains(menu_process.stdout_text, "\"context\": \"menu\"",
        "#1278: menu selection builder launch catalogs should expose menu builder contexts");
    expect_contains(menu_process.stdout_text, "\"entryPoint\": \"cf_builders.menu_designer\"",
        "#1278: menu selection builder launch catalogs should expose menu entry points");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-launch-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1278: selection builder launch catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text,
        "\"selectionBuilderLaunchCatalog\": null",
        "#1278: invalid selection builder launch catalog JSON should not expose catalog objects");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1278: invalid selection builder launch catalog contexts should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-launch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1278: selection builder launch catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1278: missing selection builder launch catalog contexts should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-launch-catalog",
            "--selection-context", "visual_object",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1278: selection builder launch catalog JSON should reject invalid records");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1278: invalid selection builder launch catalog records should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-launch-catalog",
            "--selection-context", "visual_object",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1278: selection builder launch catalog JSON should reject invocation-only options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-builder-launch-catalog option: --admit-ui-launch",
        "#1278: unknown selection builder launch catalog options should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_builder_invocation_admission(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_builder_invocation_admission_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto admitted_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "grid-builder",
            "--builder-context", "control",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "grdOrders",
            "--unique-id", "grid-guid",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(admitted_process.exit_code == 0,
        "#1216: builder invocation-admission JSON should accept context-valid builders");
    expect_contains(admitted_process.stdout_text, "\"builderInvocationAdmission\": {",
        "#1216: builder invocation-admission JSON should expose a result object");
    expect_contains(admitted_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1216: builder invocation-admission JSON should expose builder ids");
    expect_contains(admitted_process.stdout_text, "\"kind\": \"builder\"",
        "#1216: builder invocation-admission JSON should expose builder kind metadata");
    expect_contains(admitted_process.stdout_text, "\"context\": \"control\"",
        "#1216: builder invocation-admission JSON should expose resolved builder contexts");
    expect_contains(admitted_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1216: builder invocation-admission JSON should expose stable command tokens");
    expect_contains(admitted_process.stdout_text, "\"entryPoint\": \"cf_builders.grid_builder\"",
        "#1216: builder invocation-admission JSON should expose entry points");
    expect_contains(admitted_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1216: builder invocation-admission JSON should preserve asset paths");
    expect_contains(admitted_process.stdout_text, "\"recordIndex\": 4",
        "#1216: builder invocation-admission JSON should preserve record indexes");
    expect_contains(admitted_process.stdout_text, "\"objectName\": \"grdOrders\"",
        "#1216: builder invocation-admission JSON should preserve object names");
    expect_contains(admitted_process.stdout_text, "\"uniqueId\": \"grid-guid\"",
        "#1216: builder invocation-admission JSON should preserve unique ids");
    expect_contains(admitted_process.stdout_text, "\"admissionReadyBuilderIds\": [\"grid-builder\"]",
        "#1392: builder invocation-admission JSON should summarize admission-ready builder ids");
    expect_contains(admitted_process.stdout_text, "\"admissionBlockedBuilderIds\": []",
        "#1392: builder invocation-admission JSON should summarize empty blocked builder ids");
    expect_contains(admitted_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1392: builder invocation-admission JSON should summarize empty admission errors");
    expect_contains(admitted_process.stdout_text, "\"uiLaunchAdmitted\": true",
        "#1216: builder invocation-admission JSON should expose UI-admission state");
    expect_contains(admitted_process.stdout_text, "\"dryRun\": false",
        "#1216: admitted builder invocation JSON should report non-dry-run admission");
    expect_contains(admitted_process.stdout_text, "\"mutatesAsset\": false",
        "#1216: builder invocation-admission JSON should remain non-mutating");

    const auto selection_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "label-wizard",
            "--selection-context", "label_expression",
            "--path", "labels/mailing.lbx",
            "--json"
        },
        temp_root);
    expect(selection_process.exit_code == 0,
        "#1216: builder invocation-admission JSON should accept selection-context builders");
    expect_contains(selection_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#1216: selection-context invocation-admission JSON should expose Studio selection contexts");
    expect_contains(selection_process.stdout_text, "\"builderId\": \"label-wizard\"",
        "#1216: selection-context invocation-admission JSON should expose wizard ids");
    expect_contains(selection_process.stdout_text, "\"kind\": \"wizard\"",
        "#1216: selection-context invocation-admission JSON should preserve wizard kind metadata");
    expect_contains(selection_process.stdout_text, "\"admissionReadyBuilderIds\": [\"label-wizard\"]",
        "#1392: selection-context builder invocation-admission JSON should summarize admission-ready builder ids");
    expect_contains(selection_process.stdout_text, "\"admissionBlockedBuilderIds\": []",
        "#1392: selection-context builder invocation-admission JSON should summarize empty blocked builder ids");
    expect_contains(selection_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1392: selection-context builder invocation-admission JSON should summarize empty admission errors");
    expect_contains(selection_process.stdout_text, "\"uiLaunchAdmitted\": false",
        "#1216: omitted UI admission should default to false");
    expect_contains(selection_process.stdout_text, "\"dryRun\": true",
        "#1216: omitted UI admission should keep invocation plans as dry runs");

    const auto wrong_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "form-builder",
            "--selection-context", "container_object",
            "--json"
        },
        temp_root);
    expect(wrong_context_process.exit_code == 4,
        "#1216: builder invocation-admission JSON should reject wrong-context builders");
    expect_contains(wrong_context_process.stdout_text, "\"builderInvocationAdmission\": null",
        "#1216: wrong-context invocation-admission JSON should not expose a result object");
    expect_contains(wrong_context_process.stdout_text,
        "The requested builder is not available for the selected Studio context.",
        "#1216: wrong-context invocation-admission JSON should report launch validation errors");

    const auto invalid_bool_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_bool_process.exit_code == 2,
        "#1216: builder invocation-admission JSON should reject invalid UI-admission booleans");
    expect_contains(invalid_bool_process.stdout_text, "The --admit-ui-launch value must be true or false.",
        "#1216: invalid UI-admission JSON should report parser errors");

    const auto ambiguous_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(ambiguous_context_process.exit_code == 2,
        "#1216: builder invocation-admission JSON should reject simultaneous builder and selection contexts");
    expect_contains(ambiguous_context_process.stdout_text,
        "Builder invocation-admission requests cannot provide both --builder-context and --selection-context.",
        "#1216: ambiguous invocation-admission JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "grid-builder",
            "--builder-context", "control",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1216: builder invocation-admission JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1216: invalid invocation-admission record JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_builder_invocation_admission_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_builder_invocation_admission_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto admitted_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission-catalog",
            "--builder-context", "control",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "grdOrders",
            "--unique-id", "grid-guid",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(admitted_process.exit_code == 0,
        "#1271: builder invocation admission catalog JSON should accept admitted control catalogs");
    expect_contains(admitted_process.stdout_text, "\"builderInvocationAdmissionCatalog\": {",
        "#1271: builder invocation admission catalog JSON should expose catalog objects");
    expect_contains(admitted_process.stdout_text, "\"context\": \"control\"",
        "#1271: builder invocation admission catalog JSON should expose contexts");
    expect_contains(admitted_process.stdout_text, "\"builderCount\": 2",
        "#1271: builder invocation admission catalog JSON should expose builder counts");
    expect_contains(admitted_process.stdout_text, "\"admissionCount\": 2",
        "#1271: builder invocation admission catalog JSON should expose admission counts");
    expect_contains(admitted_process.stdout_text, "\"errorCount\": 0",
        "#1271: admitted builder invocation admission catalog JSON should expose error counts");
    expect_contains(admitted_process.stdout_text, "\"dryRun\": false",
        "#1271: admitted builder invocation admission catalog JSON should report non-dry-run state");
    expect_contains(admitted_process.stdout_text, "\"mutatesAsset\": false",
        "#1271: builder invocation admission catalog JSON should remain non-mutating");
    expect_contains(admitted_process.stdout_text, "\"admissionReadyBuilderIds\": [\"control-builder\", \"grid-builder\"]",
        "#1361: builder invocation admission catalog JSON should summarize admission-ready builders");
    expect_contains(admitted_process.stdout_text, "\"admissionBlockedBuilderIds\": []",
        "#1361: builder invocation admission catalog JSON should summarize empty blocked builder ids");
    expect_contains(admitted_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1361: builder invocation admission catalog JSON should summarize empty blocked admission errors");
    expect_contains(admitted_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1271: builder invocation admission catalog JSON should include grid builders");
    expect_contains(admitted_process.stdout_text, "\"admissionOk\": true",
        "#1271: builder invocation admission catalog JSON should expose admission status");
    expect_contains(admitted_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1271: builder invocation admission catalog JSON should expose command tokens");
    expect_contains(admitted_process.stdout_text, "\"entryPoint\": \"cf_builders.grid_builder\"",
        "#1271: builder invocation admission catalog JSON should expose entry points");
    expect_contains(admitted_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1271: builder invocation admission catalog JSON should preserve asset paths");
    expect_contains(admitted_process.stdout_text, "\"recordIndex\": 4",
        "#1271: builder invocation admission catalog JSON should preserve record indexes");
    expect_contains(admitted_process.stdout_text, "\"objectName\": \"grdOrders\"",
        "#1271: builder invocation admission catalog JSON should preserve object names");
    expect_contains(admitted_process.stdout_text, "\"uniqueId\": \"grid-guid\"",
        "#1271: builder invocation admission catalog JSON should preserve unique ids");
    expect_contains(admitted_process.stdout_text, "\"uiLaunchAdmitted\": true",
        "#1271: builder invocation admission catalog JSON should expose UI admission state");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission-catalog",
            "--builder-context", "control",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1271: builder invocation admission catalog JSON should accept dry-run catalogs");
    expect_contains(dry_run_process.stdout_text, "\"admissionCount\": 2",
        "#1271: dry-run builder invocation admission catalog JSON should still admit entries");
    expect_contains(dry_run_process.stdout_text, "\"dryRun\": true",
        "#1271: dry-run builder invocation admission catalog JSON should expose aggregate dry-run state");
    expect_contains(dry_run_process.stdout_text, "\"admissionReadyBuilderIds\": [\"control-builder\", \"grid-builder\"]",
        "#1361: dry-run builder invocation admission catalog JSON should preserve admission-ready builder summaries");
    expect_contains(dry_run_process.stdout_text, "\"uiLaunchAdmitted\": false",
        "#1271: dry-run builder invocation admission catalog entries should expose omitted admission");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission-catalog",
            "--builder-context", "label",
            "--path", "labels/mailing.lbx",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#1271: builder invocation admission catalog JSON should accept label contexts");
    expect_contains(label_process.stdout_text, "\"builderId\": \"label-wizard\"",
        "#1271: label builder invocation admission catalog JSON should expose label wizards");
    expect_contains(label_process.stdout_text, "\"kind\": \"wizard\"",
        "#1271: label builder invocation admission catalog JSON should preserve wizard kind metadata");
    expect_contains(label_process.stdout_text, "\"entryPoint\": \"cf_wizards.label_wizard\"",
        "#1271: label builder invocation admission catalog JSON should expose wizard entry points");

    const auto invalid_bool_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission-catalog",
            "--builder-context", "control",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_bool_process.exit_code == 2,
        "#1271: builder invocation admission catalog JSON should reject invalid booleans");
    expect_contains(invalid_bool_process.stdout_text,
        "The --admit-ui-launch value must be true or false.",
        "#1271: invalid builder invocation admission catalog booleans should report parser errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission-catalog",
            "--builder-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1271: builder invocation admission catalog JSON should reject unknown contexts");
    expect_contains(unknown_context_process.stdout_text, "\"builderInvocationAdmissionCatalog\": null",
        "#1271: invalid builder invocation admission catalog JSON should not expose catalog objects");
    expect_contains(unknown_context_process.stdout_text, "Unknown builder context token: unknown",
        "#1271: invalid builder invocation admission catalog contexts should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission-catalog",
            "--builder-context", "control",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1271: builder invocation admission catalog JSON should reject invalid records");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1271: invalid builder invocation admission catalog records should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selection_builder_invocation_admission_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_builder_invocation_admission_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-invocation-admission-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1274: selection builder invocation admission catalog JSON should accept visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"selectionBuilderInvocationAdmissionCatalog\": {",
        "#1274: selection builder invocation admission catalog JSON should expose catalog objects");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1274: selection builder invocation admission catalog JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"builderCount\": 3",
        "#1274: visual-object selection builder admission catalogs should expose mixed builder counts");
    expect_contains(visual_process.stdout_text, "\"admissionCount\": 3",
        "#1274: visual-object selection builder admission catalogs should expose admission counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1274: visual-object selection builder admission catalogs should expose error counts");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1274: admitted selection builder admission catalogs should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1274: selection builder admission catalogs should remain non-mutating");
    expect_contains(visual_process.stdout_text,
        "\"admissionReadyBuilderIds\": [\"form-builder\", \"control-builder\", \"grid-builder\"]",
        "#1406: selection builder admission catalogs should summarize admitted builders");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedBuilderIds\": []",
        "#1406: selection builder admission catalogs should summarize empty blocked builder ids");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1406: selection builder admission catalogs should summarize empty blocked admission errors");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1274: visual-object selection builder admission catalogs should include form builders");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1274: visual-object selection builder admission catalogs should include control builders");
    expect_contains(visual_process.stdout_text, "\"context\": \"form\"",
        "#1274: selection builder admission catalogs should expose resolved form contexts");
    expect_contains(visual_process.stdout_text, "\"context\": \"control\"",
        "#1274: selection builder admission catalogs should expose resolved control contexts");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1274: selection builder admission catalogs should expose command tokens");
    expect_contains(visual_process.stdout_text, "\"entryPoint\": \"cf_builders.form_builder\"",
        "#1274: selection builder admission catalogs should expose form builder entry points");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1274: selection builder admission catalogs should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1274: selection builder admission catalogs should preserve record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1274: selection builder admission catalogs should preserve object names");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1274: selection builder admission catalogs should preserve unique ids");
    expect_contains(visual_process.stdout_text, "\"uiLaunchAdmitted\": true",
        "#1274: selection builder admission catalogs should expose admitted UI state");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-invocation-admission-catalog",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1274: selection builder invocation admission catalog JSON should accept dry-run visual catalogs");
    expect_contains(dry_run_process.stdout_text, "\"dryRun\": true",
        "#1274: dry-run selection builder admission catalogs should expose aggregate dry-run state");
    expect_contains(dry_run_process.stdout_text, "\"uiLaunchAdmitted\": false",
        "#1274: dry-run selection builder admission catalogs should expose omitted UI admission");

    const auto menu_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-invocation-admission-catalog",
            "--selection-context", "menu_item",
            "--path", "menus/main.mnx",
            "--object-name", "mnuMain",
            "--unique-id", "menu-guid",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(menu_process.exit_code == 0,
        "#1274: selection builder invocation admission catalog JSON should accept menu contexts");
    expect_contains(menu_process.stdout_text, "\"builderId\": \"menu-designer\"",
        "#1274: menu selection builder admission catalogs should expose menu designers");
    expect_contains(menu_process.stdout_text, "\"context\": \"menu\"",
        "#1274: menu selection builder admission catalogs should expose menu builder contexts");
    expect_contains(menu_process.stdout_text, "\"entryPoint\": \"cf_builders.menu_designer\"",
        "#1274: menu selection builder admission catalogs should expose menu entry points");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-invocation-admission-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1274: selection builder invocation admission catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text,
        "\"selectionBuilderInvocationAdmissionCatalog\": null",
        "#1274: invalid selection builder admission catalog JSON should not expose catalog objects");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1274: invalid selection builder admission catalog contexts should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-invocation-admission-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1274: selection builder invocation admission catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1274: missing selection builder admission catalog contexts should report parser errors");

    const auto invalid_bool_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-invocation-admission-catalog",
            "--selection-context", "visual_object",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_bool_process.exit_code == 2,
        "#1274: selection builder invocation admission catalog JSON should reject invalid booleans");
    expect_contains(invalid_bool_process.stdout_text,
        "The --admit-ui-launch value must be true or false.",
        "#1274: invalid selection builder admission catalog booleans should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selection_builder_dispatch_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_builder_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-dispatch-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1276: selection builder dispatch catalog JSON should accept visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"selectionBuilderDispatchCatalog\": {",
        "#1276: selection builder dispatch catalog JSON should expose catalog objects");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1276: selection builder dispatch catalog JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"builderCount\": 3",
        "#1276: visual-object selection builder dispatch catalogs should expose mixed builder counts");
    expect_contains(visual_process.stdout_text, "\"dispatchCount\": 3",
        "#1276: visual-object selection builder dispatch catalogs should expose dispatch counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1276: visual-object selection builder dispatch catalogs should expose error counts");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1276: admitted selection builder dispatch catalogs should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1276: selection builder dispatch catalogs should remain non-mutating");
    expect_contains(visual_process.stdout_text,
        "\"dispatchReadyBuilderIds\": [\"form-builder\", \"control-builder\", \"grid-builder\"]",
        "#1406: selection builder dispatch catalogs should summarize dispatch-ready builders");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedBuilderIds\": []",
        "#1406: selection builder dispatch catalogs should summarize empty blocked builder ids");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1406: selection builder dispatch catalogs should summarize empty blocked dispatch errors");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1276: visual-object selection builder dispatch catalogs should include form builders");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1276: visual-object selection builder dispatch catalogs should include control builders");
    expect_contains(visual_process.stdout_text, "\"dispatchOk\": true",
        "#1276: admitted selection builder dispatch catalogs should expose dispatch success");
    expect_contains(visual_process.stdout_text, "\"context\": \"form\"",
        "#1276: selection builder dispatch catalogs should expose resolved form contexts");
    expect_contains(visual_process.stdout_text, "\"context\": \"control\"",
        "#1276: selection builder dispatch catalogs should expose resolved control contexts");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1276: selection builder dispatch catalogs should expose command tokens");
    expect_contains(visual_process.stdout_text, "\"entryPoint\": \"cf_builders.form_builder\"",
        "#1276: selection builder dispatch catalogs should expose form builder entry points");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1276: selection builder dispatch catalogs should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1276: selection builder dispatch catalogs should preserve record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1276: selection builder dispatch catalogs should preserve object names");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1276: selection builder dispatch catalogs should preserve unique ids");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [",
        "#1276: selection builder dispatch catalogs should expose dispatch arguments");
    expect_contains(visual_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1276: selection builder dispatch catalogs should expose admitted dispatch state");
    expect_contains(visual_process.stdout_text, "\"executed\": false",
        "#1276: selection builder dispatch catalogs should remain non-executing");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-dispatch-catalog",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1276: selection builder dispatch catalog JSON should accept dry-run visual catalogs");
    expect_contains(dry_run_process.stdout_text, "\"dispatchCount\": 0",
        "#1276: dry-run selection builder dispatch catalogs should not admit dispatches");
    expect_contains(dry_run_process.stdout_text, "\"errorCount\": 3",
        "#1276: dry-run selection builder dispatch catalogs should expose dispatch errors");
    expect_contains(dry_run_process.stdout_text, "\"dispatchReadyBuilderIds\": []",
        "#1406: dry-run selection builder dispatch catalogs should summarize empty dispatch-ready builders");
    expect_contains(dry_run_process.stdout_text,
        "\"dispatchBlockedBuilderIds\": [\"form-builder\", \"control-builder\", \"grid-builder\"]",
        "#1406: dry-run selection builder dispatch catalogs should summarize blocked builders");
    expect_contains(dry_run_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A builder dispatch request requires an admitted non-dry-run invocation.\"",
        "#1406: dry-run selection builder dispatch catalogs should summarize blocked dispatch errors");
    expect_contains(dry_run_process.stdout_text,
        "A builder dispatch request requires an admitted non-dry-run invocation.",
        "#1276: dry-run selection builder dispatch catalogs should expose admission errors");

    const auto menu_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-dispatch-catalog",
            "--selection-context", "menu_item",
            "--path", "menus/main.mnx",
            "--object-name", "mnuMain",
            "--unique-id", "menu-guid",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(menu_process.exit_code == 0,
        "#1276: selection builder dispatch catalog JSON should accept menu contexts");
    expect_contains(menu_process.stdout_text, "\"builderId\": \"menu-designer\"",
        "#1276: menu selection builder dispatch catalogs should expose menu designers");
    expect_contains(menu_process.stdout_text, "\"context\": \"menu\"",
        "#1276: menu selection builder dispatch catalogs should expose menu builder contexts");
    expect_contains(menu_process.stdout_text, "\"entryPoint\": \"cf_builders.menu_designer\"",
        "#1276: menu selection builder dispatch catalogs should expose menu entry points");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-dispatch-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1276: selection builder dispatch catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text,
        "\"selectionBuilderDispatchCatalog\": null",
        "#1276: invalid selection builder dispatch catalog JSON should not expose catalog objects");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1276: invalid selection builder dispatch catalog contexts should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1276: selection builder dispatch catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1276: missing selection builder dispatch catalog contexts should report parser errors");

    const auto invalid_bool_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-dispatch-catalog",
            "--selection-context", "visual_object",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_bool_process.exit_code == 2,
        "#1276: selection builder dispatch catalog JSON should reject invalid booleans");
    expect_contains(invalid_bool_process.stdout_text,
        "The --admit-ui-launch value must be true or false.",
        "#1276: invalid selection builder dispatch catalog booleans should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selection_builder_dispatch_execution_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_builder_dispatch_execution_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-dispatch-execution-catalog",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1407: selection builder dispatch execution catalog JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"selectionBuilderDispatchExecutionCatalog\": {",
        "#1407: selection builder dispatch execution catalog JSON should expose catalog objects");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1407: selection builder dispatch execution catalogs should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"builderCount\": 3",
        "#1407: selection builder dispatch execution catalogs should expose builder counts");
    expect_contains(visual_process.stdout_text, "\"executionReadyCount\": 3",
        "#1407: admitted selection builder dispatch execution catalogs should expose ready counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1407: admitted selection builder dispatch execution catalogs should expose empty error counts");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1407: admitted selection builder dispatch execution catalogs should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1407: selection builder dispatch execution catalogs should remain non-mutating");
    expect_contains(visual_process.stdout_text,
        "\"executionReadyBuilderIds\": [\"form-builder\", \"control-builder\", \"grid-builder\"]",
        "#1407: selection builder dispatch execution catalogs should summarize execution-ready builders");
    expect_contains(visual_process.stdout_text, "\"executionBlockedBuilderIds\": []",
        "#1407: selection builder dispatch execution catalogs should summarize empty blocked builder ids");
    expect_contains(visual_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1407: selection builder dispatch execution catalogs should summarize empty blocked execution errors");
    expect_contains(visual_process.stdout_text, "\"executionAdmitted\": true",
        "#1407: selection builder dispatch execution catalog entries should expose execution admission");
    expect_contains(visual_process.stdout_text, "\"executionReady\": true",
        "#1407: selection builder dispatch execution catalog entries should expose execution readiness");
    expect_contains(visual_process.stdout_text, "\"dispatchOk\": true",
        "#1407: selection builder dispatch execution catalog entries should preserve dispatch state");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [",
        "#1407: selection builder dispatch execution catalog entries should preserve dispatch arguments");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-dispatch-execution-catalog",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1407: selection builder dispatch execution catalog JSON should accept dry-run visual catalogs");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyCount\": 0",
        "#1407: dry-run selection builder dispatch execution catalogs should not mark builders ready");
    expect_contains(dry_run_process.stdout_text, "\"errorCount\": 3",
        "#1407: dry-run selection builder dispatch execution catalogs should expose readiness errors");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyBuilderIds\": []",
        "#1407: dry-run selection builder dispatch execution catalogs should summarize empty ready builders");
    expect_contains(dry_run_process.stdout_text,
        "\"executionBlockedBuilderIds\": [\"form-builder\", \"control-builder\", \"grid-builder\"]",
        "#1407: dry-run selection builder dispatch execution catalogs should summarize blocked builders");
    expect_contains(dry_run_process.stdout_text,
        "\"executionBlockedErrors\": [\"A builder dispatch request requires an admitted non-dry-run invocation.\"",
        "#1407: dry-run selection builder dispatch execution catalogs should summarize blocked errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--selection-builder-dispatch-execution-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1407: selection builder dispatch execution catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text,
        "\"selectionBuilderDispatchExecutionCatalog\": null",
        "#1407: missing selection builder dispatch execution catalog JSON should not expose catalog objects");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1407: missing selection builder dispatch execution catalog contexts should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_builder_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_builder_dispatch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto admitted_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch", "grid-builder",
            "--builder-context", "control",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "grdOrders",
            "--unique-id", "grid-guid",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(admitted_process.exit_code == 0,
        "#1230: builder dispatch JSON should accept admitted context-valid builders");
    expect_contains(admitted_process.stdout_text, "\"builderDispatch\": {",
        "#1230: builder dispatch JSON should expose a result object");
    expect_contains(admitted_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1230: builder dispatch JSON should expose builder ids");
    expect_contains(admitted_process.stdout_text, "\"kind\": \"builder\"",
        "#1230: builder dispatch JSON should expose builder kind metadata");
    expect_contains(admitted_process.stdout_text, "\"selectionContext\": null",
        "#1230: builder-context dispatch JSON should expose null Studio selection contexts");
    expect_contains(admitted_process.stdout_text, "\"context\": \"control\"",
        "#1230: builder dispatch JSON should expose resolved builder contexts");
    expect_contains(admitted_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1230: builder dispatch JSON should expose stable command tokens");
    expect_contains(admitted_process.stdout_text, "\"entryPoint\": \"cf_builders.grid_builder\"",
        "#1230: builder dispatch JSON should expose entry points");
    expect_contains(admitted_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1230: builder dispatch JSON should preserve asset paths");
    expect_contains(admitted_process.stdout_text, "\"recordIndex\": 4",
        "#1230: builder dispatch JSON should preserve record indexes");
    expect_contains(admitted_process.stdout_text, "\"objectName\": \"grdOrders\"",
        "#1230: builder dispatch JSON should preserve object names");
    expect_contains(admitted_process.stdout_text, "\"uniqueId\": \"grid-guid\"",
        "#1230: builder dispatch JSON should preserve unique ids");
    expect_contains(admitted_process.stdout_text, "\"dispatchReadyBuilderIds\": [\"grid-builder\"]",
        "#1393: builder dispatch JSON should summarize dispatch-ready builder ids");
    expect_contains(admitted_process.stdout_text, "\"dispatchBlockedBuilderIds\": []",
        "#1393: builder dispatch JSON should summarize empty blocked builder ids");
    expect_contains(admitted_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1393: builder dispatch JSON should summarize empty dispatch errors");
    expect_contains(admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1230: builder dispatch JSON should expose dispatch arguments");
    expect_contains(admitted_process.stdout_text, "\"--builder-id\"",
        "#1230: builder dispatch JSON should expose builder-id arguments");
    expect_contains(admitted_process.stdout_text, "\"grid-builder\"",
        "#1230: builder dispatch JSON should expose builder-id values");
    expect_contains(admitted_process.stdout_text, "\"--builder-context\"",
        "#1230: builder dispatch JSON should expose context arguments");
    expect_contains(admitted_process.stdout_text, "\"control\"",
        "#1230: builder dispatch JSON should expose context values");
    expect_contains(admitted_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1230: admitted builder dispatch JSON should expose dispatch-admitted state");
    expect_contains(admitted_process.stdout_text, "\"dryRun\": false",
        "#1230: admitted builder dispatch JSON should not be dry-run");
    expect_contains(admitted_process.stdout_text, "\"executed\": false",
        "#1230: builder dispatch JSON should not execute builder processes");
    expect_contains(admitted_process.stdout_text, "\"mutatesAsset\": false",
        "#1230: builder dispatch JSON should remain non-mutating");

    const auto selection_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch", "label-wizard",
            "--selection-context", "label_expression",
            "--path", "labels/mailing.lbx",
            "--record", "2",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(selection_process.exit_code == 0,
        "#1230: builder dispatch JSON should accept selection-context builders");
    expect_contains(selection_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#1230: selection-context builder dispatch JSON should expose Studio selection contexts");
    expect_contains(selection_process.stdout_text, "\"builderId\": \"label-wizard\"",
        "#1230: selection-context builder dispatch JSON should expose wizard ids");
    expect_contains(selection_process.stdout_text, "\"kind\": \"wizard\"",
        "#1230: selection-context builder dispatch JSON should preserve wizard kind metadata");
    expect_contains(selection_process.stdout_text, "\"context\": \"label\"",
        "#1230: selection-context builder dispatch JSON should expose resolved builder contexts");
    expect_contains(selection_process.stdout_text, "\"dispatchReadyBuilderIds\": [\"label-wizard\"]",
        "#1393: selection-context builder dispatch JSON should summarize dispatch-ready builder ids");
    expect_contains(selection_process.stdout_text, "\"dispatchBlockedBuilderIds\": []",
        "#1393: selection-context builder dispatch JSON should summarize empty blocked builder ids");
    expect_contains(selection_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1393: selection-context builder dispatch JSON should summarize empty dispatch errors");
    expect_contains(selection_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1230: selection-context builder dispatch JSON should expose admitted dispatch state");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch", "grid-builder",
            "--builder-context", "control",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 4,
        "#1230: builder dispatch JSON should reject dry-run dispatch requests");
    expect_contains(dry_run_process.stdout_text, "\"builderDispatch\": null",
        "#1230: dry-run builder dispatch JSON should not expose a result object");
    expect_contains(dry_run_process.stdout_text,
        "A builder dispatch request requires an admitted non-dry-run invocation.",
        "#1230: dry-run builder dispatch JSON should report dispatch admission errors");

    const auto wrong_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch", "form-builder",
            "--selection-context", "container_object",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(wrong_context_process.exit_code == 4,
        "#1230: builder dispatch JSON should reject wrong-context builders");
    expect_contains(wrong_context_process.stdout_text, "\"builderDispatch\": null",
        "#1230: wrong-context builder dispatch JSON should not expose a result object");
    expect_contains(wrong_context_process.stdout_text,
        "The requested builder is not available for the selected Studio context.",
        "#1230: wrong-context builder dispatch JSON should report launch validation errors");

    const auto invalid_bool_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_bool_process.exit_code == 2,
        "#1230: builder dispatch JSON should reject invalid UI-admission booleans");
    expect_contains(invalid_bool_process.stdout_text, "The --admit-ui-launch value must be true or false.",
        "#1230: invalid builder dispatch UI-admission JSON should report parser errors");

    const auto ambiguous_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(ambiguous_context_process.exit_code == 2,
        "#1230: builder dispatch JSON should reject simultaneous builder and selection contexts");
    expect_contains(ambiguous_context_process.stdout_text,
        "Builder dispatch requests cannot provide both --builder-context and --selection-context.",
        "#1230: ambiguous builder dispatch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch", "grid-builder",
            "--builder-context", "control",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1230: builder dispatch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1230: invalid builder dispatch record JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch", "grid-builder",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1230: builder dispatch JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No builder or selection context was provided.",
        "#1230: missing-context builder dispatch JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_builder_execution(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_builder_execution_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto admitted_process = run_process_capture(
        studio_host_path,
        {
            "--builder-execute", "grid-builder",
            "--builder-context", "control",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "grdOrders",
            "--unique-id", "grid-guid",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "true",
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(admitted_process.exit_code == 0,
        "#1319: builder execution JSON should accept admitted context-valid builders");
    expect_contains(admitted_process.stdout_text, "\"builderExecution\": {",
        "#1319: builder execution JSON should expose a result object");
    expect_contains(admitted_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1319: builder execution JSON should expose builder ids");
    expect_contains(admitted_process.stdout_text, "\"selectionContext\": null",
        "#1319: builder-context execution JSON should expose null Studio selection contexts");
    expect_contains(admitted_process.stdout_text, "\"context\": \"control\"",
        "#1319: builder execution JSON should expose resolved builder contexts");
    expect_contains(admitted_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1319: builder execution JSON should expose stable command tokens");
    expect_contains(admitted_process.stdout_text, "\"entryPoint\": \"cf_builders.grid_builder\"",
        "#1319: builder execution JSON should expose entry points");
    expect_contains(admitted_process.stdout_text, "\"executionReadyBuilderIds\": [\"grid-builder\"]",
        "#1394: builder execution JSON should summarize execution-ready builder ids");
    expect_contains(admitted_process.stdout_text, "\"executionBlockedBuilderIds\": []",
        "#1394: builder execution JSON should summarize empty blocked builder ids");
    expect_contains(admitted_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1394: builder execution JSON should summarize empty execution errors");
    expect_contains(admitted_process.stdout_text, "\"launchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1319: builder execution JSON should expose launch commands");
    expect_contains(admitted_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(COPPERFIN_TEST_SUCCESS_COMMAND, {}),
        "#1319: builder execution JSON should expose the shell command");
    expect_contains(admitted_process.stdout_text, "\"observedExitCode\": 0",
        "#1319: successful builder execution JSON should expose zero exit status");
    expect_contains(admitted_process.stdout_text, "\"executionAdmitted\": true",
        "#1319: builder execution JSON should expose execution admission");
    expect_contains(admitted_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1319: builder execution JSON should expose dispatch admission");
    expect_contains(admitted_process.stdout_text, "\"executed\": true",
        "#1319: admitted builder execution JSON should mark execution complete");
    expect_contains(admitted_process.stdout_text, "\"dryRun\": false",
        "#1319: admitted builder execution JSON should not be dry-run");
    expect_contains(admitted_process.stdout_text, "\"mutatesAsset\": false",
        "#1319: builder execution JSON should remain non-mutating");

    const auto selection_process = run_process_capture(
        studio_host_path,
        {
            "--builder-execute", "label-wizard",
            "--selection-context", "label_expression",
            "--path", "labels/mailing.lbx",
            "--record", "2",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "true",
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(selection_process.exit_code == 0,
        "#1319: builder execution JSON should accept selection-context builders");
    expect_contains(selection_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#1319: selection-context builder execution JSON should expose Studio selection contexts");
    expect_contains(selection_process.stdout_text, "\"builderId\": \"label-wizard\"",
        "#1319: selection-context builder execution JSON should expose wizard ids");
    expect_contains(selection_process.stdout_text, "\"kind\": \"wizard\"",
        "#1319: selection-context builder execution JSON should preserve wizard kind metadata");
    expect_contains(selection_process.stdout_text, "\"executionReadyBuilderIds\": [\"label-wizard\"]",
        "#1394: selection-context builder execution JSON should summarize execution-ready builder ids");
    expect_contains(selection_process.stdout_text, "\"executionBlockedBuilderIds\": []",
        "#1394: selection-context builder execution JSON should summarize empty blocked builder ids");
    expect_contains(selection_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1394: selection-context builder execution JSON should summarize empty execution errors");
    expect_contains(selection_process.stdout_text, "\"executed\": true",
        "#1319: selection-context builder execution JSON should mark execution complete");

    const auto missing_command_process = run_process_capture(
        studio_host_path,
        {
            "--builder-execute", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "true",
            "--json"
        },
        temp_root);
    expect(missing_command_process.exit_code == 2,
        "#1319: builder execution JSON should reject missing launch commands");
    expect_contains(missing_command_process.stdout_text, "No builder launch command was provided.",
        "#1319: missing builder execution launch commands should report parser errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--builder-execute", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "false",
            "--admit-builder-execution", "true",
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 4,
        "#1319: builder execution JSON should reject dry-run dispatch requests");
    expect_contains(dry_run_process.stdout_text, "\"builderExecution\": null",
        "#1319: dry-run builder execution JSON should not expose a result object");
    expect_contains(dry_run_process.stdout_text,
        "A builder dispatch request requires an admitted non-dry-run invocation.",
        "#1319: dry-run builder execution JSON should report dispatch admission errors");

    const auto unadmitted_execution_process = run_process_capture(
        studio_host_path,
        {
            "--builder-execute", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "false",
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(unadmitted_execution_process.exit_code == 4,
        "#1319: builder execution JSON should require explicit execution admission");
    expect_contains(unadmitted_execution_process.stdout_text,
        "A builder dispatch execution request requires explicit execution admission.",
        "#1319: unadmitted builder execution JSON should report execution admission errors");
    expect_contains(unadmitted_execution_process.stdout_text, "\"executed\": false",
        "#1319: unadmitted builder execution JSON should not mark execution complete");

    const auto failed_command_process = run_process_capture(
        studio_host_path,
        {
            "--builder-execute", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "true",
            "--builder-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(failed_command_process.exit_code == 4,
        "#1319: builder execution JSON should report nonzero process exits");
    expect_contains(failed_command_process.stdout_text,
        "Builder launch command returned a non-zero exit code.",
        "#1319: failed builder execution JSON should report process errors");
    expect_contains(failed_command_process.stdout_text, "\"observedExitCode\": 1",
        "#1347: failed builder execution JSON should report normalized child exit codes");
    expect_contains(failed_command_process.stdout_text, "\"executed\": false",
        "#1319: failed builder execution JSON should not mark execution complete");

    const auto ambiguous_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-execute", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(ambiguous_context_process.exit_code == 2,
        "#1319: builder execution JSON should reject simultaneous builder and selection contexts");
    expect_contains(ambiguous_context_process.stdout_text,
        "Builder execute requests cannot provide both --builder-context and --selection-context.",
        "#1319: ambiguous builder execution JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_builder_dispatch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_builder_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto control_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-catalog",
            "--builder-context", "control",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "grdOrders",
            "--unique-id", "grid-guid",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(control_process.exit_code == 0,
        "#1232: builder dispatch catalog JSON should accept admitted control catalogs");
    expect_contains(control_process.stdout_text, "\"builderDispatchCatalog\": {",
        "#1232: builder dispatch catalog JSON should expose a catalog object");
    expect_contains(control_process.stdout_text, "\"context\": \"control\"",
        "#1232: builder dispatch catalog JSON should expose builder contexts");
    expect_contains(control_process.stdout_text, "\"builderCount\": 2",
        "#1232: control builder dispatch catalog JSON should expose builder counts");
    expect_contains(control_process.stdout_text, "\"dispatchCount\": 2",
        "#1232: admitted control builder dispatch catalog JSON should expose dispatch counts");
    expect_contains(control_process.stdout_text, "\"errorCount\": 0",
        "#1232: admitted control builder dispatch catalog JSON should expose error counts");
    expect_contains(control_process.stdout_text, "\"dryRun\": false",
        "#1232: admitted builder dispatch catalog JSON should not be dry-run");
    expect_contains(control_process.stdout_text, "\"mutatesAsset\": false",
        "#1232: builder dispatch catalog JSON should remain non-mutating");
    expect_contains(control_process.stdout_text, "\"dispatchReadyBuilderIds\": [\"control-builder\", \"grid-builder\"]",
        "#1362: builder dispatch catalog JSON should summarize dispatch-ready builders");
    expect_contains(control_process.stdout_text, "\"dispatchBlockedBuilderIds\": []",
        "#1362: admitted builder dispatch catalog JSON should summarize empty blocked builder ids");
    expect_contains(control_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1362: admitted builder dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(control_process.stdout_text, "\"entries\": [",
        "#1232: builder dispatch catalog JSON should expose per-builder entries");
    expect_contains(control_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1232: builder dispatch catalog JSON should include grid builders");
    expect_contains(control_process.stdout_text, "\"kind\": \"builder\"",
        "#1232: builder dispatch catalog JSON should expose builder kind metadata");
    expect_contains(control_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1232: builder dispatch catalog JSON should expose command tokens");
    expect_contains(control_process.stdout_text, "\"entryPoint\": \"cf_builders.grid_builder\"",
        "#1232: builder dispatch catalog JSON should expose entry points");
    expect_contains(control_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1232: builder dispatch catalog JSON should preserve asset paths");
    expect_contains(control_process.stdout_text, "\"recordIndex\": 4",
        "#1232: builder dispatch catalog JSON should preserve record indexes");
    expect_contains(control_process.stdout_text, "\"objectName\": \"grdOrders\"",
        "#1232: builder dispatch catalog JSON should preserve object names");
    expect_contains(control_process.stdout_text, "\"uniqueId\": \"grid-guid\"",
        "#1232: builder dispatch catalog JSON should preserve unique ids");
    expect_contains(control_process.stdout_text, "\"dispatchArguments\": [",
        "#1232: builder dispatch catalog JSON should expose dispatch arguments");
    expect_contains(control_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1232: builder dispatch catalog JSON should expose admitted dispatch state");
    expect_contains(control_process.stdout_text, "\"executed\": false",
        "#1232: builder dispatch catalog JSON should not execute builder processes");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-catalog",
            "--builder-context", "control",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1232: builder dispatch catalog JSON should report dry-run per-builder rejections");
    expect_contains(dry_run_process.stdout_text, "\"dispatchCount\": 0",
        "#1232: dry-run builder dispatch catalog JSON should expose zero dispatch count");
    expect_contains(dry_run_process.stdout_text, "\"errorCount\": 2",
        "#1232: dry-run builder dispatch catalog JSON should expose per-builder error counts");
    expect_contains(dry_run_process.stdout_text, "\"dryRun\": true",
        "#1232: dry-run builder dispatch catalog JSON should expose aggregate dry-run state");
    expect_contains(dry_run_process.stdout_text, "\"dispatchReadyBuilderIds\": []",
        "#1362: dry-run builder dispatch catalog JSON should summarize empty dispatch-ready builders");
    expect_contains(dry_run_process.stdout_text,
        "\"dispatchBlockedBuilderIds\": [\"control-builder\", \"grid-builder\"]",
        "#1362: dry-run builder dispatch catalog JSON should summarize blocked builders");
    expect_contains(dry_run_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A builder dispatch request requires an admitted non-dry-run invocation.\"",
        "#1362: dry-run builder dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_process.stdout_text,
        "A builder dispatch request requires an admitted non-dry-run invocation.",
        "#1232: dry-run builder dispatch catalog JSON should expose dispatch errors");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-catalog",
            "--builder-context", "label",
            "--path", "labels/mailing.lbx",
            "--admit-ui-launch", "true",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#1232: builder dispatch catalog JSON should accept admitted label catalogs");
    expect_contains(label_process.stdout_text, "\"context\": \"label\"",
        "#1232: label dispatch catalog JSON should expose builder contexts");
    expect_contains(label_process.stdout_text, "\"builderId\": \"label-wizard\"",
        "#1232: label dispatch catalog JSON should include label wizards");
    expect_contains(label_process.stdout_text, "\"kind\": \"wizard\"",
        "#1232: label dispatch catalog JSON should expose wizard metadata");
    expect_contains(label_process.stdout_text, "\"entryPoint\": \"cf_wizards.label_wizard\"",
        "#1232: label dispatch catalog JSON should expose wizard entry points");

    const auto invalid_bool_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-catalog",
            "--builder-context", "control",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_bool_process.exit_code == 2,
        "#1232: builder dispatch catalog JSON should reject invalid UI-admission booleans");
    expect_contains(invalid_bool_process.stdout_text, "The --admit-ui-launch value must be true or false.",
        "#1232: invalid builder dispatch catalog UI-admission JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-catalog",
            "--builder-context", "control",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1232: builder dispatch catalog JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1232: invalid builder dispatch catalog record JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1232: builder dispatch catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No builder context was provided.",
        "#1232: missing-context builder dispatch catalog JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_builder_dispatch_execution_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_builder_dispatch_execution_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto admitted_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-execution-catalog",
            "--builder-context", "control",
            "--path", "forms/customer.scx",
            "--record", "4",
            "--object-name", "grdOrders",
            "--unique-id", "grid-guid",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "true",
            "--json"
        },
        temp_root);
    expect(admitted_process.exit_code == 0,
        "#1327: builder dispatch execution catalog JSON should accept admitted control catalogs");
    expect_contains(admitted_process.stdout_text, "\"builderDispatchExecutionCatalog\": {",
        "#1327: builder dispatch execution catalog JSON should expose a catalog object");
    expect_contains(admitted_process.stdout_text, "\"context\": \"control\"",
        "#1327: builder dispatch execution catalog JSON should expose builder contexts");
    expect_contains(admitted_process.stdout_text, "\"builderCount\": 2",
        "#1327: builder dispatch execution catalog JSON should expose builder counts");
    expect_contains(admitted_process.stdout_text, "\"executionReadyCount\": 2",
        "#1327: admitted builder dispatch execution catalog JSON should expose readiness counts");
    expect_contains(admitted_process.stdout_text, "\"errorCount\": 0",
        "#1327: admitted builder dispatch execution catalog JSON should expose zero errors");
    expect_contains(admitted_process.stdout_text, "\"dryRun\": false",
        "#1327: admitted builder dispatch execution catalog JSON should not be dry-run");
    expect_contains(admitted_process.stdout_text,
        "\"executionReadyBuilderIds\": [\"control-builder\", \"grid-builder\"]",
        "#1363: admitted builder dispatch execution catalog JSON should summarize execution-ready builders");
    expect_contains(admitted_process.stdout_text, "\"executionBlockedBuilderIds\": []",
        "#1363: admitted builder dispatch execution catalog JSON should summarize empty blocked builder ids");
    expect_contains(admitted_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1363: admitted builder dispatch execution catalog JSON should summarize empty blocked execution errors");
    expect_contains(admitted_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1327: builder dispatch execution catalog JSON should expose builder ids");
    expect_contains(admitted_process.stdout_text, "\"launchOk\": true",
        "#1327: builder dispatch execution catalog JSON should expose launch readiness");
    expect_contains(admitted_process.stdout_text, "\"admissionOk\": true",
        "#1327: builder dispatch execution catalog JSON should expose invocation admission readiness");
    expect_contains(admitted_process.stdout_text, "\"dispatchOk\": true",
        "#1327: builder dispatch execution catalog JSON should expose dispatch readiness");
    expect_contains(admitted_process.stdout_text, "\"executionAdmitted\": true",
        "#1327: builder dispatch execution catalog JSON should expose execution admission state");
    expect_contains(admitted_process.stdout_text, "\"executionReady\": true",
        "#1327: builder dispatch execution catalog JSON should expose execution readiness");
    expect_contains(admitted_process.stdout_text, "\"executionError\": \"\"",
        "#1327: admitted builder dispatch execution catalog JSON should expose empty execution errors");
    expect_contains(admitted_process.stdout_text, "\"entryPoint\": \"cf_builders.grid_builder\"",
        "#1327: builder dispatch execution catalog JSON should preserve dispatch entry points");
    expect_contains(admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1327: builder dispatch execution catalog JSON should expose dispatch arguments");
    expect_contains(admitted_process.stdout_text, "\"executed\": false",
        "#1327: builder dispatch execution catalog JSON should not launch builders");

    const auto unadmitted_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-execution-catalog",
            "--builder-context", "control",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "false",
            "--json"
        },
        temp_root);
    expect(unadmitted_process.exit_code == 0,
        "#1327: builder dispatch execution catalog JSON should report unadmitted execution as catalog errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionReadyCount\": 0",
        "#1327: unadmitted builder dispatch execution catalog JSON should expose zero readiness");
    expect_contains(unadmitted_process.stdout_text, "\"errorCount\": 2",
        "#1327: unadmitted builder dispatch execution catalog JSON should expose per-builder errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionAdmitted\": false",
        "#1327: unadmitted builder dispatch execution catalog JSON should expose admission false");
    expect_contains(unadmitted_process.stdout_text, "\"executionReadyBuilderIds\": []",
        "#1363: unadmitted builder dispatch execution catalog JSON should summarize empty ready builder ids");
    expect_contains(unadmitted_process.stdout_text,
        "\"executionBlockedBuilderIds\": [\"control-builder\", \"grid-builder\"]",
        "#1363: unadmitted builder dispatch execution catalog JSON should summarize blocked builder ids");
    expect_contains(unadmitted_process.stdout_text,
        "\"executionBlockedErrors\": [\"A builder dispatch execution catalog entry requires explicit execution admission.\"",
        "#1363: unadmitted builder dispatch execution catalog JSON should summarize blocked execution errors");
    expect_contains(unadmitted_process.stdout_text,
        "A builder dispatch execution catalog entry requires explicit execution admission.",
        "#1327: unadmitted builder dispatch execution catalog JSON should expose execution errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-execution-catalog",
            "--builder-context", "control",
            "--admit-builder-execution", "true",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1327: builder dispatch execution catalog JSON should report dry-run dispatch failures");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyCount\": 0",
        "#1327: dry-run builder dispatch execution catalog JSON should expose zero readiness");
    expect_contains(dry_run_process.stdout_text, "\"errorCount\": 2",
        "#1327: dry-run builder dispatch execution catalog JSON should expose per-builder errors");
    expect_contains(dry_run_process.stdout_text,
        "\"executionBlockedBuilderIds\": [\"control-builder\", \"grid-builder\"]",
        "#1363: dry-run builder dispatch execution catalog JSON should summarize dispatch-blocked builder ids");
    expect_contains(dry_run_process.stdout_text,
        "\"executionBlockedErrors\": [\"A builder dispatch request requires an admitted non-dry-run invocation.\"",
        "#1363: dry-run builder dispatch execution catalog JSON should summarize dispatch-blocked errors");
    expect_contains(dry_run_process.stdout_text,
        "A builder dispatch request requires an admitted non-dry-run invocation.",
        "#1327: dry-run builder dispatch execution catalog JSON should expose dispatch errors");

    const auto invalid_execution_bool_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-execution-catalog",
            "--builder-context", "control",
            "--admit-builder-execution", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_execution_bool_process.exit_code == 2,
        "#1327: builder dispatch execution catalog JSON should reject invalid execution-admission booleans");
    expect_contains(invalid_execution_bool_process.stdout_text,
        "The --admit-builder-execution value must be true or false.",
        "#1327: invalid builder execution catalog admission JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--builder-dispatch-execution-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1327: builder dispatch execution catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No builder context was provided.",
        "#1327: missing-context builder dispatch execution catalog JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
