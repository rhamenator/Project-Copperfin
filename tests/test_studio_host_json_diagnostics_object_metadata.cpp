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
}  // namespace cf_test_studio_host_json
