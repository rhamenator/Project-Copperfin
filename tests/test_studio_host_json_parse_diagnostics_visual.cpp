// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_visual_property_core_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_property_core_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--visual-property-filter", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2401: default visual-property parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2401: default visual-property parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualPropertyFilter\": null",
        "#2401: default visual-property parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2401: default visual-property parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--visual-property-filter", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2401: pseudo-localized missing-path diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2401: pseudo-localized missing-path diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualPropertyFilter\": null",
        "#2401: pseudo-localized missing-path diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2401: pseudo-localized missing-path diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No asset path was provided.",
        "#2401: pseudo-localized missing-path diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-filter", "--path", "forms/customer.scx", "--unexpected", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2401: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2401: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "visual-property-filter",
        "#2401: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--unexpected",
        "#2401: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown visual-property-filter option: --unexpected",
        "#2401: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-query",
            "--path", "forms/customer.scx",
            "--property-name", "Caption",
            "--record", "-1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2401: pseudo-localized invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2401: pseudo-localized invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2401: pseudo-localized invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be a non-negative integer.",
        "#2401: pseudo-localized invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-clear", "--path", "forms/customer.scx", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2401: pseudo-localized missing-property-name diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2401: pseudo-localized missing-property-name diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No property name was provided.",
        "#2401: pseudo-localized missing-property-name diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-update-batch", "--path", "forms/customer.scx", "--property-value", "Blue", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2401: pseudo-localized update-batch item diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2401: pseudo-localized update-batch item diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--property-name",
        "#2401: pseudo-localized update-batch item diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Visual property update batch item options require a preceding --property-name.",
        "#2401: pseudo-localized update-batch item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-clear-batch", "--path", "forms/customer.scx", "--record", "1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2401: pseudo-localized clear-batch item diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2401: pseudo-localized clear-batch item diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--property-name",
        "#2401: pseudo-localized clear-batch item diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Visual property clear batch item options require a preceding --property-name.",
        "#2401: pseudo-localized clear-batch item diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_visual_property_copy_move_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_property_copy_move_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--visual-property-copy", "--property-name", "Caption", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2402: default visual-property copy parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2402: default visual-property copy parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualPropertyCopy\": null",
        "#2402: default visual-property copy parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2402: default visual-property copy parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--visual-property-copy", "--property-name", "Caption", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2402: pseudo-localized missing-path diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2402: pseudo-localized missing-path diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualPropertyCopy\": null",
        "#2402: pseudo-localized missing-path diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2402: pseudo-localized missing-path diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No asset path was provided.",
        "#2402: pseudo-localized missing-path diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", "forms/customer.scx",
            "--property-name", "Caption",
            "--source-record", "-1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2402: pseudo-localized invalid-source-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2402: pseudo-localized invalid-source-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--source-record",
        "#2402: pseudo-localized invalid-source-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#2402: pseudo-localized invalid-source-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", "forms/customer.scx",
            "--property-name", "Caption",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2402: pseudo-localized invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2402: pseudo-localized invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--replace-existing",
        "#2402: pseudo-localized invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2402: pseudo-localized invalid-boolean diagnostics should preserve true boolean values");
    expect_contains(process.stdout_text,
        "false",
        "#2402: pseudo-localized invalid-boolean diagnostics should preserve false boolean values");
    expect_not_contains(process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#2402: pseudo-localized invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-move", "--path", "forms/customer.scx", "--unexpected", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2402: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2402: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "visual-property-move",
        "#2402: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--unexpected",
        "#2402: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown visual-property-move option: --unexpected",
        "#2402: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-copy-batch", "--path", "forms/customer.scx", "--target-record", "1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2402: pseudo-localized copy-batch item diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2402: pseudo-localized copy-batch item diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--property-name",
        "#2402: pseudo-localized copy-batch item diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Visual property copy batch item options require a preceding --property-name.",
        "#2402: pseudo-localized copy-batch item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-move-batch", "--path", "forms/customer.scx", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2402: pseudo-localized empty-move-batch diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2402: pseudo-localized empty-move-batch diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No property moves were provided.",
        "#2402: pseudo-localized empty-move-batch diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_visual_property_rename_reorder_parse_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_property_rename_reorder_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", "forms/customer.scx",
            "--property-name", "Caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2403: default visual-property rename parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2403: default visual-property rename parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualPropertyRename\": null",
        "#2403: default visual-property rename parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No target property name was provided.",
        "#2403: default visual-property rename parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", "forms/customer.scx",
            "--property-name", "Caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2403: pseudo-localized missing-target-name diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2403: pseudo-localized missing-target-name diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualPropertyRename\": null",
        "#2403: pseudo-localized missing-target-name diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2403: pseudo-localized missing-target-name diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No target property name was provided.",
        "#2403: pseudo-localized missing-target-name diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", "forms/customer.scx",
            "--property-name", "Caption",
            "--record", "-1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2403: pseudo-localized invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2403: pseudo-localized invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2403: pseudo-localized invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be a non-negative integer.",
        "#2403: pseudo-localized invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-reorder", "--path", "forms/customer.scx", "--unexpected", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2403: pseudo-localized unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2403: pseudo-localized unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "visual-property-reorder",
        "#2403: pseudo-localized unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--unexpected",
        "#2403: pseudo-localized unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown visual-property-reorder option: --unexpected",
        "#2403: pseudo-localized unknown-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-rename-batch", "--path", "forms/customer.scx", "--new-property-name", "Text", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2403: pseudo-localized rename-batch item diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2403: pseudo-localized rename-batch item diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--property-name",
        "#2403: pseudo-localized rename-batch item diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Visual property rename batch item options require a preceding --property-name.",
        "#2403: pseudo-localized rename-batch item diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", "forms/customer.scx",
            "--property-name", "Caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2403: pseudo-localized missing-placement diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2403: pseudo-localized missing-placement diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No property placement was provided.",
        "#2403: pseudo-localized missing-placement diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-property-reorder-batch", "--path", "forms/customer.scx", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2403: pseudo-localized empty-reorder-batch diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2403: pseudo-localized empty-reorder-batch diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No property reorders were provided.",
        "#2403: pseudo-localized empty-reorder-batch diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_visual_list_navigation_parse_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_list_navigation_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--visual-property-list", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2404: default visual-property list parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2404: default visual-property list parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualPropertyList\": null",
        "#2404: default visual-property list parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2404: default visual-property list parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--visual-property-list", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2404: pseudo-localized visual-property missing-path diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2404: pseudo-localized visual-property missing-path diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualPropertyList\": null",
        "#2404: pseudo-localized visual-property missing-path diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2404: pseudo-localized visual-property missing-path diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No asset path was provided.",
        "#2404: pseudo-localized visual-property missing-path diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-list", "--unexpected", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2404: pseudo-localized visual-object unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2404: pseudo-localized visual-object unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "visual-object-list",
        "#2404: pseudo-localized visual-object unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--unexpected",
        "#2404: pseudo-localized visual-object unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown visual-object-list option: --unexpected",
        "#2404: pseudo-localized visual-object unknown-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-children", "--path", "forms/customer.scx", "--record", "-1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2404: pseudo-localized visual-object invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2404: pseudo-localized visual-object invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2404: pseudo-localized visual-object invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be a non-negative integer.",
        "#2404: pseudo-localized visual-object invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-descendants", "--path", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2404: pseudo-localized visual-object missing-value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2404: pseudo-localized visual-object missing-value diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--path",
        "#2404: pseudo-localized visual-object missing-value diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value for --path.",
        "#2404: pseudo-localized visual-object missing-value diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-ancestors", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2404: pseudo-localized visual-object missing-path diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2404: pseudo-localized visual-object missing-path diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "\"visualObjectAncestors\": null",
        "#2404: pseudo-localized visual-object missing-path diagnostics should preserve JSON payload contracts");
    expect_not_contains(process.stdout_text,
        "No asset path was provided.",
        "#2404: pseudo-localized visual-object missing-path diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_visual_object_reparent_duplicate_parse_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_object_reparent_duplicate_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--visual-object-reparent-batch", "--selected-unique-id", "textbox-guid", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2405: default visual-object reparent parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2405: default visual-object reparent parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualObjectReparentBatch\": null",
        "#2405: default visual-object reparent parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2405: default visual-object reparent parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--visual-object-reparent-batch", "--selected-unique-id", "textbox-guid", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2405: pseudo-localized reparent missing-path diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2405: pseudo-localized reparent missing-path diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualObjectReparentBatch\": null",
        "#2405: pseudo-localized reparent missing-path diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2405: pseudo-localized reparent missing-path diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No asset path was provided.",
        "#2405: pseudo-localized reparent missing-path diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-duplicate-batch", "--path", "forms/customer.scx", "--selected-record", "-1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2405: pseudo-localized duplicate invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2405: pseudo-localized duplicate invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--selected-record",
        "#2405: pseudo-localized duplicate invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --selected-record value must be a non-negative integer.",
        "#2405: pseudo-localized duplicate invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-duplicate-batch", "--path", "forms/customer.scx", "--new-name", "copy", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2405: pseudo-localized duplicate item-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2405: pseudo-localized duplicate item-order diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "Visual object duplicate batch item options require a preceding selected-object selector.",
        "#2405: pseudo-localized duplicate item-order diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-duplicate-subtree", "--path", "forms/customer.scx", "--record", "-1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2405: pseudo-localized subtree invalid-root-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2405: pseudo-localized subtree invalid-root-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2405: pseudo-localized subtree invalid-root-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be a non-negative integer.",
        "#2405: pseudo-localized subtree invalid-root-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-duplicate-subtree", "--path", "forms/customer.scx", "--unique-id", "root-guid", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2405: pseudo-localized subtree missing-replacements diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2405: pseudo-localized subtree missing-replacements diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No subtree replacement identities were provided.",
        "#2405: pseudo-localized subtree missing-replacements diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", "forms/customer.scx",
            "--unique-id", "root-guid",
            "--new-name", "copy",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2405: pseudo-localized subtree replacement-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2405: pseudo-localized subtree replacement-order diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--replacement-source-unique-id",
        "#2405: pseudo-localized subtree replacement-order diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Subtree duplicate replacement options require a preceding --replacement-source-unique-id.",
        "#2405: pseudo-localized subtree replacement-order diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_visual_object_rename_reorder_update_parse_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_object_rename_reorder_update_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--visual-object-rename-batch", "--selected-unique-id", "textbox-guid", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2406: default visual-object rename parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2406: default visual-object rename parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualObjectRenameBatch\": null",
        "#2406: default visual-object rename parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2406: default visual-object rename parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--visual-object-rename-batch", "--path", "forms/customer.scx", "--new-name", "copy", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2406: pseudo-localized rename item-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2406: pseudo-localized rename item-order diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "Visual object rename batch item options require a preceding selected-object selector.",
        "#2406: pseudo-localized rename item-order diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-reorder-batch", "--path", "forms/customer.scx", "--selected-record", "-1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2406: pseudo-localized reorder invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2406: pseudo-localized reorder invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--selected-record",
        "#2406: pseudo-localized reorder invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --selected-record value must be a non-negative integer.",
        "#2406: pseudo-localized reorder invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", "forms/customer.scx",
            "--selected-unique-id", "textbox-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2406: pseudo-localized reorder missing-placement diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2406: pseudo-localized reorder missing-placement diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No visual object placement was provided.",
        "#2406: pseudo-localized reorder missing-placement diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-update-batch", "--path", "forms/customer.scx", "--property-value", "caption", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2406: pseudo-localized update item-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2406: pseudo-localized update item-order diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "Visual object update batch property options require a preceding selected-object selector.",
        "#2406: pseudo-localized update item-order diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", "forms/customer.scx",
            "--selected-unique-id", "textbox-guid",
            "--property-value", "caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2406: pseudo-localized update property-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2406: pseudo-localized update property-order diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--property-name",
        "#2406: pseudo-localized update property-order diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Visual object update batch property values require a preceding --property-name.",
        "#2406: pseudo-localized update property-order diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-object-update-batch", "--path", "forms/customer.scx", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2406: pseudo-localized update missing-edits diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2406: pseudo-localized update missing-edits diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No visual object edits were provided.",
        "#2406: pseudo-localized update missing-edits diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_visual_method_core_parse_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_method_core_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--visual-method-list", "--record", "0", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2407: default visual-method list parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2407: default visual-method list parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualMethodList\": null",
        "#2407: default visual-method list parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2407: default visual-method list parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--visual-method-list", "--path", "forms/customer.scx", "--record", "-1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2407: pseudo-localized visual-method list invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2407: pseudo-localized visual-method list invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2407: pseudo-localized visual-method list invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be a non-negative integer.",
        "#2407: pseudo-localized visual-method list invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-query", "--path", "forms/customer.scx", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2407: pseudo-localized visual-method query missing-name diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodQuery\": null",
        "#2407: pseudo-localized visual-method query missing-name diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2407: pseudo-localized visual-method query missing-name diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No method name was provided.",
        "#2407: pseudo-localized visual-method query missing-name diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--path", "forms/customer.scx",
            "--method-name", "When",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2407: pseudo-localized visual-method update missing-kind diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodUpdate\": null",
        "#2407: pseudo-localized visual-method update missing-kind diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2407: pseudo-localized visual-method update missing-kind diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No method kind was provided.",
        "#2407: pseudo-localized visual-method update missing-kind diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--path", "forms/customer.scx",
            "--method-name", "When",
            "--method-kind", "procedure",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2407: pseudo-localized visual-method update missing-source diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2407: pseudo-localized visual-method update missing-source diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No method source was provided.",
        "#2407: pseudo-localized visual-method update missing-source diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-delete", "--path", "forms/customer.scx", "--bogus", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2407: pseudo-localized visual-method delete unknown-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodDelete\": null",
        "#2407: pseudo-localized visual-method delete unknown-option diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2407: pseudo-localized visual-method delete unknown-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "visual-method-delete",
        "#2407: pseudo-localized visual-method delete unknown-option diagnostics should preserve command names");
    expect_contains(process.stdout_text,
        "--bogus",
        "#2407: pseudo-localized visual-method delete unknown-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Unknown visual-method-delete option: --bogus",
        "#2407: pseudo-localized visual-method delete unknown-option diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_visual_method_delete_rename_parse_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_method_delete_rename_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--visual-method-delete-batch", "--method-name", "When", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2408: default visual-method delete-batch parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2408: default visual-method delete-batch parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualMethodDeleteBatch\": null",
        "#2408: default visual-method delete-batch parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2408: default visual-method delete-batch parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--visual-method-delete-batch", "--path", "forms/customer.scx", "--record", "1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2408: pseudo-localized delete-batch item-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2408: pseudo-localized delete-batch item-order diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--method-name",
        "#2408: pseudo-localized delete-batch item-order diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Visual method delete batch item options require a preceding --method-name.",
        "#2408: pseudo-localized delete-batch item-order diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-rename", "--path", "forms/customer.scx", "--record", "-1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2408: pseudo-localized rename invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodRename\": null",
        "#2408: pseudo-localized rename invalid-record diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2408: pseudo-localized rename invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2408: pseudo-localized rename invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be a non-negative integer.",
        "#2408: pseudo-localized rename invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-rename", "--path", "forms/customer.scx", "--method-name", "When", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2408: pseudo-localized rename missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2408: pseudo-localized rename missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No target method name was provided.",
        "#2408: pseudo-localized rename missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-rename-batch", "--path", "forms/customer.scx", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2408: pseudo-localized rename-batch missing-items diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodRenameBatch\": null",
        "#2408: pseudo-localized rename-batch missing-items diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2408: pseudo-localized rename-batch missing-items diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No method renames were provided.",
        "#2408: pseudo-localized rename-batch missing-items diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-rename-batch", "--path", "forms/customer.scx", "--new-method-name", "AfterRow", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2408: pseudo-localized rename-batch item-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2408: pseudo-localized rename-batch item-order diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--method-name",
        "#2408: pseudo-localized rename-batch item-order diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Visual method rename batch item options require a preceding --method-name.",
        "#2408: pseudo-localized rename-batch item-order diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_visual_method_copy_move_parse_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_method_copy_move_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--visual-method-copy", "--method-name", "When", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2409: default visual-method copy parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2409: default visual-method copy parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualMethodCopy\": null",
        "#2409: default visual-method copy parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2409: default visual-method copy parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--visual-method-copy", "--path", "forms/customer.scx", "--source-record", "-1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2409: pseudo-localized copy invalid-source-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodCopy\": null",
        "#2409: pseudo-localized copy invalid-source-record diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2409: pseudo-localized copy invalid-source-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--source-record",
        "#2409: pseudo-localized copy invalid-source-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#2409: pseudo-localized copy invalid-source-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--path", "forms/customer.scx",
            "--method-name", "When",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2409: pseudo-localized copy invalid-boolean diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2409: pseudo-localized copy invalid-boolean diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--replace-existing",
        "#2409: pseudo-localized copy invalid-boolean diagnostics should preserve CLI option names");
    expect_contains(process.stdout_text,
        "true",
        "#2409: pseudo-localized copy invalid-boolean diagnostics should preserve true token");
    expect_contains(process.stdout_text,
        "false",
        "#2409: pseudo-localized copy invalid-boolean diagnostics should preserve false token");
    expect_not_contains(process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#2409: pseudo-localized copy invalid-boolean diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-copy-batch", "--path", "forms/customer.scx", "--target-record", "1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2409: pseudo-localized copy-batch item-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodCopyBatch\": null",
        "#2409: pseudo-localized copy-batch item-order diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2409: pseudo-localized copy-batch item-order diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--method-name",
        "#2409: pseudo-localized copy-batch item-order diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Visual method copy batch item options require a preceding --method-name.",
        "#2409: pseudo-localized copy-batch item-order diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-move-batch", "--path", "forms/customer.scx", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2409: pseudo-localized move-batch missing-items diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodMoveBatch\": null",
        "#2409: pseudo-localized move-batch missing-items diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2409: pseudo-localized move-batch missing-items diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No method moves were provided.",
        "#2409: pseudo-localized move-batch missing-items diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-move", "--path", "forms/customer.scx", "--target-record", "-1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2409: pseudo-localized move invalid-target-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodMove\": null",
        "#2409: pseudo-localized move invalid-target-record diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2409: pseudo-localized move invalid-target-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--target-record",
        "#2409: pseudo-localized move invalid-target-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --target-record value must be a non-negative integer.",
        "#2409: pseudo-localized move invalid-target-record diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_visual_method_reorder_parse_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_visual_method_reorder_parse_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--visual-method-reorder", "--method-name", "When", "--placement", "after", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2410: default visual-method reorder parser diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"status\": \"error\"",
        "#2410: default visual-method reorder parser diagnostics should preserve JSON status contracts");
    expect_contains(process.stdout_text,
        "\"visualMethodReorder\": null",
        "#2410: default visual-method reorder parser diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "No asset path was provided.",
        "#2410: default visual-method reorder parser diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--visual-method-reorder", "--path", "forms/customer.scx", "--record", "-1", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2410: pseudo-localized reorder invalid-record diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2410: pseudo-localized reorder invalid-record diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record",
        "#2410: pseudo-localized reorder invalid-record diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --record value must be a non-negative integer.",
        "#2410: pseudo-localized reorder invalid-record diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-reorder", "--path", "forms/customer.scx", "--method-name", "When", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2410: pseudo-localized reorder missing-placement diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2410: pseudo-localized reorder missing-placement diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No method placement was provided.",
        "#2410: pseudo-localized reorder missing-placement diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-reorder-batch", "--path", "forms/customer.scx", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2410: pseudo-localized reorder-batch missing-items diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "\"visualMethodReorderBatch\": null",
        "#2410: pseudo-localized reorder-batch missing-items diagnostics should preserve JSON payload contracts");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2410: pseudo-localized reorder-batch missing-items diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "No method reorders were provided.",
        "#2410: pseudo-localized reorder-batch missing-items diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--visual-method-reorder-batch", "--path", "forms/customer.scx", "--placement", "after", "--json"},
        temp_root);

    expect(process.exit_code == 2,
        "#2410: pseudo-localized reorder-batch item-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2410: pseudo-localized reorder-batch item-order diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--method-name",
        "#2410: pseudo-localized reorder-batch item-order diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Visual method reorder batch item options require a preceding --method-name.",
        "#2410: pseudo-localized reorder-batch item-order diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
