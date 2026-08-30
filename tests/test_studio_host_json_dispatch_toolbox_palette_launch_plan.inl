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
