void test_studio_host_toolbox_palette_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_palette_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--toolbox-palette-query", "--toolbox-context", "menu_item", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2398: default toolbox palette parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2398: default toolbox palette parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"toolboxPaletteQuery\": null",
        "#2398: default toolbox palette parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Unknown toolbox context token: menu_item",
        "#2398: default toolbox palette parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-palette-query", "--toolbox-context", "menu_item", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2398: pseudo-localized unknown-toolbox-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2398: pseudo-localized unknown-toolbox-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"toolboxPaletteQuery\": null",
        "#2398: pseudo-localized unknown-toolbox-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2398: pseudo-localized unknown-toolbox-context diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "menu_item",
        "#2398: pseudo-localized unknown-toolbox-context diagnostics should preserve toolbox context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: menu_item",
        "#2398: pseudo-localized unknown-toolbox-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-palette-query", "--toolbox-context", "form"},
        temp_root);

    expect(process.exit_code == 0,
        "#4689: English toolbox-palette text output should preserve success status");
    expect_contains(process.stdout_text,
        "status: ok",
        "#4689: English toolbox-palette text output should preserve the status display contract");
    expect_contains(process.stdout_text,
        "toolbox_context: form",
        "#4689: English toolbox-palette text output should preserve invariant context tokens");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-palette-query", "--toolbox-context", "form"},
        temp_root);

    expect(process.exit_code == 0,
        "#4689: pseudo-localized toolbox-palette text output should preserve success status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#4689: pseudo-localized toolbox-palette text output should decorate display labels");
    expect_contains(process.stdout_text,
        "form",
        "#4689: pseudo-localized toolbox-palette text output should preserve invariant context tokens");
    expect_not_contains(process.stdout_text,
        "toolbox_context:",
        "#4689: pseudo-localized toolbox-palette text output should not retain the raw English label");

    process = run_process_capture(
        studio_host_path,
        {"--toolbox-palette-query", "--toolbox-context", "form", "--toolbox-search", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2398: pseudo-localized missing-value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2398: pseudo-localized missing-value diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--toolbox-search",
        "#2398: pseudo-localized missing-value diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value for --toolbox-search.",
        "#2398: pseudo-localized missing-value diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--toolbox-palette-launch-catalog", "--selection-context", "visual_object", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2398: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2398: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "toolbox-palette-launch-catalog",
        "#2398: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--selection-context",
        "#2398: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox-palette-launch-catalog option: --selection-context",
        "#2398: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-palette-query", "--toolbox-context", "menu_item", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2582: es-419 unknown-toolbox-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2582: es-419 unknown-toolbox-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"toolboxPaletteQuery\": null",
        "#2582: es-419 unknown-toolbox-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Token de contexto de la caja de herramientas desconocido: menu_item",
        "#2582: es-419 unknown-toolbox-context diagnostics should localize toolbox-context token prose");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: menu_item",
        "#2582: es-419 unknown-toolbox-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-palette-query", "--toolbox-context", "form", "--toolbox-search", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2582: pt-BR missing-value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Falta um valor para --toolbox-search.",
        "#2582: pt-BR missing-value diagnostics should localize missing-value prose");
    expect_contains(process.stdout_text,
        "--toolbox-search",
        "#2582: pt-BR missing-value diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value for --toolbox-search.",
        "#2582: pt-BR missing-value diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-palette-launch-catalog", "--selection-context", "visual_object", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2582: es-419 unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Opcion desconocida de toolbox-palette-launch-catalog: --selection-context",
        "#2582: es-419 unknown-option diagnostics should localize command-option prose");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox-palette-launch-catalog option: --selection-context",
        "#2582: es-419 unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
