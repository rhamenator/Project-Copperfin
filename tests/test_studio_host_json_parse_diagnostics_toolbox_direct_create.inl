void test_studio_host_toolbox_direct_create_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_direct_create_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create", "textbox",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2425: default toolbox-create diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2425: default toolbox-create diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2425: default toolbox-create diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create", "textbox",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2425: default selection-toolbox-create diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "No selection context was provided.",
        "#2425: default selection-toolbox-create diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create", "textbox",
            "--path", "forms/customer.scx",
            "--toolbox-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2425: pseudo-localized toolbox-create context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2425: pseudo-localized toolbox-create context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2425: pseudo-localized toolbox-create context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: unknown",
        "#2425: pseudo-localized toolbox-create context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create", "textbox",
            "--path", "forms/customer.scx",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2425: pseudo-localized toolbox-create field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2425: pseudo-localized toolbox-create field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2425: pseudo-localized toolbox-create field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2425: pseudo-localized toolbox-create field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2425: pseudo-localized selection-toolbox-create context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2425: pseudo-localized selection-toolbox-create context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2425: pseudo-localized selection-toolbox-create context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2425: pseudo-localized selection-toolbox-create context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2425: pseudo-localized selection-toolbox-create unknown-option diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2425: pseudo-localized selection-toolbox-create unknown-option diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "selection-toolbox-create",
        "#2425: pseudo-localized selection-toolbox-create unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--toolbox-context",
        "#2425: pseudo-localized selection-toolbox-create unknown-option diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Unknown selection-toolbox-create option: --toolbox-context",
        "#2425: pseudo-localized selection-toolbox-create unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
