// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_launch_object_metadata_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_object_metadata_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--form-set-class-object",
            "--form-set-class", "BaseFormSet",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2426: default object metadata diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "status: error",
        "#2426: default object metadata diagnostics should preserve text status contracts");
    expect_contains(process.stdout_text,
        "An object form set class assignment requires at least one target selector.",
        "#2426: default object metadata diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--form-set-class-target-object-name"},
        temp_root);

    expect(process.exit_code == 2,
        "#2487: default form-set-class target object-name missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --form-set-class-target-object-name.",
        "#2487: default form-set-class target object-name missing diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--whats-this-button-target-unique-id"},
        temp_root);

    expect(process.exit_code == 2,
        "#2487: default whats-this-button target unique-id missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Missing value after --whats-this-button-target-unique-id.",
        "#2487: default whats-this-button target unique-id missing diagnostics should preserve en-US prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--clear-property",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2504: pseudo-localized clear-property required option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2504: pseudo-localized clear-property required option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--property-name",
        "#2504: pseudo-localized clear-property required option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "A property clear requires --property-name.",
        "#2504: pseudo-localized clear-property required option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--rename-property",
            "--property-name", "Caption",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2504: pseudo-localized rename-property new-name diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2504: pseudo-localized rename-property new-name diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--new-property-name",
        "#2504: pseudo-localized rename-property new-name diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "A property rename requires --new-property-name.",
        "#2504: pseudo-localized rename-property new-name diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--form-set-class-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2506: pseudo-localized form-set-class missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2506: pseudo-localized form-set-class missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--form-set-class",
        "#2506: pseudo-localized form-set-class missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "form set class",
        "#2506: pseudo-localized form-set-class missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--default-file-path-object",
            "--default-file-path-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2426: pseudo-localized missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2426: pseudo-localized missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--default-file-path",
        "#2426: pseudo-localized missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object default file path assignment requires --default-file-path.",
        "#2426: pseudo-localized missing-option diagnostics should not fall back to raw English prose");
    expect_not_contains(process.stdout_text,
        "default file path",
        "#2506: pseudo-localized default-file-path diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--whats-this-button", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2426: pseudo-localized stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2426: pseudo-localized stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--whats-this-button-object",
        "#2426: pseudo-localized stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Whats-this-button arguments can only be used with --whats-this-button-object.",
        "#2426: pseudo-localized stray-argument diagnostics should not fall back to raw English prose");
    expect_not_contains(process.stdout_text,
        "Whats-this-button",
        "#2506: pseudo-localized whats-this-button diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--tab-order-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2507: pseudo-localized tab-order target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2507: pseudo-localized tab-order target diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--tab-order-object",
        "#2507: pseudo-localized tab-order target diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " tab-order ",
        "#2507: pseudo-localized tab-order target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--tab-order-object",
            "--starting-tab-index", "-1",
            "--tab-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2507: pseudo-localized tab-order non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2507: pseudo-localized tab-order non-negative diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "starting tab index",
        "#2507: pseudo-localized tab-order non-negative diagnostics should not preserve raw value-label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--enabled", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2507: pseudo-localized enabled stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2507: pseudo-localized enabled stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--enabled-object",
        "#2507: pseudo-localized enabled stray-argument diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Enabled arguments",
        "#2507: pseudo-localized enabled stray-argument diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--picture", "assets/logo.bmp",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2507: pseudo-localized picture stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2507: pseudo-localized picture stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--picture-object",
        "#2507: pseudo-localized picture stray-argument diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Picture arguments",
        "#2507: pseudo-localized picture stray-argument diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--ole-drag-picture-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2508: pseudo-localized OLE drag-picture missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2508: pseudo-localized OLE drag-picture missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drag-picture",
        "#2508: pseudo-localized OLE drag-picture missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " OLE drag-picture ",
        "#2508: pseudo-localized OLE drag-picture missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--mouse-icon-object",
            "--mouse-icon", "cursor.ico",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2508: pseudo-localized mouse-icon target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2508: pseudo-localized mouse-icon target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " mouse-icon ",
        "#2508: pseudo-localized mouse-icon target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--drag-mode", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2508: pseudo-localized drag-mode stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2508: pseudo-localized drag-mode stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--drag-mode-object",
        "#2508: pseudo-localized drag-mode stray-argument diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Drag-mode arguments",
        "#2508: pseudo-localized drag-mode stray-argument diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--ole-drop-text-insertion-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2508: pseudo-localized OLE drop text-insertion stray diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2508: pseudo-localized OLE drop text-insertion stray diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--ole-drop-text-insertion-object",
        "#2508: pseudo-localized OLE drop text-insertion stray diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "OLE drop text-insertion arguments",
        "#2508: pseudo-localized OLE drop text-insertion diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--button-count-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2509: pseudo-localized button-count missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2509: pseudo-localized button-count missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--button-count",
        "#2509: pseudo-localized button-count missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " button-count ",
        "#2509: pseudo-localized button-count missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--curvature-object",
            "--curvature", "5",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2509: pseudo-localized curvature target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2509: pseudo-localized curvature target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " curvature ",
        "#2509: pseudo-localized curvature target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--draw-mode", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2509: pseudo-localized draw-mode stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2509: pseudo-localized draw-mode stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--draw-mode-object",
        "#2509: pseudo-localized draw-mode stray-argument diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Draw-mode arguments",
        "#2509: pseudo-localized draw-mode stray-argument diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--buffer-mode-override-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2509: pseudo-localized buffer-mode-override stray diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2509: pseudo-localized buffer-mode-override stray diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--buffer-mode-override-object",
        "#2509: pseudo-localized buffer-mode-override stray diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Buffer-mode-override arguments",
        "#2509: pseudo-localized buffer-mode-override diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--grid-line-color-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2510: pseudo-localized grid-line-color missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2510: pseudo-localized grid-line-color missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--grid-line-color",
        "#2510: pseudo-localized grid-line-color missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " grid-line-color ",
        "#2510: pseudo-localized grid-line-color missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--header-height-object",
            "--header-height", "32",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2510: pseudo-localized header-height target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2510: pseudo-localized header-height target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " header-height ",
        "#2510: pseudo-localized header-height target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--lock-columns-left-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2510: pseudo-localized lock-columns-left stray diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2510: pseudo-localized lock-columns-left stray diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--lock-columns-left-object",
        "#2510: pseudo-localized lock-columns-left stray diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Lock-columns-left arguments",
        "#2510: pseudo-localized lock-columns-left diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--partition-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2511: pseudo-localized partition missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2511: pseudo-localized partition missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--partition",
        "#2511: pseudo-localized partition missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " partition ",
        "#2511: pseudo-localized partition missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--column-order-object",
            "--column-order", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2511: pseudo-localized column-order target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2511: pseudo-localized column-order target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " column-order ",
        "#2511: pseudo-localized column-order target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--highlight-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2511: pseudo-localized highlight-style stray diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2511: pseudo-localized highlight-style stray diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--highlight-style-object",
        "#2511: pseudo-localized highlight-style stray diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "HighlightStyle arguments",
        "#2511: pseudo-localized highlight-style diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--record-source-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2512: pseudo-localized record-source missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2512: pseudo-localized record-source missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record-source",
        "#2512: pseudo-localized record-source missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " record source ",
        "#2512: pseudo-localized record-source missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--tooltip-text-object",
            "--tooltip-text", "Save the record",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2512: pseudo-localized tooltip-text target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2512: pseudo-localized tooltip-text target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " tooltip text ",
        "#2512: pseudo-localized tooltip-text target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--control-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2512: pseudo-localized control-source stray diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2512: pseudo-localized control-source stray diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--control-source-object",
        "#2512: pseudo-localized control-source stray diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Control-source arguments",
        "#2512: pseudo-localized control-source diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--row-source-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2513: pseudo-localized row-source missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2513: pseudo-localized row-source missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--row-source",
        "#2513: pseudo-localized row-source missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " row-source ",
        "#2513: pseudo-localized row-source missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--column-widths-object",
            "--column-widths", "120,80",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2513: pseudo-localized column-widths target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2513: pseudo-localized column-widths target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " column-widths ",
        "#2513: pseudo-localized column-widths target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--incremental-search-object",
            "--incremental-search-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2513: pseudo-localized incremental-search missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2513: pseudo-localized incremental-search missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--incremental-search",
        "#2513: pseudo-localized incremental-search missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " incremental-search ",
        "#2513: pseudo-localized incremental-search missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--multi-select-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2513: pseudo-localized multi-select stray diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2513: pseudo-localized multi-select stray diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--multi-select-object",
        "#2513: pseudo-localized multi-select stray diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Multi-select arguments",
        "#2513: pseudo-localized multi-select diagnostics should not preserve raw title label prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--default-file-path"},
        temp_root);

    expect(process.exit_code == 2,
        "#2487: pseudo-localized default-file-path missing diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2487: pseudo-localized default-file-path missing diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--default-file-path",
        "#2487: pseudo-localized default-file-path missing diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "Missing value after --default-file-path.",
        "#2487: pseudo-localized default-file-path missing diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--tab-orientation", "left"},
        temp_root);

    expect(process.exit_code == 2,
        "#2487: pseudo-localized tab-orientation integer diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2487: pseudo-localized tab-orientation integer diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--tab-orientation",
        "#2487: pseudo-localized tab-orientation integer diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --tab-orientation value must be an integer.",
        "#2487: pseudo-localized tab-orientation integer diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--help-context-id", "-1"},
        temp_root);

    expect(process.exit_code == 2,
        "#2487: pseudo-localized help-context-id not-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2487: pseudo-localized help-context-id not-negative diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--help-context-id",
        "#2487: pseudo-localized help-context-id not-negative diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --help-context-id value must not be negative.",
        "#2487: pseudo-localized help-context-id not-negative diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {"--json", "--whats-this-help", "maybe"},
        temp_root);

    expect(process.exit_code == 2,
        "#2487: pseudo-localized whats-this-help logical diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2487: pseudo-localized whats-this-help logical diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--whats-this-help",
        "#2487: pseudo-localized whats-this-help logical diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "The --whats-this-help value must be a logical value.",
        "#2487: pseudo-localized whats-this-help logical diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_layout_action_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_layout_action_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--align-object",
            "--alignment-mode", "left",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2427: default layout action diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object alignment requires --anchor-object-name or --anchor-unique-id.",
        "#2427: default layout action diagnostics should preserve en-US either-option prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--align-object",
            "--alignment-mode", "left",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2503: pseudo-localized alignment either-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2503: pseudo-localized alignment either-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--anchor-object-name",
        "#2503: pseudo-localized alignment either-option diagnostics should preserve first CLI option");
    expect_contains(process.stdout_text,
        "--anchor-unique-id",
        "#2503: pseudo-localized alignment either-option diagnostics should preserve second CLI option");
    expect_not_contains(process.stdout_text,
        "An object alignment requires --anchor-object-name or --anchor-unique-id.",
        "#2503: pseudo-localized alignment either-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--distribute-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2427: pseudo-localized missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2427: pseudo-localized missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--distribution-mode",
        "#2427: pseudo-localized missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object distribution requires --distribution-mode.",
        "#2427: pseudo-localized missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--resize-object",
            "--resize-mode", "match-width",
            "--anchor-object-name", "cmdSave",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2427: pseudo-localized missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2427: pseudo-localized missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object resize requires at least one target selector.",
        "#2427: pseudo-localized missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--snap-mode", "grid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2427: pseudo-localized stray-mode diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2427: pseudo-localized stray-mode diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--snap-object",
        "#2427: pseudo-localized stray-mode diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Snap arguments can only be used with --snap-object.",
        "#2427: pseudo-localized stray-mode diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--nudge-mode", "up",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2503: pseudo-localized nudge stray-mode diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2503: pseudo-localized nudge stray-mode diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--nudge-object",
        "#2503: pseudo-localized nudge stray-mode diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Nudge arguments can only be used with --nudge-object.",
        "#2503: pseudo-localized nudge stray-mode diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_tab_visibility_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_tab_visibility_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--tab-order-object",
            "--starting-tab-index", "-1",
            "--tab-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2428: default tab-order diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object tab-order assignment requires a non-negative starting tab index.",
        "#2428: default tab-order diagnostics should preserve en-US non-negative-value prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--tab-stop-object",
            "--tab-stop-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2428: pseudo-localized tab-stop missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2428: pseudo-localized tab-stop missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--tab-stop",
        "#2428: pseudo-localized tab-stop missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object tab-stop assignment requires --tab-stop.",
        "#2428: pseudo-localized tab-stop missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--visibility-object",
            "--visible", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2428: pseudo-localized visibility missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2428: pseudo-localized visibility missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object visibility assignment requires at least one target selector.",
        "#2428: pseudo-localized visibility missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--enabled", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2428: pseudo-localized enabled stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2428: pseudo-localized enabled stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--enabled-object",
        "#2428: pseudo-localized enabled stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Enabled arguments can only be used with --enabled-object.",
        "#2428: pseudo-localized enabled stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_basic_visual_property_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_basic_visual_property_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--caption-object",
            "--caption", "Save",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2429: default caption diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object caption assignment requires at least one target selector.",
        "#2429: default caption diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--read-only-object",
            "--read-only-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2429: pseudo-localized read-only missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2429: pseudo-localized read-only missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--object-read-only",
        "#2429: pseudo-localized read-only missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object read-only assignment requires --object-read-only.",
        "#2429: pseudo-localized read-only missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--locked-object",
            "--locked", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2429: pseudo-localized locked missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2429: pseudo-localized locked missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object locked assignment requires at least one target selector.",
        "#2429: pseudo-localized locked missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--picture", "save.bmp",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2429: pseudo-localized picture stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2429: pseudo-localized picture stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--picture-object",
        "#2429: pseudo-localized picture stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Picture arguments can only be used with --picture-object.",
        "#2429: pseudo-localized picture stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_list_grid_property_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_list_grid_property_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--partition-object",
            "--partition", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2433: default partition diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object partition assignment requires at least one target selector.",
        "#2433: default partition diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--record-source-type-object",
            "--record-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2433: pseudo-localized record-source-type missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2433: pseudo-localized record-source-type missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--record-source-type",
        "#2433: pseudo-localized record-source-type missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object record-source-type assignment requires --record-source-type.",
        "#2433: pseudo-localized record-source-type missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--fill-color-object",
            "--fill-color", "255",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2433: pseudo-localized fill-color missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2433: pseudo-localized fill-color missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object fill-color assignment requires at least one target selector.",
        "#2433: pseudo-localized fill-color missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--list-item-id", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2433: pseudo-localized list-item-id stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2433: pseudo-localized list-item-id stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--list-item-id-object",
        "#2433: pseudo-localized list-item-id stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "ListItemId arguments can only be used with --list-item-id-object.",
        "#2433: pseudo-localized list-item-id stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_text_binding_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_text_binding_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--record-source-object",
            "--record-source", "customers",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2434: default record-source diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object record source assignment requires at least one target selector.",
        "#2434: default record-source diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--tooltip-text-object",
            "--tooltip-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2434: pseudo-localized tooltip-text missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2434: pseudo-localized tooltip-text missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--tooltip-text",
        "#2434: pseudo-localized tooltip-text missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object tooltip text assignment requires --tooltip-text.",
        "#2434: pseudo-localized tooltip-text missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--control-source-object",
            "--control-source", "customer.name",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2434: pseudo-localized control-source missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2434: pseudo-localized control-source missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object control-source assignment requires at least one target selector.",
        "#2434: pseudo-localized control-source missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--format-object",
            "--format-target-object-name", "txtName",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2505: pseudo-localized format-object missing-format diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2505: pseudo-localized format-object missing-format diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--format",
        "#2505: pseudo-localized format-object missing-format diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object format assignment requires --format.",
        "#2505: pseudo-localized format-object missing-format diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--format", "@!",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2434: pseudo-localized format stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2434: pseudo-localized format stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--format-object",
        "#2434: pseudo-localized format stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Format arguments can only be used with --format-object.",
        "#2434: pseudo-localized format stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_row_list_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_row_list_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--row-source-object",
            "--row-source", "customers.name",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2435: default row-source diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object row-source assignment requires at least one target selector.",
        "#2435: default row-source diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--column-widths-object",
            "--column-widths-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2435: pseudo-localized column-widths missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2435: pseudo-localized column-widths missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--column-widths",
        "#2435: pseudo-localized column-widths missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object column-widths assignment requires --column-widths.",
        "#2435: pseudo-localized column-widths missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--integral-height-object",
            "--integral-height", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2435: pseudo-localized integral-height missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2435: pseudo-localized integral-height missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object integral-height assignment requires at least one target selector.",
        "#2435: pseudo-localized integral-height missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--multi-select", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2435: pseudo-localized multi-select stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2435: pseudo-localized multi-select stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--multi-select-object",
        "#2435: pseudo-localized multi-select stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Multi-select arguments can only be used with --multi-select-object.",
        "#2435: pseudo-localized multi-select stray-argument diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--row-source-type-object",
            "--row-source-type", "-1",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2435: pseudo-localized row-source-type non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2435: pseudo-localized row-source-type non-negative diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " row-source-type assignment ",
        "#2515: pseudo-localized row-source-type non-negative diagnostics should not preserve raw label prose");
    expect_not_contains(process.stdout_text,
        " value.",
        "#2515: pseudo-localized row-source-type non-negative diagnostics should not preserve raw value label prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_deleted_state_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_deleted_state_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--deleted-states",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2436: default deleted-states diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "A deleted-states request requires at least one target selector.",
        "#2436: default deleted-states diagnostics should preserve en-US target-selector prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--deleted-states",
            "--deleted-state-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2436: pseudo-localized deleted-state item diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2436: pseudo-localized deleted-state item diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--deleted-state",
        "#2436: pseudo-localized deleted-state item diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " deleted-states item ",
        "#2514: pseudo-localized deleted-state item diagnostics should not preserve raw request label prose");
    expect_not_contains(process.stdout_text,
        " target selector",
        "#2514: pseudo-localized deleted-state item diagnostics should not preserve raw selector label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--deleted-state-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2436: pseudo-localized deleted-state target mode diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2436: pseudo-localized deleted-state target mode diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--deleted-states",
        "#2436: pseudo-localized deleted-state target mode diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Deleted-state target arguments can only be used with --deleted-states.",
        "#2514: pseudo-localized deleted-state target mode diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--subtree-deleted-state",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2436: pseudo-localized subtree missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2436: pseudo-localized subtree missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--subtree-deleted",
        "#2436: pseudo-localized subtree missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " subtree deleted-state request ",
        "#2514: pseudo-localized subtree missing-option diagnostics should not preserve raw request label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--subtree-deleted-state",
            "--subtree-deleted", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2436: pseudo-localized subtree root diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2436: pseudo-localized subtree root diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " subtree deleted-state request ",
        "#2514: pseudo-localized subtree root diagnostics should not preserve raw request label prose");
    expect_not_contains(process.stdout_text,
        " root selector",
        "#2514: pseudo-localized subtree root diagnostics should not preserve raw selector label prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_bound_list_numeric_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_bound_list_numeric_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--bound-column-object",
            "--bound-column", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2437: default bound-column diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object bound-column assignment requires at least one target selector.",
        "#2437: default bound-column diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--column-count-object",
            "--column-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2437: pseudo-localized column-count missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2437: pseudo-localized column-count missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--column-count",
        "#2437: pseudo-localized column-count missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " column-count assignment ",
        "#2515: pseudo-localized column-count missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--style-object",
            "--style", "-1",
            "--style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2437: pseudo-localized style non-negative diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2437: pseudo-localized style non-negative diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " style assignment ",
        "#2515: pseudo-localized style non-negative diagnostics should not preserve raw label prose");
    expect_not_contains(process.stdout_text,
        " value.",
        "#2515: pseudo-localized style non-negative diagnostics should not preserve raw value label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--display-value-object",
            "--display-value", "display text",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2437: pseudo-localized display-value missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2437: pseudo-localized display-value missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " display-value assignment ",
        "#2515: pseudo-localized display-value missing-target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--left-column", "1",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2437: pseudo-localized left-column stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2437: pseudo-localized left-column stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--left-column-object",
        "#2437: pseudo-localized left-column stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Left-column arguments can only be used with --left-column-object.",
        "#2515: pseudo-localized left-column stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_window_flag_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_window_flag_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--closable-object",
            "--closable", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2440: default closable diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object closable assignment requires at least one target selector.",
        "#2440: default closable diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--control-box-object",
            "--control-box-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2440: pseudo-localized control-box missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2440: pseudo-localized control-box missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--control-box",
        "#2440: pseudo-localized control-box missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " control-box assignment ",
        "#2518: pseudo-localized control-box missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--allow-output-object",
            "--allow-output", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2440: pseudo-localized allow-output missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2440: pseudo-localized allow-output missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " allow-output assignment ",
        "#2518: pseudo-localized allow-output missing-target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--max-button", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2440: pseudo-localized max-button stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2440: pseudo-localized max-button stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--max-button-object",
        "#2440: pseudo-localized max-button stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Max-button arguments can only be used with --max-button-object.",
        "#2518: pseudo-localized max-button stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_form_bounds_style_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_form_bounds_style_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--min-height-object",
            "--min-height", "120",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2441: default min-height diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object min-height assignment requires at least one target selector.",
        "#2441: default min-height diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--min-width-object",
            "--min-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2441: pseudo-localized min-width missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2441: pseudo-localized min-width missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--min-width",
        "#2441: pseudo-localized min-width missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        " min-width assignment ",
        "#2519: pseudo-localized min-width missing-option diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--half-height-caption-object",
            "--half-height-caption", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2441: pseudo-localized half-height-caption missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2441: pseudo-localized half-height-caption missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        " half-height-caption assignment ",
        "#2519: pseudo-localized half-height-caption missing-target diagnostics should not preserve raw label prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--border-color", "255",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2441: pseudo-localized border-color stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2441: pseudo-localized border-color stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--border-color-object",
        "#2441: pseudo-localized border-color stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Border-color arguments can only be used with --border-color-object.",
        "#2519: pseudo-localized border-color stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_dynamic_expression_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_dynamic_expression_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--dynamic-input-mask-object",
            "--dynamic-input-mask", "IIF(.T., '999', '')",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2521: default dynamic-input-mask diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object dynamic-input-mask assignment requires at least one target selector.",
        "#2521: default dynamic-input-mask diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--dynamic-line-height-object",
            "--dynamic-line-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2521: pseudo-localized dynamic-line-height missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2521: pseudo-localized dynamic-line-height missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-line-height",
        "#2521: pseudo-localized dynamic-line-height missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object dynamic-line-height assignment requires --dynamic-line-height.",
        "#2521: pseudo-localized dynamic-line-height missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--dynamic-font-name-object",
            "--dynamic-font-name", "IIF(.T., 'Arial', 'Courier')",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2521: pseudo-localized dynamic-font-name missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2521: pseudo-localized dynamic-font-name missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object dynamic-font-name assignment requires at least one target selector.",
        "#2521: pseudo-localized dynamic-font-name missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--dynamic-font-strikethru", "IIF(.T., .T., .F.)",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2521: pseudo-localized dynamic-font-strikethru stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2521: pseudo-localized dynamic-font-strikethru stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--dynamic-font-strikethru-object",
        "#2521: pseudo-localized dynamic-font-strikethru stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Dynamic-font-strikethru arguments can only be used with --dynamic-font-strikethru-object.",
        "#2521: pseudo-localized dynamic-font-strikethru stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_max_auto_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_max_auto_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--max-width-object",
            "--max-width", "400",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2523: default max-width diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object max-width assignment requires at least one target selector.",
        "#2523: default max-width diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--max-left-object",
            "--max-left-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2523: pseudo-localized max-left missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2523: pseudo-localized max-left missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--max-left",
        "#2523: pseudo-localized max-left missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object max-left assignment requires --max-left.",
        "#2523: pseudo-localized max-left missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--continuous-scroll-object",
            "--continuous-scroll", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2523: pseudo-localized continuous-scroll missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2523: pseudo-localized continuous-scroll missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object continuous-scroll assignment requires at least one target selector.",
        "#2523: pseudo-localized continuous-scroll missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--sparse", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2523: pseudo-localized sparse stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2523: pseudo-localized sparse stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--sparse-object",
        "#2523: pseudo-localized sparse stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Sparse arguments can only be used with --sparse-object.",
        "#2523: pseudo-localized sparse stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_selection_marker_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_selection_marker_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--lock-screen-object",
            "--lock-screen", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2523: default lock-screen diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object lock-screen assignment requires at least one target selector.",
        "#2523: default lock-screen diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--hide-selection-object",
            "--hide-selection-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2524: pseudo-localized hide-selection missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2524: pseudo-localized hide-selection missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--hide-selection",
        "#2524: pseudo-localized hide-selection missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object hide-selection assignment requires --hide-selection.",
        "#2524: pseudo-localized hide-selection missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--highlight-row-object",
            "--highlight-row", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2524: pseudo-localized highlight-row missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2524: pseudo-localized highlight-row missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object highlight-row assignment requires at least one target selector.",
        "#2524: pseudo-localized highlight-row missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--allow-row-sizing", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2524: pseudo-localized allow-row-sizing stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2524: pseudo-localized allow-row-sizing stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--allow-row-sizing-object",
        "#2524: pseudo-localized allow-row-sizing stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Allow-row-sizing arguments can only be used with --allow-row-sizing-object.",
        "#2524: pseudo-localized allow-row-sizing stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_sizing_zorder_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_sizing_zorder_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--resizable-object",
            "--resizable", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2524: default resizable diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object resizable assignment requires at least one target selector.",
        "#2524: default resizable diagnostics should preserve en-US missing-target prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--add-line-feeds-object",
            "--add-line-feeds-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2524: pseudo-localized add-line-feeds missing-option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2524: pseudo-localized add-line-feeds missing-option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--add-line-feeds",
        "#2524: pseudo-localized add-line-feeds missing-option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object add-line-feeds assignment requires --add-line-feeds.",
        "#2524: pseudo-localized add-line-feeds missing-option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--always-on-top-object",
            "--always-on-top", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2525: pseudo-localized always-on-top missing-target diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2525: pseudo-localized always-on-top missing-target diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object always-on-top assignment requires at least one target selector.",
        "#2525: pseudo-localized always-on-top missing-target diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--always-on-bottom", "true",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2525: pseudo-localized always-on-bottom stray-argument diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2525: pseudo-localized always-on-bottom stray-argument diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--always-on-bottom-object",
        "#2525: pseudo-localized always-on-bottom stray-argument diagnostics should preserve required mode option");
    expect_not_contains(process.stdout_text,
        "Always-on-bottom arguments can only be used with --always-on-bottom-object.",
        "#2525: pseudo-localized always-on-bottom stray-argument diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_launch_command_mode_diagnostics_localize(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_launch_command_mode_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");

    auto process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--anchor-object-name", "cmdSave",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2448: default anchor selector mode diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "Anchor selectors can only be used with --align-object or --resize-object.",
        "#2448: default anchor selector mode diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--clear-property",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2488: default clear-property required option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "A property clear requires --property-name.",
        "#2488: default clear-property required option diagnostics should preserve en-US prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--rename-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2502: default rename-object required option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object rename requires --new-object-name, --new-name, or --new-unique-id.",
        "#2502: default rename-object required option diagnostics should preserve en-US list grammar");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--reparent-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2502: default reparent-object required option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "An object reparent requires --parent-name, --parent-unique-id, or --clear-parent.",
        "#2502: default reparent-object required option diagnostics should preserve en-US list grammar");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--set-property",
            "--clear-property",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2448: pseudo-localized duplicate property command diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2448: pseudo-localized duplicate property command diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "Only one property command can be used at a time.",
        "#2448: pseudo-localized duplicate property command diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--delete-object",
            "--restore-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2448: pseudo-localized duplicate object command diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2448: pseudo-localized duplicate object command diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "Only one object command can be used at a time.",
        "#2448: pseudo-localized duplicate object command diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--delete-object",
            "--clear-property",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2448: pseudo-localized mixed command diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2448: pseudo-localized mixed command diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "Object commands cannot be combined with property commands.",
        "#2448: pseudo-localized mixed command diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--rename-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2488: pseudo-localized rename-object required option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2488: pseudo-localized rename-object required option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--new-object-name",
        "#2488: pseudo-localized rename-object required option diagnostics should preserve first CLI option");
    expect_contains(process.stdout_text,
        "--new-name",
        "#2488: pseudo-localized rename-object required option diagnostics should preserve second CLI option");
    expect_contains(process.stdout_text,
        "--new-unique-id",
        "#2488: pseudo-localized rename-object required option diagnostics should preserve third CLI option");
    expect_not_contains(process.stdout_text,
        "An object rename requires --new-object-name, --new-name, or --new-unique-id.",
        "#2502: pseudo-localized rename-object required option diagnostics should not fall back to raw English list grammar");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--reparent-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2502: pseudo-localized reparent-object required option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2502: pseudo-localized reparent-object required option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--parent-name",
        "#2502: pseudo-localized reparent-object required option diagnostics should preserve first CLI option");
    expect_contains(process.stdout_text,
        "--parent-unique-id",
        "#2502: pseudo-localized reparent-object required option diagnostics should preserve second CLI option");
    expect_contains(process.stdout_text,
        "--clear-parent",
        "#2502: pseudo-localized reparent-object required option diagnostics should preserve third CLI option");
    expect_not_contains(process.stdout_text,
        "An object reparent requires --parent-name, --parent-unique-id, or --clear-parent.",
        "#2502: pseudo-localized reparent-object required option diagnostics should not fall back to raw English list grammar");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--reorder-object",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2505: pseudo-localized reorder-object required option diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2505: pseudo-localized reorder-object required option diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--placement",
        "#2505: pseudo-localized reorder-object required option diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object reorder requires --placement.",
        "#2505: pseudo-localized reorder-object required option diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--group-object",
            "--group-child-object-name", "cmdSave",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2488: pseudo-localized group-object required field-value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2488: pseudo-localized group-object required field-value diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--field-value",
        "#2488: pseudo-localized group-object required field-value diagnostics should preserve CLI option names");
    expect_not_contains(process.stdout_text,
        "An object group requires at least one --field-value.",
        "#2488: pseudo-localized group-object required field-value diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--group-object",
            "--field-value", "OBJNAME=cntGroup",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2488: pseudo-localized group-object required child selector diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2488: pseudo-localized group-object required child selector diagnostics should decorate human-facing prose");
    expect_not_contains(process.stdout_text,
        "An object group requires at least one grouped child selector.",
        "#2488: pseudo-localized group-object required child selector diagnostics should not fall back to raw English prose");

    process = run_process_capture(
        studio_host_path,
        {
            "--path", "forms/customer.scx",
            "--field-value", "OBJNAME=cntGroup",
            "--json"
        },
        temp_root);

    expect(process.exit_code == 2,
        "#2488: pseudo-localized group-only field-value diagnostics should preserve parse-failure exit status");
    expect_contains(process.stdout_text,
        "[!! ",
        "#2488: pseudo-localized group-only field-value diagnostics should decorate human-facing prose");
    expect_contains(process.stdout_text,
        "--group-object",
        "#2488: pseudo-localized group-only field-value diagnostics should preserve group command option");
    expect_not_contains(process.stdout_text,
        "--field-value can only be used with --group-object.",
        "#2488: pseudo-localized group-only field-value diagnostics should not fall back to raw English prose");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
