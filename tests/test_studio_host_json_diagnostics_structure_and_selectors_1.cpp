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

}  // namespace cf_test_studio_host_json
