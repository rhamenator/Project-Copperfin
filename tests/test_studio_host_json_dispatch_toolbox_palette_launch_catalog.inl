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
