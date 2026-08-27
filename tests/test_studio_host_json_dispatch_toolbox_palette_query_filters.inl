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
