// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_launch_layout_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_layout_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--anchor-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2466: default anchor-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --anchor-object-name.",
        "#2466: default anchor-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--align-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2466: default align-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --align-target-object-name.",
        "#2466: default align-target-object-name missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--resize-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2466: pseudo-localized resize-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2466: pseudo-localized resize-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--resize-target-unique-id",
        "#2466: pseudo-localized resize-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --resize-target-unique-id.",
        "#2466: pseudo-localized resize-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--snap-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2466: pseudo-localized snap-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2466: pseudo-localized snap-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--snap-target-object-name",
        "#2466: pseudo-localized snap-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --snap-target-object-name.",
        "#2466: pseudo-localized snap-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--nudge-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2466: pseudo-localized nudge-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2466: pseudo-localized nudge-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--nudge-target-unique-id",
        "#2466: pseudo-localized nudge-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --nudge-target-unique-id.",
        "#2466: pseudo-localized nudge-target-unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_state_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_state_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--tab-order-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2467: default tab-order-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --tab-order-target-object-name.",
        "#2467: default tab-order-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--visibility-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2467: default visibility-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --visibility-target-unique-id.",
        "#2467: default visibility-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--tab-stop-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2467: pseudo-localized tab-stop-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2467: pseudo-localized tab-stop-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--tab-stop-target-unique-id",
        "#2467: pseudo-localized tab-stop-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --tab-stop-target-unique-id.",
        "#2467: pseudo-localized tab-stop-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--enabled-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2467: pseudo-localized enabled-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2467: pseudo-localized enabled-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--enabled-target-object-name",
        "#2467: pseudo-localized enabled-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --enabled-target-object-name.",
        "#2467: pseudo-localized enabled-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--locked-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2467: pseudo-localized locked-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2467: pseudo-localized locked-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--locked-target-unique-id",
        "#2467: pseudo-localized locked-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --locked-target-unique-id.",
        "#2467: pseudo-localized locked-target-unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_media_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_media_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--caption-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2468: default caption-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --caption-target-object-name.",
        "#2468: default caption-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--picture-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2468: default picture-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --picture-target-unique-id.",
        "#2468: default picture-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--down-picture-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2468: pseudo-localized down-picture-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2468: pseudo-localized down-picture-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--down-picture-target-object-name",
        "#2468: pseudo-localized down-picture-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --down-picture-target-object-name.",
        "#2468: pseudo-localized down-picture-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--disabled-picture-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2468: pseudo-localized disabled-picture-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2468: pseudo-localized disabled-picture-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--disabled-picture-target-unique-id",
        "#2468: pseudo-localized disabled-picture-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --disabled-picture-target-unique-id.",
        "#2468: pseudo-localized disabled-picture-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--ole-drag-picture-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2468: pseudo-localized ole-drag-picture-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2468: pseudo-localized ole-drag-picture-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drag-picture-target-object-name",
        "#2468: pseudo-localized ole-drag-picture-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --ole-drag-picture-target-object-name.",
        "#2468: pseudo-localized ole-drag-picture-target-object-name missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_mouse_drag_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_mouse_drag_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--mouse-icon-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2469: default mouse-icon-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --mouse-icon-target-object-name.",
        "#2469: default mouse-icon-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--drag-mode-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2469: default drag-mode-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --drag-mode-target-unique-id.",
        "#2469: default drag-mode-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--drag-icon-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2469: pseudo-localized drag-icon-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2469: pseudo-localized drag-icon-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--drag-icon-target-object-name",
        "#2469: pseudo-localized drag-icon-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --drag-icon-target-object-name.",
        "#2469: pseudo-localized drag-icon-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--ole-drag-mode-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2469: pseudo-localized ole-drag-mode-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2469: pseudo-localized ole-drag-mode-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drag-mode-target-unique-id",
        "#2469: pseudo-localized ole-drag-mode-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --ole-drag-mode-target-unique-id.",
        "#2469: pseudo-localized ole-drag-mode-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--ole-drop-mode-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2469: pseudo-localized ole-drop-mode-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2469: pseudo-localized ole-drop-mode-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drop-mode-target-object-name",
        "#2469: pseudo-localized ole-drop-mode-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --ole-drop-mode-target-object-name.",
        "#2469: pseudo-localized ole-drop-mode-target-object-name missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_grid_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_grid_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--grid-line-color-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2472: default grid-line-color-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --grid-line-color-target-object-name.",
        "#2472: default grid-line-color-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--highlight-row-line-width-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2472: default highlight-row-line-width-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --highlight-row-line-width-target-unique-id.",
        "#2472: default highlight-row-line-width-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--header-height-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2472: pseudo-localized header-height-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2472: pseudo-localized header-height-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--header-height-target-unique-id",
        "#2472: pseudo-localized header-height-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --header-height-target-unique-id.",
        "#2472: pseudo-localized header-height-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--lock-columns-left-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2472: pseudo-localized lock-columns-left-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2472: pseudo-localized lock-columns-left-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--lock-columns-left-target-object-name",
        "#2472: pseudo-localized lock-columns-left-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --lock-columns-left-target-object-name.",
        "#2472: pseudo-localized lock-columns-left-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--grid-lines-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2472: pseudo-localized grid-lines-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2472: pseudo-localized grid-lines-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--grid-lines-target-unique-id",
        "#2472: pseudo-localized grid-lines-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --grid-lines-target-unique-id.",
        "#2472: pseudo-localized grid-lines-target-unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_partition_list_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_partition_list_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--partition-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2473: default partition-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --partition-target-object-name.",
        "#2473: default partition-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--list-item-id-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2473: default list-item-id-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --list-item-id-target-unique-id.",
        "#2473: default list-item-id-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--record-source-type-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2473: pseudo-localized record-source-type-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2473: pseudo-localized record-source-type-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record-source-type-target-unique-id",
        "#2473: pseudo-localized record-source-type-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --record-source-type-target-unique-id.",
        "#2473: pseudo-localized record-source-type-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--highlight-style-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2473: pseudo-localized highlight-style-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2473: pseudo-localized highlight-style-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--highlight-style-target-object-name",
        "#2473: pseudo-localized highlight-style-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --highlight-style-target-object-name.",
        "#2473: pseudo-localized highlight-style-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--fill-color-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2473: pseudo-localized fill-color-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2473: pseudo-localized fill-color-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--fill-color-target-unique-id",
        "#2473: pseudo-localized fill-color-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --fill-color-target-unique-id.",
        "#2473: pseudo-localized fill-color-target-unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_record_source_text_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_record_source_text_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--record-source-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2474: default record-source-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --record-source-target-object-name.",
        "#2474: default record-source-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--format-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2474: default format-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --format-target-unique-id.",
        "#2474: default format-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--tooltip-text-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2474: pseudo-localized tooltip-text-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2474: pseudo-localized tooltip-text-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--tooltip-text-target-unique-id",
        "#2474: pseudo-localized tooltip-text-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --tooltip-text-target-unique-id.",
        "#2474: pseudo-localized tooltip-text-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--link-master-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2474: pseudo-localized link-master-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2474: pseudo-localized link-master-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--link-master-target-object-name",
        "#2474: pseudo-localized link-master-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --link-master-target-object-name.",
        "#2474: pseudo-localized link-master-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--input-mask-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2474: pseudo-localized input-mask-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2474: pseudo-localized input-mask-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--input-mask-target-unique-id",
        "#2474: pseudo-localized input-mask-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --input-mask-target-unique-id.",
        "#2474: pseudo-localized input-mask-target-unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_row_source_list_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_row_source_list_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--row-source-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2475: default row-source-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --row-source-target-object-name.",
        "#2475: default row-source-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--multi-select-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2475: default multi-select-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --multi-select-target-unique-id.",
        "#2475: default multi-select-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--column-widths-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2475: pseudo-localized column-widths-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2475: pseudo-localized column-widths-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--column-widths-target-unique-id",
        "#2475: pseudo-localized column-widths-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --column-widths-target-unique-id.",
        "#2475: pseudo-localized column-widths-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--integral-height-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2475: pseudo-localized integral-height-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2475: pseudo-localized integral-height-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--integral-height-target-object-name",
        "#2475: pseudo-localized integral-height-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --integral-height-target-object-name.",
        "#2475: pseudo-localized integral-height-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--incremental-search-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2475: pseudo-localized incremental-search-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2475: pseudo-localized incremental-search-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--incremental-search-target-unique-id",
        "#2475: pseudo-localized incremental-search-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --incremental-search-target-unique-id.",
        "#2475: pseudo-localized incremental-search-target-unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_row_source_type_list_index_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_row_source_type_list_index_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--row-source-type-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2476: default row-source-type-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --row-source-type-target-object-name.",
        "#2476: default row-source-type-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--display-value-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2476: default display-value-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --display-value-target-unique-id.",
        "#2476: default display-value-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--bound-column-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2476: pseudo-localized bound-column-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2476: pseudo-localized bound-column-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--bound-column-target-unique-id",
        "#2476: pseudo-localized bound-column-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --bound-column-target-unique-id.",
        "#2476: pseudo-localized bound-column-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--style-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2476: pseudo-localized style-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2476: pseudo-localized style-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--style-target-object-name",
        "#2476: pseudo-localized style-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --style-target-object-name.",
        "#2476: pseudo-localized style-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--left-column-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2476: pseudo-localized left-column-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2476: pseudo-localized left-column-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--left-column-target-unique-id",
        "#2476: pseudo-localized left-column-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --left-column-target-unique-id.",
        "#2476: pseudo-localized left-column-target-unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_form_boolean_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_form_boolean_target_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--closable-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2479: default closable target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --closable-target-object-name.",
        "#2479: default closable target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--min-button-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2479: default min-button target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --min-button-target-unique-id.",
        "#2479: default min-button target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--auto-verb-menu-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2479: pseudo-localized auto-verb-menu target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2479: pseudo-localized auto-verb-menu target object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--auto-verb-menu-target-object-name",
        "#2479: pseudo-localized auto-verb-menu target object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --auto-verb-menu-target-object-name.",
        "#2479: pseudo-localized auto-verb-menu target object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--mac-desktop-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2479: pseudo-localized mac-desktop target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2479: pseudo-localized mac-desktop target unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--mac-desktop-target-unique-id",
        "#2479: pseudo-localized mac-desktop target unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --mac-desktop-target-unique-id.",
        "#2479: pseudo-localized mac-desktop target unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_dimension_border_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_dimension_border_target_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--min-height-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2480: default min-height target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --min-height-target-object-name.",
        "#2480: default min-height target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--border-color-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2480: default border-color target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --border-color-target-unique-id.",
        "#2480: default border-color target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--half-height-caption-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2480: pseudo-localized half-height-caption target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2480: pseudo-localized half-height-caption target object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--half-height-caption-target-object-name",
        "#2480: pseudo-localized half-height-caption target object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --half-height-caption-target-object-name.",
        "#2480: pseudo-localized half-height-caption target object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--border-style-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2480: pseudo-localized border-style target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2480: pseudo-localized border-style target unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--border-style-target-unique-id",
        "#2480: pseudo-localized border-style target unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --border-style-target-unique-id.",
        "#2480: pseudo-localized border-style target unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_dynamic_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_dynamic_target_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-input-mask-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2482: default dynamic-input-mask target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --dynamic-input-mask-target-object-name.",
        "#2482: default dynamic-input-mask target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-font-shadow-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2482: default dynamic-font-shadow target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --dynamic-font-shadow-target-unique-id.",
        "#2482: default dynamic-font-shadow target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-current-control-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2482: pseudo-localized dynamic-current-control target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2482: pseudo-localized dynamic-current-control target object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-current-control-target-object-name",
        "#2482: pseudo-localized dynamic-current-control target object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --dynamic-current-control-target-object-name.",
        "#2482: pseudo-localized dynamic-current-control target object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-font-outline-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2482: pseudo-localized dynamic-font-outline target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2482: pseudo-localized dynamic-font-outline target unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-font-outline-target-unique-id",
        "#2482: pseudo-localized dynamic-font-outline target unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --dynamic-font-outline-target-unique-id.",
        "#2482: pseudo-localized dynamic-font-outline target unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_max_selection_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_max_selection_target_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--max-width-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2484: default max-width target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --max-width-target-object-name.",
        "#2484: default max-width target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--hide-selection-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2484: default hide-selection target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --hide-selection-target-unique-id.",
        "#2484: default hide-selection target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--continuous-scroll-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2484: pseudo-localized continuous-scroll target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2484: pseudo-localized continuous-scroll target object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--continuous-scroll-target-object-name",
        "#2484: pseudo-localized continuous-scroll target object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --continuous-scroll-target-object-name.",
        "#2484: pseudo-localized continuous-scroll target object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--allow-cell-selection-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2484: pseudo-localized allow-cell-selection target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2484: pseudo-localized allow-cell-selection target unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--allow-cell-selection-target-unique-id",
        "#2484: pseudo-localized allow-cell-selection target unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --allow-cell-selection-target-unique-id.",
        "#2484: pseudo-localized allow-cell-selection target unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
