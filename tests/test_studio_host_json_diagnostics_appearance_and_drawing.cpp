// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_launch_ole_icon_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_ole_icon_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--ole-drop-text-insertion-object",
            "--ole-drop-text-insertion", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2430: default OLE drop text-insertion diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object OLE drop text-insertion assignment requires at least one target selector.",
        "#2430: default OLE drop text-insertion diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--ole-drag-picture-object",
            "--ole-drag-picture-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2430: pseudo-localized OLE drag-picture missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2430: pseudo-localized OLE drag-picture missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drag-picture",
        "#2430: pseudo-localized OLE drag-picture missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object OLE drag-picture assignment requires --ole-drag-picture.",
        "#2430: pseudo-localized OLE drag-picture missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--mouse-icon-object",
            "--mouse-icon", "pointer.ico",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2430: pseudo-localized mouse-icon missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2430: pseudo-localized mouse-icon missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object mouse-icon assignment requires at least one target selector.",
        "#2430: pseudo-localized mouse-icon missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--drag-mode", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2430: pseudo-localized drag-mode stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2430: pseudo-localized drag-mode stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--drag-mode-object",
        "#2430: pseudo-localized drag-mode stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Drag-mode arguments can only be used with --drag-mode-object.",
        "#2430: pseudo-localized drag-mode stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_drawing_data_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_drawing_data_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--data-session-object",
            "--data-session", "2",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2431: default data-session diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object data-session assignment requires at least one target selector.",
        "#2431: default data-session diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--button-count-object",
            "--button-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2431: pseudo-localized button-count missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2431: pseudo-localized button-count missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--button-count",
        "#2431: pseudo-localized button-count missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object button-count assignment requires --button-count.",
        "#2431: pseudo-localized button-count missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--curvature-object",
            "--curvature", "10",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2431: pseudo-localized curvature missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2431: pseudo-localized curvature missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object curvature assignment requires at least one target selector.",
        "#2431: pseudo-localized curvature missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--draw-mode", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2431: pseudo-localized draw-mode stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2431: pseudo-localized draw-mode stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--draw-mode-object",
        "#2431: pseudo-localized draw-mode stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Draw-mode arguments can only be used with --draw-mode-object.",
        "#2431: pseudo-localized draw-mode stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_grid_layout_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_grid_layout_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--header-height-object",
            "--header-height", "20",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2432: default header-height diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object header-height assignment requires at least one target selector.",
        "#2432: default header-height diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--grid-line-color-object",
            "--grid-line-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2432: pseudo-localized grid-line-color missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2432: pseudo-localized grid-line-color missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--grid-line-color",
        "#2432: pseudo-localized grid-line-color missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object grid-line-color assignment requires --grid-line-color.",
        "#2432: pseudo-localized grid-line-color missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--row-height-object",
            "--row-height", "20",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2432: pseudo-localized row-height missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2432: pseudo-localized row-height missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object row-height assignment requires at least one target selector.",
        "#2432: pseudo-localized row-height missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--grid-lines", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2432: pseudo-localized grid-lines stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2432: pseudo-localized grid-lines stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--grid-lines-object",
        "#2432: pseudo-localized grid-lines stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Grid-lines arguments can only be used with --grid-lines-object.",
        "#2432: pseudo-localized grid-lines stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_list_item_color_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_list_item_color_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--selected-back-color-object",
            "--selected-back-color", "255",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2438: default selected-back-color diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object selected-back-color assignment requires at least one target selector.",
        "#2438: default selected-back-color diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--selected-fore-color-object",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2438: pseudo-localized selected-fore-color missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2438: pseudo-localized selected-fore-color missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--selected-fore-color",
        "#2438: pseudo-localized selected-fore-color missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " selected-fore-color assignment ",
        "#2516: pseudo-localized selected-fore-color missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--selected-item-back-color-object",
            "--selected-item-back-color", "-1",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2438: pseudo-localized selected-item-back-color non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2438: pseudo-localized selected-item-back-color non-negative diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " selected-item-back-color assignment ",
        "#2516: pseudo-localized selected-item-back-color non-negative diagnostics should not preserve raw label prose");
    expect_not_contains(process.stdout_text,
        " value.",
        "#2516: pseudo-localized selected-item-back-color non-negative diagnostics should not preserve raw value label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "128",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2438: pseudo-localized disabled-item-fore-color missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2438: pseudo-localized disabled-item-fore-color missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " disabled-item-fore-color assignment ",
        "#2516: pseudo-localized disabled-item-fore-color missing-target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--item-fore-color", "255",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2438: pseudo-localized item-fore-color stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2438: pseudo-localized item-fore-color stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--item-fore-color-object",
        "#2438: pseudo-localized item-fore-color stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Item-fore-color arguments can only be used with --item-fore-color-object.",
        "#2516: pseudo-localized item-fore-color stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_general_color_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_general_color_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--highlight-back-color-object",
            "--highlight-back-color", "255",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2439: default highlight-back-color diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object highlight-back-color assignment requires at least one target selector.",
        "#2439: default highlight-back-color diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--highlight-fore-color-object",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2439: pseudo-localized highlight-fore-color missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2439: pseudo-localized highlight-fore-color missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--highlight-fore-color",
        "#2439: pseudo-localized highlight-fore-color missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " highlight-fore-color assignment ",
        "#2517: pseudo-localized highlight-fore-color missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--back-color-object",
            "--back-color", "-1",
            "--back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2439: pseudo-localized back-color non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2439: pseudo-localized back-color non-negative diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " back-color assignment ",
        "#2517: pseudo-localized back-color non-negative diagnostics should not preserve raw label prose");
    expect_not_contains(process.stdout_text,
        " value.",
        "#2517: pseudo-localized back-color non-negative diagnostics should not preserve raw value label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--disabled-fore-color-object",
            "--disabled-fore-color", "128",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2439: pseudo-localized disabled-fore-color missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2439: pseudo-localized disabled-fore-color missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " disabled-fore-color assignment ",
        "#2517: pseudo-localized disabled-fore-color missing-target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--dynamic-fore-color", "IIF(.T., 255, 0)",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2439: pseudo-localized dynamic-fore-color stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2439: pseudo-localized dynamic-fore-color stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-fore-color-object",
        "#2439: pseudo-localized dynamic-fore-color stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Dynamic-fore-color arguments can only be used with --dynamic-fore-color-object.",
        "#2517: pseudo-localized dynamic-fore-color stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_form_appearance_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_form_appearance_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--special-effect-object",
            "--special-effect", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2520: default special-effect diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object special-effect assignment requires at least one target selector.",
        "#2520: default special-effect diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--scroll-bars-object",
            "--scroll-bars-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2520: pseudo-localized scroll-bars missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2520: pseudo-localized scroll-bars missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--scroll-bars",
        "#2520: pseudo-localized scroll-bars missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object scroll-bars assignment requires --scroll-bars.",
        "#2520: pseudo-localized scroll-bars missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--picture-margin-object",
            "--picture-margin", "2",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2520: pseudo-localized picture-margin missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2520: pseudo-localized picture-margin missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object picture-margin assignment requires at least one target selector.",
        "#2520: pseudo-localized picture-margin missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--picture-selection-display", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2520: pseudo-localized picture-selection-display stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2520: pseudo-localized picture-selection-display stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--picture-selection-display-object",
        "#2520: pseudo-localized picture-selection-display stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Picture-selection-display arguments can only be used with --picture-selection-display-object.",
        "#2520: pseudo-localized picture-selection-display stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_font_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_font_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--dynamic-font-outline-object",
            "--dynamic-font-outline", "IIF(.T., .T., .F.)",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2521: default dynamic-font-outline diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object dynamic-font-outline assignment requires at least one target selector.",
        "#2521: default dynamic-font-outline diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2521: pseudo-localized dynamic-font-shadow missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2521: pseudo-localized dynamic-font-shadow missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-font-shadow",
        "#2521: pseudo-localized dynamic-font-shadow missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object dynamic-font-shadow assignment requires --dynamic-font-shadow.",
        "#2521: pseudo-localized dynamic-font-shadow missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--font-name-object",
            "--font-name", "Arial",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2522: pseudo-localized font-name missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2522: pseudo-localized font-name missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object font-name assignment requires at least one target selector.",
        "#2522: pseudo-localized font-name missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--font-shadow", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2522: pseudo-localized font-shadow stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2522: pseudo-localized font-shadow stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--font-shadow-object",
        "#2522: pseudo-localized font-shadow stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Font-shadow arguments can only be used with --font-shadow-object.",
        "#2522: pseudo-localized font-shadow stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_drag_ole_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_drag_ole_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--drag-mode"},
        temp_root);

    expect(process.exit_code == 2,
        "#2453: default drag-mode missing value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --drag-mode.",
        "#2453: default drag-mode missing value diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--ole-drag-mode", "move"},
        temp_root);

    expect(process.exit_code == 2,
        "#2453: pseudo-localized ole-drag-mode integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2453: pseudo-localized ole-drag-mode integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drag-mode",
        "#2453: pseudo-localized ole-drag-mode integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --ole-drag-mode value must be an integer.",
        "#2453: pseudo-localized ole-drag-mode integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--ole-drop-effects", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2453: pseudo-localized ole-drop-effects non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2453: pseudo-localized ole-drop-effects non-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drop-effects",
        "#2453: pseudo-localized ole-drop-effects non-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --ole-drop-effects value must be non-negative.",
        "#2453: pseudo-localized ole-drop-effects non-negative diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--ole-drop-text-insertion", "insert"},
        temp_root);

    expect(process.exit_code == 2,
        "#2453: pseudo-localized ole-drop-text-insertion integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2453: pseudo-localized ole-drop-text-insertion integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drop-text-insertion",
        "#2453: pseudo-localized ole-drop-text-insertion integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --ole-drop-text-insertion value must be an integer.",
        "#2453: pseudo-localized ole-drop-text-insertion integer diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_drawing_buffer_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_drawing_buffer_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--button-count"},
        temp_root);

    expect(process.exit_code == 2,
        "#2454: default button-count missing value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --button-count.",
        "#2454: default button-count missing value diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--draw-mode", "copy"},
        temp_root);

    expect(process.exit_code == 2,
        "#2454: pseudo-localized draw-mode integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2454: pseudo-localized draw-mode integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--draw-mode",
        "#2454: pseudo-localized draw-mode integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --draw-mode value must be an integer.",
        "#2454: pseudo-localized draw-mode integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--draw-width", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2454: pseudo-localized draw-width non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2454: pseudo-localized draw-width non-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--draw-width",
        "#2454: pseudo-localized draw-width non-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --draw-width value must be non-negative.",
        "#2454: pseudo-localized draw-width non-negative diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--buffer-mode-override", "override"},
        temp_root);

    expect(process.exit_code == 2,
        "#2454: pseudo-localized buffer-mode-override integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2454: pseudo-localized buffer-mode-override integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--buffer-mode-override",
        "#2454: pseudo-localized buffer-mode-override integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --buffer-mode-override value must be an integer.",
        "#2454: pseudo-localized buffer-mode-override integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--data-session", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2454: pseudo-localized data-session non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2454: pseudo-localized data-session non-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--data-session",
        "#2454: pseudo-localized data-session non-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --data-session value must be non-negative.",
        "#2454: pseudo-localized data-session non-negative diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_display_dynamic_color_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_display_dynamic_color_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--display-value"},
        temp_root);

    expect(process.exit_code == 2,
        "#2457: default display-value missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --display-value.",
        "#2457: default display-value missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-back-color"},
        temp_root);

    expect(process.exit_code == 2,
        "#2457: pseudo-localized dynamic-back-color missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2457: pseudo-localized dynamic-back-color missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-back-color",
        "#2457: pseudo-localized dynamic-back-color missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --dynamic-back-color.",
        "#2457: pseudo-localized dynamic-back-color missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-fore-color"},
        temp_root);

    expect(process.exit_code == 2,
        "#2457: pseudo-localized dynamic-fore-color missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2457: pseudo-localized dynamic-fore-color missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-fore-color",
        "#2457: pseudo-localized dynamic-fore-color missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --dynamic-fore-color.",
        "#2457: pseudo-localized dynamic-fore-color missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_form_appearance_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_form_appearance_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--special-effect"},
        temp_root);

    expect(process.exit_code == 2,
        "#2460: default special-effect missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --special-effect.",
        "#2460: default special-effect missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--special-effect", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2460: default special-effect not-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --special-effect value must not be negative.",
        "#2460: default special-effect not-negative diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--window-state", "zoomed"},
        temp_root);

    expect(process.exit_code == 2,
        "#2460: pseudo-localized window-state integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2460: pseudo-localized window-state integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--window-state",
        "#2460: pseudo-localized window-state integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --window-state value must be an integer.",
        "#2460: pseudo-localized window-state integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--picture-position", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2460: pseudo-localized picture-position not-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2460: pseudo-localized picture-position not-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--picture-position",
        "#2460: pseudo-localized picture-position not-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --picture-position value must not be negative.",
        "#2460: pseudo-localized picture-position not-negative diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--picture-selection-display", "selected"},
        temp_root);

    expect(process.exit_code == 2,
        "#2460: pseudo-localized picture-selection-display integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2460: pseudo-localized picture-selection-display integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--picture-selection-display",
        "#2460: pseudo-localized picture-selection-display integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --picture-selection-display value must be an integer.",
        "#2460: pseudo-localized picture-selection-display integer diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_font_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_font_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--font-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2462: default font-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --font-name.",
        "#2462: default font-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--font-size", "huge"},
        temp_root);

    expect(process.exit_code == 2,
        "#2462: default font-size numeric diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --font-size value must be numeric.",
        "#2462: default font-size numeric diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--font-size", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2462: pseudo-localized font-size not-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2462: pseudo-localized font-size not-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--font-size",
        "#2462: pseudo-localized font-size not-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --font-size value must not be negative.",
        "#2462: pseudo-localized font-size not-negative diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--font-bold", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2462: pseudo-localized font-bold true/false diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2462: pseudo-localized font-bold true/false diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--font-bold",
        "#2462: pseudo-localized font-bold true/false diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --font-bold value must be true or false.",
        "#2462: pseudo-localized font-bold true/false diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--font-shadow"},
        temp_root);

    expect(process.exit_code == 2,
        "#2462: pseudo-localized font-shadow missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2462: pseudo-localized font-shadow missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--font-shadow",
        "#2462: pseudo-localized font-shadow missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --font-shadow.",
        "#2462: pseudo-localized font-shadow missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_marker_sizing_zorder_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_marker_sizing_zorder_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--delete-mark"},
        temp_root);

    expect(process.exit_code == 2,
        "#2464: default delete-mark missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --delete-mark.",
        "#2464: default delete-mark missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--record-mark", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2464: default record-mark true/false diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --record-mark value must be true or false.",
        "#2464: default record-mark true/false diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--highlight-row"},
        temp_root);

    expect(process.exit_code == 2,
        "#2464: pseudo-localized highlight-row missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2464: pseudo-localized highlight-row missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--highlight-row",
        "#2464: pseudo-localized highlight-row missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --highlight-row.",
        "#2464: pseudo-localized highlight-row missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--allow-header-sizing", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2464: pseudo-localized allow-header-sizing true/false diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2464: pseudo-localized allow-header-sizing true/false diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--allow-header-sizing",
        "#2464: pseudo-localized allow-header-sizing true/false diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --allow-header-sizing value must be true or false.",
        "#2464: pseudo-localized allow-header-sizing true/false diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--always-on-bottom", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2464: pseudo-localized always-on-bottom true/false diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2464: pseudo-localized always-on-bottom true/false diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--always-on-bottom",
        "#2464: pseudo-localized always-on-bottom true/false diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --always-on-bottom value must be true or false.",
        "#2464: pseudo-localized always-on-bottom true/false diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_color_value_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_color_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--selected-back-color"},
        temp_root);

    expect(process.exit_code == 2,
        "#2465: default selected-back-color missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --selected-back-color.",
        "#2465: default selected-back-color missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--selected-item-fore-color", "blue"},
        temp_root);

    expect(process.exit_code == 2,
        "#2465: default selected-item-fore-color integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "The --selected-item-fore-color value must be an integer.",
        "#2465: default selected-item-fore-color integer diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--disabled-item-back-color"},
        temp_root);

    expect(process.exit_code == 2,
        "#2465: pseudo-localized disabled-item-back-color missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2465: pseudo-localized disabled-item-back-color missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--disabled-item-back-color",
        "#2465: pseudo-localized disabled-item-back-color missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --disabled-item-back-color.",
        "#2465: pseudo-localized disabled-item-back-color missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--highlight-fore-color", "bright"},
        temp_root);

    expect(process.exit_code == 2,
        "#2465: pseudo-localized highlight-fore-color integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2465: pseudo-localized highlight-fore-color integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--highlight-fore-color",
        "#2465: pseudo-localized highlight-fore-color integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --highlight-fore-color value must be an integer.",
        "#2465: pseudo-localized highlight-fore-color integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--disabled-fore-color", "dim"},
        temp_root);

    expect(process.exit_code == 2,
        "#2465: pseudo-localized disabled-fore-color integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2465: pseudo-localized disabled-fore-color integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--disabled-fore-color",
        "#2465: pseudo-localized disabled-fore-color integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --disabled-fore-color value must be an integer.",
        "#2465: pseudo-localized disabled-fore-color integer diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_ole_drop_drawing_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_ole_drop_drawing_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--ole-drop-effects-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2470: default ole-drop-effects-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --ole-drop-effects-target-object-name.",
        "#2470: default ole-drop-effects-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--draw-style-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2470: default draw-style-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --draw-style-target-unique-id.",
        "#2470: default draw-style-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--ole-drop-text-insertion-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2470: pseudo-localized ole-drop-text-insertion-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2470: pseudo-localized ole-drop-text-insertion-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drop-text-insertion-target-unique-id",
        "#2470: pseudo-localized ole-drop-text-insertion-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --ole-drop-text-insertion-target-unique-id.",
        "#2470: pseudo-localized ole-drop-text-insertion-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--button-count-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2470: pseudo-localized button-count-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2470: pseudo-localized button-count-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--button-count-target-object-name",
        "#2470: pseudo-localized button-count-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --button-count-target-object-name.",
        "#2470: pseudo-localized button-count-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--draw-mode-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2470: pseudo-localized draw-mode-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2470: pseudo-localized draw-mode-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--draw-mode-target-unique-id",
        "#2470: pseudo-localized draw-mode-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --draw-mode-target-unique-id.",
        "#2470: pseudo-localized draw-mode-target-unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_drawing_buffer_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_drawing_buffer_target_selector_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--draw-width-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2471: default draw-width-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --draw-width-target-object-name.",
        "#2471: default draw-width-target-object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--data-session-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2471: default data-session-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --data-session-target-unique-id.",
        "#2471: default data-session-target-unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--fill-style-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2471: pseudo-localized fill-style-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2471: pseudo-localized fill-style-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--fill-style-target-unique-id",
        "#2471: pseudo-localized fill-style-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --fill-style-target-unique-id.",
        "#2471: pseudo-localized fill-style-target-unique-id missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--buffer-mode-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2471: pseudo-localized buffer-mode-target-object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2471: pseudo-localized buffer-mode-target-object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--buffer-mode-target-object-name",
        "#2471: pseudo-localized buffer-mode-target-object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --buffer-mode-target-object-name.",
        "#2471: pseudo-localized buffer-mode-target-object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--buffer-mode-override-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2471: pseudo-localized buffer-mode-override-target-unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2471: pseudo-localized buffer-mode-override-target-unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--buffer-mode-override-target-unique-id",
        "#2471: pseudo-localized buffer-mode-override-target-unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --buffer-mode-override-target-unique-id.",
        "#2471: pseudo-localized buffer-mode-override-target-unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_color_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_color_target_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--selected-back-color-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2478: default selected-back-color target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --selected-back-color-target-object-name.",
        "#2478: default selected-back-color target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--disabled-fore-color-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2478: default disabled-fore-color target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --disabled-fore-color-target-unique-id.",
        "#2478: default disabled-fore-color target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-back-color-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2478: pseudo-localized dynamic-back-color target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2478: pseudo-localized dynamic-back-color target object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-back-color-target-object-name",
        "#2478: pseudo-localized dynamic-back-color target object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --dynamic-back-color-target-object-name.",
        "#2478: pseudo-localized dynamic-back-color target object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--dynamic-fore-color-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2478: pseudo-localized dynamic-fore-color target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2478: pseudo-localized dynamic-fore-color target unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-fore-color-target-unique-id",
        "#2478: pseudo-localized dynamic-fore-color target unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --dynamic-fore-color-target-unique-id.",
        "#2478: pseudo-localized dynamic-fore-color target unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_window_picture_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_window_picture_target_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--special-effect-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2481: default special-effect target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --special-effect-target-object-name.",
        "#2481: default special-effect target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--picture-selection-display-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2481: default picture-selection-display target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --picture-selection-display-target-unique-id.",
        "#2481: default picture-selection-display target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--window-state-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2481: pseudo-localized window-state target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2481: pseudo-localized window-state target object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--window-state-target-object-name",
        "#2481: pseudo-localized window-state target object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --window-state-target-object-name.",
        "#2481: pseudo-localized window-state target object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--picture-spacing-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2481: pseudo-localized picture-spacing target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2481: pseudo-localized picture-spacing target unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--picture-spacing-target-unique-id",
        "#2481: pseudo-localized picture-spacing target unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --picture-spacing-target-unique-id.",
        "#2481: pseudo-localized picture-spacing target unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_font_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_font_target_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--font-name-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2483: default font-name target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --font-name-target-object-name.",
        "#2483: default font-name target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--font-shadow-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2483: default font-shadow target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --font-shadow-target-unique-id.",
        "#2483: default font-shadow target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--font-bold-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2483: pseudo-localized font-bold target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2483: pseudo-localized font-bold target object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--font-bold-target-object-name",
        "#2483: pseudo-localized font-bold target object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --font-bold-target-object-name.",
        "#2483: pseudo-localized font-bold target object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--font-outline-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2483: pseudo-localized font-outline target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2483: pseudo-localized font-outline target unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--font-outline-target-unique-id",
        "#2483: pseudo-localized font-outline target unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --font-outline-target-unique-id.",
        "#2483: pseudo-localized font-outline target unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_marker_sizing_zorder_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_marker_sizing_zorder_target_value_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {"--json", "--delete-mark-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2485: default delete-mark target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --delete-mark-target-object-name.",
        "#2485: default delete-mark target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--always-on-bottom-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2485: default always-on-bottom target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --always-on-bottom-target-unique-id.",
        "#2485: default always-on-bottom target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {"--json", "--highlight-row-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2485: pseudo-localized highlight-row target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2485: pseudo-localized highlight-row target object-name missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--highlight-row-target-object-name",
        "#2485: pseudo-localized highlight-row target object-name missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --highlight-row-target-object-name.",
        "#2485: pseudo-localized highlight-row target object-name missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--allow-row-sizing-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2485: pseudo-localized allow-row-sizing target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2485: pseudo-localized allow-row-sizing target unique-id missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--allow-row-sizing-target-unique-id",
        "#2485: pseudo-localized allow-row-sizing target unique-id missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --allow-row-sizing-target-unique-id.",
        "#2485: pseudo-localized allow-row-sizing target unique-id missing diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
