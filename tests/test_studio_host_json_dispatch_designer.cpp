// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_preserves_sidecar_path_spelling(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_json_sidecar_spelling";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "sample.scx";
    const fs::path inferred_sidecar_path = temp_root / "sample.sct";
    const fs::path temporary_sidecar_path = temp_root / "sidecar.rename";
    const fs::path actual_sidecar_path = temp_root / "Sample.SCT";
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        form_path.string(),
        {{.name = "OBJNAME", .type = 'M', .length = 4U}},
        {{"sample-object"}});
    expect(create_result.ok, "#3992: mixed-case sidecar JSON fixture should be created");
    fs::rename(inferred_sidecar_path, temporary_sidecar_path, ignored);
    ignored.clear();
    fs::rename(temporary_sidecar_path, actual_sidecar_path, ignored);

    const auto process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--json"},
        temp_root);
    if (process.exit_code != 0) {
        std::cerr << "studio host sidecar spelling stdout:\n" << process.stdout_text << "\n";
        std::cerr << "studio host sidecar spelling stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "#3992: mixed-case sidecar JSON smoke should exit successfully");
    const std::size_t sidecar_key = process.stdout_text.find("\"sidecarPath\":");
    const std::size_t sidecar_line_end = sidecar_key == std::string::npos
        ? std::string::npos
        : process.stdout_text.find('\n', sidecar_key);
    const std::string sidecar_line = sidecar_key == std::string::npos
        ? std::string{}
        : process.stdout_text.substr(sidecar_key, sidecar_line_end - sidecar_key);
    expect(sidecar_line.find(actual_sidecar_path.filename().string()) != std::string::npos &&
               sidecar_line.find(inferred_sidecar_path.filename().string()) == std::string::npos,
           "#3992: sidecarPath JSON should preserve actual directory-entry spelling");
    expect_contains(process.stdout_text, "\"hasSidecar\": true",
                    "#3992: mixed-case sidecar JSON should report companion availability");

    fs::remove_all(temp_root, ignored);
}

#include "test_studio_host_json_dispatch_designer_contexts.inl"


