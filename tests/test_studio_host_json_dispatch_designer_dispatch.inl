void test_studio_host_json_exposes_designer_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_designer_dispatch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
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
        "#1238: designer dispatch JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"designerDispatch\": {",
        "#1238: designer dispatch JSON should expose a dispatch object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1238: designer dispatch JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1238: designer dispatch JSON should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1238: designer dispatch JSON should preserve record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1238: designer dispatch JSON should preserve object names");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1238: designer dispatch JSON should preserve unique ids");
    expect_contains(visual_process.stdout_text, "\"symbol\": \"Click\"",
        "#1238: designer dispatch JSON should preserve editor symbols");
    expect_contains(visual_process.stdout_text, "\"line\": 12",
        "#1238: designer dispatch JSON should preserve editor lines");
    expect_contains(visual_process.stdout_text, "\"column\": 4",
        "#1238: designer dispatch JSON should preserve editor columns");
    expect_contains(visual_process.stdout_text, "\"dispatchOkSelectionContexts\": [\"visual_object\"]",
        "#1401: designer dispatch JSON should summarize dispatch-clean selected contexts");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedSelectionContexts\": []",
        "#1401: designer dispatch JSON should expose empty blocked selected contexts for clean dispatch");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1401: designer dispatch JSON should expose empty blocked errors for clean dispatch");
    expect_contains(visual_process.stdout_text, "\"editorActionDispatchCount\": ",
        "#1238: designer dispatch JSON should expose editor dispatch counts");
    expect_contains(visual_process.stdout_text, "\"builderDispatchCount\": ",
        "#1238: designer dispatch JSON should expose builder dispatch counts");
    expect_contains(visual_process.stdout_text, "\"toolboxDispatchCount\": 1",
        "#1238: designer dispatch JSON should expose toolbox dispatch counts");
    expect_contains(visual_process.stdout_text, "\"dispatchCount\": ",
        "#1238: designer dispatch JSON should expose aggregate dispatch counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1238: admitted designer dispatch JSON should expose zero aggregate errors");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1238: designer dispatch JSON should include editor action dispatches");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1238: designer dispatch JSON should include builder dispatches");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1238: designer dispatch JSON should include toolbox dispatches");
    expect_contains(visual_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1238: admitted designer dispatch JSON should expose dispatch-admitted surfaces");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1238: admitted designer dispatch JSON should not be aggregate dry-run");
    expect_contains(visual_process.stdout_text, "\"executed\": false",
        "#1238: designer dispatch JSON should not execute UI");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1238: designer dispatch JSON should remain non-mutating");

    const auto menu_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--selection-context", "menu_item",
            "--path", "menus/main.mnx",
            "--record", "5",
            "--object-name", "FileExit",
            "--unique-id", "menu-guid",
            "--json"
        },
        temp_root);
    expect(menu_process.exit_code == 0,
        "#1238: designer dispatch JSON should keep dry-run menu contexts as aggregate successes");
    expect_contains(menu_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1238: menu designer dispatch JSON should expose selected Studio contexts");
    expect_contains(menu_process.stdout_text, "\"dispatchCount\": 0",
        "#1238: default designer dispatch JSON should expose zero aggregate dispatches");
    expect_contains(menu_process.stdout_text, "\"errorCount\": 5",
        "#1238: default menu designer dispatch JSON should expose per-surface dispatch errors");
    expect_contains(menu_process.stdout_text, "\"dispatchOkSelectionContexts\": []",
        "#1401: default menu dispatch JSON should expose empty clean selected contexts");
    expect_contains(menu_process.stdout_text, "\"dispatchBlockedSelectionContexts\": [\"menu_item\"]",
        "#1401: default menu dispatch JSON should summarize dispatch-blocked selected contexts");
    expect_contains(menu_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"An editor action dispatch request requires an admitted non-dry-run invocation.\"]",
        "#1401: default menu dispatch JSON should summarize first blocked selected-context reason");
    expect_contains(menu_process.stdout_text, "\"dispatchOk\": false",
        "#1238: default menu designer dispatch JSON should expose rejected dispatches");
    expect_contains(menu_process.stdout_text,
        "An editor action dispatch request requires an admitted non-dry-run invocation.",
        "#1238: default menu designer dispatch JSON should report editor dispatch errors");
    expect_contains(menu_process.stdout_text,
        "A builder dispatch request requires an admitted non-dry-run invocation.",
        "#1238: default menu designer dispatch JSON should report builder dispatch errors");
    expect_contains(menu_process.stdout_text,
        "The selected Studio context does not expose a toolbox palette.",
        "#1238: menu designer dispatch JSON should preserve unsupported toolbox reasons");
    expect_contains(menu_process.stdout_text, "\"dryRun\": true",
        "#1238: default designer dispatch JSON should expose dry-run state");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--selection-context", "visual_object",
            "--admit-builder-invocations", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1238: designer dispatch JSON should reject invalid builder admission booleans");
    expect_contains(invalid_boolean_process.stdout_text, "\"designerDispatch\": null",
        "#1238: invalid designer dispatch boolean JSON should not expose a dispatch object");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-builder-invocations value must be true or false.",
        "#1238: invalid designer dispatch boolean JSON should report parser errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1238: designer dispatch JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1238: unknown designer dispatch context JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1238: designer dispatch JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1238: invalid designer dispatch line JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1238: designer dispatch JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1238: missing designer dispatch context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#include "test_studio_host_json_dispatch_designer_execution.inl"
