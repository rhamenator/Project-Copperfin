void test_studio_host_toolbox_direct_plan_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_direct_plan_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-dispatch-plan", "textbox",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2424: default toolbox-create-dispatch-plan diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2424: default toolbox-create-dispatch-plan diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2424: default toolbox-create-dispatch-plan diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-plan",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2424: default toolbox-create-batch-plan diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "No toolbox item ids were provided.",
        "#2424: default toolbox-create-batch-plan diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-dispatch-plan", "textbox",
            "--path", "forms/customer.scx",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2424: pseudo-localized toolbox-create-dispatch-plan admission diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2424: pseudo-localized toolbox-create-dispatch-plan admission diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--admit-create-operation",
        "#2424: pseudo-localized toolbox-create-dispatch-plan admission diagnostics should preserve option names");
    expect_contains(process.stdout_text,
        "true",
        "#2424: pseudo-localized toolbox-create-dispatch-plan admission diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2424: pseudo-localized toolbox-create-dispatch-plan admission diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#2424: pseudo-localized toolbox-create-dispatch-plan admission diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-dispatch-plan", "textbox",
            "--path", "forms/customer.scx",
            "--toolbox-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2424: pseudo-localized toolbox-create-dispatch-plan context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2424: pseudo-localized toolbox-create-dispatch-plan context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2424: pseudo-localized toolbox-create-dispatch-plan context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: unknown",
        "#2424: pseudo-localized toolbox-create-dispatch-plan context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-plan",
            "--path", "forms/customer.scx",
            "--object-name", "txtName",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2424: pseudo-localized toolbox-create-batch-plan orphan-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2424: pseudo-localized toolbox-create-batch-plan orphan-item diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--toolbox-item",
        "#2424: pseudo-localized toolbox-create-batch-plan orphan-item diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#2424: pseudo-localized toolbox-create-batch-plan orphan-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-plan",
            "--path", "forms/customer.scx",
            "--toolbox-item", "textbox",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2424: pseudo-localized toolbox-create-batch-plan field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2424: pseudo-localized toolbox-create-batch-plan field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2424: pseudo-localized toolbox-create-batch-plan field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2424: pseudo-localized toolbox-create-batch-plan field diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
