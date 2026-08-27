// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#include "test_studio_host_json_parse_diagnostics_usage_catalogs.inl"
void test_studio_host_list_subsystems_localizes_descriptor_text(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_list_subsystems_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(studio_host_path, {"--list-subsystems"}, temp_root);

    expect(process.exit_code == 0,
        "#2395: list-subsystems text output should succeed");
    expect_contains(process.stdout_text,
        "title: Report Designer",
        "#2395: default list-subsystems text output should preserve en-US titles");
    expect_contains(process.stdout_text,
        "current_status: implemented",
        "#2395: default list-subsystems text output should preserve invariant status values");
    expect_contains(process.stdout_text,
        "vfp9_equivalent: FRX/FRT designer, ReportBuilder.app, ReportPreview.app, ReportOutput.app",
        "#2395: default list-subsystems text output should preserve VFP-equivalent identifiers");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(studio_host_path, {"--list-subsystems", "--json"}, temp_root);

    expect(process.exit_code == 0,
        "#2395: pseudo-localized list-subsystems JSON output should succeed");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2395: pseudo-localized list-subsystems JSON should decorate descriptor prose");
    expect_contains(process.stdout_text,
        "\"id\": \"report-designer\"",
        "#2395: pseudo-localized list-subsystems JSON should preserve subsystem ids");
    expect_contains(process.stdout_text,
        "\"currentStatus\": \"implemented\"",
        "#2395: pseudo-localized list-subsystems JSON should preserve status values");
    expect_contains(process.stdout_text,
        "\"vfp9Equivalent\": \"FRX/FRT designer, ReportBuilder.app, ReportPreview.app, ReportOutput.app\"",
        "#2395: pseudo-localized list-subsystems JSON should preserve VFP-equivalent identifiers");
    expect_contains(process.stdout_text,
        "\"vfp9EquivalentDisplay\": \"[!! ",
        "#4246: pseudo-localized list-subsystems JSON should expose localized VFP-equivalent display text");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_builder_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_builder_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2396: default builder parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2396: default builder parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2396: default builder parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Unknown builder context token: unknown",
        "#2396: default builder parser diagnostics should preserve en-US prose");

    const std::string control_option = std::string("--unknown") + static_cast<char>(0x1f) + "value";
    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-catalog", "--builder-context", "control", "--json", control_option},
        temp_root);
    expect(process.exit_code == 2,
        "#4873: control-character unknown options should preserve parse-failure exit status");
    expect_contains(process.stdout_text, "--unknown\\u001fvalue",
        "#4873: Studio-host JSON should canonically escape control bytes");
    expect(process.stdout_text.find(static_cast<char>(0x1f)) == std::string::npos,
        "#4873: Studio-host JSON should not emit raw control bytes");

    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown"},
        temp_root);

    expect(process.exit_code == 2,
        "#2567: default text-mode builder diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "status: error",
        "#2567: default text-mode builder diagnostics should preserve machine-readable status");
    expect_contains(process.stdout_text,
        "error: Unknown builder context token: unknown",
        "#2567: default text-mode builder diagnostics should preserve the en-US prefixed error line");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2396: pseudo-localized unknown-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2396: pseudo-localized unknown-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2396: pseudo-localized unknown-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2396: pseudo-localized unknown-context diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2396: pseudo-localized unknown-context diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown builder context token: unknown",
        "#2396: pseudo-localized unknown-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown"},
        temp_root);

    const std::string pseudo_error_prefix =
        copperfin::localization::load_catalogs(
            copperfin::localization::resolve_catalog_root(),
            "qps-ploc").translate("StudioHost.Prefix.Error");
    expect(process.exit_code == 2,
        "#2567: pseudo-localized text-mode builder diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "status: error",
        "#2567: pseudo-localized text-mode builder diagnostics should preserve machine-readable status");
    expect_contains(process.stdout_text,
        pseudo_error_prefix,
        "#2567: pseudo-localized text-mode builder diagnostics should route the error prefix through localization");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2567: pseudo-localized text-mode builder diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2567: pseudo-localized text-mode builder diagnostics should preserve context tokens");
    expect_not_contains(process.stdout_text,
        "error: Unknown builder context token: unknown",
        "#2567: pseudo-localized text-mode builder diagnostics should not fall back to the raw English prefixed error");

    process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    const auto pseudo_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "qps-ploc");
    const std::string pseudo_launch_plan_request_name =
        pseudo_catalog.translate("StudioHost.BuilderParse.RequestName.LaunchPlan");
    expect(process.exit_code == 2,
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        pseudo_launch_plan_request_name,
        "#2568: pseudo-localized ambiguous builder-context diagnostics should route request labels through localization");
    expect_contains(process.stdout_text,
        "--builder-context",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "--selection-context",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Builder launch-plan requests cannot provide both --builder-context and --selection-context.",
        "#2568: pseudo-localized ambiguous builder-context diagnostics should not fall back to raw English request labels");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2577: es-419 ambiguous builder-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2577: es-419 ambiguous builder-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2577: es-419 ambiguous builder-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Las solicitudes de plan de lanzamiento del builder no pueden proporcionar --builder-context y --selection-context al mismo tiempo.",
        "#2577: es-419 ambiguous builder-context diagnostics should localize builder request conflict prose");
    expect_not_contains(process.stdout_text,
        "Builder launch-plan requests cannot provide both --builder-context and --selection-context.",
        "#2577: es-419 ambiguous builder-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--builder-launch-plan", "grid-builder",
            "--builder-context", "control",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2577: pt-BR ambiguous builder-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2577: pt-BR ambiguous builder-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"builderLaunchPlan\": null",
        "#2577: pt-BR ambiguous builder-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Solicitacoes de plano de lancamento do builder nao podem fornecer --builder-context e --selection-context ao mesmo tempo.",
        "#2577: pt-BR ambiguous builder-context diagnostics should localize builder request conflict prose");
    expect_not_contains(process.stdout_text,
        "Builder launch-plan requests cannot provide both --builder-context and --selection-context.",
        "#2577: pt-BR ambiguous builder-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "grid-builder", "--builder-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2578: es-419 unknown builder-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Token de contexto del builder desconocido: unknown",
        "#2578: es-419 unknown builder-context diagnostics should localize builder-context token prose");
    expect_not_contains(process.stdout_text,
        "Unknown builder context token: unknown",
        "#2578: es-419 unknown builder-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-plan", "form-builder", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2578: pt-BR missing-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Nenhum contexto do builder ou de selecao foi fornecido.",
        "#2578: pt-BR missing-context diagnostics should localize missing builder-or-selection context prose");
    expect_not_contains(process.stdout_text,
        "No builder or selection context was provided.",
        "#2578: pt-BR missing-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2578: es-419 invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "El valor de --admit-ui-launch debe ser true o false.",
        "#2578: es-419 invalid-boolean diagnostics should localize boolean validation prose");
    expect_contains(process.stdout_text,
        "true",
        "#2578: es-419 invalid-boolean diagnostics should preserve invariant true tokens");
    expect_contains(process.stdout_text,
        "false",
        "#2578: es-419 invalid-boolean diagnostics should preserve invariant false tokens");
    expect_not_contains(process.stdout_text,
        "The --admit-ui-launch value must be true or false.",
        "#2578: es-419 invalid-boolean diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    process = run_process_capture(
        studio_host_path,
        {
            "--builder-invocation-admission", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2396: pseudo-localized invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2396: pseudo-localized invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--admit-ui-launch",
        "#2396: pseudo-localized invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2396: pseudo-localized invalid-boolean diagnostics should preserve true boolean values");
    expect_contains(process.stdout_text,
        "false",
        "#2396: pseudo-localized invalid-boolean diagnostics should preserve false boolean values");
    expect_not_contains(process.stdout_text,
        "The --admit-ui-launch value must be true or false.",
        "#2396: pseudo-localized invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--builder-launch-catalog", "--builder-context", "control", "--admit-ui-launch", "true", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2396: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2396: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "builder-launch-catalog",
        "#2396: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--admit-ui-launch",
        "#2396: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown builder-launch-catalog option: --admit-ui-launch",
        "#2396: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_editor_action_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_editor_action_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--editor-action-launch-plan", "show-property-grid", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2397: default editor-action parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2397: default editor-action parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"editorActionLaunchPlan\": null",
        "#2397: default editor-action parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2397: default editor-action parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--editor-action-launch-plan", "show-property-grid", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2397: pseudo-localized unknown-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2397: pseudo-localized unknown-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"editorActionLaunchPlan\": null",
        "#2397: pseudo-localized unknown-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2397: pseudo-localized unknown-context diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2397: pseudo-localized unknown-context diagnostics should preserve selection-context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2397: pseudo-localized unknown-context diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2397: pseudo-localized invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2397: pseudo-localized invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--admit-editor-invocation",
        "#2397: pseudo-localized invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2397: pseudo-localized invalid-boolean diagnostics should preserve true boolean values");
    expect_contains(process.stdout_text,
        "false",
        "#2397: pseudo-localized invalid-boolean diagnostics should preserve false boolean values");
    expect_not_contains(process.stdout_text,
        "The --admit-editor-invocation value must be true or false.",
        "#2397: pseudo-localized invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2397: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2397: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "editor-action-launch-catalog",
        "#2397: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--admit-editor-invocation",
        "#2397: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown editor-action-launch-catalog option: --admit-editor-invocation",
        "#2397: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--editor-action-launch-plan", "show-property-grid", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2580: es-419 unknown-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2580: es-419 unknown-context diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"editorActionLaunchPlan\": null",
        "#2580: es-419 unknown-context diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Token de contexto de seleccion desconocido: unknown",
        "#2580: es-419 unknown-context diagnostics should localize selection-context token prose");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2580: es-419 unknown-context diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-invocation-admission", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2580: pt-BR invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "O valor de --admit-editor-invocation deve ser true ou false.",
        "#2580: pt-BR invalid-boolean diagnostics should localize boolean validation prose");
    expect_contains(process.stdout_text,
        "true",
        "#2580: pt-BR invalid-boolean diagnostics should preserve invariant true tokens");
    expect_contains(process.stdout_text,
        "false",
        "#2580: pt-BR invalid-boolean diagnostics should preserve invariant false tokens");
    expect_not_contains(process.stdout_text,
        "The --admit-editor-invocation value must be true or false.",
        "#2580: pt-BR invalid-boolean diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-launch-catalog",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2580: es-419 unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Opcion desconocida de editor-action-launch-catalog: --admit-editor-invocation",
        "#2580: es-419 unknown-option diagnostics should localize command-option prose");
    expect_not_contains(process.stdout_text,
        "Unknown editor-action-launch-catalog option: --admit-editor-invocation",
        "#2580: es-419 unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_designer_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surfaces", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2400: default designer parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2400: default designer parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"designerLaunchSurfaces\": null",
        "#2400: default designer parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2400: default designer parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surfaces", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2400: pseudo-localized unknown-selection diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2400: pseudo-localized unknown-selection diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"designerLaunchSurfaces\": null",
        "#2400: pseudo-localized unknown-selection diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2400: pseudo-localized unknown-selection diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "unknown",
        "#2400: pseudo-localized unknown-selection diagnostics should preserve selection-context tokens");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2400: pseudo-localized unknown-selection diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2400: pseudo-localized invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2400: pseudo-localized invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--admit-editor-invocations",
        "#2400: pseudo-localized invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2400: pseudo-localized invalid-boolean diagnostics should preserve true boolean values");
    expect_contains(process.stdout_text,
        "false",
        "#2400: pseudo-localized invalid-boolean diagnostics should preserve false boolean values");
    expect_not_contains(process.stdout_text,
        "The --admit-editor-invocations value must be true or false.",
        "#2400: pseudo-localized invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surface-catalog", "--selection-context", "visual_object", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2400: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2400: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "designer-launch-surface-catalog",
        "#2400: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--selection-context",
        "#2400: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown designer-launch-surface-catalog option: --selection-context",
        "#2400: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2400: pseudo-localized missing-launch-command diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2400: pseudo-localized missing-launch-command diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No designer editor action launch command was provided.",
        "#2400: pseudo-localized missing-launch-command diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surfaces", "--selection-context", "unknown", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2581: es-419 unknown-selection diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2581: es-419 unknown-selection diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"designerLaunchSurfaces\": null",
        "#2581: es-419 unknown-selection diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "Token de contexto de seleccion desconocido: unknown",
        "#2581: es-419 unknown-selection diagnostics should localize selection-context token prose");
    expect_not_contains(process.stdout_text,
        "Unknown selection context token: unknown",
        "#2581: es-419 unknown-selection diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2581: pt-BR invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "O valor de --admit-editor-invocations deve ser true ou false.",
        "#2581: pt-BR invalid-boolean diagnostics should localize boolean validation prose");
    expect_contains(process.stdout_text,
        "true",
        "#2581: pt-BR invalid-boolean diagnostics should preserve invariant true tokens");
    expect_contains(process.stdout_text,
        "false",
        "#2581: pt-BR invalid-boolean diagnostics should preserve invariant false tokens");
    expect_not_contains(process.stdout_text,
        "The --admit-editor-invocations value must be true or false.",
        "#2581: pt-BR invalid-boolean diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    process = run_process_capture(
        studio_host_path,
        {"--designer-launch-surface-catalog", "--selection-context", "visual_object", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2581: es-419 unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Opcion desconocida de designer-launch-surface-catalog: --selection-context",
        "#2581: es-419 unknown-option diagnostics should localize command-option prose");
    expect_not_contains(process.stdout_text,
        "Unknown designer-launch-surface-catalog option: --selection-context",
        "#2581: es-419 unknown-option diagnostics should not fall back to raw English prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2581: pt-BR missing-launch-command diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Nenhum comando de lancamento da acao do editor do designer foi fornecido.",
        "#2581: pt-BR missing-launch-command diagnostics should localize designer editor-action launch-command prose");
    expect_not_contains(process.stdout_text,
        "No designer editor action launch command was provided.",
        "#2581: pt-BR missing-launch-command diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_execution_fallback_errors_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_execution_fallback_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    auto process = run_process_capture(
        studio_host_path,
        {
            "--builder-execute", "grid-builder",
            "--builder-context", "control",
            "--admit-ui-launch", "true",
            "--admit-builder-execution", "true",
            "--builder-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(process.exit_code == 4,
        "#2560: pseudo-localized builder execution failures should preserve nonzero exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2560: pseudo-localized builder execution failures should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "\"observedExitCode\": 1",
        "#2560: pseudo-localized builder execution failures should preserve child exit codes");
    expect_not_contains(process.stdout_text,
        "Builder launch command returned a non-zero exit code.",
        "#2560: pseudo-localized builder execution failures should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--editor-action-execute", "edit-visual-method",
            "--selection-context", "visual_object",
            "--admit-editor-invocation", "true",
            "--admit-editor-action-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(process.exit_code == 4,
        "#2560: pseudo-localized editor-action execution failures should preserve nonzero exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2560: pseudo-localized editor-action execution failures should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "\"observedExitCode\": 1",
        "#2560: pseudo-localized editor-action execution failures should preserve child exit codes");
    expect_not_contains(process.stdout_text,
        "Editor action launch command returned a non-zero exit code.",
        "#2560: pseudo-localized editor-action execution failures should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--toolbox-execute",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-toolbox-execution", "true",
            "--toolbox-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(process.exit_code == 4,
        "#2560: pseudo-localized toolbox execution failures should preserve nonzero exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2560: pseudo-localized toolbox execution failures should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "\"observedExitCode\": 1",
        "#2560: pseudo-localized toolbox execution failures should preserve child exit codes");
    expect_not_contains(process.stdout_text,
        "Toolbox launch command returned a non-zero exit code.",
        "#2560: pseudo-localized toolbox execution failures should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(process.exit_code == 4,
        "#2560: pseudo-localized designer execution failures should preserve nonzero exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2560: pseudo-localized designer execution failures should decorate aggregate and child prose");
    expect_contains(process.stdout_text,
        "\"error\": \"[!! ",
        "#2560: pseudo-localized designer execution failures should localize the aggregate error field");
    expect_contains(process.stdout_text,
        "\"executionBlockedErrors\": [\"[!! ",
        "#2560: pseudo-localized designer execution failures should localize blocked child errors");
    expect_not_contains(process.stdout_text,
        "Designer builder launch command returned a non-zero exit code.",
        "#2560: pseudo-localized designer execution failures should not fall back to raw English child prose");
    expect_not_contains(process.stdout_text,
        "One or more designer child executions failed.",
        "#2560: pseudo-localized designer execution failures should not fall back to the raw English aggregate error");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