void test_studio_host_json_exposes_designer_launch_surfaces(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_designer_launch_surfaces_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1212: designer launch-surface JSON should accept visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"designerLaunchSurfaces\": {",
        "#1212: designer launch-surface JSON should expose a plan object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1212: designer launch-surface JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1212: designer launch-surface JSON should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1212: designer launch-surface JSON should preserve record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1212: designer launch-surface JSON should preserve object names");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1212: designer launch-surface JSON should preserve unique ids");
    expect_contains(visual_process.stdout_text, "\"symbol\": \"Click\"",
        "#1212: designer launch-surface JSON should preserve editor symbols");
    expect_contains(visual_process.stdout_text, "\"line\": 12",
        "#1212: designer launch-surface JSON should preserve editor lines");
    expect_contains(visual_process.stdout_text, "\"column\": 4",
        "#1212: designer launch-surface JSON should preserve editor columns");
    expect_contains(visual_process.stdout_text, "\"launchReadySelectionContexts\": [\"visual_object\"]",
        "#1399: designer launch-surface JSON should summarize launch-ready selected contexts");
    expect_contains(visual_process.stdout_text, "\"launchBlockedSelectionContexts\": []",
        "#1399: designer launch-surface JSON should expose empty blocked selected contexts for ready launches");
    expect_contains(visual_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1399: designer launch-surface JSON should expose empty blocked errors for ready launches");
    expect_contains(visual_process.stdout_text, "\"editorActionLaunchPlanCount\": ",
        "#1212: designer launch-surface JSON should expose action launch-plan counts");
    expect_contains(visual_process.stdout_text, "\"builderLaunchPlanCount\": ",
        "#1212: designer launch-surface JSON should expose builder launch-plan counts");
    expect_contains(visual_process.stdout_text, "\"toolboxAvailable\": true",
        "#1212: designer launch-surface JSON should expose toolbox availability");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"show-property-grid\"",
        "#1212: designer launch-surface JSON should include property-grid action plans");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"show-toolbox\"",
        "#1212: designer launch-surface JSON should include toolbox action plans");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1212: designer launch-surface JSON should include form builder plans");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1212: designer launch-surface JSON should include control builder plans");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1212: designer launch-surface JSON should include resolved toolbox contexts");
    expect_contains(visual_process.stdout_text, "\"id\": \"textbox\"",
        "#1212: designer launch-surface JSON should include toolbox item descriptors");

    const auto menu_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--selection-context", "menu_item",
            "--path", "menus/main.mnx",
            "--record", "5",
            "--object-name", "FileExit",
            "--unique-id", "menu-guid",
            "--json"
        },
        temp_root);
    expect(menu_process.exit_code == 0,
        "#1212: designer launch-surface JSON should keep unsupported toolbox contexts as aggregate successes");
    expect_contains(menu_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1212: menu launch-surface JSON should expose selected Studio contexts");
    expect_contains(menu_process.stdout_text, "\"toolboxAvailable\": false",
        "#1212: menu launch-surface JSON should expose unsupported toolbox availability");
    expect_contains(menu_process.stdout_text, "\"toolboxPaletteLaunchPlan\": null",
        "#1212: menu launch-surface JSON should expose null toolbox plans");
    expect_contains(menu_process.stdout_text, "\"launchReadySelectionContexts\": []",
        "#1399: menu launch-surface JSON should expose empty ready selected contexts");
    expect_contains(menu_process.stdout_text, "\"launchBlockedSelectionContexts\": [\"menu_item\"]",
        "#1399: menu launch-surface JSON should summarize launch-blocked selected contexts");
    expect_contains(menu_process.stdout_text,
        "\"launchBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"]",
        "#1399: menu launch-surface JSON should summarize blocked selected-context reasons");
    expect_contains(menu_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1212: menu launch-surface JSON should preserve unsupported toolbox reasons");
    expect_contains(menu_process.stdout_text, "\"actionId\": \"show-property-grid\"",
        "#1212: menu launch-surface JSON should still include supported editor actions");
    expect_contains(menu_process.stdout_text, "\"actionId\": \"edit-menu-command\"",
        "#1413: menu launch-surface JSON should include menu command editor actions");
    expect_contains(menu_process.stdout_text, "\"builderId\": \"menu-designer\"",
        "#1212: menu launch-surface JSON should still include supported builders");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1212: designer launch-surface JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "\"designerLaunchSurfaces\": null",
        "#1212: unknown context JSON should not expose a plan object");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1212: unknown designer launch-surface context JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1212: designer launch-surface JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1212: missing designer launch-surface context JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1212: designer launch-surface JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1212: invalid designer launch-surface line JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_invocation_admission(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_invocation_admission_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1222: designer invocation-admission JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"designerInvocationAdmission\": {",
        "#1222: designer invocation-admission JSON should expose a plan object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1222: designer invocation-admission JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1222: designer invocation-admission JSON should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1222: designer invocation-admission JSON should preserve record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1222: designer invocation-admission JSON should preserve object names");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1222: designer invocation-admission JSON should preserve unique ids");
    expect_contains(visual_process.stdout_text, "\"symbol\": \"Click\"",
        "#1222: designer invocation-admission JSON should preserve editor symbols");
    expect_contains(visual_process.stdout_text, "\"line\": 12",
        "#1222: designer invocation-admission JSON should preserve editor lines");
    expect_contains(visual_process.stdout_text, "\"column\": 4",
        "#1222: designer invocation-admission JSON should preserve editor columns");
    expect_contains(visual_process.stdout_text, "\"admissionOkSelectionContexts\": [\"visual_object\"]",
        "#1400: designer invocation-admission JSON should summarize admission-clean selected contexts");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedSelectionContexts\": []",
        "#1400: designer invocation-admission JSON should expose empty blocked selected contexts for clean admission");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1400: designer invocation-admission JSON should expose empty blocked errors for clean admission");
    expect_contains(visual_process.stdout_text, "\"editorActionInvocationCount\": ",
        "#1222: designer invocation-admission JSON should expose editor invocation counts");
    expect_contains(visual_process.stdout_text, "\"builderInvocationCount\": ",
        "#1222: designer invocation-admission JSON should expose builder invocation counts");
    expect_contains(visual_process.stdout_text, "\"toolboxAvailable\": true",
        "#1222: designer invocation-admission JSON should expose toolbox availability");
    expect_contains(visual_process.stdout_text, "\"toolboxItemCount\": ",
        "#1222: designer invocation-admission JSON should expose toolbox item counts");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1222: designer invocation-admission JSON should include admitted editor actions");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1222: designer invocation-admission JSON should include admitted builders");
    expect_contains(visual_process.stdout_text, "\"editorInvocationAdmitted\": true",
        "#1222: admitted designer invocation-admission JSON should admit editor invocations");
    expect_contains(visual_process.stdout_text, "\"uiLaunchAdmitted\": true",
        "#1222: admitted designer invocation-admission JSON should admit builder invocations");
    expect_contains(visual_process.stdout_text, "\"paletteInvocationAdmitted\": true",
        "#1222: admitted designer invocation-admission JSON should admit toolbox invocation");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1222: admitted designer invocation-admission JSON should not be aggregate dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1222: designer invocation-admission JSON should remain non-mutating");

    const auto menu_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "menu_item",
            "--path", "menus/main.mnx",
            "--record", "5",
            "--object-name", "FileExit",
            "--unique-id", "menu-guid",
            "--json"
        },
        temp_root);
    expect(menu_process.exit_code == 0,
        "#1222: designer invocation-admission JSON should keep unsupported toolbox contexts as aggregate successes");
    expect_contains(menu_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1222: menu designer invocation-admission JSON should expose selected Studio contexts");
    expect_contains(menu_process.stdout_text, "\"toolboxAvailable\": false",
        "#1222: menu designer invocation-admission JSON should expose unsupported toolbox availability");
    expect_contains(menu_process.stdout_text, "\"toolboxInvocation\": null",
        "#1222: menu designer invocation-admission JSON should expose null toolbox invocation");
    expect_contains(menu_process.stdout_text, "\"admissionOkSelectionContexts\": []",
        "#1400: menu invocation-admission JSON should expose empty clean selected contexts");
    expect_contains(menu_process.stdout_text, "\"admissionBlockedSelectionContexts\": [\"menu_item\"]",
        "#1400: menu invocation-admission JSON should summarize admission-blocked selected contexts");
    expect_contains(menu_process.stdout_text,
        "\"admissionBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"]",
        "#1400: menu invocation-admission JSON should summarize blocked selected-context reasons");
    expect_contains(menu_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1222: menu designer invocation-admission JSON should preserve unsupported toolbox reasons");
    expect_contains(menu_process.stdout_text, "\"actionId\": \"show-property-grid\"",
        "#1222: menu designer invocation-admission JSON should still include editor actions");
    expect_contains(menu_process.stdout_text, "\"actionId\": \"edit-menu-command\"",
        "#1413: menu designer invocation-admission JSON should include menu command editor actions");
    expect_contains(menu_process.stdout_text, "\"builderId\": \"menu-designer\"",
        "#1222: menu designer invocation-admission JSON should still include builders");
    expect_contains(menu_process.stdout_text, "\"editorInvocationAdmitted\": false",
        "#1222: default designer invocation-admission JSON should not admit editor invocations");
    expect_contains(menu_process.stdout_text, "\"uiLaunchAdmitted\": false",
        "#1222: default designer invocation-admission JSON should not admit builder invocations");
    expect_contains(menu_process.stdout_text, "\"dryRun\": true",
        "#1222: default designer invocation-admission JSON should expose dry-run state");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1222: designer invocation-admission JSON should reject invalid editor admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-editor-invocations value must be true or false.",
        "#1222: invalid designer invocation-admission boolean JSON should report parser errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1222: designer invocation-admission JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "\"designerInvocationAdmission\": null",
        "#1222: unknown designer invocation-admission context JSON should not expose a plan object");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1222: unknown designer invocation-admission context JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1222: designer invocation-admission JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1222: invalid designer invocation-admission line JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1222: designer invocation-admission JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1222: missing designer invocation-admission context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#include "test_studio_host_json_dispatch_designer_dispatch.inl"


