// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
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

void test_studio_host_toolbox_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--toolbox-invocation-admission", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2399: default toolbox parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2399: default toolbox parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"toolboxInvocationAdmission\": null",
        "#2399: default toolbox parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2399: default toolbox parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-invocation-admission", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2399: pseudo-localized unknown-selection diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2399: pseudo-localized unknown-selection diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"toolboxInvocationAdmission\": null",
        "#2399: pseudo-localized unknown-selection diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2399: pseudo-localized unknown-selection diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2399: pseudo-localized unknown-selection diagnostics should preserve selection-context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2399: pseudo-localized unknown-selection diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--toolbox-dispatch-catalog", "--toolbox-context", "menu_item", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2399: pseudo-localized unknown-toolbox-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2399: pseudo-localized unknown-toolbox-context diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "menu_item",
        "#2399: pseudo-localized unknown-toolbox-context diagnostics should preserve toolbox context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: menu_item",
        "#2399: pseudo-localized unknown-toolbox-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2399: pseudo-localized invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2399: pseudo-localized invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--admit-palette-invocation",
        "#2399: pseudo-localized invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2399: pseudo-localized invalid-boolean diagnostics should preserve true boolean values");
    expect_contains(process.stdout_text,
        "false",
        "#2399: pseudo-localized invalid-boolean diagnostics should preserve false boolean values");
    expect_not_contains(process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#2399: pseudo-localized invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--selection-toolbox-dispatch-catalog", "--selection-context", "visual_object", "--toolbox-context", "form", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2399: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2399: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "selection-toolbox-dispatch-catalog",
        "#2399: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--toolbox-context",
        "#2399: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown selection-toolbox-dispatch-catalog option: --toolbox-context",
        "#2399: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2399: pseudo-localized missing-launch-command diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2399: pseudo-localized missing-launch-command diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No toolbox launch command was provided.",
        "#2399: pseudo-localized missing-launch-command diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-invocation-admission", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2583: es-419 unknown-selection diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2583: es-419 unknown-selection diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"toolboxInvocationAdmission\": null",
        "#2583: es-419 unknown-selection diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Token de contexto de seleccion desconocido: unknown",
        "#2583: es-419 unknown-selection diagnostics should localize selection-context token prose");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2583: es-419 unknown-selection diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-dispatch-catalog", "--toolbox-context", "menu_item", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2583: pt-BR unknown-toolbox-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Token de contexto da caixa de ferramentas desconhecido: menu_item",
        "#2583: pt-BR unknown-toolbox-context diagnostics should localize toolbox-context token prose");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: menu_item",
        "#2583: pt-BR unknown-toolbox-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-dispatch",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2583: es-419 invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "El valor de --admit-palette-invocation debe ser true o false.",
        "#2583: es-419 invalid-boolean diagnostics should localize boolean validation prose");
    expect_contains(process.stdout_text,
        "true",
        "#2583: es-419 invalid-boolean diagnostics should preserve invariant true tokens");
    expect_contains(process.stdout_text,
        "false",
        "#2583: es-419 invalid-boolean diagnostics should preserve invariant false tokens");
    expect_not_contains(process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#2583: es-419 invalid-boolean diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {"--selection-toolbox-dispatch-catalog", "--selection-context", "visual_object", "--toolbox-context", "form", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2583: pt-BR unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Opcao desconhecida de selection-toolbox-dispatch-catalog: --toolbox-context",
        "#2583: pt-BR unknown-option diagnostics should localize command-option prose");
    expect_not_contains(process.stdout_text,
        "Unknown selection-toolbox-dispatch-catalog option: --toolbox-context",
        "#2583: pt-BR unknown-option diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2583: es-419 missing-launch-command diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "No se proporciono un comando de lanzamiento de la caja de herramientas.",
        "#2583: es-419 missing-launch-command diagnostics should localize toolbox launch-command prose");
    expect_not_contains(process.stdout_text,
        "No toolbox launch command was provided.",
        "#2583: es-419 missing-launch-command diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_create_plan_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_plan_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--toolbox-create-plan", "textbox", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2411: default toolbox-create-plan parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2411: default toolbox-create-plan parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"toolboxCreatePlan\": null",
        "#2411: default toolbox-create-plan parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2411: default toolbox-create-plan parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--toolbox-create-plan", "textbox", "--path", "forms/customer.scx", "--toolbox-context", "menu_item", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2411: pseudo-localized toolbox-create-plan context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2411: pseudo-localized toolbox-create-plan context diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "menu_item",
        "#2411: pseudo-localized toolbox-create-plan context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: menu_item",
        "#2411: pseudo-localized toolbox-create-plan context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--toolbox-create-plan", "textbox", "--path", "forms/customer.scx", "--field-value", "caption", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2411: pseudo-localized toolbox-create-plan field diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2411: pseudo-localized toolbox-create-plan field diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2411: pseudo-localized toolbox-create-plan field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2411: pseudo-localized toolbox-create-plan field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--record", "-1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2411: pseudo-localized dispatch-plan invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2411: pseudo-localized dispatch-plan invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2411: pseudo-localized dispatch-plan invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be a non-negative integer.",
        "#2411: pseudo-localized dispatch-plan invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2411: pseudo-localized dispatch-plan invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2411: pseudo-localized dispatch-plan invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--admit-palette-invocation",
        "#2411: pseudo-localized dispatch-plan invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2411: pseudo-localized dispatch-plan invalid-boolean diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2411: pseudo-localized dispatch-plan invalid-boolean diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#2411: pseudo-localized dispatch-plan invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--toolbox-create-from-dispatch-plan", "textbox", "--path", "forms/customer.scx", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2411: pseudo-localized dispatch-plan missing-selection diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2411: pseudo-localized dispatch-plan missing-selection diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No selection context was provided.",
        "#2411: pseudo-localized dispatch-plan missing-selection diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_dispatch_create_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_dispatch_create_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--toolbox-create-from-dispatch", "textbox", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2412: default toolbox-create-from-dispatch parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2412: default toolbox-create-from-dispatch parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"toolboxCreateFromDispatch\": {",
        "#2412: default toolbox-create-from-dispatch parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "\"createPlan\": null",
        "#2412: default toolbox-create-from-dispatch parser diagnostics should preserve nested plan contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2412: default toolbox-create-from-dispatch parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-from-dispatch", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--record", "-1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2412: pseudo-localized create-from-dispatch invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2412: pseudo-localized create-from-dispatch invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2412: pseudo-localized create-from-dispatch invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be a non-negative integer.",
        "#2412: pseudo-localized create-from-dispatch invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-from-dispatch", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2412: pseudo-localized create-from-dispatch field diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2412: pseudo-localized create-from-dispatch field diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2412: pseudo-localized create-from-dispatch field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2412: pseudo-localized create-from-dispatch field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-dispatch-from-dispatch-plan", "textbox",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2412: pseudo-localized dispatch-from-dispatch missing-selection diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"toolboxCreateDispatchPlan\": null",
        "#2412: pseudo-localized dispatch-from-dispatch missing-selection diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2412: pseudo-localized dispatch-from-dispatch missing-selection diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No selection context was provided.",
        "#2412: pseudo-localized dispatch-from-dispatch missing-selection diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-dispatch-from-dispatch-plan", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2412: pseudo-localized dispatch-from-dispatch invalid-create-admission diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2412: pseudo-localized dispatch-from-dispatch invalid-create-admission diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--admit-create-operation",
        "#2412: pseudo-localized dispatch-from-dispatch invalid-create-admission diagnostics should preserve option names");
    expect_contains(process.stdout_text,
        "true",
        "#2412: pseudo-localized dispatch-from-dispatch invalid-create-admission diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2412: pseudo-localized dispatch-from-dispatch invalid-create-admission diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#2412: pseudo-localized dispatch-from-dispatch invalid-create-admission diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_batch_dispatch_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_batch_dispatch_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--toolbox-create-batch-from-dispatch-plan", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2413: default toolbox-create-batch-from-dispatch-plan diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2413: default toolbox-create-batch-from-dispatch-plan diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"toolboxCreateBatchPlan\": null",
        "#2413: default toolbox-create-batch-from-dispatch-plan diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2413: default toolbox-create-batch-from-dispatch-plan diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-from-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2413: pseudo-localized batch-from-dispatch-plan empty-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "\"toolboxCreateBatchPlan\": null",
        "#2413: pseudo-localized batch-from-dispatch-plan empty-item diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2413: pseudo-localized batch-from-dispatch-plan empty-item diagnostics should decorate prose");
    expect_not_contains(process.stdout_text,
        "No toolbox item ids were provided.",
        "#2413: pseudo-localized batch-from-dispatch-plan empty-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-from-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--create-object-name", "txtName",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2413: pseudo-localized batch-from-dispatch-plan orphan-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2413: pseudo-localized batch-from-dispatch-plan orphan-item diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--toolbox-item",
        "#2413: pseudo-localized batch-from-dispatch-plan orphan-item diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#2413: pseudo-localized batch-from-dispatch-plan orphan-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-from-dispatch",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2413: pseudo-localized batch-from-dispatch field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "\"toolboxCreateBatchFromDispatch\": {",
        "#2413: pseudo-localized batch-from-dispatch field diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2413: pseudo-localized batch-from-dispatch field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2413: pseudo-localized batch-from-dispatch field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2413: pseudo-localized batch-from-dispatch field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-from-dispatch",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2413: pseudo-localized batch-from-dispatch admission diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2413: pseudo-localized batch-from-dispatch admission diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--admit-palette-invocation",
        "#2413: pseudo-localized batch-from-dispatch admission diagnostics should preserve option names");
    expect_contains(process.stdout_text,
        "true",
        "#2413: pseudo-localized batch-from-dispatch admission diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2413: pseudo-localized batch-from-dispatch admission diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#2413: pseudo-localized batch-from-dispatch admission diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_batch_dispatch_plan_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_batch_dispatch_plan_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--toolbox-create-batch-dispatch-from-dispatch-plan", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2414: default toolbox-create-batch-dispatch-from-dispatch-plan diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2414: default toolbox-create-batch-dispatch-from-dispatch-plan diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "\"toolboxCreateBatchDispatchPlan\": null",
        "#2414: default toolbox-create-batch-dispatch-from-dispatch-plan diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2414: default toolbox-create-batch-dispatch-from-dispatch-plan diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2414: pseudo-localized batch-dispatch empty-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "\"toolboxCreateBatchDispatchPlan\": null",
        "#2414: pseudo-localized batch-dispatch empty-item diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2414: pseudo-localized batch-dispatch empty-item diagnostics should decorate prose");
    expect_not_contains(process.stdout_text,
        "No toolbox item ids were provided.",
        "#2414: pseudo-localized batch-dispatch empty-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--create-object-name", "txtName",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2414: pseudo-localized batch-dispatch orphan-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2414: pseudo-localized batch-dispatch orphan-item diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--toolbox-item",
        "#2414: pseudo-localized batch-dispatch orphan-item diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#2414: pseudo-localized batch-dispatch orphan-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2414: pseudo-localized batch-dispatch field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2414: pseudo-localized batch-dispatch field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2414: pseudo-localized batch-dispatch field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2414: pseudo-localized batch-dispatch field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2414: pseudo-localized batch-dispatch create-admission diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2414: pseudo-localized batch-dispatch create-admission diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--admit-create-operation",
        "#2414: pseudo-localized batch-dispatch create-admission diagnostics should preserve option names");
    expect_contains(process.stdout_text,
        "true",
        "#2414: pseudo-localized batch-dispatch create-admission diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2414: pseudo-localized batch-dispatch create-admission diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#2414: pseudo-localized batch-dispatch create-admission diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_selection_toolbox_create_plan_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_plan_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--selection-toolbox-create-plan", "textbox", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2415: default selection-toolbox-create-plan diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2415: default selection-toolbox-create-plan diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "\"selectionToolboxCreatePlan\": null",
        "#2415: default selection-toolbox-create-plan diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2415: default selection-toolbox-create-plan diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--selection-toolbox-create-plan", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2415: pseudo-localized selection-toolbox-create-plan missing-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2415: pseudo-localized selection-toolbox-create-plan missing-item diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--selection-toolbox-create-plan",
        "#2415: pseudo-localized selection-toolbox-create-plan missing-item diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Missing value for --selection-toolbox-create-plan.",
        "#2415: pseudo-localized selection-toolbox-create-plan missing-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-plan", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2415: pseudo-localized selection-toolbox-create-plan field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2415: pseudo-localized selection-toolbox-create-plan field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2415: pseudo-localized selection-toolbox-create-plan field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2415: pseudo-localized selection-toolbox-create-plan field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan admission diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "\"selectionToolboxCreateDispatchPlan\": {",
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan admission diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--admit-create-operation",
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan admission diagnostics should preserve option names");
    expect_contains(process.stdout_text,
        "true",
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan admission diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan admission diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan admission diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--path", "forms/customer.scx",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2415: pseudo-localized selection-toolbox-create-dispatch-plan context diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_selection_toolbox_batch_create_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_batch_create_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--selection-toolbox-create-batch-plan", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2416: default selection-toolbox-create-batch-plan diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2416: default selection-toolbox-create-batch-plan diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "\"selectionToolboxCreateBatchPlan\": {",
        "#2416: default selection-toolbox-create-batch-plan diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2416: default selection-toolbox-create-batch-plan diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--object-name", "txtName",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2416: default selection-toolbox-create-batch orphan-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "\"selectionToolboxCreateBatch\": {",
        "#2416: default selection-toolbox-create-batch orphan-item diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "Selection toolbox batch create item options require a preceding --toolbox-item.",
        "#2416: default selection-toolbox-create-batch orphan-item diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2416: pseudo-localized selection-toolbox-create-batch-plan empty-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2416: pseudo-localized selection-toolbox-create-batch-plan empty-item diagnostics should decorate prose");
    expect_not_contains(process.stdout_text,
        "No toolbox item ids were provided.",
        "#2416: pseudo-localized selection-toolbox-create-batch-plan empty-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--object-name", "txtName",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2416: pseudo-localized selection-toolbox-create-batch-plan orphan-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2416: pseudo-localized selection-toolbox-create-batch-plan orphan-item diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--toolbox-item",
        "#2416: pseudo-localized selection-toolbox-create-batch-plan orphan-item diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Selection toolbox batch item options require a preceding --toolbox-item.",
        "#2416: pseudo-localized selection-toolbox-create-batch-plan orphan-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2416: pseudo-localized selection-toolbox-create-batch field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2416: pseudo-localized selection-toolbox-create-batch field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2416: pseudo-localized selection-toolbox-create-batch field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2416: pseudo-localized selection-toolbox-create-batch field diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_selection_toolbox_batch_dispatch_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_batch_dispatch_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--selection-toolbox-create-batch-dispatch-plan", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2417: default selection-toolbox-create-batch-dispatch-plan diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2417: default selection-toolbox-create-batch-dispatch-plan diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "\"selectionToolboxCreateBatchDispatchPlan\": {",
        "#2417: default selection-toolbox-create-batch-dispatch-plan diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2417: default selection-toolbox-create-batch-dispatch-plan diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--object-name", "txtName",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2417: default selection-toolbox-create-batch-dispatch-plan orphan-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "Selection toolbox batch dispatch item options require a preceding --toolbox-item.",
        "#2417: default selection-toolbox-create-batch-dispatch-plan orphan-item diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan empty-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan empty-item diagnostics should decorate prose");
    expect_not_contains(process.stdout_text,
        "No toolbox item ids were provided.",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan empty-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-plan",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan admission diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan admission diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--admit-create-operation",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan admission diagnostics should preserve option names");
    expect_contains(process.stdout_text,
        "true",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan admission diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan admission diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#2417: pseudo-localized selection-toolbox-create-batch-dispatch-plan admission diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_batch_dispatch_direct_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_batch_dispatch_direct_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--toolbox-create-batch-dispatch-plan", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2418: default toolbox-create-batch-dispatch-plan diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2418: default toolbox-create-batch-dispatch-plan diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "\"toolboxCreateBatchDispatchPlan\": null",
        "#2418: default toolbox-create-batch-dispatch-plan diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2418: default toolbox-create-batch-dispatch-plan diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-plan",
            "--path", "forms/customer.scx",
            "--toolbox-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan toolbox-context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan toolbox-context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan toolbox-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: unknown",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan toolbox-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-plan",
            "--path", "forms/customer.scx",
            "--object-name", "txtName",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan orphan-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan orphan-item diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--toolbox-item",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan orphan-item diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan orphan-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-plan",
            "--path", "forms/customer.scx",
            "--toolbox-item", "textbox",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-plan",
            "--path", "forms/customer.scx",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan admission diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan admission diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--admit-create-operation",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan admission diagnostics should preserve option names");
    expect_contains(process.stdout_text,
        "true",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan admission diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan admission diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#2418: pseudo-localized toolbox-create-batch-dispatch-plan admission diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_batch_create_direct_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_batch_create_direct_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--toolbox-create-batch", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2419: default toolbox-create-batch diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2419: default toolbox-create-batch diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "\"toolboxCreateBatch\": {",
        "#2419: default toolbox-create-batch diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2419: default toolbox-create-batch diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch",
            "--path", "forms/customer.scx",
            "--toolbox-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2419: pseudo-localized toolbox-create-batch toolbox-context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2419: pseudo-localized toolbox-create-batch toolbox-context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2419: pseudo-localized toolbox-create-batch toolbox-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: unknown",
        "#2419: pseudo-localized toolbox-create-batch toolbox-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch",
            "--path", "forms/customer.scx",
            "--object-name", "txtName",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2419: pseudo-localized toolbox-create-batch orphan-item diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2419: pseudo-localized toolbox-create-batch orphan-item diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--toolbox-item",
        "#2419: pseudo-localized toolbox-create-batch orphan-item diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#2419: pseudo-localized toolbox-create-batch orphan-item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch",
            "--path", "forms/customer.scx",
            "--toolbox-item", "textbox",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2419: pseudo-localized toolbox-create-batch field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2419: pseudo-localized toolbox-create-batch field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2419: pseudo-localized toolbox-create-batch field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2419: pseudo-localized toolbox-create-batch field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch",
            "--path", "forms/customer.scx",
            "--toolbox-item", "textbox",
            "--unexpected",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2419: pseudo-localized toolbox-create-batch unknown-option diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2419: pseudo-localized toolbox-create-batch unknown-option diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "toolbox-create-batch",
        "#2419: pseudo-localized toolbox-create-batch unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--unexpected",
        "#2419: pseudo-localized toolbox-create-batch unknown-option diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox-create-batch option: --unexpected",
        "#2419: pseudo-localized toolbox-create-batch unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_batch_plan_catalog_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_batch_plan_catalog_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-plan-catalog",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2420: default toolbox-create-batch-plan-catalog diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2420: default toolbox-create-batch-plan-catalog diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "\"toolboxCreateBatchPlanCatalog\": null",
        "#2420: default toolbox-create-batch-plan-catalog diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No toolbox context was provided.",
        "#2420: default toolbox-create-batch-plan-catalog diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-plan-catalog",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2420: default selection-toolbox-create-batch-plan-catalog diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"selectionToolboxCreateBatchPlanCatalog\": null",
        "#2420: default selection-toolbox-create-batch-plan-catalog diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No selection context was provided.",
        "#2420: default selection-toolbox-create-batch-plan-catalog diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-plan-catalog",
            "--path", "forms/customer.scx",
            "--toolbox-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2420: pseudo-localized toolbox-create-batch-plan-catalog toolbox-context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2420: pseudo-localized toolbox-create-batch-plan-catalog toolbox-context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2420: pseudo-localized toolbox-create-batch-plan-catalog toolbox-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: unknown",
        "#2420: pseudo-localized toolbox-create-batch-plan-catalog toolbox-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-plan-catalog",
            "--path", "forms/customer.scx",
            "--toolbox-context", "form",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2420: pseudo-localized toolbox-create-batch-plan-catalog field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2420: pseudo-localized toolbox-create-batch-plan-catalog field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2420: pseudo-localized toolbox-create-batch-plan-catalog field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2420: pseudo-localized toolbox-create-batch-plan-catalog field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-plan-catalog",
            "--path", "forms/customer.scx",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2420: pseudo-localized selection-toolbox-create-batch-plan-catalog selection-context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2420: pseudo-localized selection-toolbox-create-batch-plan-catalog selection-context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2420: pseudo-localized selection-toolbox-create-batch-plan-catalog selection-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2420: pseudo-localized selection-toolbox-create-batch-plan-catalog selection-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-plan-catalog",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2420: pseudo-localized selection-toolbox-create-batch-plan-catalog unknown-option diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2420: pseudo-localized selection-toolbox-create-batch-plan-catalog unknown-option diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "selection-toolbox-create-batch-plan-catalog",
        "#2420: pseudo-localized selection-toolbox-create-batch-plan-catalog unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--toolbox-context",
        "#2420: pseudo-localized selection-toolbox-create-batch-plan-catalog unknown-option diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Unknown selection-toolbox-create-batch-plan-catalog option: --toolbox-context",
        "#2420: pseudo-localized selection-toolbox-create-batch-plan-catalog unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_plan_catalog_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_plan_catalog_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-plan-catalog",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2421: default toolbox-create-plan-catalog diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2421: default toolbox-create-plan-catalog diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "\"toolboxCreatePlanCatalog\": null",
        "#2421: default toolbox-create-plan-catalog diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No toolbox context was provided.",
        "#2421: default toolbox-create-plan-catalog diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-plan-catalog",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2421: default selection-toolbox-create-plan-catalog diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"selectionToolboxCreatePlanCatalog\": null",
        "#2421: default selection-toolbox-create-plan-catalog diagnostics should preserve JSON contracts");
    expect_contains(process.stdout_text,
        "No selection context was provided.",
        "#2421: default selection-toolbox-create-plan-catalog diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-plan-catalog",
            "--path", "forms/customer.scx",
            "--toolbox-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2421: pseudo-localized toolbox-create-plan-catalog toolbox-context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2421: pseudo-localized toolbox-create-plan-catalog toolbox-context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2421: pseudo-localized toolbox-create-plan-catalog toolbox-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown toolbox context token: unknown",
        "#2421: pseudo-localized toolbox-create-plan-catalog toolbox-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-plan-catalog",
            "--path", "forms/customer.scx",
            "--toolbox-context", "form",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2421: pseudo-localized toolbox-create-plan-catalog field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2421: pseudo-localized toolbox-create-plan-catalog field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2421: pseudo-localized toolbox-create-plan-catalog field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2421: pseudo-localized toolbox-create-plan-catalog field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-plan-catalog",
            "--path", "forms/customer.scx",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2421: pseudo-localized selection-toolbox-create-plan-catalog selection-context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2421: pseudo-localized selection-toolbox-create-plan-catalog selection-context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2421: pseudo-localized selection-toolbox-create-plan-catalog selection-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2421: pseudo-localized selection-toolbox-create-plan-catalog selection-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-plan-catalog",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2421: pseudo-localized selection-toolbox-create-plan-catalog unknown-option diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2421: pseudo-localized selection-toolbox-create-plan-catalog unknown-option diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "selection-toolbox-create-plan-catalog",
        "#2421: pseudo-localized selection-toolbox-create-plan-catalog unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--toolbox-context",
        "#2421: pseudo-localized selection-toolbox-create-plan-catalog unknown-option diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Unknown selection-toolbox-create-plan-catalog option: --toolbox-context",
        "#2421: pseudo-localized selection-toolbox-create-plan-catalog unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_dispatch_catalog_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_dispatch_catalog_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2422: default toolbox-create-dispatch-catalog diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2422: default toolbox-create-dispatch-catalog diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "No toolbox context was provided.",
        "#2422: default toolbox-create-dispatch-catalog diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2422: default selection-toolbox-create-dispatch-catalog diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "No selection context was provided.",
        "#2422: default selection-toolbox-create-dispatch-catalog diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--toolbox-context", "form",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2422: pseudo-localized toolbox-create-dispatch-catalog admission diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2422: pseudo-localized toolbox-create-dispatch-catalog admission diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--admit-create-operation",
        "#2422: pseudo-localized toolbox-create-dispatch-catalog admission diagnostics should preserve option names");
    expect_contains(process.stdout_text,
        "true",
        "#2422: pseudo-localized toolbox-create-dispatch-catalog admission diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2422: pseudo-localized toolbox-create-dispatch-catalog admission diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#2422: pseudo-localized toolbox-create-dispatch-catalog admission diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--toolbox-context", "form",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2422: pseudo-localized toolbox-create-dispatch-catalog field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2422: pseudo-localized toolbox-create-dispatch-catalog field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2422: pseudo-localized toolbox-create-dispatch-catalog field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2422: pseudo-localized toolbox-create-dispatch-catalog field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2422: pseudo-localized selection-toolbox-create-dispatch-catalog selection-context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2422: pseudo-localized selection-toolbox-create-dispatch-catalog selection-context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2422: pseudo-localized selection-toolbox-create-dispatch-catalog selection-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2422: pseudo-localized selection-toolbox-create-dispatch-catalog selection-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2422: pseudo-localized selection-toolbox-create-dispatch-catalog unknown-option diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2422: pseudo-localized selection-toolbox-create-dispatch-catalog unknown-option diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "selection-toolbox-create-dispatch-catalog",
        "#2422: pseudo-localized selection-toolbox-create-dispatch-catalog unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--toolbox-context",
        "#2422: pseudo-localized selection-toolbox-create-dispatch-catalog unknown-option diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Unknown selection-toolbox-create-dispatch-catalog option: --toolbox-context",
        "#2422: pseudo-localized selection-toolbox-create-dispatch-catalog unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_toolbox_batch_dispatch_catalog_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_batch_dispatch_catalog_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2423: default toolbox-create-batch-dispatch-catalog diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2423: default toolbox-create-batch-dispatch-catalog diagnostics should preserve JSON status");
    expect_contains(process.stdout_text,
        "No toolbox context was provided.",
        "#2423: default toolbox-create-batch-dispatch-catalog diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2423: default selection-toolbox-create-batch-dispatch-catalog diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "No selection context was provided.",
        "#2423: default selection-toolbox-create-batch-dispatch-catalog diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--toolbox-context", "form",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog admission diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog admission diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "--admit-create-operation",
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog admission diagnostics should preserve option names");
    expect_contains(process.stdout_text,
        "true",
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog admission diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog admission diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog admission diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-create-batch-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--toolbox-context", "form",
            "--field-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog field diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog field diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog field diagnostics should preserve assignment syntax");
    expect_not_contains(process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#2423: pseudo-localized toolbox-create-batch-dispatch-catalog field diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2423: pseudo-localized selection-toolbox-create-batch-dispatch-catalog selection-context diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2423: pseudo-localized selection-toolbox-create-batch-dispatch-catalog selection-context diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2423: pseudo-localized selection-toolbox-create-batch-dispatch-catalog selection-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2423: pseudo-localized selection-toolbox-create-batch-dispatch-catalog selection-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2423: pseudo-localized selection-toolbox-create-batch-dispatch-catalog unknown-option diagnostics should preserve exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2423: pseudo-localized selection-toolbox-create-batch-dispatch-catalog unknown-option diagnostics should decorate prose");
    expect_contains(process.stdout_text,
        "selection-toolbox-create-batch-dispatch-catalog",
        "#2423: pseudo-localized selection-toolbox-create-batch-dispatch-catalog unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--toolbox-context",
        "#2423: pseudo-localized selection-toolbox-create-batch-dispatch-catalog unknown-option diagnostics should preserve option names");
    expect_not_contains(process.stdout_text,
        "Unknown selection-toolbox-create-batch-dispatch-catalog option: --toolbox-context",
        "#2423: pseudo-localized selection-toolbox-create-batch-dispatch-catalog unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

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

}  // namespace cf_test_studio_host_json
