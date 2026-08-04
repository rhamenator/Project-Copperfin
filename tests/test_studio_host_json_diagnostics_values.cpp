// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_launch_core_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_core_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--path"},
        temp_root);

    expect(process.exit_code == 2,
        "#2449: default missing path value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --path.",
        "#2449: default missing path value diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2486: default object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --object-name.",
        "#2486: default object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--undo-label"},
        temp_root);

    expect(process.exit_code == 2,
        "#2486: default undo-label missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --undo-label.",
        "#2486: default undo-label missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--path", "forms/customer.scx", "--line", "not-number"},
        temp_root);

    expect(process.exit_code == 2,
        "#2488: default line unsigned-integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --line value must be an unsigned integer.",
        "#2488: default line unsigned-integer diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--path", "forms/customer.scx", "--undo-mode", "nonsense"},
        temp_root);

    expect(process.exit_code == 2,
        "#2488: default undo-mode value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --undo-mode value must be edit or command.",
        "#2488: default undo-mode value diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--selection-context", "bogus_context"},
        temp_root);

    expect(process.exit_code == 2,
        "#2501: default selection-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --selection-context value must be visual_object, visual_method, container_object, class_designer, report_expression, label_expression, menu_item, project_item, or data_environment.",
        "#2501: default selection-context diagnostics should preserve en-US list grammar");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--selection-context", "bogus_context"},
        temp_root);

    expect(process.exit_code == 2,
        "#2449: pseudo-localized selection-context diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2449: pseudo-localized selection-context diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "visual_object",
        "#2449: pseudo-localized selection-context diagnostics should preserve selection-context tokens");
    expect_contains(process.stdout_text,
        "data_environment",
        "#2501: pseudo-localized selection-context diagnostics should preserve final selection-context token");
    expect_not_contains(process.stdout_text,
        "The --selection-context value must be visual_object, visual_method, container_object, class_designer, report_expression, label_expression, menu_item, project_item, or data_environment.",
        "#2501: pseudo-localized selection-context diagnostics should not fall back to raw English list grammar");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--record", "not-a-number"},
        temp_root);

    expect(process.exit_code == 2,
        "#2449: pseudo-localized record integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2449: pseudo-localized record integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2449: pseudo-localized record integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be an unsigned integer.",
        "#2449: pseudo-localized record integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--field-value", "missing-name-value-separator"},
        temp_root);

    expect(process.exit_code == 2,
        "#2449: pseudo-localized field-value syntax diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2449: pseudo-localized field-value syntax diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "name=value",
        "#2449: pseudo-localized field-value syntax diagnostics should preserve syntax token");
    expect_not_contains(process.stdout_text,
        "Field values must use name=value syntax.",
        "#2449: pseudo-localized field-value syntax diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--line"},
        temp_root);

    expect(process.exit_code == 2,
        "#2486: pseudo-localized line missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2486: pseudo-localized line missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--line",
        "#2486: pseudo-localized line missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --line.",
        "#2486: pseudo-localized line missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--undo-mode"},
        temp_root);

    expect(process.exit_code == 2,
        "#2486: pseudo-localized undo-mode missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2486: pseudo-localized undo-mode missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--undo-mode",
        "#2486: pseudo-localized undo-mode missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --undo-mode.",
        "#2486: pseudo-localized undo-mode missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--bogus-option"},
        temp_root);

    expect(process.exit_code == 2,
        "#2488: pseudo-localized unknown argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2488: pseudo-localized unknown argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--bogus-option",
        "#2488: pseudo-localized unknown argument diagnostics should preserve argument text");
    expect_not_contains(process.stdout_text,
        "Unknown argument: --bogus-option",
        "#2488: pseudo-localized unknown argument diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "forms/customer.scx", "extra.scx"},
        temp_root);

    expect(process.exit_code == 2,
        "#2488: pseudo-localized extra positional diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2488: pseudo-localized extra positional diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "extra.scx",
        "#2488: pseudo-localized extra positional diagnostics should preserve argument text");
    expect_not_contains(process.stdout_text,
        "Unexpected extra positional argument: extra.scx",
        "#2488: pseudo-localized extra positional diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2488: pseudo-localized missing asset path diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2488: pseudo-localized missing asset path diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No asset path was provided.",
        "#2488: pseudo-localized missing asset path diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_layout_state_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_layout_state_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--alignment-mode"},
        temp_root);

    expect(process.exit_code == 2,
        "#2450: default alignment-mode missing value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --alignment-mode.",
        "#2450: default alignment-mode missing value diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--grid-width", "wide"},
        temp_root);

    expect(process.exit_code == 2,
        "#2450: pseudo-localized grid-width numeric diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2450: pseudo-localized grid-width numeric diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--grid-width",
        "#2450: pseudo-localized grid-width numeric diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --grid-width value must be numeric.",
        "#2450: pseudo-localized grid-width numeric diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--starting-tab-index", "first"},
        temp_root);

    expect(process.exit_code == 2,
        "#2450: pseudo-localized starting-tab-index integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2450: pseudo-localized starting-tab-index integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--starting-tab-index",
        "#2450: pseudo-localized starting-tab-index integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --starting-tab-index value must be an integer.",
        "#2450: pseudo-localized starting-tab-index integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--locked", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2450: pseudo-localized locked boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2450: pseudo-localized locked boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "true",
        "#2450: pseudo-localized locked boolean diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2450: pseudo-localized locked boolean diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --locked value must be true or false.",
        "#2450: pseudo-localized locked boolean diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_text_media_list_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_text_media_list_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--caption"},
        temp_root);

    expect(process.exit_code == 2,
        "#2451: default caption missing value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --caption.",
        "#2451: default caption missing value diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--ole-drag-picture"},
        temp_root);

    expect(process.exit_code == 2,
        "#2451: pseudo-localized OLE drag picture missing value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2451: pseudo-localized OLE drag picture missing value diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drag-picture",
        "#2451: pseudo-localized OLE drag picture missing value diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --ole-drag-picture.",
        "#2451: pseudo-localized OLE drag picture missing value diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--record-source"},
        temp_root);

    expect(process.exit_code == 2,
        "#2451: pseudo-localized record-source missing value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2451: pseudo-localized record-source missing value diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record-source",
        "#2451: pseudo-localized record-source missing value diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --record-source.",
        "#2451: pseudo-localized record-source missing value diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--column-lines", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2451: pseudo-localized column-lines boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2451: pseudo-localized column-lines boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "true",
        "#2451: pseudo-localized column-lines boolean diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2451: pseudo-localized column-lines boolean diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --column-lines value must be true or false.",
        "#2451: pseudo-localized column-lines boolean diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_list_scalar_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_list_scalar_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--multi-select"},
        temp_root);

    expect(process.exit_code == 2,
        "#2452: default multi-select missing value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --multi-select.",
        "#2452: default multi-select missing value diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--multi-select", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2452: pseudo-localized multi-select boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2452: pseudo-localized multi-select boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "true",
        "#2452: pseudo-localized multi-select boolean diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2452: pseudo-localized multi-select boolean diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --multi-select value must be true or false.",
        "#2452: pseudo-localized multi-select boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--row-source-type", "table"},
        temp_root);

    expect(process.exit_code == 2,
        "#2452: pseudo-localized row-source-type integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2452: pseudo-localized row-source-type integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--row-source-type",
        "#2452: pseudo-localized row-source-type integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --row-source-type value must be an integer.",
        "#2452: pseudo-localized row-source-type integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--left-column", "first"},
        temp_root);

    expect(process.exit_code == 2,
        "#2452: pseudo-localized left-column integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2452: pseudo-localized left-column integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--left-column",
        "#2452: pseudo-localized left-column integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --left-column value must be an integer.",
        "#2452: pseudo-localized left-column integer diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_grid_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_grid_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--grid-line-color"},
        temp_root);

    expect(process.exit_code == 2,
        "#2455: default grid-line-color missing value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --grid-line-color.",
        "#2455: default grid-line-color missing value diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--header-height", "tall"},
        temp_root);

    expect(process.exit_code == 2,
        "#2455: pseudo-localized header-height integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2455: pseudo-localized header-height integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--header-height",
        "#2455: pseudo-localized header-height integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --header-height value must be an integer.",
        "#2455: pseudo-localized header-height integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--lock-columns-left", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2455: pseudo-localized lock-columns-left non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2455: pseudo-localized lock-columns-left non-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--lock-columns-left",
        "#2455: pseudo-localized lock-columns-left non-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --lock-columns-left value must be non-negative.",
        "#2455: pseudo-localized lock-columns-left non-negative diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--highlight-row-line-width", "wide"},
        temp_root);

    expect(process.exit_code == 2,
        "#2455: pseudo-localized highlight-row-line-width integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2455: pseudo-localized highlight-row-line-width integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--highlight-row-line-width",
        "#2455: pseudo-localized highlight-row-line-width integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --highlight-row-line-width value must be an integer.",
        "#2455: pseudo-localized highlight-row-line-width integer diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_record_list_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_record_list_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--partition"},
        temp_root);

    expect(process.exit_code == 2,
        "#2456: default partition missing value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --partition.",
        "#2456: default partition missing value diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--record-source-type", "alias"},
        temp_root);

    expect(process.exit_code == 2,
        "#2456: pseudo-localized record-source-type integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2456: pseudo-localized record-source-type integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record-source-type",
        "#2456: pseudo-localized record-source-type integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record-source-type value must be an integer.",
        "#2456: pseudo-localized record-source-type integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--column-order", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2456: pseudo-localized column-order non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2456: pseudo-localized column-order non-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--column-order",
        "#2456: pseudo-localized column-order non-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --column-order value must be non-negative.",
        "#2456: pseudo-localized column-order non-negative diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--list-item-id", "current"},
        temp_root);

    expect(process.exit_code == 2,
        "#2456: pseudo-localized list-item-id integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2456: pseudo-localized list-item-id integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--list-item-id",
        "#2456: pseudo-localized list-item-id integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --list-item-id value must be an integer.",
        "#2456: pseudo-localized list-item-id integer diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_window_flag_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_window_flag_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--closable"},
        temp_root);

    expect(process.exit_code == 2,
        "#2458: default closable missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --closable.",
        "#2458: default closable missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--control-box"},
        temp_root);

    expect(process.exit_code == 2,
        "#2458: pseudo-localized control-box missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2458: pseudo-localized control-box missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--control-box",
        "#2458: pseudo-localized control-box missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --control-box.",
        "#2458: pseudo-localized control-box missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--auto-verb-menu", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2458: pseudo-localized auto-verb-menu boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2458: pseudo-localized auto-verb-menu boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--auto-verb-menu",
        "#2458: pseudo-localized auto-verb-menu boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2458: pseudo-localized auto-verb-menu boolean diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2458: pseudo-localized auto-verb-menu boolean diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --auto-verb-menu value must be true or false.",
        "#2458: pseudo-localized auto-verb-menu boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--min-button", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2458: pseudo-localized min-button boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2458: pseudo-localized min-button boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--min-button",
        "#2458: pseudo-localized min-button boolean diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --min-button value must be true or false.",
        "#2458: pseudo-localized min-button boolean diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_bounds_border_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_bounds_border_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--min-height"},
        temp_root);

    expect(process.exit_code == 2,
        "#2459: default min-height missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --min-height.",
        "#2459: default min-height missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--min-height", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2459: default min-height not-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --min-height value must not be negative.",
        "#2459: default min-height not-negative diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--min-width", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2459: pseudo-localized min-width not-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2459: pseudo-localized min-width not-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--min-width",
        "#2459: pseudo-localized min-width not-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --min-width value must not be negative.",
        "#2459: pseudo-localized min-width not-negative diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--movable", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2459: pseudo-localized movable boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2459: pseudo-localized movable boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--movable",
        "#2459: pseudo-localized movable boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2459: pseudo-localized movable boolean diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2459: pseudo-localized movable boolean diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --movable value must be true or false.",
        "#2459: pseudo-localized movable boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--border-style", "solid"},
        temp_root);

    expect(process.exit_code == 2,
        "#2459: pseudo-localized border-style integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2459: pseudo-localized border-style integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--border-style",
        "#2459: pseudo-localized border-style integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --border-style value must be an integer.",
        "#2459: pseudo-localized border-style integer diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_dynamic_expression_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_dynamic_expression_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-input-mask"},
        temp_root);

    expect(process.exit_code == 2,
        "#2461: default dynamic-input-mask missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --dynamic-input-mask.",
        "#2461: default dynamic-input-mask missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-current-control"},
        temp_root);

    expect(process.exit_code == 2,
        "#2461: pseudo-localized dynamic-current-control missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2461: pseudo-localized dynamic-current-control missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-current-control",
        "#2461: pseudo-localized dynamic-current-control missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --dynamic-current-control.",
        "#2461: pseudo-localized dynamic-current-control missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-font-strikethru"},
        temp_root);

    expect(process.exit_code == 2,
        "#2461: pseudo-localized dynamic-font-strikethru missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2461: pseudo-localized dynamic-font-strikethru missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-font-strikethru",
        "#2461: pseudo-localized dynamic-font-strikethru missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --dynamic-font-strikethru.",
        "#2461: pseudo-localized dynamic-font-strikethru missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-font-shadow"},
        temp_root);

    expect(process.exit_code == 2,
        "#2461: pseudo-localized dynamic-font-shadow missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2461: pseudo-localized dynamic-font-shadow missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-font-shadow",
        "#2461: pseudo-localized dynamic-font-shadow missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --dynamic-font-shadow.",
        "#2461: pseudo-localized dynamic-font-shadow missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_max_auto_selection_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_max_auto_selection_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--max-width"},
        temp_root);

    expect(process.exit_code == 2,
        "#2463: default max-width missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --max-width.",
        "#2463: default max-width missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--max-width", "wide"},
        temp_root);

    expect(process.exit_code == 2,
        "#2463: default max-width integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --max-width value must be an integer.",
        "#2463: default max-width integer diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--max-left", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2463: pseudo-localized max-left not-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2463: pseudo-localized max-left not-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--max-left",
        "#2463: pseudo-localized max-left not-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --max-left value must not be negative.",
        "#2463: pseudo-localized max-left not-negative diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--continuous-scroll", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2463: pseudo-localized continuous-scroll true/false diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2463: pseudo-localized continuous-scroll true/false diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--continuous-scroll",
        "#2463: pseudo-localized continuous-scroll true/false diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --continuous-scroll value must be true or false.",
        "#2463: pseudo-localized continuous-scroll true/false diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--hide-selection"},
        temp_root);

    expect(process.exit_code == 2,
        "#2463: pseudo-localized hide-selection missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2463: pseudo-localized hide-selection missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--hide-selection",
        "#2463: pseudo-localized hide-selection missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --hide-selection.",
        "#2463: pseudo-localized hide-selection missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_deleted_state_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_deleted_state_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--deleted-state-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2477: default deleted-state-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --deleted-state-target-object-name.",
        "#2477: default deleted-state-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--deleted-state", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2477: default deleted-state true/false diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --deleted-state value must be true or false.",
        "#2477: default deleted-state true/false diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--deleted-state", "true"},
        temp_root);

    expect(process.exit_code == 2,
        "#2477: default deleted-state target dependency diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "A deleted-state value requires a preceding deleted-state target selector.",
        "#2477: default deleted-state target dependency diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--subtree-deleted"},
        temp_root);

    expect(process.exit_code == 2,
        "#2477: pseudo-localized subtree-deleted missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2477: pseudo-localized subtree-deleted missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--subtree-deleted",
        "#2477: pseudo-localized subtree-deleted missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --subtree-deleted.",
        "#2477: pseudo-localized subtree-deleted missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--subtree-deleted", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2477: pseudo-localized subtree-deleted true/false diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2477: pseudo-localized subtree-deleted true/false diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--subtree-deleted",
        "#2477: pseudo-localized subtree-deleted true/false diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --subtree-deleted value must be true or false.",
        "#2477: pseudo-localized subtree-deleted true/false diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--deleted-state", "true"},
        temp_root);

    expect(process.exit_code == 2,
        "#2477: pseudo-localized deleted-state target dependency diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2477: pseudo-localized deleted-state target dependency diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "A deleted-state value requires a preceding deleted-state target selector.",
        "#2477: pseudo-localized deleted-state target dependency diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