void test_studio_host_json_exposes_designer_dispatch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1240: designer dispatch catalog JSON should accept aggregate admission policies");
    expect_contains(catalog_process.stdout_text, "\"designerDispatchCatalog\": {",
        "#1240: designer dispatch catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"contextCount\": 9",
        "#1240: designer dispatch catalog JSON should expose context counts");
    expect_contains(catalog_process.stdout_text, "\"dispatchOkSelectionContexts\": [\"visual_object\"",
        "#1357: designer dispatch catalog JSON should summarize dispatch-clean contexts");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedSelectionContexts\": [\"menu_item\"",
        "#1357: designer dispatch catalog JSON should summarize dispatch-blocked contexts");
    expect_contains(catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"",
        "#1357: designer dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1240: designer dispatch catalog JSON should include visual-object contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1240: designer dispatch catalog JSON should include report-expression contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1240: designer dispatch catalog JSON should include menu-item contexts");
    expect_contains(catalog_process.stdout_text, "\"editorActionDispatchCount\": ",
        "#1240: designer dispatch catalog JSON should expose editor dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"builderDispatchCount\": ",
        "#1240: designer dispatch catalog JSON should expose builder dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"toolboxDispatchCount\": ",
        "#1240: designer dispatch catalog JSON should expose toolbox dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"dispatchCount\": ",
        "#1240: designer dispatch catalog JSON should expose aggregate dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": ",
        "#1240: designer dispatch catalog JSON should expose aggregate error counts");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1240: designer dispatch catalog JSON should expose non-dry-run admitted contexts");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1240: designer dispatch catalog JSON should remain non-mutating");
    expect_contains(catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1240: designer dispatch catalog JSON should expose nested aggregate dispatch status");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"edit-visual-method\"",
        "#1240: designer dispatch catalog JSON should expose admitted visual editor dispatch ids");
    expect_contains(catalog_process.stdout_text, "\"edit-report-expression\"",
        "#1240: designer dispatch catalog JSON should expose admitted report editor dispatch ids");
    expect_contains(catalog_process.stdout_text, "\"editorActionDispatches\": [",
        "#1336: designer dispatch catalog JSON should expose nested editor dispatch entries");
    expect_contains(catalog_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1336: designer dispatch catalog JSON should expose editor dispatch command tokens");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.method_editor.open\"",
        "#1336: designer dispatch catalog JSON should expose editor dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"builderDispatches\": [",
        "#1336: designer dispatch catalog JSON should expose nested builder dispatch entries");
    expect_contains(catalog_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1336: designer dispatch catalog JSON should expose builder dispatch command tokens");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.builder.invoke\"",
        "#1336: designer dispatch catalog JSON should expose builder dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"toolboxDispatchOk\": false",
        "#1240: designer dispatch catalog JSON should expose unsupported toolbox dispatch status");
    expect_contains(catalog_process.stdout_text, "\"toolboxCommandToken\": \"studio.toolbox.palette.invoke\"",
        "#1336: designer dispatch catalog JSON should expose toolbox dispatch command tokens");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxDispatchArguments\": [\"--command-token\", \"studio.toolbox.palette.invoke\"",
        "#1336: designer dispatch catalog JSON should expose toolbox dispatch arguments");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1240: designer dispatch catalog JSON should preserve unsupported toolbox reasons");

    const auto unadmitted_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(unadmitted_process.exit_code == 0,
        "#1357: designer dispatch catalog JSON should summarize unadmitted dispatch-blocked contexts");
    expect_contains(unadmitted_process.stdout_text, "\"dispatchOkSelectionContexts\": []",
        "#1357: unadmitted designer dispatch catalog JSON should summarize no dispatch-clean contexts");
    expect_contains(unadmitted_process.stdout_text, "\"dispatchBlockedSelectionContexts\": [\"visual_object\"",
        "#1357: unadmitted designer dispatch catalog JSON should summarize dispatch-blocked contexts");
    expect_contains(unadmitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"An editor action dispatch request requires an admitted non-dry-run invocation.\"",
        "#1357: unadmitted designer dispatch catalog JSON should summarize child dispatch errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-catalog",
            "--admit-toolbox-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1240: designer dispatch catalog JSON should reject invalid toolbox admission booleans");
    expect_contains(invalid_boolean_process.stdout_text, "\"designerDispatchCatalog\": null",
        "#1240: invalid designer dispatch catalog boolean JSON should not expose catalog objects");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-toolbox-invocation value must be true or false.",
        "#1240: invalid designer dispatch catalog boolean JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-catalog",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1240: designer dispatch catalog JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1240: invalid designer dispatch catalog line JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_dispatch_execution_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_dispatch_execution_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-execution-catalog",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1333: designer dispatch execution catalog JSON should accept admitted aggregate policies");
    expect_contains(catalog_process.stdout_text, "\"designerDispatchExecutionCatalog\": {",
        "#1333: designer dispatch execution catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"contextCount\": 9",
        "#1333: designer dispatch execution catalog JSON should expose context counts");
    expect_contains(catalog_process.stdout_text, "\"executionReadyCount\": ",
        "#1333: designer dispatch execution catalog JSON should expose readiness counts");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": ",
        "#1333: designer dispatch execution catalog JSON should expose error counts");
    expect_contains(catalog_process.stdout_text, "\"readySelectionContexts\": [\"visual_object\"",
        "#1356: designer dispatch execution catalog JSON should summarize ready contexts");
    expect_contains(catalog_process.stdout_text, "\"blockedSelectionContexts\": [\"menu_item\"",
        "#1356: designer dispatch execution catalog JSON should summarize blocked contexts");
    expect_contains(catalog_process.stdout_text,
        "\"blockedExecutionErrors\": [\"A designer dispatch execution catalog entry requires an error-free dispatch plan.\"",
        "#1356: designer dispatch execution catalog JSON should summarize blocked context errors");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1333: admitted designer dispatch execution catalog JSON should expose non-dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1333: designer dispatch execution catalog JSON should remain non-mutating");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1333: designer dispatch execution catalog JSON should include visual-object contexts");
    expect_contains(catalog_process.stdout_text, "\"dispatchErrorCount\": 0",
        "#1333: visual-object execution catalog JSON should expose clean dispatch plans");
    expect_contains(catalog_process.stdout_text, "\"executionAdmitted\": true",
        "#1333: designer dispatch execution catalog JSON should expose execution admission");
    expect_contains(catalog_process.stdout_text, "\"executionReady\": true",
        "#1333: designer dispatch execution catalog JSON should expose ready contexts");
    expect_contains(catalog_process.stdout_text, "\"executionError\": \"\"",
        "#1333: admitted ready designer dispatch execution catalog JSON should expose empty execution errors");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"edit-visual-method\"",
        "#1333: designer dispatch execution catalog JSON should preserve editor action dispatch ids");
    expect_contains(catalog_process.stdout_text, "\"editorActionDispatches\": [",
        "#1334: designer dispatch execution catalog JSON should expose nested editor dispatch entries");
    expect_contains(catalog_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1334: designer dispatch execution catalog JSON should expose editor dispatch command tokens");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.method_editor.open\"",
        "#1334: designer dispatch execution catalog JSON should expose editor dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"form-builder\"",
        "#1333: designer dispatch execution catalog JSON should preserve builder dispatch ids");
    expect_contains(catalog_process.stdout_text, "\"builderDispatches\": [",
        "#1334: designer dispatch execution catalog JSON should expose nested builder dispatch entries");
    expect_contains(catalog_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1334: designer dispatch execution catalog JSON should expose builder dispatch command tokens");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.builder.invoke\"",
        "#1334: designer dispatch execution catalog JSON should expose builder dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"toolboxDispatchOk\": true",
        "#1333: designer dispatch execution catalog JSON should expose toolbox dispatch status");
    expect_contains(catalog_process.stdout_text, "\"toolboxCommandToken\": \"studio.toolbox.palette.invoke\"",
        "#1334: designer dispatch execution catalog JSON should expose toolbox dispatch command tokens");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxDispatchArguments\": [\"--command-token\", \"studio.toolbox.palette.invoke\"",
        "#1334: designer dispatch execution catalog JSON should expose toolbox dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1333: designer dispatch execution catalog JSON should include menu contexts");
    expect_contains(catalog_process.stdout_text,
        "A designer dispatch execution catalog entry requires an error-free dispatch plan.",
        "#1333: designer dispatch execution catalog JSON should propagate dispatch-plan errors");

    const auto unadmitted_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-execution-catalog",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "false",
            "--json"
        },
        temp_root);
    expect(unadmitted_process.exit_code == 0,
        "#1333: designer dispatch execution catalog JSON should report unadmitted execution as catalog errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionReadyCount\": 0",
        "#1333: unadmitted designer dispatch execution catalog JSON should expose zero readiness");
    expect_contains(unadmitted_process.stdout_text, "\"readySelectionContexts\": []",
        "#1356: unadmitted designer dispatch execution catalog JSON should summarize no ready contexts");
    expect_contains(unadmitted_process.stdout_text, "\"blockedSelectionContexts\": [\"visual_object\"",
        "#1356: unadmitted designer dispatch execution catalog JSON should summarize blocked contexts");
    expect_contains(unadmitted_process.stdout_text,
        "\"blockedExecutionErrors\": [\"A designer dispatch execution catalog entry requires explicit execution admission.\"",
        "#1356: unadmitted designer dispatch execution catalog JSON should summarize blocked errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionAdmitted\": false",
        "#1333: unadmitted designer dispatch execution catalog JSON should expose admission false");
    expect_contains(unadmitted_process.stdout_text,
        "A designer dispatch execution catalog entry requires explicit execution admission.",
        "#1333: unadmitted designer dispatch execution catalog JSON should expose execution errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-execution-catalog",
            "--admit-designer-execution", "true",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1333: designer dispatch execution catalog JSON should report dry-run aggregate failures");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyCount\": 0",
        "#1333: dry-run designer dispatch execution catalog JSON should expose zero readiness");
    expect_contains(dry_run_process.stdout_text,
        "A designer dispatch execution catalog entry requires at least one admitted dispatch.",
        "#1333: dry-run designer dispatch execution catalog JSON should expose aggregate preflight errors");

    const auto invalid_execution_bool_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-execution-catalog",
            "--admit-designer-execution", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_execution_bool_process.exit_code == 2,
        "#1333: designer dispatch execution catalog JSON should reject invalid execution booleans");
    expect_contains(invalid_execution_bool_process.stdout_text,
        "The --admit-designer-execution value must be true or false.",
        "#1333: invalid designer execution catalog admission JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_launch_surface_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_launch_surface_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surface-catalog",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1214: designer launch-surface catalog JSON should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"designerLaunchSurfaceCatalog\": {",
        "#1214: designer launch-surface catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"contextCount\": 9",
        "#1214: designer launch-surface catalog JSON should expose all Studio contexts");
    expect_contains(catalog_process.stdout_text, "\"launchReadySelectionContexts\": [\"visual_object\"",
        "#1359: designer launch-surface catalog JSON should summarize launch-ready contexts");
    expect_contains(catalog_process.stdout_text, "\"launchBlockedSelectionContexts\": [\"menu_item\"",
        "#1359: designer launch-surface catalog JSON should summarize launch-blocked contexts");
    expect_contains(catalog_process.stdout_text,
        "\"launchBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"",
        "#1359: designer launch-surface catalog JSON should summarize blocked launch errors");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1214: designer launch-surface catalog JSON should include visual-object contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1214: designer launch-surface catalog JSON should include menu contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"project_item\"",
        "#1214: designer launch-surface catalog JSON should include project contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"data_environment\"",
        "#1214: designer launch-surface catalog JSON should include data-environment contexts");
    expect_contains(catalog_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1214: designer launch-surface catalog JSON should preserve asset paths in nested plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1214: designer launch-surface catalog JSON should preserve object names in nested plans");
    expect_contains(catalog_process.stdout_text, "\"symbol\": \"Click\"",
        "#1214: designer launch-surface catalog JSON should preserve editor symbols in nested plans");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"show-property-grid\"",
        "#1214: designer launch-surface catalog JSON should expose nested editor action ids");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"show-property-grid\", \"edit-menu-command\", \"open-builder\"]",
        "#1413: designer launch-surface catalog JSON should expose menu command editor action ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"form-builder\"",
        "#1214: designer launch-surface catalog JSON should expose nested builder ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"menu-designer\"]",
        "#1214: designer launch-surface catalog JSON should expose menu builder ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"application-wizard\"]",
        "#1214: designer launch-surface catalog JSON should expose project wizard ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"data-environment-builder\"]",
        "#1214: designer launch-surface catalog JSON should expose data-environment builder ids");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1214: designer launch-surface catalog JSON should expose visual toolbox context metadata");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1214: designer launch-surface catalog JSON should expose unsupported toolbox reasons");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surface-catalog",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1214: designer launch-surface catalog JSON should reject invalid records");
    expect_contains(invalid_record_process.stdout_text, "\"designerLaunchSurfaceCatalog\": null",
        "#1214: invalid catalog records should not expose a catalog object");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1214: invalid catalog record JSON should report parser errors");

    const auto invalid_column_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surface-catalog",
            "--column", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_column_process.exit_code == 2,
        "#1214: designer launch-surface catalog JSON should reject invalid columns");
    expect_contains(invalid_column_process.stdout_text, "The --column value must be a non-negative integer.",
        "#1214: invalid catalog column JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_invocation_admission_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_invocation_admission_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission-catalog",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "false",
            "--admit-toolbox-invocation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1224: designer invocation-admission catalog JSON should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"designerInvocationAdmissionCatalog\": {",
        "#1224: designer invocation-admission catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"contextCount\": 9",
        "#1224: designer invocation-admission catalog JSON should expose all Studio contexts");
    expect_contains(catalog_process.stdout_text, "\"admissionOkSelectionContexts\": [\"visual_object\"",
        "#1358: designer invocation-admission catalog JSON should summarize admission-clean contexts");
    expect_contains(catalog_process.stdout_text, "\"admissionBlockedSelectionContexts\": [\"menu_item\"",
        "#1358: designer invocation-admission catalog JSON should summarize admission-blocked contexts");
    expect_contains(catalog_process.stdout_text,
        "\"admissionBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"",
        "#1358: designer invocation-admission catalog JSON should summarize blocked admission errors");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1224: designer invocation-admission catalog JSON should include visual-object contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1224: designer invocation-admission catalog JSON should include report contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1224: designer invocation-admission catalog JSON should include menu contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"data_environment\"",
        "#1224: designer invocation-admission catalog JSON should include data-environment contexts");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"show-property-grid\"",
        "#1224: designer invocation-admission catalog JSON should expose nested editor action ids");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"show-property-grid\", \"edit-menu-command\", \"open-builder\"]",
        "#1413: designer invocation-admission catalog JSON should expose menu command editor action ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"form-builder\"",
        "#1224: designer invocation-admission catalog JSON should expose visual builder ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"menu-designer\"]",
        "#1224: designer invocation-admission catalog JSON should expose menu builder ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"data-environment-builder\"]",
        "#1224: designer invocation-admission catalog JSON should expose data-environment builder ids");
    expect_contains(catalog_process.stdout_text, "\"editorInvocationsAdmitted\": true",
        "#1224: designer invocation-admission catalog JSON should expose editor admission policy");
    expect_contains(catalog_process.stdout_text, "\"builderInvocationsAdmitted\": false",
        "#1224: designer invocation-admission catalog JSON should expose builder admission policy");
    expect_contains(catalog_process.stdout_text, "\"toolboxInvocationAdmitted\": true",
        "#1224: designer invocation-admission catalog JSON should expose toolbox admission policy");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1224: designer invocation-admission catalog JSON should expose unsupported toolbox reasons");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1224: designer invocation-admission catalog JSON should remain non-mutating");

    const auto unadmitted_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission-catalog",
            "--json"
        },
        temp_root);
    expect(unadmitted_process.exit_code == 0,
        "#1358: designer invocation-admission catalog JSON should summarize dry-run admission catalogs");
    expect_contains(unadmitted_process.stdout_text, "\"admissionOkSelectionContexts\": [\"visual_object\"",
        "#1358: dry-run designer invocation-admission catalog JSON should preserve admission-clean contexts");
    expect_contains(unadmitted_process.stdout_text, "\"admissionBlockedSelectionContexts\": [\"menu_item\"",
        "#1358: dry-run designer invocation-admission catalog JSON should summarize admission-blocked contexts");
    expect_contains(unadmitted_process.stdout_text,
        "\"admissionBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"",
        "#1358: dry-run designer invocation-admission catalog JSON should summarize blocked admission errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission-catalog",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1224: designer invocation-admission catalog JSON should reject invalid records");
    expect_contains(invalid_record_process.stdout_text, "\"designerInvocationAdmissionCatalog\": null",
        "#1224: invalid catalog records should not expose a catalog object");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1224: invalid catalog record JSON should report parser errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission-catalog",
            "--admit-toolbox-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1224: designer invocation-admission catalog JSON should reject invalid booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-toolbox-invocation value must be true or false.",
        "#1224: invalid catalog boolean JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
