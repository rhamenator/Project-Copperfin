// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#include "test_studio_host_json_dispatch_toolbox_palette_launch_plan.inl"
#include "test_studio_host_json_dispatch_toolbox_palette_query_filters.inl"
#include "test_studio_host_json_dispatch_toolbox_palette_launch_catalog.inl"
#include "test_studio_host_json_dispatch_toolbox_palette_creation_plan.inl"

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

#include "test_studio_host_json_dispatch_toolbox_palette_creation_batch.inl"

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
